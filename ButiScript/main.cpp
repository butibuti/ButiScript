#include "stdafx.h"
#pragma comment(lib,"ButiUtil.lib")
#define _CRTDBG_MAP_ALLOC
class Sample:public ButiEngine::enable_value_from_this<Sample> {
public:
	std::int32_t count = 0;
	Sample() {
	}
	Sample(const std::int32_t arg) {
		count = arg;
	}
	~Sample() {	
		std::int32_t i= 0;
	}
	std::int32_t TestMethod() {
		std::cout << "Sample::TestMethod() is Called! count:" << count << std::endl;
		count++;
		return count;
	}
	std::int32_t ShowMethod()const {
		std::cout << "Sample::TestMethod() is Called! count:" << count << std::endl;
		return count;
	}
};
class ValueTypeTest {
public:
	ValueTypeTest() {
		v = 0;
		std::cout << "ValueTypeTest() Called. v:" <<v<< std::endl;
	}
	~ValueTypeTest() {
		std::cout << "~ValueTypeTest() called. v:" << v << std::endl;
	}
	void SetValue(const std::int32_t& arg_v) { v = arg_v; }
	void Show()const {
		std::cout<<"v:" << v << std::endl;
	}
	std::int32_t GetValue()const { return v; }
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

#include"BuiltInTypeRegister.h"
#include "Compiler.h"
std::int32_t main(const std::int32_t argCount, const char* args[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	ButiScript::Compiler driver;
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func<&CreateInstance<std::int32_t>>, TYPE_INTEGER, "CreateInstance", "", { TYPE_INTEGER });
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func<&CreateInstance<float>>, TYPE_FLOAT, "CreateInstance", "", {TYPE_FLOAT});

	ButiScript::SystemTypeRegister::GetInstance()->RegistValueSystemType<ValueTypeTest>("ValueTypeTest", "ValueTypeTest");

	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method<&ValueTypeTest::SetValue, &ButiScript::VirtualMachine::GetTypePtr<ValueTypeTest>,&ButiScript::VirtualMachine::GetTypePtr<std::int32_t> >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), TYPE_VOID, "SetValue", "i");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method<&ValueTypeTest::GetValue, &ButiScript::VirtualMachine::GetTypePtr  >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), TYPE_INTEGER, "GetValue", "");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method<&ValueTypeTest::Show, &ButiScript::VirtualMachine::GetTypePtr>, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), TYPE_VOID, "Show", "");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func<&GetValuePtrInstance>, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("ValueTypeTest"), "CreateValueTest", "");
	
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