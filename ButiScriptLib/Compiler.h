#ifndef BUTISCRIPTCOMPILER_H
#define BUTISCRIPTCOMPILER_H

#include "VirtualMachine.h"
#include "Node.h"
#include<unordered_map>
#include"ButiUtil/ButiUtil/Helper/StringHelper.h"
#include<typeinfo>
namespace ButiScript {



// 仮想マシンコード生成

class VMCode {
public:
	VMCode(const std::uint8_t arg_operation)
		: size(sizeof(std::uint8_t)), op(arg_operation)
	{
	}
	VMCode(const std::uint8_t arg_operation, const std::int32_t arg_constValue)
		: size(sizeof(std::uint8_t)+sizeof(std::int32_t)), op(arg_operation)
	{
		constType = TYPE_INTEGER;
		p_constValue = new std::int32_t(arg_constValue);
	}
	VMCode(const std::uint8_t arg_operation, const float arg_constValue)
		: size(sizeof(std::uint8_t) + sizeof(float)), op(arg_operation)
	{
		constType = TYPE_FLOAT;
		p_constValue = new float(arg_constValue);
	}

	~VMCode() {
	}

	void Release() {
		if (p_constValue) {
			delete p_constValue;
		}
	}

	std::uint8_t* Get(std::uint8_t* p) const
	{
		if (op != VM_MAXCOMMAND) {			// ラベルのダミーコマンド
			*p++ = op;
			if (size > 1) {
				if (p_constValue) {
					switch (constType)
					{
					case TYPE_INTEGER:
						*(std::int32_t*)p = *((std::int32_t*)p_constValue);

						p += 4;
						break;
					case TYPE_FLOAT:
						*(float*)p = *((float*)p_constValue);

						p += 4;
						break;
					default:
						break;
					}

				}
				else {
				}
			}
		}
		return p;
	}
	template<typename T>
	T  GetConstValue()const {
		assert(p_constValue);
		return *((T*)p_constValue);
	}
	template<typename T>
	void  SetConstValue(const T arg_v){
		if (p_constValue) {
			delete p_constValue;
		}
		p_constValue = new T(arg_v);
	}

public:
	std::uint8_t size;
	std::uint8_t op;
	std::int32_t constType;
	
	
	//定数のポインタ
	void* p_constValue =nullptr;

};

//名前空間の定義
class NameSpace:ButiEngine::enable_value_from_this<NameSpace> {
public:
	NameSpace(const std::string& arg_name) :name(arg_name) {	}

	const std::string& GetNameString()const;
	std::string GetGlobalNameString()const;
	void Regist(Compiler* arg_compiler);
	void SetParent(NameSpace_t arg_parent);
	NameSpace_t GetParent()const;
	void PushFunction(Function_t arg_func);
	void PushClass(Class_t arg_class);
	
	void AnalyzeFunctions(Compiler* c);
	void AnalyzeClasses(Compiler* c);
	void Clear();
private:
	std::string name;
	NameSpace_t vlp_parentNamespace;
	ButiEngine::List<Function_t> list_analyzeFunctionBuffer;
	ButiEngine::List<Class_t> list_analyzeClassBuffer;
};



// ラベル

class Label {
public:
	Label(const std::int32_t arg_index)
		: index(arg_index), pos(0)
	{
	}
	~Label()
	{
	}

public:
	std::int32_t index;
	std::int32_t pos;
};


// コンパイラ

class VirtualMachine;
using SysFunction = void (VirtualMachine::*)();



class Compiler {
public:



	Compiler();
	virtual ~Compiler();

	static void CreateBaseInstance();
	static Compiler* GetBaseInstance();

