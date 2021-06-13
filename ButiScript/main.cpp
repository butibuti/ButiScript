#include "stdafx.h"

#include "Compiler.h"

int main()
{
	ButiScript::Data data;

	bool compile_result;
	{
		ButiScript::Compiler driver;
		compile_result = driver.Compile("input.bs", data);
	}
	if (compile_result) {
		ButiScript::VirtualCPU machine(data);
		machine.Initialize();
		int result = machine.Run();
		std::cout << "result = " << result << std::endl;
		std::system("pause");
	}

	return 0;
}