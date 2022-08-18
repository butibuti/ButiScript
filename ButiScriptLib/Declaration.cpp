#include "stdafx.h"
#ifndef BOOST_INCLUDE_H
#define BOOST_INCLUDE_H
#include <boost/bind.hpp>
#include <boost/spirit.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix1_functions.hpp>
#include <boost/spirit/include/phoenix1_new.hpp>
#include <boost/mem_fn.hpp>
#endif
#include "Declaration.h"
#include "Node.h"
#include "compiler.h"

namespace ButiScript {

const char* thisPtrName = "this";
constexpr std::int32_t argmentAddressStart = -4;
void Enum::SetIdentifer(const std::string& arg_name)
{
	map_identifer.emplace(arg_name, map_identifer.size());
}
void Enum::SetIdentifer(const std::string& arg_name, const std::int32_t arg_value)
{
	map_identifer.emplace(arg_name, arg_value);
}
std::int32_t Enum::Analyze(Compiler* arg_compiler)
{
	arg_compiler->RegistEnumType(typeName);
	auto tag = arg_compiler->GetEnumTag(arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + typeName);
	auto end = map_identifer.end();
	for (auto itr = map_identifer.begin(); itr != end; itr++) {
		tag->SetValue(itr->first, itr->second);
	}
	return 0;
}
std::int32_t Class::Analyze(Compiler* arg_compiler)
{
	arg_compiler->AnalyzeScriptType(name, map_values);
	auto typeTag = arg_compiler->GetType(name);
	auto methodTable = &typeTag->methods;
	arg_compiler->PushCurrentThisType(typeTag);
	for (auto itr:list_methods) {
		(itr)->PushCompiler(arg_compiler, methodTable);
		(itr)->Analyze(arg_compiler, methodTable);
	}
	arg_compiler->PopCurrentThisType();
	list_methods.Clear();
	return 0;
}

std::int32_t Class::PushCompiler(Compiler* arg_compiler)
{
	arg_compiler->PushAnalyzeClass(value_from_this());
	return 0;
}
std::int32_t Class::Regist(Compiler* arg_compiler)
{
	arg_compiler->RegistScriptType(name);
	return 0;
}
void Class::RegistMethod(Function_t arg_method, Compiler* arg_compiler)
{
	list_methods.Add(arg_method);
	arg_compiler->PopNameSpace();
}
void Class::SetValue(const std::string& arg_name, const std::string& arg_typeName, const AccessModifier arg_accessType)
{
	map_values.emplace(arg_name,std::pair{ arg_typeName,arg_accessType });
}

std::int32_t Function::PushCompiler(Compiler* arg_compiler, FunctionTable* arg_p_funcTable)
{
	ownNameSpace = arg_compiler->GetCurrentNameSpace();
	arg_compiler->PopAnalyzeFunction();
	arg_compiler->RegistFunction(arg_compiler->GetTypeIndex(returnTypeName), name, args, block, accessType, arg_p_funcTable);
	if (!arg_p_funcTable) {
		arg_compiler->GetCurrentNameSpace()->PushFunction(value_from_this());
	}
	searchName = arg_p_funcTable ? name : arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + name;

	return 0;
}

std::int32_t Function::PushCompiler_sub(Compiler* arg_compiler)
{
	ownNameSpace = arg_compiler->GetCurrentNameSpace();
	arg_compiler->PopAnalyzeFunction();
	arg_compiler->RegistFunction(arg_compiler->GetTypeIndex(returnTypeName), name, args, block, accessType);
	searchName = arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + name;
	arg_compiler->PushSubFunction(value_from_this());
	return 0;
}


// 引数の変数名を登録
struct add_value {
	ButiScript::Compiler* p_compiler;
	ButiScript::ValueTable& values;
	std::int32_t addr;
	add_value(ButiScript::Compiler* arg_p_comp, ButiScript::ValueTable& arg_values, const std::int32_t arg_addres = argmentAddressStart) : p_compiler(arg_p_comp), values(arg_values), addr(arg_addres)
	{
	}

	void operator()(const ButiScript::ArgDefine& arg_argDefine) const
	{
		if (!values.add_arg(arg_argDefine.GetType(), arg_argDefine.GetName(), addr)) {
			p_compiler->error("引数 " + arg_argDefine.GetName() + " は既に登録されています。");
		}
	}
};
// キャプチャ変数を登録
struct add_capture {
	ButiScript::Compiler* p_compiler;
	ButiScript::ValueTable& values;
	std::int32_t addr;
	add_capture(ButiScript::Compiler* arg_p_comp, ButiScript::ValueTable& arg_values, const std::int32_t arg_addres = argmentAddressStart) : p_compiler(arg_p_comp), values(arg_values), addr(arg_addres)
	{
	}

