

#include "VirtualMachine.h"
#include "Node.h"
#include<unordered_map>
#include"../../Header/Device/Helper/StringHelper.h"
namespace ButiScript {



// ���z�}�V���R�[�h����

class VMCode {
public:
	VMCode(const unsigned char Op)
		: size_(1), op_(Op)
	{
	}
	VMCode(const unsigned char Op, const int arg)
		: size_(5), op_(Op)
	{
		constType = TYPE_INTEGER;
		p_constValue = new int(arg);
	}
	VMCode(const unsigned char Op, const float arg)
		: size_(5), op_(Op)
	{
		constType = TYPE_FLOAT;
		p_constValue = new float(arg);
	}

	~VMCode() {
	}

	void Release() {
		if (p_constValue) {
			delete p_constValue;
		}
	}
	static VMCode GetCode(const char Op, const int arg1, const int arg2) {

	}

	unsigned char* Get(unsigned char* p) const
	{
		if (op_ != VM_MAXCOMMAND) {			// ���x���̃_�~�[�R�}���h
			*p++ = op_;
			if (size_ > 1) {
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
	unsigned char size_;
	unsigned char op_;
	int constType;
	
	
	//�萔�̃|�C���^
	void* p_constValue =nullptr;

};

//���O��Ԃ̒�`
class NameSpace:std::enable_shared_from_this<NameSpace> {
public:
	NameSpace(const std::string& arg_name) :name(arg_name) {	}

	const std::string& GetNameString()const;
	std::string GetGlobalNameString()const;
	void Regist(Compiler* arg_compiler);
	void SetParent(std::shared_ptr<NameSpace>arg_parent);
	std::shared_ptr<NameSpace> GetParent()const;
private:
	std::string name;
	std::shared_ptr<NameSpace> shp_parentNamespace;
};

using NameSpace_t = std::shared_ptr<NameSpace>;


// ���x��

class Label {
public:
	Label(const int index)
		: index_(index), pos_(0)
	{
	}
	~Label()
	{
	}

public:
	int index_;
	int pos_;
};


// �R���p�C��

class VirtualCPU;
using SysFunction = void (VirtualCPU::*)();
class Compiler {
public:


	Compiler();
	virtual ~Compiler();
	/// <summary>
	/// �f�t�H���g�̑g�ݍ��݌^�A�g�ݍ��݊֐��̐ݒ�
	/// </summary>
	void RegistDefaultSystems();
	/// <summary>
	/// �R���p�C��
	/// </summary>
	/// <param name="arg_filePath">�t�@�C���p�X</param>
	/// <param name="arg_ref_data">�R���p�C���ς݃f�[�^�̏o�͐�</param>
	/// <returns>����/���s</returns>
	bool Compile(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data);
	/// <summary>
	/// �R���p�C���ς݃f�[�^�̃t�@�C���o��
	/// </summary>
	/// <param name="arg_filePath">�t�@�C���p�X</param>
	/// <param name="arg_ref_data">�R���p�C���ς݃f�[�^</param>
	/// <returns>����/���s</returns>
	int OutputCompiledData(const std::string& arg_filePath, const ButiScript::CompiledData& arg_ref_data);
	/// <summary>
	/// �R���p�C���ς݃f�[�^�̃t�@�C������
	/// </summary>
	/// <param name="arg_filePath">�t�@�C���p�X</param>
	/// <param name="arg_ref_data">�R���p�C���ς݃f�[�^�̏o�͐�</param>
	/// <returns>����/���s</returns>
	int InputCompiledData(const std::string& arg_filePath,ButiScript::CompiledData& arg_ref_data);

#ifdef	_DEBUG
	void debug_dump();
#endif

	/// <summary>
	/// �g�ݍ��݊֐��̓o�^
	/// </summary>
	/// <param name="arg_op">�֐��̃|�C���^</param>
	/// <param name="retType">�Ԃ�l�̌^</param>
	/// <param name="name">�֐���</param>
	/// <param name="args">�����̑g�ݍ��킹</param>
	/// <returns>����</returns>
	bool DefineSystemFunction(SysFunction arg_op,const int retType, const std::string& name, const std::string& args);

	bool DefineSystemMethod(SysFunction arg_p_method, const int type, const int retType, const std::string& name, const std::string& arg_args);

	/// <summary>
	/// �g�ݍ��݌^�̓o�^
	/// </summary>
	/// <typeparam name="T">�^���</typeparam>
	/// <param name="arg_typeIndex">�^�̃C���f�b�N�X</param>
	/// <param name="arg_name">�^��</param>
	/// <param name="arg_argmentName">�����Ɏg������</param>
	/// <param name="memberInfo">�����o���</param>
	template <typename T,int arg_typeIndex>
	void RegistSystemType( const std::string& arg_name,  const std::string& arg_argmentName,const std::string& memberInfo="") {
		TypeTag type;
		type.typeFunc = &VirtualCPU::pushValue<T, arg_typeIndex>;
		type.refTypeFunc = &VirtualCPU::pushValue<Type_Null,arg_typeIndex|TYPE_REF>;
		type.isSystem = true;
		long long int address = *(long long int*) & type.typeFunc;
		map_valueAllocCallsIndex.emplace(address, vec_valueAllocCall.size());
		vec_valueAllocCall.push_back(type.typeFunc);

		address = *(long long int*) & type.refTypeFunc;
		map_refValueAllocCallsIndex.emplace(address, vec_refValueAllocCall.size());
		vec_refValueAllocCall.push_back(type.refTypeFunc);

		type.typeName = arg_name;
		type.typeIndex = arg_typeIndex;
		type.argName = arg_argmentName;

		if (memberInfo.size()) {
			auto identiferSplited = StringHelper::Split(memberInfo, ",");

			for (int i = 0; i < identiferSplited.size(); i++) {
				auto typeSplited=StringHelper::Split(identiferSplited[i], ":");
				if (typeSplited.size() != 2) {
					error("�g�ݍ��݌^�̃����o�ϐ��̎w�肪�Ԉ���Ă��܂�");
				}
				auto memberTypeIndex = types.GetArgmentKeyMap().at(typeSplited[1]);
				type.map_memberIndex.emplace(typeSplited[0], i);
				type.map_memberType.emplace(typeSplited[0], memberTypeIndex);

			}
		}
		types.RegistType(type);
		//�X�N���v�g��`�̌^�������o�Ƃ��ė��p����^�̓o�^
		PushCreateMemberInstance<T>();
	}
	void RegistScriptType(const std::string& arg_typeName,  const std::map<std::string, int>& arg_memberInfo);
	const std::vector<TypeTag* >& GetSystemTypes()const {
		return types.GetSystemType();
	}

	void ValueDefine(const int type, const std::vector<Node_t>& node);
	void FunctionDefine(const int type, const std::string& name, const std::vector<int>& args);
	void AddFunction(const int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block, FunctionTable* arg_funcTable = nullptr);
	void RegistFunction(const int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block,FunctionTable* arg_funcTable=nullptr);

	void RegistEnum(const std::string& arg_typeName, const std::string& identiferName, const int value);
	void RegistEnumType(const std::string& arg_typeName);
	void RegistSystemEnumType(const std::string& arg_typeName);

	const EnumTag* GetEnumTag(const std::string& arg_name) const {
		return enums.FindType(arg_name);
	}
	EnumTag* GetEnumTag(const std::string& arg_name)  {
		return enums.FindType(arg_name);
	}

	// �ϐ��̌����A�����̃u���b�N���猟������B
	const ValueTag* GetValueTag(const std::string& name) const
	{
		int size = (int)variables.size();
		for (int i = size - 1; i >= 0; i--) {
			const ValueTag* tag = variables[i].find(name);
			if (tag)
				return tag;
		}
		return nullptr;
	}

	// �֐��̌���
	const FunctionTag* GetFunctionTag(const std::string& name,const std::vector<int>& args,const bool isStrict) const
	{
		if(isStrict)
		return functions.Find_strict(name,args);
		else {
			return functions.Find(name, args);
		}
	}

	//�^�̌���
	int GetTypeIndex(const std::string& arg_typeName)const {
		auto tag = types.GetType(arg_typeName);
		if (tag) {
			return tag->typeIndex;
		}
		return -1;
	}
	//

	TypeTag* GetType(const int index) {
		return types.GetType(index);
	}
	const TypeTag* GetType(const int index)const {
		return types.GetType(index);
	}

	TypeTag* GetType(const std::string& index) {
		return types.GetType(index);
	}
	const TypeTag* GetType(const std::string& index)const {
		return types.GetType(index);
	}
	int GetSystemTypeSize()const {
		return types.GetSystemTypeSize();
	}
	NameSpace_t GetCurrentNameSpace()const {
		return currentNameSpace;
	}

	const TypeTag* GetCurrentThisType()const {
		return currentThisType;
	}
	TypeTag* GetCurrentThisType() {
		return currentThisType;
	}
	void SetCurrentThisType(TypeTag* arg_this) {
		currentThisType = arg_this;
	}

	// for code generator.
#define	VM_CREATE
#include "VM_create.h"
#undef	VM_CREATE

	void BlockIn();
	void BlockOut();
	void AllocStack();
	int LabelSetting();

	int SetBreakLabel(const int label)
	{
		int old_index = break_index;
		break_index = label;
		return old_index;
	}
	bool JmpBreakLabel();

	int MakeLabel();

	void AddValue(const int type, const std::string& name, Node_t node);

	void SetLabel(const int label);

	void PushString(const std::string& name);
	int GetFunctionType() const { return current_function_type; }
	bool CreateData(ButiScript::CompiledData& Data,const int code_size);

	void PushNameSpace(NameSpace_t arg_namespace);
	void PopNameSpace();
	// Error handling.
	void error(const std::string& m);

	void ClearStatement();
	std::string GetTypeName(const int type) const;


private:
	FunctionTable functions;
	TypeTable types;
	EnumTable enums;
	TypeTag* currentThisType = nullptr;
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
	int break_index;
	int error_count;

	std::string current_function_name;
	int current_function_type;

};
}
