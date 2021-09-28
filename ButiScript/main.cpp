#include "stdafx.h"
#include"BuiltInTypeRegister.h"
#include "Compiler.h"


int main()
{
	ButiScript::Compiler driver;
	std::shared_ptr< ButiScript::CompiledData> data=std::make_shared<ButiScript::CompiledData>();
	bool compile_result=true;

	{
		//ButiScript::SystemTypeRegister::GetInstance()->SetDefaultSystemType();
		//ButiScript::SystemFuntionRegister::GetInstance()->SetDefaultFunctions();
		driver.RegistDefaultSystems();
		//driver.RegistSystemType<Sample>("Sample", "Sample");
		//driver.DefineSystemFunction(&ButiScript::VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "Sample"); 
		compile_result = driver.Compile("input.bs", *data);
		driver.OutputCompiledData("output/compiled.cbs", *data);
	}

	data = std::make_shared<ButiScript::CompiledData>();
	driver.InputCompiledData("output/compiled.cbs", *data);

	ButiScript::VirtualCPU machine(data);
	machine.Initialize();
	machine.AllocGlobalValue();
	//machine.SetGlobalVariable(1, "g_i");
	//machine.SetGlobalVariable(2, "g_i1");
	auto retunCode = machine.Execute<int>("main");
	std::cout << "return : " << std::to_string(retunCode) << std::endl;
	std::system("pause");



	return 0;
}