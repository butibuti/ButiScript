#include "stdafx.h"
#include "Compiler.h"

#include"VirtualMachine.h"
#include <iomanip>
#include "Parser.h"

// コンストラクタ
ButiScript:: Compiler::Compiler()
	: break_index(-1), error_count(0)
{
}

ButiScript::Compiler::~Compiler()
{
}

// コンパイル
bool ButiScript::Compiler::Compile(const std::string& file, ButiScript::Data& Data)
{
	//グローバル名前空間の設定
	currentNameSpace = std::make_shared<NameSpace>("");
	//組み込み関数の設定

	DefineSystemFunction(&ButiScript::VirtualCPU::sys_print, TYPE_VOID, "print", "s");
	DefineSystemFunction(&ButiScript::VirtualCPU::Sys_pause, TYPE_VOID, "pause", nullptr);
	DefineSystemFunction(&ButiScript::VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "i");
	DefineSystemFunction(&ButiScript::VirtualCPU::sys_tostrf, TYPE_STRING, "ToString", "f");

	//変数テーブルをセット
	variables.push_back(ValueTable());
	variables[0].set_global();
	OpHalt();

	bool result = ScriptParser(file, this);	// 構文解析

	if (!result)
		return false;// パーサーエラー

	int code_size = LabelSetting();				// ラベルにアドレスを設定
	CraeteData(Data, code_size);				// バイナリ生成
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
	switch (type)
	{
	case TYPE_INTEGER:
		output = "int";
		break;
	case TYPE_FLOAT:
		output = "float";
		break;
	case TYPE_STRING:
		output = "string";
		break;
	}

	if (arg_type&TYPE_REF) {
		output += "&";
	}

	return output;
}


// 内部関数の定義
bool ButiScript::Compiler::DefineSystemFunction(SysFunction arg_op,const int type, const char* name, const char* args)
{
	FunctionTag func(type);
	if (!func.SetArgs(args))		// 引数を設定
		return false;

	func.SetDeclaration();			// 宣言済み
	func.SetSystem();				// Systemフラグセット
	func.SetIndex(vec_sysCalls.size());			// 組み込み関数番号を設定
	vec_sysCalls.push_back(arg_op);
	if (functions.Add(name, func) == 0) {
		return false;
	}
	return true;
}

// 外部変数の定義
struct Define_value {
	ButiScript::Compiler* comp_;
	int type_;
	Define_value(ButiScript::Compiler* comp, int type) : comp_(comp), type_(type)
	{
	}

	void operator()(ButiScript::Node_t node) const
	{
		comp_->AddValue(type_, node->GetString(), node->GetLeft());
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
		FunctionTag func(type);
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
	mutable int addr_;
	add_value(ButiScript::Compiler* comp, ButiScript::ValueTable& values) : comp_(comp), values_(values), addr_(-4)
	{
	}

	void operator()(const ButiScript::ArgDefine& arg) const
	{
		if (!values_.add_arg(arg.type(), arg.name(), addr_)) {
			comp_->error("引数 " + arg.name() + " は既に登録されています。");
		}
		addr_--;
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
		FunctionTag func(type);
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
	for (auto itr = args.rbegin(); itr != endItr; itr++) {
		add_value(this, variables.back())(*itr);
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
		FunctionTag func(type);
		func.SetArgs(args);				// 引数を設定
		func.SetDeclaration();			// 宣言済み
		func.SetIndex(MakeLabel());		// ラベル登録
		if (functions.Add(functionName, func) == 0) {
			error("内部エラー：関数テーブルに登録できません");
		}
	}
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
	if (break_index < 0)
		return false;
	OpJmp(break_index);
	return true;
}

// ブロック内では、新しい変数セットに変数を登録する

void ButiScript::Compiler::BlockIn()
{
	int start_addr = 0;					// 変数アドレスの開始位置
	if (variables.size() >= 1) {			// ブロックの入れ子は、開始アドレスを続きからにする。
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

bool ButiScript::Compiler::CraeteData(ButiScript::Data& Data, int code_size)
{
	const FunctionTag* tag = GetFunctionTag("main",std::vector<int>(),false);	// 開始位置
	if (tag == 0) {
		error("関数 \"main\" が見つかりません。");
		return false;
	}

	Data.commandTable = new unsigned char[code_size];
	Data.textBuffer = new char[text_table.size()];
	Data.commandSize = code_size;
	Data.textSize = (int)text_table.size();
	Data.valueSize = (int)variables[0].size();
	Data.entryPoint = labels[tag->index_].pos_;
	Data.vec_sysCalls = vec_sysCalls;

	if (Data.textSize != 0)
		memcpy(Data.textBuffer, &text_table[0], Data.textSize);

	std::for_each(statement.begin(), statement.end(), copy_code(Data.commandTable));

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
	auto end = variables_.rend();
	for (auto itr = variables_.rbegin(); itr != end; itr++)
	{
		arg_comp->OpAllocStack(itr->second.type_);
	}
}
