#pragma once
#ifndef	__VM_H__
#define	__VM_H__

#include <vector>
#include<unordered_map>
#include "vm_value.h"
#include"Tags.h"
namespace ButiScript {


#include"value_type.h"

#define	VM_ENUMDEF
	enum {
#include "VM_enum.h"
		VM_MAXCOMMAND,
	};
#undef	VM_ENUMDEF
	

	class CompiledData {
	public:
		CompiledData() : commandTable(0), textBuffer(0)
		{
		}
		~CompiledData()
		{
			delete[] commandTable;
			delete[] textBuffer;
		}

	public:
		unsigned char* commandTable;	// �R�}���h�e�[�u��
		char* textBuffer;			// �e�L�X�g�f�[�^
		int commandSize;			// �R�}���h�T�C�Y
		int textSize;				// �e�L�X�g�T�C�Y
		int valueSize;			// �O���[�o���ϐ��T�C�Y

		std::vector<OperationFunction> vec_sysCalls;
		std::vector<OperationFunction> vec_sysCallMethods;
		std::vector<TypeTag> vec_types;
		std::unordered_map<std::string, int> map_entryPoints;
		std::map< int,const std::string*> map_functionJumpPointsTable;
		std::map<std::string, int>map_globalValueAddress;
		std::map<int, EnumTag> map_enumTag;
		FunctionTable functions;
		std::vector<ScriptClassInfo> vec_scriptClassInfo;
		int systemTypeCount;
		std::string sourceFilePath;
	};


	// 0���Z
	class DevideByZero : public std::exception {
	public:
		const char* what() const throw()
		{
			return "devide by zero";
		}
	};

	// ���z�}�V��
	class VirtualCPU {
	public:
		const static int STACK_SIZE = 2048;
		const static int global_flag = 0x4000000;
		const static int global_mask = 0x3ffffff;


	public:
		VirtualCPU(std::shared_ptr<CompiledData> arg_data)
			: data_(arg_data)
		{
		}
		~VirtualCPU()
		{
			free( p_op);
			free( p_syscall );
			
			free( p_sysMethodCall );
			free(p_pushValues );
			free(p_pushRefValues );
		}
		template<typename T>
		T Execute(const std::string& entryPoint = "main") {
			if (!data_->map_entryPoints.count(entryPoint)) {
				return T();
			}
			stack_base = valueStack.size();					// �X�^�b�N�Q�ƈʒu������
			push(0);										// main�ւ̈����J�E���g��push
			push(stack_base);								// stack_base�̏����l��push
			push(0);										// �v���O�����I���ʒu��push

			Execute_(entryPoint);

			auto ret = top().v_->Get<T>();
			valueStack.resize(globalValue_size);
			return ret;
		}
		template<>
		void Execute(const std::string& entryPoint ) {
			if (!data_->map_entryPoints.count(entryPoint)) {
				return;
			}
			stack_base = valueStack.size();					// �X�^�b�N�Q�ƈʒu������
			push(0);										// main�ւ̈����J�E���g��push
			push(stack_base);								// stack_base�̏����l��push
			push(0);										// �v���O�����I���ʒu��push

			Execute_(entryPoint);

			valueStack.resize(globalValue_size);
		}

		template<typename T, typename U>
		int Execute(const std::string& entryPoint, U argment) {

			stack_base = valueStack.size();						// �X�^�b�N�Q�ƈʒu������
			push(argment);									//����push
			push(1);										// main�ւ̈����J�E���g��push
			push(stack_base);										// stack_base�̏����l��push
			push(0);										// �v���O�����I���ʒu��push
			Execute_(entryPoint);

			auto ret = top().v_->Get<T>();
			valueStack.resize(globalValue_size);
			return ret;
		}
		void AllocGlobalValue();

