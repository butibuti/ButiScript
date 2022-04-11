#pragma once
#ifndef	__VM_H__
#define	__VM_H__
#ifdef _BUTIENGINEBUILD
#include"Header/GameObjects/DefaultGameComponent/ButiScriptBehavior.h"
#endif // _BUTIENGINEBUILD

#include <vector>
#include<unordered_map>
#include "vm_value.h"
#include"Tags.h"
namespace ButiScript {


#include"value_type.h"

#define	VM_ENUMDEF
	enum VM_ENUM{
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

		std::uint8_t* commandTable;	// �R�}���h�e�[�u��
		char* textBuffer;			// �e�L�X�g�f�[�^
		std::int32_t commandSize;			// �R�}���h�T�C�Y
		std::int32_t textSize;				// �e�L�X�g�T�C�Y
		std::int32_t valueSize;			// �O���[�o���ϐ��T�C�Y

		ButiEngine::List<OperationFunction> list_sysCalls;
		ButiEngine::List<OperationFunction> list_sysCallMethods;
		ButiEngine::List<TypeTag> list_types;
		std::unordered_map<std::string, std::int32_t> map_entryPoints;
		std::map< std::int32_t,const std::string*> map_functionJumpPointsTable;
		std::map<std::string, std::int32_t>map_globalValueAddress;
		std::map<std::int32_t ,std::string>map_addressToValueName;
		std::map<std::int32_t, EnumTag> map_enumTag;
		FunctionTable functions;
		ButiEngine::List<ScriptClassInfo> list_scriptClassInfo;
		std::int32_t systemTypeCount,functionTypeCount;
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


	class VirtualMachine {
	private:
		template<std::int32_t CurrentArgmentIndex, auto Function>	friend	class SystemFunctionImple;

		template<std::int32_t CurrentArgmentIndex, auto Function>
		class SystemFunctionImple {
		public:
			template< typename... Args>
			SystemFunctionImple(VirtualMachine& arg_owner, Args... args) :owner(arg_owner) {
				auto arg = owner.top().Get<ButiTypeDetail::function_argment_type<Function, CurrentArgmentIndex - 1>>(); owner.pop();
				SystemFunctionImple<CurrentArgmentIndex - 1, Function>(owner, arg, args...);
			}
		private:
			VirtualMachine& owner;
		};
		template<auto Function>
		class SystemFunctionImple<0, Function> {
		public:

			template< typename... Args>
			SystemFunctionImple(VirtualMachine& arg_owner, Args... args) :owner(arg_owner) {
				if constexpr (std::is_void_v<ButiTypeDetail::function_return_type<Function>>) {
					(*Function)( args...);
				}
				else {
					auto ret = (*Function)( args...);
					owner.push(ret);
				}
			}
		private:
			VirtualMachine& owner;
		};

	public:
		const static std::int32_t STACK_SIZE = 2048;
		const static std::int32_t global_flag = 0x4000000;
		const static std::int32_t global_mask = 0x3ffffff;

		VirtualMachine(ButiEngine::Value_ptr<CompiledData> arg_data)
			: vlp_data(arg_data)
		{
		}
		~VirtualMachine()
		{
			free( p_op);
			free( p_syscall );
			
			free( p_sysMethodCall );
			free(p_pushValues );
			free(p_pushRefValues );
		}
		template <typename T>
		void PushArgments(T argment) {
			push(argment);
		}
		void PushArgments() {}

		template<typename T, typename... U>
		void PushArgments( T argment,U...argments ) {
			PushArgments(argment);
			PushArgments(argments...);
		}

		template<typename T>
		T Execute(const std::string& arg_entryPoint) {

			stack_base = valueStack.size();
			std::int32_t pushCount = 0;

			push(pushCount);				
			push(stack_base);				
			push(0);						
			Execute_(arg_entryPoint);

			if constexpr (std::is_void_v<T>) {
				valueStack.resize(globalValue_size);
			}
			else {
				auto ret = top().Get<T>();
				valueStack.resize(globalValue_size);
				return ret;
			}
		}
		template<typename T, typename... U>
		T Execute(const std::string& arg_entryPoint, U... argmentValue) {

			stack_base = valueStack.size();						// �X�^�b�N�Q�ƈʒu������
			std::int32_t pushCount = sizeof...(argmentValue);
			PushArgments(argmentValue...);
			push(pushCount);										// main�ւ̈����J�E���g��push
			push(stack_base);										// stack_base�̏����l��push
			push(0);										// �v���O�����I���ʒu��push
			Execute_(arg_entryPoint);
			
			if constexpr (std::is_void_v<T>) {
				valueStack.resize(globalValue_size);
			}
			else {
				auto ret = top().Get<T>();
				valueStack.resize(globalValue_size);
				return ret;
			}

		}
		void AllocGlobalValue();
		void Clear();
		bool ExistFunction(const std::string& arg_functionName)const {
			return vlp_data->functions.Find(arg_functionName);
		}
		VirtualMachine* Clone();

