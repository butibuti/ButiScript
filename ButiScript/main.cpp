#include "stdafx.h"
#define _CRTDBG_MAP_ALLOC
class Sample {
public:
	std::int32_t count = 0;
	Sample() {
	}
	Sample(const std::int32_t arg) {
		count = arg;
	}
	~Sample() {	}
	std::int32_t TestMethod() {
		std::cout << "Sample::TestMethod() is Called! count:"<<count<<std::endl;
		count++;
		return count;
	}
};
std::shared_ptr<Sample> CreateSample() {
	return std::make_shared<Sample>();
}

template<typename T> 
T CreateInstance() { 
	return T(); 
}
#include"BuiltInTypeRegister.h"
#include "Compiler.h"
std::int32_t main(const std::int32_t argCount, const char* args[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	ButiScript::Compiler driver;
	ButiScript::SystemTypeRegister::GetInstance()->RegistSharedSystemType<Sample>("Sample", "Sample");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method_ret< Sample, std::int32_t, &Sample::TestMethod, &ButiScript::VirtualMachine::GetSharedTypePtr  >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), TYPE_INTEGER, "TestMethod", "");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func_ret<std::shared_ptr<Sample>, &CreateSample >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), "CreateSample", "");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func_ret<std::int32_t, &CreateInstance<std::int32_t>>, TYPE_INTEGER, "CreateInstance", "", { TYPE_INTEGER });
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func_ret<float, &CreateInstance<float>>, TYPE_FLOAT, "CreateInstance", "", {TYPE_FLOAT});

	driver.RegistDefaultSystems();
	bool compile_result=false;
	for(std::int32_t i=1;i<argCount;i++)
	{
		std::shared_ptr< ButiScript::CompiledData> data = std::make_shared<ButiScript::CompiledData>();
		compile_result = driver.Compile(args[i], *data);
		if (!compile_result) {
			std::cout << args[i]<<"のコンパイル成功" << std::endl;
			driver.OutputCompiledData(StringHelper::GetDirectory(args[i])+"/output/"+ StringHelper::GetFileName(args[i],false)+ ".cbs", *data);
		}
		else {
			std::cout << args[i] << "のコンパイル失敗" << std::endl;
		}
		data = std::make_shared<ButiScript::CompiledData>();
		auto res= driver.InputCompiledData(StringHelper::GetDirectory(args[i]) + "/output/" + StringHelper::GetFileName(args[i], false) + ".cbs", *data);
		if (!res) {
			ButiScript::VirtualMachine* p_clone; 
			std::int32_t returnCode=0;
			{

				ButiScript::VirtualMachine machine(data);

				machine.Initialize();
				machine.AllocGlobalValue();
				//machine.SetGlobalVariable(1, "g_i");
				//machine.SetGlobalVariable(2, "g_i1");

				std::cout << args[i] << "のmain実行" << std::endl;
				std::cout << "////////////////////////////////////" << std::endl;
				returnCode= machine.Execute<std::int32_t>("main");
				std::cout << "////////////////////////////////////" << std::endl;
				std::cout << args[i] << "のreturn : " << std::to_string(returnCode) << std::endl;

				//p_clone = machine.Clone();
			}

		}
	}

	std::system("pause");



	return 0;
}