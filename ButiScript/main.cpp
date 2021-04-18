#include "stdafx.h"

#include "Compiler.h"

int main()
{
	ButiVM::Data data;

	bool compile_result;
	{
		Compiler driver;
		compile_result = driver.Compile("input.bs", data);
	}
	if (compile_result) {
		ButiVM::VirtualCPU machine(data);
		machine.Initialize();
		int result = machine.Run();
		std::cout << "result = " << result << std::endl;
		std::system("pause");
	}

	return 0;
}