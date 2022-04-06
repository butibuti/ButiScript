#include"stdafx.h"
#include"Tags.h"
#include "VirtualMachine.h"


ButiEngine::List<ButiScript::CreateMemberInstanceFunction>* p_vec_createMemberInstanceFunction = nullptr;
#ifdef _BUTIENGINEBUILD

void ButiEngine::ValuePtrRestoreObject<ButiScript::Type_ScriptClass>::RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const {
	auto this_type =& shp_compiledData->vec_scriptClassInfo[type - shp_compiledData->systemTypeCount];
	Value_ptr<ButiScript::Type_ScriptClass> vlp_scriptClass = make_value<ButiScript::Type_ScriptClass>(this_type);
	std::int32_t memberIndex=0;
	for (auto itr = vec_data.begin(), end = vec_data.end(); itr != end; itr++, memberIndex++) {
		Value_ptr<void> member;
		 
		auto useCompiledData = dynamic_value_ptr_cast<IUseCompiledData>(itr->first);

		if (useCompiledData) {
			useCompiledData->SetCompiledData(shp_compiledData);
		}
		(itr->first)->RestoreValue(member);
		vlp_scriptClass->SetMember(member,memberIndex);
	}
	arg_ref_value = vlp_scriptClass;
}
void ButiEngine::ValuePtrRestoreObject<ButiScript::Type_Enum>::RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const {
	auto this_type = shp_compiledData->vec_types[type].p_enumTag;
	arg_ref_value = make_value<ButiScript::Type_Enum>(data,this_type);
}
void ButiEngine::ValuePtrRestoreObject<ButiScript::Type_Function>::RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const {
	
	arg_ref_value = make_value<ButiScript::Type_Function>(address, &shp_compiledData->map_functionJumpPointsTable, ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>>());
}
void ButiEngine::ValuePtrRestoreObject<ButiScript::Type_Null>::RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const {

}


auto createMemberInstancesRelease = ButiEngine::Util::MemoryReleaser(&p_vec_createMemberInstanceFunction);
#else
auto createMemberInstancesRelease = MemoryReleaser(&p_vec_createMemberInstanceFunction);
#endif

ButiEngine::List<ButiScript::CreateMemberInstanceFunction>& GetCreateMemberInstanceFunction() {
	if (!p_vec_createMemberInstanceFunction) {
		p_vec_createMemberInstanceFunction = new ButiEngine::List<ButiScript::CreateMemberInstanceFunction>();
	}
	return *p_vec_createMemberInstanceFunction;
}
ButiEngine::Value_ptr<void> CreateScriptValueData(ButiScript::ScriptClassInfo& arg_info, ButiEngine::List<ButiScript::ScriptClassInfo>* p_vec_scriptClassInfo) {

	ButiEngine::List<ButiEngine::Value_ptr<void>> vec_members;
	std::int32_t memberSize = arg_info.GetMemberSize();
	for (std::int32_t i = 0; i < memberSize; i++) {
		auto typeIndex = arg_info.GetMemberTypeIndex(i);
		//ílÇÃê∂ê¨
		if (! (typeIndex & TYPE_REF) ){

			if (typeIndex < GetCreateMemberInstanceFunction().GetSize()) {
				vec_members.Add(GetCreateMemberInstanceFunction()[typeIndex]());
			}
			else {
				vec_members.Add(CreateScriptValueData(p_vec_scriptClassInfo->at(typeIndex - GetCreateMemberInstanceFunction().GetSize()), p_vec_scriptClassInfo));
			}
		}
		//éQè∆å^
		else {
			vec_members.Add(ButiEngine::Value_ptr<void>());
		}
	}
	return ButiEngine::make_value<ButiScript::Type_ScriptClass>(vec_members ,&arg_info);
	//return nullptr;
}


ButiScript::Value::Value(ScriptClassInfo& arg_info, ButiEngine::List<ButiScript::ScriptClassInfo>* p_vec_scriptClassInfo)	{
	
	valueData = CreateScriptValueData(arg_info,p_vec_scriptClassInfo);
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


#ifdef _BUTIENGINEBUILD
auto typeMapRelease= ButiEngine::Util::MemoryReleaser (&p_map_typeIndex);
#else

auto typeMapRelease = MemoryReleaser<std::map<std::int64_t,std::int32_t>>(&p_map_typeIndex);
#endif
