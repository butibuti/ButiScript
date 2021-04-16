#include "stdafx.h"
#include <exception>
#include "VirtualMachine.h"

// 実行
int ButiVM::VirtualCPU::Run()
{

	// mainをcall
	push(0);										// mainへの引数カウントをpush
	push(0);										// stack_baseの初期値をpush
	push(0);										// プログラム終了位置をpush
	stack_base = Stack.size();						// スタック参照位置初期化

	try {
		int Op;
		while ((Op = *command_ptr_++) != VM_HALT) {	// Haltするまでループ
			switch (Op) {
#define	VM_SWITCHTABLE
#include "VM_switch.h"
#undef	VM_SWITCHTABLE
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << "例外発生（" << e.what() << "）" << std::endl;
		return -1;
	}
	return top().v_->ToInt();								// main関数戻り値
}

void ButiVM::VirtualCPU::Initialize()
{
	command_ = data_.command_;						// プログラム格納位置
	text_buffer_ = data_.text_buffer_;				// テキストデータ格納位置
	command_size_ = data_.command_size_;			// プログラムの大きさ
	text_size_ = data_.text_size_;					// データの大きさ

	global_value.resize(data_.value_size_);			// 外部変数テーブル確保
	command_ptr_ = command_ + data_.entry_point_;	// プログラムカウンター初期化
}
