#include "stdafx.h"
#include <exception>
#include "VirtualMachine.h"

// ���s
int ButiVM::VirtualCPU::Run()
{

	// main��call
	push(0);										// main�ւ̈����J�E���g��push
	push(0);										// stack_base�̏����l��push
	push(0);										// �v���O�����I���ʒu��push
	stack_base = Stack.size();						// �X�^�b�N�Q�ƈʒu������

	try {
		int Op;
		while ((Op = *command_ptr_++) != VM_HALT) {	// Halt����܂Ń��[�v
			switch (Op) {
#define	VM_SWITCHTABLE
#include "VM_switch.h"
#undef	VM_SWITCHTABLE
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << "��O�����i" << e.what() << "�j" << std::endl;
		return -1;
	}
	return top().v_->ToInt();								// main�֐��߂�l
}

void ButiVM::VirtualCPU::Initialize()
{
	command_ = data_.command_;						// �v���O�����i�[�ʒu
	text_buffer_ = data_.text_buffer_;				// �e�L�X�g�f�[�^�i�[�ʒu
	command_size_ = data_.command_size_;			// �v���O�����̑傫��
	text_size_ = data_.text_size_;					// �f�[�^�̑傫��

	global_value.resize(data_.value_size_);			// �O���ϐ��e�[�u���m��
	command_ptr_ = command_ + data_.entry_point_;	// �v���O�����J�E���^�[������
}
