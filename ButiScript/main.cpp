#include "stdafx.h"
#define _CRTDBG_MAP_ALLOC
class Sample {
public:
	std::int32_t count = 0;
	Sample() {
	}
	Sample(const std::int32_t arg) {
		count = arg;
	}
	~Sample() {
		std::int32_t i = 0;
	}
	std::int32_t TestMethod() {
		std::cout << "Sample::TestMethod() is Called! count:"<<count<<std::endl;
		return count;
	}
	template <typename T>
	T TemplateMethod(const std::string& str) {
		return 1.5f;
	}
};
std::shared_ptr<Sample> CreateSample() {
	return std::make_shared<Sample>();
}

template<typename T> 
T CreateInstance() { 
	return T(); 
}
#include"BuiltInTypeRegister.h"
#include "Compiler.h"


class IDeallocator {
public:
	virtual void Destroy() = 0;
	virtual void Dispose() = 0;
};
template <typename T>
class Deallocator : public IDeallocator {
public:
	using this_type = Deallocator<T>;
	inline void Dispose()override { delete p_value; }
	inline void Destroy()override { delete this; }
	static this_type* get(T* arg_p) { return new this_type(arg_p); }
private:
	this_type& operator=(const this_type&) { return *this; }
	Deallocator(const this_type& arg_rhs) : p_value(arg_rhs.p_value) {}
	Deallocator() = delete;
	Deallocator(T* arg_p) :p_value(arg_p) {}
	T* p_value;
};
template <typename T>
class MemberDeallocator : public IDeallocator {
public:
	using this_type = MemberDeallocator<T>;
	inline void Dispose()override {  }
	inline void Destroy()override { delete this; }
	static this_type* get(T*) { return new this_type(); }
private:
	this_type& operator=(const this_type&) { return *this; }
	MemberDeallocator(const this_type&) = delete;
	MemberDeallocator(){}
};

class DeallocTypeSpecifier {
public:
	template<typename T> class type : public Deallocator<T> {
	public:
		using name = Deallocator<T>;
	};
};
class MemberDeallocTypeSpecifier {
public:
	template<typename T> class type : public MemberDeallocator<T> {
	public:
		using name = MemberDeallocator<T>;
	};
};

class RefferenceCounter {
public:
	constexpr RefferenceCounter()noexcept :p_impl(nullptr) {}
	RefferenceCounter(const RefferenceCounter& arg_r) :p_impl(arg_r.p_impl) { if (p_impl) p_impl->inc(); }
	template<typename S, typename D>
	explicit RefferenceCounter(S* arg_p, D arg_d) :p_impl(new impl(arg_p, arg_d)) {}
	~RefferenceCounter() { release(); }
	void release() {
		if (p_impl && !p_impl->dec()) {
			p_impl->p_deallocator->Dispose();
			delete p_impl;
		}
	}
	void swap(RefferenceCounter& r) { std::swap(p_impl, r.p_impl); }

	inline bool unique()const { return use_count() == 1; }
	std::uint64_t use_count()const { return p_impl ? p_impl->count() : 0; }
	inline RefferenceCounter& operator=(const RefferenceCounter& arg_r) {
		impl* tmp = arg_r.p_impl;
		if (tmp != p_impl) {
			if (tmp) tmp->inc();
			release();
			p_impl = tmp;
		}
		return *this;
	}
private:
	struct impl {
		template<typename S, typename D> explicit impl(S* arg_p, D)
			:use(1), p_deallocator(D::template type<S>::name::get(arg_p)) {}
		impl(const impl& arg_c) :use(arg_c.use), p_deallocator(arg_c.p_deallocator) {}
		inline std::uint64_t inc() { 
			return	_InterlockedIncrement64(reinterpret_cast<volatile std::int64_t*>(&use));
		}
		inline std::uint64_t dec() {			
			return	_InterlockedDecrement64(reinterpret_cast<volatile std::int64_t*>(&use));
		}