		template<typename T>
		void SetGlobalVariable(const T value, const std::string arg_variableName) {
			if (!vlp_data->map_globalValueAddress.count(arg_variableName)) {

#ifdef _BUTIENGINEBUILD
				ButiEngine::GUI::Console( arg_variableName + "�ɂ̓A�N�Z�X�ł��܂���",ButiEngine::Vector4(1.0f,0.8f,0.8f,1.0f) );
#endif
				return;
			}
			valueStack[globalValue_base + vlp_data->map_globalValueAddress.at(arg_variableName)].valueData=ButiEngine::make_value<T>(value);
		}
		template<typename T>
		T& GetGlobalVariable(const std::string arg_variableName) {
			if (!vlp_data->map_globalValueAddress.count(arg_variableName)) {

#ifdef _BUTIENGINEBUILD
				ButiEngine::GUI::Console(arg_variableName + "�ɂ̓A�N�Z�X�ł��܂���", ButiEngine::Vector4(1.0f, 0.8f, 0.8f, 1.0f));
#endif
				static T temp;
				return temp;
			}
			return valueStack[globalValue_base + vlp_data->map_globalValueAddress.at(arg_variableName)].Get<T>();
		}

#ifdef _BUTIENGINEBUILD
		void SetGameObject(ButiEngine::Value_ptr<ButiEngine::GameObject> arg_gameObject) {
			vlp_gameObject = arg_gameObject;
		}
		ButiEngine::Value_ptr<ButiEngine::GameObject>GetGameObject()const {
			return vlp_gameObject;
		}
		void SetButiScriptBehavior(ButiEngine::Value_ptr<ButiEngine::ButiScriptBehavior> arg_behavior) {
			wkp_butiScriptBehavior = arg_behavior;
		}
		ButiEngine::Value_ptr<ButiEngine::ButiScriptBehavior> GetButiScriptBehavior() {
			return wkp_butiScriptBehavior.lock();
		}
		//List��cereal�Ή�������������ύX
		void RestoreGlobalValue(std::vector<std::pair< ButiEngine::Value_ptr <ButiEngine::IValuePtrRestoreObject>, std::int32_t>>& arg_ref_list_saveObject);
		void SaveGlobalValue(std::vector<std::pair< ButiEngine::Value_ptr <ButiEngine::IValuePtrRestoreObject>,std::int32_t>>& arg_ref_list_saveObject);
		void ShowGUI();
		ButiEngine::Value_ptr<CompiledData> GetCompiledData()const { return vlp_data; }
#endif

		void Initialize();
		bool HotReload(ButiEngine::Value_ptr<CompiledData> arg_data);
	private:
		void Execute_(const std::string& arg_entryPoint );

		/////////////�萔Push��`////////////////
		// �萔Push
		inline void PushConstInt(const std::int32_t arg_val)
		{
			push(arg_val);
		}
		void PushConstInt()
		{
			PushConstInt(Constant<std::int32_t>());
		}

		// �萔Push
		inline void PushConstFloat(const float arg_val)
		{
			push(arg_val);
		}
		void PushConstFloat()
		{
			PushConstFloat(Constant<float>());
		}

		// �����萔Push
		inline void PushString(const std::int32_t arg_val)
		{
			push(std::string(textBuffer + arg_val));
		}
		void PushString()
		{
			PushString(Constant<std::int32_t>());
		}



		/////////////�ϐ�Push��`////////////////
		// �O���[�o���ϐ��̃R�s�[��Push
		inline void PushGlobalValue(const std::int32_t arg_val)
		{
			push(valueStack[globalValue_base + arg_val].Clone());
		}
		void PushGlobalValue()
		{
			PushGlobalValue(Constant<std::int32_t>());
		}

		// ���[�J���ϐ��̃R�s�[��Push
		inline void PushLocal(const std::int32_t arg_val)
		{
			push(valueStack[arg_val + stack_base].Clone());
		}

		void PushLocal()
		{
			PushLocal(Constant<std::int32_t>());
		}

		// �z�񂩂�R�s�[��Push
		inline void PushGlobalArray(const std::int32_t arg_val)
		{
			std::int32_t index = top().Get<std::int32_t>(); pop();
			push(valueStack[(std::int32_t)(arg_val + index)].Clone());
		}

		void PushGlobalArray()
		{
			PushGlobalArray(Constant<std::int32_t>());
		}

		// ���[�J���̔z�񂩂�R�s�[��Push
		inline void PushLocalArray(const std::int32_t arg_val)
		{
			std::int32_t index = top().Get<std::int32_t>(); pop();
			push(valueStack[arg_val + stack_base + index].Clone());
		}

		void PushLocalArray()
		{
			PushLocalArray(Constant<std::int32_t>());
		}

		//�����o�ϐ��̃R�s�[��push
		inline void PushMember( const std::int32_t arg_valueIndex) {
			auto valueData = top().valueData;
			pop();
			push_clone(valueData.get<IType_hasMember>()->GetMember(arg_valueIndex), valueData.get<IType_hasMember>()->GetMemberType(arg_valueIndex));
		}

		void PushMember() {
			PushMember( Constant<std::int32_t>());
		}

		/////////////�O���[�o���ϐ��̎Q��Push��`////////////////

		// �O���[�o���ϐ��̎Q�Ƃ�Push
		inline void PushGlobalValueRef(const std::int32_t arg_val)
		{
			push(valueStack[globalValue_base + arg_val]);
		}
		void PushGlobalValueRef()
		{
			PushGlobalValueRef(Constant<std::int32_t>());
		}

		// ���[�J���ϐ��̎Q�Ƃ�Push
		inline void PushLocalRef(const std::int32_t arg_val)
		{
			push(valueStack[arg_val + stack_base]);
		}

		void PushLocalRef()
		{
			PushLocalRef(Constant<std::int32_t>());
		}

