#include "stdafx.h"
#define _CRTDBG_MAP_ALLOC

class ValueTypeTest:public ButiEngine::enable_value_from_this<ValueTypeTest> {
public:
	ValueTypeTest() {
		v = 0;
		std::cout << "ValueTypeTest() Called. v:" <<v<< std::endl;
	}
	~ValueTypeTest() {
		std::cout << "~ValueTypeTest() called. v:" << v << std::endl;
	}
	void Show()const {
		std::cout<<"v:" << v << std::endl;
	}
	template <typename T>
	void SetValue(const T arg_t) { 
		v = arg_t;
	}
	std::int32_t GetValue()const { return v; }
	std::int32_t SetValueMul(const float arg_f, const std::int32_t arg_i) {
		return v = arg_f * arg_i;
	}
	void Copy(ButiEngine::Value_ptr<ValueTypeTest> arg_t) {
		v = arg_t->v;
	}
private:
	std::int32_t v;
};
template<typename T> 
T CreateInstance() { 
	return T(); 
}
ButiEngine::Value_ptr<ValueTypeTest> g_output=nullptr;
ButiEngine::Value_ptr<ValueTypeTest> GetValuePtrInstance() {
	return g_output;
}
float Ease(const float arg_f, const std::int32_t arg_ease) {
	return 0;
}

class SingleType{
public:
	SingleType(std::int32_t arg_i) {
		i = arg_i;
	}
	std::int32_t i;
};

#include"BuiltInTypeRegister.h"
#include "Compiler.h"
std::int32_t main(const std::int32_t argCount, const char* args[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	{

		auto v = ButiEngine::make_value<ValueTypeTest>();
		std::vector<ButiEngine::Value_ptr<ValueTypeTest>> map_v;
		map_v.push_back(v);
	}

	ButiScript::Compiler driver;
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func<&CreateInstance<std::int32_t>>, TYPE_INTEGER, "CreateInstance", "", { TYPE_INTEGER });
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func<&CreateInstance<float>>, TYPE_FLOAT, "CreateInstance", "", {TYPE_FLOAT});

	ButiScript::SystemTypeRegister::GetInstance()->RegistValueSystemType<ValueTypeTest>("ValueTypeTest", "ValueTypeTest");

	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method<&ValueTypeTest::SetValue<std::int32_t>,  &ButiScript::VirtualMachine::GetPtr<std::int32_t> >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), TYPE_VOID, "SetValue", "i", { TYPE_INTEGER });
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method<&ValueTypeTest::SetValue<float>,  &ButiScript::VirtualMachine::GetPtr<float> >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), TYPE_VOID, "SetValue", "f", { TYPE_FLOAT });
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method<&ValueTypeTest::Copy,  &ButiScript::VirtualMachine::GetValuePtr<ValueTypeTest> >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), TYPE_VOID, "Copy", "ValueTypeTest");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method<&ValueTypeTest::GetValue>, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), TYPE_INTEGER, "GetValue", "");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method<&ValueTypeTest::Show >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), TYPE_VOID, "Show", "");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method<&ValueTypeTest::SetValueMul, &ButiScript::VirtualMachine::GetPtr, &ButiScript::VirtualMachine::GetPtr>, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), TYPE_VOID, "SetValueMul", "f,i");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func<&GetValuePtrInstance>, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), "CreateValueTest", "");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func<&Ease>, TYPE_FLOAT, "Ease", "f,i");
	
	driver.RegistDefaultSystems();
	g_output = ButiEngine::make_value<ValueTypeTest>();

	bool compile_result=false;
	for(std::int32_t i=1;i<argCount;i++)
	{
		ButiEngine::Value_ptr< ButiScript::CompiledData> data = ButiEngine::make_value<ButiScript::CompiledData>();
		compile_result = driver.Compile(args[i], *data);
		if (!compile_result) {
			std::cout << args[i]<<"のコンパイル成功" << std::endl;
			driver.OutputCompiledData(StringHelper::GetDirectory(args[i])+"/output/"+ StringHelper::GetFileName(args[i],false)+ ".cbs", *data);
		}
		else {
			std::cout << args[i] << "のコンパイル失敗" << std::endl;
		}
		data = ButiEngine::make_value<ButiScript::CompiledData>();
		auto res= driver.InputCompiledData(StringHelper::GetDirectory(args[i]) + "/output/" + StringHelper::GetFileName(args[i], false) + ".cbs", *data);
		if (!res) {
			ButiScript::VirtualMachine* p_clone; 
			std::int32_t returnCode=0;
			{
				ButiScript::VirtualMachine machine(data);
				machine.Initialize();
				machine.AllocGlobalValue();

				std::cout << args[i] << "のmain実行" << std::endl;
				std::cout << "////////////////////////////////////" << std::endl;
				returnCode= machine.Execute<std::int32_t>("main");
				std::cout << "////////////////////////////////////" << std::endl;
				std::cout << args[i] << "のreturn : " << std::to_string(returnCode) << std::endl;

			}

		}
	}
	g_output = nullptr;
	std::system("pause");



	return 0;
}