		inline std::uint64_t count()const { return use; }
		~impl() { p_deallocator->Destroy(); }
		std::uint64_t use;
		IDeallocator* p_deallocator;
	};
	impl* p_impl;
};

class MemberNotify{};
template<typename T, typename RefferenceObject = RefferenceCounter>
class Value_ptr {
public:
	using this_type=Value_ptr<T, RefferenceObject> ;
	using refCounter_type= RefferenceObject ;
	using value_type=T;
	using element_type =T;
	using pointer=T*;
	using const_pointer=const T*;
	using reference=T&;
	using const_reference=const T&;

	inline constexpr Value_ptr()noexcept :p_value(nullptr), refferenceCounter() {}
	inline constexpr Value_ptr(std::nullptr_t)noexcept :p_value(nullptr), refferenceCounter() {}
	inline Value_ptr(const this_type& arg_s) :p_value(arg_s.p_value), refferenceCounter(arg_s.refferenceCounter) {}
	template<typename S>
	inline Value_ptr(const Value_ptr<S, refCounter_type>& arg_s) : p_value(arg_s.p_value), refferenceCounter(arg_s.refferenceCounter) {}
	template<typename S>
	inline explicit Value_ptr(const S& arg_v) {
		auto p = new S(arg_v);
		p_value = pointer(p);
		refferenceCounter = refCounter_type(p, DeallocTypeSpecifier());
	}
	template<typename S>
	inline explicit Value_ptr(S* arg_p) :p_value(arg_p), refferenceCounter(arg_p, DeallocTypeSpecifier()) {}
	template<typename S>
	inline explicit Value_ptr(S* arg_p,const MemberNotify) :p_value(arg_p), refferenceCounter(arg_p, MemberDeallocTypeSpecifier()) {}
	template<typename S, typename D>
	inline explicit Value_ptr(S* arg_p, D arg_d) :p_value(arg_p), refferenceCounter(arg_p, arg_d) {}

	~Value_ptr() {}


	inline void reset() { this_type().swap(*this); }
	inline void reset(const this_type& arg_s) { this_type(arg_s).swap(*this); }
	template<class S> void reset(S* arg_p) { this_type(arg_p).swap(*this); }
	template<class S, class D>
	inline void reset(S* arg_p, D arg_d) { this_type(arg_p, arg_d).swap(*this); }
	inline void swap(this_type& arg_other) {
		std::swap(p_value, arg_other.p_value);
		refferenceCounter.swap(arg_other.refferenceCounter);
	}

	inline this_type& operator=(const this_type& arg_s) {
		p_value = arg_s.p_value;
		refferenceCounter = arg_s.refferenceCounter;
		return *this;
	}
	template<class S>
	inline this_type& operator=(const Value_ptr<S, refCounter_type>& arg_s) {
		p_value = arg_s.p_value;
		refferenceCounter = arg_s.refferenceCounter;
		return *this;
	}

	inline reference operator*() { return *get(); }
	inline const_reference operator*()const { return *get(); }
	inline pointer operator->() { return get(); }
	inline const_pointer operator->()const { return get(); }

	inline pointer get() { return p_value; }
	inline const_pointer get()const { return p_value; }
	template<typename RetType>
	inline RetType* get() { return reinterpret_cast<RetType*>(p_value); }
	template<typename RetType>
	inline const RetType* get()const { return reinterpret_cast<RetType*>(p_value); }

	inline bool unique()const { return refferenceCounter.unique(); }
	inline std::uint64_t use_count()const { return refferenceCounter.use_count(); }

	inline operator bool()const { return !!p_value; }
	inline bool operator!()const { return !p_value; }
private:
	template<class S, class R> friend class Value_ptr;
	pointer p_value;
	refCounter_type refferenceCounter;
};

template<typename RefferenceObject  >
class Value_ptr<void, RefferenceObject> {
public:
	using this_type = Value_ptr<void, RefferenceObject>;
	using refCounter_type = RefferenceObject;
	using value_type = void;
	using element_type = void;
	using pointer = void*;
	using const_pointer = const void*;

