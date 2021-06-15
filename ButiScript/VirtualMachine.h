#pragma once
#ifndef	__VM_H__
#define	__VM_H__

#include <vector>

#include "vm_value.h"
#include"Tags.h"
namespace ButiScript {

#include"value_type.h"

#define	VM_ENUMDEF
	enum {
#include "VM_enum.h"
		VM_MAXCOMMAND,
	};
#undef	VM_ENUMDEF

	

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
		std::vector<OperationFunction> vec_sysCallMethods;
		std::vector<TypeTag> vec_types;
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
		/////////////�萔Push��`////////////////
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



		/////////////�ϐ�Push��`////////////////
		// �O���[�o���ϐ��̃R�s�[��Push
		void PushGlobalValue(const int arg_val)
		{
			push(Stack[globalValue_base + arg_val].Clone());
		}
		void PushGlobalValue()
		{
			PushGlobalValue(Value_Int());
		}

		// ���[�J���ϐ��̃R�s�[��Push
		void PushLocal(const int arg_val)
		{
			push(Stack[arg_val + stack_base].Clone());
		}

		void PushLocal()
		{
			PushLocal(Value_Int());
		}

		// �z�񂩂�R�s�[��Push
		void PushGlobalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(Stack[(int)(arg_val + index)].Clone());
		}

		void PushGlobalArray()
		{
			PushGlobalArray(Value_Int());
		}

		// ���[�J���̔z�񂩂�R�s�[��Push
		void PushLocalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(Stack[arg_val + stack_base + index].Clone());
		}

		void PushLocalArray()
		{
			PushLocalArray(Value_Int());
		}


		//�O���[�o���ϐ��̃����o�ϐ��̃R�s�[��push
		void PushGlobalMember(const int arg_val, const int arg_valueIndex) {
			pop(); push_clone(Stack[arg_val + globalValue_base].v_->GetMember(arg_valueIndex), Stack[arg_val + globalValue_base].v_->GetMemberType(arg_valueIndex));
		}

		void PushGlobalMember() {
			PushGlobalMember(top().v_->Get<int>(), Value_Int());
		}

		//���[�J���ϐ��̃����o�ϐ��̃R�s�[��push
		void PushLocalMember(const int arg_val, const int arg_valueIndex) {
			pop(); push_clone(Stack[arg_val + stack_base].v_->GetMember(arg_valueIndex), Stack[arg_val + stack_base].v_->GetMemberType(arg_valueIndex));
		}

		void PushLocalMember() {
			PushLocalMember(top().v_->Get<int>(), Value_Int());
		}

		//�O���[�o���z��̃����o�ϐ��̃R�s�[��push
		void PushGlobalArrayMember(const int arg_val, const int arg_valueIndex) {
			pop();
			int index = top().v_->Get<int>(); pop();
			push_clone(Stack[arg_val+ index + globalValue_base].v_->GetMember(arg_valueIndex), Stack[arg_val + globalValue_base].v_->GetMemberType(arg_valueIndex));
		}

		void PushGlobalArrayMember() {
			PushGlobalArrayMember(top().v_->Get<int>(), Value_Int());
		}

		//���[�J���z��̃����o�ϐ��̃R�s�[��push
		void PushLocalArrayMember(const int arg_val, const int arg_valueIndex) {
			pop();
			int index = top().v_->Get<int>(); pop();
			push_clone(Stack[arg_val+ index + stack_base].v_->GetMember(arg_valueIndex), Stack[arg_val + stack_base].v_->GetMemberType(arg_valueIndex));
		}

		void PushLocalArrayMember() {
			PushLocalArrayMember(top().v_->Get<int>(), Value_Int());
		}

		/////////////�O���[�o���ϐ��̎Q��Push��`////////////////

		// �O���[�o���ϐ��̎Q�Ƃ�Push
		void PushGlobalValueRef(const int arg_val)
		{
			push(Stack[globalValue_base + arg_val]);
		}
		void PushGlobalValueRef()
		{
			PushGlobalValueRef(Value_Int());
		}

		// ���[�J���ϐ��̎Q�Ƃ�Push
		void PushLocalRef(const int arg_val)
		{
			push(Stack[arg_val + stack_base]);
		}

		void PushLocalRef()
		{
			PushLocalRef(Value_Int());
		}

		// �z�񂩂�Q�Ƃ�Push
		void PushGlobalArrayRef(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(Stack[(int)(arg_val + index)]);
		}

		void PushGlobalArrayRef()
		{
			PushGlobalArrayRef(Value_Int());
		}

		// ���[�J���̔z�񂩂�Q�Ƃ�Push
		void PushLocalArrayRef(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(Stack[arg_val + stack_base + index]);
		}

		void PushLocalArrayRef()
		{
			PushLocalArrayRef(Value_Int());
		}


		//�O���[�o���ϐ��̃����o�ϐ��̎Q�Ƃ�push
		void PushGlobalMemberRef(const int arg_val, const int arg_valueIndex) {
			pop(); push(Stack[arg_val + globalValue_base].v_->GetMember(arg_valueIndex), Stack[arg_val + globalValue_base].v_->GetMemberType(arg_valueIndex));
		}

		void PushGlobalMemberRef() {
			PushGlobalMemberRef(top().v_->Get<int>(), Value_Int());
		}

		//���[�J���ϐ��̃����o�ϐ��̎Q�Ƃ�push
		void PushLocalMemberRef(const int arg_val, const int arg_valueIndex) {
			pop(); push(Stack[arg_val + stack_base].v_->GetMember(arg_valueIndex), Stack[arg_val + stack_base].v_->GetMemberType(arg_valueIndex));
		}

		void PushLocalMemberRef() {
			PushLocalMemberRef(top().v_->Get<int>(), Value_Int());
		}

		//�O���[�o���ϐ��̔z��̃����o�ϐ��̎Q�Ƃ�push
		void PushGlobalArrayMemberRef(const int arg_val, const int arg_valueIndex) {
			pop();
			int index = top().v_->Get<int>(); pop(); 
			push(Stack[arg_val + globalValue_base+index].v_->GetMember(arg_valueIndex), Stack[arg_val + globalValue_base+index].v_->GetMemberType(arg_valueIndex));
		}

		void PushGlobalArrayMemberRef() {
			PushGlobalArrayMemberRef(top().v_->Get<int>(), Value_Int());
		}

		//���[�J���ϐ��̔z��̃����o�ϐ��̎Q�Ƃ�push
		void PushLocalArrayMemberRef(const int arg_val, const int arg_valueIndex) {
			pop();
			int index = top().v_->Get<int>(); pop(); 
			push(Stack[arg_val + stack_base+index].v_->GetMember(arg_valueIndex), Stack[arg_val + stack_base+index].v_->GetMemberType(arg_valueIndex));
		}

		void PushLocalArrayMemberRef() {
			PushLocalArrayMemberRef(top().v_->Get<int>(), Value_Int());
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


		/////////////Pop��`////////////////
		// �ϐ���Pop
		void PopValue(const int arg_val)
		{
			Stack[globalValue_base+ arg_val] = top(); pop();
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

		// �O���[�o���ϐ��̃����o�ϐ���Pop
		void PopGlobalMember(const int arg_val, const int arg_index)
		{
			pop(); Stack[arg_val + globalValue_base].v_->GetMember(arg_index)->Set(*top().v_); pop();
		}
		void PopGlobalMember() {
			PopGlobalMember(top().v_->Get<int>(), Value_Int());
		}
		// ���[�J���ϐ��̃����o�ϐ���Pop
		void PopLocalMember(const int arg_val, const int arg_index)
		{
			pop(); Stack[arg_val + stack_base].v_->GetMember(arg_index)->Set(*top().v_); pop();
		}
		void PopLocalMember() {
			PopLocalMember(top().v_->Get<int>(), Value_Int());
		}
		// �O���[�o���ϐ��z��̃����o�ϐ���Pop
		void PopGlobalArrayMember(const int arg_val, const int arg_index)
		{
			pop();
			int index = top().v_->Get<int>(); pop();
			Stack[index+ arg_val + globalValue_base].v_->GetMember(arg_index)->Set(*top().v_); pop();
		}
		void PopGlobalArrayMember() {
			PopGlobalArrayMember(top().v_->Get<int>(), Value_Int());
		}
		// ���[�J���ϐ��z��̃����o�ϐ���Pop
		void PopLocalArrayMember(const int arg_val, const int arg_index)
		{
			pop();
			int index = top().v_->Get<int>(); pop();
			Stack[index + arg_val + stack_base].v_->GetMember(arg_index)->Set(*top().v_); pop();
		}
		void PopLocalArrayMember() {
			PopLocalArrayMember(top().v_->Get<int>(), Value_Int());
		}

		// �z��ϐ���Pop
		void PopArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			Stack[(int)(arg_val + index)] = top(); pop();
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


		// ��Pop�i�X�^�b�N�g�b�v���̂Ă�j
		void OpPop()
		{
			pop();
		}

		/////////////Alloc��`////////////////
		// ���[�J���ϐ����m��
		void OpAllocStack(const int arg_val)
		{
			(this->*p_pushValues[arg_val])();

		}
		void OpAllocStack()
		{
			OpAllocStack(Value_Int());
		}

		// ���[�J���ϐ�(�Q�ƌ^)���m��
		void OpAllocStack_Ref(const int arg_val)
		{
			(this->*p_pushRefValues[arg_val& ~TYPE_REF])();

		}
		void OpAllocStack_Ref()
		{
			OpAllocStack_Ref(Value_Int());
		}



		/////////////���Z�q��`////////////////

		// �P���}�C�i�X
		void OpNeg()
		{
			auto negV = top().v_->Clone();
			auto type = top().valueType;
			negV->Nagative();
			pop();
			push(negV,type);
		}

		// ==
		void OpEq()
		{

			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(lhs->Eq(rhs));

			rhs->release();
			lhs->release();

		}

		// !=
		void OpNe()
		{
			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(!lhs->Eq(rhs));

			rhs->release();
			lhs->release();
		}

		// >
		void OpGt()
		{
			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(lhs->Gt(rhs));

			rhs->release();
			lhs->release();
		}

		// >=
		void OpGe()
		{
			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(lhs->Ge(rhs));

			rhs->release();
			lhs->release();
		}

		// <
		void OpLt()
		{
			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(!lhs->Ge(rhs));

			rhs->release();
			lhs->release();
		}

		// <=
		void OpLe()
		{

			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(!lhs->Gt(rhs));

			rhs->release();
			lhs->release();

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
		template<typename T>
		void OpAdd()
		{
			auto rhs = top().v_->Get<T>(); pop();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs + rhs);
		}

		// -
		template<typename T>
		void OpSub()
		{
			auto rhs = top().v_->Get<T>(); pop();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs - rhs);
		}

		// *
		template<typename T>
		void OpMul()
		{
			auto rhs = top().v_->Get<T>(); pop();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs * rhs);
		}

		// /
		template<typename T>
		void OpDiv()
		{
			auto rhs = top().v_->Get<T>(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs / rhs);
		}

		// %
		template<typename T>
		void OpMod()
		{
			int rhs = top().v_->Get<int>(); pop();
			if (rhs == 0)
				throw DevideByZero();
			int lhs = top().v_->Get<int>(); pop();
			push(lhs % rhs);
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




		/////////////�A�h���X�����`////////////////
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

		/////////////�֐��Ăяo����`////////////////
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

		//�g�ݍ��݃��\�b�h
		void OpSysMethodCall(const int val) 
		{
			pop();	// arg_count
			(this->*p_sysMethodCall[val])();
		}
		void OpSysMethodCall() {
			OpSysMethodCall(Value_Int());
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

		//�g�ݍ��݃��\�b�h(return ����)
		template<typename T,void(T::*Method)() >
		void sys_method_retNo() 
		{
			auto v = &(top().v_->GetRef<T>());
			((v)->*Method)();
			pop();
		}

		template<typename T,int typeIndex>
		void pushValue() {
			auto value = Value(T());
			value.SetType(typeIndex);
			Stack.push(value);
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
		void push(IValue* arg_p_ivalue,const int arg_type) {
			Stack.push(ButiScript::Value(arg_p_ivalue, arg_type));
		}
		void push_clone(IValue* arg_p_ivalue,const int arg_type) {
			auto cloneV = arg_p_ivalue->Clone();
			Stack.push(ButiScript::Value(cloneV, arg_type));
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
			if (addr & global_flag) {
				return Stack[addr & global_mask];
			}
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
		OperationFunction* p_syscall = nullptr;
		//�g�ݍ��݃��\�b�h�e�[�u��
		OperationFunction* p_sysMethodCall = nullptr;
		//�ϐ��̊m�ۊ֐��e�[�u��
		OperationFunction* p_pushValues = nullptr;
		//�ϐ�(�Q�ƌ^)�̊m�ۊ֐��e�[�u��
		OperationFunction* p_pushRefValues = nullptr;


		ButiScript::Stack<ButiScript::Value, STACK_SIZE> Stack;

		//�X�^�b�N�̎Q�ƈʒu
		int stack_base=0;
		int globalValue_base=0;
		
	};

}

#endif
