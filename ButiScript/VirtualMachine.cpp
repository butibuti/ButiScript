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

	global_value.resize(data_.valueSize);			// �O���ϐ��e�[�u���m��
	command_ptr_ = commandTable + data_.entryPoint;	// �v���O�����J�E���^�[������

	p_op = (OperationFunction*)malloc(sizeof(OperationFunction) * VM_MAXCOMMAND);
#include "VM_table.h"

	p_syscall=(OperationFunction*)malloc(sizeof(OperationFunction) * data_.vec_sysCalls.size());
	for (int i = 0; i < data_.vec_sysCalls.size(); i++) {
		p_syscall[i] = data_.vec_sysCalls[i];
	}
}
