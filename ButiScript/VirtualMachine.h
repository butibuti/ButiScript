#pragma once
#ifndef	__VM_H__
#define	__VM_H__

#include <vector>

#include "vm_value.h"
namespace ButiScript {

#include"value_type.h"

#define	VM_ENUMDEF
	enum {
#include "VM_enum.h"
		VM_MAXCOMMAND,
	};
#undef	VM_ENUMDEF

	class VirtualCPU;
	using OperationFunction = void (VirtualCPU::*)();

	struct SystemTypeDefine {
		SystemTypeDefine(){}
		OperationFunction typeFunc;
		int typeIndex;
		std::string typeName;
	};

	class Data {
	public:
		Data() : commandTable(0), textBuffer(0)
		{
		}
		~Data()
		{
			delete[] commandTable;
			delete[] textBuffer;
		}

	public:
		unsigned char* commandTable;	// コマンドテーブル
		char* textBuffer;			// テキストデータ
		int commandSize;			// コマンドサイズ
		int textSize;				// テキストサイズ
		int valueSize;			// グローバル変数サイズ
		int entryPoint;			// エントリーポイント

		std::vector<OperationFunction> vec_sysCalls;
		std::vector<SystemTypeDefine> vec_types;
		int definedTypeCount = 0;
	};


	// 0除算
	class DevideByZero : public std::exception {
	public:
		const char* what() const throw()
		{
			return "devide by zero";
		}
	};

	// 仮想マシン
	class VirtualCPU {
	public:
		const static int STACK_SIZE = 2048;
		const static int global_flag = 0x4000000;
		const static int global_mask = 0x3ffffff;


	public:
		VirtualCPU(Data& mem)
			: data_(mem)
		{
		}
		~VirtualCPU()
		{
		}

		int Run();

		void Initialize();

	private:
		// 定数Push
		void PushConstInt(const int arg_val)
		{
			push(arg_val);
		}

		void PushConstInt()
		{
			PushConstInt(Value_Int());
		}

		// 定数Push
		void PushConstFloat(const float arg_val)
		{
			push(arg_val);
		}


		void PushConstFloat()
		{
			PushConstFloat(Value_Float());
		}

		// 文字定数Push
		void PushString(const int arg_val)
		{
			push(std::string(textBuffer + arg_val));
		}

		void PushString()
		{
			PushString(Value_Int());
		}

		// 変数Push
		void PushValue(const int arg_val)
		{
			push(global_value[arg_val]);
		}
		void PushValue()
		{
			PushValue(Value_Int());
		}

		// ローカル変数Push
		void PushLocal(const int arg_val)
		{
			push(Stack[arg_val + stack_base]);
		}

		void PushLocal()
		{
			PushLocal(Value_Int());
		}

		// 配列からPush
		void PushArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(global_value[(int)(arg_val + index)]);
		}

		void PushArray()
		{
			PushArray(Value_Int());
		}