		template<typename T>
		void SetGlobalVariable(const T value, const std::string arg_variableName) {
			if (!data_->map_globalValueAddress.count(arg_variableName)) {

#ifdef IMPL_BUTIENGINE
				ButiEngine::GUI::Console( arg_variableName + "�ɂ̓A�N�Z�X�ł��܂���",ButiEngine::Vector4(1.0f,0.8f,0.8f,1.0f) );
#endif

				return;
			}
			valueStack[globalValue_base + data_->map_globalValueAddress.at(arg_variableName)].v_->Set(value);
		}
		template<typename T>
		T& GetGlobalVariable(const std::string arg_variableName) {
			if (!data_->map_globalValueAddress.count(arg_variableName)) {

#ifdef IMPL_BUTIENGINE
				ButiEngine::GUI::Console(arg_variableName + "�ɂ̓A�N�Z�X�ł��܂���", ButiEngine::Vector4(1.0f, 0.8f, 0.8f, 1.0f));
#endif
				static T temp;
				return temp;
			}
			return valueStack[globalValue_base + data_->map_globalValueAddress.at(arg_variableName)].v_->GetRef<T>();
		}

#ifdef IMPL_BUTIENGINE
		void SetGameObject(std::shared_ptr<ButiEngine::GameObject> arg_gameObject) {
			shp_gameObject = arg_gameObject;
		}
		std::shared_ptr<ButiEngine::GameObject>GetGameObject()const {
			return shp_gameObject;
		}

		void RestoreGlobalValue(std::vector<std::shared_ptr< ButiScript::IGlobalValueSaveObject>>& arg_ref_vec_saveObject);
		void SaveGlobalValue(std::vector<std::shared_ptr< ButiScript::IGlobalValueSaveObject>>& arg_ref_vec_saveObject);
		void ShowGUI();
#endif

		void Initialize();
	private:
		void Execute_(const std::string& entryPoint );

		/////////////�萔Push��`////////////////
		// �萔Push
		inline void PushConstInt(const int arg_val)
		{
			push(arg_val);
		}
		void PushConstInt()
		{
			PushConstInt(Value_Int());
		}

		// �萔Push
		inline void PushConstFloat(const float arg_val)
		{
			push(arg_val);
		}
		void PushConstFloat()
		{
			PushConstFloat(Value_Float());
		}

		// �����萔Push
		inline void PushString(const int arg_val)
		{
			push(std::string(textBuffer + arg_val));
		}
		void PushString()
		{
			PushString(Value_Int());
		}



		/////////////�ϐ�Push��`////////////////
		// �O���[�o���ϐ��̃R�s�[��Push
		inline void PushGlobalValue(const int arg_val)
		{
			push(valueStack[globalValue_base + arg_val].Clone());
		}
		void PushGlobalValue()
		{
			PushGlobalValue(Value_Int());
		}

		// ���[�J���ϐ��̃R�s�[��Push
		inline void PushLocal(const int arg_val)
		{
			push(valueStack[arg_val + stack_base].Clone());
		}

		void PushLocal()
		{
			PushLocal(Value_Int());
		}

		// �z�񂩂�R�s�[��Push
		inline void PushGlobalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(valueStack[(int)(arg_val + index)].Clone());
		}

		void PushGlobalArray()
		{
			PushGlobalArray(Value_Int());
		}

