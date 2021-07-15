#include "stdafx.h"

#include "Compiler.h"


int main()
{
	ButiScript::Compiler driver;
	std::shared_ptr< ButiScript::CompiledData> data=std::make_shared<ButiScript::CompiledData>();
	bool compile_result=true;
	ButiEngine::Vector2 v = ButiEngine::Vector2(50,25);
	float f = v.GetLength();
	{
		driver.RegistDefaultSystems();
		compile_result = driver.Compile("input.bs", *data);
		driver.OutputCompiledData("output/compiled.cbs", *data);
	}
	if (compile_result) {
		data = std::make_shared<ButiScript::CompiledData>();
		driver.InputCompiledData("output/compiled.cbs", *data);

		ButiScript::VirtualCPU machine(data);
		machine.Initialize();
		machine.AllocGlobalValue();
		auto retunCode = machine.Execute<int>("main");
		std::cout << "return : " << retunCode << std::endl;
		std::system("pause");
	}



	return 0;
}