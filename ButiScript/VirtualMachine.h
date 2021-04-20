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
		unsigned char* command_;	// �R�}���h�e�[�u��
		char* text_buffer_;			// �e�L�X�g�f�[�^
		int command_size_;			// �R�}���h�T�C�Y
		int text_size_;				// �e�L�X�g�T�C�Y
		int value_size_;			// �O���[�o���ϐ��T�C�Y
		int entry_point_;			// �G���g���[�|�C���g
	};


	// 0���Z��O
	class DevideByZero : public std::exception {
	public:
		const char* what() const throw()
		{
			return "devide by zero";
		}
	};

	// ���z�}�V��
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
		// �萔Push
		void PushConst(const int arg_val)
		{
			push(arg_val);
		}
		// �萔Push
		void PushConstFloat(const float arg_val)
		{
			push(arg_val);
		}

		// �����萔Push
		void PushString(const int arg_val)
		{
			push(std::string(text_buffer_ + arg_val));
		}

		// �ϐ�Push
		void PushValue(const int arg_val)
		{
			push(global_value[arg_val]);
		}

		// ���[�J���ϐ�Push
		void PushLocal(const int arg_val)
		{
			push(Stack[arg_val + stack_base]);
		}

		// �z�񂩂�Push
		void PushArray(const int arg_val)
		{
			int index = *top().v_->GetIntPtr(); pop();
			push(global_value[arg_val + index]);
		}

		// ���[�J���̔z�񂩂�Push
		void PushLocalArray(const int arg_val)
		{
			int index = *top().v_->GetIntPtr(); pop();
			push(Stack[arg_val + stack_base + index]);
		}

		// ���[�J���ϐ�(�Q��)Push
		void PushLocalRef(const int arg_val)
		{
			int addr =* Stack[arg_val + stack_base].v_->GetIntPtr();
			push(ref_to_value(addr));
		}

		// ���[�J���̔z��(�Q��)����Push
		void PushLocalArrayRef(const int arg_val)
		{
			int addr =* Stack[arg_val + stack_base].v_->GetIntPtr();
			int index = *top().v_->GetIntPtr(); pop();
			push(ref_to_value(addr + index));
		}

		// �A�h���X��Push
		void PushAddr(const int arg_val)
		{
			int base = arg_val;
			if ((arg_val & global_flag) == 0)	// local
				base += +stack_base;
			push(base);
		}

		// �z��̃A�h���X��Push
		void PushArrayAddr(const int arg_val)
		{
			int base = arg_val;
			if ((arg_val & global_flag) == 0)	// local
				base += +stack_base;
			int index = *top().v_->GetIntPtr(); pop();
			push(base + index);
		}

		// �ϐ���Pop
		void PopValue(const int arg_val)
		{
			global_value[arg_val] = top(); pop();
		}

		// ���[�J���ϐ���Pop
		void PopLocal(const int arg_val)
		{
			Stack[arg_val + stack_base] = top(); pop();
		}

		// �z��ϐ���Pop
		void PopArray(const int arg_val)
		{
			int index = *top().v_->GetIntPtr(); pop();
			global_value[arg_val + index] = top(); pop();
		}

		// ���[�J���̔z��ϐ���Pop
		void PopLocalArray(const int arg_val)
		{
			int index = *top().v_->GetIntPtr(); pop();
			Stack[arg_val + stack_base + index] = top(); pop();
		}

		// ���[�J���ϐ�(�Q��)��Pop
		void PopLocalRef(const int arg_val)
		{
			int addr =* Stack[arg_val + stack_base].v_->GetIntPtr();
			set_ref(addr, top()); pop();
		}

		// ���[�J���̔z��ϐ�(�Q��)��Pop
		void PopLocalArrayRef(const int arg_val)
		{
			int addr =* Stack[arg_val + stack_base].v_->GetIntPtr();
			int index = *top().v_->GetIntPtr(); pop();
			set_ref(addr + index, top()); pop();
		}

		// ���[�J���ϐ����m��
		void OpAllocStack(const int arg_val)
		{
			Stack.resize(stack_base + arg_val);
		}

		// ��Pop�i�X�^�b�N�g�b�v���̂Ă�j
		void OpPop()
		{
			pop();
		}

		// �P���}�C�i�X
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

		// �������==
		void OpStrEq()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs == rhs);
		}

		// �������!=
		void OpStrNe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs != rhs);
		}

		// �������>
		void OpStrGt()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs > rhs);
		}

		// �������>=
		void OpStrGe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs >= rhs);
		}

		// �������<
		void OpStrLt()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs < rhs);
		}

		// �������<=
		void OpStrLe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs <= rhs);
		}

		// �������+
		void OpStrAdd()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs + rhs);
		}

		// �������W�����v
		void OpJmp(const int arg_val)
		{
			jmp(arg_val);
		}

		// �^�̎��W�����v
		void OpJmpC(const int arg_val)
		{
			int cond = *top().v_->GetIntPtr(); pop();
			if (cond)
				jmp(arg_val);
		}

		// �U�̎��W�����v
		void OpJmpNC(const int arg_val)
		{
			int cond = *top().v_->GetIntPtr(); pop();
			if (!cond)
				jmp(arg_val);
		}

		// switch���p���ꔻ��
		void OpTest(const int arg_val)
		{
			int Value = *top().v_->GetIntPtr(); pop();
			if (Value == *top().v_->GetIntPtr()) {
				pop();
				jmp(arg_val);
			}
		}

		// �֐��R�[��
		void OpCall(const int arg_val)
		{
			push(stack_base);
			push(addr());					// ���^�[���A�h���X��Push
			stack_base = Stack.size();		// �X�^�b�N�x�[�X�X�V
			jmp(arg_val);
		}

		// �����Ȃ����^�[��
		void OpReturn()
		{
			Stack.resize(stack_base);		// ���[�J���ϐ��r��
			int addr = *top().v_->GetIntPtr(); pop();
			stack_base = *top().v_->GetIntPtr(); pop();
			int arg_count = *top().v_->GetIntPtr(); pop();
			Stack.pop(arg_count);
			jmp(addr);
		}

		// �����t�����^�[��
		void OpReturnV()
		{
			ButiVM::Value result = top(); pop();
			Stack.resize(stack_base);		// ���[�J���ϐ��r��
			int addr = *top().v_->GetIntPtr(); pop();
			stack_base = *top().v_->GetIntPtr(); pop();
			int arg_count = *top().v_->GetIntPtr(); pop();
			Stack.pop(arg_count);
			push(result);
			jmp(addr);
		}

		// ���zCPU�v���O������~
		void OpHalt()
		{
		}

		// �g�ݍ��݊֐�
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

		// �g�ݍ��݊֐��iprint�j
		void sys_print()
		{
			std::cout << text(top());
			pop();
		}

		// �g�ݍ��݊֐��ipause�j
		void Sys_pause() {
			std::system("pause");
		}

		// �g�ݍ��݊֐�(���l�𕶎���ɕϊ�)
		void sys_tostr()
		{
			int v = top().v_->ToInt(); pop();
			char str[16];
			sprintf_s(str, "%d", v);
			push(std::string(str));			// �߂�l�̓X�^�b�N�ɓ����
		}
		// �g�ݍ��݊֐�(���l�𕶎���ɕϊ�)
		void sys_tostrf()
		{
			auto v = top().v_->ToText(); pop();
			push(v);			// �߂�l�̓X�^�b�N�ɓ����
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
