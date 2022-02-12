#include "stdafx.h"
#include "Compiler.h"
#include"BuiltInTypeRegister.h"
#include"VirtualMachine.h"
#include <iomanip>
#include "Parser.h"
#include<direct.h>
auto baseFunc = &ButiScript::VirtualMachine::Initialize;
const char* thisPtrName = "this";
constexpr std::int32_t functionStackSize = 4;
namespace AccessModifierStr {
const char* publicStr = "public";
const char* protectedStr = "protected";
const char* privateStr = "private";
}
ButiScript::AccessModifier ButiScript::StringToAccessModifier(const std::string& arg_modifierStr)
{
	if (arg_modifierStr == AccessModifierStr::publicStr) {
		return AccessModifier::Public;
	}
	if (arg_modifierStr == AccessModifierStr::privateStr) {
		return AccessModifier::Private;
	}
	if (arg_modifierStr == AccessModifierStr::protectedStr) {
		return AccessModifier::Protected;
	}
	return AccessModifier::Public;
}

// コンストラクタ
ButiScript:: Compiler::Compiler()
	: break_index(-1), error_count(0)
{
}

ButiScript::Compiler::~Compiler()
{
	types.Release();
}

ButiScript::Compiler* p_instance;

void ButiScript::Compiler::CreateBaseInstance()
{
	p_instance = new Compiler();
	p_instance->RegistDefaultSystems();
}

ButiScript::Compiler* ButiScript::Compiler::GetBaseInstance()
{
	if (!p_instance) {
		CreateBaseInstance();
	}
	return p_instance;
}

void ButiScript::Compiler::RegistDefaultSystems()
{
	types = SystemTypeRegister::GetInstance()->types;
	enums = SystemTypeRegister::GetInstance()->enums;
	map_valueAllocCallsIndex = SystemTypeRegister::GetInstance()->map_valueAllocCallsIndex;
	map_refValueAllocCallsIndex = SystemTypeRegister::GetInstance()->map_refValueAllocCallsIndex;
	vec_valueAllocCall = SystemTypeRegister::GetInstance()->vec_valueAllocCall;
	vec_refValueAllocCall = SystemTypeRegister::GetInstance()->vec_refValueAllocCall;
	functions = SystemFuntionRegister::GetInstance()->functions;
	map_sysCallsIndex = SystemFuntionRegister::GetInstance()->map_sysCallsIndex;
	map_sysMethodCallsIndex = SystemFuntionRegister::GetInstance()->map_sysMethodCallsIndex;
	vec_sysCalls = SystemFuntionRegister::GetInstance()->vec_sysCalls;
	vec_sysMethodCalls =SystemFuntionRegister::GetInstance()->vec_sysMethodCalls;


	//グローバル名前空間の設定
	globalNameSpace = std::make_shared<NameSpace>("");
	PushNameSpace(globalNameSpace);

}

// コンパイル
bool ButiScript::Compiler::Compile(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data)
{

	//変数テーブルをセット
	variables.push_back(ValueTable());
	variables[0].set_global();
	OpHalt();
	bool result = ScriptParse(arg_filePath, this);	// 構文解析

	if (!result) {

		labels.clear();
		statement.clear();
		text_table.clear();
		variables.clear();
		functions.Clear_notSystem();
		types.Clear_notSystem();
		enums.Clear_notSystem();
		error_count = 0;
		return true;// パーサーエラー
	}

	std::int32_t code_size = LabelSetting();				// ラベルにアドレスを設定
	CreateData(arg_ref_data, code_size);				// バイナリ生成

	arg_ref_data.sourceFilePath = arg_filePath;

	labels.clear();
	statement.clear();
	text_table.clear();
	variables.clear();
	functions.Clear_notSystem();
	types.Clear_notSystem();
	enums.Clear_notSystem();

	return false;
}


// エラーメッセージを出力
void ButiScript::Compiler::error(const std::string& arg_message)
{
	std::cerr << arg_message << std::endl;
	error_count++;
}

void ButiScript::Compiler::ClearStatement()
{
	statement.clear();
}

std::string ButiScript::Compiler::GetTypeName(const std::int32_t arg_type) const
{
	std::int32_t type = arg_type & ~TYPE_REF;
	std::string output = "";
	
	output = types.GetType(type)->typeName;

	if (arg_type&TYPE_REF) {
		output += "&";
	}

	return output;
}

void ButiScript::Compiler::LambdaCountReset()
{
	lambdaCount = 0;
}

void ButiScript::Compiler::PushAnalyzeFunction(Function_t arg_function)
{
	PushNameSpace(std::make_shared<NameSpace>(arg_function->GetName()));
	if (vec_parentFunction.size()) {
		arg_function->SetParent(vec_parentFunction.back());
	}
	vec_parentFunction.push_back(arg_function);

}

void ButiScript::Compiler::PopAnalyzeFunction()
{
	if (vec_parentFunction.size()) {
		vec_parentFunction.erase(vec_parentFunction.end() - 1);
		PopNameSpace();
	}
}

void ButiScript::Compiler::PushSubFunction(Function_t arg_function)
{
	vec_parentFunction.back()->AddSubFunction(arg_function);
}

void ButiScript::Compiler::PushAnalyzeClass(Class_t arg_class)
{
	currentNameSpace->PushClass(arg_class);
}

void ButiScript::Compiler::ClearNameSpace()
{
	vec_namespaces.clear();
	vec_namespaces.push_back(globalNameSpace);
}

void ButiScript::Compiler::Analyze()
{
	auto nameSpaceBuffer = currentNameSpace;
	for (auto namespaceItr = vec_namespaces.begin(), namespaceEnd = vec_namespaces.end(); namespaceItr != namespaceEnd; namespaceItr++) {
		currentNameSpace = (*namespaceItr);
		(*namespaceItr)->AnalyzeClasses(this);
		(*namespaceItr)->AnalyzeFunctions(this);
	}
	currentNameSpace = nameSpaceBuffer;
}

void ButiScript::Compiler::IncreaseLambdaCount()
{
	lambdaCount++;
}

void ButiScript::Compiler::PopCurrentFunctionType()
{
	if (vec_function_type.size()) {
		vec_function_type.erase((vec_function_type.end()-1));
	}
}

void ButiScript::Compiler::PopCurrentFunctionName()
{
	if (vec_function_name.size()) {
		vec_function_name.erase((vec_function_name.end() - 1));
	}
}

void ButiScript::Compiler::ClearGlobalNameSpace()
{
	globalNameSpace->Clear();
}



