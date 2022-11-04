#include "stdafx.h"
#include "Compiler.h"
#include "Node.h"
#include"ButiScript_VirtualMachine/BuiltInTypeRegister.h"
#include"ButiScript_VirtualMachine/VirtualMachine.h"
#include <iomanip>
#include "Parser.h"
#include<direct.h>
#include"ButiUtil/ButiUtil/ObjectFactory.h"
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
	: breakIndex(-1), errorCount(0)
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
	list_valueAllocCall = SystemTypeRegister::GetInstance()->list_valueAllocCall;
	list_refValueAllocCall = SystemTypeRegister::GetInstance()->list_refValueAllocCall;
	functions = SystemFuntionRegister::GetInstance()->functions;
	map_sysCallsIndex = SystemFuntionRegister::GetInstance()->map_sysCallsIndex;
	map_sysMethodCallsIndex = SystemFuntionRegister::GetInstance()->map_sysMethodCallsIndex;
	list_sysCalls = SystemFuntionRegister::GetInstance()->list_sysCalls;
	list_sysMethodCalls =SystemFuntionRegister::GetInstance()->list_sysMethodCalls;


	//グローバル名前空間の設定
	globalNameSpace = ButiEngine::make_value<NameSpace>("");
	PushNameSpace(globalNameSpace);

}

// コンパイル
bool ButiScript::Compiler::Compile(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data)
{

	Clear();
	//変数テーブルをセット
	variables.Add(ValueTable());
	variables[0].set_global();
	OpHalt();
	bool result = ScriptParse(arg_filePath, this);	// 構文解析
	if (!result) {
		Clear();
		return true;// パーサーエラー
	}

	std::int32_t code_size = LabelSetting();				// ラベルにアドレスを設定
	CreateData(arg_ref_data, code_size);				// バイナリ生成

	arg_ref_data.sourceFilePath = arg_filePath;
	Clear();
	return false;
}


// エラーメッセージを出力
void ButiScript::Compiler::error(const std::string& arg_message)
{
	std::cerr << arg_message << std::endl;
	errorCount++;
}

void ButiScript::Compiler::ClearStatement()
{
	statement.Clear();
}

std::string ButiScript::Compiler::GetTypeName(const std::int32_t arg_type) const
{
	std::int32_t type = arg_type & ~TYPE_REF;
	
	std::string output = types.GetType(type) ? types.GetType(type)->typeName: "";
	

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
	PushNameSpace(ButiEngine::make_value<NameSpace>(arg_function->GetName()));
	if (list_parentFunction.GetSize()) {
		arg_function->SetParent(list_parentFunction.GetLast());
	}
	list_parentFunction.Add(arg_function);

}

void ButiScript::Compiler::PopAnalyzeFunction()
{
	if (list_parentFunction.GetSize()) {
		list_parentFunction.erase(list_parentFunction.end() - 1);
		PopNameSpace();
	}
}

void ButiScript::Compiler::PushSubFunction(Function_t arg_function)
{
	list_parentFunction.GetLast()->AddSubFunction(arg_function);
}

void ButiScript::Compiler::PushAnalyzeClass(Class_t arg_class)
{
	currentNameSpace->PushClass(arg_class);
}

void ButiScript::Compiler::ClearNameSpace()
{
	for (auto analyzeNamespace: list_namespaces) {
		analyzeNamespace->Clear();
	}
	list_namespaces.Clear();
	list_namespaces.Add(globalNameSpace);
}

void ButiScript::Compiler::Analyze()
{
	auto nameSpaceBuffer = currentNameSpace;
	for (auto namespaceItr : list_namespaces) {
		currentNameSpace = namespaceItr;
		namespaceItr->DeclareClasses(this);
	}
	for (auto namespaceItr : list_namespaces) {
		currentNameSpace = namespaceItr;
		namespaceItr->DeclareValues(this);
	}
	OpHalt();
	for (auto namespaceItr : list_namespaces) {
		currentNameSpace = namespaceItr;
		namespaceItr->DeclareFunctions(this);
	}
	for (auto namespaceItr : list_namespaces) {
		currentNameSpace = namespaceItr;
		namespaceItr->AnalyzeClasses(this);
	}
	for (auto namespaceItr : list_namespaces) {
		currentNameSpace = namespaceItr;
		namespaceItr->AnalyzeFunctions(this);
	}

	currentNameSpace = nameSpaceBuffer;
}

void ButiScript::Compiler::IncreaseLambdaCount()
{
	lambdaCount++;
}

void ButiScript::Compiler::PopCurrentFunctionType()
{
	if (list_function_type.GetSize()) {
		list_function_type.erase((list_function_type.end()-1));
	}
}