		// �z�񂩂�Q�Ƃ�Push
		inline void PushGlobalArrayRef(const std::int32_t arg_val)
		{
			std::int32_t index = top().Get<std::int32_t>(); pop();
			push(valueStack[(std::int32_t)(arg_val + index)]);
		}

		void PushGlobalArrayRef()
		{
			PushGlobalArrayRef(Constant<std::int32_t>());
		}

		// ���[�J���̔z�񂩂�Q�Ƃ�Push
		inline void PushLocalArrayRef(const std::int32_t arg_val)
		{
			std::int32_t index = top().Get<std::int32_t>(); pop();
			push(valueStack[arg_val + stack_base + index]);
		}

		void PushLocalArrayRef()
		{
			PushLocalArrayRef(Constant<std::int32_t>());
		}


		//�����o�ϐ��̎Q�Ƃ�push
		inline void PushMemberRef( const std::int32_t arg_valueIndex) {
			auto v = top().valueData;
			pop();
			push(v.get<IType_hasMember>()->GetMember(arg_valueIndex), v.get<IType_hasMember>()->GetMemberType(arg_valueIndex));
		}

		void PushMemberRef() {
			PushMemberRef( Constant<std::int32_t>());
		}



		// �A�h���X��Push
		inline void PushAddr(const std::int32_t arg_val)
		{
			std::int32_t base = arg_val;
			if ((arg_val & global_flag) == 0)	// local
				base += +stack_base;
			push(base);
		}

		void PushAddr() {
			PushAddr(Constant<std::int32_t>());
		}

		// �z��̃A�h���X��Push
		inline void PushArrayAddr(const std::int32_t arg_val)
		{
			std::int32_t base = arg_val;
			if ((arg_val & global_flag) == 0)	// local
				base += +stack_base;
			std::int32_t index = top().Get<std::int32_t>(); pop();
			push(base + index);
		}
		void PushArrayAddr() {
			PushArrayAddr(Constant<std::int32_t>());
		}


		/////////////Pop��`////////////////
		// �ϐ���Pop
		inline void PopValue(const std::int32_t arg_val)
		{
			valueStack[globalValue_base+ arg_val] = top(); pop();
		}
		void PopValue() {
			PopValue(Constant<std::int32_t>());
		}
		// ���[�J���ϐ���Pop
		inline void PopLocal(const std::int32_t arg_val)
		{
			valueStack[arg_val + stack_base] = top(); pop();
		}
		void PopLocal() {
			PopLocal(Constant<std::int32_t>());
		}

		//�����o�ϐ���Pop
		inline void PopMember(const std::int32_t arg_index)
		{
			auto v = top().valueData;
			pop();
			v.get<IType_hasMember>()->SetMember(top().valueData.Clone(), arg_index); pop();
		}
		void PopMember() {
			PopMember(Constant<std::int32_t>());
		}
		//�����o�ϐ���Pop(std::int32_t�Afloat)
		inline void PopMemberValueType(const std::int32_t arg_index)
		{
			auto v = top().valueData;
			pop();
			auto hasMember = v.get<IType_hasMember>();
			hasMember->GetMember(arg_index).Write(top().valueData.get()); pop();
		}
		void PopMemberValueType() {
			PopMemberValueType(Constant<std::int32_t>());
		}
		//�����o�ϐ���Pop(�Q��)
		inline void PopMemberRef(const std::int32_t arg_index)
		{
			auto v = top().valueData;
			pop();
			v.get<IType_hasMember>()->SetMember(top().valueData, arg_index); pop();
		}
		void PopMemberRef() {
			PopMemberRef(Constant<std::int32_t>());
		}

		// �z��ϐ���Pop
		inline void PopArray(const std::int32_t arg_val)
		{
			std::int32_t index = top().Get<std::int32_t>(); pop();
			valueStack[(std::int32_t)(arg_val + index)] = top(); pop();
		}
		void PopArray() {
			PopArray(Constant<std::int32_t>());
		}

		// ���[�J���̔z��ϐ���Pop
		inline void PopLocalArray(const std::int32_t arg_val)
		{
			std::int32_t index = top().Get<std::int32_t>(); pop();
			valueStack[arg_val + stack_base + index] = top(); pop();
		}

		void PopLocalArray() {
			PopLocalArray(Constant<std::int32_t>());
		}

		// ���[�J���ϐ�(�Q��)��Pop
		inline void PopLocalRef(const std::int32_t arg_val)
		{
			std::int32_t addr = valueStack[arg_val + stack_base].Get<std::int32_t>();
			SetRef(addr, top()); pop();
		}
		void PopLocalRef() {
			PopLocalRef(Constant<std::int32_t>());
		}
		// ���[�J���̔z��ϐ�(�Q��)��Pop
		inline void PopLocalArrayRef(const std::int32_t arg_val)
		{
			std::int32_t addr = valueStack[arg_val + stack_base].Get<std::int32_t>();
			std::int32_t index = top().Get<std::int32_t>(); pop();
			SetRef(addr + index, top()); pop();
		}
		void PopLocalArrayRef()
		{
			PopLocalArrayRef(Constant<std::int32_t>());
		}


		// ��Pop�i�X�^�b�N�g�b�v���̂Ă�j
		void OpPop()
		{
			pop();
		}

		/////////////Alloc��`////////////////
		// ���[�J���ϐ����m��
		inline void OpAllocStack(const std::int32_t arg_val)
		{
			(this->*p_pushValues[arg_val])();
		}
		void OpAllocStack()
		{
			OpAllocStack(Constant<std::int32_t>());
		}