	void operator()(const ButiScript::ArgDefine& arg_argDefine) const
	{
		if (!values.add_capture(arg_argDefine.GetType(), arg_argDefine.GetName(), addr)) {
			p_compiler->error("キャプチャ変数 " + arg_argDefine.GetName() + " は既に登録されています。");
		}
	}
};

// 関数の解析
std::int32_t Function::Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable)
{
	auto currentNameSpace = arg_compiler->GetCurrentNameSpace();
	ownNameSpace = ownNameSpace ? ownNameSpace : currentNameSpace;
	arg_compiler->SetCurrentNameSpace(ownNameSpace);
	FunctionTable* p_functable = arg_p_funcTable ? arg_p_funcTable : &arg_compiler->GetFunctions();
	auto returnTypeIndex = arg_compiler->GetTypeIndex(returnTypeName);

	FunctionTag* tag = p_functable->Find_strict(searchName, args, &arg_compiler->GetTypeTable());
	if (tag) {
		if (tag->IsDefinition()) {
			arg_compiler->error("関数 " + searchName + " は既に定義されています");
			return 0;
		}
		if (tag->IsDeclaration() && !tag->CheckArgList_strict(args)) {
			arg_compiler->error("関数 " + searchName + " に異なる型の引数が指定されています");
			return 0;
		}
		tag->SetDefinition();	// 定義済みに設定
	}
	else {
		FunctionTag func(returnTypeIndex, searchName);
		func.SetArgs(args);				// 引数を設定
		func.SetDefinition();			// 定義済み
		func.SetIndex(arg_compiler->MakeLabel());		// ラベル登録
		tag = p_functable->Add(searchName, func, &arg_compiler->GetTypeTable());
		if (tag == nullptr)
			arg_compiler->error("プログラムエラー：関数テーブルに登録できません");
	}

	arg_compiler->PushCurrentFunctionName(searchName);		// 処理中の関数名を登録
	arg_compiler->PushCurrentFunctionType(returnTypeIndex);		// 処理中の関数型を登録

	// 関数のエントリーポイントにラベルを置く

	arg_compiler->SetLabel(tag->GetIndex());

	arg_compiler->BlockIn(false, true);		// 変数スタックを増やす

	// 引数リストを登録
	std::int32_t address = argmentAddressStart;
	//メンバ関数の場合thisを引数に追加
	if (arg_p_funcTable) {
		ArgDefine argDef(arg_compiler->GetCurrentThisType()->typeIndex, thisPtrName);
		add_value(arg_compiler, arg_compiler->GetValueTable().GetLast(), address)(argDef);
		address--;
	}

	for (auto itr = args.rbegin(), endItr = args.rend(); itr != endItr; itr++) {
		add_value(arg_compiler, arg_compiler->GetValueTable().GetLast(), address)(*itr);
		address--;
	}
	arg_compiler->ValueAddressSubtract(address);

	// 文があれば、文を登録
	if (block) {
		std::int32_t ret = block->Analyze(arg_compiler, list_subFunctions);
	}

	const VMCode& code = arg_compiler->GetStatement().GetLast();
	if (returnTypeIndex == TYPE_VOID) {
		if (code.op != VM_RETURN)		// returnが無ければreturnを追加
			arg_compiler->OpReturn();
	}
	else {
		if (code.op != VM_RETURNV) {
			arg_compiler->error("関数 " + searchName + " の最後にreturn文が有りません。");
		}
	}


	for (auto itr = list_subFunctions.begin(), end = list_subFunctions.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler, nullptr);
	}
	list_subFunctions.Clear();
	arg_compiler->ValueAddressAddition(-address);
	arg_compiler->BlockOut();		// 変数スタックを減らす

	arg_compiler->PopCurrentFunctionName();		// 処理中の関数名を消去
	arg_compiler->PopCurrentFunctionType();
	arg_compiler->SetCurrentNameSpace(currentNameSpace);
	return 0;
}

void Function::AddSubFunction(Function_t arg_function)
{
	list_subFunctions.Add(arg_function);
}


