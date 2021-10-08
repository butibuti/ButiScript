#include "stdafx.h"
#include "Compiler.h"
#include"BuiltInTypeRegister.h"
#include"VirtualMachine.h"
#include <iomanip>
#include "Parser.h"
#include<direct.h>
auto baseFunc = &ButiScript::VirtualCPU::Initialize;
const char* thisPtrName = "this";

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
	currentNameSpace = std::make_shared<NameSpace>("");
	

}

// コンパイル
bool ButiScript::Compiler::Compile(const std::string& file, ButiScript::CompiledData& Data)
{

	//変数テーブルをセット
	variables.push_back(ValueTable());
	variables[0].set_global();
	OpHalt();
	bool result = ScriptParser(file, this);	// 構文解析

	if (!result)
		return false;// パーサーエラー

	int code_size = LabelSetting();				// ラベルにアドレスを設定
	CreateData(Data, code_size);				// バイナリ生成

	Data.sourceFilePath = file;

	labels.clear();
	statement.clear();
	text_table.clear();
	variables.clear();
	functions.Clear_notSystem();
	types.Clear_notSystem();
	enums.Clear_notSystem();

	return error_count == 0;
}


// エラーメッセージを出力
void ButiScript::Compiler::error(const std::string& m)
{
	std::cerr << m << std::endl;
	error_count++;
}

void ButiScript::Compiler::ClearStatement()
{
	statement.clear();
}

std::string ButiScript::Compiler::GetTypeName(const int arg_type) const
{
	int type = arg_type & ~TYPE_REF;
	std::string output = "";
	
	output = types.GetType(type)->typeName;

	if (arg_type&TYPE_REF) {
		output += "&";
	}

	return output;
}

void ButiScript::Compiler::RamdaCountReset()
{
	ramdaCount = 0;
}



// 外部変数の定義
struct Define_value {
	ButiScript::Compiler* comp_;
	int valueType;
	ButiScript::AccessModifier access;
	Define_value(ButiScript::Compiler* comp, const int type,ButiScript::AccessModifier arg_access ) : comp_(comp), valueType(type),access(arg_access)
	{
	}

	void operator()(ButiScript::Node_t node) const
	{
		comp_->AddValue(valueType, node->GetString(), node->GetLeft(),access);
	}
};

void ButiScript::Compiler::AnalyzeScriptType(const std::string& arg_typeName, const std::map < std::string, std::pair< int, AccessModifier>>& arg_memberInfo)
{
	auto typeTag = GetType(arg_typeName);
	if (arg_memberInfo.size()) {
		auto memberInfoEnd = arg_memberInfo.end();
		int i = 0;
		for (auto itr = arg_memberInfo.begin(); itr != memberInfoEnd; i++,itr++) {
			if (typeTag->typeIndex == itr->second.first) {
				error("クラス "+itr->first + "が自身と同じ型をメンバ変数として保持しています。");
			}
			MemberValueInfo info = { i ,itr->second.first,itr->second.second };
			typeTag->map_memberValue.emplace(itr->first, info);

		}
	}
}

void ButiScript::Compiler::RegistScriptType(const std::string& arg_typeName)
{

	TypeTag type;
	int typeIndex = types.GetSize();
	type.isSystem = false;


	type.typeName = arg_typeName;
	type.typeIndex = typeIndex;
	type.argName = arg_typeName;


	types.RegistType(type);
}

void ButiScript::Compiler::ValueDefine(int type, const std::vector<Node_t>& node, const AccessModifier arg_access)
{
	std::for_each(node.begin(), node.end(), Define_value(this, type,arg_access));
}

// 関数宣言
void ButiScript::Compiler::FunctionDefine(int type, const std::string& name, const std::vector<int>& args)
{
	const FunctionTag* tag = functions.Find_strict(name,args);
	if (tag) {			// 既に宣言済み
		if (!tag->CheckArgList_strict(args)) {
			error("関数 " + name + " に異なる型の引数が指定されています");
			return;
		}
	}
	else {
		FunctionTag func(type, name);
		func.SetArgs(args);				// 引数を設定
		func.SetDeclaration();			// 宣言済み
		func.SetIndex(MakeLabel());		// ラベル登録
		if (functions.Add(name, func) == 0) {
			error("内部エラー：関数テーブルに登録できません");
		}
	}
}

// 関数定義
//
//	関数が呼ばれた時点のスタック
//
//	+--------------+
//	|     arg2     | -5
//	+--------------+
//	|     arg1     | -4
//	+--------------+
//	|  arg count   | -3
//	+--------------+
//	| base_pointer | -2
//	+--------------+
//	| return addr  | -1
//	+--------------+
//
//	