		void OpAllocStackEnumType() {
			std::int32_t type = Constant<std::int32_t>();
			auto value = Value(Type_Enum(0,&vlp_data->map_enumTag.at(type)));
			value.SetType(type);
			this->valueStack.push(value);
		}
		//�֐��I�u�W�F�N�g�^�̊m��
		void OpAllocStackFunctionType() {
			std::int32_t type = Constant<std::int32_t>();
			auto value = Value(Type_Function( &vlp_data->map_functionJumpPointsTable, ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>,std::int32_t>>()));
			value.SetType(type);
			this->valueStack.push(value);
		}

		// ���[�J���ϐ�(�Q�ƌ^)���m��
		inline void OpAllocStack_Ref(const std::int32_t arg_val)
		{
			(this->*p_pushRefValues[arg_val & ~TYPE_REF])();

		}
		void OpAllocStack_Ref()
		{
			OpAllocStack_Ref(Constant<std::int32_t>());
		}
		// ���[�J���ϐ����m��(�X�N���v�g��`)
		inline void OpAllocStack_ScriptType(const std::int32_t arg_val)
		{
			pushValue(&list_scriptClassInfo[arg_val],&list_scriptClassInfo);

		}
		void OpAllocStack_ScriptType()
		{
			OpAllocStack_ScriptType(Constant<std::int32_t>());
		}

		// ���[�J���ϐ�(�Q�ƌ^)���m��(�X�N���v�g��`)
		inline void OpAllocStack_Ref_ScriptType(const std::int32_t arg_val)
		{
			pushValue_ref(&list_scriptClassInfo[arg_val]);
		}
		void OpAllocStack_Ref_ScriptType()
		{
			OpAllocStack_Ref_ScriptType(Constant<std::int32_t>());
		}
		// ���[�J���ϐ�(�Q�ƌ^)���m��(�񋓌^)
		void OpAllocStack_Ref_EnumType()
		{
			OpAllocStack_Ref(Constant<std::int32_t>());
		}
		// ���[�J���ϐ�(�Q�ƌ^)���m��(�֐��^)
		void OpAllocStack_Ref_FunctionType()
		{
			OpAllocStack_Ref(Constant<std::int32_t>());
		}




		/////////////���Z�q��`////////////////

		// �L���X�g
		template<typename BeforeType,typename AfterType>
		void OpCast()
		{
			auto cast= top();
			pop();
			push(cast.Cast<BeforeType,AfterType>());
		}

		// -
		template<typename T>
		void OpNeg()
		{
			auto negV = top().Clone();
			negV.Negative<T>();
			pop();
			push(negV);
		}

		// !
		template<typename T>
		void OpNot() {
			auto notV = top().Clone();
			notV.Not<T>();
			pop();
			push(notV);
		}

		// ==
		template<typename LeftType,typename RightType>
		void OpEq()
		{
			auto rhs = top().valueData; pop();
			auto lhs = top().valueData; pop();
			push(ButiValueOperator::Equal<LeftType, RightType>(lhs, rhs));
		}

		// !=
		template<typename LeftType, typename RightType>
		void OpNe()
		{
			auto rhs = top().valueData; pop();
			auto lhs = top().valueData; pop();
			push(!ButiValueOperator::Equal<LeftType, RightType>(lhs, rhs));
		}

		// >
		template<typename LeftType, typename RightType>
		void OpGt()
		{
			auto rhs = top().valueData; pop();
			auto lhs = top().valueData; pop();
			push(ButiValueOperator::GreaterThan<LeftType, RightType>(lhs, rhs));
		}

		// >=
		template<typename LeftType, typename RightType>
		void OpGe()
		{
			auto rhs = top().valueData; pop();
			auto lhs = top().valueData; pop();
			push(ButiValueOperator::GreaterEqual<LeftType, RightType>(lhs, rhs));
		}

		// <
		template<typename LeftType, typename RightType>
		void OpLt()
		{
			auto rhs = top().valueData; pop();
			auto lhs = top().valueData; pop();
			push(!ButiValueOperator::GreaterEqual<LeftType, RightType>(lhs, rhs));
		}

		// <=
		template<typename LeftType, typename RightType>
		void OpLe()
		{
			auto rhs = top().valueData; pop();
			auto lhs = top().valueData; pop();
			push(!ButiValueOperator::GreaterThan<LeftType, RightType>(lhs, rhs));

		}

		// &&
		void OpLogAnd()
		{
			auto rhs = top().Get<std::int32_t>(); pop();
			auto lhs = top().Get<std::int32_t>(); pop();
			push(lhs && rhs);
		}

		// ||
		void OpLogOr()
		{
			auto rhs = top().Get<std::int32_t>(); pop();
			auto lhs = top().Get<std::int32_t>(); pop();
			push(lhs || rhs);
		}

		// &
		void OpAnd()
		{
			auto rhs = top().Get<std::int32_t>(); pop();
			auto lhs = top().Get<std::int32_t>(); pop();
			push(lhs & rhs);
		}

		// |
		void OpOr()
		{
			auto rhs = top().Get<std::int32_t>(); pop();
			auto lhs = top().Get<std::int32_t>(); pop();
			push(lhs | rhs);
		}

		// <<
		void OpLeftShift()
		{
			auto rhs = top().Get<std::int32_t>(); pop();
			auto lhs = top().Get<std::int32_t>(); pop();
			push(lhs << rhs);
		}

