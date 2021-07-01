#include "stdafx.h"

#include "Compiler.h"

int DoSomething() {
	return 52;
}
int DoSomething(int i) {
	return 52 + i;
}

int DoSomething(int i,int j,int k) {
	return 52 + i*j-k*2;
}


int main()
{
	std::shared_ptr< ButiScript::CompiledData> data=std::make_shared<ButiScript::CompiledData>();
	std::shared_ptr< ButiScript::VirtualCPU >machine,machine2;
	bool compile_result;
	{
		ButiScript::Compiler driver;
		driver.RegistDefaultSystems();
		driver.DefineSystemFunction(&ButiScript::VirtualCPU::sys_func_ret<int,int,int ,int, DoSomething>, TYPE_INTEGER, "DoSomething", "i,i,i");
		compile_result = driver.Compile("input.bs", *data);
		ButiScript::Compiler::OutputCompiledData("compiled.cbs", *data);
	}
	if (compile_result) {
		machine = std::make_shared<ButiScript::VirtualCPU>(data);
		machine2 = std::make_shared<ButiScript::VirtualCPU>(data);
		machine->Initialize();
		machine2->Initialize();
		machine->AllocGlobalValue();
		machine2->AllocGlobalValue();
		auto retunCode = machine->Execute<float>("main");
		std::cout << "return : " << retunCode << std::endl;
		std::system("pause");
	}
	if (compile_result) {
		machine2 = std::make_shared<ButiScript::VirtualCPU>(data);
		machine2->Initialize();
		machine2->AllocGlobalValue();
		auto retunCode = machine2->Execute<int>("main2:i",12);
		std::cout << "return : " << retunCode << std::endl;
		std::system("pause");
	}



	return 0;
}