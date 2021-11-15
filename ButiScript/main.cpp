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
			std::cout << args[i]<<"�̃R���p�C������" << std::endl;
			driver.OutputCompiledData(StringHelper::GetDirectory(args[i])+"/output/"+ StringHelper::GetFileName(args[i],false)+ ".cbs", *data);
		}
		else {
			std::cout << args[i] << "�̃R���p�C�����s" << std::endl;
		}
		data = std::make_shared<ButiScript::CompiledData>();
		auto res= driver.InputCompiledData(StringHelper::GetDirectory(args[i]) + "/output/" + StringHelper::GetFileName(args[i], false) + ".cbs", *data);
		if (res) {
			ButiScript::VirtualMachine* p_clone; 
			int returnCode;
			{

				ButiScript::VirtualMachine machine(data);

				machine.Initialize();
				machine.AllocGlobalValue();
				//machine.SetGlobalVariable(1, "g_i");
				//machine.SetGlobalVariable(2, "g_i1");

				std::cout << args[i] << "��main���s" << std::endl;
				std::cout << "////////////////////////////////////" << std::endl;
				returnCode = machine.Execute<int>("main");
				std::cout << "////////////////////////////////////" << std::endl;
				std::cout << args[i] << "��return : " << std::to_string(returnCode) << std::endl;

				//p_clone = machine.Clone();
			}

		}
	}

	std::system("pause");



	return 0;
}