#pragma once
#ifndef	__VM_H__
#define	__VM_H__

#include <vector>
#include "vm_value.h"


#define	VM_ENUMDEF
enum {
#include "VM_enum.h"
	VM_MAXCOMMAND,
};
#undef	VM_ENUMDEF

namespace ButiVM {

	enum {
		SYS_PRINT,
		SYS_PAUSE,
		SYS_TOSTR,
		SYS_TOSTRF,
	};

	class Data {
	public:
		Data() : command_(0), text_buffer_(0)
		{
		}
		~Data()
		{
			delete[] command_;
			delete[] text_buffer_;
		}

	public:
		unsigned char* command_;	// コマンドテーブル
		char* text_buffer_;			// テキストデータ
		int command_size_;			// コマンドサイズ
		int text_size_;				// テキストサイズ
		int value_size_;			// グローバル変数サイズ
		int entry_point_;			// エントリーポイント
	};


	// 0除算例外
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
		void PushConst(const int arg_val)
		{
			push(arg_val);
		}
		// 定数Push
		void PushConstFloat(const float arg_val)
		{
			push(arg_val);
		}

		// 文字定数Push
		void PushString(const int arg_val)
		{
			push(std::string(text_buffer_ + arg_val));
		}

		// 変数Push
		void PushValue(const int arg_val)
		{
			push(global_value[arg_val]);
		}

		// ローカル変数Push
		void PushLocal(const int arg_val)
		{
			push(Stack[arg_val + stack_base]);
		}

		// 配列からPush
		void PushArray(const int arg_val)
		{
			int index = *top().v_->GetIntPtr(); pop();
			push(global_value[arg_val + index]);
		}

		// ローカルの配列からPush
		void PushLocalArray(const int arg_val)
		{
			int index = *top().v_->GetIntPtr(); pop();
			push(Stack[arg_val + stack_base + index]);
		}

		// ローカル変数(参照)Push
		void PushLocalRef(const int arg_val)
		{
			int addr =* Stack[arg_val + stack_base].v_->GetIntPtr();
			push(ref_to_value(addr));
		}

		// ローカルの配列(参照)からPush
		void PushLocalArrayRef(const int arg_val)
		{
			int addr =* Stack[arg_val + stack_base].v_->GetIntPtr();
			int index = *top().v_->GetIntPtr(); pop();
			push(ref_to_value(addr + index));
		}

		// アドレスをPush
		void PushAddr(const int arg_val)
		{
			int base = arg_val;
			if ((arg_val & global_flag) == 0)	// local
				base += +stack_base;
			push(base);
		}

		// 配列のアドレスをPush
		void PushArrayAddr(const int arg_val)
		{
			int base = arg_val;
			if ((arg_val & global_flag) == 0)	// local
				base += +stack_base;
			int index = *top().v_->GetIntPtr(); pop();
			push(base + index);
		}

		// 変数にPop
		void PopValue(const int arg_val)
		{
			global_value[arg_val] = top(); pop();
		}

		// ローカル変数にPop
		void PopLocal(const int arg_val)
		{
			Stack[arg_val + stack_base] = top(); pop();
		}

		// 配列変数にPop
		void PopArray(const int arg_val)
		{
			int index = *top().v_->GetIntPtr(); pop();
			global_value[arg_val + index] = top(); pop();
		}

		// ローカルの配列変数にPop
		void PopLocalArray(const int arg_val)
		{
			int index = *top().v_->GetIntPtr(); pop();
			Stack[arg_val + stack_base + index] = top(); pop();
		}

		// ローカル変数(参照)にPop
		void PopLocalRef(const int arg_val)
		{
			int addr =* Stack[arg_val + stack_base].v_->GetIntPtr();
			set_ref(addr, top()); pop();
		}

		// ローカルの配列変数(参照)にPop
		void PopLocalArrayRef(const int arg_val)
		{
			int addr =* Stack[arg_val + stack_base].v_->GetIntPtr();
			int index = *top().v_->GetIntPtr(); pop();
			set_ref(addr + index, top()); pop();
		}

		// ローカル変数を確保
		void OpAllocStack(const int arg_val)
		{
			Stack.resize(stack_base + arg_val);
		}

		// 空Pop（スタックトップを捨てる）
		void OpPop()
		{
			pop();
		}