		// ���[�J���̔z�񂩂�R�s�[��Push
		inline void PushLocalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(valueStack[arg_val + stack_base + index].Clone());
		}

		void PushLocalArray()
		{
			PushLocalArray(Value_Int());
		}

		//�O���[�o���ϐ��̃����o�ϐ��̃R�s�[��push
		inline void PushMember( const int arg_valueIndex) {
			auto v_ = top().v_;
			pop();
			push_clone(v_->GetMember(arg_valueIndex), v_->GetMemberType(arg_valueIndex));
			top().v_->release();
		}

		void PushMember() {
			PushMember( Value_Int());
		}

		/////////////�O���[�o���ϐ��̎Q��Push��`////////////////

		// �O���[�o���ϐ��̎Q�Ƃ�Push
		inline void PushGlobalValueRef(const int arg_val)
		{
			push(valueStack[globalValue_base + arg_val]);
		}
		void PushGlobalValueRef()
		{
			PushGlobalValueRef(Value_Int());
		}

		// ���[�J���ϐ��̎Q�Ƃ�Push
		inline void PushLocalRef(const int arg_val)
		{
			push(valueStack[arg_val + stack_base]);
		}

		void PushLocalRef()
		{
			PushLocalRef(Value_Int());
		}

		// �z�񂩂�Q�Ƃ�Push
		inline void PushGlobalArrayRef(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(valueStack[(int)(arg_val + index)]);
		}

		void PushGlobalArrayRef()
		{
			PushGlobalArrayRef(Value_Int());
		}

		// ���[�J���̔z�񂩂�Q�Ƃ�Push
		inline void PushLocalArrayRef(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(valueStack[arg_val + stack_base + index]);
		}

		void PushLocalArrayRef()
		{
			PushLocalArrayRef(Value_Int());
		}


		//�����o�ϐ��̎Q�Ƃ�push
		inline void PushMemberRef( const int arg_valueIndex) {
			auto v = top().v_;
			pop();
			push(v->GetMember(arg_valueIndex), v->GetMemberType(arg_valueIndex));
		}

		void PushMemberRef() {
			PushMemberRef( Value_Int());
		}



		// �A�h���X��Push
		inline void PushAddr(const int arg_val)
		{
			int base = arg_val;
			if ((arg_val & global_flag) == 0)	// local
				base += +stack_base;
			push(base);
		}

		void PushAddr() {
			PushAddr(Value_Int());
		}

		// �z��̃A�h���X��Push
		inline void PushArrayAddr(const int arg_val)
		{
			int base = arg_val;
			if ((arg_val & global_flag) == 0)	// local
				base += +stack_base;
			int index = top().v_->Get<int>(); pop();
			push(base + index);
		}
		void PushArrayAddr() {
			PushArrayAddr(Value_Int());
		}


		/////////////Pop��`////////////////
		// �ϐ���Pop
		inline void PopValue(const int arg_val)
		{
			valueStack[globalValue_base+ arg_val] = top(); pop();
		}
		void PopValue() {
			PopValue(Value_Int());
		}
		// ���[�J���ϐ���Pop
		inline void PopLocal(const int arg_val)
		{
			valueStack[arg_val + stack_base] = top(); pop();
		}
		void PopLocal() {
			PopLocal(Value_Int());
		}

		//�����o�ϐ���Pop
		inline void PopMember(const int arg_index)
		{
			auto v = top().v_;
			pop();
			v->GetMember(arg_index)->Set(*top().v_); pop();
		}
		void PopMember() {
			PopMember(Value_Int());
		}
		//�����o�ϐ���Pop(�Q��)
		inline void PopMemberRef(const int arg_index)
		{
			auto v = top().v_;
			pop();
			v->SetMember(top().v_, arg_index); pop();
		}
		void PopMemberRef() {
			PopMemberRef(Value_Int());
		}

		// �z��ϐ���Pop
		inline void PopArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			valueStack[(int)(arg_val + index)] = top(); pop();
		}
		void PopArray() {
			PopArray(Value_Int());
		}

		// ���[�J���̔z��ϐ���Pop
		inline void PopLocalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			valueStack[arg_val + stack_base + index] = top(); pop();
		}

		void PopLocalArray() {
			PopLocalArray(Value_Int());
		}

		// ���[�J���ϐ�(�Q��)��Pop
		inline void PopLocalRef(const int arg_val)
		{
			int addr = valueStack[arg_val + stack_base].v_->Get<int>();
			set_ref(addr, top()); pop();
		}
		void PopLocalRef() {
			PopLocalRef(Value_Int());
		}
		// ���[�J���̔z��ϐ�(�Q��)��Pop
		inline void PopLocalArrayRef(const int arg_val)
		{
			int addr = valueStack[arg_val + stack_base].v_->Get<int>();
			int index = top().v_->Get<int>(); pop();
			set_ref(addr + index, top()); pop();
		}
		void PopLocalArrayRef()
		{
			PopLocalArrayRef(Value_Int());
		}


		// ��Pop�i�X�^�b�N�g�b�v���̂Ă�j
		void OpPop()
		{
			pop();
		}

		/////////////Alloc��`////////////////
		// ���[�J���ϐ����m��
		inline void OpAllocStack(const int arg_val)
		{
			(this->*p_pushValues[arg_val])();
		}
		void OpAllocStack()
		{
			OpAllocStack(Value_Int());
		}

		void OpAllocStackEnumType() {
			int type = Value_Int();
			auto value = Value(Type_Enum(), &data_->map_enumTag.at(type));
			value.SetType(type);
			this->valueStack.push(value);
		}
		void OpAllocStackFunctionType() {
			int type = Value_Int();
			auto value = Value(Type_Func(), &data_->map_functionJumpPointsTable);
			value.SetType(type);
			this->valueStack.push(value);
		}

		// ���[�J���ϐ�(�Q�ƌ^)���m��
		inline void OpAllocStack_Ref(const int arg_val)
		{
			(this->*p_pushRefValues[arg_val & ~TYPE_REF])();

		}
		void OpAllocStack_Ref()
		{
			OpAllocStack_Ref(Value_Int());
		}
		// ���[�J���ϐ����m��(�X�N���v�g��`)
		inline void OpAllocStack_ScriptType(const int arg_val)
		{
			pushValue(&vec_scriptClassInfo[arg_val],&vec_scriptClassInfo);

		}
		void OpAllocStack_ScriptType()
		{
			OpAllocStack_ScriptType(Value_Int());
		}

		// ���[�J���ϐ�(�Q�ƌ^)���m��(�X�N���v�g��`)
		inline void OpAllocStack_Ref_ScriptType(const int arg_val)
		{
			pushValue_ref(&vec_scriptClassInfo[arg_val]);
		}
		void OpAllocStack_Ref_ScriptType()
		{
			OpAllocStack_Ref_ScriptType(Value_Int());
		}
		// ���[�J���ϐ�(�Q�ƌ^)���m��(�񋓌^)
		void OpAllocStack_Ref_EnumType()
		{
			OpAllocStack_Ref(Value_Int());
		}
		// ���[�J���ϐ�(�Q�ƌ^)���m��(�֐��^)
		void OpAllocStack_Ref_FunctionType()
		{
			OpAllocStack_Ref(Value_Int());
		}




		/////////////���Z�q��`////////////////

		// �P���}�C�i�X
		void OpNeg()
		{
			auto negV = top().v_->Clone();
			auto type = top().valueType;
			negV->Nagative();
			pop();
			push(negV,type);
		}

		// ==
		void OpEq()
		{

			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(lhs->Eq(rhs));

			rhs->release();
			lhs->release();

		}

		// !=
		void OpNe()
		{
			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(!lhs->Eq(rhs));

			rhs->release();
			lhs->release();
		}

		// >
		void OpGt()
		{
			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(lhs->Gt(rhs));

			rhs->release();
			lhs->release();
		}

		// >=
		void OpGe()
		{
			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(lhs->Ge(rhs));

			rhs->release();
			lhs->release();
		}

		// <
		void OpLt()
		{
			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(!lhs->Ge(rhs));

			rhs->release();
			lhs->release();
		}

		// <=
		void OpLe()
		{

			auto rhs = top().v_; rhs->addref(); pop();
			auto lhs = top().v_; lhs->addref(); pop();
			push(!lhs->Gt(rhs));

			rhs->release();
			lhs->release();

		}

		// &&
		void OpLogAnd()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs && rhs);
		}

		// ||
		void OpLogOr()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs || rhs);
		}

		// &
		void OpAnd()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs & rhs);
		}

		// |
		void OpOr()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs | rhs);
		}

		// <<
		void OpLeftShift()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs << rhs);
		}

		// >>
		void OpRightShift()
		{
			auto rhs = top().v_->Get<int>(); pop();
			auto lhs = top().v_->Get<int>(); pop();
			push(lhs >> rhs);
		}

		// +
		template<typename T>
		void OpAdd()
		{
			auto rhs = top().v_->Get<T>(); pop();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs + rhs);
		}

		// -
		template<typename T>
		void OpSub()
		{
			auto rhs = top().v_->Get<T>(); pop();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs - rhs);
		}

		// *
		template<typename T>
		void OpMul()
		{
			auto rhs = top().v_->Get<T>(); pop();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs * rhs);
		}

		// /
		template<typename T>
		void OpDiv()
		{
			auto rhs = top().v_->Get<T>(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs / rhs);
		}

		// �قȂ�^*
		template<typename T,typename U>
		void OpMul()
		{
			auto rhs = top().v_->Get<U>(); pop();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs * rhs);
		}

		// �قȂ�^/
		template<typename T, typename U>
		void OpDiv()
		{
			auto rhs = top().v_->Get<U>(); pop();
			if (rhs == 0)
				throw DevideByZero();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs / rhs);
		}

		// %
		template<typename T>
		void OpMod()
		{
			int rhs = top().v_->Get<int>(); pop();
			if (rhs == 0)
				throw DevideByZero();
			int lhs = top().v_->Get<int>(); pop();
			push(lhs % rhs);
		}




		// �������==
		void OpStrEq()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs == rhs);
		}

		// �������!=
		void OpStrNe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs != rhs);
		}

		// �������>
		void OpStrGt()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs > rhs);
		}

		// �������>=
		void OpStrGe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs >= rhs);
		}

		// �������<
		void OpStrLt()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs < rhs);
		}

		// �������<=
		void OpStrLe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs <= rhs);
		}

		// �������+
		void OpStrAdd()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs + rhs);
		}




		/////////////�A�h���X�����`////////////////
		// �������W�����v
		inline void OpJmp(const int arg_val)
		{
			jmp(arg_val);
		}
		void OpJmp() {
			OpJmp(Value_Int());
		}

		// �^�̎��W�����v
		inline void OpJmpC(const int arg_val)
		{
			int cond = top().v_->Get<int>(); pop();
			if (cond)
				jmp(arg_val);
		}
		void OpJmpC() {
			OpJmpC(Value_Int());
		}

		// �U�̎��W�����v
		inline void OpJmpNC(const int arg_val)
		{
			int cond = top().v_->Get<int>(); pop();
			if (!cond)
				jmp(arg_val);
		}
		void OpJmpNC() {
			OpJmpNC(Value_Int());
		}

		// switch���p���ꔻ��
		inline void OpTest(const int arg_val)
		{
			int Value = top().v_->Get<int>(); pop();
			if (Value == top().v_->Get<int>()) {
				pop();
				jmp(arg_val);
			}
		}
		void OpTest() {
			OpTest(Value_Int());
		}

		/////////////�֐��Ăяo����`////////////////
		// �֐��R�[��
		inline void OpCall(const int arg_val)
		{
			push(stack_base);
			push(addr());					// ���^�[���A�h���X��Push
			stack_base = valueStack.size();		// �X�^�b�N�x�[�X�X�V
			jmp(arg_val);
		}

		void OpCall() {
			OpCall(Value_Int());
		}

		void OpCallByVariable() {
			int addr = top().v_->Get<int>(); pop();
			OpCall(addr);
		}

		// �����Ȃ����^�[��
		void OpReturn()
		{
			valueStack.resize(stack_base);		// ���[�J���ϐ��r��
			int addr = top().v_->Get<int>(); pop();
			stack_base = top().v_->Get<int>(); pop();
			int arg_count = top().v_->Get<int>(); pop();
			valueStack.pop(arg_count);
			jmp(addr);
		}

		// �����t�����^�[��
		void OpReturnV()
		{
			ButiScript::Value result = top(); 
			pop();
			valueStack.resize(stack_base);		// ���[�J���ϐ��r��
			int addr = top().v_->Get<int>(); pop();
			stack_base = top().v_->Get<int>(); pop();
			int arg_count = top().v_->Get<int>(); pop();
			valueStack.pop(arg_count);
			push(result);
			jmp(addr);
		}

		//�֐��I�u�W�F�N�g�̃A�h���X��Push
		inline void OpPushFunctionAddress(const int address) {
			push(address);
		}
		//�֐��I�u�W�F�N�g�̃A�h���X��Push
		void OpPushFunctionAddress() {

			OpPushFunctionAddress(Value_Int());
		}

		// ���zCPU��~
		void OpHalt()
		{
		}
		// �g�ݍ��݊֐�
		inline void OpSysCall(const int val)
		{
			pop();	// arg_count
			(this->*p_syscall[val])();
		}

		void OpSysCall() {
			OpSysCall(Value_Int());
		}

		//�g�ݍ��݃��\�b�h
		inline void OpSysMethodCall(const int val)
		{
			pop();	// arg_count
			(this->*p_sysMethodCall[val])();
		}
		void OpSysMethodCall() {
			OpSysMethodCall(Value_Int());
		}

	public:


#ifdef IMPL_BUTIENGINE

		void sys_addEventMessanger();
		void sys_registEventListner();
		void sys_unregistEventListner();
		void sys_executeEvent();
		void sys_pushTask();
		void sys_LoadTextureAsync();
		void sys_LoadWaveAsync();
		void sys_LoadWavesAsync();
		void sys_LoadTexture();
		void sys_LoadWave();
		void sys_translate() {

			auto v = top().v_->Get<ButiEngine::Vector3>(); pop();
			shp_gameObject->transform->Translate(v);
		}
		void sys_setworldposition() {

			auto v = top().v_->Get<ButiEngine::Vector3>(); pop();
			shp_gameObject->transform->SetWorldPosition(v);
		}
		void sys_get_ownGameObject() {
			push(shp_gameObject);
		}
		void sys_get_gameObjectByName() {
			std::string name = top().v_->GetRef<std::string>(); pop();
			push(shp_gameObject->GetGameObjectManager().lock()->GetGameObject(name).lock());
		}
		void sys_getKeyboard() {

			int k = top().v_->Get<int>(); pop();
			int res = ButiEngine::GameDevice::GetInput()->CheckKey(k);
			push(res);
		}
		void sys_triggerKeyboard() {

			int k = top().v_->Get<int>(); pop();
			int res = ButiEngine::GameDevice::GetInput()->TriggerKey(k);
			push(res);
		}
		void sys_releaseKeyboard() {

			int k = top().v_->Get<int>(); pop();
			int res = ButiEngine::GameDevice::GetInput()->ReleaseKey(k);
			push(res);
		}
		void sys_printColor() {
			ButiEngine::Vector4 color=top().v_->Get<ButiEngine::Vector4>();	pop();
			auto message = text(top());	pop();
			ButiEngine::GUI::Console(message,color);
		}