// 引数の変数名を登録
struct add_value {
	ButiScript::Compiler* comp_;
	ButiScript::ValueTable& values_;
	int addr_;
	add_value(ButiScript::Compiler* comp, ButiScript::ValueTable& values,const int arg_addres=-4) : comp_(comp), values_(values), addr_(arg_addres)
	{
	}

	void operator()(const ButiScript::ArgDefine& arg) const
	{
		if (!values_.add_arg(arg.type(), arg.name(), addr_)) {
			comp_->error("引数 " + arg.name() + " は既に登録されています。");
		}
	}
};

void ButiScript::Compiler::AddFunction(int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block,const AccessModifier access, FunctionTable* arg_funcTable )
{

	std::string functionName = currentNameSpace->GetGlobalNameString()+ name;
	FunctionTable* p_functable = arg_funcTable ? arg_funcTable : &functions;


	FunctionTag* tag = p_functable->Find_strict(functionName,args);
	if (tag) {
		if (tag->IsDefinition()) {
			error("関数 " + functionName + " は既に定義されています");
			return;
		}
		if (tag->IsDeclaration() && !tag->CheckArgList_strict(args)) {
			error("関数 " + functionName + " に異なる型の引数が指定されています");
			return;
		}
		tag->SetDefinition();	// 定義済みに設定
	}
	else {
		FunctionTag func(type,functionName);
		func.SetArgs(args);				// 引数を設定
		func.SetDefinition();			// 定義済み
		func.SetIndex(MakeLabel());		// ラベル登録
		tag = p_functable->Add(functionName, func);
		if (tag == nullptr)
			error("内部エラー：関数テーブルに登録できません");
	}

	current_function_name = functionName;		// 処理中の関数名を登録
	current_function_type = type;		// 処理中の関数型を登録

	// 関数のエントリーポイントにラベルを置く

	SetLabel(tag->GetIndex());

	BlockIn();		// 変数スタックを増やす

	// 引数リストを登録
	int address = -4;
	//メンバ関数の場合thisを引数に追加
	if (arg_funcTable) {
		ArgDefine argDef(GetCurrentThisType()->typeIndex, thisPtrName);
		add_value(this, variables.back(), address)(argDef);
		address--;
	}
	auto endItr = args.rend();
	for (auto itr = args.rbegin(); itr != endItr; itr++) {
		add_value(this, variables.back(),address)(*itr);
		address--;
	}


	// 文があれば、文を登録
	if (block) {
		int ret=block->Analyze(this);
	}

	const VMCode& code = statement.back();
	if (type == TYPE_VOID) {			
		if (code.op_ != VM_RETURN)		// returnが無ければreturnを追加
			OpReturn();					
	}
	else {
		if (code.op_ != VM_RETURNV) {	
			error("関数 " + functionName + " の最後にreturn文が有りません。");
		}
	}

	BlockOut();		// 変数スタックを減らす

	current_function_name.clear();		// 処理中の関数名を消去

}

void ButiScript::Compiler::AddRamda(const int type, const std::vector<ArgDefine>& args, Block_t block, FunctionTable* arg_funcTable)
{
	AddFunction(type, "@ramda:" + std::to_string(ramdaCount), args, block, AccessModifier::Public, arg_funcTable);

	ramdaCount++;
}

void ButiScript::Compiler::RegistFunction(const int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block, const AccessModifier access,FunctionTable* arg_funcTable )
{
	std::string functionName = currentNameSpace->GetGlobalNameString() + name;
	if (!arg_funcTable) {
		arg_funcTable = &functions;
	}

	const FunctionTag* tag = arg_funcTable->Find_strict(functionName,args);
	if (tag) {			// 既に宣言済み
		if (!tag->CheckArgList_strict(args)) {
			error("関数 " + functionName + " に異なる型の引数が指定されています");
			return;
		}
	}
	else {
		FunctionTag func(type, functionName);
		func.SetArgs(args);				// 引数を設定
		func.SetDeclaration();			// 宣言済み
		func.SetIndex(MakeLabel());		// ラベル登録
		func.SetAccessType(access);
		if (arg_funcTable->Add(functionName, func) == 0) {
			error("内部エラー：関数テーブルに登録できません");
		}
	}
}

void ButiScript::Compiler::RegistRamda(const int type, const std::vector<ArgDefine>& args, FunctionTable* arg_functionTable)
{
	RegistFunction(type, "@ramda:" + std::to_string(ramdaCount), args, nullptr, AccessModifier::Public, arg_functionTable);

	ramdaCount++;
}

void ButiScript::Compiler::RegistEnum(const std::string& arg_typeName, const std::string& identiferName, const int value)
{
	auto enumType = GetEnumTag(arg_typeName);
	if (!enumType) {
		RegistEnumType(arg_typeName);
		enumType = enums.FindType(arg_typeName);
	}

	enumType->SetValue(identiferName, value);
}

