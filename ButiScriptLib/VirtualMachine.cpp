#include "stdafx.h"
#include <exception>
#include "VirtualMachine.h"
#ifdef IMPL_BUTIENGINE
#include"ButiEventSystem/ButiEventSystem/EventSystem.h"
#include"ButiEventSystem/ButiEventSystem/TaskSystem.h"
#include<filesystem>
#endif // IMPL_BUTIENGINE


void ButiScript::VirtualMachine::AllocGlobalValue()
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

ButiScript::VirtualMachine* ButiScript::VirtualMachine::Clone()
{
	auto output = new VirtualMachine(shp_data);
	output->Initialize();

	output->stack_base = output->valueStack.size();						// スタック参照位置初期化
	output->globalValue_base = output->stack_base;
	output->stack_base = output->valueStack.size();
	{
		for (int i = globalValue_base; i < globalValue_base + globalValue_size; i++) {
			output->push(valueStack[i].valueData, valueStack[i].valueType);
		}
	}
	output->globalValue_size = globalValue_size;
	return output;
}

void ButiScript::VirtualMachine::Initialize()
{
	commandTable = shp_data->commandTable;						// プログラム格納位置
	textBuffer = shp_data->textBuffer;				// テキストデータ格納位置
	commandSize = shp_data->commandSize;			// プログラムの大きさ
	textSize = shp_data->textSize;					// データの大きさ


	allocCommand_ptr_ = commandTable +1;
	p_op = (OperationFunction*)malloc(sizeof(OperationFunction) * VM_MAXCOMMAND);
#include "VM_table.h"

	p_syscall=(OperationFunction*)malloc(sizeof(OperationFunction) * shp_data->vec_sysCalls.size());
	for (int i = 0; i < shp_data->vec_sysCalls.size(); i++) {
		p_syscall[i] = shp_data->vec_sysCalls[i];
	}

	p_sysMethodCall= (OperationFunction*)malloc(sizeof(OperationFunction) * shp_data->vec_sysCallMethods.size());
	for (int i = 0; i < shp_data->vec_sysCallMethods.size(); i++) {
		p_sysMethodCall[i] = shp_data->vec_sysCallMethods[i];
	}


	p_pushValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (shp_data->vec_types.size() ));
	p_pushRefValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (shp_data->vec_types.size() ));
	for (int i = 0; i < shp_data->vec_types.size(); i++) {

		p_pushValues[shp_data->vec_types.at(i).typeIndex] = shp_data->vec_types.at(i).typeFunc;
		p_pushRefValues[shp_data->vec_types.at(i).typeIndex] = shp_data->vec_types.at(i).refTypeFunc;
	}

	vec_scriptClassInfo = shp_data->vec_scriptClassInfo;

}

void ButiScript::VirtualMachine::Execute_(const std::string& entryPoint)
{
	command_ptr_ = commandTable + shp_data->map_entryPoints[entryPoint];
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

	command_ptr_ = commandTable + shp_data->map_entryPoints[entryPoint];	// プログラムカウンター初期化
}



#ifdef IMPL_BUTIENGINE

void ButiScript::VirtualMachine::sys_addEventMessanger()
{
	std::string eventName = top().valueData->GetRef<std::string>(); pop();
	ButiEventSystem::AddEventMessenger<void>(eventName);
}
void ButiScript::VirtualMachine::sys_removeEventMessanger()
{
	std::string eventName = top().valueData->GetRef<std::string>(); pop();
	ButiEventSystem::RemoveEventMessenger(eventName);
}

void ButiScript::VirtualMachine::sys_registEventListner()
{
	std::string functionName = top().valueData->GetRef<std::string>(); pop();
	std::string keyName = top().valueData->GetRef<std::string>(); pop();
	std::string eventName = top().valueData->GetRef<std::string>(); pop();
	auto retKey= ButiEventSystem::RegistEventListner(eventName, keyName,
		std::function<void()>([this,functionName]()->void {
			this->Execute<void>(functionName);
			}), false);

	push(retKey);
}
void ButiScript::VirtualMachine::sys_unregistEventListner()
{
	std::string keyName = top().valueData->GetRef<std::string>(); pop();
	std::string eventName = top().valueData->GetRef<std::string>(); pop();
	ButiEventSystem::UnRegistEventListner<void>(eventName, keyName);

}
void ButiScript::VirtualMachine::sys_executeEvent()
{
	std::string eventName = top().valueData->GetRef<std::string>(); pop();
	ButiEventSystem::Execute(eventName);
}

void ButiScript::VirtualMachine::sys_pushTask()
{
	std::string taskName = top().valueData->GetRef<std::string>(); pop();
	auto clone = Clone();
	ButiTaskSystem::PushTask(
		std::function<void()>([clone, taskName]()->void {
				clone->Execute<void>(taskName);
				delete clone;
			})
	);
}

void ButiScript::VirtualMachine::sys_LoadTextureAsync()
{
	std::string fileName = top().valueData->GetRef<std::string>(); pop();
	ButiEngine::GUI::PushMessage(fileName + "を読み込みます");
	ButiTaskSystem::PushTask(
		std::function<void()>([this, fileName]()->void {
			this->GetGameObject()->GetResourceContainer()->LoadTexture(fileName);

			})
	);
}
void ButiScript::VirtualMachine::sys_LoadWaveAsync()
{
	std::string dirName = top().valueData->GetRef<std::string>(); pop();
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
void ButiScript::VirtualMachine::sys_LoadTexture()
{
	std::string fileName = top().valueData->GetRef<std::string>(); pop();
	this->GetGameObject()->GetResourceContainer()->LoadTexture(fileName);
}
void ButiScript::VirtualMachine::sys_LoadWave()
{
	std::string dirName = top().valueData->GetRef<std::string>(); pop();
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
void ButiScript::VirtualMachine::SaveGlobalValue(std::vector<std::shared_ptr<ButiScript::IGlobalValueSaveObject>>& arg_ref_vec_saveObject) {
	for (int i = 0; i < globalValue_size- globalValue_base; i++) {
		arg_ref_vec_saveObject.push_back(valueStack[globalValue_base+i].valueData->GetSaveObject());
		arg_ref_vec_saveObject.at(i)->SetTypeIndex(valueStack[globalValue_base + i].valueType);
	}
}
void ButiScript::VirtualMachine::RestoreGlobalValue(std::vector<std::shared_ptr< ButiScript::IGlobalValueSaveObject>>& arg_ref_vec_saveObject) {
	if (globalValue_size - globalValue_base != arg_ref_vec_saveObject.size()) {
		ButiEngine::GUI::Console("保存されているグローバル変数の値とスクリプトで定義されているグローバル変数の数が異なります"); 
		
		return;
	}
	for (int i = 0; i < globalValue_size - globalValue_base; i++) {
		if (valueStack[globalValue_base + i].valueType != arg_ref_vec_saveObject.at(i)->GetTypeIndex()) {
			continue;
		}
		if (valueStack[globalValue_base + i].valueData) {
			valueStack[globalValue_base + i].valueData->release();
		}
		arg_ref_vec_saveObject.at(i)->SetCompiledData(shp_data);
		arg_ref_vec_saveObject.at(i)->RestoreValue(&valueStack[globalValue_base + i].valueData);
		
	}
}
void ButiScript::VirtualMachine::ShowGUI() {
	
	for (auto itr = shp_data->map_globalValueAddress.begin(), end = shp_data->map_globalValueAddress.end(); itr != end;itr++) {
		valueStack[globalValue_base + itr->second].valueData->ShowGUI(itr->first);
	}
}
#endif