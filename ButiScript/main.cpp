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
		driver.DefineSystemFunction(&ButiScript::VirtualCPU::sys_func_ret<Sample_t, std::make_shared<Sample>>, driver.GetTypeIndex("Sample_t"), "CreateSample_t","");

		driver.DefineSystemMethod(&ButiScript::VirtualCPU::sys_method_retNo< Sample, &Sample::SampleMethod, &ButiScript::VirtualCPU::GetSharedTypePtr >, TYPE_VOID + 5, TYPE_VOID, "SampleMethod", "");
		compile_result = driver.Compile("input.bs", *data);
		driver.OutputCompiledData("output/compiled.cbs", *data);
	}

	data = std::make_shared<ButiScript::CompiledData>();
	driver.InputCompiledData("output/compiled.cbs", *data);

	ButiScript::VirtualCPU machine(data);
	machine.Initialize();
	machine.AllocGlobalValue();
	machine.SetGlobalVariable(1, "g_i");
	machine.SetGlobalVariable(2, "g_i1");
	auto retunCode = machine.Execute<int>("main");
	std::cout << "return : " << std::to_string(retunCode) << std::endl;
	std::system("pause");



	return 0;
}