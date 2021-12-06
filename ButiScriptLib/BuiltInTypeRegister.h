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
	/// �g�ݍ��݊֐��̓o�^
	/// </summary>
	/// <param name="arg_op">�֐��̃|�C���^</param>
	/// <param name="retType">�Ԃ�l�̌^</param>
	/// <param name="name">�֐���</param>
	/// <param name="args">�����̑g�ݍ��킹</param>
	/// <returns>����</returns>
	bool DefineSystemFunction(SysFunction arg_op, const int retType, const std::string& name, const std::string& args, const std::vector<int>& arg_vec_templateTypes);
	inline bool DefineSystemFunction(SysFunction arg_op, const int retType, const std::string& name, const std::string& args) {
		static auto temps= std::vector<int>();
		return DefineSystemFunction(arg_op, retType, name, args, temps);
	}
	bool DefineSystemMethod(SysFunction arg_p_method, const int type, const int retType, const std::string& name, const std::string& arg_args,const std::vector<int>& arg_vec_templateTypes);
	inline bool DefineSystemMethod(SysFunction arg_p_method, const int type, const int retType, const std::string& name, const std::string& arg_args) {
		static auto temps = std::vector<int>();
		return DefineSystemMethod(arg_p_method, type,retType, name,arg_args, temps);
	}

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
	/// �g�ݍ��݌^�̓o�^
	/// </summary>
	/// <typeparam name="T">�^���</typeparam>
	/// <param name="arg_name">�^��</param>
	/// <param name="arg_argmentName">�����Ɏg������</param>
	/// <param name="memberInfo">�����o���</param>
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
					// "�g�ݍ��݌^�̃����o�ϐ��̎w�肪�Ԉ���Ă��܂�"
					assert(0);
				}
				auto memberTypeIndex = types.GetArgmentKeyMap().at(typeSplited[1]);
				MemberValueInfo info = { i,memberTypeIndex ,AccessModifier::Public };
				type.map_memberValue.emplace(typeSplited[0], info);

			}
		}
		types.RegistType(type);
		//�X�N���v�g��`�̌^�������o�Ƃ��ė��p����^�̓o�^
		PushCreateMemberInstance<T>();

	}

	template <typename T>
	void RegistSharedSystemType(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		TypeTag type;
		int index = Value::SetTypeIndex(TypeSpecific<T>());
		type.isSystem = true;
		type.isShared = true;
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
					// "�g�ݍ��݌^�̃����o�ϐ��̎w�肪�Ԉ���Ă��܂�"
					assert(0);
				}
				auto memberTypeIndex = types.GetArgmentKeyMap().at(typeSplited[1]);
				MemberValueInfo info = { i,memberTypeIndex ,AccessModifier::Public };
				type.map_memberValue.emplace(typeSplited[0], info);

			}
		}
		types.RegistType(type);
		//�X�N���v�g��`�̌^�������o�Ƃ��ė��p����^�̓o�^
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
