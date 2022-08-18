#ifndef _BUTI_DECL_H
#define _BUTI_DECL_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include"ButiMemorySystem/ButiMemorySystem/ButiPtr.h"
#include"ButiMemorySystem/ButiMemorySystem/ButiList.h"
#include"Tags.h"
namespace ButiScript {

class NameSpace;
class Class;
class Block;
class Function;
using NameSpace_t = ButiEngine::Value_ptr<NameSpace>;
using Class_t = ButiEngine::Value_ptr<Class>;
using Block_t = ButiEngine::Value_ptr<Block>;
using Function_t = ButiEngine::Value_ptr<Function>;
// ä÷êî
class Function :public ButiEngine::enable_value_from_this<Function> {
public:
	Function(const std::string& arg_name)
		: name(arg_name), searchName(arg_name)
	{
		accessType = AccessModifier::Public;
	}
	Function(const std::string& arg_name, const AccessModifier arg_access)
		: name(arg_name)
	{
		if (arg_access == AccessModifier::Public || arg_access == AccessModifier::Private || arg_access == AccessModifier::Protected) {
			accessType = arg_access;
		}
	}

	void Add(ArgDefine arg_argDefine)
	{
		args.Add(arg_argDefine);
	}

	void Add(Block_t arg_block)
	{
		block = arg_block;
	}
	std::int32_t PushCompiler(Compiler* arg_compiler, FunctionTable* arg_p_funcTable = nullptr);
	std::int32_t PushCompiler_sub(Compiler* arg_compiler);
	virtual std::int32_t Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable = nullptr);
	void SetReturnTypeName(const std::string& arg_name) {
		returnTypeName = arg_name;
	}
	void AddSubFunction(Function_t);
	const std::string& GetName()const {
		return name;
	}
	void SetParent(Function_t arg_function) { parentFunction = arg_function; }
	virtual void LambdaCapture(Compiler* arg_compiler) {}
	void SetAccess(const AccessModifier arg_access) {
		if (arg_access == AccessModifier::Public || arg_access == AccessModifier::Private || arg_access == AccessModifier::Protected) {
			accessType = arg_access;
		}
	}
protected:
	Function() {}
	std::string name, searchName,returnTypeName;
	ButiEngine::List<ArgDefine> args;
	AccessModifier accessType = AccessModifier::Public;
	Block_t block;
	ButiEngine::List<Function_t> list_subFunctions;
	Function_t parentFunction;
	NameSpace_t ownNameSpace;
};

class Lambda :public Function {
public:
	Lambda(const std::string& arg_retTypeName, const ButiEngine::List<ArgDefine>& arg_args, Compiler* arg_compiler);
	std::int32_t PushCompiler(Compiler* arg_compiler);
	std::int32_t Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable = nullptr);
	void LambdaCapture(Compiler* arg_compiler)override;
private:
	std::int32_t lambdaIndex;
	std::map<std::string, const ValueTag*> map_lambdaCapture;
};
using Lambda_t = ButiEngine::Value_ptr<Lambda>;

class Class :public ButiEngine::enable_value_from_this<Class> {
public:
	Class(const std::string& arg_name)
		: name(arg_name)
	{
	}
	std::int32_t Analyze(Compiler* arg_compiler);
	std::int32_t Regist(Compiler* arg_compiler);
	std::int32_t PushCompiler(Compiler* arg_compiler);
	void RegistMethod(Function_t arg_method, Compiler* arg_compiler);
	void SetValue(const std::string& arg_name, const std::string& arg_typeName, const AccessModifier arg_accessType);
private:
	std::map < std::string, std::pair< std::string, AccessModifier>> map_values;
	ButiEngine::List<Function_t> list_methods;
	std::string name;
};
using Class_t = ButiEngine::Value_ptr<Class>;

//óÒãìå^
class Enum {
public:
	Enum(const std::string& arg_typeName) :typeName(arg_typeName) {}

	void SetIdentifer(const std::string& arg_name);
	void SetIdentifer(const std::string& arg_name, const std::int32_t arg_value);
	std::int32_t Analyze(Compiler* arg_compiler);
private:
	std::string typeName;
	std::map<std::string, std::int32_t> map_identifer;
};
using Enum_t = ButiEngine::Value_ptr<Enum>;
}

#endif // !_BUTI_DECL_H
