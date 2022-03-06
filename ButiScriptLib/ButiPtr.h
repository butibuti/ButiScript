
#ifndef BUTIPTR_H
#define BUTIPTR_H
namespace std {
template <typename T>
inline std::string to_string(const T&)noexcept {
	return "このクラスはto_string()に対応していません";
}
#ifdef __vm_value_h__
template<typename T>
inline std::string to_string(const ButiScript::Type_hasMember<T>& arg_v);
#endif // __vm_value_h__

}
namespace ButiEngine {

namespace SmartPtrDetail {
template <typename T>
T* CloneValue(const T* p_v) {
	return new T(*p_v);
}

class MemberPtrNotify {};
static MemberPtrNotify MemberNotify;
namespace TypeStorage {
template <typename T>
class TypeStorage;
template <typename T>
class MemberTypeStorage;
/// <summary>
/// インスタンスが生成された際の型を記憶し、適切なデストラクタ呼び出し、コピー、String変換を行う
/// </summary>
class ITypeStorage {
public:
	virtual void Destroy() = 0;
	virtual void Dispose() = 0;
	virtual ITypeStorage* Clone() const= 0;
	virtual ITypeStorage* CreateSameTypeStorage(void * arg_src)const = 0;
	virtual std::string ToString() const= 0;
	virtual void* ptr() = 0;
	virtual const void* ptr() const = 0;
	template <typename OwnType,typename CastType>
	inline ITypeStorage* StaticCast()const;
	template<std::int32_t size>
	inline void Write(const void* arg_src) {
		memcpy_s(ptr(), size, arg_src, size);
	}
	virtual void Write(const void* arg_src) = 0;
};
template <typename T>
class TypeStorage : public ITypeStorage {
public:
	using this_type = TypeStorage<T>;
	TypeStorage(T* arg_p) :p_value(arg_p) {}
	inline void Dispose()override { delete p_value; }
	inline void Destroy()override { delete this; }
	//値のクローン作成(今後メモリ確保は変更)
	inline ITypeStorage* Clone()const override { 
		return new this_type(CloneValue(p_value)); 
	}
	//同じ型のTypeStorageを作成する
	inline ITypeStorage* CreateSameTypeStorage(void* arg_src)const override { return new this_type(reinterpret_cast<T*>(arg_src)); }
	inline std::string ToString()const override { return std::to_string(*p_value); }
	inline  void* ptr()override { return p_value; }
	inline const void* ptr()const override { return p_value; }
	inline void Write(const void* arg_src)override {
		*p_value = *reinterpret_cast<const T*>(arg_src);
	}
	static this_type* get(T* arg_p) { return new this_type(arg_p); }
private:
	template<typename T> friend class MemberTypeStorage;
	this_type& operator=(const this_type&) { return *this; }
	TypeStorage(const this_type& arg_rhs) : p_value(arg_rhs.p_value) {}
	TypeStorage() = delete;
	T* p_value;
}; 
template <>
class TypeStorage<std::string> : public ITypeStorage {
public:
	using this_type = TypeStorage<std::string>;
	TypeStorage(std::string* arg_p) :p_value(arg_p) {}
	inline void Dispose()override { delete p_value; }
	inline void Destroy()override { delete this; }
	//値のクローン作成(今後メモリ確保は変更)
	inline ITypeStorage* Clone()const override { return new this_type(CloneValue(p_value)); }
	//同じ型のTypeStorageを作成する
	inline ITypeStorage* CreateSameTypeStorage(void* arg_src)const override { return new this_type(reinterpret_cast<std::string*>( arg_src)); }
	inline std::string ToString()const override { return *p_value; }
	inline  void* ptr()override { return p_value; }
	inline const void* ptr()const override { return p_value; }
	static this_type* get(std::string* arg_p) { return new this_type(arg_p); }
	inline void Write(const void* arg_src)override {
		*p_value = *reinterpret_cast<const std::string*>(arg_src);
	}
private:
	template<typename T> friend class MemberTypeStorage;
	this_type& operator=(const this_type&) { return *this; }
	TypeStorage(const this_type& arg_rhs) : p_value(arg_rhs.p_value) {}
	TypeStorage() = delete;
	std::string* p_value;
};
template <typename T>
class MemberTypeStorage : public ITypeStorage {
public:
	using this_type = MemberTypeStorage<T>;
	inline void Dispose()override {  }
	inline void Destroy()override { delete this; }
	//値のクローン作成(今後メモリ確保は変更)
	inline ITypeStorage* Clone()const override { 
		return new TypeStorage<T>(CloneValue(p_value));
	}
	//同じ型のTypeStorageを作成する
	inline ITypeStorage* CreateSameTypeStorage(void* arg_src)const override { return new this_type(reinterpret_cast<T*>(arg_src)); }
	inline  void* ptr()override { return p_value; }
	inline const void* ptr()const override { return p_value; }
	inline std::string ToString()const override { return std::to_string(*p_value); }
	static this_type* get(T* arg_p) { return new this_type(arg_p); }
	inline void Write(const void* arg_src)override {
		*p_value = *reinterpret_cast<const T*>(arg_src);
	}
private:
	this_type& operator=(const this_type&) { return *this; }
	MemberTypeStorage(const this_type&) = delete;
	MemberTypeStorage() = delete;
	MemberTypeStorage(T* arg_p) :p_value(arg_p) {}
	T* p_value;
};
template <>
class MemberTypeStorage<std::string> : public ITypeStorage {
public:
	using this_type = MemberTypeStorage<std::string>;
	inline void Dispose()override {  }
	inline void Destroy()override { delete this; }
	//値のクローン作成(今後メモリ確保は変更)
	inline ITypeStorage* Clone()const override { return new TypeStorage<std::string>(CloneValue(p_value)); }
	//同じ型のTypeStorageを作成する
	inline ITypeStorage* CreateSameTypeStorage(void* arg_src)const override { return new this_type(reinterpret_cast<std::string*>(arg_src)); }
	inline  void* ptr()override { return p_value; }
	inline const void* ptr()const override { return p_value; }
	inline std::string ToString()const override { return *p_value; }
	static this_type* get(std::string* arg_p) { return new this_type(arg_p); }
	inline void Write(const void* arg_src)override {
		*p_value = *reinterpret_cast<const std::string*>(arg_src);
	}
private:
	this_type& operator=(const this_type&) { return *this; }
	MemberTypeStorage(const this_type&) = delete;
	MemberTypeStorage() = delete;
	MemberTypeStorage(std::string* arg_p) :p_value(arg_p) {}
	std::string* p_value;
};
template<typename OwnType, typename CastType>
inline ITypeStorage* ITypeStorage::StaticCast() const
{
	return new TypeStorage(new CastType( static_cast<CastType>(* reinterpret_cast<const OwnType*>(ptr()))));
}

class TypeStorageSpecifier {
public:
	template<typename T> class type : public TypeStorage<T> {
	public:
		using name = TypeStorage<T>;
	};
};
class MemberTypeStorageSpecifier {
public:
	template<typename T> class type : public MemberTypeStorage<T> {
	public:
		using name = MemberTypeStorage<T>;
	};
};
}
namespace RefferenceCounter {
class RefferenceCounter {
	struct impl;
public:
	constexpr RefferenceCounter()noexcept :p_impl(nullptr) {}
	constexpr RefferenceCounter(impl* arg_p_impl) :p_impl(arg_p_impl) {}
	RefferenceCounter(RefferenceCounter* arg_r) : p_impl(arg_r->p_impl) {}
	RefferenceCounter(const RefferenceCounter& arg_r) :p_impl(arg_r.p_impl) { if (p_impl) p_impl->inc(); }
	template<typename S, typename D>
	explicit RefferenceCounter(S* arg_p, D arg_d) :p_impl(new impl(arg_p, arg_d)) {}
	template<typename S, typename D>
	explicit RefferenceCounter(S* arg_p, D* arg_p_d) :p_impl(new impl(arg_p, arg_p_d)) {}
	~RefferenceCounter() { release(); }
	inline void release() {
		if (p_impl && !p_impl->dec()) {
			p_impl->p_typeStorage->Dispose();
			delete p_impl;
		}
	}
	inline void Write(const void* arg_src) {
		p_impl->p_typeStorage->Write(arg_src);
	}
	inline void swap(RefferenceCounter& r) { std::swap(p_impl, r.p_impl); }
	inline TypeStorage::ITypeStorage* GetTypeStorage() { return p_impl->p_typeStorage; }
	inline const TypeStorage::ITypeStorage* GetTypeStorage()const { return p_impl->p_typeStorage; }
	inline bool unique()const { return use_count() == 1; }
	inline std::uint64_t use_count()const { return p_impl ? p_impl->count() : 0; }
	inline RefferenceCounter& operator=(const RefferenceCounter& arg_r) {
		impl* tmp = arg_r.p_impl;
		if (tmp != p_impl) {
			if (tmp) tmp->inc();
			release();
			p_impl = tmp;
		}
		return *this;
	}
	inline RefferenceCounter CreateSameType(void* arg_p_src)const {
		return RefferenceCounter(p_impl->CreateSameTypeStorage(arg_p_src));
	}
private:
	struct impl {
		template<typename S, typename D> explicit impl(S* arg_p, D)
			:use(1), p_typeStorage(D::template type<S>::name::get(arg_p)) {}
		template<typename S, typename D> explicit impl(S* arg_p, D* arg_p_typeStorage)
			:use(1), p_typeStorage(arg_p_typeStorage) {}
		impl(TypeStorage::ITypeStorage* arg_p_typeStorage):use(1),p_typeStorage(arg_p_typeStorage){}
		impl(const impl& arg_c) :use(arg_c.use), p_typeStorage(arg_c.p_typeStorage) {}
		inline std::uint64_t inc() {
			return	_InterlockedIncrement64(reinterpret_cast<volatile std::int64_t*>(&use));
		}
		inline std::uint64_t dec() {
			return	_InterlockedDecrement64(reinterpret_cast<volatile std::int64_t*>(&use));
		}