void ButiScript::Compiler::PopCurrentFunctionName()
{
	if (list_function_name.GetSize()) {
		list_function_name.erase((list_function_name.end() - 1));
	}
}

void ButiScript::Compiler::ClearGlobalNameSpace()
{
	globalNameSpace->Clear();
}

std::int32_t ButiScript::Compiler::AddFunction(Function_t arg_function)
{
	if (currentNameSpace) {
		currentNameSpace->PushFunction(arg_function);
		PopNameSpace();
		return 0;
	}
	return -1;
}

std::int32_t ButiScript::Compiler::AddValue(Declaration_t arg_valueDecl)
{
	if (currentNameSpace) {
		currentNameSpace->PushValue(arg_valueDecl);
		return 0;
	}
	return -1;
}



// 外部変数の定義
void DefineValue(ButiScript::Compiler* arg_p_comp, const std::int32_t arg_type, ButiScript::AccessModifier arg_access,const std::string& arg_varName ) {

	arg_p_comp->AddValue(arg_type, arg_varName, arg_access);
}

void ButiScript::Compiler::AnalyzeScriptType(const std::string& arg_typeName, const std::map < std::string, std::pair< std::string, AccessModifier>>& arg_memberInfo)
{
	auto typeTag = GetType(arg_typeName)?GetType(arg_typeName): RegistScriptType(arg_typeName);
	std::int32_t memberIndex = 0;
	for (auto& itr : arg_memberInfo) {
		if (typeTag->typeIndex == GetTypeIndex(itr.second.first)) {
			error("クラス " + itr.first + "が自身と同じ型をメンバ変数として保持しています。");
		}
		MemberValueInfo info = { memberIndex ,GetTypeIndex(itr.second.first),itr.second.second };
		typeTag->map_memberValue.emplace(itr.first, info);
		memberIndex++;
	}
}

ButiScript::TypeTag* ButiScript::Compiler::RegistScriptType(const std::string& arg_typeName)
{

	TypeTag type;
	std::int32_t typeIndex = types.GetSize();
	type.isSystem = false;


	type.typeName = arg_typeName;
	type.typeIndex = typeIndex;
	type.argName = arg_typeName;


	types.RegistType(type);
	return GetType(arg_typeName);
}

void ButiScript::Compiler::ValueDefine(std::int32_t arg_type, const ButiEngine::List<std::string>& arg_varName, const AccessModifier arg_access)
{
	for (auto& name: arg_varName) {
		DefineValue(this, arg_type, arg_access,name);
	}
}

// 関数宣言
void ButiScript::Compiler::FunctionDefine(std::int32_t arg_type, const std::string& arg_name, const ButiEngine::List<std::int32_t>& arg_list_argIndex)
{
	const FunctionTag* tag = functions.Find_strict(arg_name,arg_list_argIndex,&GetTypeTable());
	if (tag) {			// 既に宣言済み
		if (!tag->CheckArgList_strict(arg_list_argIndex)) {
			error("関数 " + arg_name + " に異なる型の引数が指定されています");
			return;
		}
	}
	else {
		FunctionTag func(arg_type, arg_name);
		func.SetArgs(arg_list_argIndex);				// 引数を設定
		func.SetDeclaration();			// 宣言済み
		func.SetIndex(MakeLabel());		// ラベル登録
		if (functions.Add(arg_name, func, &GetTypeTable()) == 0) {
			error("内部エラー：関数テーブルに登録できません");
		}
	}
}




ButiScript::FunctionTag* ButiScript::Compiler::RegistFunction(const std::int32_t arg_type, const std::string& arg_name, const ButiEngine::List<ArgDefine>& arg_list_argDefines, Block_t arg_block, const AccessModifier arg_access,FunctionTable* arg_funcTable )
{
	std::string functionName =arg_funcTable? arg_name: currentNameSpace->GetGlobalNameString() + arg_name;
	arg_funcTable = arg_funcTable ? arg_funcTable : &functions;

	FunctionTag* tag = arg_funcTable->Find_strict(functionName,arg_list_argDefines,&GetTypeTable());
	if (tag) {			// 既に宣言済み
		if (!tag->CheckArgList_strict(arg_list_argDefines)) {
			error("関数 " + functionName + " に異なる型の引数が指定されています");
			return tag;
		}
	}
	else {
		FunctionTag func(arg_type, functionName);
		func.SetArgs(arg_list_argDefines);				// 引数を設定
		func.SetDeclaration();			// 宣言済み
		func.SetIndex(MakeLabel());		// ラベル登録
		func.SetAccessType(arg_access);
		if (arg_funcTable->Add(functionName, func, &GetTypeTable()) == 0) {
			error("内部エラー：関数テーブルに登録できません");
		}
	}
	return  arg_funcTable->Find_strict(functionName, arg_list_argDefines,&GetTypeTable());
}