		// ローカルの配列からPush
		void PushLocalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(Stack[arg_val + stack_base + index]);
		}

		void PushLocalArray()
		{
			PushLocalArray(Value_Int());
		}

		// ローカル変数(参照)Push
		void PushLocalRef(const int arg_val)
		{
			int addr = Stack[arg_val + stack_base].v_->Get<int>();
			push(ref_to_value(addr));
		}
		void PushLocalRef()
		{
			PushLocalRef(Value_Int());
		}

		// ローカルの配列(参照)からPush
		void PushLocalArrayRef(const int arg_val)
		{
			int addr = Stack[arg_val + stack_base].v_->Get<int>();
			int index = top().v_->Get<int>(); pop();
			push(ref_to_value(addr + index));
		}
		void PushLocalArrayRef(){
			PushLocalArrayRef(Value_Int());
		}

		// アドレスをPush
		void PushAddr(const int arg_val)
		{
			int base = arg_val;
			if ((arg_val & global_flag) == 0)	// local
				base += +stack_base;
			push(base);
		}

		void PushAddr() {
			PushAddr(Value_Int());
		}

		// 配列のアドレスをPush
		void PushArrayAddr(const int arg_val)
		{
			int base = arg_val;
			if ((arg_val & global_flag) == 0)	// local
				base += +stack_base;
			int index = top().v_->Get<int>(); pop();
			push(base + index);
		}
		void PushArrayAddr() {
			PushArrayAddr(Value_Int());
		}
		// 変数にPop
		void PopValue(const int arg_val)
		{
			global_value[arg_val] = top(); pop();
		}
		void PopValue() {
			PopValue(Value_Int());
		}
		// ローカル変数にPop
		void PopLocal(const int arg_val)
		{
			Stack[arg_val + stack_base] = top(); pop();
		}
		void PopLocal() {
			PopLocal(Value_Int());
		}

		// 配列変数にPop
		void PopArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			global_value[(int)(arg_val + index)] = top(); pop();
		}
		void PopArray() {
			PopArray(Value_Int());
		}

		// ローカルの配列変数にPop
		void PopLocalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			Stack[arg_val + stack_base + index] = top(); pop();
		}

		void PopLocalArray() {
			PopLocalArray(Value_Int());
		}

		// ローカル変数(参照)にPop
		void PopLocalRef(const int arg_val)
		{
			int addr = Stack[arg_val + stack_base].v_->Get<int>();
			set_ref(addr, top()); pop();
		}
		void PopLocalRef() {
			PopLocalRef(Value_Int());
		}
		// ローカルの配列変数(参照)にPop
		void PopLocalArrayRef(const int arg_val)
		{
			int addr = Stack[arg_val + stack_base].v_->Get<int>();
			int index = top().v_->Get<int>(); pop();
			set_ref(addr + index, top()); pop();
		}
		void PopLocalArrayRef()
		{
			PopLocalArrayRef(Value_Int());
		}

		// ローカル変数を確保
		void OpAllocStack(const int arg_val)
		{
			(this->*p_pushValues[arg_val])();
			
		}
		void OpAllocStack()
		{
			OpAllocStack(Value_Int());
		}

		// 空Pop（スタックトップを捨てる）
		void OpPop()
		{
			pop();
		}

		// 単項マイナス
		void OpNeg()
		{
			top().v_->Nagative();
		}

		// ==
		void OpEq()
		{
			auto rhs = top().v_; pop();
			auto lhs = top().v_; pop();
			push(lhs ->Eq( rhs));
		}

		// !=
		void OpNe()
		{
			auto rhs = top().v_; pop();
			auto lhs = top().v_; pop();
			push(!lhs->Eq(rhs));
		}

		// >
		void OpGt()
		{
			auto rhs = top().v_; pop();
			auto lhs = top().v_; pop();
			push(lhs ->Gt( rhs));
		}

		// >=
		void OpGe()
		{
			auto rhs = top().v_; pop();
			auto lhs = top().v_; pop();
			push(lhs->Ge(rhs));
		}

		// <
		void OpLt()
		{
			auto rhs = top().v_; pop();
			auto lhs = top().v_; pop();
			push(!lhs->Ge(rhs));
		}

		// <=
		void OpLe()
		{
			auto rhs = top().v_; pop();
			auto lhs = top().v_; pop();
			push(!lhs->Gt(rhs));
		}

		// &&
		void OpLogAnd()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs && rhs);
		}

		// ||
		void OpLogOr()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs || rhs);
		}

		// &
		void OpAnd()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs & rhs);
		}

		// |
		void OpOr()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs | rhs);
		}

		// <<
		void OpLeftShift()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs << rhs);
		}

		// >>
		void OpRightShift()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs >> rhs);
		}

		// +
		void OpAdd()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs + rhs);
		}

		// -
		void OpSub()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs - rhs);
		}

		// *
		void OpMul()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs * rhs);
		}

		// /
		void OpDiv()
		{
			auto rhs = top().v_->Get<int>(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs / rhs);
		}

		// %
		void OpMod()
		{
			auto rhs = top().v_->Get<int>(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs % rhs);
		}


		// +
		void OpFloatAdd()
		{
			auto rhs = top().v_->Get<float>(); pop();
			auto lhs = top().v_->Get<float>(); pop();
			push(lhs + rhs);
		}

		// -
		void OpFloatSub()
		{
			auto rhs = top().v_->Get<float>(); pop();
			auto lhs = top().v_->Get<float>(); pop();
			push(lhs - rhs);
		}

		// *
		void OpFloatMul()
		{
			auto rhs = top().v_->Get<float>(); pop();
			auto lhs = top().v_->Get<float>(); pop();
			push(lhs * rhs);
		}

		// /
		void OpFloatDiv()
		{
			auto rhs = top().v_->Get<float>(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->Get<float>(); pop();
			push(lhs / rhs);
		}

		// %
		void OpFloatMod()
		{
			auto rhs = top().v_->Get<float>(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->Get<float>(); pop();
			push((int)lhs %(int) rhs);
		}

		// 文字列の==
		void OpStrEq()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs == rhs);
		}

		// 文字列の!=
		void OpStrNe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs != rhs);
		}

		// 文字列の>
		void OpStrGt()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs > rhs);
		}

		// 文字列の>=
		void OpStrGe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs >= rhs);
		}

		// 文字列の<
		void OpStrLt()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs < rhs);
		}

		// 文字列の<=
		void OpStrLe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs <= rhs);
		}

		// 文字列の+
		void OpStrAdd()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs + rhs);
		}

		// 無条件ジャンプ
		void OpJmp(const int arg_val)
		{
			jmp(arg_val);
		}
		void OpJmp() {
			OpJmp(Value_Int());
		}

		// 真の時ジャンプ
		void OpJmpC(const int arg_val)
		{
			int cond = top().v_->Get<int>(); pop();
			if (cond)
				jmp(arg_val);
		}

		void OpJmpC() {
			OpJmpC(Value_Int());
		}

		// 偽の時ジャンプ
		void OpJmpNC(const int arg_val)
		{
			int cond = top().v_->Get<int>(); pop();
			if (!cond)
				jmp(arg_val);
		}

		void OpJmpNC() {
			OpJmpNC(Value_Int());
		}

		// switch文用特殊判定
		void OpTest(const int arg_val)
		{
			int Value = top().v_->Get<int>(); pop();
			if (Value == top().v_->Get<int>()) {
				pop();
				jmp(arg_val);
			}
		}

		void OpTest() {
			OpTest(Value_Int());
		}

		// 関数コール
		void OpCall(const int arg_val)
		{
			push(stack_base);
			push(addr());					// リターンアドレスをPush
			stack_base = Stack.size();		// スタックベース更新
			jmp(arg_val);
		}

		void OpCall() {
			OpCall(Value_Int());
		}

		// 引数なしリターン
		void OpReturn()
		{
			Stack.resize(stack_base);		// ローカル変数排除
			int addr = top().v_->Get<int>(); pop();
			stack_base = top().v_->Get<int>(); pop();
			int arg_count = top().v_->Get<int>(); pop();
			Stack.pop(arg_count);
			jmp(addr);
		}

		// 引数付きリターン
		void OpReturnV()
		{
			ButiScript::Value result = top(); pop();
			Stack.resize(stack_base);		// ローカル変数排除
			int addr = top().v_->Get<int>(); pop();
			stack_base = top().v_->Get<int>(); pop();
			int arg_count = top().v_->Get<int>(); pop();
			Stack.pop(arg_count);
			push(result);
			jmp(addr);
		}

		// 仮想CPU停止
		void OpHalt()
		{
		}

		// 組み込み関数
		void OpSysCall(const int val)
		{
			pop();	// arg_count

			(this->*p_syscall[val])();

		}

		void OpSysCall() {
			OpSysCall(Value_Int());
		}
	public:
		// 組み込み関数（print）
		void sys_print()
		{
			std::cout << text(top());
			pop();
		}

		// 組み込み関数（pause）
		void Sys_pause() {
			std::system("pause");
		}

		// 組み込み関数(数値を文字列に変換)
		void sys_tostr()
		{
			auto v = top().v_->Get<std::string>(); pop();
			push(v);			// 戻り値はスタックに入れる
		}
		// 組み込み関数(数値を文字列に変換)
		void sys_tostrf()
		{
			auto v = top().v_->Get<std::string>(); pop();
			push(v);			// 戻り値はスタックに入れる
		}

		template<typename T>
		void pushValue() {
			Stack.push(Value(T()));
		}

	private:
		int Value_Int() { int v = *(int*)command_ptr_; command_ptr_ += 4; return v; }
		float Value_Float() { float v = *(float*)command_ptr_; command_ptr_ += 4; return v; }
		int addr() const { return (int)(command_ptr_ - commandTable); }
		void jmp(int addr) { command_ptr_ = commandTable + addr; }
		void push(int v) { 
			Stack.push(ButiScript::Value(v));
		}
		void push(float v) { 
			Stack.push(ButiScript::Value(v)); 
		}
		void push(const std::string& v) { 
			Stack.push(ButiScript::Value(v)); 
		}
		void push(const ButiScript::Value& v) { 
			Stack.push(v); 
		}
		void pop() { 
			Stack.pop(); 
		}
		const ButiScript::Value& top() const { 
			return Stack.top();
		}
		ButiScript::Value& top() { 
			return Stack.top(); 
		}
		std::string text(const ButiScript::Value& v) { return v.v_->Get<std::string>(); }
		const ButiScript::Value& ref_to_value(const int addr) const
		{
			if (addr & global_flag)
				return global_value[addr & global_mask];
			return Stack[addr];
		}
		void set_ref(const int addr, const ButiScript::Value& v)
		{
			if (addr & global_flag)
				PopLocal(addr-1);
			else
				Stack[addr] = v;
		}

	private:
		Data& data_;

		//コマンド羅列
		unsigned char* commandTable;
		//現在参照してるコマンドの位置
		unsigned char* command_ptr_;
		//グローバル変数の確保コマンド
		unsigned char* allocCommand_ptr_;
		//プログラム全体のサイズ
		int commandSize;
		//文字列データ
		char* textBuffer;
		//文字列データのサイズ
		int textSize;

		//命令テーブル
		OperationFunction* p_op=nullptr;
		//組み込み関数テーブル
		OperationFunction* p_syscall=nullptr;
		//変数の確保関数テーブル
		OperationFunction* p_pushValues = nullptr;


		ButiScript::Stack<ButiScript::Value, STACK_SIZE> Stack;
		//グローバル変数
		std::vector<ButiScript::Value> global_value;
		//スタックの参照位置
		int stack_base;

		
	};

}

#endif