		inline std::uint64_t count()const { return use; }
		~impl() { p_typeStorage->Destroy(); }
		impl* CreateSameTypeStorage(void* arg_p_src)const {
			return new impl(p_typeStorage->CreateSameTypeStorage(arg_p_src));
		}
		std::uint64_t use;
		TypeStorage::ITypeStorage* p_typeStorage;
	};
	impl* p_impl;
};
}

}

template<typename T, typename RefferenceObject = SmartPtrDetail::RefferenceCounter::RefferenceCounter>
class Value_ptr {
	friend class Valu_ptrHelper;
public:
	using this_type = Value_ptr<T, RefferenceObject>;
	using refCounter_type = RefferenceObject;
	using value_type = T;
	using element_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;

	inline constexpr Value_ptr()noexcept :p_value(nullptr), refferenceCounter() {}
	inline constexpr Value_ptr(std::nullptr_t)noexcept :p_value(nullptr), refferenceCounter() {}
	inline Value_ptr(const this_type& arg_s) : p_value(arg_s.p_value), refferenceCounter(arg_s.refferenceCounter) {}
	template<typename S>
	inline Value_ptr(const Value_ptr<S, refCounter_type>& arg_s) : p_value(arg_s.p_value), refferenceCounter(arg_s.refferenceCounter) {}
	template<typename S>
	inline explicit Value_ptr(const S& arg_v) {
		auto p = new S(arg_v);
		p_value = pointer(p);
		refferenceCounter = refCounter_type(p, SmartPtrDetail::TypeStorage::TypeStorageSpecifier());
	}
	inline Value_ptr(T* arg_p, RefferenceObject arg_refObj) :p_value(arg_p), refferenceCounter(arg_refObj) {}
	template<typename S>
	inline explicit Value_ptr(S* arg_p) :p_value(arg_p), refferenceCounter(arg_p, SmartPtrDetail::TypeStorage::TypeStorageSpecifier()) {}
	template<typename S>
	inline explicit Value_ptr(S* arg_p, const SmartPtrDetail::MemberPtrNotify) :p_value(arg_p), refferenceCounter(arg_p, SmartPtrDetail::TypeStorage::MemberTypeStorageSpecifier()) {}
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
	inline std::string ToString() const{
		return refferenceCounter.refferenceCounter.GetTypeStorage()->ToString();
	}
	Value_ptr<T> Clone()const {
		auto typeStorage = refferenceCounter.GetTypeStorage()->Clone();
		return Value_ptr<T>(reinterpret_cast<pointer>(typeStorage->ptr()), typeStorage);
	}
	/// <summary>
	/// データの直接書き込み
	/// </summary>
	/// <param name="arg_src">書き込み元のポインタ、型が同一と信頼</param>
	inline void Write(const void* arg_src) {
		refferenceCounter.Write(arg_src);
	}
	template<typename CastType>
	inline Value_ptr<CastType> StaticCast()const {
		auto typeStorage = refferenceCounter.GetTypeStorage()->StaticCast<T, CastType>();
		return Value_ptr<CastType>(reinterpret_cast<CastType*>(typeStorage->ptr()), typeStorage);
	}
	template<typename OwnType, typename CastType>
	inline Value_ptr<CastType> StaticCast()const {
		auto typeStorage = refferenceCounter.GetTypeStorage()->StaticCast<OwnType, CastType>();
		return Value_ptr<CastType>(reinterpret_cast<CastType*>(typeStorage->ptr()), typeStorage);
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
	template<typename S, typename R> friend class Value_ptr;
	pointer p_value;
	refCounter_type refferenceCounter;
};

template<typename RefferenceObject  >
class Value_ptr<void, RefferenceObject> {
	friend class Valu_ptrHelper;
public:
	using this_type = Value_ptr<void, RefferenceObject>;
	using refCounter_type = RefferenceObject;
	using value_type = void;
	using element_type = void;
	using pointer = void*;
	using const_pointer = const void*;