		// >>
		void OpRightShift()
		{
			auto rhs = top().Get<std::int32_t>(); pop();
			auto lhs = top().Get<std::int32_t>(); pop();
			push(lhs >> rhs);
		}

		//++
		template<typename T>
		void OpIncrement()
		{
			top().Increment<T>();			
		}
		//--
		template<typename T>
		void OpDecrement()
		{
			top().Decrement<T>();
		}

		// +
		template<typename LeftType,typename RightType,typename RetType>
		void OpAdd()
		{
			auto rhs = top(); pop();
			auto lhs = top(); pop();
			push(ButiValueOperator::Addition<LeftType,RightType,RetType>( lhs , rhs));
		}

		// -
		template<typename LeftType, typename RightType, typename RetType>
		void OpSub()
		{
			auto rhs = top(); pop();
			auto lhs = top(); pop();
			push(ButiValueOperator::Subtract<LeftType, RightType, RetType>(lhs , rhs));
		}

		// *
		template<typename LeftType, typename RightType, typename RetType>
		void OpMul()
		{
			auto rhs = top(); pop();
			auto lhs = top(); pop();
			push(ButiValueOperator::Multiple<LeftType, RightType, RetType>(lhs , rhs));
		}

		// /
		template<typename LeftType, typename RightType, typename RetType>
		void OpDiv()
		{
			auto rhs = top(); pop();
			auto lhs = top(); pop();
			push(ButiValueOperator::Division<LeftType, RightType, RetType>(lhs , rhs));
		}


