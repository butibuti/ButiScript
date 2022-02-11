#include"stdafx.h"
#include"Tags.h"
#include "VirtualMachine.h"


std::vector<ButiScript::CreateMemberInstanceFunction>* p_vec_createMemberInstanceFunction = nullptr;
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
ButiScript::IValueData* GetScriptIValue(ButiScript::ScriptClassInfo& arg_info, std::vector<ButiScript::ScriptClassInfo>* p_vec_scriptClassInfo) {

	std::vector<ButiScript::IValueData*> vec_members;
	std::int32_t memberSize = arg_info.GetMemberSize();
	for (std::int32_t i = 0; i < memberSize; i++) {
		auto typeIndex = arg_info.GetMemberTypeIndex(i);
		//ílÇÃê∂ê¨
		if (! (typeIndex & TYPE_REF) ){

			if (typeIndex < GetCreateMemberInstanceFunction().size()) {
				vec_members.push_back(GetCreateMemberInstanceFunction()[typeIndex]());
			}
			else {
				vec_members.push_back(GetScriptIValue(p_vec_scriptClassInfo->at(typeIndex - GetCreateMemberInstanceFunction().size()), p_vec_scriptClassInfo));
			}
		}
		//éQè∆å^
		else {
			vec_members.push_back(ButiScript::GetNullValueData());
		}
	}
	return new ButiScript::ValueData<ButiScript::ScriptClassInfo>(&arg_info, vec_members, 1);
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
	
	valueData = GetScriptIValue(arg_info,p_vec_scriptClassInfo);
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

auto nullValueData=ButiScript::ValueData<ButiScript::Type_Null>(1);
ButiScript::ValueData<ButiScript::Type_Null>* ButiScript::GetNullValueData()
{
	nullValueData.addref();
	return &nullValueData;
}

#ifdef _BUTIENGINEBUILD
auto typeMapRelease= ButiEngine::Util::MemoryReleaser (&p_map_typeIndex);
#else

template<typename T>
class MemoryReleaser {
public:
	MemoryReleaser(T** arg_p_memoryAddress) :p_memoryAddress(arg_p_memoryAddress) {}
	~MemoryReleaser()
	{
		if (*p_memoryAddress) {
			delete (*p_memoryAddress);
		}
	}
private:
	T** p_memoryAddress;
};

auto typeMapRelease = MemoryReleaser<std::map<std::int64_t,std::int32_t>>(&p_map_typeIndex);
#endif
