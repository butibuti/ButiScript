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
		unsigned char* commandTable;	// コマンドテーブル
		char* textBuffer;			// テキストデータ
		int commandSize;			// コマンドサイズ
		int textSize;				// テキストサイズ
		int valueSize;			// グローバル変数サイズ

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


	// 0除算
	class DevideByZero : public std::exception {
	public:
		const char* what() const throw()
		{
			return "devide by zero";
		}
	};

	// 仮想マシン
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
			stack_base = valueStack.size();					// スタック参照位置初期化
			push(0);										// mainへの引数カウントをpush
			push(stack_base);								// stack_baseの初期値をpush
			push(0);										// プログラム終了位置をpush

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
			stack_base = valueStack.size();					// スタック参照位置初期化
			push(0);										// mainへの引数カウントをpush
			push(stack_base);								// stack_baseの初期値をpush
			push(0);										// プログラム終了位置をpush

			Execute_(entryPoint);

			valueStack.resize(globalValue_size);
		}

		template<typename T, typename U>
		int Execute(const std::string& entryPoint, U argment) {

			stack_base = valueStack.size();						// スタック参照位置初期化
			push(argment);									//引数push
			push(1);										// mainへの引数カウントをpush
			push(stack_base);										// stack_baseの初期値をpush
			push(0);										// プログラム終了位置をpush
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
				ButiEngine::GUI::Console( arg_variableName + "にはアクセスできません",ButiEngine::Vector4(1.0f,0.8f,0.8f,1.0f) );
#endif

				return;
			}
			valueStack[globalValue_base + data_->map_globalValueAddress.at(arg_variableName)].v_->Set(value);
		}
		template<typename T>
		T& GetGlobalVariable(const std::string arg_variableName) {
			if (!data_->map_globalValueAddress.count(arg_variableName)) {

#ifdef IMPL_BUTIENGINE
				ButiEngine::GUI::Console(arg_variableName + "にはアクセスできません", ButiEngine::Vector4(1.0f, 0.8f, 0.8f, 1.0f));
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

		/////////////定数Push定義////////////////
		// 定数Push
		inline void PushConstInt(const int arg_val)
		{
			push(arg_val);
		}
		void PushConstInt()
		{
			PushConstInt(Value_Int());
		}

		// 定数Push
		inline void PushConstFloat(const float arg_val)
		{
			push(arg_val);
		}
		void PushConstFloat()
		{
			PushConstFloat(Value_Float());
		}

		// 文字定数Push
		inline void PushString(const int arg_val)
		{
			push(std::string(textBuffer + arg_val));
		}
		void PushString()
		{
			PushString(Value_Int());
		}



		/////////////変数Push定義////////////////
		// グローバル変数のコピーをPush
		inline void PushGlobalValue(const int arg_val)
		{
			push(valueStack[globalValue_base + arg_val].Clone());
		}
		void PushGlobalValue()
		{
			PushGlobalValue(Value_Int());
		}

		// ローカル変数のコピーをPush
		inline void PushLocal(const int arg_val)
		{
			push(valueStack[arg_val + stack_base].Clone());
		}

		void PushLocal()
		{
			PushLocal(Value_Int());
		}

		// 配列からコピーをPush
		inline void PushGlobalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(valueStack[(int)(arg_val + index)].Clone());
		}

		void PushGlobalArray()
		{
			PushGlobalArray(Value_Int());
		}

		// ローカルの配列からコピーをPush
		inline void PushLocalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(valueStack[arg_val + stack_base + index].Clone());
		}

		void PushLocalArray()
		{
			PushLocalArray(Value_Int());
		}

		//グローバル変数のメンバ変数のコピーをpush
		inline void PushMember( const int arg_valueIndex) {
			auto v_ = top().v_;
			pop();
			push_clone(v_->GetMember(arg_valueIndex), v_->GetMemberType(arg_valueIndex));
			top().v_->release();
		}

		void PushMember() {
			PushMember( Value_Int());
		}

		/////////////グローバル変数の参照Push定義////////////////

		// グローバル変数の参照をPush
		inline void PushGlobalValueRef(const int arg_val)
		{
			push(valueStack[globalValue_base + arg_val]);
		}
		void PushGlobalValueRef()
		{
			PushGlobalValueRef(Value_Int());
		}

		// ローカル変数の参照をPush
		inline void PushLocalRef(const int arg_val)
		{
			push(valueStack[arg_val + stack_base]);
		}

		void PushLocalRef()
		{
			PushLocalRef(Value_Int());
		}

		// 配列から参照をPush
		inline void PushGlobalArrayRef(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(valueStack[(int)(arg_val + index)]);
		}

		void PushGlobalArrayRef()
		{
			PushGlobalArrayRef(Value_Int());
		}

		// ローカルの配列から参照をPush
		inline void PushLocalArrayRef(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			push(valueStack[arg_val + stack_base + index]);
		}

		void PushLocalArrayRef()
		{
			PushLocalArrayRef(Value_Int());
		}


		//メンバ変数の参照をpush
		inline void PushMemberRef( const int arg_valueIndex) {
			auto v = top().v_;
			pop();
			push(v->GetMember(arg_valueIndex), v->GetMemberType(arg_valueIndex));
		}

		void PushMemberRef() {
			PushMemberRef( Value_Int());
		}



		// アドレスをPush
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

		// 配列のアドレスをPush
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


		/////////////Pop定義////////////////
		// 変数にPop
		inline void PopValue(const int arg_val)
		{
			valueStack[globalValue_base+ arg_val] = top(); pop();
		}
		void PopValue() {
			PopValue(Value_Int());
		}
		// ローカル変数にPop
		inline void PopLocal(const int arg_val)
		{
			valueStack[arg_val + stack_base] = top(); pop();
		}
		void PopLocal() {
			PopLocal(Value_Int());
		}

		//メンバ変数にPop
		inline void PopMember(const int arg_index)
		{
			auto v = top().v_;
			pop();
			v->GetMember(arg_index)->Set(*top().v_); pop();
		}
		void PopMember() {
			PopMember(Value_Int());
		}
		//メンバ変数にPop(参照)
		inline void PopMemberRef(const int arg_index)
		{
			auto v = top().v_;
			pop();
			v->SetMember(top().v_, arg_index); pop();
		}
		void PopMemberRef() {
			PopMemberRef(Value_Int());
		}

		// 配列変数にPop
		inline void PopArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			valueStack[(int)(arg_val + index)] = top(); pop();
		}
		void PopArray() {
			PopArray(Value_Int());
		}

		// ローカルの配列変数にPop
		inline void PopLocalArray(const int arg_val)
		{
			int index = top().v_->Get<int>(); pop();
			valueStack[arg_val + stack_base + index] = top(); pop();
		}

		void PopLocalArray() {
			PopLocalArray(Value_Int());
		}

		// ローカル変数(参照)にPop
		inline void PopLocalRef(const int arg_val)
		{
			int addr = valueStack[arg_val + stack_base].v_->Get<int>();
			set_ref(addr, top()); pop();
		}
		void PopLocalRef() {
			PopLocalRef(Value_Int());
		}
		// ローカルの配列変数(参照)にPop
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


		// 空Pop（スタックトップを捨てる）
		void OpPop()
		{
			pop();
		}

		/////////////Alloc定義////////////////
		// ローカル変数を確保
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

		// ローカル変数(参照型)を確保
		inline void OpAllocStack_Ref(const int arg_val)
		{
			(this->*p_pushRefValues[arg_val & ~TYPE_REF])();

		}
		void OpAllocStack_Ref()
		{
			OpAllocStack_Ref(Value_Int());
		}
		// ローカル変数を確保(スクリプト定義)
		inline void OpAllocStack_ScriptType(const int arg_val)
		{
			pushValue(&vec_scriptClassInfo[arg_val],&vec_scriptClassInfo);

		}
		void OpAllocStack_ScriptType()
		{
			OpAllocStack_ScriptType(Value_Int());
		}

		// ローカル変数(参照型)を確保(スクリプト定義)
		inline void OpAllocStack_Ref_ScriptType(const int arg_val)
		{
			pushValue_ref(&vec_scriptClassInfo[arg_val]);
		}
		void OpAllocStack_Ref_ScriptType()
		{
			OpAllocStack_Ref_ScriptType(Value_Int());
		}
		// ローカル変数(参照型)を確保(列挙型)
		void OpAllocStack_Ref_EnumType()
		{
			OpAllocStack_Ref(Value_Int());
		}
		// ローカル変数(参照型)を確保(関数型)
		void OpAllocStack_Ref_FunctionType()
		{
			OpAllocStack_Ref(Value_Int());
		}




		/////////////演算子定義////////////////

		// 単項マイナス
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

		// 異なる型*
		template<typename T,typename U>
		void OpMul()
		{
			auto rhs = top().v_->Get<U>(); pop();
			auto lhs = top().v_->Get<T>(); pop();
			push(lhs * rhs);
		}

		// 異なる型/
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




		// 文字列の==
		void OpStrEq()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs == rhs);
		}

		// 文字列の!=
		void OpStrNe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs != rhs);
		}

		// 文字列の>
		void OpStrGt()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs > rhs);
		}

		// 文字列の>=
		void OpStrGe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs >= rhs);
		}

		// 文字列の<
		void OpStrLt()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs < rhs);
		}

		// 文字列の<=
		void OpStrLe()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs <= rhs);
		}

		// 文字列の+
		void OpStrAdd()
		{
			const std::string& rhs = text(top()); pop();
			const std::string& lhs = text(top()); pop();

			push(lhs + rhs);
		}




		/////////////アドレス操作定義////////////////
		// 無条件ジャンプ
		inline void OpJmp(const int arg_val)
		{
			jmp(arg_val);
		}
		void OpJmp() {
			OpJmp(Value_Int());
		}

		// 真の時ジャンプ
		inline void OpJmpC(const int arg_val)
		{
			int cond = top().v_->Get<int>(); pop();
			if (cond)
				jmp(arg_val);
		}
		void OpJmpC() {
			OpJmpC(Value_Int());
		}

		// 偽の時ジャンプ
		inline void OpJmpNC(const int arg_val)
		{
			int cond = top().v_->Get<int>(); pop();
			if (!cond)
				jmp(arg_val);
		}
		void OpJmpNC() {
			OpJmpNC(Value_Int());
		}

		// switch文用特殊判定
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

		/////////////関数呼び出し定義////////////////
		// 関数コール
		inline void OpCall(const int arg_val)
		{
			push(stack_base);
			push(addr());					// リターンアドレスをPush
			stack_base = valueStack.size();		// スタックベース更新
			jmp(arg_val);
		}

		void OpCall() {
			OpCall(Value_Int());
		}

		void OpCallByVariable() {
			int addr = top().v_->Get<int>(); pop();
			OpCall(addr);
		}

		// 引数なしリターン
		void OpReturn()
		{
			valueStack.resize(stack_base);		// ローカル変数排除
			int addr = top().v_->Get<int>(); pop();
			stack_base = top().v_->Get<int>(); pop();
			int arg_count = top().v_->Get<int>(); pop();
			valueStack.pop(arg_count);
			jmp(addr);
		}

		// 引数付きリターン
		void OpReturnV()
		{
			ButiScript::Value result = top(); 
			pop();
			valueStack.resize(stack_base);		// ローカル変数排除
			int addr = top().v_->Get<int>(); pop();
			stack_base = top().v_->Get<int>(); pop();
			int arg_count = top().v_->Get<int>(); pop();
			valueStack.pop(arg_count);
			push(result);
			jmp(addr);
		}

		//関数オブジェクトのアドレスをPush
		inline void OpPushFunctionAddress(const int address) {
			push(address);
		}
		//関数オブジェクトのアドレスをPush
		void OpPushFunctionAddress() {

			OpPushFunctionAddress(Value_Int());
		}

		// 仮想CPU停止
		void OpHalt()
		{
		}
		// 組み込み関数
		inline void OpSysCall(const int val)
		{
			pop();	// arg_count
			(this->*p_syscall[val])();
		}

		void OpSysCall() {
			OpSysCall(Value_Int());
		}

		//組み込みメソッド
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

		// 組み込み関数（print）
		void sys_print()
		{
#ifdef IMPL_BUTIENGINE

			ButiEngine::GUI::Console( text(top()));
#else

			std::cout << text(top());
#endif // IMPL_BUTIENGINE

			pop();
		}

		// 組み込み関数（pause）
		void Sys_pause() {
			std::system("pause");			
		}

		// 組み込み関数(数値を文字列に変換)
		void sys_tostr()
		{
			auto v = top().v_->Get<std::string>(); pop();
			push(v);
			// 戻り値はスタックに入れる
		}

		//組み込み関数(return 無し)
		template< void(*Method)() >
		void sys_func_retNo()
		{
			(*Method)();
		}
		//組み込み関数(return 有り)
		template<typename T, T(*Method)() >
		void sys_func_ret()
		{
			T ret = (*Method)();
			push(ret);
		}

		//組み込み関数(return 無し)
		template<typename Arg, void(*Method)(Arg) >
		void sys_func_retNo()
		{
			auto arg = top().v_->Get<Arg>(); pop(); 
			(*Method)(arg);
		}
		//組み込み関数(return 有り)
		template<typename T, typename Arg, T(*Method)(Arg) >
		void sys_func_ret()
		{
			auto arg = top().v_->Get<Arg>(); pop();
			T ret = (*Method)(arg);
			push(ret);
		}
		//組み込み関数(return 有り)
		template<typename T, typename Arg1, typename Arg2, T(*Method)(Arg1, Arg2) >
		void sys_func_ret()
		{
			auto arg2 = top().v_->Get<Arg2>(); pop();
			auto arg1 = top().v_->Get<Arg1>(); pop();
			T ret = (*Method)(arg1, arg2);
			push(ret);
		}
		//組み込み関数(return 有り)
		template<typename T, typename Arg1, typename Arg2, typename Arg3, T(*Method)(Arg1, Arg2, Arg3) >
		void sys_func_ret()
		{
			auto arg3 = top().v_->Get<Arg3>(); pop();
			auto arg2 = top().v_->Get<Arg2>(); pop();
			auto arg1 = top().v_->Get<Arg1>(); pop();
			T ret = (*Method)(arg1, arg2,arg3);
			push(ret);
		}
		

		/////////////メソッド呼び出し定義////////////////

		template<typename T>
		T* GetSharedTypePtr() {
			return &(*((Value_Shared<T>*)top().v_)->Get());
		}
		template<typename T>
		T* GetTypePtr() {
			return &(top().v_->GetRef<T>());
		}

		//組み込みメソッド(return 無し)
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
		//組み込みメソッド(return 有り)
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

		//組み込みメソッド(return 無し、引数有り)
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

		//組み込みメソッド(return 有り、引数有り)
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

		//コマンド羅列
		unsigned char* commandTable;
		//現在参照してるコマンドの位置
		unsigned char* command_ptr_=nullptr;
		//グローバル変数の確保コマンド
		unsigned char* allocCommand_ptr_=nullptr;
		//プログラム全体のサイズ
		int commandSize;
		//文字列データ
		char* textBuffer;
		//文字列データのサイズ
		int textSize;

		//命令テーブル
		OperationFunction* p_op=nullptr;
		//組み込み関数テーブル
		OperationFunction* p_syscall = nullptr;
		//組み込みメソッドテーブル
		OperationFunction* p_sysMethodCall = nullptr;
		//変数の確保関数テーブル
		OperationFunction* p_pushValues = nullptr;
		//変数(参照型)の確保関数テーブル
		OperationFunction* p_pushRefValues = nullptr;

		std::vector<ScriptClassInfo> vec_scriptClassInfo;

		ButiScript::Stack<ButiScript::Value, STACK_SIZE> valueStack;

		//スタックの参照位置
		int stack_base=0;
		int globalValue_base = 0;
		int globalValue_size=0;

#ifdef IMPL_BUTIENGINE
		std::shared_ptr<ButiEngine::GameObject> shp_gameObject;
#endif
	};

}


#endif
