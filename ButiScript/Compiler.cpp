#include "stdafx.h"
#include "Compiler.h"

#include"VirtualMachine.h"
#include <iomanip>
#include "Parser.h"
#include<direct.h>
auto baseFunc = &ButiScript::VirtualCPU::Initialize;

// コンストラクタ
ButiScript:: Compiler::Compiler()
	: break_index(-1), error_count(0)
{
}

ButiScript::Compiler::~Compiler()
{
}

void ButiScript::Compiler::RegistDefaultSystems()
{

	//グローバル名前空間の設定
	currentNameSpace = std::make_shared<NameSpace>("");
	//組み込み関数の設定
	RegistSystemType<int, TYPE_INTEGER>("int", "i");
	RegistSystemType<float, TYPE_FLOAT>("float", "f");
	RegistSystemType<std::string, TYPE_STRING>("string", "s");
	RegistSystemType<int, TYPE_VOID>("void", "v");
	RegistSystemType<ButiEngine::Vector2, TYPE_VOID + 1>("Vector2", "vec2", "x:f,y:f");
	RegistSystemType<ButiEngine::Vector3, TYPE_VOID + 2>("Vector3", "vec3", "x:f,y:f,z:f");
	RegistSystemType<ButiEngine::Vector4, TYPE_VOID + 3>("Vector4", "vec4", "x:f,y:f,z:f,w:f");
	{
		using namespace ButiEngine;
		DefineSystemFunction(&VirtualCPU::sys_print, TYPE_VOID, "print", "s");
		DefineSystemFunction(&VirtualCPU::Sys_pause, TYPE_VOID, "pause", "");
		DefineSystemFunction(&VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "i");
		DefineSystemFunction(&VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "f");
		DefineSystemFunction(&VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "vec2");
		DefineSystemFunction(&VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "vec3");
		DefineSystemFunction(&VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "vec4");

		DefineSystemMethod(&VirtualCPU::sys_method_retNo< Vector2, &Vector2::Normalize >, TYPE_VOID + 1, TYPE_VOID, "Normalize", "");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2, &Vector2::GetNormalize >, TYPE_VOID + 1, TYPE_VOID + 1, "GetNormalize", "");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, float, &Vector2::GetLength >, TYPE_VOID + 1, TYPE_FLOAT, "GetLength", "");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, float, &Vector2::GetLengthSqr >, TYPE_VOID + 1, TYPE_FLOAT, "GetLengthSqr", "");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, float, Vector2, &Vector2::Dot >, TYPE_VOID + 1, TYPE_FLOAT, "Dot", "vec2");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2&, int, &Vector2::Floor >, TYPE_VOID + 1, (TYPE_VOID + 1), "Floor", "i");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2&, int, &Vector2::Round >, TYPE_VOID + 1, (TYPE_VOID + 1), "Round", "i");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2&, int, &Vector2::Ceil >, TYPE_VOID + 1, (TYPE_VOID + 1), "Ceil", "i");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2, int, &Vector2::GetFloor >, TYPE_VOID + 1, TYPE_VOID + 1, "GetFloor", "i");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2, int, &Vector2::GetRound >, TYPE_VOID + 1, TYPE_VOID + 1, "GetRound", "i");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector2, Vector2, int, &Vector2::GetCeil >, TYPE_VOID + 1, TYPE_VOID + 1, "GetCeil", "i");

		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3&, &Vector3::Normalize >, TYPE_VOID + 2, TYPE_VOID, "Normalize", "");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3, &Vector3::GetNormalize >, TYPE_VOID + 2, TYPE_VOID + 2, "GetNormalize", "");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, float, &Vector3::GetLength >, TYPE_VOID + 2, TYPE_FLOAT, "GetLength", "");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, float, &Vector3::GetLengthSqr >, TYPE_VOID + 2, TYPE_FLOAT, "GetLengthSqr", "");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, float, Vector3, &Vector3::Dot >, TYPE_VOID + 2, TYPE_FLOAT, "Dot", "vec3");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3, Vector3, &Vector3::GetCross >, TYPE_VOID + 2, TYPE_VOID + 2, "GetCross", "vec3");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3&, int, &Vector3::Floor >, TYPE_VOID + 2, (TYPE_VOID + 2), "Floor", "i");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3&, int, &Vector3::Round >, TYPE_VOID + 2, (TYPE_VOID + 2), "Round", "i");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3&, int, &Vector3::Ceil >, TYPE_VOID + 2, (TYPE_VOID + 2), "Ceil", "i");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3, int, &Vector3::GetFloor >, TYPE_VOID + 2, TYPE_VOID + 2, "GetFloor", "i");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3, int, &Vector3::GetRound >, TYPE_VOID + 2, TYPE_VOID + 2, "GetRound", "i");
		DefineSystemMethod(&VirtualCPU::sys_method_ret< Vector3, Vector3, int, &Vector3::GetCeil >, TYPE_VOID + 2, TYPE_VOID + 2, "GetCeil", "i");

	}

	RegistEnum("TestEnum", "First", 1);
	RegistEnum("TestEnum", "Second", 2);
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