void ButiScript::Compiler::RegistLambda(const std::int32_t arg_type, const std::string& arg_name, const ButiEngine::List<ArgDefine>& arg_list_argDefines, FunctionTable* arg_functionTable)
{
	auto tag= RegistFunction(arg_type,arg_name , arg_list_argDefines, nullptr, AccessModifier::Public, arg_functionTable);
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

std::int32_t ButiScript::Compiler::GetfunctionTypeIndex(const ButiEngine::List<ArgDefine>& arg_list_argmentTypes, const std::int32_t arg_retType)
{
	ButiEngine::List<std::int32_t> list_argTypes;
	for (auto& argmentType : arg_list_argmentTypes) {
		list_argTypes.Add(argmentType.GetType());
	}

	
	return GetfunctionTypeIndex(list_argTypes,arg_retType);
}
std::int32_t ButiScript::Compiler::GetfunctionTypeIndex(const ButiEngine::List<std::int32_t>& arg_list_argmentTypes, const std::int32_t arg_retType)
{
	auto type = types.GetFunctionType(arg_list_argmentTypes, arg_retType);
	if (!type) {
		type = types.CreateFunctionType(arg_list_argmentTypes, arg_retType);
	}
	return type->typeIndex;
}


// 変数の登録
void ButiScript::Compiler::AddValue(const std::int32_t arg_typeIndex, const std::string& arg_name, const AccessModifier arg_access)
{
	std::string valueName = GetCurrentNameSpace()->GetGlobalNameString() + arg_name;
	

	ValueTable& values = variables.GetLast();
	if (!values.Add(arg_typeIndex, valueName, arg_access,1)) {
		error("変数 " + valueName + " は既に登録されています。");
	}
}

// ラベル生成
std::int32_t ButiScript::Compiler::MakeLabel()
{
	std::int32_t index = (std::int32_t)labels.GetSize();
	labels.Add(Label(index));
	return index;
}

// ラベルのダミーコマンドをステートメントリストに登録する
void ButiScript::Compiler::SetLabel(const std::int32_t arg_label)
{
	statement.Add(VMCode(VM_MAXCOMMAND, arg_label));
}

// 文字列定数をpush
void ButiScript::Compiler::PushString(const std::string& arg_str)
{
	PushString(((std::int32_t)text_table.GetSize()));
	text_table.Insert(text_table.end(), arg_str.begin(), arg_str.end());
	text_table.Add('\0');
}

// break文に対応したJmpコマンド生成

bool ButiScript::Compiler::JmpBreakLabel()
{
	if (breakIndex < 0) {
		return false;
	}
	OpJmp(breakIndex);
	return true;
}

// ブロック内では新しい変数セットに変数を登録する
void ButiScript::Compiler::BlockIn(const bool arg_isFunctionBlock, const bool arg_isSubFunctionBlock)
{
	std::int32_t start_addr = 0;					
	if (variables.GetSize() > 1&&!arg_isSubFunctionBlock) {			
		start_addr = variables.GetLast().size();
	}
	variables.Add(ValueTable(start_addr,arg_isFunctionBlock));
}

void ButiScript::Compiler::BlockOut()
{
	variables.RemoveLast();
}

void ButiScript::Compiler::ValueAddressAddition(const std::int32_t arg_difference)
{
	auto difference = arg_difference;
	std::int32_t endIndex = 1;
	for (std::int32_t index = variables.GetSize() - 2; index >= endIndex; index--) {
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
	for (std::int32_t index = variables.GetSize() - 2; index >= endIndex; index--) {
		difference = variables[index].AddressSub(difference);
		if (variables[index - 1].IsFunctionBlock()) {
			difference -= functionStackSize;
		}
	}
}

// ローカル変数用にスタックを確保
void ButiScript::Compiler::AllocStack()
{
	variables.GetLast().Alloc(this);
}

// アドレス計算
void CalcAddress(ButiEngine::List<ButiScript::Label>& arg_list_labels, std::int32_t& arg_pos, const ButiScript::VMCode& arg_code) {

	if (arg_code.op == ButiScript::VM_MAXCOMMAND) {			// ラベルのダミーコマンド
		arg_list_labels[arg_code.GetConstValue<std::int32_t>()].pos = arg_pos;
	}
	else {
		arg_pos += arg_code.size;
	}
}
void SetAddress(ButiEngine::List<ButiScript::Label>& arg_list_labels,   ButiScript::VMCode& arg_code) {
	switch (arg_code.op) {
	case ButiScript::VM_JMP:
	case ButiScript::VM_JMPC:
	case ButiScript::VM_JMPNC:
	case ButiScript::VM_TEST:
	case ButiScript::VM_CALL:
	case ButiScript::VM_PUSHFUNCTIONOBJECT:
	case ButiScript::VM_PUSHRAMDA:
		arg_code.SetConstValue(arg_list_labels[arg_code.GetConstValue<std::int32_t>()].pos);
		break;
	}
}

std::int32_t ButiScript::Compiler::LabelSetting()
{
	// アドレス計算
	std::int32_t pos = 0;
	for (auto& statementItr : statement)
	{
		CalcAddress(labels, pos, statementItr);
	}
	for (auto& statementItr : statement)
	{
		SetAddress(labels,  statementItr);
	}

	return pos;
}

// バイナリデータ生成
std::uint8_t* CodeCopy(std::uint8_t* arg_p_code, const ButiScript::VMCode& arg_code) {

	return arg_code.Get(arg_p_code);
}


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
	arg_ref_data.textBuffer = new char[text_table.GetSize()];
	arg_ref_data.commandSize = arg_codeSize;
	arg_ref_data.textSize = (std::int32_t)text_table.GetSize();
	arg_ref_data.valueSize = (std::int32_t)variables[0].size();

	arg_ref_data.list_sysCalls = list_sysCalls;
	arg_ref_data.list_sysCallMethods = list_sysMethodCalls;
	types.CreateTypeVec(arg_ref_data.list_types);
	arg_ref_data.list_scriptClassInfo = types.GetScriptClassInfo();
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
	auto commandStartPtr = arg_ref_data.commandTable;
	for (auto& statementItr : statement) {
		arg_ref_data.commandTable=CodeCopy(arg_ref_data.commandTable, statementItr);
	}
	arg_ref_data.commandTable = commandStartPtr;
	for (auto& statementItr : statement) {
		statementItr.Release();
	}
	return true;
}

void ButiScript::Compiler::PushNameSpace(NameSpace_t arg_namespace)
{
	if (!arg_namespace) {
		arg_namespace = ButiEngine::make_value<NameSpace>("");
	}
	if (currentNameSpace) {
		arg_namespace->SetParent(currentNameSpace);
	}
	currentNameSpace = arg_namespace;
	list_namespaces.Add(arg_namespace);
}

void ButiScript::Compiler::PopNameSpace()
{
	if (currentNameSpace) {
		currentNameSpace = currentNameSpace->GetParent();
	}
}

void ButiScript::Compiler::DebugDump()
{
	std::cout << "---variables---" << std::endl;
	std::uint64_t vsize = variables.GetSize();
	std::cout << "value stack = " << vsize << std::endl;
	for (std::uint64_t i = 0; i < vsize; i++) {
		//variables[i].Dump();
	}
	std::cout << "---code---" << std::endl;

	static const char* op_name[] = {
#define	VM_NAMETABLE
#include "VM_nametable.h"
#include "../ButiScriptLib/Tags.h"
#undef	VM_NAMETABLE
		"LABEL",
	};

	std::int32_t	pos = 0;
	std::uint64_t size = statement.GetSize();
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

std::int32_t ButiScript::Compiler::InputCompiledData(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data)
{
	try
	{
		auto reader =ButiEngine::ObjectFactory::Create< ButiEngine::BinaryReader_File>();
		if (!reader->ReadStart(arg_filePath)) {
			return -1;
		}
		InputCompiledData(reader, arg_ref_data);

	}
	catch (const std::exception&)
	{
		return -1;
	}
	return 0;
}

std::int32_t ButiScript::Compiler::InputCompiledData(const char* arg_dataPtr, const std::int32_t arg_size, ButiScript::CompiledData& arg_ref_data)
{
	try
	{
		auto reader = ButiEngine::ObjectFactory::Create< ButiEngine::BinaryReader_Memory>(arg_dataPtr,arg_size);
		InputCompiledData(reader, arg_ref_data);

	}
	catch (const std::exception&)
	{
		return -1;
	}
	return 0;
}

std::int32_t ButiScript::Compiler::InputCompiledData(ButiEngine::Value_ptr<ButiEngine::IBinaryReader> reader, ButiScript::CompiledData& arg_ref_data)
{
	std::int32_t sourceFilePathStrSize = reader->ReadVariable<std::int32_t>();

	arg_ref_data.sourceFilePath = reader->ReadCharactor(sourceFilePathStrSize);
	arg_ref_data.commandSize = reader->ReadVariable<std::int32_t>();
	arg_ref_data.commandTable = reinterpret_cast<std::uint8_t*>(reader->ReadData(arg_ref_data.commandSize));

	arg_ref_data.textSize = reader->ReadVariable<std::int32_t>();
	arg_ref_data.textBuffer = reinterpret_cast<char*>(reader->ReadData(arg_ref_data.textSize));



	arg_ref_data.valueSize = reader->ReadVariable<std::int32_t>();

	std::int32_t sysCallSize = 0;
	sysCallSize = reader->ReadVariable<std::int32_t>();
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		std::int32_t index = reader->ReadVariable<std::int32_t>();
		SysFunction sysFunc = list_sysCalls[index];
		arg_ref_data.list_sysCalls.Add(sysFunc);
	}



	sysCallSize = reader->ReadVariable<std::int32_t>();
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		std::int32_t index = reader->ReadVariable<std::int32_t>();
		SysFunction sysFunc = list_sysMethodCalls[index];
		arg_ref_data.list_sysCallMethods.Add(sysFunc);
	}


	sysCallSize = reader->ReadVariable<std::int32_t>();
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		TypeTag typeTag;

		std::int32_t size = reader->ReadVariable<std::int32_t>();
		typeTag.argName = reader->ReadCharactor(size);

		size = reader->ReadVariable<std::int32_t>();
		typeTag.typeName = reader->ReadCharactor(size);

		std::int32_t index = reader->ReadVariable<std::int32_t>();
		SysFunction sysFunc = nullptr;
		if (index < list_valueAllocCall.GetSize() && index >= 0) {
			sysFunc = list_valueAllocCall[index];
		}
		typeTag.typeFunc = sysFunc;

		index = reader->ReadVariable<std::int32_t>();
		if (index < list_refValueAllocCall.GetSize() && index >= 0) {
			sysFunc = list_refValueAllocCall[index];
		}
		typeTag.refTypeFunc = sysFunc;



		typeTag.typeIndex = reader->ReadVariable<std::int32_t>();

		std::int32_t typeMapSize = reader->ReadVariable<std::int32_t>();

		for (std::int32_t j = 0; j < typeMapSize; j++) {
			std::int32_t size = reader->ReadVariable<std::int32_t>();
			std::string typeNameStr = reader->ReadCharactor(size);

			MemberValueInfo memberInfo = reader->ReadVariable<MemberValueInfo >();
			typeTag.map_memberValue.emplace(typeNameStr, memberInfo);
		}

		typeTag.methods.FileInput(reader);
		typeTag.isSystem = true;
		arg_ref_data.list_types.Add(typeTag);
	}


	std::int32_t entryPointsSize = reader->ReadVariable<std::int32_t>();

	for (std::int32_t count = 0; count < entryPointsSize; count++) {


		std::int32_t size = reader->ReadVariable<std::int32_t>();

		std::string name = reader->ReadCharactor(size);
		auto entryPoint = reader->ReadVariable<std::int32_t>();
		arg_ref_data.map_entryPoints.emplace(name, entryPoint);

	}
	for (auto entryPointItr = arg_ref_data.map_entryPoints.begin(), endItr = arg_ref_data.map_entryPoints.end(); entryPointItr != endItr; entryPointItr++) {

		arg_ref_data.map_functionJumpPointsTable.emplace(entryPointItr->second, &entryPointItr->first);
	}

	std::int32_t publicGlobalValue = reader->ReadVariable<std::int32_t>();
	for (std::int32_t count = 0; count < publicGlobalValue; count++) {
		std::int32_t strSize = reader->ReadVariable<std::int32_t>();
		auto str = reader->ReadCharactor(strSize);
		std::int32_t address = reader->ReadVariable<std::int32_t>();
		arg_ref_data.map_globalValueAddress.emplace(str, address);
		arg_ref_data.map_addressToValueName.emplace(address, str);
	}

	std::int32_t definedTypeCount = reader->ReadVariable<std::int32_t>();
	arg_ref_data.list_scriptClassInfo.Resize(definedTypeCount);
	for (std::int32_t index = 0; index < definedTypeCount; index++) {
		arg_ref_data.list_scriptClassInfo.At(index).InputFile(reader);
	}
	arg_ref_data.functionTypeCount = reader->ReadVariable<std::int32_t>();
	arg_ref_data.systemTypeCount = arg_ref_data.list_types.GetSize() - definedTypeCount - arg_ref_data.functionTypeCount;
	for (std::int32_t index = 0; index < definedTypeCount; index++) {
		arg_ref_data.list_scriptClassInfo.At(index).SetSystemTypeCount(arg_ref_data.systemTypeCount);
		arg_ref_data.list_types.At(arg_ref_data.list_scriptClassInfo.At(index).GetTypeIndex()).isSystem = false;
	}
	std::int32_t enumCount = reader->ReadVariable<std::int32_t>();
	for (std::int32_t index = 0; index < enumCount; index++) {
		EnumTag tag;
		std::int32_t typeIndex = reader->ReadVariable<std::int32_t>();
		tag.InputFile(reader);
		arg_ref_data.map_enumTag.emplace(typeIndex, tag);
		arg_ref_data.map_enumTag.at(typeIndex).typeIndex = typeIndex;
		arg_ref_data.map_enumTag.at(typeIndex).CreateEnumMap();
		arg_ref_data.list_types.At(typeIndex).p_enumTag = &arg_ref_data.map_enumTag.at(typeIndex);
	}

	arg_ref_data.functions.FileInput(reader);
	return 0;
}