	inline constexpr Value_ptr()noexcept :p_value(nullptr), refferenceCounter() {}
	inline constexpr Value_ptr(std::nullptr_t)noexcept :p_value(nullptr), refferenceCounter() {}
	inline Value_ptr(const this_type& arg_s) :p_value(arg_s.p_value), refferenceCounter(arg_s.refferenceCounter) {}
	template<typename S>
	inline Value_ptr(const Value_ptr<S, refCounter_type>& arg_s) : p_value(arg_s.p_value), refferenceCounter(arg_s.refferenceCounter) {}
	template<typename S>
	inline explicit Value_ptr(const S& arg_v) {
		auto p = new S(arg_v);
		p_value = pointer(p);
		refferenceCounter = refCounter_type(p, DeallocTypeSpecifier());
	}
	template<typename S>
	inline explicit Value_ptr(S* arg_p) :p_value(arg_p), refferenceCounter(arg_p, DeallocTypeSpecifier()) {}
	template<typename S>
	inline explicit Value_ptr(S* arg_p, const MemberNotify) :p_value(arg_p), refferenceCounter(arg_p, MemberDeallocTypeSpecifier()) {}
	template<typename S, typename D>
	inline explicit Value_ptr(S* arg_p, D arg_d) :p_value(arg_p), refferenceCounter(arg_p, arg_d) {}

	~Value_ptr() {}

	// primitive operations
	inline void reset() { this_type().swap(*this); }
	inline void reset(const this_type& arg_s) { this_type(arg_s).swap(*this); }
	template<class S> 
	inline void reset(S* arg_p){ this_type(arg_p).swap(*this); }
	template<class S, class D>
	inline void reset(S* arg_p, D arg_d) { this_type(arg_p, arg_d).swap(*this); }
	inline void swap(this_type& arg_other) {
		std::swap(p_value, arg_other.p_value);
		refferenceCounter.swap(arg_other.refferenceCounter);
	}

	inline this_type& operator=(const this_type& arg_s) {
		p_value = arg_s.p_value;
		refferenceCounter = arg_s.refferenceCounter;
		return *this;
	}
	template<class S>
	inline this_type& operator=(const Value_ptr<S, refCounter_type>& arg_s) {
		p_value = arg_s.p_value;
		refferenceCounter = arg_s.refferenceCounter;
		return *this;
	}

	inline pointer operator->() { return get(); }
	inline const_pointer operator->()const { return get(); }
	inline pointer get() { return p_value; }
	inline const_pointer get()const { return p_value; }
	template<typename RetType>
	inline RetType* get() { return reinterpret_cast<RetType*>( p_value); }
	template<typename RetType>
	inline const RetType* get()const { return reinterpret_cast<RetType*>(p_value); }
	inline bool unique()const { return refferenceCounter.unique(); }
	inline std::uint64_t use_count()const { return refferenceCounter.use_count(); }

	inline operator bool()const { return !!p_value; }
	inline bool operator!()const { return !p_value; }
private:
	template<class S, class R> friend class Value_ptr;
	pointer p_value;
	refCounter_type refferenceCounter;
};

template<class T, class U, class R>
inline bool operator==(const Value_ptr<T, R>& arg_lhs, const Value_ptr<U, R>& arg_rhs) {
	return arg_lhs.get() == arg_rhs.get();
}
template<class T, class U, class R>
inline bool operator!=(const Value_ptr<T, R>& arg_lhs, const Value_ptr<U, R>& arg_rhs) {
	return arg_lhs.get() != arg_rhs.get();
}

template<typename T ,typename... Args>
inline constexpr Value_ptr<T> make_value(Args&&... args) {
	return Value_ptr<T>(new T(args...));
}