		// %
		template<typename LeftType, typename RightType>
		void OpMod()
		{
			std::int32_t rhs = top().Get<RightType>(); pop();
			std::int32_t lhs = top().Get<LeftType>(); pop();
			push(ButiValueOperator::Mod<LeftType, RightType>(lhs, rhs));
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
		inline void OpJmp(const std::int32_t arg_val)
		{
			jmp(arg_val);
		}
		void OpJmp() {
			OpJmp(Constant<std::int32_t>());
		}

		// �^�̎��W�����v
		inline void OpJmpC(const std::int32_t arg_val)
		{
			std::int32_t cond = top().Get<std::int32_t>(); pop();
			if (cond)
				jmp(arg_val);
		}
		void OpJmpC() {
			OpJmpC(Constant<std::int32_t>());
		}

		// �U�̎��W�����v
		inline void OpJmpNC(const std::int32_t arg_val)
		{
			std::int32_t cond = top().Get<std::int32_t>(); pop();
			if (!cond)
				jmp(arg_val);
		}
		void OpJmpNC() {
			OpJmpNC(Constant<std::int32_t>());
		}

		// switch���p���ꔻ��
		inline void OpTest(const std::int32_t arg_val)
		{
			std::int32_t Value = top().Get<std::int32_t>(); pop();
			if (Value == top().Get<std::int32_t>()) {
				pop();
				jmp(arg_val);
			}
		}
		void OpTest() {
			OpTest(Constant<std::int32_t>());
		}

		/////////////�֐��Ăяo����`////////////////
		// �֐��R�[��
		inline void OpCall(const std::int32_t arg_val)
		{
			push(stack_base);
			push(addr());					
			stack_base = valueStack.size();		
			jmp(arg_val);
		}

		void OpCall() {
			OpCall(Constant<std::int32_t>());
		}

		void OpCallByVariable() {
			auto& functionObject = top().Get<Type_Function>(); pop();
			push(stack_base);
			push(addr());					
			stack_base = valueStack.size();		
			for (auto& capturedValue: functionObject.list_capturedValue) {
				push(capturedValue.first, capturedValue.second);
			}

			jmp(functionObject.address);
		}

		// �����Ȃ����^�[��
		void OpReturn()
		{
			valueStack.resize(stack_base);		
			std::int32_t addr = top().Get<std::int32_t>(); pop();
			stack_base = top().Get<std::int32_t>(); pop();
			std::int32_t arg_count = top().Get<std::int32_t>(); pop();
			valueStack.pop(arg_count);
			jmp(addr);
		}

		// �����t�����^�[��
		void OpReturnV()
		{
			ButiScript::Value result = top(); 
			pop();
			valueStack.resize(stack_base);		
			std::int32_t addr = top().Get<std::int32_t>(); pop();
			stack_base = top().Get<std::int32_t>(); pop();
			std::int32_t arg_count = top().Get<std::int32_t>(); pop();
			valueStack.pop(arg_count);
			push(result);
			jmp(addr);
		}

		//null��Push
		inline void PushNull() {
			push(Type_Null());
		}

		//�֐��I�u�W�F�N�g�̃A�h���X��Push
		inline void OpPushFunctionAddress(const std::int32_t arg_address) {
			push(arg_address);
		}
		//�֐��I�u�W�F�N�g�̃A�h���X��Push
		void OpPushFunctionAddress() {

			OpPushFunctionAddress(Constant<std::int32_t>());
		}
		//�����_���̐�����Push
		void OpPushLambda() {
			std::int32_t captureListSize = top().Get<std::int32_t>(); pop();
			ButiEngine::List<std::int32_t> captureList;
			for (std::int32_t i = 0; i < captureListSize; i++) {
				captureList.Add(top().Get<std::int32_t>());
				pop();
			}

			std::int32_t type = top().Get<std::int32_t>(); pop();
			std::int32_t address = Constant<std::int32_t>();
			auto value = Value(Type_Function(address, &vlp_data->map_functionJumpPointsTable, ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>,std::int32_t>>()));
			
			for (auto itr = captureList.rbegin(), end = captureList.rend(); itr != end;itr++) {
				value.valueData.get<Type_Function>()->AddCapture (valueStack[stack_base+ *itr].valueData, valueStack[stack_base + *itr].valueType);
			}
			value.SetType(type);
			this->valueStack.push(value);
		}

		// ���zCPU��~
		void OpHalt()
		{
		}
		// �g�ݍ��݊֐�
		inline void OpSysCall(const std::int32_t arg_val)
		{
			pop();	// arg_count
			(this->*p_syscall[arg_val])();
		}

		void OpSysCall() {
			OpSysCall(Constant<std::int32_t>());
		}

		//�g�ݍ��݃��\�b�h
		inline void OpSysMethodCall(const std::int32_t arg_val)
		{
			pop();	// arg_count
			(this->*p_sysMethodCall[arg_val])();
		}
		void OpSysMethodCall() {
			OpSysMethodCall(Constant<std::int32_t>());
		}

	public:


#ifdef _BUTIENGINEBUILD

		void sys_addEventMessanger();
		void sys_removeEventMessanger();
		void sys_registEventListner();
		void sys_unregistEventListner();
		void sys_executeEvent();
		void sys_pushTask();
		void sys_LoadTextureAsync();
		void sys_LoadWaveAsync();
		void sys_LoadWavesAsync();
		void sys_LoadTexture();
		void sys_LoadWave();
		void sys_getSelfScriptBehavior();
		void sys_get_ownGameObject() {
			push(vlp_gameObject);
		}
		void sys_get_gameObjectByName() {
			std::string name = top().Get<std::string>(); pop();
			auto obj = vlp_gameObject->GetGameObjectManager().lock()->GetGameObject(name).lock();
			push(obj);
		}
		void sys_getKeyboard() {
			std::int32_t k = top().Get<std::int32_t>(); pop();
			std::int32_t res = ButiEngine::GameDevice::GetInput()->CheckKey(k);
			push(res);
		}
		void sys_triggerKeyboard() {
			std::int32_t k = top().Get<std::int32_t>(); pop();
			std::int32_t res = ButiEngine::GameDevice::GetInput()->TriggerKey(k);
			push(res);
		}
		void sys_releaseKeyboard() {
			std::int32_t k = top().Get<std::int32_t>(); pop();
			std::int32_t res = ButiEngine::GameDevice::GetInput()->ReleaseKey(k);
			push(res);
		}
		void sys_checkAnyKeyboard() {
			std::int32_t res = ButiEngine::GameDevice::GetInput()->GetAnyButton();
			push(res);
		}
		void sys_triggerAnyKeyboard() {
			std::int32_t res = ButiEngine::GameDevice::GetInput()->GetAnyButtonTrigger();
			push(res);
		}
		void sys_getPadButton() {
			std::int32_t k = top().Get<std::int32_t>(); pop();
			std::int32_t res = ButiEngine::GameDevice::GetInput()->GetPadButton((ButiEngine::PadButtons)k);
			push(res);
		}
		void sys_triggerPadButton() {
			std::int32_t k = top().Get<std::int32_t>(); pop();
			std::int32_t res = ButiEngine::GameDevice::GetInput()->GetPadButtonTrigger((ButiEngine::PadButtons)k);
			push(res);
		}
		void sys_releasePadButton() {
			std::int32_t k = top().Get<std::int32_t>(); pop();
			std::int32_t res = ButiEngine::GameDevice::GetInput()->GetPadButtonRelease((ButiEngine::PadButtons)k);
			push(res);
		}
		void sys_getMouseButton() {
			std::int32_t k = top().Get<std::int32_t>(); pop();
			std::int32_t res = ButiEngine::GameDevice::GetInput()->GetMouseButton((ButiEngine::MouseButtons)k);
			push(res);
		}
		void sys_triggerMouseButton() {
			std::int32_t k = top().Get<std::int32_t>(); pop();
			std::int32_t res = ButiEngine::GameDevice::GetInput()->GetMouseTrigger((ButiEngine::MouseButtons)k);
			push(res);
		}
		void sys_releaseMouseButton() {
			std::int32_t k = top().Get<std::int32_t>(); pop();
			std::int32_t res = ButiEngine::GameDevice::GetInput()->GetMouseReleaseTrigger((ButiEngine::MouseButtons)k);
			push(res);
		}
		void sys_getLStick() {
			auto res = ButiEngine::GameDevice::GetInput()->GetLeftStick();
			push(res);
		}
		void sys_getRStick() {
			auto res = ButiEngine::GameDevice::GetInput()->GetRightStick();
			push(res);
		}
		void sys_getMousePos() {
			auto res = ButiEngine::GameDevice::GetInput()->GetMousePos();
			push(res);
		}
		void sys_getMouseMove() {
			auto res = ButiEngine::GameDevice::GetInput()->GetMouseMove();
			push(res);
		}
		void sys_getWheel() {
			auto res = ButiEngine::GameDevice::GetInput()->GetMouseWheelMove();
			push(res);
		}
		void sys_getAnyButton() {
			std::int32_t res = ButiEngine::GameDevice::GetInput()->GetAnyButton();
			push(res);
		}
		void sys_triggerAnyButton() {
			std::int32_t res = ButiEngine::GameDevice::GetInput()->GetAnyButtonTrigger();
			push(res);
		}
		void sys_getLeftTrigger() {
			float res = ButiEngine::GameDevice::GetInput()->GetLeftTrigger();
			push(res);
		}
		void sys_getRightTrigger() {
			float res = ButiEngine::GameDevice::GetInput()->GetRightTrigger();
			push(res);
		}
		void sys_playSE() {
			auto seName = top().Get<std::string>(); pop();
			auto volume = top().Get<float>(); pop();
			vlp_gameObject->GetApplication().lock()->GetSoundManager()->PlaySE(ButiEngine::SoundTag(seName), volume);
		}
		void sys_playSE_noVolume() {
			auto seName = top().Get<std::string>(); pop();
			vlp_gameObject->GetApplication().lock()->GetSoundManager()->PlaySE(ButiEngine::SoundTag(seName), 1.0f);
		}
		void sys_playBGM() {
			auto bgmName = top().Get<std::string>(); pop();
			auto volume = top().Get<float>(); pop();
			vlp_gameObject->GetApplication().lock()->GetSoundManager()->PlayBGM(ButiEngine::SoundTag(bgmName), volume);
		}
		void sys_playBGM_noVolume() {
			auto bgmName = top().Get<std::string>(); pop();
			vlp_gameObject->GetApplication().lock()->GetSoundManager()->PlayBGM(ButiEngine::SoundTag(bgmName), 1.0f);
		}

		void sys_printColor() {
			ButiEngine::Vector4 color=top().Get<ButiEngine::Vector4>();	pop();
			auto message = text(top());	pop();
			ButiEngine::GUI::Console(message,color);
		}
#endif // _BUTIENGINEBUILD

		// �g�ݍ��݊֐��iprint�j
		void sys_print()
		{
#ifdef _BUTIENGINEBUILD

			ButiEngine::GUI::Console( text(top()));
#else
			std::cout << text(top());
#endif // _BUTIENGINEBUILD
			pop();
		}

		void sys_debugPrint() {
			std::cout << text(top());
			pop();
		}

		// �g�ݍ��݊֐��ipause�j
		void Sys_pause() {
			std::system("pause");			
		}

		// �g�ݍ��݊֐�(���l�𕶎���ɕϊ�)
		void sys_tostr()
		{
			auto v = top().valueData.ToString(); pop();
			push(v);
			// �߂�l�̓X�^�b�N�ɓ����
		}

		//�g�ݍ��݊֐�

		template<auto Function >
		void sys_func()
		{
			if constexpr (ButiTypeDetail::function_argment_size<Function>) {
				SystemFunctionImple< ButiTypeDetail::function_argment_size<Function>, Function>(*this);
			}
			else
			if constexpr (std::is_void_v<ButiTypeDetail::function_return_type<Function>>) {
				(*Function)();
			}
			else {
				auto ret = (*Function)();
				push(ret);
			}
		}

		/////////////���\�b�h�Ăяo����`////////////////

		template<typename T>
		T* GetSharedTypePtr() {
			return &(top().Get<std::_Remove_cvref_t< T>>());
		}
		template<typename T>
		T* GetTypePtr() {
			return &(top().Get<std::_Remove_cvref_t< T>>());
		}

		//�g�ݍ��݃��\�b�h
		template<auto Method, ButiTypeDetail::member_function_class_type<Method>* (VirtualMachine::* getValueFunc)() >
		void sys_method()
		{
			auto v = ((this)->*getValueFunc)();
			if constexpr (std::is_void_v<ButiTypeDetail::member_function_return_type<Method>>) {
				((v)->*Method)();
				pop();
			}
			else {
				auto ret = ((v)->*Method)();
				pop();
				push(ret);
			}
		}
		template<auto Method, typename CastType, ButiTypeDetail::member_function_class_type<Method>* (VirtualMachine::* getValueFunc)() >
		void sys_method_cast()
		{
			auto v = ((this)->*getValueFunc)();
			CastType ret =static_cast<CastType>( ((v)->*Method)());
			pop();
			push(ret);
		}
		//�g�ݍ��݃��\�b�h(�����L��)
		template<auto Method,  ButiTypeDetail::member_function_class_type<Method>* (VirtualMachine::* getValueFunc)(), 
			std::_Remove_cvref_t<ButiTypeDetail::member_function_argment_type<Method,0>>* (VirtualMachine::* getArgValueFunc)() >
		void sys_method()
		{
			auto valueV = top().valueData;
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg = *((this)->*getArgValueFunc)();
			pop(); 
			if constexpr (std::is_void_v<ButiTypeDetail::member_function_return_type<Method>>) {
				((v)->*Method)(arg);
			}
			else {
				auto ret = ((v)->*Method)(arg);
				push(ret);
			}
		}
		//�g�ݍ��݃��\�b�h(����2)
		template<auto Method, ButiTypeDetail::member_function_class_type<Method>* (VirtualMachine::* getValueFunc)()
			, std::_Remove_cvref_t<ButiTypeDetail::member_function_argment_type<Method, 0>>* (VirtualMachine::* getArg1ValueFunc)(), std::_Remove_cvref_t<ButiTypeDetail::member_function_argment_type<Method, 1>>* (VirtualMachine::* getArg2ValueFunc)() >
		void sys_method()
		{
			auto valueV = top().valueData;
			auto v = ((this)->*getValueFunc)();
			pop();
			auto arg2 =* ((this)->*getArg2ValueFunc)();
			pop();
			auto arg1 = *((this)->*getArg1ValueFunc)();
			pop();
			if constexpr (std::is_void_v<ButiTypeDetail::member_function_return_type<Method>>) {
				((v)->*Method)(arg1,arg2);
			}
			else {
				auto ret = ((v)->*Method)(arg1,arg2);
				push(ret);
			}

		}

		template<typename T>
		void pushValue() {
			auto value = Value(T());
			std::int64_t address = TypeSpecific<T>();
			value.SetType(Value::GetTypeIndex(address));
			this->valueStack.push(value);
		}
		template<typename T>
		void pushValue_valueptr() {
			auto value = Value();
			std::int64_t address = TypeSpecific<T>();
			value.SetType(Value::GetTypeIndex(address));
			this->valueStack.push(value);
		}
		template<>
		void pushValue<Type_Null>() {
			auto value = Value(Type_Null());
			std::int64_t address = TypeSpecific<Type_Null>();
			value.SetType(Value::GetTypeIndex(address));
			this->valueStack.push(value);
		}
		template<typename T>
		void pushValue_ref() {
			auto value = Value();
			std::int64_t address = TypeSpecific<T>();
			value.SetType(Value::GetTypeIndex(address) | TYPE_REF);
			this->valueStack.push(value);
		}
		template<typename T>
		void pushValue_valueptr_ref() {
			auto value = Value();
			std::int64_t address = TypeSpecific<T>();
			value.SetType(Value::GetTypeIndex(address) | TYPE_REF);
			this->valueStack.push(value);
		}
		void pushValue(ScriptClassInfo* info, ButiEngine::List<ButiScript::ScriptClassInfo>* p_list_scriptClassInfo) {
			auto value = Value(*info,p_list_scriptClassInfo);
			this->valueStack.push(value);
		}
		inline void pushValue_ref(ScriptClassInfo* info) {
			auto value = Value(Type_Null());
			value.SetType(info->GetTypeIndex() | TYPE_REF);
			this->valueStack.push(value);
		}

	private:

		template <typename T>
		T Constant(){ T v = *(T*)command_ptr_; command_ptr_ += sizeof(T); return v; }

		std::int32_t addr() const { return (std::int32_t)(command_ptr_ - commandTable); }
		void jmp(const std::int32_t addr) { command_ptr_ = commandTable + addr; }
		void push(const std::int32_t arg_v) {
			valueStack.push(ButiScript::Value(arg_v));
		}
		void push(const float arg_v) {
			valueStack.push(ButiScript::Value(arg_v));
		}
		void push(const std::string& arg_v) {
			valueStack.push(ButiScript::Value(arg_v));
		}
		template<typename T>
		void push(ButiEngine::Value_ptr<T>& arg_valueData) {
			valueStack.push(ButiScript::Value(arg_valueData,TYPE_VOID));
		}
		template<typename T>
		void push_clone(const ButiEngine::Value_ptr<T>& arg_valueData) {
			valueStack.push(ButiScript::Value(arg_valueData.Clone(), TYPE_VOID));
		}
		void push(ButiEngine::Value_ptr<void>& arg_valueData, const std::int32_t arg_type) {
			valueStack.push(ButiScript::Value(arg_valueData, arg_type));
		}
		void push_clone(const ButiEngine::Value_ptr<void>& arg_valueData, const std::int32_t arg_type) {
			valueStack.push(ButiScript::Value(arg_valueData.Clone(), arg_type));
		}
		void push(const ButiScript::Value& arg_v) {
			valueStack.push(arg_v);
		}
		void push(const Type_Null&) {
			auto value = Value(Type_Null());
			value.SetType(TYPE_VOID);
			this->valueStack.push(value);
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
		std::string text(const ButiScript::Value& arg_v) { return arg_v.valueData.ToString(); }
		const ButiScript::Value& ref_to_value(const std::int32_t arg_addr) const
		{
			if (arg_addr & global_flag) {
				return valueStack[arg_addr & global_mask];
			}
			return valueStack[arg_addr];
		}
		void SetRef(const std::int32_t arg_addr, const ButiScript::Value& arg_v)
		{
			if (arg_addr & global_flag)
				PopLocal(arg_addr-1);
			else
				valueStack[arg_addr] = arg_v;
		}

	private:
		ButiEngine::Value_ptr<CompiledData> vlp_data;

		//�R�}���h����
		std::uint8_t* commandTable;
		//���ݎQ�Ƃ��Ă�R�}���h�̈ʒu
		std::uint8_t* command_ptr_=nullptr;
		//�O���[�o���ϐ��̊m�ۃR�}���h
		std::uint8_t* allocCommand_ptr_=nullptr;
		//�v���O�����S�̂̃T�C�Y
		std::int32_t commandSize;
		//������f�[�^
		char* textBuffer;
		//������f�[�^�̃T�C�Y
		std::int32_t textSize;

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

		ButiEngine::List<ScriptClassInfo> list_scriptClassInfo;

		ButiScript::Stack<ButiScript::Value, STACK_SIZE> valueStack;

		//�X�^�b�N�̎Q�ƈʒu
		std::int32_t stack_base=0;
		std::int32_t globalValue_base = 0;
		std::int32_t globalValue_size=0;
		//�O���[�o���ϐ��m�ۂ̖��ߐ�
		std::int32_t globalValueAllocOpSize=0;
#ifdef _BUTIENGINEBUILD
		ButiEngine::Value_ptr<ButiEngine::GameObject> vlp_gameObject;
		ButiEngine::Value_weak_ptr < ButiEngine::ButiScriptBehavior >wkp_butiScriptBehavior;
#endif
	};
}


#endif
