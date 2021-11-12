#ifndef BUTISCRIPTBUILTINTYPEREGISTER
#define BUTISCRIPTBUILTINTYPEREGISTER
#include "VirtualMachine.h"
#include"../../ButiUtil/Util/Helper/StringHelper.h"
namespace ButiScript {

class VirtualMachine;
using SysFunction = void (VirtualMachine::*)();

class SystemFuntionRegister {
	friend class Compiler;
public:
	static SystemFuntionRegister* GetInstance();
	void SetDefaultFunctions();
	/// <summary>
	/// 組み込み関数の登録
	/// </summary>
	/// <param name="arg_op">関数のポインタ</param>
	/// <param name="retType">返り値の型</param>
	/// <param name="name">関数名</param>
	/// <param name="args">引数の組み合わせ</param>
	/// <returns>成功</returns>
	bool DefineSystemFunction(SysFunction arg_op, const int retType, const std::string& name, const std::string& args);
	bool DefineSystemMethod(SysFunction arg_p_method, const int type, const int retType, const std::string& name, const std::string& arg_args);

private:

	FunctionTable functions;
	std::vector<SysFunction> vec_sysCalls;
	std::vector<SysFunction> vec_sysMethodCalls;
	std::map<long long int, int> map_sysCallsIndex;
	std::map<long long int, int> map_sysMethodCallsIndex;
};

class SystemTypeRegister {
	friend class Compiler;
	friend class SystemFuntionRegister;
public:
	static SystemTypeRegister* GetInstance();

	void SetDefaultSystemType();

	void RegistSystemEnumType(const std::string& arg_typeName);
	void RegistEnum(const std::string& arg_typeName, const std::string& identiferName, const int value);
	/// <summary>
	/// 組み込み型の登録
	/// </summary>
	/// <typeparam name="T">型情報</typeparam>
	/// <param name="arg_name">型名</param>
	/// <param name="arg_argmentName">引数に使う略名</param>
	/// <param name="memberInfo">メンバ情報</param>
	template <typename T>
	void RegistSystemType_(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		TypeTag type;


		int index = Value::SetTypeIndex(TypeSpecific<T>());
		type.isSystem = true;
		map_valueAllocCallsIndex.emplace(index, vec_valueAllocCall.size());
		vec_valueAllocCall.push_back(&VirtualMachine::pushValue<T>);

		map_refValueAllocCallsIndex.emplace(index, vec_refValueAllocCall.size());
		vec_refValueAllocCall.push_back(&VirtualMachine::pushValue_ref<T>);

		type.typeName = arg_name;
		type.typeIndex = index;
		type.argName = arg_argmentName;

		if (memberInfo.size()) {
			auto identiferSplited = StringHelper::Split(memberInfo, ",");

			for (int i = 0; i < identiferSplited.size(); i++) {
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
		PushCreateMemberInstance<T>();

	}

	template <typename T>
	void RegistSharedSystemType(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		TypeTag type;
		int index = Value::SetTypeIndex(TypeSpecific<T>());
		type.isSystem = true;
		map_valueAllocCallsIndex.emplace(index, vec_valueAllocCall.size());
		vec_valueAllocCall.push_back(&VirtualMachine::pushSharedValue<T>);

		map_refValueAllocCallsIndex.emplace(index, vec_refValueAllocCall.size());
		vec_refValueAllocCall.push_back(&VirtualMachine::pushSharedValue_ref<T>);

		type.typeName = arg_name;
		type.typeIndex = index;
		type.argName = arg_argmentName;

		if (memberInfo.size()) {
			auto identiferSplited = StringHelper::Split(memberInfo, ",");

			for (int i = 0; i < identiferSplited.size(); i++) {
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
		PushCreateSharedMemberInstance<T>();
	}
	int GetIndex(const std::string& arg_typeName);
private:
	SystemTypeRegister() {}

	EnumTable enums;
	TypeTable types;
	std::vector<SysFunction> vec_valueAllocCall;
	std::vector<SysFunction> vec_refValueAllocCall;
	std::map<long long int, int> map_valueAllocCallsIndex;
	std::map<long long int, int> map_refValueAllocCallsIndex;
};

}
#endif // !BUTISCRIPTBUILTINTYPEREGISTER
