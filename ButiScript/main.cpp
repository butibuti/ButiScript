#include "stdafx.h"
#include"BuiltInTypeRegister.h"
#include "Compiler.h"


int main(const int argCount, const char* args[])
{
	ButiScript::Compiler driver;
	driver.RegistDefaultSystems();
	//driver.RegistSystemType<Sample>("Sample", "Sample");
	//driver.DefineSystemFunction(&ButiScript::VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "Sample"); 
	bool compile_result=false;
	for(int i=1;i<argCount;i++)
	{
		std::shared_ptr< ButiScript::CompiledData> data = std::make_shared<ButiScript::CompiledData>();
		compile_result = driver.Compile(args[i], *data);
		if (compile_result) {
			std::cout << args[i]<<"のコンパイル成功" << std::endl;
			driver.OutputCompiledData(StringHelper::GetDirectory(args[i])+"/output/"+ StringHelper::GetFileName(args[i],false)+ ".cbs", *data);
		}
		else {
			std::cout << args[i] << "のコンパイル失敗" << std::endl;
		}
		data = std::make_shared<ButiScript::CompiledData>();
		auto res= driver.InputCompiledData(StringHelper::GetDirectory(args[i]) + "/output/" + StringHelper::GetFileName(args[i], false) + ".cbs", *data);
		if (res) {

			ButiScript::VirtualCPU machine(data);
			machine.Initialize();
			machine.AllocGlobalValue();
			//machine.SetGlobalVariable(1, "g_i");
			//machine.SetGlobalVariable(2, "g_i1");

			std::cout << args[i] << "のmain実行" << std::endl;
			std::cout  << "////////////////////////////////////" << std::endl;
			auto retunCode = machine.Execute<int>("main");
			std::cout << "////////////////////////////////////" << std::endl;
			std::cout << args[i] << "のreturn : " << std::to_string(retunCode) << std::endl;
		}
	}

	std::system("pause");



	return 0;
}