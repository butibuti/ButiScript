#include "stdafx.h"
#include <exception>
#include "VirtualMachine.h"

// 実行
int ButiScript::VirtualCPU::Run()
{
	// mainをcall
	push(0);										// mainへの引数カウントをpush
	push(0);										// stack_baseの初期値をpush
	push(0);										// プログラム終了位置をpush
	stack_base = Stack.size();						// スタック参照位置初期化

	globalValue_base= stack_base;

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

	//mainから開始
	try {
		int Op;
		while ((Op = *command_ptr_++) != VM_HALT) {	// Haltするまでループ

			(this->*p_op[Op])();
		}
	}
	catch (const std::exception& e) {
		std::cerr << "例外発生（" << e.what() << "）" << std::endl;
		return -1;
	}
	return top().v_->Get<int>();
}

void ButiScript::VirtualCPU::Initialize()
{
	commandTable = data_.commandTable;						// プログラム格納位置
	textBuffer = data_.textBuffer;				// テキストデータ格納位置
	commandSize = data_.commandSize;			// プログラムの大きさ
	textSize = data_.textSize;					// データの大きさ

	command_ptr_ = commandTable + data_.entryPoint;	// プログラムカウンター初期化
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
