#include"stdafx.h"
#include"Tags.h"
#include "VirtualMachine.h"


std::vector<ButiScript::CreateMemberInstanceFunction>* p_vec_createMemberInstanceFunction = nullptr;
auto createMemberInstancesRelease = MemoryReleaser(&p_vec_createMemberInstanceFunction);
#ifdef _BUTIENGINEBUILD
void  ButiScript::GlobalValueSaveObject<ButiScript::Type_Enum>::RestoreValue(ButiScript::IValueData** arg_v) const
{
	*arg_v = new ButiScript::ValueData<Type_Enum>(data, 1, &shp_compiledData->map_enumTag.at(type));
}
void  ButiScript::GlobalValueSaveObject<ButiScript::Type_Func>::RestoreValue(ButiScript::IValueData** arg_v) const
{
	*arg_v = new ButiScript::ValueData<Type_Func>(1, nullptr, std::vector<std::pair< IValueData*, std::int32_t>>());
}
void  ButiScript::GlobalValueSaveObject<ButiScript::Type_Null>::RestoreValue(ButiScript::IValueData** arg_v) const
{
	*arg_v = new ButiScript::ValueData<Type_Null>(1);
}

auto createMemberInstancesRelease = ButiEngine::Util::MemoryReleaser(&p_vec_createMemberInstanceFunction);
#endif

std::vector<ButiScript::CreateMemberInstanceFunction>& GetCreateMemberInstanceFunction() {
	if (!p_vec_createMemberInstanceFunction) {
		p_vec_createMemberInstanceFunction = new std::vector<ButiScript::CreateMemberInstanceFunction>();
	}
	return *p_vec_createMemberInstanceFunction;
}
ButiEngine::Value_ptr<void> CreateScriptValueData(ButiScript::ScriptClassInfo& arg_info, std::vector<ButiScript::ScriptClassInfo>* p_vec_scriptClassInfo) {

	std::vector<ButiEngine::Value_ptr<void>> vec_members;
	std::int32_t memberSize = arg_info.GetMemberSize();
	for (std::int32_t i = 0; i < memberSize; i++) {
		auto typeIndex = arg_info.GetMemberTypeIndex(i);
		//ílÇÃê∂ê¨
		if (! (typeIndex & TYPE_REF) ){

			if (typeIndex < GetCreateMemberInstanceFunction().size()) {
				vec_members.push_back(GetCreateMemberInstanceFunction()[typeIndex]());
			}
			else {
				vec_members.push_back(CreateScriptValueData(p_vec_scriptClassInfo->at(typeIndex - GetCreateMemberInstanceFunction().size()), p_vec_scriptClassInfo));
			}
		}
		//éQè∆å^
		else {
			vec_members.push_back(ButiEngine::Value_ptr<void>());
		}
	}
	return ButiEngine::make_value<ButiScript::Type_ScriptClass>(vec_members ,&arg_info);
	//return nullptr;
}

#ifdef _BUTIENGINEBUILD

void ButiScript::GlobalScriptTypeValueSaveObject::RestoreValue(IValueData** arg_v) const
{
	std::vector<ButiScript::IValueData*> vec_members;
	
	for (auto itr = vec_data.begin(), end = vec_data.end(); itr != end; itr++) {
		IValueData* member;
		(*itr)->RestoreValue(&member);
		vec_members.push_back(member);
	}
	*arg_v = new ButiScript::ValueData<ButiScript::ScriptClassInfo>(&shp_compiledData->vec_scriptClassInfo[type - shp_compiledData-> systemTypeCount], vec_members, 1);

}
#endif
ButiScript::Value::Value(ScriptClassInfo& arg_info, std::vector<ButiScript::ScriptClassInfo>* p_vec_scriptClassInfo)	{
	
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
	GetCreateMemberInstanceFunction().push_back(arg_function);
}


#ifdef _BUTIENGINEBUILD
auto typeMapRelease= ButiEngine::Util::MemoryReleaser (&p_map_typeIndex);
#else

auto typeMapRelease = MemoryReleaser<std::map<std::int64_t,std::int32_t>>(&p_map_typeIndex);
#endif
