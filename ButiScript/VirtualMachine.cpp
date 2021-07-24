#include "stdafx.h"
#include <exception>
#include "VirtualMachine.h"


void ButiScript::VirtualCPU::AllocGlobalValue()
{
	stack_base = valueStack.size();						// �X�^�b�N�Q�ƈʒu������
	globalValue_base = stack_base;
	stack_base = valueStack.size();
	//�O���[�o���ϐ��̊m��
	{

		auto buff = command_ptr_;
		command_ptr_ = allocCommand_ptr_;
		int Op;
		stack_base = valueStack.size();
		while ((Op = *command_ptr_++) != VM_HALT) {	// Halt����܂Ń��[�v

			(this->*p_op[Op])();
		}
		command_ptr_ = buff;
	}
	globalValue_size = valueStack.size();
}

void ButiScript::VirtualCPU::Initialize()
{
	commandTable = data_->commandTable;						// �v���O�����i�[�ʒu
	textBuffer = data_->textBuffer;				// �e�L�X�g�f�[�^�i�[�ʒu
	commandSize = data_->commandSize;			// �v���O�����̑傫��
	textSize = data_->textSize;					// �f�[�^�̑傫��


	allocCommand_ptr_ = commandTable +1;
	p_op = (OperationFunction*)malloc(sizeof(OperationFunction) * VM_MAXCOMMAND);
#include "VM_table.h"

	p_syscall=(OperationFunction*)malloc(sizeof(OperationFunction) * data_->vec_sysCalls.size());
	for (int i = 0; i < data_->vec_sysCalls.size(); i++) {
		p_syscall[i] = data_->vec_sysCalls[i];
	}

	p_sysMethodCall= (OperationFunction*)malloc(sizeof(OperationFunction) * data_->vec_sysCallMethods.size());
	for (int i = 0; i < data_->vec_sysCallMethods.size(); i++) {
		p_sysMethodCall[i] = data_->vec_sysCallMethods[i];
	}


	p_pushValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (data_->vec_types.size() ));
	p_pushRefValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (data_->vec_types.size() ));
	for (int i = 0; i < data_->vec_types.size(); i++) {

		p_pushValues[data_->vec_types.at(i).typeIndex] = data_->vec_types.at(i).typeFunc;
		p_pushRefValues[data_->vec_types.at(i).typeIndex] = data_->vec_types.at(i).refTypeFunc;
	}

	vec_scriptClassInfo = data_->vec_scriptClassInfo;

}

void ButiScript::VirtualCPU::Execute_(const std::string& entryPoint)
{
	command_ptr_ = commandTable + data_->map_entryPoints[entryPoint];
	stack_base = valueStack.size();

	int Op;
	//main����J�n
	try {
		while ((Op = *command_ptr_++) != VM_HALT) {	// Halt����܂Ń��[�v

			(this->*p_op[Op])();
		}
	}
	catch (const std::exception& e) {
		std::cerr << "��O�����i" << e.what() << "�j" << std::endl;
		return ;
	}

	command_ptr_ = commandTable + data_->map_entryPoints[entryPoint];	// �v���O�����J�E���^�[������
}


#ifdef IMPL_BUTIENGINE

void ButiScript::VirtualCPU::SaveGlobalValue(std::vector<std::shared_ptr<ButiScript::IGlobalValueSaveObject>>& arg_ref_vec_saveObject) {
	for (int i = 0; i < globalValue_size- globalValue_base; i++) {
		arg_ref_vec_saveObject.push_back(valueStack[globalValue_base+i].v_->GetSaveObject());
		arg_ref_vec_saveObject.at(i)->SetTypeIndex(valueStack[globalValue_base + i].valueType);
	}
}
void ButiScript::VirtualCPU::RestoreGlobalValue(std::vector<std::shared_ptr< ButiScript::IGlobalValueSaveObject>>& arg_ref_vec_saveObject) {
	if (globalValue_size - globalValue_base != arg_ref_vec_saveObject.size()) {
		ButiEngine::GUI::Console("�ۑ�����Ă���O���[�o���ϐ��̒l�ƃX�N���v�g�Œ�`����Ă���O���[�o���ϐ��̐����قȂ�܂�"); 
		
		return;
	}
	for (int i = 0; i < globalValue_size - globalValue_base; i++) {
		if (valueStack[globalValue_base + i].v_) {
			valueStack[globalValue_base + i].v_->release();
		}
		arg_ref_vec_saveObject.at(i)->RestoreValue(&valueStack[globalValue_base + i].v_);
		valueStack[globalValue_base + i].valueType = arg_ref_vec_saveObject.at(i)->GetTypeIndex();
	}
}
void ButiScript::VirtualCPU::ShowGUI() {
	auto end = data_->map_globalValueAddress.end();
	for (auto itr = data_->map_globalValueAddress.begin(); itr != end;itr++) {
		valueStack[globalValue_base + itr->second].v_->ShowGUI(itr->first);
	}
}
#endif