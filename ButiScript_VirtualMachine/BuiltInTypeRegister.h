#ifndef BUTISCRIPTBUILTINTYPEREGISTER
#define BUTISCRIPTBUILTINTYPEREGISTER
#include"Common.h"
#include "ButiScript/ButiScript_VirtualMachine/VirtualMachine.h"
#include"ButiMemorySystem/ButiMemorySystem/ButiList.h"
#include"ButiUtil/ButiUtil/Helper/StringHelper.h"
namespace ButiScript {

class VirtualMachine;
using SysFunction = void (VirtualMachine::*)();

class SystemFuntionRegister {
	friend class Compiler;
public:
	BUTISCRIPT_VM_API static SystemFuntionRegister* GetInstance();
	BUTISCRIPT_VM_API void SetDefaultFunctions();
	/// <summary>
	/// 組み込み関数の登録
	/// </summary>
	/// <param name="arg_op">関数のポインタ</param>
	/// <param name="retType">返り値の型</param>
	/// <param name="name">関数名</param>
	/// <param name="args">引数の組み合わせ</param>
	/// <returns>成功</returns>
	BUTISCRIPT_VM_API bool DefineSystemFunction(SysFunction arg_op, const std::int32_t retType, const std::string& name, const std::string& args, const ButiEngine::List<std::int32_t>& arg_list_templateTypes);
	inline bool DefineSystemFunction(SysFunction arg_op, const std::int32_t retType, const std::string& name, const std::string& args) {
		static auto temps= ButiEngine::List<std::int32_t>();
		return DefineSystemFunction(arg_op, retType, name, args, temps);
	}
	BUTISCRIPT_VM_API bool DefineSystemMethod(SysFunction arg_p_method, const std::int32_t type, const std::int32_t retType, const std::string& name, const std::string& arg_args,const ButiEngine::List<std::int32_t>& arg_list_templateTypes);
	inline bool DefineSystemMethod(SysFunction arg_p_method, const std::int32_t type, const std::int32_t retType, const std::string& name, const std::string& arg_args) {
		static auto temps = ButiEngine::List<std::int32_t>();
		return DefineSystemMethod(arg_p_method, type,retType, name,arg_args, temps);
	}

private:

	FunctionTable functions;
	ButiEngine::List<SysFunction> list_sysCalls;
	ButiEngine::List<SysFunction> list_sysMethodCalls;
	std::map<std::int64_t, std::int32_t> map_sysCallsIndex;
	std::map<std::int64_t, std::int32_t> map_sysMethodCallsIndex;
};

class SystemTypeRegister {
	friend class Compiler;
	friend class SystemFuntionRegister;
public:
	BUTISCRIPT_VM_API static SystemTypeRegister* GetInstance();

	BUTISCRIPT_VM_API void SetDefaultSystemType();

	BUTISCRIPT_VM_API void RegistSystemEnumType(const std::string& arg_typeName);
	BUTISCRIPT_VM_API void RegistEnum(const std::string& arg_typeName, const std::string& identiferName, const std::int32_t value);
	/// <summary>
	/// 組み込み型の登録
	/// </summary>
	/// <typeparam name="T">型情報</typeparam>
	/// <param name="arg_name">型名</param>
	/// <param name="arg_argmentName">引数に使う略名</param>
	/// <param name="memberInfo">メンバ情報</param>
	template <typename T>
	void RegistSystemType(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		RegistSystemType<T, T>(arg_name, arg_argmentName, memberInfo);
	}
	/// <summary>
	/// 組み込み型の登録
	/// </summary>
	/// <typeparam name="T">型情報</typeparam>
	/// <param name="arg_name">型名</param>
	/// <param name="arg_argmentName">引数に使う略名</param>
	/// <param name="memberInfo">メンバ情報</param>
	template <typename T, typename GenerateType>
	void RegistSystemType(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		TypeTag type;


		std::int32_t index = Value::SetTypeIndex(TypeSpecific<T>());
		type.isSystem = true;
		map_valueAllocCallsIndex.emplace(index, list_valueAllocCall.GetSize());
		list_valueAllocCall.Add(&VirtualMachine::pushValue<T>);

		map_refValueAllocCallsIndex.emplace(index, list_refValueAllocCall.GetSize());
		list_refValueAllocCall.Add(&VirtualMachine::pushValue_ref<T>);

		type.typeName = arg_name;
		type.typeIndex = index;
		type.argName = arg_argmentName;

		if (memberInfo.size()) {
			auto identiferSplited = StringHelper::Split(memberInfo, ",");

			for (std::int32_t i = 0; i < identiferSplited.size(); i++) {
				auto typeSplited = StringHelper::Split(identiferSplited[i], ":");
				if (typeSplited.size() != 2) {
					// "組み込み型のメンバ変数の指定が間違っています"
					assert(0);
				}
				auto memberTypeIndex = types.GetArgmentKeyMap().at(typeSplited[1]);
				MemberValueInfo info = { i,memberTypeIndex ,AccessModifier::Public };
				type.map_memberValue.emplace(typeSplited[0], info);

			}
		}
		types.RegistType(type);
		//スクリプト定義の型がメンバとして利用する型の登録
		PushCreateMemberInstance<GenerateType>();

	}
	
	template <typename T>
	void RegistValueSystemType(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		TypeTag type;


		std::int32_t index = Value::SetTypeIndex(TypeSpecific<T>());
		type.isSystem = true;
		map_valueAllocCallsIndex.emplace(index, list_valueAllocCall.GetSize());
		list_valueAllocCall.Add(&VirtualMachine::pushValue_valueptr< T>);

		map_refValueAllocCallsIndex.emplace(index, list_refValueAllocCall.GetSize());
		list_refValueAllocCall.Add(&VirtualMachine::pushValue_valueptr_ref<T>);

		type.typeName = arg_name;
		type.typeIndex = index;
		type.argName = arg_argmentName;

		if (memberInfo.size()) {
			auto identiferSplited = StringHelper::Split(memberInfo, ",");

			for (std::int32_t i = 0; i < identiferSplited.size(); i++) {
				auto typeSplited = StringHelper::Split(identiferSplited[i], ":");
				if (typeSplited.size() != 2) {
					// "組み込み型のメンバ変数の指定が間違っています"
					assert(0);
				}
				auto memberTypeIndex = types.GetArgmentKeyMap().at(typeSplited[1]);
				MemberValueInfo info = { i,memberTypeIndex ,AccessModifier::Public };
				type.map_memberValue.emplace(typeSplited[0], info);

			}
		}
		types.RegistType(type);
		//スクリプト定義の型がメンバとして利用する型の登録
		PushCreateMemberInstance<ButiEngine::Value_ptr<T>>();

	}

	BUTISCRIPT_VM_API std::int32_t GetIndex(const std::string& arg_typeName);
private:
	SystemTypeRegister() {}

	EnumTable enums;
	TypeTable types;
	ButiEngine::List<SysFunction> list_valueAllocCall;
	ButiEngine::List<SysFunction> list_refValueAllocCall;
	std::map<std::int64_t, std::int32_t> map_valueAllocCallsIndex;
	std::map<std::int64_t, std::int32_t> map_refValueAllocCallsIndex;
};

}
#endif // !BUTISCRIPTBUILTINTYPEREGISTER
