#ifndef BUTISCRIPTBUILTINTYPEREGISTER
#define BUTISCRIPTBUILTINTYPEREGISTER
#include "VirtualMachine.h"
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
	/// ‘g‚İ‚İŠÖ”‚Ì“o˜^
	/// </summary>
	/// <param name="arg_op">ŠÖ”‚Ìƒ|ƒCƒ“ƒ^</param>
	/// <param name="retType">•Ô‚è’l‚ÌŒ^</param>
	/// <param name="name">ŠÖ”–¼</param>
	/// <param name="args">ˆø”‚Ì‘g‚İ‡‚í‚¹</param>
	/// <returns>¬Œ÷</returns>
	bool DefineSystemFunction(SysFunction arg_op, const std::int32_t retType, const std::string& name, const std::string& args, const std::vector<std::int32_t>& arg_vec_templateTypes);
	inline bool DefineSystemFunction(SysFunction arg_op, const std::int32_t retType, const std::string& name, const std::string& args) {
		static auto temps= std::vector<std::int32_t>();
		return DefineSystemFunction(arg_op, retType, name, args, temps);
	}
	bool DefineSystemMethod(SysFunction arg_p_method, const std::int32_t type, const std::int32_t retType, const std::string& name, const std::string& arg_args,const std::vector<std::int32_t>& arg_vec_templateTypes);
	inline bool DefineSystemMethod(SysFunction arg_p_method, const std::int32_t type, const std::int32_t retType, const std::string& name, const std::string& arg_args) {
		static auto temps = std::vector<std::int32_t>();
		return DefineSystemMethod(arg_p_method, type,retType, name,arg_args, temps);
	}

private:

	FunctionTable functions;
	std::vector<SysFunction> vec_sysCalls;
	std::vector<SysFunction> vec_sysMethodCalls;
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
	/// ‘g‚İ‚İŒ^‚Ì“o˜^
	/// </summary>
	/// <typeparam name="T">Œ^î•ñ</typeparam>
	/// <param name="arg_name">Œ^–¼</param>
	/// <param name="arg_argmentName">ˆø”‚Ég‚¤—ª–¼</param>
	/// <param name="memberInfo">ƒƒ“ƒoî•ñ</param>
	template <typename T>
	void RegistSystemType_(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		RegistSystemType_<T, T>(arg_name, arg_argmentName, memberInfo);
	}
	/// <summary>
	/// ‘g‚İ‚İŒ^‚Ì“o˜^
	/// </summary>
	/// <typeparam name="T">Œ^î•ñ</typeparam>
	/// <param name="arg_name">Œ^–¼</param>
	/// <param name="arg_argmentName">ˆø”‚Ég‚¤—ª–¼</param>
	/// <param name="memberInfo">ƒƒ“ƒoî•ñ</param>
	template <typename T,typename GenerateType>
	void RegistSystemType_(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		TypeTag type;


		std::int32_t index = Value::SetTypeIndex(TypeSpecific<T>());
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

			for (std::int32_t i = 0; i < identiferSplited.size(); i++) {
				auto typeSplited = StringHelper::Split(identiferSplited[i], ":");
				if (typeSplited.size() != 2) {
					// "‘g‚İ‚İŒ^‚Ìƒƒ“ƒo•Ï”‚Ìw’è‚ªŠÔˆá‚Á‚Ä‚¢‚Ü‚·"
					assert(0);
				}
				auto memberTypeIndex = types.GetArgmentKeyMap().at(typeSplited[1]);
				MemberValueInfo info = { i,memberTypeIndex ,AccessModifier::Public };
				type.map_memberValue.emplace(typeSplited[0], info);

			}
		}
		types.RegistType(type);
		//ƒXƒNƒŠƒvƒg’è‹`‚ÌŒ^‚ªƒƒ“ƒo‚Æ‚µ‚Ä—˜—p‚·‚éŒ^‚Ì“o˜^
		PushCreateMemberInstance<GenerateType>();

	}

	template <typename T>
	void RegistSharedSystemType(const std::string& arg_name, const std::string& arg_argmentName, const std::string& memberInfo = "") {
		TypeTag type;
		std::int32_t index = Value::SetTypeIndex(TypeSpecific<T>());
		type.isSystem = true;
		type.isShared = true;
		map_valueAllocCallsIndex.emplace(index, vec_valueAllocCall.size());
		vec_valueAllocCall.push_back(&VirtualMachine::pushValue<std::shared_ptr< T>>);

		map_refValueAllocCallsIndex.emplace(index, vec_refValueAllocCall.size());
		vec_refValueAllocCall.push_back(&VirtualMachine::pushValue_ref<std::shared_ptr< T>>);

		type.typeName = arg_name;
		type.typeIndex = index;
		type.argName = arg_argmentName;

		if (memberInfo.size()) {
			auto identiferSplited = StringHelper::Split(memberInfo, ",");

			for (std::int32_t i = 0; i < identiferSplited.size(); i++) {
				auto typeSplited = StringHelper::Split(identiferSplited[i], ":");
				if (typeSplited.size() != 2) {
					// "‘g‚İ‚İŒ^‚Ìƒƒ“ƒo•Ï”‚Ìw’è‚ªŠÔˆá‚Á‚Ä‚¢‚Ü‚·"
					assert(0);
				}
				auto memberTypeIndex = types.GetArgmentKeyMap().at(typeSplited[1]);
				MemberValueInfo info = { i,memberTypeIndex ,AccessModifier::Public };
				type.map_memberValue.emplace(typeSplited[0], info);

			}
		}
		types.RegistType(type);
		//ƒXƒNƒŠƒvƒg’è‹`‚ÌŒ^‚ªƒƒ“ƒo‚Æ‚µ‚Ä—˜—p‚·‚éŒ^‚Ì“o˜^
		PushCreateMemberInstance<std::shared_ptr<T>>();
	}
	std::int32_t GetIndex(const std::string& arg_typeName);
private:
	SystemTypeRegister() {}

	EnumTable enums;
	TypeTable types;
	std::vector<SysFunction> vec_valueAllocCall;
	std::vector<SysFunction> vec_refValueAllocCall;
	std::map<std::int64_t, std::int32_t> map_valueAllocCallsIndex;
	std::map<std::int64_t, std::int32_t> map_refValueAllocCallsIndex;
};

}
#endif // !BUTISCRIPTBUILTINTYPEREGISTER