// 内部関数の定義
bool ButiScript::Compiler::DefineSystemFunction(SysFunction arg_op,const int type, const std::string& name, const std::string&  args)
{
	FunctionTag func(type,name);
	if (!func.SetArgs(args,types.GetArgmentKeyMap()))		
		return false;

	func.SetDeclaration();			
	func.SetSystem();				// Systemフラグセット
	func.SetIndex(vec_sysCalls.size());			// 組み込み関数番号を設定

	long long int address = *(long long int*) & arg_op;
	map_sysCallsIndex.emplace(address, vec_sysCalls.size());
	vec_sysCalls.push_back(arg_op);
	if (functions.Add(name, func) == 0) {
		return false;
	}
	return true;
}

bool ButiScript::Compiler::DefineSystemMethod(SysFunction arg_p_method, const int type, const int retType, const std::string& name, const std::string& arg_args)
{
	FunctionTag func(retType,name);
	if (!func.SetArgs(arg_args, types.GetArgmentKeyMap()))
		return false;

	func.SetDeclaration();
	func.SetSystem();				// Systemフラグセット
	func.SetIndex(vec_sysMethodCalls.size());			// 組み込み関数番号を設定

	long long int address = *(long long int*) & arg_p_method;
	map_sysMethodCallsIndex.emplace(address, vec_sysMethodCalls.size());
	vec_sysMethodCalls.push_back(arg_p_method);
	if (types.GetType(type)->AddMethod(name, func) == 0) {
		return false;
	}
	return true;
}

// 外部変数の定義
struct Define_value {
	ButiScript::Compiler* comp_;
	int valueType;
	Define_value(ButiScript::Compiler* comp, int type) : comp_(comp), valueType(type)
	{
	}

	void operator()(ButiScript::Node_t node) const
	{
		comp_->AddValue(valueType, node->GetString(), node->GetLeft());
	}
};

void ButiScript::Compiler::ValueDefine(int type, const std::vector<Node_t>& node)
{
	std::for_each(node.begin(), node.end(), Define_value(this, type));
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

void ButiScript::Compiler::AddFunction(int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block, bool isReRegist)
{

	std::string functionName = currentNameSpace->GetGlobalNameString()+ name;
	FunctionTag* tag = functions.Find_strict(functionName,args);
	if (tag) {
		if (tag->IsDefinition()&&!isReRegist) {
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
		tag = functions.Add(functionName, func);
		if (tag == nullptr)
			error("内部エラー：関数テーブルに登録できません");
	}

	current_function_name = functionName;		// 処理中の関数名を登録
	current_function_type = type;		// 処理中の関数型を登録

	// 関数のエントリーポイントにラベルを置く

	SetLabel(tag->GetIndex());

	BlockIn();		// 変数スタックを増やす

	// 引数リストを登録
	auto endItr = args.rend();
	int address = -4;
	for (auto itr = args.rbegin(); itr != endItr; itr++) {
		add_value(this, variables.back(),address)(*itr);
		address--;
	}

	// 文があれば、文を登録
	if (block) {
		int ret=block->Analyze(this);
	}

	const VMCode& code = statement.back();
	if (type == TYPE_VOID) {			// 戻り値無し
		if (code.op_ != VM_RETURN)		// returnが無いならば
			OpReturn();					// returnを追加
	}
	else {
		if (code.op_ != VM_RETURNV) {	// returnが無いならば
			error("関数 " + functionName + " の最後にreturn文が有りません。");
		}
	}

	BlockOut();		// 変数スタックを減らす

	current_function_name.clear();		// 処理中の関数名を消去

}

void ButiScript::Compiler::RegistFunction(const int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block, const bool isReRegist)
{
	std::string functionName = currentNameSpace->GetGlobalNameString() + name;
	const FunctionTag* tag = functions.Find_strict(functionName,args);
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
		if (functions.Add(functionName, func) == 0) {
			error("内部エラー：関数テーブルに登録できません");
		}
	}
}

void ButiScript::Compiler::RegistEnum(const std::string& arg_typeName, const std::string& identiferName, const int value)
{
	auto enumType = GetEnumTag(arg_typeName);
	if (!enumType) {
		auto typeName = currentNameSpace->GetGlobalNameString() + arg_typeName;
		EnumTag tag(typeName);
		enums.SetEnum(tag);
		enumType = enums.FindType(arg_typeName);
	}

	enumType->SetValue(identiferName, value);
}

void ButiScript::Compiler::RegistEnumType(const std::string& arg_typeName)
{
	auto typeName = currentNameSpace->GetGlobalNameString()+arg_typeName;
	EnumTag tag(typeName);
	enums.SetEnum(tag);
}

void ButiScript::Compiler::RegistSystemEnumType(const std::string& arg_typeName)
{
	EnumTag tag(arg_typeName);
	tag.isSystem = true;
	enums.SetEnum(tag);
}

// 変数の登録
void ButiScript::Compiler::AddValue(int type, const std::string& name, Node_t node)
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
	if (!values.Add(type, valueName, size)) {
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
		Data.map_entryPoints.emplace(functions[i]-> GetNameWithArgment(types),labels[functions[i]->index_].pos_);
	}


	Data.commandTable = new unsigned char[code_size];
	Data.textBuffer = new char[text_table.size()];
	Data.commandSize = code_size;
	Data.textSize = (int)text_table.size();
	Data.valueSize = (int)variables[0].size();

	Data.vec_sysCalls = vec_sysCalls;
	Data.vec_sysCallMethods = vec_sysMethodCalls;
	types.CreateTypeVec(Data.vec_types);


	if (Data.textSize != 0)
		memcpy(Data.textBuffer, &text_table[0], Data.textSize);

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
#endif

int ButiScript::Compiler::InputCompiledData(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data)
{
	std::ifstream fIn(arg_filePath);

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
		SysFunction sysFunc = vec_valueAllocCall[index];
		typeTag.typeFunc = sysFunc;

		index = 0;
		fIn.read((char*)&index, sizeof(index));
		sysFunc = vec_refValueAllocCall[index];
		typeTag.refTypeFunc = sysFunc;




		fIn.read((char*)&typeTag.typeIndex, sizeof(int));

		int typeMapSize = 0;
		fIn.read((char*)&typeMapSize, sizeof(typeMapSize));

		for (int j=0; j < typeMapSize;j++) {
			int size =0;
			std::string typeNameStr;
			int typeIndex;
			fIn.read((char*)&size, sizeof(size));
			char* p_strBuff = (char*)malloc(size);
			free(p_strBuff);
			fIn.read(p_strBuff, size);
			typeNameStr = std::string(p_strBuff,size);
			fIn.read((char*)&typeIndex, sizeof(typeIndex));
			typeTag.map_memberType.emplace(typeNameStr, typeIndex);
		}
		fIn.read((char*)&typeMapSize, sizeof(typeMapSize));
		
		for (int j=0; j < typeMapSize; j++) {
			int size = 0;
			std::string typeNameStr;
			int typeIndex;
			fIn.read((char*)&size, sizeof(size));
			char* p_strBuff = (char*)malloc(size);
			free(p_strBuff);
			fIn.read(p_strBuff, size);
			typeNameStr = std::string(p_strBuff, size);
			fIn.read((char*)&typeIndex, sizeof(typeIndex));
			typeTag.map_memberIndex.emplace(typeNameStr, typeIndex);
		}

		typeTag.methods.FileInput(fIn);

		arg_ref_data.vec_types.push_back(typeTag);
	}


	int entryPointsSize = 0;
	fIn.read((char*)&entryPointsSize, sizeof(entryPointsSize));
	for (int i = 0; i < entryPointsSize;i++) {
		int size =0;

		fIn.read((char*)&size, sizeof(size));
		char* buff = (char*)malloc(size);
		fIn.read(buff, size);
		int entryPoint = 0;
		fIn.read((char*)&entryPoint, sizeof(entryPoint));
		std::string name =std::string( buff,size);
		free(buff);
		arg_ref_data.map_entryPoints.emplace(name, entryPoint);
	}


	fIn.read((char*)&arg_ref_data.definedTypeCount, sizeof(arg_ref_data.definedTypeCount));

	fIn.close();

	return 0;
}

