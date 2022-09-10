#include "stdafx.h"
#include <exception>
#include "VirtualMachine.h"
#ifdef _BUTIENGINEBUILD
#include"ButiEventSystem/ButiEventSystem/EventSystem.h"
#include"ButiEventSystem/ButiEventSystem/TaskSystem.h"
#include<filesystem>
#endif // _BUTIENGINEBUILD


void ButiScript::VirtualMachine::AllocGlobalValue()
{
	stack_base = valueStack.size();
	globalValue_base = stack_base;
	{

		auto buff = command_ptr_;
		command_ptr_ = allocCommand_ptr_;
		std::int32_t Op;
		while ((Op = *command_ptr_++) != VM_HALT) {
			globalValueAllocOpSize++;
			(this->*p_op[Op])();
		}
		command_ptr_ = buff;
	}
	globalValue_size = valueStack.size();
}

void ButiScript::VirtualMachine::Clear()
{
	valueStack.resize(0);
}

ButiScript::VirtualMachine* ButiScript::VirtualMachine::Clone()
{
	auto output = new VirtualMachine(vlp_data);
	output->Initialize();

	output->stack_base = output->valueStack.size();
	output->globalValue_base = output->stack_base;
	{
		for (std::int32_t i = globalValue_base; i < globalValue_size- globalValue_base  ; i++) {
			output->push(valueStack[i].valueData, valueStack[i].valueType);
		}
	}
	output->globalValue_size = globalValue_size;
	return output;
}

void ButiScript::VirtualMachine::Initialize()
{
	commandTable = vlp_data->commandTable;
	textBuffer = vlp_data->textBuffer;
	commandSize = vlp_data->commandSize;
	textSize = vlp_data->textSize;


	allocCommand_ptr_ = commandTable +1;
	p_op = (OperationFunction*)malloc(sizeof(OperationFunction) * VM_MAXCOMMAND);
#include "VM_table.h"

	p_syscall=(OperationFunction*)malloc(sizeof(OperationFunction) * vlp_data->list_sysCalls.GetSize());
	for (std::int32_t index = 0; index < vlp_data->list_sysCalls.GetSize(); index++) {
		p_syscall[index] = vlp_data->list_sysCalls[index];
	}

	p_sysMethodCall= (OperationFunction*)malloc(sizeof(OperationFunction) * vlp_data->list_sysCallMethods.GetSize());
	for (std::int32_t index = 0; index < vlp_data->list_sysCallMethods.GetSize(); index++) {
		p_sysMethodCall[index] = vlp_data->list_sysCallMethods[index];
	}


	p_pushValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (vlp_data->list_types.GetSize() ));
	p_pushRefValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (vlp_data->list_types.GetSize() ));
	for (std::int32_t index = 0; index < vlp_data->list_types.GetSize(); index++) {

		p_pushValues[vlp_data->list_types.At(index).typeIndex] = vlp_data->list_types.At(index).typeFunc;
		p_pushRefValues[vlp_data->list_types.At(index).typeIndex] = vlp_data->list_types.At(index).refTypeFunc;
	}

	list_scriptClassInfo = vlp_data->list_scriptClassInfo;

}

bool ButiScript::VirtualMachine::HotReload(ButiEngine::Value_ptr<CompiledData> arg_data)
{
	bool output = false;
	free(p_op);
	free(p_syscall);

	free(p_sysMethodCall);
	free(p_pushValues);
	free(p_pushRefValues);

	if (std::memcmp(commandTable, arg_data->commandTable, globalValueAllocOpSize)) {
		output = true;
	}

	commandTable = arg_data->commandTable;
	textBuffer = arg_data->textBuffer;
	commandSize = arg_data->commandSize;
	textSize = arg_data->textSize;


	allocCommand_ptr_ = commandTable + 1;
	p_op = (OperationFunction*)malloc(sizeof(OperationFunction) * VM_MAXCOMMAND);
#include "VM_table.h"

	p_syscall = (OperationFunction*)malloc(sizeof(OperationFunction) * arg_data->list_sysCalls.GetSize());
	for (std::int32_t index = 0; index < arg_data->list_sysCalls.GetSize(); index++) {
		p_syscall[index] = arg_data->list_sysCalls[index];
	}

	p_sysMethodCall = (OperationFunction*)malloc(sizeof(OperationFunction) * arg_data->list_sysCallMethods.GetSize());
	for (std::int32_t index = 0; index < arg_data->list_sysCallMethods.GetSize(); index++) {
		p_sysMethodCall[index] = arg_data->list_sysCallMethods[index];
	}


	p_pushValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (arg_data->list_types.GetSize()));
	p_pushRefValues = (OperationFunction*)malloc(sizeof(OperationFunction) * (arg_data->list_types.GetSize()));
	for (std::int32_t index = 0; index < arg_data->list_types.GetSize(); index++) {

		p_pushValues[arg_data->list_types.At(index).typeIndex] = arg_data->list_types.At(index).typeFunc;
		p_pushRefValues[arg_data->list_types.At(index).typeIndex] = arg_data->list_types.At(index).refTypeFunc;
	}
	if (list_scriptClassInfo.GetSize() != arg_data->list_scriptClassInfo.GetSize()) {
		output = true;
	}
	else {
		for (auto itr = list_scriptClassInfo.begin(), end = list_scriptClassInfo.end(), otherItr = arg_data->list_scriptClassInfo.begin(); itr != end; itr++, otherItr++) {
			if ( *itr!= *otherItr) {
				output = true;
				break;
			}
		}
	}
	list_scriptClassInfo = arg_data->list_scriptClassInfo;

	if (output) {
		Clear();
		AllocGlobalValue();
	}
	else {
		for (std::int32_t index = globalValue_base; index < globalValue_size; index++) {
			auto typeIndex = valueStack[index].valueType;
			auto& tag = vlp_data->list_types.At(typeIndex & ~TYPE_REF);
			if (!tag.isSystem&&!tag.IsFunctionObjectType() ) {
				(valueStack[index].Get<Type_ScriptClass>()).SetClassInfoUpdate (arg_data->list_types , list_scriptClassInfo, arg_data->systemTypeCount,typeIndex);
			}
			else if(tag.p_enumTag){
				(valueStack[index].Get<Type_Enum>()).p_enumTag=(arg_data->list_types.At(typeIndex).p_enumTag);
			}
		}
	}

	vlp_data = arg_data;
	return output;
}