std::int32_t main(const std::int32_t argCount, const char* args[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);


	Value_ptr<void> p = make_value<Sample>(24);
	Value_ptr<void> member = Value_ptr<int>(& (p.get<Sample>())-> count, MemberNotify());

	{
		auto p1 = p;
		((Sample*)p.get())->count += 12;
	}
	std::cout <<* (member.get<int>()) << std::endl;

	auto p_s = make_value<std::string>("AAAAAAAAAa");
	std::cout << p_s->size() << std::endl;

	//ButiScript::Compiler driver;
	//ButiScript::SystemTypeRegister::GetInstance()->RegistSharedSystemType<Sample>("Sample", "Sample");
	//ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method_ret< Sample, std::int32_t, &Sample::TestMethod, &ButiScript::VirtualMachine::GetSharedTypePtr  >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), TYPE_INTEGER, "TestMethod", "");
	//ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method_ret< Sample,std::int32_t,const std::string&, &Sample::TemplateMethod<std::int32_t>, &ButiScript::VirtualMachine::GetSharedTypePtr ,&ButiScript::VirtualMachine::GetTypePtr >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), TYPE_INTEGER, "TemplateMethod", "s", { TYPE_INTEGER });
	//ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemMethod(&ButiScript::VirtualMachine::sys_method_ret< Sample,float, const std::string&, &Sample::TemplateMethod<float>, &ButiScript::VirtualMachine::GetSharedTypePtr, &ButiScript::VirtualMachine::GetTypePtr>, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), TYPE_FLOAT, "TemplateMethod", "s", {TYPE_FLOAT});
	//ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func_ret<std::shared_ptr<Sample>, &CreateSample >, ButiScript::SystemTypeRegister::GetInstance()->GetIndex("Sample"), "CreateSample", "");
	//ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func_ret<std::int32_t, &CreateInstance<std::int32_t>>, TYPE_INTEGER, "CreateInstance", "", { TYPE_INTEGER });
	//ButiScript::SystemFuntionRegister::GetInstance()->DefineSystemFunction(&ButiScript::VirtualMachine::sys_func_ret<float, &CreateInstance<float>>, TYPE_FLOAT, "CreateInstance", "", {TYPE_FLOAT});

	//driver.RegistDefaultSystems(); 
	//bool compile_result=false;
	//for(std::int32_t i=1;i<argCount;i++)
	//{
	//	std::shared_ptr< ButiScript::CompiledData> data = std::make_shared<ButiScript::CompiledData>();
	//	compile_result = driver.Compile(args[i], *data);
	//	if (!compile_result) {
	//		std::cout << args[i]<<"のコンパイル成功" << std::endl;
	//		driver.OutputCompiledData(StringHelper::GetDirectory(args[i])+"/output/"+ StringHelper::GetFileName(args[i],false)+ ".cbs", *data);
	//	}
	//	else {
	//		std::cout << args[i] << "のコンパイル失敗" << std::endl;
	//	}
	//	data = std::make_shared<ButiScript::CompiledData>();
	//	auto res= driver.InputCompiledData(StringHelper::GetDirectory(args[i]) + "/output/" + StringHelper::GetFileName(args[i], false) + ".cbs", *data);
	//	if (!res) {
	//		ButiScript::VirtualMachine* p_clone; 
	//		std::int32_t returnCode=0;
	//		{

	//			ButiScript::VirtualMachine machine(data);

	//			machine.Initialize();
	//			machine.AllocGlobalValue();
	//			//machine.SetGlobalVariable(1, "g_i");
	//			//machine.SetGlobalVariable(2, "g_i1");

	//			std::cout << args[i] << "のmain実行" << std::endl;
	//			std::cout << "////////////////////////////////////" << std::endl;
	//			returnCode= machine.Execute<std::int32_t>("main");
	//			std::cout << "////////////////////////////////////" << std::endl;
	//			std::cout << args[i] << "のreturn : " << std::to_string(returnCode) << std::endl;

	//			//p_clone = machine.Clone();
	//		}

	//	}
	//}

	std::system("pause");



	return 0;
}