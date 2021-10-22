#ifndef BUTISCRIPTCOMPILER_H
#define BUTISCRIPTCOMPILER_H

#include "VirtualMachine.h"
#include "Node.h"
#include<unordered_map>
#include"../../ButiUtil/Util/Helper/StringHelper.h"
#include<typeinfo>
namespace ButiScript {



// 仮想マシンコード生成

class VMCode {
public:
	VMCode(const unsigned char arg_operation)
		: size(sizeof(unsigned char)), op(arg_operation)
	{
	}
	VMCode(const unsigned char arg_operation, const int arg_constValue)
		: size(sizeof(unsigned char)+sizeof(int)), op(arg_operation)
	{
		constType = TYPE_INTEGER;
		p_constValue = new int(arg_constValue);
	}
	VMCode(const unsigned char arg_operation, const float arg_constValue)
		: size(sizeof(unsigned char) + sizeof(float)), op(arg_operation)
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

	unsigned char* Get(unsigned char* p) const
	{
		if (op != VM_MAXCOMMAND) {			// ラベルのダミーコマンド
			*p++ = op;
			if (size > 1) {
				if (p_constValue) {
					switch (constType)
					{
					case TYPE_INTEGER:
						*(int*)p = *((int*)p_constValue);

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
	unsigned char size;
	unsigned char op;
	int constType;
	
	
	//定数のポインタ
	void* p_constValue =nullptr;

};

//名前空間の定義
class NameSpace:std::enable_shared_from_this<NameSpace> {
public:
	NameSpace(const std::string& arg_name) :name(arg_name) {	}

	const std::string& GetNameString()const;
	std::string GetGlobalNameString()const;
	void Regist(Compiler* arg_compiler);
	void SetParent(std::shared_ptr<NameSpace>arg_parent);
	std::shared_ptr<NameSpace> GetParent()const;
	void PushFunction(Function_t arg_func);
	void PushClass(Class_t arg_class);
	
	void AnalyzeFunctions(Compiler* c);
	void AnalyzeClasses(Compiler* c);
private:
	std::string name;
	std::shared_ptr<NameSpace> shp_parentNamespace;
	std::vector<Function_t> vec_analyzeFunctionBuffer;
	std::vector<Class_t> vec_analyzeClassBuffer;
};

using NameSpace_t = std::shared_ptr<NameSpace>;


// ラベル

class Label {
public:
	Label(const int arg_index)
		: index(arg_index), pos(0)
	{
	}
	~Label()
	{
	}

public:
	int index;
	int pos;
};


// コンパイラ

class VirtualCPU;
using SysFunction = void (VirtualCPU::*)();



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
	int OutputCompiledData(const std::string& arg_filePath, const ButiScript::CompiledData& arg_ref_data);
	/// <summary>
	/// コンパイル済みデータのファイル入力
	/// </summary>
	/// <param name="arg_filePath">ファイルパス</param>
	/// <param name="arg_ref_data">コンパイル済みデータの出力先</param>
	/// <returns>成功/失敗</returns>
	int InputCompiledData(const std::string& arg_filePath,ButiScript::CompiledData& arg_ref_data);

#ifdef	_DEBUG
	void DebugDump();
#endif



	void AnalyzeScriptType(const std::string& arg_typeName, const std::map < std::string, std::pair< int, AccessModifier>>& arg_memberInfo);
	void RegistScriptType(const std::string& arg_typeName);
	const std::vector<TypeTag* >& GetSystemTypes()const {
		return types.GetSystemType();
	}

	void ValueDefine(const int arg_type, const std::vector<Node_t>& node,const AccessModifier arg_access);
	void FunctionDefine(const int arg_type, const std::string& arg_name, const std::vector<int>& arg_vec_argIndex);
	void RegistFunction(const int arg_type, const std::string& arg_name, const std::vector<ArgDefine>& arg_vec_argDefine, Block_t arg_block, const AccessModifier arg_access,FunctionTable* arg_funcTable=nullptr);
	void RegistRamda(const int arg_type, const std::string& arg_name, const std::vector<ArgDefine>& arg_vec_argDefine,FunctionTable* arg_functionTable);
	void RegistEnum(const std::string& arg_typeName, const std::string& arg_identiferName, const int arg_value);
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
		int size = (int)variables.size();
		for (int i = size - 1; i >= 0; i--) {
			const ValueTag* tag = variables[i].find(arg_name);
			if (tag)
				return tag;
		}
		return nullptr;
	}

	// 関数の検索
	const FunctionTag* GetFunctionTag(const std::string& arg_name, const std::vector<int>& arg_vec_argIndex, const bool arg_isStrict) const
	{
		if (arg_isStrict) {
			return functions.Find_strict(arg_name, arg_vec_argIndex);
		}
		else {
			return functions.Find(arg_name, arg_vec_argIndex);
		}
	}
	// 関数の検索
	const FunctionTag* GetFunctionTag(const std::string& arg_name) const
	{
		return functions.Find(arg_name);
	}

	//型の検索
	int GetTypeIndex(const std::string& arg_typeName)const {
		auto tag = types.GetType(arg_typeName);
		if (tag) {
			return tag->typeIndex;
		}
		return -1;
	}
	int GetTypeIndex(const std::pair<int, std::vector<ArgDefine>>& arg_funcTypePair) {
		return arg_funcTypePair.first;
	}
	//関数型の検索
	int GetfunctionTypeIndex(const std::vector<ArgDefine>& arg_vec_argmentTypes, const int arg_retType);
	int GetfunctionTypeIndex(const std::vector<int>& arg_vec_argmentTypes, const int arg_retType);

	TypeTag* GetType(const int arg_index) {
		return types.GetType(arg_index);
	}
	const TypeTag* GetType(const int arg_index)const {
		return types.GetType(arg_index);
	}

	TypeTag* GetType(const std::string& arg_name) {
		return types.GetType(arg_name);
	}
	const TypeTag* GetType(const std::string& arg_name)const {
		return types.GetType(arg_name);
	}
	int GetSystemTypeSize()const {
		return types.GetSystemTypeSize();
	}
	NameSpace_t GetCurrentNameSpace()const {
		return currentNameSpace;
	}
	void SetCurrentNameSpace(NameSpace_t arg_namespace) {
		currentNameSpace = arg_namespace;
	}

	const TypeTag* GetCurrentThisType()const {
		if (!vec_thisType.size()) {
			return nullptr;
		}

		return  vec_thisType.back();
	}
	TypeTag* GetCurrentThisType() {
		if (!vec_thisType.size()) {
			return nullptr;
		}
		return vec_thisType.back();
	}
	void PushCurrentThisType(TypeTag* arg_this) {
		vec_thisType.push_back(arg_this);
	}
	void PopCurrentThisType() {
		vec_thisType.pop_back();
	}

	// for code generator.
#define	VM_CREATE
#include "VM_create.h"
#undef	VM_CREATE

	void BlockIn(const bool arg_isFunctionBlock = false, const bool arg_isSubFunctionBlock = false);
	void BlockOut();

	void ValueAddressAddition(const int arg_difference);
	void ValueAddressSubtract(const int arg_difference);

	void AllocStack();
	int LabelSetting();

	int SetBreakLabel(const int arg_label)
	{
		int old_index = break_index;
		break_index = arg_label;
		return old_index;
	}
	bool JmpBreakLabel();

	int MakeLabel();

	void AddValue(const int arg_typeIndex, const std::string& arg_name, Node_t arg_node, const AccessModifier arg_access);

	void SetLabel(const int arg_label);

	void PushString(const std::string& arg_str);
	int GetCurrentFunctionType() const { return vec_function_type.back(); }
	bool CreateData(ButiScript::CompiledData& arg_ref_data,const int arg_codeSize);

	void PushNameSpace(NameSpace_t arg_namespace);
	void PopNameSpace();
	// Error handling.
	void error(const std::string& arg_message);

	void ClearStatement();
	std::string GetTypeName(const int arg_type) const;

	void RamdaCountReset();
	int GetRamdaCount()const { return ramdaCount; }
	void PushAnalyzeFunction(Function_t arg_function);
	void PopAnalyzeFunction();
	void PushSubFunction(Function_t arg_function);
	void PushAnalyzeClass(Class_t arg_class);
	void ClearNameSpace();
	void Analyze();
	void IncreaseRamdaCount();
	std::vector<ValueTable>& GetValueTable() { return variables; }
	void PushCurrentFunctionType(const int arg_type) { vec_function_type.push_back(arg_type); }
	void PushCurrentFunctionName(const std::string& arg_name) { vec_function_name.push_back( arg_name); }
	void PopCurrentFunctionType();
	void PopCurrentFunctionName();
	FunctionTable& GetFunctions() { return functions; }
	const std::vector<VMCode>& GetStatement()const { return statement; }
	
private:

	FunctionTable functions;
	TypeTable types;
	EnumTable enums;
	std::vector<Function_t> vec_parentFunction;
	std::vector< TypeTag*> vec_thisType ;
	std::vector<ValueTable> variables;
	std::vector<VMCode> statement;
	std::vector<Label> labels;
	std::vector<char> text_table;
	std::vector<SysFunction> vec_sysCalls;
	std::vector<SysFunction> vec_sysMethodCalls;
	std::vector<SysFunction> vec_valueAllocCall;
	std::vector<SysFunction> vec_refValueAllocCall;
	std::map<long long int,int> map_sysCallsIndex;
	std::map<long long int, int> map_sysMethodCallsIndex;
	std::map<long long int, int> map_valueAllocCallsIndex;
	std::map<long long int,int> map_refValueAllocCallsIndex;

	NameSpace_t currentNameSpace = nullptr;
	NameSpace_t globalNameSpace = nullptr;
	std::vector<NameSpace_t> vec_namespaces;
	int break_index;
	int error_count;

	std::vector< std::string >vec_function_name;
	std::vector<int> vec_function_type;
	int ramdaCount;
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