	/// <summary>
	/// デフォルトの組み込み型、組み込み関数の設定
	/// </summary>
	void RegistDefaultSystems();
	/// <summary>
	/// コンパイル
	/// </summary>
	/// <param name="arg_filePath">ファイルパス</param>
	/// <param name="arg_ref_data">コンパイル済みデータの出力先</param>
	/// <returns>成功/失敗</returns>
	bool Compile(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data);
	/// <summary>
	/// コンパイル済みデータのファイル出力
	/// </summary>
	/// <param name="arg_filePath">ファイルパス</param>
	/// <param name="arg_ref_data">コンパイル済みデータ</param>
	/// <returns>成功/失敗</returns>
	std::int32_t OutputCompiledData(const std::string& arg_filePath, const ButiScript::CompiledData& arg_ref_data);
	/// <summary>
	/// コンパイル済みデータのファイル入力
	/// </summary>
	/// <param name="arg_filePath">ファイルパス</param>
	/// <param name="arg_ref_data">コンパイル済みデータの出力先</param>
	/// <returns>成功/失敗</returns>
	std::int32_t InputCompiledData(const std::string& arg_filePath,ButiScript::CompiledData& arg_ref_data);

#ifdef	_DEBUG
	void DebugDump();
#endif



	void AnalyzeScriptType(const std::string& arg_typeName, const std::map < std::string, std::pair< std::int32_t, AccessModifier>>& arg_memberInfo);
	void RegistScriptType(const std::string& arg_typeName);
	const ButiEngine::List<TypeTag* >& GetSystemTypes()const {
		return types.GetSystemType();
	}

	void ValueDefine(const std::int32_t arg_type, const ButiEngine::List<Node_t>& node,const AccessModifier arg_access);
	void FunctionDefine(const std::int32_t arg_type, const std::string& arg_name, const ButiEngine::List<std::int32_t>& arg_list_argIndex);
	FunctionTag* RegistFunction(const std::int32_t arg_type, const std::string& arg_name, const ButiEngine::List<ArgDefine>& arg_list_argDefine, Block_t arg_block, const AccessModifier arg_access,FunctionTable* arg_funcTable=nullptr);
	void RegistLambda(const std::int32_t arg_type, const std::string& arg_name, const ButiEngine::List<ArgDefine>& arg_list_argDefine,FunctionTable* arg_functionTable);
	void RegistEnum(const std::string& arg_typeName, const std::string& arg_identiferName, const std::int32_t arg_value);
	void RegistEnumType(const std::string& arg_typeName);
	const EnumTag* GetEnumTag(const std::string& arg_name) const {
		return enums.FindType(arg_name);
	}
	EnumTag* GetEnumTag(const std::string& arg_name)  {
		return enums.FindType(arg_name);
	}

	// 内側のブロックから変数検索
	const ValueTag* GetValueTag(const std::string& arg_name) const
	{
		std::int32_t size = (std::int32_t)variables.GetSize();
		for (std::int32_t i = size - 1; i >= 0; i--) {
			const ValueTag* tag = variables[i].find(arg_name);
			if (tag)
				return tag;
		}
		return nullptr;
	}

