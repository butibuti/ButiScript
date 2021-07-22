#include "stdafx.h"

#include "Compiler.h"


int main()
{
	ButiScript::Compiler driver;
	std::shared_ptr< ButiScript::CompiledData> data=std::make_shared<ButiScript::CompiledData>();
	bool compile_result=true;

	{
		driver.RegistDefaultSystems();
		driver.RegistSystemType<Sample>("Sample", "Sample");
		driver.RegistSharedSystemType<Sample>("Sample_t", "Sample_t");
		driver.DefineSystemFunction(&ButiScript::VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "Sample");
		driver.DefineSystemMethod(&ButiScript::VirtualCPU::sys_method_retNo< Sample, &Sample::SampleMethod, &ButiScript::VirtualCPU::GetSharedTypePtr >, TYPE_VOID + 5, TYPE_VOID, "SampleMethod", "");
		compile_result = driver.Compile("input.bs", *data);
		driver.OutputCompiledData("output/compiled.cbs", *data);
	}
	if (compile_result) {
		data = std::make_shared<ButiScript::CompiledData>();
		driver.InputCompiledData("output/compiled.cbs", *data);

		ButiScript::VirtualCPU machine(data);
		machine.Initialize();
		machine.AllocGlobalValue();
		auto retunCode = machine.Execute<ButiEngine::Vector2>("main");
		std::cout << "return : " << std::to_string(retunCode) << std::endl;
		std::system("pause");
	}



	return 0;
}