// 外部変数の定義
struct Define_value {
	ButiScript::Compiler* p_compiler;
	std::int32_t valueType;
	ButiScript::AccessModifier arg_access;
	Define_value(ButiScript::Compiler* arg_p_comp, const std::int32_t arg_type,ButiScript::AccessModifier arg_access ) : p_compiler(arg_p_comp), valueType(arg_type),arg_access(arg_access)
	{
	}

	void operator()(ButiScript::Node_t node) const
	{
		p_compiler->AddValue(valueType, node->GetString(), node->GetLeft(),arg_access);
	}
};

void ButiScript::Compiler::AnalyzeScriptType(const std::string& arg_typeName, const std::map < std::string, std::pair< std::int32_t, AccessModifier>>& arg_memberInfo)
{
	auto typeTag = GetType(arg_typeName);
	if (arg_memberInfo.size()) {
		auto memberInfoEnd = arg_memberInfo.end();
		std::int32_t memberIndex = 0;
		for (auto itr = arg_memberInfo.begin(); itr != memberInfoEnd; memberIndex++,itr++) {
			if (typeTag->typeIndex == itr->second.first) {
				error("クラス "+itr->first + "が自身と同じ型をメンバ変数として保持しています。");
			}
			MemberValueInfo info = { memberIndex ,itr->second.first,itr->second.second };
			typeTag->map_memberValue.emplace(itr->first, info);

		}
	}
}

void ButiScript::Compiler::RegistScriptType(const std::string& arg_typeName)
{

	TypeTag type;
	std::int32_t typeIndex = types.GetSize();
	type.isSystem = false;


	type.typeName = arg_typeName;
	type.typeIndex = typeIndex;
	type.argName = arg_typeName;


	types.RegistType(type);
}

void ButiScript::Compiler::ValueDefine(std::int32_t arg_type, const std::vector<Node_t>& arg_node, const AccessModifier arg_access)
{
	std::for_each(arg_node.begin(), arg_node.end(), Define_value(this, arg_type,arg_access));
}

// 関数宣言
void ButiScript::Compiler::FunctionDefine(std::int32_t arg_type, const std::string& arg_name, const std::vector<std::int32_t>& arg_vec_argIndex)
{
	const FunctionTag* tag = functions.Find_strict(arg_name,arg_vec_argIndex,&GetTypeTable());
	if (tag) {			// 既に宣言済み
		if (!tag->CheckArgList_strict(arg_vec_argIndex)) {
			error("関数 " + arg_name + " に異なる型の引数が指定されています");
			return;
		}
	}
	else {
		FunctionTag func(arg_type, arg_name);
		func.SetArgs(arg_vec_argIndex);				// 引数を設定
		func.SetDeclaration();			// 宣言済み
		func.SetIndex(MakeLabel());		// ラベル登録
		if (functions.Add(arg_name, func, &GetTypeTable()) == 0) {
			error("内部エラー：関数テーブルに登録できません");
		}
	}
}




ButiScript::FunctionTag* ButiScript::Compiler::RegistFunction(const std::int32_t arg_type, const std::string& arg_name, const std::vector<ArgDefine>& arg_vec_argDefines, Block_t arg_block, const AccessModifier arg_access,FunctionTable* arg_funcTable )
{
	std::string functionName =arg_funcTable? arg_name: currentNameSpace->GetGlobalNameString() + arg_name;
	arg_funcTable = arg_funcTable ? arg_funcTable : &functions;

	FunctionTag* tag = arg_funcTable->Find_strict(functionName,arg_vec_argDefines,&GetTypeTable());
	if (tag) {			// 既に宣言済み
		if (!tag->CheckArgList_strict(arg_vec_argDefines)) {
			error("関数 " + functionName + " に異なる型の引数が指定されています");
			return tag;
		}
	}
	else {
		FunctionTag func(arg_type, functionName);
		func.SetArgs(arg_vec_argDefines);				// 引数を設定
		func.SetDeclaration();			// 宣言済み
		func.SetIndex(MakeLabel());		// ラベル登録
		func.SetAccessType(arg_access);
		if (arg_funcTable->Add(functionName, func, &GetTypeTable()) == 0) {
			error("内部エラー：関数テーブルに登録できません");
		}
	}
	return  arg_funcTable->Find_strict(functionName, arg_vec_argDefines,&GetTypeTable());
}

void ButiScript::Compiler::RegistLambda(const std::int32_t arg_type, const std::string& arg_name, const std::vector<ArgDefine>& arg_vec_argDefines, FunctionTable* arg_functionTable)
{
	auto tag= RegistFunction(arg_type,arg_name , arg_vec_argDefines, nullptr, AccessModifier::Public, arg_functionTable);
	tag->isLambda = true;
}

void ButiScript::Compiler::RegistEnum(const std::string& arg_typeName, const std::string& arg_identiferName, const std::int32_t arg_value)
{
	auto enumType = GetEnumTag(arg_typeName);
	if (!enumType) {
		RegistEnumType(arg_typeName);
		enumType = enums.FindType(arg_typeName);
	}

	enumType->SetValue(arg_identiferName, arg_value);
}

void ButiScript::Compiler::RegistEnumType(const std::string& arg_typeName)
{
	auto typeName = currentNameSpace->GetGlobalNameString()+arg_typeName;
	EnumTag tag(typeName);
	enums.SetEnum(tag);
	types.RegistType(*enums.FindType(arg_typeName));
}

std::int32_t ButiScript::Compiler::GetfunctionTypeIndex(const std::vector<ArgDefine>& arg_vec_argmentTypes, const std::int32_t arg_retType)
{
	std::vector<std::int32_t> vec_argTypes;
	std::for_each(arg_vec_argmentTypes.begin(), arg_vec_argmentTypes.end(), [&](const ArgDefine& itr)->void {vec_argTypes.push_back(itr.GetType()); });
	
	return GetfunctionTypeIndex(vec_argTypes,arg_retType);
}
std::int32_t ButiScript::Compiler::GetfunctionTypeIndex(const std::vector<std::int32_t>& arg_vec_argmentTypes, const std::int32_t arg_retType)
{
	auto type = types.GetFunctionType(arg_vec_argmentTypes, arg_retType);
	if (!type) {
		type = types.CreateFunctionType(arg_vec_argmentTypes, arg_retType);
	}
	return type->typeIndex;
}


