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
		unsigned char* commandTable;	// �R�}���h�e�[�u��
		char* textBuffer;			// �e�L�X�g�f�[�^
		int commandSize;			// �R�}���h�T�C�Y
		int textSize;				// �e�L�X�g�T�C�Y
		int valueSize;			// �O���[�o���ϐ��T�C�Y
		int entryPoint;			// �G���g���[�|�C���g

		std::vector<OperationFunction> vec_sysCalls;
		std::vector<SystemTypeDefine> vec_types;
		int definedTypeCount = 0;
	};


	// 0���Z
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
		void PushConstInt(const int arg_val)
		{
			push(arg_val);
		}

		void PushConstInt()
		{
			PushConstInt(Value_Int());
		}

		// �萔Push
		void PushConstFloat(const float arg_val)
		{
			push(arg_val);
		}


		void PushConstFloat()
		{
			PushConstFloat(Value_Float());
		}

		// �����萔Push
		void PushString(const int arg_val)
		{
			push(std::string(textBuffer + arg_val));
		}

		void PushString()
		{
			PushString(Value_Int());
		}

		// �ϐ�Push
		void PushValue(const int arg_val)
		{
			push(global_value[arg_val]);
		}
		void PushValue()
		{
			PushValue(Value_Int());
		}

		// ���[�J���ϐ�Push
		void PushLocal(const int arg_val)
		{
			push(Stack[arg_val + stack_base]);
		}

		void PushLocal()
		{
			PushLocal(Value_Int());
		}

		// �z�񂩂�Push
		void PushArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(global_value[(int)(arg_val + index)]);
		}

		void PushArray()
		{
			PushArray(Value_Int());
		}

		// ���[�J���̔z�񂩂�Push
		void PushLocalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(Stack[arg_val + stack_base + index]);
		}

		void PushLocalArray()
		{
			PushLocalArray(Value_Int());
		}

		// ���[�J���ϐ�(�Q��)Push
		void PushLocalRef(const int arg_val)
		{
			int addr = Stack[arg_val + stack_base].v_->Get<int>();
			push(ref_to_value(addr));
		}
		void PushLocalRef()
		{
			PushLocalRef(Value_Int());
		}

		// ���[�J���̔z��(�Q��)����Push
		void PushLocalArrayRef(const int arg_val)
		{
			int addr = Stack[arg_val + stack_base].v_->Get<int>();
			int index = top().v_->Get<int>(); pop();
			push(ref_to_value(addr + index));
		}
		void PushLocalArrayRef(){
			PushLocalArrayRef(Value_Int());
		}

		// �A�h���X��Push
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

		// �z��̃A�h���X��Push
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
		// �ϐ���Pop
		void PopValue(const int arg_val)
		{
			global_value[arg_val] = top(); pop();
		}
		void PopValue() {
			PopValue(Value_Int());
		}
		// ���[�J���ϐ���Pop
		void PopLocal(const int arg_val)
		{
			Stack[arg_val + stack_base] = top(); pop();
		}
		void PopLocal() {
			PopLocal(Value_Int());
		}

		// �z��ϐ���Pop
		void PopArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			global_value[(int)(arg_val + index)] = top(); pop();
		}
		void PopArray() {
			PopArray(Value_Int());
		}

		// ���[�J���̔z��ϐ���Pop
		void PopLocalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			Stack[arg_val + stack_base + index] = top(); pop();
		}

		void PopLocalArray() {
			PopLocalArray(Value_Int());
		}

		// ���[�J���ϐ�(�Q��)��Pop
		void PopLocalRef(const int arg_val)
		{
			int addr = Stack[arg_val + stack_base].v_->Get<int>();
			set_ref(addr, top()); pop();
		}
		void PopLocalRef() {
			PopLocalRef(Value_Int());
		}
		// ���[�J���̔z��ϐ�(�Q��)��Pop
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

		// ���[�J���ϐ����m��
		void OpAllocStack(const int arg_val)
		{
			(this->*p_pushValues[arg_val])();
			
		}
		void OpAllocStack()
		{
			OpAllocStack(Value_Int());
		}

		// ��Pop�i�X�^�b�N�g�b�v���̂Ă�j
		void OpPop()
		{
			pop();
		}

		// �P���}�C�i�X
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
		void OpJmp() {
			OpJmp(Value_Int());
		}

		// �^�̎��W�����v
		void OpJmpC(const int arg_val)
		{
			int cond = top().v_->Get<int>(); pop();
			if (cond)
				jmp(arg_val);
		}

		void OpJmpC() {
			OpJmpC(Value_Int());
		}

		// �U�̎��W�����v
		void OpJmpNC(const int arg_val)
		{
			int cond = top().v_->Get<int>(); pop();
			if (!cond)
				jmp(arg_val);
		}

		void OpJmpNC() {
			OpJmpNC(Value_Int());
		}

		// switch���p���ꔻ��
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

		// �֐��R�[��
		void OpCall(const int arg_val)
		{
			push(stack_base);
			push(addr());					// ���^�[���A�h���X��Push
			stack_base = Stack.size();		// �X�^�b�N�x�[�X�X�V
			jmp(arg_val);
		}

		void OpCall() {
			OpCall(Value_Int());
		}

		// �����Ȃ����^�[��
		void OpReturn()
		{
			Stack.resize(stack_base);		// ���[�J���ϐ��r��
			int addr = top().v_->Get<int>(); pop();
			stack_base = top().v_->Get<int>(); pop();
			int arg_count = top().v_->Get<int>(); pop();
			Stack.pop(arg_count);
			jmp(addr);
		}

		// �����t�����^�[��
		void OpReturnV()
		{
			ButiScript::Value result = top(); pop();
			Stack.resize(stack_base);		// ���[�J���ϐ��r��
			int addr = top().v_->Get<int>(); pop();
			stack_base = top().v_->Get<int>(); pop();
			int arg_count = top().v_->Get<int>(); pop();
			Stack.pop(arg_count);
			push(result);
			jmp(addr);
		}

		// ���zCPU��~
		void OpHalt()
		{
		}

		// �g�ݍ��݊֐�
		void OpSysCall(const int val)
		{
			pop();	// arg_count

			(this->*p_syscall[val])();

		}

		void OpSysCall() {
			OpSysCall(Value_Int());
		}
	public:
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
			auto v = top().v_->Get<std::string>(); pop();
			push(v);			// �߂�l�̓X�^�b�N�ɓ����
		}
		// �g�ݍ��݊֐�(���l�𕶎���ɕϊ�)
		void sys_tostrf()
		{
			auto v = top().v_->Get<std::string>(); pop();
			push(v);			// �߂�l�̓X�^�b�N�ɓ����
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

		//�R�}���h����
		unsigned char* commandTable;
		//���ݎQ�Ƃ��Ă�R�}���h�̈ʒu
		unsigned char* command_ptr_;
		//�O���[�o���ϐ��̊m�ۃR�}���h
		unsigned char* allocCommand_ptr_;
		//�v���O�����S�̂̃T�C�Y
		int commandSize;
		//������f�[�^
		char* textBuffer;
		//������f�[�^�̃T�C�Y
		int textSize;

		//���߃e�[�u��
		OperationFunction* p_op=nullptr;
		//�g�ݍ��݊֐��e�[�u��
		OperationFunction* p_syscall=nullptr;
		//�ϐ��̊m�ۊ֐��e�[�u��
		OperationFunction* p_pushValues = nullptr;


		ButiScript::Stack<ButiScript::Value, STACK_SIZE> Stack;
		//�O���[�o���ϐ�
		std::vector<ButiScript::Value> global_value;
		//�X�^�b�N�̎Q�ƈʒu
		int stack_base;

		
	};

}

#endif