Lambda::Lambda(const std::string& arg_retTypeName, const ButiEngine::List<ArgDefine>& arg_list_argDefine, Compiler* arg_compiler)
{
	returnTypeName = arg_retTypeName;
	args = arg_list_argDefine;

	lambdaIndex = arg_compiler->GetLambdaCount();
	arg_compiler->IncreaseLambdaCount();
	name = "@lambda:" + std::to_string(lambdaIndex);
}
std::int32_t Lambda::PushCompiler(Compiler* arg_compiler)
{
	auto typeTag = arg_compiler->GetType(returnTypeName);

	arg_compiler->PopAnalyzeFunction();
	arg_compiler->RegistLambda(typeTag->GetFunctionObjectReturnType(), name, args, nullptr);
	searchName = arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + name;

	arg_compiler->PushSubFunction(value_from_this());

	return lambdaIndex;
}
std::int32_t Lambda::Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable)
{
	auto typeTag = arg_compiler->GetType(returnTypeName);
	auto returnType = typeTag->GetFunctionObjectReturnType();
	auto currentNameSpace = arg_compiler->GetCurrentNameSpace();
	ownNameSpace = ownNameSpace ? ownNameSpace : currentNameSpace;
	arg_compiler->SetCurrentNameSpace(ownNameSpace);
	FunctionTable* p_functable = arg_p_funcTable ? arg_p_funcTable : &arg_compiler->GetFunctions();


	FunctionTag* tag = p_functable->Find_strict(searchName, args, &arg_compiler->GetTypeTable());
	if (tag) {
		if (tag->IsDefinition()) {
			arg_compiler->error("関数 " + searchName + " は既に定義されています");
			return 0;
		}
		if (tag->IsDeclaration() && !tag->CheckArgList_strict(args)) {
			arg_compiler->error("関数 " + searchName + " に異なる型の引数が指定されています");
			return 0;
		}
		tag->SetDefinition();	// 定義済みに設定
	}
	else {
		FunctionTag func(returnType, searchName);
		func.SetArgs(args);				// 引数を設定
		func.SetDefinition();			// 定義済み
		func.SetIndex(arg_compiler->MakeLabel());		// ラベル登録
		tag = p_functable->Add(searchName, func, &arg_compiler->GetTypeTable());
		if (tag == nullptr)
			arg_compiler->error("プログラムエラー：関数テーブルに登録できません");
	}

	arg_compiler->PushCurrentFunctionName(searchName);		// 処理中の関数名を登録
	arg_compiler->PushCurrentFunctionType(returnType);		// 処理中の関数型を登録

	// 関数のエントリーポイントにラベルを置く

	arg_compiler->SetLabel(tag->GetIndex());

	arg_compiler->BlockIn(false, true);		// 変数スタックを増やす

	// 引数リストを登録
	std::int32_t address = argmentAddressStart;
	//メンバ関数の場合thisを引数に追加
	if (arg_p_funcTable) {
		ArgDefine argDef(arg_compiler->GetCurrentThisType()->typeIndex, thisPtrName);
		add_value(arg_compiler, arg_compiler->GetValueTable().GetLast(), address)(argDef);
		address--;
	}

	for (auto itr = args.rbegin(), endItr = args.rend(); itr != endItr; itr++) {
		add_value(arg_compiler, arg_compiler->GetValueTable().GetLast(), address)(*itr);
		address--;
	}
	arg_compiler->ValueAddressSubtract(address);

	// 文があれば、文を登録
	if (block) {

		///キャプチャする変数を確保
		std::int32_t i = 0;
		for (auto itr = map_lambdaCapture.begin(), end = map_lambdaCapture.end(); itr != end; i++, itr++) {
			add_capture(arg_compiler, arg_compiler->GetValueTable().GetLast(), i)(ArgDefine(itr->second->valueType, arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + itr->first));
			//tag->list_captureList.push_back(itr->second->GetAddress());
		}

		arg_compiler->BlockIn();
		///
		std::int32_t ret = block->Analyze(arg_compiler, list_subFunctions);
		arg_compiler->BlockOut();
	}

	const VMCode& code = arg_compiler->GetStatement().GetLast();
	if (returnType == TYPE_VOID) {
		if (code.op != VM_RETURN)		// returnが無ければreturnを追加
			arg_compiler->OpReturn();
	}
	else {
		if (code.op != VM_RETURNV) {
			arg_compiler->error("関数 " + searchName + " の最後にreturn文が有りません。");
		}
	}


	for (auto itr = list_subFunctions.begin(), end = list_subFunctions.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler, nullptr);
	}
	list_subFunctions.Clear();
	arg_compiler->ValueAddressAddition(-address);
	arg_compiler->BlockOut();		// 変数スタックを減らす

	arg_compiler->PopCurrentFunctionName();		// 処理中の関数名を消去
	arg_compiler->PopCurrentFunctionType();
	arg_compiler->SetCurrentNameSpace(currentNameSpace);

	return lambdaIndex;
}
void Lambda::LambdaCapture(Compiler* arg_compiler)
{
	if (block) {

		FunctionTable* p_functable = &arg_compiler->GetFunctions();


		FunctionTag* tag = p_functable->Find_strict(searchName, args, &arg_compiler->GetTypeTable());
		block->LambdaCapture(map_lambdaCapture, arg_compiler);

		///キャプチャする変数を確保
		std::int32_t i = 0;
		for (auto itr = map_lambdaCapture.begin(), end = map_lambdaCapture.end(); itr != end; i++, itr++) {
			tag->list_captureList.Add(itr->second->GetAddress());
		}

	}
}
}