#include "stdafx.h"
#include <exception>
#include "VirtualMachine.h"
#ifdef IMPL_BUTIENGINE
#include"ButiEventSystem/ButiEventSystem/EventSystem.h"
#include"ButiEventSystem/ButiEventSystem/TaskSystem.h"
#include<filesystem>
#endif // IMPL_BUTIENGINE


void ButiScript::VirtualCPU::AllocGlobalValue()
{
	stack_base = valueStack.size();						// スタック参照位置初期化
	globalValue_base = stack_base;
	stack_base = valueStack.size();
	//グローバル変数の確保
	{

		auto buff = command_ptr_;
		command_ptr_ = allocCommand_ptr_;
		int Op;
		stack_base = valueStack.size();
		while ((Op = *command_ptr_++) != VM_HALT) {	// Haltするまでループ

			(this->*p_op[Op])();
		}
		command_ptr_ = buff;
	}
	globalValue_size = valueStack.size();
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
	//mainから開始
	try {
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



#ifdef IMPL_BUTIENGINE

void ButiScript::VirtualCPU::sys_addEventMessanger()
{
	std::string eventName = top().v_->GetRef<std::string>(); pop();
	ButiEventSystem::AddEventMessenger<void>(eventName);
}

void ButiScript::VirtualCPU::sys_registEventListner()
{
	std::string functionName = top().v_->GetRef<std::string>(); pop();
	std::string keyName = top().v_->GetRef<std::string>(); pop();
	std::string eventName = top().v_->GetRef<std::string>(); pop();
	auto retKey= ButiEventSystem::RegistEventListner(eventName, keyName,
		std::function<void()>([this,functionName]()->void {
			this->Execute<void>(functionName);
			}), false);

	push(retKey);
}
void ButiScript::VirtualCPU::sys_unregistEventListner()
{
	std::string eventName = top().v_->GetRef<std::string>(); pop();
	std::string keyName = top().v_->GetRef<std::string>(); pop();
	ButiEventSystem::UnRegistEventListner<void>(eventName, keyName);

}
void ButiScript::VirtualCPU::sys_executeEvent()
{
	std::string eventName = top().v_->GetRef<std::string>(); pop();
	ButiEventSystem::Execute(eventName);
}

void ButiScript::VirtualCPU::sys_pushTask()
{
	std::string taskName = top().v_->GetRef<std::string>(); pop();
	ButiTaskSystem::PushTask(
		std::function<void()>([this,taskName]()->void {
			this->Execute<void>(taskName);
			
			})
	);
}

void ButiScript::VirtualCPU::sys_LoadTextureAsync()
{
	std::string fileName = top().v_->GetRef<std::string>(); pop();
	ButiEngine::GUI::PushMessage(fileName + "を読み込みます");
	ButiTaskSystem::PushTask(
		std::function<void()>([this, fileName]()->void {
			this->GetGameObject()->GetResourceContainer()->LoadTexture(fileName);

			})
	);
}
void ButiScript::VirtualCPU::sys_LoadWaveAsync()
{
	std::string dirName = top().v_->GetRef<std::string>(); pop();
	if (*(dirName.end() - 1) == '/') {

		std::vector< std::string> vec_filePathes;

		std::filesystem::directory_iterator itr(ButiEngine::GlobalSettings::GetResourceDirectory() + dirName), end;

		std::error_code err;

		for (; itr != end && !err; itr.increment(err)) {
			const std::filesystem::directory_entry entry = *itr;
			ButiEngine::GUI::PushMessage(entry.path().string() + u8"を読み込みます");
			vec_filePathes.push_back (StringHelper::Remove(  entry.path().string(), ButiEngine::GlobalSettings::GetResourceDirectory() ));

		}
		ButiTaskSystem::PushTask(
			std::function<void()>([this, vec_filePathes]()->void {
				this->GetGameObject()->GetResourceContainer()->LoadSound(vec_filePathes);

				ButiEngine::GUI::PushMessage(std::to_string(vec_filePathes.size())+ u8"個のwave読み込み終了");
				}
				)
		);
	}
	else {
		ButiEngine::GUI::PushMessage(dirName + u8"を読み込みます");
		ButiTaskSystem::PushTask(
			std::function<void()>([this, dirName]()->void {

				this->GetGameObject()->GetResourceContainer()->LoadSound(dirName);

				ButiEngine::GUI::PushMessage(dirName + u8"の読み込み終了");
				}
				)
		);
	}
}
void ButiScript::VirtualCPU::sys_LoadTexture()
{
	std::string fileName = top().v_->GetRef<std::string>(); pop();
	this->GetGameObject()->GetResourceContainer()->LoadTexture(fileName);
}
void ButiScript::VirtualCPU::sys_LoadWave()
{
	std::string dirName = top().v_->GetRef<std::string>(); pop();
	if (*(dirName.end() - 1) == '/') {

		std::vector< std::string> vec_filePathes;

		std::filesystem::directory_iterator itr(ButiEngine::GlobalSettings::GetResourceDirectory() + dirName), end;

		std::error_code err;

		for (; itr != end && !err; itr.increment(err)) {
			const std::filesystem::directory_entry entry = *itr;
			ButiEngine::GUI::PushMessage(entry.path().string() + u8"を読み込みます");
			vec_filePathes.push_back(StringHelper::Remove(entry.path().string(), ButiEngine::GlobalSettings::GetResourceDirectory()));

		}

		this->GetGameObject()->GetResourceContainer()->LoadSound(vec_filePathes);

		ButiEngine::GUI::PushMessage(std::to_string(vec_filePathes.size()) + u8"個のwave読み込み終了");
	}
	else {
		ButiEngine::GUI::PushMessage(dirName + u8"を読み込みます");

		this->GetGameObject()->GetResourceContainer()->LoadSound(dirName);

		ButiEngine::GUI::PushMessage(dirName + u8"の読み込み終了");
	}
}
void ButiScript::VirtualCPU::SaveGlobalValue(std::vector<std::shared_ptr<ButiScript::IGlobalValueSaveObject>>& arg_ref_vec_saveObject) {
	for (int i = 0; i < globalValue_size- globalValue_base; i++) {
		arg_ref_vec_saveObject.push_back(valueStack[globalValue_base+i].v_->GetSaveObject());
		arg_ref_vec_saveObject.at(i)->SetTypeIndex(valueStack[globalValue_base + i].valueType);
	}
}
void ButiScript::VirtualCPU::RestoreGlobalValue(std::vector<std::shared_ptr< ButiScript::IGlobalValueSaveObject>>& arg_ref_vec_saveObject) {
	if (globalValue_size - globalValue_base != arg_ref_vec_saveObject.size()) {
		ButiEngine::GUI::Console("保存されているグローバル変数の値とスクリプトで定義されているグローバル変数の数が異なります"); 
		
		return;
	}
	for (int i = 0; i < globalValue_size - globalValue_base; i++) {
		if (valueStack[globalValue_base + i].valueType != arg_ref_vec_saveObject.at(i)->GetTypeIndex()) {
			continue;
		}
		if (valueStack[globalValue_base + i].v_) {
			valueStack[globalValue_base + i].v_->release();
		}
		arg_ref_vec_saveObject.at(i)->SetCompiledData(data_);
		arg_ref_vec_saveObject.at(i)->RestoreValue(&valueStack[globalValue_base + i].v_);
		
	}
}
void ButiScript::VirtualCPU::ShowGUI() {
	
	for (auto itr = data_->map_globalValueAddress.begin(), end = data_->map_globalValueAddress.end(); itr != end;itr++) {
		valueStack[globalValue_base + itr->second].v_->ShowGUI(itr->first);
	}
}
#endif