	inline constexpr Value_ptr()noexcept :p_value(nullptr), refferenceCounter() {}
	inline constexpr Value_ptr(std::nullptr_t)noexcept :p_value(nullptr), refferenceCounter() {}
	inline Value_ptr(const this_type& arg_s) : p_value(arg_s.p_value), refferenceCounter(arg_s.refferenceCounter) {}
	template<typename S>
	inline Value_ptr(const Value_ptr<S, refCounter_type>& arg_s) : p_value(arg_s.p_value), refferenceCounter(arg_s.refferenceCounter) {}
	template<typename S>
	inline explicit Value_ptr(const S& arg_v) {
		auto p = new S(arg_v);
		p_value = pointer(p);
		refferenceCounter = refCounter_type(p, SmartPtrDetail::TypeStorage::TypeStorageSpecifier());
	}
	template<typename S>
	inline explicit Value_ptr(S* arg_p) :p_value(arg_p), refferenceCounter(arg_p, SmartPtrDetail::TypeStorage::TypeStorageSpecifier()) {}
	template<typename S>
	inline explicit Value_ptr(S* arg_p, const SmartPtrDetail::MemberPtrNotify) :p_value(arg_p), refferenceCounter(arg_p, SmartPtrDetail::TypeStorage::MemberTypeStorageSpecifier()) {}
	inline Value_ptr(void* arg_p,RefferenceObject arg_refObj) :p_value(arg_p), refferenceCounter(arg_refObj) {}
	template<typename S, typename D>
	inline explicit Value_ptr(S* arg_p, D arg_d) :p_value(arg_p), refferenceCounter(arg_p, arg_d) {}

