#include "stdafx.h"
#include "Compiler.h"

#include"VirtualMachine.h"
#include <iomanip>
#include "Parser.h"

// コンストラクタ
Compiler::Compiler()
	: break_index(-1), error_count(0)
{
}

Compiler::~Compiler()
{
}

// コンパイル
bool Compiler::Compile(const std::string& file, ButiVM::Data& Data)
{
	// 組み込み関数の設定
	DefineSystemFunction(ButiVM::SYS_PRINT, TYPE_VOID, "print", "s");
	DefineSystemFunction(ButiVM::SYS_PAUSE, TYPE_VOID, "pause", nullptr);
	DefineSystemFunction(ButiVM::SYS_TOSTR, TYPE_STRING, "ToString", "i");
	DefineSystemFunction(ButiVM::SYS_TOSTRF, TYPE_STRING, "ToStringF", "f");

	// グローバル変数用、変数テーブルをセット
	variables.push_back(ValueTable());
	variables[0].set_global();

	// 先頭はHALT命令にしておく
	OpHalt();

	bool result = ScriptParser(file, this);	// 構文解析

	if (!result)
		return false;// パーサーエラー

	int code_size = LabelSetting();				// ラベルにアドレスを設定
	CraeteData(Data, code_size);				// バイナリ生成
	return error_count == 0;
}

// エラーメッセージを出力
void Compiler::error(const std::string& m)
{
	std::cerr << m << std::endl;
	error_count++;
}

// 内部関数の定義
bool Compiler::DefineSystemFunction(int index, int type, const char* name, const char* args)
{
	FunctionTag func(type);
	if (!func.SetArgs(args))		// 引数を設定
		return false;

	func.SetDeclaration();			// 宣言済み
	func.SetSystem();				// Systemフラグセット
	func.SetIndex(index);			// 組み込み関数番号を設定
	if (functions.Add(name, func) == 0) {
		return false;
	}
	return true;
}

// 外部変数の定義
struct Define_value {
	Compiler* comp_;
	int type_;
	Define_value(Compiler* comp, int type) : comp_(comp), type_(type)
	{
	}

	void operator()(Node_t node) const
	{
		comp_->AddValue(type_, node->GetString(), node->GetLeft());
	}
};

void Compiler::ValueDefine(int type, const std::vector<Node_t>& node)
{
	std::for_each(node.begin(), node.end(), Define_value(this, type));
}