int ButiScript::Compiler::OutputCompiledData(const std::string& arg_filePath, const ButiScript::CompiledData& arg_ref_data)
{

	auto dirPath = StringHelper::GetDirectory(arg_filePath);
	if (dirPath != arg_filePath) {
		auto dirRes = _mkdir(dirPath.c_str());
	}
	std::ofstream fOut(arg_filePath);
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
		long long int address = *(long long int*) & p_sysCallFunc;
		int index = map_valueAllocCallsIndex.at(address);
		fOut.write((char*)&index, sizeof(index));

		p_sysCallFunc = p_type->refTypeFunc;
		address = *(long long int*) & p_sysCallFunc;
		index = map_refValueAllocCallsIndex.at(address);
		fOut.write((char*)&index, sizeof(index));



		fOut.write((char*)&p_type->typeIndex, sizeof(p_type->typeIndex));

		int typeMapSize = p_type->map_memberType.size();
		fOut.write((char*)&typeMapSize, sizeof(typeMapSize));
		auto end = p_type->map_memberType.end();
		for (auto itr = p_type->map_memberType.begin(); itr != end; itr++) {
			int size = itr->first.size();
			fOut.write((char*)&size, sizeof(size));
			fOut.write(itr->first.c_str(), size);
			fOut.write((char*)&itr->second, sizeof(itr->second));
		}
		typeMapSize = p_type->map_memberIndex.size();
		fOut.write((char*)&typeMapSize, sizeof(typeMapSize));
		end = p_type->map_memberIndex.end();
		for (auto itr = p_type->map_memberIndex.begin(); itr != end; itr++) {
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


	fOut.write((char*)&arg_ref_data.definedTypeCount, sizeof(arg_ref_data.definedTypeCount));	
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
			arg_comp->OpAllocStack_Ref(*itr);
		}
		else {
			arg_comp->OpAllocStack(*itr);
		}
		
	}
}