void ButiScript::Compiler::RegistEnumType(const std::string& arg_typeName)
{
	auto typeName = currentNameSpace->GetGlobalNameString()+arg_typeName;
	EnumTag tag(typeName);
	enums.SetEnum(tag);
	types.RegistType(*enums.FindType(arg_typeName));
}

int ButiScript::Compiler::GetfunctionTypeIndex(const std::vector<ArgDefine>& arg_vec_argmentTypes, const int retType)
{
	std::vector<int> vec_argTypes;
	std::for_each(arg_vec_argmentTypes.begin(), arg_vec_argmentTypes.end(), [&](const ArgDefine& itr)->void {vec_argTypes.push_back(itr.type()); });
	
	return GetfunctionTypeIndex(vec_argTypes,retType);
}
int ButiScript::Compiler::GetfunctionTypeIndex(const std::vector<int>& arg_vec_argmentTypes, const int retType)
{
	auto type = types.GetFunctionType(arg_vec_argmentTypes, retType);
	if (!type) {
		type = types.CreateFunctionType(arg_vec_argmentTypes, retType);
	}
	return type->typeIndex;
}


// 変数の登録
void ButiScript::Compiler::AddValue(const int type, const std::string& name, Node_t node ,const AccessModifier access)
{
	std::string valueName = GetCurrentNameSpace()->GetGlobalNameString() + name;
	int size = 1;
	if (node) {
		if (node->Op() != OP_INT) {
			error("配列のサイズは定数で指定してください。");
		}
		else if (node->GetNumber() <= 0) {
			error("配列のサイズは１以上の定数が必要です。");
		}
		size = node->GetNumber();
	}

	ValueTable& values = variables.back();
	if (!values.Add(type, valueName, access,size)) {
		error("変数 " + valueName + " は既に登録されています。");
	}
}

// ラベル生成
int ButiScript::Compiler::MakeLabel()
{
	int index = (int)labels.size();
	labels.push_back(Label(index));
	return index;
}

// ラベルのダミーコマンドをステートメントリストに登録する
void ButiScript::Compiler::SetLabel(int label)
{
	statement.push_back(VMCode(VM_MAXCOMMAND, label));
}

