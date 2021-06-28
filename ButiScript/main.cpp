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
	std::shared_ptr< ButiScript::VirtualCPU >machine;
	bool compile_result;
	{
		ButiScript::Compiler driver;
		driver.RegistDefaultSystems();
		driver.DefineSystemFunction(&ButiScript::VirtualCPU::sys_func_ret<int,int,int ,int, DoSomething>, TYPE_INTEGER, "DoSomething", "i,i,i");
		compile_result = driver.Compile("input.bs", *data);
	}
	if (compile_result) {
		machine=std::make_shared<ButiScript::VirtualCPU>(data);
		machine->Initialize();
		int retunCode= machine->Run();
		std::cout<<"return : " << retunCode << std::endl;
		std::system("pause");
	}



	return 0;
}