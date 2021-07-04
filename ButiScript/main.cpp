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
	ButiScript::Compiler driver;
	std::shared_ptr< ButiScript::CompiledData> data=std::make_shared<ButiScript::CompiledData>();
	bool compile_result=true;
	{
		driver.RegistDefaultSystems();
		driver.DefineSystemFunction(&ButiScript::VirtualCPU::sys_func_ret<int,int,int ,int, DoSomething>, TYPE_INTEGER, "DoSomething", "i,i,i");
		compile_result = driver.Compile("input.bs", *data);
		driver.OutputCompiledData("output/compiled.cbs", *data);
	}
	if (compile_result) {
		data = std::make_shared<ButiScript::CompiledData>();
		driver.InputCompiledData("output/compiled.cbs", *data);

		ButiScript::VirtualCPU machine(data);
		machine.Initialize();
		machine.AllocGlobalValue();
		auto retunCode = machine.Execute<float>("main");
		std::cout << "return : " << retunCode << std::endl;
		std::system("pause");
	}



	return 0;
}