// 変数の登録
void ButiScript::Compiler::AddValue(const std::int32_t arg_typeIndex, const std::string& arg_name, Node_t arg_node ,const AccessModifier arg_access)
{
	std::string valueName = GetCurrentNameSpace()->GetGlobalNameString() + arg_name;
	std::int32_t size = 1;
	if (arg_node) {
		if (arg_node->Op() != OP_INT) {
			error("配列のサイズは定数で指定してください。");
		}
		else if (arg_node->GetNumber() <= 0) {
			error("配列のサイズは１以上の定数が必要です。");
		}
		size = arg_node->GetNumber();
	}

	ValueTable& values = variables.back();
	if (!values.Add(arg_typeIndex, valueName, arg_access,size)) {
		error("変数 " + valueName + " は既に登録されています。");
	}
}

// ラベル生成
std::int32_t ButiScript::Compiler::MakeLabel()
{
	std::int32_t index = (std::int32_t)labels.size();
	labels.push_back(Label(index));
	return index;
}

// ラベルのダミーコマンドをステートメントリストに登録する
void ButiScript::Compiler::SetLabel(const std::int32_t arg_label)
{
	statement.push_back(VMCode(VM_MAXCOMMAND, arg_label));
}

// 文字列定数をpush
void ButiScript::Compiler::PushString(const std::string& arg_str)
{
	PushString(((std::int32_t)text_table.size()));
	text_table.insert(text_table.end(), arg_str.begin(), arg_str.end());
	text_table.push_back('\0');
}

// break文に対応したJmpコマンド生成

bool ButiScript::Compiler::JmpBreakLabel()
{
	if (break_index < 0) {
		return false;
	}
	OpJmp(break_index);
	return true;
}

// ブロック内では新しい変数セットに変数を登録する
void ButiScript::Compiler::BlockIn(const bool arg_isFunctionBlock, const bool arg_isSubFunctionBlock)
{
	std::int32_t start_addr = 0;					
	if (variables.size() > 1&&!arg_isSubFunctionBlock) {			
		start_addr = variables.back().size();
	}
	variables.push_back(ValueTable(start_addr,arg_isFunctionBlock));
}

void ButiScript::Compiler::BlockOut()
{
	variables.pop_back();
}

void ButiScript::Compiler::ValueAddressAddition(const std::int32_t arg_difference)
{
	auto difference = arg_difference;
	std::int32_t endIndex = 1;
	for (std::int32_t index = variables.size() - 2; index >= endIndex; index--) {
		difference=variables[index].AddressAdd(difference);
		if (variables[index - 1].IsFunctionBlock()) {
			difference += functionStackSize;
		}
	}
}

void ButiScript::Compiler::ValueAddressSubtract(const std::int32_t arg_difference)
{
	auto difference = arg_difference;
	std::int32_t endIndex = 1;
	for (std::int32_t index = variables.size() - 2; index >= endIndex; index--) {
		difference = variables[index].AddressSub(difference);
		if (variables[index - 1].IsFunctionBlock()) {
			difference -= functionStackSize;
		}
	}
}

// ローカル変数用にスタックを確保
void ButiScript::Compiler::AllocStack()
{
	variables.back().Alloc(this);
}

// アドレス計算
struct calc_addr {
	std::vector<ButiScript::Label>& vec_labels;
	std::int32_t& pos;
	calc_addr(std::vector<ButiScript::Label>& arg_vec_labels, std::int32_t& arg_pos) : vec_labels(arg_vec_labels), pos(arg_pos)
	{
	}
	void operator()(const ButiScript::VMCode& code)
	{
		if (code.op == ButiScript::VM_MAXCOMMAND) {			// ラベルのダミーコマンド
			vec_labels[code.GetConstValue<std::int32_t>()].pos = pos;
		}
		else {
			pos += code.size;
		}
	}
};

// ジャンプアドレス設定
struct set_addr {
	std::vector<ButiScript::Label>& vec_labels;
	set_addr(std::vector<ButiScript::Label>& labels) : vec_labels(labels)
	{
	}
	void operator()(ButiScript::VMCode& arg_code)
	{
		switch (arg_code.op) {
		case ButiScript:: VM_JMP:
		case ButiScript::VM_JMPC:
		case ButiScript::VM_JMPNC:
		case ButiScript::VM_TEST:
		case ButiScript::VM_CALL:
		case ButiScript::VM_PUSHFUNCTIONOBJECT:
		case ButiScript::VM_PUSHRAMDA:
			arg_code.SetConstValue( vec_labels[arg_code.GetConstValue<std::int32_t>()].pos);
			break;
		}
	}
};

std::int32_t ButiScript::Compiler::LabelSetting()
{
	// アドレス計算
	std::int32_t pos = 0;
	std::for_each(statement.begin(), statement.end(), calc_addr(labels, pos));
	// ジャンプアドレス設定
	std::for_each(statement.begin(), statement.end(), set_addr(labels));

	return pos;
}

// バイナリデータ生成

struct copy_code {
	std::uint8_t* code;
	copy_code(std::uint8_t* arg_code) : code(arg_code)
	{
	}
	void operator()(const ButiScript::VMCode& arg_code)
	{
		code = arg_code.Get(code);
	}
};

bool ButiScript::Compiler::CreateData(ButiScript::CompiledData& arg_ref_data, std::int32_t arg_codeSize)
{

	for (std::int32_t index = 0; index < functions.Size(); index++) {
		auto func = functions[index];
		if (func->IsSystem()) {
			continue;
		}
		auto name = functions[index]->GetName();
		if (!arg_ref_data.map_entryPoints.count(name)) {
			arg_ref_data.map_entryPoints.emplace(name, labels[functions[index]->index].pos);
		}
		else {
			arg_ref_data.map_entryPoints.emplace(functions[index]->GetNameWithArgment(types), labels[functions[index]->index].pos);
		}
	}
	
	for (auto itr = arg_ref_data.map_entryPoints.begin(), end = arg_ref_data.map_entryPoints.end(); itr != end;itr++) {

		const std::string* p_str = &itr->first;
		arg_ref_data.map_functionJumpPointsTable.emplace(itr->second, p_str);
	}

	arg_ref_data.functions = functions;

	arg_ref_data.commandTable = new std::uint8_t[arg_codeSize];
	arg_ref_data.textBuffer = new char[text_table.size()];
	arg_ref_data.commandSize = arg_codeSize;
	arg_ref_data.textSize = (std::int32_t)text_table.size();
	arg_ref_data.valueSize = (std::int32_t)variables[0].size();

	arg_ref_data.vec_sysCalls = vec_sysCalls;
	arg_ref_data.vec_sysCallMethods = vec_sysMethodCalls;
	types.CreateTypeVec(arg_ref_data.vec_types);
	arg_ref_data.vec_scriptClassInfo = types.GetScriptClassInfo();
	arg_ref_data.functionTypeCount = types.GetFunctionTypeSize();
	for (std::int32_t index = 0; index < arg_ref_data.valueSize; index++) {
		auto p_value = &variables[0][index];
		if (p_value->access == AccessModifier::Public) {
			arg_ref_data.map_globalValueAddress.emplace(variables[0].GetVariableName(index), p_value->GetAddress());
		}
	}
	for (std::int32_t index = 0; index < enums.Size();index++) {
		arg_ref_data.map_enumTag.emplace(enums[index]->typeIndex, *enums[index]);
	}
	if (arg_ref_data.textSize != 0) {
		memcpy(arg_ref_data.textBuffer, &text_table[0], arg_ref_data.textSize);
	}

	std::for_each(statement.begin(), statement.end(), copy_code(arg_ref_data.commandTable));

	auto end = statement.end();
	for (auto itr = statement.begin(); itr != end; itr++) {
		itr->Release();
	}
	return true;
}

