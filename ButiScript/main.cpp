#include "stdafx.h"
class Sample {
public:
	int TestMethod() {
		std::cout << "Sample::TestMethod() is Called!"<<std::endl;
		return 0;
	}
};
auto shp_sample = std::make_shared<Sample>();
std::shared_ptr<Sample> CreateSample() {
	return shp_sample;
}
#include"BuiltInTypeRegister.h"
#include "Compiler.h"


int main(const int argCount, const char* args[])
{
	ButiScript::Compiler driver;
	ButiScript::SystemTypeRegister::GetInstance()->RegistSharedSystemType<Sample>("Sample", "Sample");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method_ret< Sample, int, &Sample::TestMethod, &ButiScript::VirtualMachine::GetSharedTypePtr  >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), TYPE_INTEGER, "TestMethod", "");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func_ret<std::shared_ptr<Sample>,&CreateSample >,ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), "CreateSample", "");

	driver.RegistDefaultSystems(); 
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
			ButiScript::VirtualMachine* p_clone; 
			int returnCode=0;
			{

				ButiScript::VirtualMachine machine(data);

				machine.Initialize();
				machine.AllocGlobalValue();
				//machine.SetGlobalVariable(1, "g_i");
				//machine.SetGlobalVariable(2, "g_i1");

				std::cout << args[i] << "のmain実行" << std::endl;
				std::cout << "////////////////////////////////////" << std::endl;
				returnCode = machine.Execute<int,int ,int>("main",100,50);
				std::cout << "////////////////////////////////////" << std::endl;
				std::cout << args[i] << "のreturn : " << std::to_string(returnCode) << std::endl;

				//p_clone = machine.Clone();
			}

		}
	}

	std::system("pause");



	return 0;
}