// 文字列定数をpush
void ButiScript::Compiler::PushString(const std::string& str)
{
	PushString(((int)text_table.size()));
	text_table.insert(text_table.end(), str.begin(), str.end());
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

// ブロック内では、新しい変数セットに変数を登録する

void ButiScript::Compiler::BlockIn()
{
	int start_addr = 0;					// 変数アドレスの開始位置
	if (variables.size() >= 2) {			// ブロックの入れ子は、開始アドレスを続きからにする。最初のvariablesTableはグローバル変数用なので考慮しない
		start_addr = variables.back().size();
	}
	variables.push_back(ValueTable(start_addr));
}

// ブロックの終了で、変数スコープが消える（変数セットを削除する）

void ButiScript::Compiler::BlockOut()
{
	variables.pop_back();
}

// ローカル変数用にスタックを確保

void ButiScript::Compiler::AllocStack()
{
	variables.back().Alloc(this);
}

// ラベル解決
//
// １．アドレスを生成する
// ２．ダミーのラベルコマンドが有ったアドレスを、ラベルテーブルに登録する
// ３．Jmpコマンドの飛び先をラベルテーブルに登録されたアドレスにする

// アドレス計算
struct calc_addr {
	std::vector<ButiScript::Label>& labels_;
	int& pos_;
	calc_addr(std::vector<ButiScript::Label>& labels, int& pos) : labels_(labels), pos_(pos)
	{
	}
	void operator()(const ButiScript::VMCode& code)
	{
		if (code.op_ == ButiScript::VM_MAXCOMMAND) {			// ラベルのダミーコマンド
			labels_[code.GetConstValue<int>()].pos_ = pos_;
		}
		else {
			pos_ += code.size_;
		}
	}
};

// ジャンプアドレス設定
struct set_addr {
	std::vector<ButiScript::Label>& labels_;
	set_addr(std::vector<ButiScript::Label>& labels) : labels_(labels)
	{
	}
	void operator()(ButiScript::VMCode& code)
	{
		switch (code.op_) {
		case ButiScript:: VM_JMP:
		case ButiScript::VM_JMPC:
		case ButiScript::VM_JMPNC:
		case ButiScript::VM_TEST:
		case ButiScript::VM_CALL:
		case ButiScript::VM_PUSHFUNCTIONOBJECT:
			code.SetConstValue( labels_[code.GetConstValue<int>()].pos_);
			break;
		}
	}
};

int ButiScript::Compiler::LabelSetting()
{
	// アドレス計算
	int pos = 0;
	std::for_each(statement.begin(), statement.end(), calc_addr(labels, pos));
	// ジャンプアドレス設定
	std::for_each(statement.begin(), statement.end(), set_addr(labels));

	return pos;
}

// バイナリデータ生成

struct copy_code {
	unsigned char* p;
	copy_code(unsigned char* code) : p(code)
	{
	}
	void operator()(const ButiScript::VMCode& code)
	{
		p = code.Get(p);
	}
};

bool ButiScript::Compiler::CreateData(ButiScript::CompiledData& Data, int code_size)
{

	for (int i = 0; i < functions.Size(); i++) {
		auto func = functions[i];
		if (func->IsSystem()) {
			continue;
		}
		Data.map_entryPoints.emplace(functions[i]->GetNameWithArgment(types), labels[functions[i]->index_].pos_);
	}
	
	for (auto itr = Data.map_entryPoints.begin(), end = Data.map_entryPoints.end(); itr != end;itr++) {

		const std::string* p_str = &itr->first;
		Data.map_functionJumpPointsTable.emplace(itr->second, p_str);
	}

	Data.functions = functions;

	Data.commandTable = new unsigned char[code_size];
	Data.textBuffer = new char[text_table.size()];
	Data.commandSize = code_size;
	Data.textSize = (int)text_table.size();
	Data.valueSize = (int)variables[0].size();

	Data.vec_sysCalls = vec_sysCalls;
	Data.vec_sysCallMethods = vec_sysMethodCalls;
	types.CreateTypeVec(Data.vec_types);
	Data.vec_scriptClassInfo = types.GetScriptClassInfo();

	for (int i = 0; i < Data.valueSize; i++) {
		auto p_value = &variables[0][i];
		if (p_value->access == AccessModifier::Public) {
			Data.map_globalValueAddress.emplace(variables[0].GetVariableName(i), p_value->address);
		}
	}
	for (int i = 0; i < enums.Size();i++) {
		Data.map_enumTag.emplace(enums[i]->typeIndex, *enums[i]);
	}
	if (Data.textSize != 0) {
		memcpy(Data.textBuffer, &text_table[0], Data.textSize);
	}

	std::for_each(statement.begin(), statement.end(), copy_code(Data.commandTable));

	auto end = statement.end();
	for (auto itr = statement.begin(); itr != end; itr++) {
		itr->Release();
	}
	return true;
}

void ButiScript::Compiler::PushNameSpace(NameSpace_t arg_namespace)
{
	if (currentNameSpace) {
		arg_namespace->SetParent(currentNameSpace);
	}
	currentNameSpace = arg_namespace;
}

void ButiScript::Compiler::PopNameSpace()
{
	if (currentNameSpace) {
		currentNameSpace = currentNameSpace->GetParent();
	}
}

// デバッグダンプ
#ifdef	_DEBUG

void ButiScript::Compiler::debug_dump()

#ifdef BUTIGUI_H

// デバッグダンプ
{
	std::string message = "---variables---\n";
	size_t vsize = variables.size();
	message += "value stack = " + std::to_string(vsize) + '\n';
	for (size_t i = 0; i < vsize; i++) {
		variables[i].dump();
	}
	message += "---code---" + '\n';

	static const char* op_name[] = {
#define	VM_NAMETABLE
#include "VM_nametable.h"
#undef	VM_NAMETABLE
		"LABEL",
	};

	int	pos = 0;
	size_t size = statement.size();
	for (size_t i = 0; i < size; i++) {
		message += std::to_string(std::setw(6)) + std::to_string(pos) + ": " + op_name[statement[i].op_];
		if (statement[i].size_ > 1) {
			message += ", " + std::to_string(statement[i].GetConstValue<int>());
		}
		message += '\n';

		if (statement[i].op_ != VM_MAXCOMMAND) {
			pos += statement[i].size_;
		}
	}
	ButiEngine::GUI::Console(message);
}
#else

{
	std::cout << "---variables---" << std::endl;
	size_t vsize = variables.size();
	std::cout << "value stack = " << vsize << std::endl;
	for (size_t i = 0; i < vsize; i++) {
		variables[i].dump();
	}
	std::cout << "---code---" << std::endl;

	static const char* op_name[] = {
#define	VM_NAMETABLE
#include "VM_nametable.h"
#include "Tags.h"
#undef	VM_NAMETABLE
		"LABEL",
	};

	int	pos = 0;
	size_t size = statement.size();
	for (size_t i = 0; i < size; i++) {
		std::cout << std::setw(6) << pos << ": " << op_name[statement[i].op_];
		if (statement[i].size_ > 1) {
			std::cout << ", " << statement[i].GetConstValue<int>();
		}
		std::cout << std::endl;

		if (statement[i].op_ != VM_MAXCOMMAND) {
			pos += statement[i].size_;
		}
	}
}
#endif // BUTIGUI_H
#endif // _DEBUG


int ButiScript::Compiler::InputCompiledData(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data)
{
	std::ifstream fIn;
	fIn.open(arg_filePath,std::ios::binary);



	int sourceFilePathStrSize = 0;
	fIn.read((char*)&sourceFilePathStrSize, sizeof(sourceFilePathStrSize));
	char* sourceFilePathStrBuff = (char*)malloc(sourceFilePathStrSize);
	fIn.read(sourceFilePathStrBuff, sourceFilePathStrSize);
	arg_ref_data.sourceFilePath = std::string(sourceFilePathStrBuff, sourceFilePathStrSize);
	free(sourceFilePathStrBuff);


	fIn.read((char*)&arg_ref_data.commandSize, 4);
	arg_ref_data.commandTable = (unsigned char*)malloc(arg_ref_data.commandSize);
	fIn.read((char*)arg_ref_data.commandTable, arg_ref_data.commandSize);

	fIn.read((char*)&arg_ref_data.textSize, 4);
	arg_ref_data.textBuffer = (char*)malloc(arg_ref_data.textSize);
	fIn.read((char*)arg_ref_data.textBuffer, arg_ref_data.textSize);


	fIn.read((char*)&arg_ref_data.valueSize, 4);

	int sysCallSize=0;
	fIn.read((char*)&sysCallSize, 4);
	for (int i = 0; i < sysCallSize; i++) {
		int index=0;
		fIn.read((char*)&index, sizeof(index));
		SysFunction sysFunc = vec_sysCalls[index];
		arg_ref_data.vec_sysCalls.push_back(sysFunc);
	}



	fIn.read((char*)&sysCallSize, 4);
	for (int i = 0; i < sysCallSize; i++) {
		int index = 0;
		fIn.read((char*)&index, sizeof(index));
		SysFunction sysFunc = vec_sysMethodCalls[index];
		arg_ref_data.vec_sysCallMethods.push_back(sysFunc);
	}


	fIn.read((char*)&sysCallSize, 4);
	for (int i = 0; i < sysCallSize; i++) {
		TypeTag typeTag;

		int size=0;
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

		int index = 0;
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




		fIn.read((char*)&typeTag.typeIndex, sizeof(int));

		int typeMapSize = 0;
		fIn.read((char*)&typeMapSize, sizeof(typeMapSize));

		for (int j=0; j < typeMapSize;j++) {
			int size =0;
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

		arg_ref_data.vec_types.push_back(typeTag);
	}


	int entryPointsSize = 0;
	fIn.read((char*)&entryPointsSize, sizeof(entryPointsSize));
	
	for (int i = 0; i < entryPointsSize;i++) {
		

		int size =0, entryPoint = 0;

		fIn.read((char*)&size, sizeof(size));
		char* buff = (char*)malloc(size);
		fIn.read(buff, size);
		std::string name = std::string(buff, size);
		free(buff); 
		fIn.read((char*)&entryPoint, sizeof(int));
		int current = fIn.tellg();
		arg_ref_data.map_entryPoints.emplace(name, entryPoint);

	}
	for (auto entryPointItr = arg_ref_data.map_entryPoints.begin(), endItr = arg_ref_data.map_entryPoints.end(); entryPointItr != endItr; entryPointItr++) {

		arg_ref_data.map_functionJumpPointsTable.emplace(entryPointItr->second, &entryPointItr->first);
	}

	int publicGlobalValue = 0;
	fIn.read((char*)&publicGlobalValue, sizeof(publicGlobalValue));
	for (int i = 0; i < publicGlobalValue; i++) {
		int strSize = 0;
		fIn.read((char*)&strSize, sizeof(int));
		char* strBuff = (char*)malloc(strSize);
		fIn.read(strBuff, strSize);
		int address = 0;
		fIn.read((char*)&address, sizeof(int));
		arg_ref_data.map_globalValueAddress.emplace(std::string(strBuff, strSize), address);
		free(strBuff);
	}

	int definedTypeCount = 0;
	fIn.read((char*)&definedTypeCount, sizeof(definedTypeCount));
	arg_ref_data.vec_scriptClassInfo.resize(definedTypeCount);
	for (int i = 0; i < definedTypeCount; i++) {
		arg_ref_data.vec_scriptClassInfo.at(i).InputFile(fIn);
	}
	arg_ref_data.systemTypeCount = arg_ref_data.vec_types.size() - definedTypeCount;
	int enumCount =0;
	fIn.read((char*)&enumCount, sizeof(enumCount));
	for (int i = 0; i < enumCount;i++) {
		EnumTag tag;
		int typeIndex = 0;
		fIn.read((char*)&typeIndex, sizeof(int));
		tag.InputFile(fIn);
		arg_ref_data.map_enumTag.emplace(typeIndex,tag);
		arg_ref_data.map_enumTag.at(typeIndex).CreateEnumMap();
	}

	arg_ref_data.functions.FileInput(fIn);

	fIn.close();

	return 0;
}

int ButiScript::Compiler::OutputCompiledData(const std::string& arg_filePath, const ButiScript::CompiledData& arg_ref_data)
{

	auto dirPath = StringHelper::GetDirectory(arg_filePath);
	if (dirPath != arg_filePath) {
		auto dirRes = _mkdir(dirPath.c_str());
	}
	std::ofstream fOut(arg_filePath,  std::ios::binary);
	int sourcePathSize = arg_ref_data.sourceFilePath.size();
	fOut.write((char*)&sourcePathSize,sizeof(sourcePathSize));
	fOut.write(arg_ref_data.sourceFilePath.c_str(),sourcePathSize);

	fOut.write((char*)&arg_ref_data.commandSize, 4);
	fOut.write((char*)arg_ref_data.commandTable, arg_ref_data.commandSize);

	fOut.write((char*)&arg_ref_data.textSize, 4);
	fOut.write((char*)arg_ref_data.textBuffer,  arg_ref_data.textSize);


	fOut.write((char*)&arg_ref_data.valueSize, 4);

	int sysCallSize = arg_ref_data.vec_sysCalls.size();
	fOut.write((char*)&sysCallSize, 4);
	for (int i = 0; i < sysCallSize; i++) {
		 auto p_sysCallFunc=arg_ref_data.vec_sysCalls[i];
		 long long int address = *(long long int*) & p_sysCallFunc;
		 int index = map_sysCallsIndex.at(address);

		 fOut.write((char*)&index, sizeof(index));
	}

	sysCallSize = arg_ref_data.vec_sysCallMethods.size();
	fOut.write((char*)&sysCallSize, 4);
	for (int i = 0; i < sysCallSize; i++) {
		auto p_sysCallFunc = arg_ref_data.vec_sysCallMethods[i];
		long long int address = *(long long int*) & p_sysCallFunc;
		int index = map_sysMethodCallsIndex.at(address);

		fOut.write((char*)&index, sizeof(index));
	}


	sysCallSize = arg_ref_data.vec_types.size();
	fOut.write((char*)&sysCallSize, 4);
	for (int i = 0; i < sysCallSize; i++) {
		auto p_type =& arg_ref_data.vec_types[i];

		int size = p_type->argName.size();
		fOut.write((char*)&size, sizeof(size));
		fOut.write(p_type->argName.c_str(), size); 

		size = p_type->typeName.size();
		fOut.write((char*)&size, sizeof(size));
		fOut.write(p_type->typeName.c_str(),size);

		auto p_sysCallFunc = p_type->typeFunc;

		int index = -1;
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

		int typeMapSize = p_type->map_memberValue.size();
		fOut.write((char*)&typeMapSize, sizeof(typeMapSize));
		auto end = p_type->map_memberValue.end();
		for (auto itr = p_type->map_memberValue.begin(); itr != end; itr++) {
			int size = itr->first.size();
			fOut.write((char*)&size, sizeof(size));
			fOut.write(itr->first.c_str(), size);
			fOut.write((char*)&itr->second, sizeof(itr->second));
		}

		p_type->methods.FileOutput(fOut);

	}

	int entryPointsSize = arg_ref_data.map_entryPoints.size();
	fOut.write((char*)&entryPointsSize, sizeof(entryPointsSize));
	auto end = arg_ref_data.map_entryPoints.end();
	for (auto itr = arg_ref_data.map_entryPoints.begin(); itr != end; itr++) {
		int size = itr->first.size();
		fOut.write((char*)&size,sizeof(size));
		fOut.write(itr->first.c_str(), size);
		fOut.write((char*)&itr->second, sizeof(itr->second));
	}

	int publicGlobalValue = arg_ref_data.map_globalValueAddress.size();
	fOut.write((char*)&publicGlobalValue, sizeof(publicGlobalValue));
	auto itr = arg_ref_data.map_globalValueAddress.begin();
	for (int i = 0; i < publicGlobalValue; i++,itr++) {
		int strSize =itr->first.size();
		fOut.write((char*)&strSize, sizeof(int));
		fOut.write(itr->first.c_str(), strSize);
		fOut.write((char*)&itr->second, sizeof(int));
	}

	int defineTypeCount = arg_ref_data.vec_scriptClassInfo.size();
	fOut.write((char*)&defineTypeCount, sizeof(defineTypeCount));
	for (int i = 0; i < defineTypeCount; i++) {
		arg_ref_data.vec_scriptClassInfo[i].OutputFile(fOut);
	}

	int enumCount = arg_ref_data.map_enumTag.size();
	fOut.write((char*)&enumCount, sizeof(enumCount));
	for (auto itr = arg_ref_data.map_enumTag.begin(), end = arg_ref_data.map_enumTag.end(); itr != end; itr++) {
		fOut.write((char*)&itr->first,sizeof(int));
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

void ButiScript::ValueTable::Alloc(Compiler* arg_comp) const
{
	auto end = vec_variableTypes.end();
	for (auto itr = vec_variableTypes.begin(); itr != end; itr++)
	{
		if (*itr & TYPE_REF) {
			int typeIndex = *itr &~TYPE_REF;
			auto type = arg_comp->GetType(typeIndex);
			if (type->isSystem) {
				arg_comp->OpAllocStack_Ref(*itr);
			}
			else  if (type->p_enumTag) {
				arg_comp->OpAllocStack_Ref_EnumType(*itr);
			}
			else  if (type->isFunctionObject) {
				arg_comp->OpAllocStack_Ref_FunctionType(*itr);
			}
			else {
				arg_comp->OpAllocStack_Ref_ScriptType(*itr - arg_comp->GetSystemTypeSize());
			}
		}
		else {
			auto type = arg_comp->GetType(*itr);
			if (type->isSystem) {
				arg_comp->OpAllocStack(*itr);
			}
			else  if (type->p_enumTag) {
				arg_comp->OpAllocStackEnumType(*itr);
			}
			else  if (type->isFunctionObject) {
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
	RegistSystemType_<int>("int", "i");
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
void ButiScript::SystemTypeRegister::RegistEnum(const std::string& arg_typeName, const std::string& identiferName, const int value)
{
	auto enumType = enums.FindType(arg_typeName);
	if (!enumType) {
		RegistSystemEnumType(arg_typeName);
		enumType = enums.FindType(arg_typeName);
	}

	enumType->SetValue(identiferName, value);
}
int ButiScript::SystemTypeRegister::GetIndex(const std::string& arg_typeName)
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
	DefineSystemFunction(&VirtualCPU::sys_print, TYPE_VOID, "print", "s");
	DefineSystemFunction(&VirtualCPU::Sys_pause, TYPE_VOID, "pause", "");
	DefineSystemFunction(&VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "i");
	DefineSystemFunction(&VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "f");
	DefineSystemFunction(&VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "vec2");
	DefineSystemFunction(&VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "vec3");
	DefineSystemFunction(&VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "vec4");
#ifdef IMPL_BUTIENGINE

	DefineSystemFunction(&VirtualCPU::sys_registEventListner, TYPE_STRING, "RegistEvent", "s,s,s");
	DefineSystemFunction(&VirtualCPU::sys_unregistEventListner, TYPE_VOID, "UnRegistEvent", "s,s");
	DefineSystemFunction(&VirtualCPU::sys_addEventMessanger, TYPE_VOID, "AddEventMessanger", "s");
	DefineSystemFunction(&VirtualCPU::sys_executeEvent, TYPE_VOID, "EventExecute", "s");
	DefineSystemFunction(&VirtualCPU::sys_pushTask, TYPE_VOID, "PushTask", "s");
#endif // IMPL_BUTIENGINE


	DefineSystemMethod(&VirtualCPU::sys_method_retNo< Vector2, &Vector2::Normalize, &VirtualCPU::GetTypePtr >, TYPE_VOID + 1, TYPE_VOID, "Normalize", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2, &Vector2::GetNormalize, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetNormalize", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, float, &Vector2::GetLength, &VirtualCPU::GetTypePtr >, TYPE_VOID + 1, TYPE_FLOAT, "GetLength", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, float, &Vector2::GetLengthSqr, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 1, TYPE_FLOAT, "GetLengthSqr", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, float, Vector2, &Vector2::Dot, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 1, TYPE_FLOAT, "Dot", "vec2");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2&, int, &Vector2::Floor, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 1, (TYPE_VOID + 1), "Floor", "i");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2&, int, &Vector2::Round, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 1, (TYPE_VOID + 1), "Round", "i");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2&, int, &Vector2::Ceil, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr >, TYPE_VOID + 1, (TYPE_VOID + 1), "Ceil", "i");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2, int, &Vector2::GetFloor, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetFloor", "i");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2, int, &Vector2::GetRound, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetRound", "i");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2, int, &Vector2::GetCeil, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetCeil", "i");

	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3&, &Vector3::Normalize, &VirtualCPU::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID, "Normalize", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3, &Vector3::GetNormalize, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 2, TYPE_VOID + 2, "GetNormalize", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, float, &Vector3::GetLength, &VirtualCPU::GetTypePtr >, TYPE_VOID + 2, TYPE_FLOAT, "GetLength", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, float, &Vector3::GetLengthSqr, &VirtualCPU::GetTypePtr >, TYPE_VOID + 2, TYPE_FLOAT, "GetLengthSqr", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, float, Vector3, &Vector3::Dot, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr >, TYPE_VOID + 2, TYPE_FLOAT, "Dot", "vec3");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3, Vector3, &Vector3::GetCross, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetCross", "vec3");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3&, int, &Vector3::Floor, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 2, (TYPE_VOID + 2), "Floor", "i");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3&, int, &Vector3::Round, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 2, (TYPE_VOID + 2), "Round", "i");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3&, int, &Vector3::Ceil, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr >, TYPE_VOID + 2, (TYPE_VOID + 2), "Ceil", "i");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3, int, &Vector3::GetFloor, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr  >, TYPE_VOID + 2, TYPE_VOID + 2, "GetFloor", "i");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3, int, &Vector3::GetRound, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetRound", "i");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3, int, &Vector3::GetCeil, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetCeil", "i");

	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4, Matrix4x4&, &Matrix4x4::Transpose, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "Transpose", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4, Matrix4x4, &Matrix4x4::GetTranspose, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "GetTranspose", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4, Matrix4x4&, &Matrix4x4::Inverse, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "Inverse", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4, Matrix4x4, &Matrix4x4::GetInverse, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "GetInverse", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4, Matrix4x4&, &Matrix4x4::Identity, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "Identity", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4, Vector3, &Matrix4x4::GetEulerOneValue, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetEulerOneValue", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4, Vector3, &Matrix4x4::GetPosition, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetPosition", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4, Vector3, &Matrix4x4::GetPosition_Transpose, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetPosition_Transpose", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4,Vector3, &Matrix4x4::GetEulerOneValue_local, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetEulerOneValue_local", "");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4, Matrix4x4&, Vector3, &Matrix4x4::CreateFromEuler, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "CreateFromEuler", "vec3");
	DefineSystemMethod(&VirtualCPU::sys_method_ret< Matrix4x4, Matrix4x4&, Vector3, &Matrix4x4::CreateFromEuler_local, &VirtualCPU::GetTypePtr, &VirtualCPU::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "CreateFromEuler_local", "vec3");
	
}

bool ButiScript::SystemFuntionRegister::DefineSystemFunction(SysFunction arg_op, const int retType, const std::string& name, const std::string& args)
{
	FunctionTag func(retType, name);
	if (!func.SetArgs(args, SystemTypeRegister::GetInstance()->types .GetArgmentKeyMap()))
		return false;

	func.SetDeclaration();
	func.SetSystem();				// Systemフラグセット
	int index = vec_sysCalls.size();
	func.SetIndex(index);			// 組み込み関数番号を設定

	long long int address = *(long long int*) & arg_op;
	map_sysCallsIndex.emplace(address, vec_sysCalls.size());
	vec_sysCalls.push_back(arg_op);
	if (functions.Add(name, func) == 0) {
		return false;
	}
	return true;
}

bool ButiScript::SystemFuntionRegister::DefineSystemMethod(SysFunction arg_p_method, const int type, const int retType, const std::string& name, const std::string& arg_args)
{
	FunctionTag func(retType, name);
	if (!func.SetArgs(arg_args, SystemTypeRegister::GetInstance()->types.GetArgmentKeyMap()))
		return false;

	func.SetDeclaration();
	func.SetSystem();				// Systemフラグセット
	func.SetIndex(vec_sysMethodCalls.size());			// 組み込み関数番号を設定

	long long int address = *(long long int*) & arg_p_method;
	map_sysMethodCallsIndex.emplace(address, vec_sysMethodCalls.size());
	vec_sysMethodCalls.push_back(arg_p_method);
	auto typeTag = SystemTypeRegister::GetInstance()->types.GetType(type);
	if (typeTag->AddMethod(name, func) == 0) {
		return false;
	}
	return true;
}
int ButiScript::TypeTag::GetFunctionObjectReturnType() const
{
	if (!isFunctionObject) {
		return -1;
	}
	auto retTypeStr = StringHelper::Split(StringHelper::Split(typeName, ":")[1], ",")[0];
	return std::stoi(retTypeStr);
}
int ButiScript::TypeTag::GetFunctionObjectArgSize() const
{
	if (!isFunctionObject) {
		return -1;
	}
	auto argTypeStrs = StringHelper::Split(StringHelper::Split(typeName, ":")[1], ",");
	return argTypeStrs.size()-1;
}

std::vector<int> ButiScript::TypeTag::GetFunctionObjectArgment() const
{
	std::vector<int> output;
	auto argTypeStrs = StringHelper::Split(StringHelper::Split(typeName, ":")[1], ",");
	for (int i = 1,size= argTypeStrs.size(); i < size; i++) {
		output.push_back(std::stoi(argTypeStrs[ i]));
	}
	return output;
}

#ifndef IMPL_BUTIENGINE
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
