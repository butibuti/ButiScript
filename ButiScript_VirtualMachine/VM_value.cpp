#include"stdafx.h"
#include"ButiScriptLib/Tags.h"
#include "VirtualMachine.h"
#include"ButiUtil/ButiUtil/Util.h"

ButiEngine::List<ButiScript::CreateMemberInstanceFunction>* p_list_createMemberInstanceFunction = nullptr;

void ButiEngine::ValuePtrRestoreObject<ButiScript::Type_ScriptClass>::RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const {
	auto this_type = &vlp_compiledData->list_scriptClassInfo[type - vlp_compiledData->systemTypeCount];
	Value_ptr<ButiScript::Type_ScriptClass> vlp_scriptClass = make_value<ButiScript::Type_ScriptClass>(this_type);
	std::int32_t memberIndex = 0;
	for (auto itr = list_data.begin(), end = list_data.end(); itr != end; itr++, memberIndex++) {
		Value_ptr<void> member;

		auto useCompiledData = dynamic_value_ptr_cast<IUseCompiledData>(itr->first);

		if (useCompiledData) {
			useCompiledData->SetCompiledData(vlp_compiledData);
		}
		(itr->first)->RestoreValue(member);
		vlp_scriptClass->SetMember(member, memberIndex);
	}
	arg_ref_value = vlp_scriptClass;
}
void ButiEngine::ValuePtrRestoreObject<ButiScript::Type_Enum>::RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const {
	auto this_type = vlp_compiledData->list_types[type].p_enumTag;
	arg_ref_value = make_value<ButiScript::Type_Enum>(data, this_type);
}
void ButiEngine::ValuePtrRestoreObject<ButiScript::Type_Function>::RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const {

	arg_ref_value = make_value<ButiScript::Type_Function>(address, &vlp_compiledData->map_functionJumpPointsTable, ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>>());
}
void ButiEngine::ValuePtrRestoreObject<ButiScript::Type_Null>::RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const {

}


auto createMemberInstancesRelease = ButiEngine::Util::MemoryReleaser(&p_list_createMemberInstanceFunction);

ButiEngine::List<ButiScript::CreateMemberInstanceFunction>& GetCreateMemberInstanceFunction() {
	if (!p_list_createMemberInstanceFunction) {
		p_list_createMemberInstanceFunction = new ButiEngine::List<ButiScript::CreateMemberInstanceFunction>();
	}
	return *p_list_createMemberInstanceFunction;
}
ButiEngine::Value_ptr<void> CreateScriptValueData(ButiScript::ScriptClassInfo& arg_info, ButiEngine::List<ButiScript::ScriptClassInfo>* p_list_scriptClassInfo) {

	ButiEngine::List<ButiEngine::Value_ptr<void>> list_members;
	std::int32_t memberSize = arg_info.GetMemberSize();
	for (std::int32_t i = 0; i < memberSize; i++) {
		auto typeIndex = arg_info.GetMemberTypeIndex(i);
		//ílÇÃê∂ê¨
		if (! (typeIndex & TYPE_REF) ){

			if (typeIndex < GetCreateMemberInstanceFunction().GetSize()) {
				list_members.Add(GetCreateMemberInstanceFunction()[typeIndex]());
			}
			else {
				list_members.Add(CreateScriptValueData(p_list_scriptClassInfo->at(typeIndex - GetCreateMemberInstanceFunction().GetSize()), p_list_scriptClassInfo));
			}
		}
		//éQè∆å^
		else {
			list_members.Add(ButiEngine::Value_ptr<void>());
		}
	}
	return ButiEngine::make_value<ButiScript::Type_ScriptClass>(list_members ,&arg_info);
	//return nullptr;
}


ButiScript::Value::Value(ScriptClassInfo& arg_info, ButiEngine::List<ButiScript::ScriptClassInfo>* p_list_scriptClassInfo)	{
	
	valueData = CreateScriptValueData(arg_info,p_list_scriptClassInfo);
	valueType = arg_info.GetTypeIndex();
}

std::map<std::int64_t, std::int32_t>* p_map_typeIndex;

std::map<std::int64_t, std::int32_t>& GetTypeIndexMap() {
	if (!p_map_typeIndex) {

		p_map_typeIndex = new std::map<std::int64_t, std::int32_t>();
	}
	return *p_map_typeIndex;
}

std::int32_t ButiScript::Value::SetTypeIndex(std::int64_t arg_typeFunc)
{
	auto index = GetTypeIndexMap().size();
	auto r= GetTypeIndexMap().emplace( arg_typeFunc, index );
	return index;
}

std::int32_t ButiScript::Value::GetTypeIndex(std::int64_t arg_typeFunc)
{
	return GetTypeIndexMap().at(arg_typeFunc);
}

void ButiScript::PushCreateMemberInstance(CreateMemberInstanceFunction arg_function)
{
	GetCreateMemberInstanceFunction().Add(arg_function);
}


auto typeMapRelease = ButiEngine::Util::MemoryReleaser(&p_map_typeIndex);