void ButiScript::Compiler::PushNameSpace(NameSpace_t arg_namespace)
{
	if (!arg_namespace) {
		arg_namespace = std::make_shared<NameSpace>("");
	}
	if (currentNameSpace) {
		arg_namespace->SetParent(currentNameSpace);
	}
	currentNameSpace = arg_namespace;
	vec_namespaces.push_back(arg_namespace);
}

void ButiScript::Compiler::PopNameSpace()
{
	if (currentNameSpace) {
		currentNameSpace = currentNameSpace->GetParent();
	}
}

// デバッグダンプ
#ifdef	_DEBUG

void ButiScript::Compiler::DebugDump()

#ifdef BUTIGUI_H

// デバッグダンプ
{
	std::string message = "---variables---\n";
	std::uint64_t vsize = variables.size();
	message += "value stack = " + std::to_string(vsize) + '\n';
	for (std::uint64_t index = 0; index < vsize; index++) {
		variables[index].Dump();
	}
	message += "---code---" + '\n';

	static const char* op_name[] = {
#define	VM_NAMETABLE
#include "VM_nametable.h"
#undef	VM_NAMETABLE
		"LABEL",
	};

	std::int32_t	pos = 0;
	std::uint64_t size = statement.size();
	for (std::uint64_t index = 0; index < size; index++) {
		message += std::to_string(std::setw(6)) + std::to_string(pos) + ": " + op_name[statement[index].op];
		if (statement[index].size > 1) {
			message += ", " + std::to_string(statement[index].GetConstValue<std::int32_t>());
		}
		message += '\n';

		if (statement[index].op != VM_MAXCOMMAND) {
			pos += statement[index].size;
		}
	}
	ButiEngine::GUI::Console(message);
}
#else

{
	std::cout << "---variables---" << std::endl;
	std::uint64_t vsize = variables.size();
	std::cout << "value stack = " << vsize << std::endl;
	for (std::uint64_t i = 0; i < vsize; i++) {
		variables[i].Dump();
	}
	std::cout << "---code---" << std::endl;

	static const char* op_name[] = {
#define	VM_NAMETABLE
#include "VM_nametable.h"
#include "Tags.h"
#undef	VM_NAMETABLE
		"LABEL",
	};

	std::int32_t	pos = 0;
	std::uint64_t size = statement.size();
	for (std::uint64_t i = 0; i < size; i++) {
		std::cout << std::setw(6) << pos << ": " << op_name[statement[i].op];
		if (statement[i].size > 1) {
			std::cout << ", " << statement[i].GetConstValue<std::int32_t>();
		}
		std::cout << std::endl;

		if (statement[i].op != VM_MAXCOMMAND) {
			pos += statement[i].size;
		}
	}
}
#endif // BUTIGUI_H
#endif // _DEBUG


