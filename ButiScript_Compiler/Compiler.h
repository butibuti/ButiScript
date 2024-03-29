#ifndef BUTISCRIPTCOMPILER_H
#define BUTISCRIPTCOMPILER_H
#include"Common.h"
#include "Declaration.h"
#include<unordered_map>
#include"ButiUtil/ButiUtil/Helper/StringHelper.h"
#include"../ButiScriptLib/VM_enum.h"
#include"../ButiScriptLib/CompiledData.h"
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
	void PushValue(Declaration_t arg_value);
	void PushFunction(Function_t arg_func);
	void PushClass(Class_t arg_class);
	
	void AnalyzeFunctions(Compiler* arg_compiler);
	void AnalyzeClasses(Compiler* arg_compiler);
	void DeclareValues(Compiler* arg_compiler);
	void DeclareFunctions(Compiler* arg_compiler);
	void DeclareClasses(Compiler* arg_compiler);
	void Clear();
private:
	std::string name;
	NameSpace_t vlp_parentNamespace;
	ButiEngine::List<Function_t> list_analyzeFunctionBuffer;
	ButiEngine::List<Class_t> list_analyzeClassBuffer;
	ButiEngine::List<Declaration_t> list_analyzeGlobalValueBuffer;
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
	BUTISCRIPT_CMP_API Compiler();
	BUTISCRIPT_CMP_API virtual ~Compiler();

	BUTISCRIPT_CMP_API static void CreateBaseInstance();
	BUTISCRIPT_CMP_API static Compiler* GetBaseInstance();

	/// <summary>
	/// デフォルトの組み込み型、組み込み関数の設定
	/// </summary>
	BUTISCRIPT_CMP_API void RegistDefaultSystems();
	/// <summary>
	/// コンパイル
	/// </summary>
	/// <param name="arg_filePath">ファイルパス</param>
	/// <param name="arg_ref_data">コンパイル済みデータの出力先</param>
	/// <returns>成功/失敗</returns>
	BUTISCRIPT_CMP_API bool Compile(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data);
	/// <summary>
	/// コンパイル済みデータのファイル出力
	/// </summary>
	/// <param name="arg_filePath">ファイルパス</param>
	/// <param name="arg_ref_data">コンパイル済みデータ</param>
	/// <returns>成功/失敗</returns>
	BUTISCRIPT_CMP_API std::int32_t OutputCompiledData(const std::string& arg_filePath, const ButiScript::CompiledData& arg_ref_data);
	/// <summary>
	/// コンパイル済みデータのファイル入力
	/// </summary>
	/// <param name="arg_filePath">ファイルパス</param>
	/// <param name="arg_ref_data">コンパイル済みデータの出力先</param>
	/// <returns>成功/失敗</returns>
	BUTISCRIPT_CMP_API std::int32_t InputCompiledData(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data);
	BUTISCRIPT_CMP_API std::int32_t InputCompiledData(const char* arg_dataPtr,const std::int32_t arg_size,ButiScript::CompiledData& arg_ref_data);
	BUTISCRIPT_CMP_API std::int32_t InputCompiledData(ButiEngine::Value_ptr<ButiEngine::IBinaryReader> arg_reader, ButiScript::CompiledData& arg_ref_data);

	BUTISCRIPT_CMP_API void Clear();
	BUTISCRIPT_CMP_API void DebugDump();



	BUTISCRIPT_CMP_API void AnalyzeScriptType(const std::string& arg_typeName, const std::map < std::string, std::pair< std::string, AccessModifier>>& arg_memberInfo);
	BUTISCRIPT_CMP_API TypeTag* RegistScriptType(const std::string& arg_typeName);
	const ButiEngine::List<TypeTag* >& GetSystemTypes()const {
		return types.GetSystemType();
	}

	void ValueDefine(const std::int32_t arg_type, const ButiEngine::List<std::string>& arg_varName,const AccessModifier arg_access);
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
		if (arg_typeName[arg_typeName.size() - 1] == '&') {

			auto tag = types.GetType(StringHelper::Remove(arg_typeName, "&"));
			return tag ? tag->typeIndex|TYPE_REF : -1;
		}
		auto tag = types.GetType(arg_typeName);
		return tag ? tag->typeIndex : -1;
	}
	std::int32_t GetTypeIndex(const std::pair<std::int32_t, ButiEngine::List<ArgDefine>>& arg_funcTypePair) {
		return arg_funcTypePair.first;
	}

	void TemplateTypeStrToTypeIndex(const ButiEngine::List<std::string>& arg_list_str, ButiEngine::List<std::int32_t>& arg_output)const {
		for (auto& argStr : arg_list_str) {
			arg_output.Add(GetTypeIndex(argStr));
		}
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
		std::int32_t old_index = breakIndex;
		breakIndex = arg_label;
		return old_index;
	}
	bool JmpBreakLabel();

	std::int32_t MakeLabel();

	void AddValue(const std::int32_t arg_typeIndex, const std::string& arg_name, const AccessModifier arg_access);

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
	void SetCurrentThisLocation(const std::int32_t arg_location) { 
		currentThisLocation = arg_location; 
	}
	std::int32_t GetCurrentThisLocation()const { 
		return currentThisLocation;
	}
	std::int32_t AddFunction(Function_t arg_function);
	std::int32_t AddValue(Declaration_t arg_valueDecl);
	void SetBuildLogPath(const std::string& arg_path) { m_buildLogPath = arg_path; }
	const std::string& GetBuildLogPath()const { return m_buildLogPath; }
	void SetIsBuildLogOutput(const bool arg_isBuildLog) { m_isBuildLog = arg_isBuildLog; }
	bool GetIsBuildLogOutput()const { return m_isBuildLog; }
	std::uint64_t GetCumCompileTime()const { return m_cumCompileTime; }
	void ResetCumCompileTime() { m_cumCompileTime = 0; }
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

	NameSpace_t currentNameSpace = nullptr, globalNameSpace = nullptr;
	ButiEngine::List<NameSpace_t> list_namespaces;
	std::int32_t breakIndex, errorCount, lambdaCount,currentThisLocation;
	std::uint64_t m_cumCompileTime=0;
	ButiEngine::List< std::string >list_function_name;
	ButiEngine::List<std::int32_t> list_function_type;
	bool m_isBuildLog=false;
	std::string m_buildLogPath="BuildLog.txt";
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