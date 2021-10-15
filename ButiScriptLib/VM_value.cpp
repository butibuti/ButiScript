#include"stdafx.h"
#include"Tags.h"
#include "VirtualMachine.h"


#ifdef IMPL_BUTIENGINE
void  ButiScript::GlobalValueSaveObject<ButiScript::Type_Enum>::RestoreValue(ButiScript::IValueData** arg_v) const
{
	*arg_v = new ButiScript::ValueData<Type_Enum>(data, 1,&shp_compiledData->map_enumTag.at(type));
}
std::vector<ButiScript::CreateMemberInstanceFunction>* p_vec_createMemberInstanceFunction=nullptr;
std::vector<ButiScript::CreateMemberInstanceFunction>& GetCreateMemberInstanceFunction() {
	if (!p_vec_createMemberInstanceFunction) {
		p_vec_createMemberInstanceFunction = new std::vector<ButiScript::CreateMemberInstanceFunction>();
	}
	return *p_vec_createMemberInstanceFunction;
}

auto createMemberInstancesRelease = ButiEngine::Util::MemoryReleaser(&p_vec_createMemberInstanceFunction);
#endif
ButiScript::IValueData* GetScriptIValue(ButiScript::ScriptClassInfo& arg_info, std::vector<ButiScript::ScriptClassInfo>* p_vec_scriptClassInfo) {

#ifdef IMPL_BUTIENGINE
	std::vector<ButiScript::IValueData*> vec_members;
	int memberSize = arg_info.GetMemberSize();
	for (int i = 0; i < memberSize; i++) {
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
			vec_members.push_back(nullptr);
		}
	}
	return new ButiScript::ValueData<ButiScript::ScriptClassInfo>(&arg_info, vec_members, 1);
#else
	return nullptr;
#endif
}

#ifdef IMPL_BUTIENGINE

void ButiScript::GlobalScriptTypeValueSaveObject::RestoreValue(IValueData** arg_v) const
{
	std::vector<ButiScript::IValueData*> vec_members;
	auto end = vec_data.end();
	for (auto itr = vec_data.begin(); itr != end; itr++) {
		IValueData* member;
		(*itr)->RestoreValue(&member);
		vec_members.push_back(member);
	}
	*arg_v = new ButiScript::ValueData<ButiScript::ScriptClassInfo>(&shp_compiledData->vec_scriptClassInfo[type - shp_compiledData-> systemTypeCount], vec_members, 1);

}
#endif
ButiScript::Value::Value(ScriptClassInfo& arg_info, std::vector<ButiScript::ScriptClassInfo>* p_vec_scriptClassInfo)	{
	
	v_ = GetScriptIValue(arg_info,p_vec_scriptClassInfo);
	valueType = arg_info.GetTypeIndex();
}

std::map<long long int, int>* p_map_typeIndex;

std::map<long long int, int>& GetTypeIndexMap() {
	if (!p_map_typeIndex) {

		p_map_typeIndex = new std::map<long long int, int>();
	}
	return *p_map_typeIndex;
}

int ButiScript::Value::SetTypeIndex(long long int arg_typeFunc)
{
	auto index = GetTypeIndexMap().size();
	auto r= GetTypeIndexMap().emplace( arg_typeFunc, index );
	return index;
}

int ButiScript::Value::GetTypeIndex(long long int arg_typeFunc)
{
	return GetTypeIndexMap().at(arg_typeFunc);
}

void ButiScript::PushCreateMemberInstance(CreateMemberInstanceFunction arg_function)
{

#ifdef IMPL_BUTIENGINE
	GetCreateMemberInstanceFunction().push_back(arg_function);
#endif
}

#ifdef IMPL_BUTIENGINE
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

auto typeMapRelease = MemoryReleaser<std::map<long long,int>>(&p_map_typeIndex);
#endif