std::int32_t ButiScript::Compiler::InputCompiledData(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data)
{
	std::ifstream fIn;
	fIn.open(arg_filePath,std::ios::binary);
	if (!fIn.is_open()) {
		fIn.close();
		return 1;
	}


	std::int32_t sourceFilePathStrSize = 0;
	fIn.read((char*)&sourceFilePathStrSize, sizeof(sourceFilePathStrSize));
	char* sourceFilePathStrBuff = (char*)malloc(sourceFilePathStrSize);
	fIn.read(sourceFilePathStrBuff, sourceFilePathStrSize);
	arg_ref_data.sourceFilePath = std::string(sourceFilePathStrBuff, sourceFilePathStrSize);
	free(sourceFilePathStrBuff);


	fIn.read((char*)&arg_ref_data.commandSize, sizeof(std::int32_t));
	arg_ref_data.commandTable = (std::uint8_t*)malloc(arg_ref_data.commandSize);
	fIn.read((char*)arg_ref_data.commandTable, arg_ref_data.commandSize);

	fIn.read((char*)&arg_ref_data.textSize, sizeof(std::int32_t));
	arg_ref_data.textBuffer = (char*)malloc(arg_ref_data.textSize);
	fIn.read((char*)arg_ref_data.textBuffer, arg_ref_data.textSize);


	fIn.read((char*)&arg_ref_data.valueSize, sizeof(std::int32_t));

	std::int32_t sysCallSize=0;
	fIn.read((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		std::int32_t index=0;
		fIn.read((char*)&index, sizeof(index));
		SysFunction sysFunc = vec_sysCalls[index];
		arg_ref_data.vec_sysCalls.push_back(sysFunc);
	}



	fIn.read((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		std::int32_t index = 0;
		fIn.read((char*)&index, sizeof(index));
		SysFunction sysFunc = vec_sysMethodCalls[index];
		arg_ref_data.vec_sysCallMethods.push_back(sysFunc);
	}


	fIn.read((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		TypeTag typeTag;

		std::int32_t size=0;
		fIn.read((char*)&size, sizeof(size));
		char* p_strBuff = (char*)malloc(size);
		fIn.read(p_strBuff, size);
		typeTag.argName =std::string( p_strBuff,size);
		free(p_strBuff);

		fIn.read((char*)&size, sizeof(size));
		p_strBuff = (char*)malloc(size);
		fIn.read(p_strBuff, size);
		typeTag.typeName = std::string(p_strBuff, size);
		free(p_strBuff);

		std::int32_t index = 0;
		fIn.read((char*)&index, sizeof(index));
		SysFunction sysFunc=nullptr;
		if (index < vec_valueAllocCall.size()&&index>=0) {
			sysFunc = vec_valueAllocCall[index];
		}
		typeTag.typeFunc = sysFunc;

		index = 0;
		fIn.read((char*)&index, sizeof(index));
		if (index < vec_refValueAllocCall.size() && index >= 0) {
			sysFunc = vec_refValueAllocCall[index];
		}
		typeTag.refTypeFunc = sysFunc;




		fIn.read((char*)&typeTag.typeIndex, sizeof(std::int32_t));

		std::int32_t typeMapSize = 0;
		fIn.read((char*)&typeMapSize, sizeof(typeMapSize));

		for (std::int32_t j=0; j < typeMapSize;j++) {
			std::int32_t size =0;
			std::string typeNameStr;
			MemberValueInfo memberInfo;
			fIn.read((char*)&size, sizeof(size));
			char* p_strBuff = (char*)malloc(size);
			free(p_strBuff);
			fIn.read(p_strBuff, size);
			typeNameStr = std::string(p_strBuff,size);
			fIn.read((char*)&memberInfo, sizeof(memberInfo));
			typeTag.map_memberValue.emplace(typeNameStr, memberInfo);
		}

		typeTag.methods.FileInput(fIn);
		typeTag.isSystem = true;
		arg_ref_data.vec_types.push_back(typeTag);
	}


	std::int32_t entryPointsSize = 0;
	fIn.read((char*)&entryPointsSize, sizeof(entryPointsSize));
	
	for (std::int32_t count = 0; count < entryPointsSize;count++) {
		

		std::int32_t size =0, entryPoint = 0;

		fIn.read((char*)&size, sizeof(size));
		char* buff = (char*)malloc(size);
		fIn.read(buff, size);
		std::string name = std::string(buff, size);
		free(buff); 
		fIn.read((char*)&entryPoint, sizeof(std::int32_t));
		std::int32_t current = fIn.tellg();
		arg_ref_data.map_entryPoints.emplace(name, entryPoint);

	}
	for (auto entryPointItr = arg_ref_data.map_entryPoints.begin(), endItr = arg_ref_data.map_entryPoints.end(); entryPointItr != endItr; entryPointItr++) {

		arg_ref_data.map_functionJumpPointsTable.emplace(entryPointItr->second, &entryPointItr->first);
	}

	std::int32_t publicGlobalValue = 0;
	fIn.read((char*)&publicGlobalValue, sizeof(publicGlobalValue));
	for (std::int32_t count = 0; count < publicGlobalValue; count++) {
		std::int32_t strSize = 0;
		fIn.read((char*)&strSize, sizeof(std::int32_t));
		char* strBuff = (char*)malloc(strSize);
		fIn.read(strBuff, strSize);
		std::int32_t address = 0;
		fIn.read((char*)&address, sizeof(std::int32_t));
		arg_ref_data.map_globalValueAddress.emplace(std::string(strBuff, strSize), address);
		arg_ref_data.map_addressToValueName.emplace(address, std::string(strBuff, strSize));
		free(strBuff);
	}

	std::int32_t definedTypeCount = 0;
	fIn.read((char*)&definedTypeCount, sizeof(definedTypeCount));
	arg_ref_data.vec_scriptClassInfo.resize(definedTypeCount);
	for (std::int32_t index = 0; index < definedTypeCount; index++) {
		arg_ref_data.vec_scriptClassInfo.at(index).InputFile(fIn);
	}

	fIn.read((char*)&arg_ref_data.functionTypeCount, sizeof(arg_ref_data.functionTypeCount));

	arg_ref_data.systemTypeCount = arg_ref_data.vec_types.size() - definedTypeCount-arg_ref_data.functionTypeCount;
	for (std::int32_t index = 0; index < definedTypeCount; index++) {
		arg_ref_data.vec_scriptClassInfo.at(index).SetSystemTypeCount(arg_ref_data.systemTypeCount);
		arg_ref_data.vec_types.at(arg_ref_data.vec_scriptClassInfo.at(index).GetTypeIndex()).isSystem = false;
	}
	std::int32_t enumCount =0;
	fIn.read((char*)&enumCount, sizeof(enumCount));
	for (std::int32_t index = 0; index < enumCount;index++) {
		EnumTag tag;
		std::int32_t typeIndex = 0;
		fIn.read((char*)&typeIndex, sizeof(std::int32_t));
		tag.InputFile(fIn);
		arg_ref_data.map_enumTag.emplace(typeIndex,tag);
		arg_ref_data.map_enumTag.at(typeIndex).CreateEnumMap();
		arg_ref_data.vec_types.at(typeIndex).p_enumTag = &arg_ref_data.map_enumTag.at(typeIndex);
	}

	arg_ref_data.functions.FileInput(fIn);

	fIn.close();

	return 0;
}

std::int32_t ButiScript::Compiler::OutputCompiledData(const std::string& arg_filePath, const ButiScript::CompiledData& arg_ref_data)
{

	auto dirPath = StringHelper::GetDirectory(arg_filePath);
	if (dirPath != arg_filePath) {
		auto dirRes = _mkdir(dirPath.c_str());
	}
	std::ofstream fOut(arg_filePath,  std::ios::binary);
	std::int32_t sourcePathSize = arg_ref_data.sourceFilePath.size();
	fOut.write((char*)&sourcePathSize,sizeof(sourcePathSize));
	fOut.write(arg_ref_data.sourceFilePath.c_str(),sourcePathSize);

	fOut.write((char*)&arg_ref_data.commandSize, sizeof(std::int32_t));
	fOut.write((char*)arg_ref_data.commandTable, arg_ref_data.commandSize);

	fOut.write((char*)&arg_ref_data.textSize, sizeof(std::int32_t));
	fOut.write((char*)arg_ref_data.textBuffer,  arg_ref_data.textSize);


	fOut.write((char*)&arg_ref_data.valueSize, sizeof(std::int32_t));

	std::int32_t sysCallSize = arg_ref_data.vec_sysCalls.size();
	fOut.write((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		 auto p_sysCallFunc=arg_ref_data.vec_sysCalls[count];
		 std::int64_t address = *(std::int64_t*) & p_sysCallFunc;
		 std::int32_t index = map_sysCallsIndex.at(address);

		 fOut.write((char*)&index, sizeof(index));
	}

	sysCallSize = arg_ref_data.vec_sysCallMethods.size();
	fOut.write((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		auto p_sysCallFunc = arg_ref_data.vec_sysCallMethods[count];
		std::int64_t address = *(std::int64_t*) & p_sysCallFunc;
		std::int32_t index = map_sysMethodCallsIndex.at(address);

		fOut.write((char*)&index, sizeof(index));
	}


	sysCallSize = arg_ref_data.vec_types.size();
	fOut.write((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		auto p_type =& arg_ref_data.vec_types[count];

		std::int32_t size = p_type->argName.size();
		fOut.write((char*)&size, sizeof(size));
		fOut.write(p_type->argName.c_str(), size); 

		size = p_type->typeName.size();
		fOut.write((char*)&size, sizeof(size));
		fOut.write(p_type->typeName.c_str(),size);

		auto p_sysCallFunc = p_type->typeFunc;

		std::int32_t index = -1;
		if (p_type->isSystem&&!p_type->p_enumTag) {
			index = map_valueAllocCallsIndex.at(p_type->typeIndex);
		}
		fOut.write((char*)&index, sizeof(index));

		p_sysCallFunc = p_type->refTypeFunc;
		if (p_type->isSystem&&!p_type->p_enumTag) {
			index = map_refValueAllocCallsIndex.at(p_type->typeIndex);
		}
		fOut.write((char*)&index, sizeof(index));



		fOut.write((char*)&p_type->typeIndex, sizeof(p_type->typeIndex));

		std::int32_t typeMapSize = p_type->map_memberValue.size();
		fOut.write((char*)&typeMapSize, sizeof(typeMapSize));
		auto end = p_type->map_memberValue.end();
		for (auto itr = p_type->map_memberValue.begin(); itr != end; itr++) {
			std::int32_t size = itr->first.size();
			fOut.write((char*)&size, sizeof(size));
			fOut.write(itr->first.c_str(), size);
			fOut.write((char*)&itr->second, sizeof(itr->second));
		}

		p_type->methods.FileOutput(fOut);

	}

	std::int32_t entryPointsSize = arg_ref_data.map_entryPoints.size();
	fOut.write((char*)&entryPointsSize, sizeof(entryPointsSize));
	auto end = arg_ref_data.map_entryPoints.end();
	for (auto itr = arg_ref_data.map_entryPoints.begin(); itr != end; itr++) {
		std::int32_t size = itr->first.size();
		fOut.write((char*)&size,sizeof(size));
		fOut.write(itr->first.c_str(), size);
		fOut.write((char*)&itr->second, sizeof(itr->second));
	}

	std::int32_t publicGlobalValue = arg_ref_data.map_globalValueAddress.size();
	fOut.write((char*)&publicGlobalValue, sizeof(publicGlobalValue));
	auto itr = arg_ref_data.map_globalValueAddress.begin();
	for (std::int32_t count = 0; count < publicGlobalValue; count++,itr++) {
		std::int32_t strSize =itr->first.size();
		fOut.write((char*)&strSize, sizeof(std::int32_t));
		fOut.write(itr->first.c_str(), strSize);
		fOut.write((char*)&itr->second, sizeof(std::int32_t));
	}

	std::int32_t defineTypeCount = arg_ref_data.vec_scriptClassInfo.size();
	fOut.write((char*)&defineTypeCount, sizeof(defineTypeCount));
	for (std::int32_t index = 0; index < defineTypeCount; index++) {
		arg_ref_data.vec_scriptClassInfo[index].OutputFile(fOut);
	}

	fOut.write((char*)&arg_ref_data.functionTypeCount, sizeof(arg_ref_data.functionTypeCount));


	std::int32_t enumCount = arg_ref_data.map_enumTag.size();
	fOut.write((char*)&enumCount, sizeof(enumCount));
	for (auto itr = arg_ref_data.map_enumTag.begin(), end = arg_ref_data.map_enumTag.end(); itr != end; itr++) {
		fOut.write((char*)&itr->first,sizeof(std::int32_t));
		itr->second.OutputFile(fOut);

	}
	arg_ref_data.functions.FileOutput(fOut);
	
	fOut.close();

	return 0;
}
const std::string& ButiScript::NameSpace::GetNameString() const
{
	return name;
}

std::string ButiScript::NameSpace::GetGlobalNameString() const
{
	if (shp_parentNamespace) {
		return shp_parentNamespace->GetGlobalNameString() + name+ "::";
	}

	if (name.size()==0) {
		return "";
	}

	return name+"::";
}

void ButiScript::NameSpace::Regist(Compiler* arg_compiler)
{
	arg_compiler->PushNameSpace(std::make_shared<NameSpace>(name));
}

void ButiScript::NameSpace::SetParent(std::shared_ptr<NameSpace> arg_parent)
{
	shp_parentNamespace = arg_parent;
}

std::shared_ptr<ButiScript::NameSpace> ButiScript::NameSpace::GetParent() const
{
	return shp_parentNamespace;
}

void ButiScript::NameSpace::PushFunction(Function_t arg_func)
{
	vec_analyzeFunctionBuffer.push_back(arg_func);
}

void ButiScript::NameSpace::PushClass(Class_t arg_class)
{
	vec_analyzeClassBuffer.push_back(arg_class);
}



void ButiScript::NameSpace::AnalyzeFunctions(Compiler* arg_compiler)
{
	for (auto itr = vec_analyzeFunctionBuffer.begin(), end = vec_analyzeFunctionBuffer.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler, nullptr);
	}
}

void ButiScript::NameSpace::AnalyzeClasses(Compiler* arg_compiler)
{
	for (auto itr = vec_analyzeClassBuffer.begin(), end = vec_analyzeClassBuffer.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler);
	}
}

void ButiScript::NameSpace::Clear()
{
	vec_analyzeClassBuffer.clear();
	vec_analyzeFunctionBuffer.clear();
}


void ButiScript::ValueTable::Alloc(Compiler* arg_comp) const
{
	auto end = vec_variableTypes.end();
	for (auto itr = vec_variableTypes.begin(); itr != end; itr++)
	{
		if (*itr & TYPE_REF) {
			std::int32_t typeIndex = *itr &~TYPE_REF;
			auto type = arg_comp->GetType(typeIndex);
			if (type->isSystem) {
				arg_comp->OpAllocStack_Ref(*itr);
			}
			else  if (type->p_enumTag) {
				arg_comp->OpAllocStack_Ref_EnumType(*itr);
			}
			else  if (type->IsFunctionObjectType()) {
				arg_comp->OpAllocStack_Ref_FunctionType(*itr);
			}
			else {
				arg_comp->OpAllocStack_Ref_ScriptType(*itr - arg_comp->GetSystemTypeSize());
			}
		}
		else {
			auto type = arg_comp->GetType(*itr);
			if (type->p_enumTag) {
				arg_comp->OpAllocStackEnumType(*itr);
			}else if (type->isSystem) {
				arg_comp->OpAllocStack(*itr);
			}
			else  if (type->IsFunctionObjectType()) {
				arg_comp->OpAllocStackFunctionType(*itr);
			}
			else
			{
				arg_comp->OpAllocStack_ScriptType(*itr - arg_comp->GetSystemTypeSize());
			}
		}
		
	}
}

ButiScript::SystemTypeRegister* p_sysreginstance ;

ButiScript::SystemTypeRegister* ButiScript::SystemTypeRegister::GetInstance()
{
	if (!p_sysreginstance) {

		p_sysreginstance = new SystemTypeRegister();

		p_sysreginstance->SetDefaultSystemType();

	}
	return p_sysreginstance;
}
void ButiScript::SystemTypeRegister::SetDefaultSystemType()
{
	RegistSystemType_<std::int32_t>("std::int32_t", "i");
	RegistSystemType_<float>("float", "f");
	RegistSystemType_<std::string>("string", "s");
	RegistSystemType_<Type_Null>("void", "v");
	RegistSystemType_<ButiEngine::Vector2>("Vector2", "vec2", "x:f,y:f");
	RegistSystemType_<ButiEngine::Vector3>("Vector3", "vec3", "x:f,y:f,z:f");
	RegistSystemType_<ButiEngine::Vector4>("Vector4", "vec4", "x:f,y:f,z:f,w:f");
	RegistSystemType_<ButiEngine::Matrix4x4>("Matrix4x4", "Matrix4x4", "_m11:f,_m12:f,_m13:f,_m14:f,_m21:f,_m22:f,_m23:f,_m24:f,_m31:f,_m32:f,_m33:f,_m34:f,_m41:f,_m42:f,_m43:f,_m44:f");
}
void ButiScript::SystemTypeRegister::RegistSystemEnumType(const std::string& arg_typeName)
{
	EnumTag tag(arg_typeName);
	tag.isSystem = true;
	enums.SetEnum(tag);
	types.RegistType(*enums.FindType(arg_typeName));
}
void ButiScript::SystemTypeRegister::RegistEnum(const std::string& arg_typeName, const std::string& arg_identiferName, const std::int32_t arg_value)
{
	auto enumType = enums.FindType(arg_typeName);
	if (!enumType) {
		RegistSystemEnumType(arg_typeName);
		enumType = enums.FindType(arg_typeName);
	}

	enumType->SetValue(arg_identiferName, arg_value);
}
std::int32_t ButiScript::SystemTypeRegister::GetIndex(const std::string& arg_typeName)
{
	auto tag = types.GetType(arg_typeName);
	if (tag) {
		return tag->typeIndex;
	}
	return -1;
}

ButiScript::SystemFuntionRegister* p_sysfuncregister;
ButiScript::SystemFuntionRegister* ButiScript::SystemFuntionRegister::GetInstance()
{
	if (!p_sysfuncregister) {
		p_sysfuncregister = new SystemFuntionRegister();
		p_sysfuncregister->SetDefaultFunctions();
	}

	return p_sysfuncregister;
}

void ButiScript::SystemFuntionRegister::SetDefaultFunctions()
{
	using namespace ButiEngine;
	DefineSystemFunction(&VirtualMachine::sys_print, TYPE_VOID, "print", "s");
	//DefineSystemFunction(&VirtualMachine::sys_debugPrint, TYPE_VOID, "print_debug", "s");
	DefineSystemFunction(&VirtualMachine::Sys_pause, TYPE_VOID, "pause", "");
	DefineSystemFunction(&VirtualMachine::sys_tostr, TYPE_STRING, "ToString", "i");
	DefineSystemFunction(&VirtualMachine::sys_tostr, TYPE_STRING, "ToString", "f");
	DefineSystemFunction(&VirtualMachine::sys_tostr, TYPE_STRING, "ToString", "vec2");
	DefineSystemFunction(&VirtualMachine::sys_tostr, TYPE_STRING, "ToString", "vec3");
	DefineSystemFunction(&VirtualMachine::sys_tostr, TYPE_STRING, "ToString", "vec4");
#ifdef _BUTIENGINEBUILD

	DefineSystemFunction(&VirtualMachine::sys_registEventListner, TYPE_STRING, "RegistEvent", "s,s,s");
	DefineSystemFunction(&VirtualMachine::sys_unregistEventListner, TYPE_VOID, "UnRegistEvent", "s,s");
	DefineSystemFunction(&VirtualMachine::sys_addEventMessanger, TYPE_VOID, "AddEventMessanger", "s");
	DefineSystemFunction(&VirtualMachine::sys_removeEventMessanger, TYPE_VOID, "RemoveEventMessanger", "s");
	DefineSystemFunction(&VirtualMachine::sys_executeEvent, TYPE_VOID, "EventExecute", "s");
	DefineSystemFunction(&VirtualMachine::sys_pushTask, TYPE_VOID, "PushTask", "s");
#endif // _BUTIENGINEBUILD

	DefineSystemMethod(&VirtualMachine::sys_method_retCast< std::string, std::uint64_t,std::int32_t, &std::string::size, &VirtualMachine::GetTypePtr  >, TYPE_STRING, TYPE_INTEGER, "Size", "");

	DefineSystemMethod(&VirtualMachine::sys_method_retNo< Vector2, &Vector2::Normalize, &VirtualMachine::GetTypePtr >, TYPE_VOID + 1, TYPE_VOID, "Normalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector2, Vector2, &Vector2::GetNormalize, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetNormalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector2, float, &Vector2::GetLength, &VirtualMachine::GetTypePtr >, TYPE_VOID + 1, TYPE_FLOAT, "GetLength", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector2, float, &Vector2::GetLengthSqr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_FLOAT, "GetLengthSqr", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector2, float,const Vector2&, &Vector2::Dot, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_FLOAT, "Dot", "vec2");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector2, Vector2&, std::int32_t, &Vector2::Floor, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, (TYPE_VOID + 1), "Floor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector2, Vector2&, std::int32_t, &Vector2::Round, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, (TYPE_VOID + 1), "Round", "i");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector2, Vector2&, std::int32_t, &Vector2::Ceil, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 1, (TYPE_VOID + 1), "Ceil", "i");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector2, Vector2, std::int32_t, &Vector2::GetFloor, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetFloor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector2, Vector2, std::int32_t, &Vector2::GetRound, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetRound", "i");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector2, Vector2, std::int32_t, &Vector2::GetCeil, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetCeil", "i");

	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, Vector3&, &Vector3::Normalize, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID, "Normalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, Vector3, &Vector3::GetNormalize, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 2, TYPE_VOID + 2, "GetNormalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, float, &Vector3::GetLength, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_FLOAT, "GetLength", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, float, &Vector3::GetLengthSqr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_FLOAT, "GetLengthSqr", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, float,const Vector3&, &Vector3::Dot, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_FLOAT, "Dot", "vec3");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, Vector3,const Vector3&, &Vector3::GetCross, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetCross", "vec3");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, Vector3&, std::int32_t, &Vector3::Floor, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 2, (TYPE_VOID + 2), "Floor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, Vector3&, std::int32_t, &Vector3::Round, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 2, (TYPE_VOID + 2), "Round", "i");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, Vector3&, std::int32_t, &Vector3::Ceil, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, (TYPE_VOID + 2), "Ceil", "i");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, Vector3, std::int32_t, &Vector3::GetFloor, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 2, TYPE_VOID + 2, "GetFloor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, Vector3, std::int32_t, &Vector3::GetRound, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetRound", "i");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Vector3, Vector3, std::int32_t, &Vector3::GetCeil, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetCeil", "i");

	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4, Matrix4x4&, &Matrix4x4::Transpose, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "Transpose", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4, Matrix4x4, &Matrix4x4::GetTranspose, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "GetTranspose", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4, Matrix4x4&, &Matrix4x4::Inverse, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "Inverse", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4, Matrix4x4, &Matrix4x4::GetInverse, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "GetInverse", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4, Matrix4x4&, &Matrix4x4::Identity, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "Identity", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4, Vector3, &Matrix4x4::GetEulerOneValue, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetEulerOneValue", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4, Vector3, &Matrix4x4::GetPosition, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetPosition", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4, Vector3, &Matrix4x4::GetPosition_Transpose, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetPosition_Transpose", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4,Vector3, &Matrix4x4::GetEulerOneValue_local, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetEulerOneValue_local", "");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4, Matrix4x4&,const Vector3&, &Matrix4x4::CreateFromEuler, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "CreateFromEuler", "vec3");
	DefineSystemMethod(&VirtualMachine::sys_method_ret< Matrix4x4, Matrix4x4&, const Vector3&, &Matrix4x4::CreateFromEuler_local, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "CreateFromEuler_local", "vec3");
	
}