void ButiScript::VirtualMachine::Execute_(const std::string& entryPoint)
{
	command_ptr_ = commandTable + vlp_data->map_entryPoints[entryPoint];
	stack_base = valueStack.size();

	auto Op=VM_HALT;
	try {
		while ((Op =static_cast<VM_ENUM> (*command_ptr_++)) != VM_HALT) {
			(this->*p_op[Op])();
		}
	}
	catch (const std::exception& e) {
		std::cerr << "例外発生（" << e.what() << "）" << std::endl;
		return ;
	}

	command_ptr_ = commandTable + vlp_data->map_entryPoints[entryPoint];
}



#ifdef _BUTIENGINEBUILD

void ButiScript::VirtualMachine::sys_addEventMessanger()
{
	std::string eventName = top().Get<std::string>(); pop();
	ButiEventSystem::AddEventMessenger<void>(eventName);
}
void ButiScript::VirtualMachine::sys_removeEventMessanger()
{
	std::string eventName = top().Get<std::string>(); pop();
	ButiEventSystem::RemoveEventMessenger(eventName);
}

void ButiScript::VirtualMachine::sys_registEventListner()
{
	std::string functionName = top().Get<std::string>(); pop();
	std::string keyName = top().Get<std::string>(); pop();
	std::string eventName = top().Get<std::string>(); pop();
	auto retKey= ButiEventSystem::RegistEventListner(eventName, keyName,
		std::function<void()>([this,functionName]()->void {
			this->Execute<void>(functionName);
			}), false);

	push(retKey);
}
void ButiScript::VirtualMachine::sys_unregistEventListner()
{
	std::string keyName = top().Get<std::string>(); pop();
	std::string eventName = top().Get<std::string>(); pop();
	ButiEventSystem::UnRegistEventListner<void>(eventName, keyName);

}
void ButiScript::VirtualMachine::sys_executeEvent()
{
	std::string eventName = top().Get<std::string>(); pop();
	ButiEventSystem::Execute(eventName);
}