#endif // IMPL_BUTIENGINE

		// �g�ݍ��݊֐��iprint�j
		void sys_print()
		{
#ifdef IMPL_BUTIENGINE

			ButiEngine::GUI::Console( text(top()));
#else

			std::cout << text(top());
#endif // IMPL_BUTIENGINE

			pop();
		}

		// �g�ݍ��݊֐��ipause�j
		void Sys_pause() {
			std::system("pause");			
		}

		// �g�ݍ��݊֐�(���l�𕶎���ɕϊ�)
		void sys_tostr()
		{
			auto v = top().v_->Get<std::string>(); pop();
			push(v);
			// �߂�l�̓X�^�b�N�ɓ����
		}

		//�g�ݍ��݊֐�(return ����)
		template< void(*Method)() >
		void sys_func_retNo()
		{
			(*Method)();
		}
		//�g�ݍ��݊֐�(return �L��)
		template<typename T, T(*Method)() >
		void sys_func_ret()
		{
			T ret = (*Method)();
			push(ret);
		}

		//�g�ݍ��݊֐�(return ����)
		template<typename Arg, void(*Method)(Arg) >
		void sys_func_retNo()
		{
			auto arg = top().v_->Get<Arg>(); pop(); 
			(*Method)(arg);
		}
		//�g�ݍ��݊֐�(return �L��)
		template<typename T, typename Arg, T(*Method)(Arg) >
		void sys_func_ret()
		{
			auto arg = top().v_->Get<Arg>(); pop();
			T ret = (*Method)(arg);
			push(ret);
		}
		//�g�ݍ��݊֐�(return �L��)
		template<typename T, typename Arg1, typename Arg2, T(*Method)(Arg1, Arg2) >
		void sys_func_ret()
		{
			auto arg2 = top().v_->Get<Arg2>(); pop();
			auto arg1 = top().v_->Get<Arg1>(); pop();
			T ret = (*Method)(arg1, arg2);
			push(ret);
		}
		//�g�ݍ��݊֐�(return �L��)
		template<typename T, typename Arg1, typename Arg2, typename Arg3, T(*Method)(Arg1, Arg2, Arg3) >
		void sys_func_ret()
		{
			auto arg3 = top().v_->Get<Arg3>(); pop();
			auto arg2 = top().v_->Get<Arg2>(); pop();
			auto arg1 = top().v_->Get<Arg1>(); pop();
			T ret = (*Method)(arg1, arg2,arg3);
			push(ret);
		}
		

		/////////////���\�b�h�Ăяo����`////////////////

		template<typename T>
		T* GetSharedTypePtr() {
			return &(*((Value_Shared<T>*)top().v_)->Get());
		}
		template<typename T>
		T* GetTypePtr() {
			return &(top().v_->GetRef<T>());
		}

		//�g�ݍ��݃��\�b�h(return ����)
		template<typename T, void(T::* Method)() ,T*(VirtualCPU::*getValueFunc)() >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			((v)->*Method)();
			pop();
		}
		template<typename T, void(T::* Method)() const, T* (VirtualCPU::* getValueFunc)() >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			((v)->*Method)();
			pop();
		}
		//�g�ݍ��݃��\�b�h(return �L��)
		template<typename T, typename U, U(T::* Method)(), T* (VirtualCPU::* getValueFunc)() >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			U ret = ((v)->*Method)();
			pop();
			push(ret);
		}
		template<typename T, typename U, U(T::* Method)() const, T* (VirtualCPU::* getValueFunc)() >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			U ret = ((v)->*Method)();
			pop();
			push(ret);
		}

		//�g�ݍ��݃��\�b�h(return �����A�����L��)
		template<typename T, typename Arg, void(T::* Method)(Arg) const, T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()  >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			((v)->*Method)(arg);
			pop();

		}
		template<typename T, typename Arg, void(T::* Method)(Arg&) const, T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()   >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			((v)->*Method)(arg);
			pop();
		}
		template<typename T, typename Arg, void(T::* Method)(const Arg&) const, T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()  >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			((v)->*Method)(arg);
			pop();
		}
		template<typename T, typename Arg, void(T::* Method)(Arg*)const, T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()   >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			((v)->*Method)(&arg);
			pop();
		}
		template<typename T, typename Arg, void(T::* Method)(const Arg*) const, T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()   >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			((v)->*Method)(&arg);
			pop();
		}
		template<typename T, typename Arg, void(T::* Method)(Arg), T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()  >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			((v)->*Method)(*arg);
			pop();

		}
		template<typename T, typename Arg, void(T::* Method)(Arg&), T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()   >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			((v)->*Method)(*arg);
			pop();
		}
		template<typename T, typename Arg, void(T::* Method)(const Arg&), T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()  >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			((v)->*Method)(*arg);
			pop();
		}
		template<typename T, typename Arg, void(T::* Method)(Arg*), T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()   >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			((v)->*Method)(&arg);
			pop();
		}
		template<typename T, typename Arg, void(T::* Method)(const Arg*), T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()   >
		void sys_method_retNo()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			((v)->*Method)(&arg);
			pop();
		}

		//�g�ݍ��݃��\�b�h(return �L��A�����L��)
		template<typename T, typename U, typename Arg, U(T::* Method)(Arg) const, T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()  >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			U ret = ((v)->*Method)(*arg);
			pop();
			push(ret);
		}

		template<typename T, typename U, typename Arg, U(T::* Method)(Arg&) const, T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()   >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			U ret = ((v)->*Method)(*arg);
			pop();
			push(ret);
		}

		template<typename T, typename U, typename Arg, U(T::* Method)(const Arg&) const, T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()   >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			U ret = ((v)->*Method)(*arg);
			pop();
			push(ret);
		}

		template<typename T, typename U, typename Arg, U(T::* Method)(Arg*) const, T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()   >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			U ret = ((v)->*Method)(&arg);
			pop();
			push(ret);
		}

		template<typename T, typename U, typename Arg, U(T::* Method)(const Arg*) const, T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()  >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			U ret = ((v)->*Method)(&arg);
			pop();
			push(ret);
		}
		template<typename T, typename U, typename Arg, U(T::* Method)(Arg), T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()  >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			U ret = ((v)->*Method)(*arg);
			pop();
			push(ret);
		}

		template<typename T, typename U, typename Arg, U(T::* Method)(Arg&), T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()  >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			U ret = ((v)->*Method)(*arg);
			pop();
			push(ret);
		}

		template<typename T, typename U, typename Arg, U(T::* Method)(const Arg&), T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()    >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			U ret = ((v)->*Method)(*arg);
			pop();
			push(ret);
		}

		template<typename T, typename U, typename Arg, U(T::* Method)(Arg*), T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()    >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			U ret = ((v)->*Method)(&arg);
			pop();
			push(ret);
		}

		template<typename T, typename U, typename Arg, U(T::* Method)(const Arg*), T* (VirtualCPU::* getValueFunc)(), Arg* (VirtualCPU::* getArgValueFunc)()  >
		void sys_method_ret()
		{
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = ((this)->*getArgValueFunc)();
			U ret = ((v)->*Method)(&arg);
			pop();
			push(ret);
		}

	

		template<typename T>
		void pushValue() {
			auto value = Value(T());
			long long int address = TypeSpecific<T>();
			value.SetType(Value::GetTypeIndex(address));
			this->valueStack.push(value);
		}
		template<>
		void pushValue<Type_Null>() {
			auto value = Value(Type_Null());
			long long int address = TypeSpecific<Type_Null>();
			value.SetType(Value::GetTypeIndex(address));
			this->valueStack.push(value);
		}
		template<typename T>
		void pushValue_ref() {
			auto value = Value();
			long long int address = TypeSpecific<T>();
			value.SetType(Value::GetTypeIndex(address)|TYPE_REF);
			this->valueStack.push(value);
		}
		template<typename T>
		void pushSharedValue() {
			auto value = Value();
			long long int address = TypeSpecific<T>();
			value.SetType(Value::GetTypeIndex(address));
			this->valueStack.push(value);
		}
		template<typename T>
		void pushSharedValue_ref() {
			auto value = Value();
			long long int address = TypeSpecific<T>();
			value.SetType(Value::GetTypeIndex(address)|TYPE_REF);
			this->valueStack.push(value);
		}
		void pushValue(ScriptClassInfo* info, std::vector<ButiScript::ScriptClassInfo>* p_vec_scriptClassInfo) {
			auto value = Value(*info,p_vec_scriptClassInfo);
			this->valueStack.push(value);
		}
		inline void pushValue_ref(ScriptClassInfo* info) {
			auto value = Value(Type_Null());
			value.SetType(info->GetTypeIndex() | TYPE_REF);
			this->valueStack.push(value);
		}

	private:
		int Value_Int() { int v = *(int*)command_ptr_; command_ptr_ += 4; return v; }
		float Value_Float() { float v = *(float*)command_ptr_; command_ptr_ += 4; return v; }
		int addr() const { return (int)(command_ptr_ - commandTable); }
		void jmp(int addr) { command_ptr_ = commandTable + addr; }
		void push(int v) { 
			valueStack.push(ButiScript::Value(v));
		}
		void push(float v) { 
			valueStack.push(ButiScript::Value(v)); 
		}
		void push(const std::string& v) {
			valueStack.push(ButiScript::Value(v));
		}
		template <typename T>
		void push(std::shared_ptr<T> v) {
			valueStack.push(ButiScript::Value(v));

			auto ptr = &VirtualCPU::pushSharedValue<T>;
			auto address = *(long long int*) & (ptr);
			top().SetTypeIndex(address);
		}
		void push(IValue* arg_p_ivalue,const int arg_type) {
			valueStack.push(ButiScript::Value(arg_p_ivalue, arg_type));
		}
		void push_clone(IValue* arg_p_ivalue,const int arg_type) {
			valueStack.push(ButiScript::Value(arg_p_ivalue->Clone(), arg_type));
		}
		void push(const ButiScript::Value& v) { 
			valueStack.push(v); 
		}

		void pop() { 
			valueStack.pop(); 
		}
		const ButiScript::Value& top() const { 
			return valueStack.top();
		}
		ButiScript::Value& top() { 
			return valueStack.top(); 
		}
		std::string text(const ButiScript::Value& v) { return v.v_->Get<std::string>(); }
		const ButiScript::Value& ref_to_value(const int addr) const
		{
			if (addr & global_flag) {
				return valueStack[addr & global_mask];
			}
			return valueStack[addr];
		}
		void set_ref(const int addr, const ButiScript::Value& v)
		{
			if (addr & global_flag)
				PopLocal(addr-1);
			else
				valueStack[addr] = v;
		}

	private:
		std::shared_ptr<CompiledData> data_;

		//�R�}���h����
		unsigned char* commandTable;
		//���ݎQ�Ƃ��Ă�R�}���h�̈ʒu
		unsigned char* command_ptr_=nullptr;
		//�O���[�o���ϐ��̊m�ۃR�}���h
		unsigned char* allocCommand_ptr_=nullptr;
		//�v���O�����S�̂̃T�C�Y
		int commandSize;
		//������f�[�^
		char* textBuffer;
		//������f�[�^�̃T�C�Y
		int textSize;

		//���߃e�[�u��
		OperationFunction* p_op=nullptr;
		//�g�ݍ��݊֐��e�[�u��
		OperationFunction* p_syscall = nullptr;
		//�g�ݍ��݃��\�b�h�e�[�u��
		OperationFunction* p_sysMethodCall = nullptr;
		//�ϐ��̊m�ۊ֐��e�[�u��
		OperationFunction* p_pushValues = nullptr;
		//�ϐ�(�Q�ƌ^)�̊m�ۊ֐��e�[�u��
		OperationFunction* p_pushRefValues = nullptr;

		std::vector<ScriptClassInfo> vec_scriptClassInfo;

		ButiScript::Stack<ButiScript::Value, STACK_SIZE> valueStack;

		//�X�^�b�N�̎Q�ƈʒu
		int stack_base=0;
		int globalValue_base = 0;
		int globalValue_size=0;

#ifdef IMPL_BUTIENGINE
		std::shared_ptr<ButiEngine::GameObject> shp_gameObject;
#endif
	};

}


#endif
