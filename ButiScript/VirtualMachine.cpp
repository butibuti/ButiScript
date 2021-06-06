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

	global_value.resize(data_.valueSize);			// 外部変数テーブル確保
	command_ptr_ = commandTable + data_.entryPoint;	// プログラムカウンター初期化

	p_op = (OperationFunction*)malloc(sizeof(OperationFunction) * VM_MAXCOMMAND);
#include "VM_table.h"

	p_syscall=(OperationFunction*)malloc(sizeof(OperationFunction) * data_.vec_sysCalls.size());
	for (int i = 0; i < data_.vec_sysCalls.size(); i++) {
		p_syscall[i] = data_.vec_sysCalls[i];
	}
}
