#include"stdafx.h"
#include"Tags.h"
#include "VM_value.h"


std::vector<ButiScript::CreateMemberInstanceFunction> vec_createMemberInstanceFunction;

ButiScript::IValue* GetScriptIValue(ButiScript::ScriptClassInfo& arg_info, std::vector<ButiScript::ScriptClassInfo>* p_vec_scriptClassInfo) {
	std::vector<ButiScript::IValue*> vec_members;
	int memberSize = arg_info.GetMemberSize();
	for (int i = 0; i < memberSize; i++) {
		auto typeIndex = arg_info.GetMemberTypeIndex(i);
		//ílÇÃê∂ê¨
		if (! (typeIndex & TYPE_REF) ){
			if (typeIndex < vec_createMemberInstanceFunction.size()) {
				vec_members.push_back(vec_createMemberInstanceFunction[typeIndex]());
			}
			else {
				vec_members.push_back(GetScriptIValue(p_vec_scriptClassInfo->at(typeIndex - vec_createMemberInstanceFunction.size()), p_vec_scriptClassInfo));
			}
		}
		//éQè∆å^
		else {
			vec_members.push_back(nullptr);
		}
	}
	return new ButiScript::Value_wrap<ButiScript::ScriptClassInfo>(&arg_info, vec_members, 1);
}
ButiScript::Value::Value(ScriptClassInfo& arg_info, std::vector<ButiScript::ScriptClassInfo>* p_vec_scriptClassInfo)	{
	
	v_ = GetScriptIValue(arg_info,p_vec_scriptClassInfo);
	valueType = arg_info.GetTypeIndex();
}

std::map<long long int, int> map_typeIndex;

int ButiScript::Value::SetTypeIndex(long long int arg_typeFunc)
{
	auto index = map_typeIndex.size();
	map_typeIndex.emplace(arg_typeFunc, index);
	return index;
}

int ButiScript::Value::GetTypeIndex(long long int arg_typeFunc)
{
	return map_typeIndex.at(arg_typeFunc);
}

void ButiScript::PushCreateMemberInstance(CreateMemberInstanceFunction arg_function)
{
	vec_createMemberInstanceFunction.push_back(arg_function);
}