bool ButiScript::SystemFuntionRegister::DefineSystemFunction(SysFunction arg_op, const std::int32_t arg_retType, const std::string& arg_name, const std::string& arg_vec_argDefines, const std::vector<std::int32_t>& arg_vec_templateTypes)
{
	FunctionTag func(arg_retType, arg_name);
	if (!func.SetArgs(arg_vec_argDefines, SystemTypeRegister::GetInstance()->types .GetArgmentKeyMap()))
		return false;

	func.SetDeclaration();
	func.SetSystem();				// Systemフラグセット
	std::int32_t index = vec_sysCalls.size();
	func.SetIndex(index);			// 組み込み関数番号を設定
	func.SetTemplateType(arg_vec_templateTypes);
	std::int64_t address = *(std::int64_t*) & arg_op;
	map_sysCallsIndex.emplace(address, vec_sysCalls.size());
	vec_sysCalls.push_back(arg_op);
	if (functions.Add(arg_name, func, &SystemTypeRegister::GetInstance()->types) == 0) {
		return false;
	}
	return true;
}

bool ButiScript::SystemFuntionRegister::DefineSystemMethod(SysFunction arg_p_method, const std::int32_t arg_type, const std::int32_t arg_retType, const std::string& arg_name, const std::string& arg_args, const std::vector<std::int32_t>& arg_vec_templateTypes)
{
	FunctionTag func(arg_retType, arg_name);
	if (!func.SetArgs(arg_args, SystemTypeRegister::GetInstance()->types.GetArgmentKeyMap()))
		return false;

	func.SetDeclaration();
	func.SetSystem();				// Systemフラグセット
	func.SetIndex(vec_sysMethodCalls.size());			// 組み込み関数番号を設定
	func.SetTemplateType(arg_vec_templateTypes);
	std::int64_t address = *(std::int64_t*) & arg_p_method;
	map_sysMethodCallsIndex.emplace(address, vec_sysMethodCalls.size());
	vec_sysMethodCalls.push_back(arg_p_method);
	auto typeTag = SystemTypeRegister::GetInstance()->types.GetType(arg_type);
	if (typeTag->AddMethod(arg_name, func,&SystemTypeRegister::GetInstance()->types) == 0) {
		return false;
	}
	return true;
}
std::int32_t ButiScript::TypeTag::GetFunctionObjectReturnType() const
{
	if (!p_functionObjectData) {
		return -1;
	}

	return p_functionObjectData->returnType;
}
std::int32_t ButiScript::TypeTag::GetFunctionObjectArgSize() const
{
	if (!p_functionObjectData) {
		return -1;
	}
	return p_functionObjectData->vec_argTypes.size();
}