	// 関数の検索
	const FunctionTag* GetFunctionTag(const std::string& arg_name, const ButiEngine::List<std::int32_t>& arg_list_argIndex, const bool arg_isStrict, NameSpace_t arg_namespace) const
	{
		const FunctionTag* output=nullptr;
		const FunctionTag* (FunctionTable::* findMethod)(const std::string&, const ButiEngine::List<std::int32_t>&,const TypeTable* )const;
		if (arg_isStrict) {

			findMethod = &FunctionTable::Find_strict;

		}
		else {
			findMethod = &FunctionTable::Find;
		}
		while (!output) {
			if (!arg_namespace) {
				break;
			}
			output = (functions.*findMethod)(arg_namespace->GetGlobalNameString() + arg_name, arg_list_argIndex,&GetTypeTable());
			arg_namespace = arg_namespace->GetParent();
		}
		return output;
	}
	const FunctionTag* GetFunctionTag(const std::string& arg_name, const ButiEngine::List<std::int32_t>& arg_list_argIndex, const bool arg_isStrict) const {
		return GetFunctionTag(arg_name, arg_list_argIndex, arg_isStrict, GetCurrentNameSpace());
	}
	const FunctionTag* GetFunctionTag(const std::string& arg_name, const ButiEngine::List<std::int32_t>& arg_list_argIndex, const ButiEngine::List<std::int32_t>& arg_list_template,const bool arg_isStrict) const {
		
		return GetFunctionTag(arg_name+ GetTemplateName(arg_list_template, &GetTypeTable()), arg_list_argIndex, arg_isStrict, GetCurrentNameSpace());
	}
	// 関数の検索
	const FunctionTag* GetFunctionTag(const std::string& arg_name, NameSpace_t arg_namespace) const
	{
		const FunctionTag* output=nullptr;
		
		while (!output) {
			if (!arg_namespace) {
				break;
			}
			output = functions.Find(arg_namespace->GetGlobalNameString()+ arg_name);
			arg_namespace = arg_namespace->GetParent();
		}
		return output;
	}
	const FunctionTag* GetFunctionTag(const std::string& arg_name) const {
		return GetFunctionTag(arg_name, GetCurrentNameSpace());
	}
	const FunctionTag* GetFunctionTag(const std::string& arg_name,const ButiEngine::List<std::int32_t>& arg_list_temps) const {
		return GetFunctionTag(arg_name+GetTemplateName(arg_list_temps,& GetTypeTable()), GetCurrentNameSpace());
	}

	const TypeTable& GetTypeTable()const {
		return types;
	}


	//型の検索
	std::int32_t GetTypeIndex(const std::string& arg_typeName)const {
		auto tag = types.GetType(arg_typeName);
		if (tag) {
			return tag->typeIndex;
		}
		return -1;
	}
	std::int32_t GetTypeIndex(const std::pair<std::int32_t, ButiEngine::List<ArgDefine>>& arg_funcTypePair) {
		return arg_funcTypePair.first;
	}
	//関数型の検索
	std::int32_t GetfunctionTypeIndex(const ButiEngine::List<ArgDefine>& arg_list_argmentTypes, const std::int32_t arg_retType);
	std::int32_t GetfunctionTypeIndex(const ButiEngine::List<std::int32_t>& arg_list_argmentTypes, const std::int32_t arg_retType);

	TypeTag* GetType(const std::int32_t arg_index) {
		return types.GetType(arg_index);
	}
	const TypeTag* GetType(const std::int32_t arg_index)const {
		return types.GetType(arg_index);
	}

	TypeTag* GetType(const std::string& arg_name) {
		return types.GetType(arg_name);
	}
	const TypeTag* GetType(const std::string& arg_name)const {
		return types.GetType(arg_name);
	}
	std::int32_t GetSystemTypeSize()const {
		return types.GetSystemTypeSize();
	}
	NameSpace_t GetCurrentNameSpace()const {
		return currentNameSpace;
	}
	void SetCurrentNameSpace(NameSpace_t arg_namespace) {
		currentNameSpace = arg_namespace;
	}

	const TypeTag* GetCurrentThisType()const {
		if (!list_thisType.GetSize()) {
			return nullptr;
		}

		return  list_thisType.GetLast();
	}
	TypeTag* GetCurrentThisType() {
		if (!list_thisType.GetSize()) {
			return nullptr;
		}
		return list_thisType.GetLast();
	}
	void PushCurrentThisType(TypeTag* arg_this) {
		list_thisType.Add(arg_this);
	}
	void PopCurrentThisType() {
		list_thisType.RemoveLast();
	}

	// for code generator.
#define	VM_CREATE
#include "VM_create.h"
#undef	VM_CREATE

	void BlockIn(const bool arg_isFunctionBlock = false, const bool arg_isSubFunctionBlock = false);
	void BlockOut();

