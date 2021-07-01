#include "stdafx.h"
#include <exception>
#include "VirtualMachine.h"


void ButiScript::VirtualCPU::AllocGlobalValue()
{
	stack_base = Stack.size();						// スタック参照位置初期化
	globalValue_base = stack_base;
	//グローバル変数の確保
	{

		auto buff = command_ptr_;
		command_ptr_ = allocCommand_ptr_;
		int Op;
		while ((Op = *command_ptr_++) != VM_HALT) {	// Haltするまでループ

			(this->*p_op[Op])();
		}
		command_ptr_ = buff;
	}
	globalValue_size = Stack.size();
}

void ButiScript::VirtualCPU::Initialize()
{
	commandTable = data_->commandTable;						// プログラム格納位置
	textBuffer = data_->textBuffer;				// テキストデータ格納位置
	commandSize = data_->commandSize;			// プログラムの大きさ
	textSize = data_->textSize;					// データの大きさ


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


	p_pushValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (data_->vec_types.size() + data_->definedTypeCount));
	p_pushRefValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (data_->vec_types.size() +data_->definedTypeCount));
	for (int i = 0; i < data_->vec_types.size(); i++) {

		p_pushValues[data_->vec_types.at(i).typeIndex] = data_->vec_types.at(i).typeFunc;
		p_pushRefValues[data_->vec_types.at(i).typeIndex] = data_->vec_types.at(i).refTypeFunc;
	}



}

void ButiScript::VirtualCPU::Execute_(const std::string& entryPoint)
{
	command_ptr_ = commandTable + data_->map_entryPoints[entryPoint];
	stack_base = Stack.size();

	//mainから開始
	try {
		int Op;
		while ((Op = *command_ptr_++) != VM_HALT) {	// Haltするまでループ

			(this->*p_op[Op])();
		}
	}
	catch (const std::exception& e) {
		std::cerr << "例外発生（" << e.what() << "）" << std::endl;
		return ;
	}

	command_ptr_ = commandTable + data_->map_entryPoints[entryPoint];	// プログラムカウンター初期化
}
