#include "stdafx.h"
#include <exception>
#include "VirtualMachine.h"

// ���s
int ButiScript::VirtualCPU::Run()
{
	// main��call
	push(0);										// main�ւ̈����J�E���g��push
	push(0);										// stack_base�̏����l��push
	push(0);										// �v���O�����I���ʒu��push
	stack_base = Stack.size();						// �X�^�b�N�Q�ƈʒu������

	globalValue_base= stack_base;

	//�O���[�o���ϐ��̊m��
	{

		auto buff = command_ptr_;
		command_ptr_ = allocCommand_ptr_;
		int Op;
		while ((Op = *command_ptr_++) != VM_HALT) {	// Halt����܂Ń��[�v

			(this->*p_op[Op])();
		}
		command_ptr_ = buff;
	}

	//main����J�n
	try {
		int Op;
		while ((Op = *command_ptr_++) != VM_HALT) {	// Halt����܂Ń��[�v

			(this->*p_op[Op])();
		}
	}
	catch (const std::exception& e) {
		std::cerr << "��O�����i" << e.what() << "�j" << std::endl;
		return -1;
	}
	return top().v_->Get<int>();
}

void ButiScript::VirtualCPU::Initialize()
{
	commandTable = data_.commandTable;						// �v���O�����i�[�ʒu
	textBuffer = data_.textBuffer;				// �e�L�X�g�f�[�^�i�[�ʒu
	commandSize = data_.commandSize;			// �v���O�����̑傫��
	textSize = data_.textSize;					// �f�[�^�̑傫��

	command_ptr_ = commandTable + data_.entryPoint;	// �v���O�����J�E���^�[������
	allocCommand_ptr_ = commandTable +1;
	p_op = (OperationFunction*)malloc(sizeof(OperationFunction) * VM_MAXCOMMAND);
#include "VM_table.h"

	p_syscall=(OperationFunction*)malloc(sizeof(OperationFunction) * data_.vec_sysCalls.size());
	for (int i = 0; i < data_.vec_sysCalls.size(); i++) {
		p_syscall[i] = data_.vec_sysCalls[i];
	}

	p_sysMethodCall= (OperationFunction*)malloc(sizeof(OperationFunction) * data_.vec_sysCallMethods.size());
	for (int i = 0; i < data_.vec_sysCallMethods.size(); i++) {
		p_sysMethodCall[i] = data_.vec_sysCallMethods[i];
	}


	p_pushValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (data_.vec_types.size() + data_.definedTypeCount));
	p_pushRefValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (data_.vec_types.size() +data_.definedTypeCount));
	for (int i = 0; i < data_.vec_types.size(); i++) {

		p_pushValues[data_.vec_types.at(i).typeIndex] = data_.vec_types.at(i).typeFunc;
		p_pushRefValues[data_.vec_types.at(i).typeIndex] = data_.vec_types.at(i).refTypeFunc;
	}



}