void ButiScript::VirtualMachine::sys_pushTask()
{
	std::string taskName = top().Get<std::string>(); pop();
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
	std::string fileName = top().Get<std::string>(); pop();
	ButiEngine::GUI::PushNotification(fileName + "を読み込みます");
	ButiTaskSystem::PushTask(
		std::function<void()>([this, fileName]()->void {
			this->GetGameObject()->GetResourceContainer()->LoadTexture(fileName);

			})
	);
}
void ButiScript::VirtualMachine::sys_LoadWaveAsync()
{
	std::string dirName = top().Get<std::string>(); pop();
	if (*(dirName.end() - 1) == '/') {

		ButiEngine::List< std::string> list_filePathes;

		std::filesystem::directory_iterator itr(ButiEngine::GlobalSettings::GetResourceDirectory() + dirName), end;

		std::error_code err;

		for (; itr != end && !err; itr.increment(err)) {
			const std::filesystem::directory_entry entry = *itr;
			ButiEngine::GUI::PushNotification(entry.path().string() + u8"を読み込みます");
			list_filePathes.Add (StringHelper::Remove(  entry.path().string(), ButiEngine::GlobalSettings::GetResourceDirectory() ));

		}

		assert(0 && "ResourceContainerのListへの対応がまだです");
		ButiTaskSystem::PushTask(
			std::function<void()>([this, list_filePathes]()->void {
				//this->GetGameObject()->GetResourceContainer()->LoadSound(list_filePathes);

				ButiEngine::GUI::PushNotification(std::to_string(list_filePathes.GetSize())+ u8"個のwave読み込み終了");
				}
				)
		);
	}
	else {
		ButiEngine::GUI::PushNotification(dirName + u8"を読み込みます");
		ButiTaskSystem::PushTask(
			std::function<void()>([this, dirName]()->void {

				this->GetGameObject()->GetResourceContainer()->LoadSound(dirName);

				ButiEngine::GUI::PushNotification(dirName + u8"の読み込み終了");
				}
				)
		);
	}
}
void ButiScript::VirtualMachine::sys_LoadTexture()
{
	std::string fileName = top().Get<std::string>(); pop();
	this->GetGameObject()->GetResourceContainer()->LoadTexture(fileName);
}
void ButiScript::VirtualMachine::sys_LoadWave()
{
	std::string dirName = top().Get<std::string>(); pop();
	if (*(dirName.end() - 1) == '/') {

		ButiEngine::List< std::string> list_filePathes;

		std::filesystem::directory_iterator itr(ButiEngine::GlobalSettings::GetResourceDirectory() + dirName), end;

		std::error_code err;

		for (; itr != end && !err; itr.increment(err)) {
			const std::filesystem::directory_entry entry = *itr;
			ButiEngine::GUI::PushNotification(entry.path().string() + u8"を読み込みます");
			list_filePathes.Add(StringHelper::Remove(entry.path().string(), ButiEngine::GlobalSettings::GetResourceDirectory()));

		}
		assert(0 && "ResourceContainerのListへの対応がまだです");
		//this->GetGameObject()->GetResourceContainer()->LoadSound(list_filePathes);

		ButiEngine::GUI::PushNotification(std::to_string(list_filePathes.GetSize()) + u8"個のwave読み込み終了");
	}
	else {
		ButiEngine::GUI::PushNotification(dirName + u8"を読み込みます");

		this->GetGameObject()->GetResourceContainer()->LoadSound(dirName);

		ButiEngine::GUI::PushNotification(dirName + u8"の読み込み終了");
	}
}
void ButiScript::VirtualMachine::sys_addGameObjectFromCereal()
{
	std::string name = top().Get<std::string>(); pop();
	auto obj= vlp_gameObject->GetGameObjectManager().lock()->AddObjectFromCereal(name).lock();
	push(obj);
}
void ButiScript::VirtualMachine::sys_getSelfScriptBehavior()
{
	auto this_behavior = vwp_butiScriptBehavior.lock();
	push(this_behavior);
}
void ButiScript::VirtualMachine::SaveGlobalValue(std::vector<std::pair< ButiEngine::Value_ptr <ButiEngine::IValuePtrRestoreObject>, std::int32_t>>& arg_ref_list_saveObject) {
	for (std::int32_t index = 0; index < globalValue_size- globalValue_base; index++) {
		auto type = valueStack[globalValue_base + index].valueType;
		if (type & TYPE_REF|| !valueStack[globalValue_base + index].valueData) {
			arg_ref_list_saveObject.push_back({ ButiEngine::make_value<ButiEngine::ValuePtrRestoreObject<Type_Null>>() ,type});
		}
		else {
			arg_ref_list_saveObject.push_back({ valueStack[globalValue_base + index].valueData.GetRestoreObject(),type });
		}
	}
}
void ButiScript::VirtualMachine::RestoreGlobalValue(std::vector<std::pair< ButiEngine::Value_ptr <ButiEngine::IValuePtrRestoreObject>, std::int32_t>>& arg_ref_list_saveObject) {
	if (globalValue_size - globalValue_base != arg_ref_list_saveObject.size()) {
		ButiEngine::GUI::Console("保存されているグローバル変数の値とスクリプトで定義されているグローバル変数の数が異なります"); 
		
		return;
	}
	for (std::int32_t index = 0; index < globalValue_size - globalValue_base; index++) {
		if (valueStack[globalValue_base + index].valueType!= arg_ref_list_saveObject.at(index).second) {
			continue;
		}
		auto useCompiled = ButiEngine::dynamic_value_ptr_cast<ButiEngine::IUseCompiledData>(arg_ref_list_saveObject.at(index).first);
		if (useCompiled)
		{
			useCompiled->SetCompiledData(vlp_data);
		}
		arg_ref_list_saveObject.at(index).first->RestoreValue(valueStack[globalValue_base + index].valueData);
		
	}
}
void ButiScript::VirtualMachine::ShowGUI() {
	for (auto itr = vlp_data->map_addressToValueName.begin(), end = vlp_data->map_addressToValueName.end(); itr != end;itr++) {
		valueStack[globalValue_base + itr->first].valueData.ShowGUI(itr->second);
	}
}
#endif