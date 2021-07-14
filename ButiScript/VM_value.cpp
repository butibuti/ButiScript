#include"stdafx.h"
#include"Tags.h"
#include "VM_value.h"


std::vector<ButiScript::CreateMemberInstanceFunction> vec_createMemberInstanceFunction;
ButiScript::Value::Value(ScriptClassInfo& arg_info)	{
	std::vector<IValue*> vec_members;
	int memberSize = arg_info.GetMemberSize();
	for (int i = 0; i < memberSize; i++) {
		vec_members.push_back(vec_createMemberInstanceFunction[arg_info.GetMemberTypeIndex(i)]());
	}
	v_ = new Value_wrap<ScriptClassInfo>(&arg_info, vec_members, 1);
	valueType = arg_info.GetTypeIndex();
}

void ButiScript::PushCreateMemberInstance(CreateMemberInstanceFunction arg_function)
{
	vec_createMemberInstanceFunction.push_back(arg_function);
}
