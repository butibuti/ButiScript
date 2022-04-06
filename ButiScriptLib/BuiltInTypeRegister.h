#ifndef BUTISCRIPTBUILTINTYPEREGISTER
#define BUTISCRIPTBUILTINTYPEREGISTER
#include "VirtualMachine.h"
#include"ButiMemorySystem/ButiMemorySystem/ButiList.h"
#include"ButiUtil/ButiUtil/Helper/StringHelper.h"
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
	bool DefineSystemFunction(SysFunction arg_op, const std::int32_t retType, const std::string& name, const std::string& args, const ButiEngine::List<std::int32_t>& arg_list_templateTypes);
	inline bool DefineSystemFunction(SysFunction arg_op, const std::int32_t retType, const std::string& name, const std::string& args) {
		static auto temps= ButiEngine::List<std::int32_t>();
		return DefineSystemFunction(arg_op, retType, name, args, temps);
	}
	bool DefineSystemMethod(SysFunction arg_p_method, const std::int32_t type, const std::int32_t retType, const std::string& name, const std::string& arg_args,const ButiEngine::List<std::int32_t>& arg_list_templateTypes);
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
	static SystemTypeRegister* GetInstance();

	void SetDefaultSystemType();

	void RegistSystemEnumType(const std::string& arg_typeName);
	void RegistEnum(const std::string& arg_typeName, const std::string& identiferName, const std::int32_t value);
	/// <summary>
	/// �g�ݍ��݌^�̓o�^
	/// </summary>
	/// <typeparam name="T">�^���</typeparam>
	/// <param name="arg_name">�^��</param>
	/// <param name="arg_argmentName">�����Ɏg������</param>
	/// <param name="memberInfo">�����o���</param>
	template <typename T>
	void RegistSystemType(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		RegistSystemType<T, T>(arg_name, arg_argmentName, memberInfo);
	}
	/// <summary>
	/// �g�ݍ��݌^�̓o�^
	/// </summary>
	/// <typeparam name="T">�^���</typeparam>
	/// <param name="arg_name">�^��</param>
	/// <param name="arg_argmentName">�����Ɏg������</param>
	/// <param name="memberInfo">�����o���</param>
	template <typename T, typename GenerateType>
	void RegistSystemType(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		TypeTag type;


		std::int32_t index = Value::SetTypeIndex(TypeSpecific<T>());
		type.isSystem = true;
		map_valueAllocCallsIndex.emplace(index, vec_valueAllocCall.GetSize());
		vec_valueAllocCall.Add(&VirtualMachine::pushValue<T>);

		map_refValueAllocCallsIndex.emplace(index, vec_refValueAllocCall.GetSize());
		vec_refValueAllocCall.Add(&VirtualMachine::pushValue_ref<T>);

		type.typeName = arg_name;
		type.typeIndex = index;
		type.argName = arg_argmentName;

		if (memberInfo.size()) {
			auto identiferSplited = StringHelper::Split(memberInfo, ",");

			for (std::int32_t i = 0; i < identiferSplited.size(); i++) {
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
		PushCreateMemberInstance<GenerateType>();

	}
	
	template <typename T>
	void RegistValueSystemType(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		TypeTag type;


		std::int32_t index = Value::SetTypeIndex(TypeSpecific<T>());
		type.isSystem = true;
		map_valueAllocCallsIndex.emplace(index, vec_valueAllocCall.GetSize());
		vec_valueAllocCall.Add(&VirtualMachine::pushValue_valueptr< T>);

		map_refValueAllocCallsIndex.emplace(index, vec_refValueAllocCall.GetSize());
		vec_refValueAllocCall.Add(&VirtualMachine::pushValue_valueptr_ref<T>);

		type.typeName = arg_name;
		type.typeIndex = index;
		type.argName = arg_argmentName;

		if (memberInfo.size()) {
			auto identiferSplited = StringHelper::Split(memberInfo, ",");

			for (std::int32_t i = 0; i < identiferSplited.size(); i++) {
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
		PushCreateMemberInstance<ButiEngine::Value_ptr<T>>();

	}

	template <typename T>
	void RegistSharedSystemType(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		TypeTag type;
		std::int32_t index = Value::SetTypeIndex(TypeSpecific<T>());
		type.isSystem = true;
		type.isShared = true;
		map_valueAllocCallsIndex.emplace(index, vec_valueAllocCall.GetSize());
		vec_valueAllocCall.Add(&VirtualMachine::pushValue_sharedptr< T>);

		map_refValueAllocCallsIndex.emplace(index, vec_refValueAllocCall.GetSize());
		vec_refValueAllocCall.Add(&VirtualMachine::pushValue_sharedptr_ref<T>);

		type.typeName = arg_name;
		type.typeIndex = index;
		type.argName = arg_argmentName;

		if (memberInfo.size()) {
			auto identiferSplited = StringHelper::Split(memberInfo, ",");

			for (std::int32_t i = 0; i < identiferSplited.size(); i++) {
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
		PushCreateMemberInstance<std::shared_ptr<T>>();
	}
	std::int32_t GetIndex(const std::string& arg_typeName);
private:
	SystemTypeRegister() {}

	EnumTable enums;
	TypeTable types;
	ButiEngine::List<SysFunction> vec_valueAllocCall;
	ButiEngine::List<SysFunction> vec_refValueAllocCall;
	std::map<std::int64_t, std::int32_t> map_valueAllocCallsIndex;
	std::map<std::int64_t, std::int32_t> map_refValueAllocCallsIndex;
};

}
#endif // !BUTISCRIPTBUILTINTYPEREGISTER