		// 単項マイナス
		void OpNeg()
		{
			top().v_->ToNegative();
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
			auto rhs = top().v_->ToInt(); pop();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs && rhs);
		}

		// ||
		void OpLogOr()
		{
			auto rhs = top().v_->ToInt(); pop();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs || rhs);
		}

		// &
		void OpAnd()
		{
			auto rhs = top().v_->ToInt(); pop();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs & rhs);
		}

		// |
		void OpOr()
		{
			auto rhs = top().v_->ToInt(); pop();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs | rhs);
		}

		// <<
		void OpLeftShift()
		{
			auto rhs = top().v_->ToInt(); pop();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs << rhs);
		}

		// >>
		void OpRightShift()
		{
			auto rhs = top().v_->ToInt(); pop();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs >> rhs);
		}

		// +
		void OpAdd()
		{
			auto rhs = top().v_->ToInt(); pop();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs + rhs);
		}

		// -
		void OpSub()
		{
			auto rhs = top().v_->ToInt(); pop();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs - rhs);
		}

		// *
		void OpMul()
		{
			auto rhs = top().v_->ToInt(); pop();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs * rhs);
		}

		// /
		void OpDiv()
		{
			auto rhs = top().v_->ToInt(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs / rhs);
		}

		// %
		void OpMod()
		{
			auto rhs = top().v_->ToInt(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->ToInt(); pop();
			push(lhs % rhs);
		}


		// +
		void OpFloatAdd()
		{
			auto rhs = top().v_->ToFloat(); pop();
			auto lhs = top().v_->ToFloat(); pop();
			push(lhs + rhs);
		}

		// -
		void OpFloatSub()
		{
			auto rhs = top().v_->ToFloat(); pop();
			auto lhs = top().v_->ToFloat(); pop();
			push(lhs - rhs);
		}

		// *
		void OpFloatMul()
		{
			auto rhs = top().v_->ToFloat(); pop();
			auto lhs = top().v_->ToFloat(); pop();
			push(lhs * rhs);
		}

		// /
		void OpFloatDiv()
		{
			auto rhs = top().v_->ToFloat(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->ToFloat(); pop();
			push(lhs / rhs);
		}

		// %
		void OpFloatMod()
		{
			auto rhs = top().v_->ToFloat(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->ToFloat(); pop();
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

		// 真の時ジャンプ
		void OpJmpC(const int arg_val)
		{
			int cond = *top().v_->GetIntPtr(); pop();
			if (cond)
				jmp(arg_val);
		}

		// 偽の時ジャンプ
		void OpJmpNC(const int arg_val)
		{
			int cond = *top().v_->GetIntPtr(); pop();
			if (!cond)
				jmp(arg_val);
		}

		// switch文用特殊判定
		void OpTest(const int arg_val)
		{
			int Value = *top().v_->GetIntPtr(); pop();
			if (Value == *top().v_->GetIntPtr()) {
				pop();
				jmp(arg_val);
			}
		}

		// 関数コール
		void OpCall(const int arg_val)
		{
			push(stack_base);
			push(addr());					// リターンアドレスをPush
			stack_base = Stack.size();		// スタックベース更新
			jmp(arg_val);
		}

		// 引数なしリターン
		void OpReturn()
		{
			Stack.resize(stack_base);		// ローカル変数排除
			int addr = *top().v_->GetIntPtr(); pop();
			stack_base = *top().v_->GetIntPtr(); pop();
			int arg_count = *top().v_->GetIntPtr(); pop();
			Stack.pop(arg_count);
			jmp(addr);
		}

		// 引数付きリターン
		void OpReturnV()
		{
			ButiVM::Value result = top(); pop();
			Stack.resize(stack_base);		// ローカル変数排除
			int addr = *top().v_->GetIntPtr(); pop();
			stack_base = *top().v_->GetIntPtr(); pop();
			int arg_count = *top().v_->GetIntPtr(); pop();
			Stack.pop(arg_count);
			push(result);
			jmp(addr);
		}

		// 仮想CPUプログラム停止
		void OpHalt()
		{
		}

		// 組み込み関数
		void OpSysCall(int val)
		{
			pop();	// arg_count
			switch (val) {
			case SYS_PRINT:
				sys_print();
				break;

			case SYS_TOSTR:
				sys_tostr();
				break;

			case SYS_TOSTRF:
				sys_tostrf();
				break;
			case SYS_PAUSE:
				Sys_pause();
				break;
			}
		}

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
			int v = top().v_->ToInt(); pop();
			char str[16];
			sprintf_s(str, "%d", v);
			push(std::string(str));			// 戻り値はスタックに入れる
		}
		// 組み込み関数(数値を文字列に変換)
		void sys_tostrf()
		{
			auto v = top().v_->ToText(); pop();
			push(v);			// 戻り値はスタックに入れる
		}

	private:
		int Value() { int v = *(int*)command_ptr_; command_ptr_ += 4; return v; }
		float Value_Float() { float v = *(float*)command_ptr_; command_ptr_ += 4; return v; }
		int addr() const { return (int)(command_ptr_ - command_); }
		void jmp(int addr) { command_ptr_ = command_ + addr; }
		void push(int v) { 
			Stack.push(ButiVM::Value(v));
		}
		void push(float v) { Stack.push(ButiVM::Value(v)); }
		void push(const std::string& v) { Stack.push(ButiVM::Value(v)); }
		void push(const ButiVM::Value& v) { 
			Stack.push(v); 
		}
		void pop() { Stack.pop(); }
		const ButiVM::Value& top() const { return Stack.top(); }
		ButiVM::Value& top() { return Stack.top(); }
		std::string text(const ButiVM::Value& v) { return v.v_->ToText(); }
		const ButiVM::Value& ref_to_value(int addr) const
		{
			if (addr & global_flag)
				return global_value[addr & global_mask];
			return Stack[addr];
		}
		void set_ref(int addr, const ButiVM::Value& v)
		{
			if (addr & global_flag)
				PopLocal(addr-1);
			else
				Stack[addr] = v;
		}

	private:
		Data& data_;
		unsigned char* command_;
		unsigned char* command_ptr_;
		int command_size_;
		char* text_buffer_;
		int text_size_;

		ButiVM::Stack<ButiVM::Value, STACK_SIZE> Stack;
		std::vector<ButiVM::Value> global_value;
		int stack_base;
	};

}

#endif