void ButiScript::Compiler::Clear()
{
	labels.Clear();
	statement.Clear();
	text_table.Clear();
	variables.Clear();
	functions.Clear_notSystem();
	types.Clear_notSystem();
	enums.Clear_notSystem();
	errorCount = 0;
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

	std::int32_t sysCallSize = arg_ref_data.list_sysCalls.GetSize();
	fOut.write((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		 auto p_sysCallFunc=arg_ref_data.list_sysCalls[count];
		 std::int64_t address = *(std::int64_t*) & p_sysCallFunc;
		 std::int32_t index = map_sysCallsIndex.at(address);

		 fOut.write((char*)&index, sizeof(index));
	}

	sysCallSize = arg_ref_data.list_sysCallMethods.GetSize();
	fOut.write((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		auto p_sysCallFunc = arg_ref_data.list_sysCallMethods[count];
		std::int64_t address = *(std::int64_t*) & p_sysCallFunc;
		std::int32_t index = map_sysMethodCallsIndex.at(address);

		fOut.write((char*)&index, sizeof(index));
	}


	sysCallSize = arg_ref_data.list_types.GetSize();
	fOut.write((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		auto p_type =& arg_ref_data.list_types[count];

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

	std::int32_t defineTypeCount = arg_ref_data.list_scriptClassInfo.GetSize();
	fOut.write((char*)&defineTypeCount, sizeof(defineTypeCount));
	for (std::int32_t index = 0; index < defineTypeCount; index++) {
		arg_ref_data.list_scriptClassInfo[index].OutputFile(fOut);
	}

	fOut.write((char*)&arg_ref_data.functionTypeCount, sizeof(arg_ref_data.functionTypeCount));


	std::int32_t enumCount = arg_ref_data.map_enumTag.size();
	fOut.write((char*)&enumCount, sizeof(enumCount));
	for (auto& itr : arg_ref_data.map_enumTag) {
		fOut.write((char*)&itr.first,sizeof(std::int32_t));
		itr.second.OutputFile(fOut);

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
	if (vlp_parentNamespace) {
		return vlp_parentNamespace->GetGlobalNameString() + name+ "::";
	}

	if (name.size()==0) {
		return "";
	}

	return name+"::";
}

void ButiScript::NameSpace::Regist(Compiler* arg_compiler)
{
	arg_compiler->PushNameSpace(ButiEngine::make_value<NameSpace>(name));
}

void ButiScript::NameSpace::SetParent(NameSpace_t arg_parent)
{
	if (arg_parent->GetParent().get() != this && arg_parent.get() != this) {
		vlp_parentNamespace = arg_parent;
	}
	else {
		assert(0);
	}
}

ButiScript::NameSpace_t ButiScript::NameSpace::GetParent() const
{
	return vlp_parentNamespace;
}

void ButiScript::NameSpace::PushValue(Declaration_t arg_value)
{
	list_analyzeGlobalValueBuffer.Add(arg_value);
}

void ButiScript::NameSpace::PushFunction(Function_t arg_func)
{
	list_analyzeFunctionBuffer.Add(arg_func);
}

void ButiScript::NameSpace::PushClass(Class_t arg_class)
{
	list_analyzeClassBuffer.Add(arg_class);
}



void ButiScript::NameSpace::AnalyzeFunctions(Compiler* arg_compiler)
{
	for (auto itr : list_analyzeFunctionBuffer) {
		itr->Analyze(arg_compiler, nullptr);
	}
}

void ButiScript::NameSpace::AnalyzeClasses(Compiler* arg_compiler)
{
	for (auto itr :list_analyzeClassBuffer) {
		itr->Analyze(arg_compiler);
	}
}

void ButiScript::NameSpace::DeclareValues(Compiler* arg_compiler)
{
	for (auto itr : list_analyzeGlobalValueBuffer) {
		itr->Analyze(arg_compiler);
	}
}

void ButiScript::NameSpace::DeclareFunctions(Compiler* arg_compiler)
{
	for (auto itr : list_analyzeFunctionBuffer) {
		itr->PushCompiler(arg_compiler, nullptr);
	}
}

void ButiScript::NameSpace::DeclareClasses(Compiler* arg_compiler)
{
	for (auto itr : list_analyzeClassBuffer) {
		itr->Regist(arg_compiler);
	}
}

void ButiScript::NameSpace::Clear()
{
	for (auto analyzeClass : list_analyzeClassBuffer) {
		analyzeClass->Release();
	}
	for (auto analyzeFunction: list_analyzeFunctionBuffer) {
		analyzeFunction->Release();
	}
	list_analyzeClassBuffer.Clear();
	list_analyzeFunctionBuffer.Clear();
	list_analyzeGlobalValueBuffer.Clear();
}


void ButiScript::ValueTable::Alloc(Compiler* arg_comp) const
{
	auto end = list_variableTypes.end();
	for (auto itr = list_variableTypes.begin(); itr != end; itr++)
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
	RegistSystemType<std::int32_t>("int", "i");
	RegistSystemType<float>("float", "f");
	RegistSystemType<std::string>("string", "s");
	RegistSystemType<Type_Null>("void", "v");
	RegistSystemType<ButiEngine::Vector2,Type_hasMember<ButiEngine::Vector2>>("Vector2", "vec2", "x:f,y:f");
	RegistSystemType<ButiEngine::Vector3,Type_hasMember<ButiEngine::Vector3>>("Vector3", "vec3", "x:f,y:f,z:f");
	RegistSystemType<ButiEngine::Vector4,Type_hasMember<ButiEngine::Vector4>>("Vector4", "vec4", "x:f,y:f,z:f,w:f");
	RegistSystemType<ButiEngine::Matrix4x4,Type_hasMember<ButiEngine::Matrix4x4>>("Matrix4x4", "Matrix4x4", "_m11:f,_m12:f,_m13:f,_m14:f,_m21:f,_m22:f,_m23:f,_m24:f,_m31:f,_m32:f,_m33:f,_m34:f,_m41:f,_m42:f,_m43:f,_m44:f");
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

	DefineSystemFunction(&VirtualMachine::sys_registEventListner, TYPE_STRING, "RegistEvent", "s,s,s");
	DefineSystemFunction(&VirtualMachine::sys_unregistEventListner, TYPE_VOID, "UnRegistEvent", "s,s");
	DefineSystemFunction(&VirtualMachine::sys_addEventMessanger, TYPE_VOID, "AddEventMessanger", "s");
	DefineSystemFunction(&VirtualMachine::sys_removeEventMessanger, TYPE_VOID, "RemoveEventMessanger", "s");
	DefineSystemFunction(&VirtualMachine::sys_executeEvent, TYPE_VOID, "EventExecute", "s");
	DefineSystemFunction(&VirtualMachine::sys_pushTask, TYPE_VOID, "PushTask", "s");

	DefineSystemMethod(&VirtualMachine::sys_method_cast< &std::string::size, std::int32_t>, TYPE_STRING, TYPE_INTEGER, "Size", "");

	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::Normalize >, TYPE_VOID + 1, TYPE_VOID, "Normalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetNormalize  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetNormalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetLength >, TYPE_VOID + 1, TYPE_FLOAT, "GetLength", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetLengthSqr >, TYPE_VOID + 1, TYPE_FLOAT, "GetLengthSqr", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::Dot, &VirtualMachine::GetPtr  >, TYPE_VOID + 1, TYPE_FLOAT, "Dot", "vec2");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::Floor, &VirtualMachine::GetPtr  >, TYPE_VOID + 1, (TYPE_VOID + 1), "Floor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::Round, &VirtualMachine::GetPtr  >, TYPE_VOID + 1, (TYPE_VOID + 1), "Round", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::Ceil, &VirtualMachine::GetPtr >, TYPE_VOID + 1, (TYPE_VOID + 1), "Ceil", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetFloor, &VirtualMachine::GetPtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetFloor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetRound, &VirtualMachine::GetPtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetRound", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetCeil, &VirtualMachine::GetPtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetCeil", "i");

	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::Normalize>, TYPE_VOID + 2, TYPE_VOID, "Normalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetNormalize>, TYPE_VOID + 2, TYPE_VOID + 2, "GetNormalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetLength>, TYPE_VOID + 2, TYPE_FLOAT, "GetLength", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetLengthSqr>, TYPE_VOID + 2, TYPE_FLOAT, "GetLengthSqr", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::Dot,&VirtualMachine::GetPtr >, TYPE_VOID + 2, TYPE_FLOAT, "Dot", "vec3");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetCross, &VirtualMachine::GetPtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetCross", "vec3");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::Floor,&VirtualMachine::GetPtr  >, TYPE_VOID + 2, (TYPE_VOID + 2), "Floor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::Round,  &VirtualMachine::GetPtr  >, TYPE_VOID + 2, (TYPE_VOID + 2), "Round", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::Ceil, &VirtualMachine::GetPtr >, TYPE_VOID + 2, (TYPE_VOID + 2), "Ceil", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetFloor, &VirtualMachine::GetPtr  >, TYPE_VOID + 2, TYPE_VOID + 2, "GetFloor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetRound, &VirtualMachine::GetPtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetRound", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetCeil, &VirtualMachine::GetPtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetCeil", "i");

	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::Transpose>, TYPE_VOID + 4, TYPE_VOID + 4, "Transpose", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetTranspose>, TYPE_VOID + 4, TYPE_VOID + 4, "GetTranspose", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::Inverse>, TYPE_VOID + 4, TYPE_VOID + 4, "Inverse", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetInverse>, TYPE_VOID + 4, TYPE_VOID + 4, "GetInverse", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::Identity>, TYPE_VOID + 4, TYPE_VOID + 4, "Identity", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetEulerOneValue>, TYPE_VOID + 4, TYPE_VOID + 2, "GetEulerOneValue", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetPosition>, TYPE_VOID + 4, TYPE_VOID + 2, "GetPosition", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetPosition_Transpose>, TYPE_VOID + 4, TYPE_VOID + 2, "GetPosition_Transpose", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetEulerOneValue_local>, TYPE_VOID + 4, TYPE_VOID + 2, "GetEulerOneValue_local", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::CreateFromEuler, &VirtualMachine::GetPtr >, TYPE_VOID + 4, TYPE_VOID + 4, "CreateFromEuler", "vec3");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::CreateFromEuler_local,&VirtualMachine::GetPtr >, TYPE_VOID + 4, TYPE_VOID + 4, "CreateFromEuler_local", "vec3");
	
}

bool ButiScript::SystemFuntionRegister::DefineSystemFunction(SysFunction arg_op, const std::int32_t arg_retType, const std::string& arg_name, const std::string& arg_list_argDefines, const ButiEngine::List<std::int32_t>& arg_list_templateTypes)
{
	FunctionTag func(arg_retType, arg_name);
	if (!func.SetArgs(arg_list_argDefines, SystemTypeRegister::GetInstance()->types .GetArgmentKeyMap()))
		return false;

	func.SetDeclaration();
	func.SetSystem();				// Systemフラグセット
	std::int32_t index = list_sysCalls.GetSize();
	func.SetIndex(index);			// 組み込み関数番号を設定
	func.SetTemplateType(arg_list_templateTypes);
	std::int64_t address = *(std::int64_t*) & arg_op;
	map_sysCallsIndex.emplace(address, list_sysCalls.GetSize());
	list_sysCalls.Add(arg_op);
	if (functions.Add(arg_name, func, &SystemTypeRegister::GetInstance()->types) == 0) {
		return false;
	}
	return true;
}

bool ButiScript::SystemFuntionRegister::DefineSystemMethod(SysFunction arg_p_method, const std::int32_t arg_type, const std::int32_t arg_retType, const std::string& arg_name, const std::string& arg_args, const ButiEngine::List<std::int32_t>& arg_list_templateTypes)
{
	FunctionTag func(arg_retType, arg_name);
	if (!func.SetArgs(arg_args, SystemTypeRegister::GetInstance()->types.GetArgmentKeyMap()))
		return false;

	func.SetDeclaration();
	func.SetSystem();				// Systemフラグセット
	func.SetIndex(list_sysMethodCalls.GetSize());			// 組み込み関数番号を設定
	func.SetTemplateType(arg_list_templateTypes);
	std::int64_t address = *(std::int64_t*) & arg_p_method;
	map_sysMethodCallsIndex.emplace(address, list_sysMethodCalls.GetSize());
	list_sysMethodCalls.Add(arg_p_method);
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
	return p_functionObjectData->list_argTypes.GetSize();
}

const ButiEngine::List<std::int32_t>& ButiScript::TypeTag::GetFunctionObjectArgment() const
{

	return p_functionObjectData->list_argTypes;
}

void ButiScript::TypeTable::Release() {
	for (auto& type: map_types) {
		type.second.Release();
	}
	map_types.clear();
	map_argmentChars.clear();
	list_types.Clear();
}

void ButiScript::ArgDefine::SpecficType(const Compiler* arg_compiler)
{
	valueType = arg_compiler->GetTypeIndex(StringHelper::Remove( valueTypeName,"&"));
	if (StringHelper::Contains(valueTypeName, "&")) {
		SetRef();
	}
}

auto compilerRelease = ButiEngine::Util::MemoryReleaser(&p_instance);
auto sysTypeRelease = ButiEngine::Util::MemoryReleaser(&p_sysreginstance);
auto sysFuncRelease = ButiEngine::Util::MemoryReleaser(&p_sysfuncregister);