// 関数宣言
void Compiler::FunctionDefine(int type, const std::string& name, const std::vector<int>& args)
{
	const FunctionTag* tag = functions.find(name);
	if (tag) {			// 既に宣言済み
		if (!tag->ChkArgList(args)) {
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
//	したがって、引数の開始アドレスは-4となり、デクリメントしていく。

// 引数の変数名を登録
struct add_value {
	Compiler* comp_;
	ValueTable& values_;
	mutable int addr_;
	add_value(Compiler* comp, ValueTable& values) : comp_(comp), values_(values), addr_(-4)
	{
	}

	void operator()(const ArgDefine& arg) const
	{
		if (!values_.add_arg(arg.type(), arg.name(), addr_)) {
			comp_->error("引数 " + arg.name() + " は既に登録されています。");
		}
		addr_--;
	}
};

void Compiler::AddFunction(int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block, bool isReRegist)
{
	FunctionTag* tag = functions.find(name);
	if (tag) {
		if (tag->IsDefinition()&&!isReRegist) {
			error("関数 " + name + " は既に定義されています");
			return;
		}
		if (tag->IsDeclaration() && !tag->ChkArgList(args)) {
			error("関数 " + name + " に異なる型の引数が指定されています");
			return;
		}
		tag->SetDefinition();	// 定義済みに設定
	}
	else {
		FunctionTag func(type);
		func.SetArgs(args);				// 引数を設定
		func.SetDefinition();			// 定義済み
		func.SetIndex(MakeLabel());		// ラベル登録
		tag = functions.Add(name, func);
		if (tag == nullptr)
			error("内部エラー：関数テーブルに登録できません");
	}

	current_function_name = name;		// 処理中の関数名を登録しておく
	current_function_type = type;		// 処理中の関数型を登録しておく
										// 関数内関数（入れ子構造）は無いので、
										// グローバル変数１つでよい

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

		if (ret != 0) {
			if (isReRegist) {
				error("定義されていない関数を参照しています");
				return;
			}


			BlockOut();		// 変数スタックを減らす
			statement.pop_back();
			current_function_name.clear();		// 処理中の関数名を消去
			AddCallingNonDeclaredFunction(type, name, args, block);
			return;
		}
	}

	const VMCode& code = statement.back();
	if (type == TYPE_VOID) {			// 戻り値無し
		if (code.op_ != VM_RETURN)		// returnが無いならば
			OpReturn();					// returnを追加
	}
	else {
		if (code.op_ != VM_RETURNV) {	// returnが無いならば
			error("関数 " + name + " の最後にreturn文が有りません。");
		}
	}

	BlockOut();		// 変数スタックを減らす

	current_function_name.clear();		// 処理中の関数名を消去
}

// 変数の登録
void Compiler::AddValue(int type, const std::string& name, Node_t node)
{
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
	if (!values.Add(type, name, size)) {
		error("変数 " + name + " は既に登録されています。");
	}
}

// ラベル生成
int Compiler::MakeLabel()
{
	int index = (int)labels.size();
	labels.push_back(Label(index));
	return index;
}

// ラベルのダミーコマンドをステートメントリストに登録する
void Compiler::SetLabel(int label)
{
	statement.push_back(VMCode(VM_MAXCOMMAND, label));
}

// 文字列定数をpush
void Compiler::PushString(const std::string& str)
{
	PushString(((int)text_table.size()));
	text_table.insert(text_table.end(), str.begin(), str.end());
	text_table.push_back('\0');
}

// break文に対応したJmpコマンド生成

bool Compiler::JmpBreakLabel()
{
	if (break_index < 0)
		return false;
	OpJmp(break_index);
	return true;
}

// ブロック内では、新しい変数セットに変数を登録する

void Compiler::BlockIn()
{
	int start_addr = 0;					// 変数アドレスの開始位置
	if (variables.size() > 1) {			// ブロックの入れ子は、開始アドレスを続きからにする。
		start_addr = variables.back().size();
	}
	variables.push_back(ValueTable(start_addr));
}

// ブロックの終了で、変数スコープが消える（変数セットを削除する）

void Compiler::BlockOut()
{
	variables.pop_back();
}

// ローカル変数用にスタックを確保

void Compiler::AllocStack()
{
	OpAllocStack(variables.back().size());
}

// ラベル解決
//
// １．アドレスを生成する
// ２．ダミーのラベルコマンドが有ったアドレスを、ラベルテーブルに登録する
// ３．Jmpコマンドの飛び先をラベルテーブルに登録されたアドレスにする

// アドレス計算
struct calc_addr {
	std::vector<Label>& labels_;
	int& pos_;
	calc_addr(std::vector<Label>& labels, int& pos) : labels_(labels), pos_(pos)
	{
	}
	void operator()(const VMCode& code)
	{
		if (code.op_ == VM_MAXCOMMAND) {			// ラベルのダミーコマンド
			labels_[code.codeValue].pos_ = pos_;
		}
		else {
			pos_ += code.size_;
		}
	}
};

// ジャンプアドレス設定
struct set_addr {
	std::vector<Label>& labels_;
	set_addr(std::vector<Label>& labels) : labels_(labels)
	{
	}
	void operator()(VMCode& code)
	{
		switch (code.op_) {
		case VM_JMP:
		case VM_JMPC:
		case VM_JMPNC:
		case VM_TEST:
		case VM_CALL:
			code.codeValue = labels_[code.codeValue].pos_;
			break;
		}
	}
};

int Compiler::LabelSetting()
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
	void operator()(const VMCode& code)
	{
		p = code.Get(p);
	}
};

bool Compiler::CraeteData(ButiVM::Data& Data, int code_size)
{
	const FunctionTag* tag = GetFunctionTag("main");	// 開始位置
	if (tag == 0) {
		error("関数 \"main\" が見つかりません。");
		return false;
	}

	Data.command_ = new unsigned char[code_size];
	Data.text_buffer_ = new char[text_table.size()];
	Data.command_size_ = code_size;
	Data.text_size_ = (int)text_table.size();
	Data.value_size_ = (int)variables[0].size();
	Data.entry_point_ = labels[tag->index_].pos_;

	if (Data.text_size_ != 0)
		memcpy(Data.text_buffer_, &text_table[0], Data.text_size_);

	std::for_each(statement.begin(), statement.end(), copy_code(Data.command_));

	return true;
}

void Compiler::AddCallingNonDeclaredFunction(int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block)
{
	AddFunctionInfo info;
	info.type = type;
	info.name = name,
	info.args = args;
	info.block = block;
	vec_callingNonDeclaredFunctions.push_back(info);
}

void Compiler::ReRegistFunctions()
{
	auto end = vec_callingNonDeclaredFunctions.end();
	for (auto itr = vec_callingNonDeclaredFunctions.begin(); itr != end; itr++) {
		AddFunction(itr->type, itr->name, itr->args, itr->block,true);
	}
}

// デバッグダンプ
#ifdef	_DEBUG
void Compiler::debug_dump()
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
			std::cout << ", " << statement[i].codeValue;
		}
		std::cout << std::endl;

		if (statement[i].op_ != VM_MAXCOMMAND) {
			pos += statement[i].size_;
		}
	}
}
#endif