const std::vector<std::int32_t>& ButiScript::TypeTag::GetFunctionObjectArgment() const
{

	return p_functionObjectData->vec_argTypes;
}

void ButiScript::TypeTable::Release() {
	for (auto& type: map_types) {
		type.second.Release();
	}
	map_types.clear();
	map_argmentChars.clear();
	vec_types.clear();
}

#ifndef _BUTIENGINEBUILD
template<typename T>
class MemoryReleaser {
public:
	MemoryReleaser(T** arg_p_memoryAddress) :p_memoryAddress(arg_p_memoryAddress) {}
	~MemoryReleaser()
	{
		if (*p_memoryAddress) {
			delete (*p_memoryAddress);
		}
	}
private:
	T** p_memoryAddress;
};

auto compilerRelease = MemoryReleaser<ButiScript::Compiler>(&p_instance);
auto sysTypeRelease = MemoryReleaser<ButiScript::SystemTypeRegister>(&p_sysreginstance);
auto sysFuncRelease = MemoryReleaser<ButiScript::SystemFuntionRegister>(&p_sysfuncregister);
#else

auto compilerRelease = ButiEngine::Util::MemoryReleaser(&p_instance);
auto sysTypeRelease = ButiEngine::Util::MemoryReleaser(&p_sysreginstance);
auto sysFuncRelease = ButiEngine::Util::MemoryReleaser(&p_sysfuncregister);
#endif