	void ValueAddressAddition(const std::int32_t arg_difference);
	void ValueAddressSubtract(const std::int32_t arg_difference);

	void AllocStack();
	std::int32_t LabelSetting();

	std::int32_t SetBreakLabel(const std::int32_t arg_label)
	{
		std::int32_t old_index = break_index;
		break_index = arg_label;
		return old_index;
	}
	bool JmpBreakLabel();

	std::int32_t MakeLabel();

	void AddValue(const std::int32_t arg_typeIndex, const std::string& arg_name, Node_t arg_node, const AccessModifier arg_access);

	void SetLabel(const std::int32_t arg_label);

	void PushString(const std::string& arg_str);
	std::int32_t GetCurrentFunctionType() const { return list_function_type.GetLast(); }
	bool CreateData(ButiScript::CompiledData& arg_ref_data,const std::int32_t arg_codeSize);

	void PushNameSpace(NameSpace_t arg_namespace);
	void PopNameSpace();
	// Error handling.
	void error(const std::string& arg_message);

	void ClearStatement();
	std::string GetTypeName(const std::int32_t arg_type) const;

	void LambdaCountReset();
	std::int32_t GetLambdaCount()const { return lambdaCount; }
	void PushAnalyzeFunction(Function_t arg_function);
	void PopAnalyzeFunction();
	void PushSubFunction(Function_t arg_function);
	void PushAnalyzeClass(Class_t arg_class);
	void ClearNameSpace();
	void Analyze();
	void IncreaseLambdaCount();
	ButiEngine::List<ValueTable>& GetValueTable() { return variables; }
	void PushCurrentFunctionType(const std::int32_t arg_type) { list_function_type.Add(arg_type); }
	void PushCurrentFunctionName(const std::string& arg_name) { list_function_name.Add( arg_name); }
	void PopCurrentFunctionType();
	void PopCurrentFunctionName();
	FunctionTable& GetFunctions() { return functions; }
	const ButiEngine::List<VMCode>& GetStatement()const { return statement; }
	void ClearGlobalNameSpace();
private:

	FunctionTable functions;
	TypeTable types;
	EnumTable enums;
	ButiEngine::List<Function_t> list_parentFunction;
	ButiEngine::List< TypeTag*> list_thisType ;
	ButiEngine::List<ValueTable> variables;
	ButiEngine::List<VMCode> statement;
	ButiEngine::List<Label> labels;
	ButiEngine::List<char> text_table;
	ButiEngine::List<SysFunction> list_sysCalls;
	ButiEngine::List<SysFunction> list_sysMethodCalls;
	ButiEngine::List<SysFunction> list_valueAllocCall;
	ButiEngine::List<SysFunction> list_refValueAllocCall;
	std::map<std::int64_t,std::int32_t> map_sysCallsIndex;
	std::map<std::int64_t, std::int32_t> map_sysMethodCallsIndex;
	std::map<std::int64_t, std::int32_t> map_valueAllocCallsIndex;
	std::map<std::int64_t,std::int32_t> map_refValueAllocCallsIndex;

	NameSpace_t currentNameSpace = nullptr;
	NameSpace_t globalNameSpace = nullptr;
	ButiEngine::List<NameSpace_t> list_namespaces;
	std::int32_t break_index;
	std::int32_t error_count;

	ButiEngine::List< std::string >list_function_name;
	ButiEngine::List<std::int32_t> list_function_type;
	std::int32_t lambdaCount;
};
template<typename T>
struct CompilerSystemTypeRegister {
	CompilerSystemTypeRegister() {
		Compiler::RegistSystemType<T>(typeid(T).name(), typeid(T).name());
	}
};
template<typename T>
struct CompilerSystemSharedTypeRegister {
	CompilerSystemSharedTypeRegister() {
		Compiler::RegistSharedSystemType<T>(typeid(T).name(), typeid(T).name());
	}
};
}
#endif