	~Value_ptr() {}


	inline void reset() { this_type().swap(*this); }
	inline void reset(const this_type& arg_s) { this_type(arg_s).swap(*this); }
	template<class S>
	inline void reset(S* arg_p) { this_type(arg_p).swap(*this); }
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

	inline std::string ToString()const {
		return refferenceCounter.GetTypeStorage()->ToString();
	}

	template<typename OwnType, typename CastType>
	inline Value_ptr<CastType> StaticCast()const {
		auto typeStorage = refferenceCounter.GetTypeStorage()->StaticCast<OwnType, CastType>();
		return Value_ptr<CastType>(reinterpret_cast<CastType*>(typeStorage->ptr()), typeStorage);
	}
	Value_ptr<void> Clone() const{
		auto typeStorage = refferenceCounter.GetTypeStorage()->Clone();
		return Value_ptr<void>(reinterpret_cast<pointer>(typeStorage->ptr()), typeStorage);
	}
	/// <summary>
	/// データの直接書き込み
	/// </summary>
	/// <param name="arg_src">書き込み元のポインタ、型が同一と信頼</param>
	inline void Write(const void* arg_src) {
		refferenceCounter.Write(arg_src);
	}
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
	template<typename S, typename R> friend class Value_ptr;
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

template<typename T, typename... Args>
inline constexpr Value_ptr<T> make_value(Args&&... args) {
	return Value_ptr<T>(new T(args...));
}
template <typename T>
inline Value_ptr<T> to_value(T* v) {
	return Value_ptr<T>(v, SmartPtrDetail::MemberNotify);
}
class Valu_ptrHelper {
public:
	template <typename T, typename RefferenceObject>
	static inline Value_ptr<T, RefferenceObject> CreateSameTypeValuePtr(Value_ptr<T, RefferenceObject> arg_baseValuePtr, void* arg_p_src) {
		return  Value_ptr<T,RefferenceObject>(arg_p_src, arg_baseValuePtr.refferenceCounter.CreateSameType(arg_p_src));
	}
};
}

#endif // !BUTIPTR_H
