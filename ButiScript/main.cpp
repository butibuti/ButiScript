#include "stdafx.h"
class Sample {
public:
	int count = 0;
	Sample() {
		static int c_s = 0;
		count = c_s;
		c_s++;
	}
	~Sample() {
		int i = 0;
	}
	int TestMethod() {
		std::cout << "Sample::TestMethod() is Called! count:"<<count<<std::endl;
		return count;
	}
	template <typename T>
	T TemplateMethod(const std::string& str) {
		return 1.5f;
	}
};
std::shared_ptr<Sample> CreateSample() {
	return std::make_shared<Sample>();
}

template<typename T> 
T CreateInstance() { 
	return T(); 
}
#include"BuiltInTypeRegister.h"
#include "Compiler.h"


int main(const int argCount, const char* args[])
{
	ButiScript::Compiler driver;
	ButiScript::SystemTypeRegister::GetInstance()->RegistSharedSystemType<Sample>("Sample", "Sample");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method_ret< Sample, int, &Sample::TestMethod, &ButiScript::VirtualMachine::GetSharedTypePtr  >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), TYPE_INTEGER, "TestMethod", "");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method_ret< Sample,int,const std::string&, &Sample::TemplateMethod<int>, &ButiScript::VirtualMachine::GetSharedTypePtr ,&ButiScript::VirtualMachine::GetTypePtr >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), TYPE_INTEGER, "TemplateMethod", "s", { TYPE_INTEGER });
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method_ret< Sample,float, const std::string&, &Sample::TemplateMethod<float>, &ButiScript::VirtualMachine::GetSharedTypePtr, &ButiScript::VirtualMachine::GetTypePtr>, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), TYPE_FLOAT, "TemplateMethod", "s", {TYPE_FLOAT});
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func_ret<std::shared_ptr<Sample>, &CreateSample >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), "CreateSample", "");
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func_ret<int, &CreateInstance<int>>, TYPE_INTEGER, "CreateInstance", "", { TYPE_INTEGER });
	ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func_ret<float, &CreateInstance<float>>, TYPE_FLOAT, "CreateInstance", "", {TYPE_FLOAT});

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
				returnCode= machine.Execute<int>("main");
				std::cout << "////////////////////////////////////" << std::endl;
				std::cout << args[i] << "のreturn : " << std::to_string(returnCode) << std::endl;

				//p_clone = machine.Clone();
			}

		}
	}

	std::system("pause");



	return 0;
}