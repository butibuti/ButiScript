#pragma once
#ifndef	__vm_value_h__
#define	__vm_value_h__
#include"value_type.h"
#include <iostream>
#include <exception>
#include <string>
#include"Tags.h"
#include"../../Header/Common/ButiMath.h"

////////////////////////////////
////テンプレート継承の実装用マクロ////
////////////////////////////////
#define RegistGet(T) \
virtual T operator()(const IValue* b, const T * p_dummy) const {\
return static_cast<const Class*>(b)->get_value_stub< T >();\
}\

#define RegistGetRef(T) \
virtual T& operator()(IValue* b, const T * p_dummy) const {\
return static_cast<Class*>(b)->get_ref_stub< T >();\
}\

#define RegistSet(T) \
virtual void operator()(IValue* b,const T v) const {\
static_cast<Class*>(b)->set_value_stub<T>(v);\
}\

#define RegistSetRef(T) \
virtual void operator()(IValue* b,const T & v) const {\
static_cast<Class*>(b)->set_value_stub<T>(v);\
}\

#define RegistEq(T) \
virtual bool operator()(const IValue* b,const T & v) const {\
return static_cast<const Class*>(b)->eq_stub<T>(v);\
}\

#define RegistGt(T) \
virtual bool operator()(const IValue* b,const T & v) const {\
 return static_cast<const Class*>(b)->gt_stub<T>(v);\
}\

#define RegistGe(T) \
virtual bool operator()(const IValue* b,const T & v) const {\
return static_cast<const Class*>(b)->ge_stub<T>(v);\
}\


//////////////////////////////////
////変数の中身の明示的特殊化用マクロ////
//////////////////////////////////
#define RegistSpecialization(Type)\
bool Eq(IValue* p_other)const override {\
	return p_other->Equal(*( Type *)p_instance);\
}\
bool Gt(IValue* p_other)const override {\
	return false;\
}\
bool Ge(IValue* p_other)const override {\
	return false;\
}\
void ValueCopy(IValue* p_other) const override {\
	p_other->Set< Type >(*( Type *)p_instance);\
}\
IValue* Clone()const {\
	return new Value_wrap< Type >(*( Type *)p_instance, 1);\
}\
void Nagative()override {\
	*( Type *)p_instance = -1 * (*( Type *)p_instance);\
}\
virtual const IValue::get_value_base_t& get_value() const {\
	static const IValue::get_value_t<Value_wrap< Type >> s;\
	return s;\
}\
virtual const IValue::get_ref_base_t& get_ref() const {\
	static const IValue::get_ref_t<Value_wrap< Type >> s;\
	return s;\
}\
virtual const IValue::set_value_base_t& set_value() const {\
	static const IValue::set_value_t<Value_wrap< Type >> s;\
	return s;\
}\
virtual const IValue::eq_base_t& eq() const {\
	static const IValue::eq_t<Value_wrap< Type >> s;\
	return s;\
}\
virtual const IValue::gt_base_t& gt() const {\
	static const IValue::gt_t<Value_wrap< Type >> s;\
	return s;\
}\
virtual const IValue::ge_base_t& ge() const {\
	static const IValue::ge_t<Value_wrap< Type >> s;\
	return s;\
}\
template <typename U> U get_value_stub()const {\
	return U();\
}\
template <typename U> U& get_ref_stub() {\
	auto v = U();\
	return v;\
}\
template <>  Type & get_ref_stub() {\
	return *( Type *)p_instance;\
}\
template <>  Type  get_value_stub()const {\
	return (*( Type *)p_instance);\
}\
template <> std::string get_value_stub()const {\
	return std::to_string(*( Type *)p_instance);\
}\
template <typename U> void set_value_stub(const U& arg_v) {\
	*(std::string*)p_instance = std::to_string(arg_v);\
}\
template <>void set_value_stub(const  Type & arg_v) {\
	*( Type *)p_instance = (arg_v);\
}\
template <>void set_value_stub(const std::string& arg_v) {\
	*( Type *)p_instance = StrConvert::ConvertString< Type >(arg_v);\
}\
template <typename U> bool eq_stub(const U& arg_v)const {\
	return  false;\
}\
template <>bool eq_stub(const  Type & arg_v)const {\
	return *( Type *)p_instance == arg_v;\
}\


namespace ButiScript {

class IValue {
public:
	IValue(const int arg_ref) {
		ref_ = arg_ref;
	}

	virtual ~IValue() {
	}

	template <typename T> T Get()const {
		static T* p_dummy = nullptr;
		return get_value()(this, p_dummy);
	}
	template <typename T> T& GetRef() {
		static T* p_dummy = nullptr;
		return get_ref()(this, p_dummy);
	}
	template <typename T> void Set(const T& arg_v) {
		return set_value()(this, arg_v);
	}
	template <> void Set(const IValue& arg_v) {
		arg_v.ValueCopy(this);
	}


	IValue* GetMember(const int index)const {
		return ary_p_member[index];
	}
	int GetMemberType(const int index)const {
		return ary_memberType[index];
	}

	void SetMember(IValue* arg_v,const int index) {
		ary_p_member[index] = arg_v;
	}

	template <typename T> bool Equal(const T& arg_v)const {
		return eq()(this, arg_v);
	}
	template <typename T> bool GreaterThan(const T& arg_v)const {
		return gt()(this, arg_v);
	}
	template <typename T> bool GreaterEq(const T& arg_v)const {
		return ge()(this, arg_v);
	}

	//比較
	virtual bool Eq(IValue* p_other)const = 0;
	virtual bool Gt(IValue* p_other)const = 0;
	virtual bool Ge(IValue* p_other)const = 0;

	//値のコピー
	virtual void ValueCopy(IValue* p_other) const = 0;

	//変数そのもののコピー
	virtual IValue* Clone()const = 0;

	//単項マイナス
	virtual void Nagative() = 0;

	//
	void addref()
	{
		ref_++;
	}
	//
	void release()
	{
		if (--ref_ == 0)
			delete this;
	}

protected:
	template <typename Class, typename Super = IValue>
	struct get_value_t : public Super::get_value_base_t {
			virtual ~get_value_t() {}
			RegistGet(int);
			RegistGet(char);
			RegistGet(short);
			RegistGet(long long);
			RegistGet(float);
			RegistGet(double);
			RegistGet(std::string);
			RegistGet(ButiEngine::Vector2);
			RegistGet(ButiEngine::Vector3);
			RegistGet(ButiEngine::Vector4);
		};
	template <typename Class, typename Super = IValue>
	struct get_ref_t : public Super::get_ref_base_t {
			virtual ~get_ref_t() {}
			RegistGetRef(int);
			RegistGetRef(char);
			RegistGetRef(short);
			RegistGetRef(long long);
			RegistGetRef(float);
			RegistGetRef(double);
			RegistGetRef(std::string);
			RegistGetRef(ButiEngine::Vector2);
			RegistGetRef(ButiEngine::Vector3);
			RegistGetRef(ButiEngine::Vector4);
		};
	template <typename Class, typename Super = IValue>
	struct set_value_t : public Super::set_value_base_t {
			virtual ~set_value_t() {}
			RegistSet(int);
			RegistSet(char);
			RegistSet(short);
			RegistSet(long long);
			RegistSet(float);
			RegistSet(double);
			RegistSet(ButiEngine::Vector2);
			RegistSet(ButiEngine::Vector3);
			RegistSet(ButiEngine::Vector4);
			RegistSetRef(std::string);

		};
	template <typename Class, typename Super = IValue>
	struct eq_t : public Super::eq_base_t {
			virtual ~eq_t() {}
			RegistEq(int);
			RegistEq(char);
			RegistEq(short);
			RegistEq(long long);
			RegistEq(float);
			RegistEq(double);
			RegistEq(ButiEngine::Vector2);
			RegistEq(ButiEngine::Vector3);
			RegistEq(ButiEngine::Vector4);
			RegistEq(std::string);

		};
	template <typename Class, typename Super = IValue>
	struct gt_t : public Super::gt_base_t {
			virtual ~gt_t() {}
			RegistGt(int);
			RegistGt(char);
			RegistGt(short);
			RegistGt(long long);
			RegistGt(float);
			RegistGt(double);
			RegistGt(ButiEngine::Vector2);
			RegistGt(ButiEngine::Vector3);
			RegistGt(ButiEngine::Vector4);
			RegistGt(std::string);
		};
	template <typename Class, typename Super = IValue>
	struct ge_t : public Super::ge_base_t {
			virtual ~ge_t() {}
			RegistGe(int);
			RegistGe(char);
			RegistGe(short);
			RegistGe(long long);
			RegistGe(float);
			RegistGe(double);
			RegistGe(ButiEngine::Vector2);
			RegistGe(ButiEngine::Vector3);
			RegistGe(ButiEngine::Vector4);
			RegistGe(std::string);

		};


	struct empty_class { struct get_value_base_t {}; struct get_ref_base_t {}; struct set_value_base_t {}; struct eq_base_t {}; struct gt_base_t {}; struct ge_base_t {}; };
	typedef get_value_t<const IValue, empty_class> get_value_base_t;
	typedef get_ref_t<IValue, empty_class> get_ref_base_t;
	typedef set_value_t<IValue, empty_class> set_value_base_t;
	typedef eq_t<const IValue, empty_class> eq_base_t;
	typedef gt_t<const IValue, empty_class> gt_base_t;
	typedef ge_t<const IValue, empty_class> ge_base_t;
	virtual const get_value_base_t& get_value() const = 0;
	virtual const get_ref_base_t& get_ref() const = 0;
	virtual const set_value_base_t& set_value() const = 0;
	virtual const eq_base_t& eq() const = 0;
	virtual const gt_base_t& gt() const = 0;
	virtual const ge_base_t& ge() const = 0;


	template <typename T> T get_value_stub()const {
		assert(0);
		return T();
	}
	template <typename T> T& get_ref_stub() {
		assert(0);
		auto ret = T();
		return ret;
	}
	template <typename T> void set_value_stub(const T& arg_v) {
		assert(0);
	}
	template <typename T> bool eq_stub(const T& arg_v)const {
		return false;
	}
	template <typename T> bool gt_stub(const T& arg_v)const {
		return false;
	}
	template <typename T> bool ge_stub(const T& arg_v)const {
		return false;
	}
	//実体
	void* p_instance=nullptr;
	//メンバ変数
	IValue** ary_p_member=nullptr;
	//メンバ変数の型
	int* ary_memberType=nullptr;
	bool isOuterMemoryRef = false;
private:
	int ref_ = 0;
};


template <typename T>
class Value_wrap : public IValue {
	public:
		Value_wrap(const T v, const int ref) :IValue(ref) {
			p_instance = new T(v);
		}
		Value_wrap(T* v, const int ref) :IValue(ref) {
			p_instance = v;
			isOuterMemoryRef = true;
		}
		Value_wrap(const int ref) :IValue(ref) {
			p_instance = new T();
		}
		~Value_wrap() {
			if (!isOuterMemoryRef) {
				delete ((T*)p_instance);
			}
			if (ary_memberType) {
				delete ary_memberType;
			}
		}
		IValue* Clone()const override {
			return new Value_wrap<T>(*(T*)p_instance, 1);
		}

		void Nagative() override {
			*(T*)p_instance = -1 * (*(T*)p_instance);
		}

		bool Eq(IValue* p_other)const override {
			return p_other->Equal(*(T*)p_instance);
		}
		bool Gt(IValue* p_other)const override {
			return !p_other->GreaterEq(*(T*)p_instance);
		}
		bool Ge(IValue* p_other)const override {
			return !p_other->GreaterThan(*(T*)p_instance);
		}
		void ValueCopy(IValue* p_other) const override {
			p_other->Set<T>(*(T*)p_instance);
		}

		virtual const IValue::get_value_base_t& get_value() const {
			static const IValue::get_value_t<Value_wrap<T>> s;
			return s;
		}
		virtual const IValue::get_ref_base_t& get_ref() const {
			static const IValue::get_ref_t<Value_wrap<T>> s;
			return s;
		}
		virtual const IValue::set_value_base_t& set_value() const {
			static const IValue::set_value_t<Value_wrap<T>> s;
			return s;
		}
		virtual const IValue::eq_base_t& eq() const {
			static const IValue::eq_t<Value_wrap<T>> s;
			return s;
		}
		virtual const IValue::gt_base_t& gt() const {
			static const IValue::gt_t<Value_wrap<T>> s;
			return s;
		}
		virtual const IValue::ge_base_t& ge() const {
			static const IValue::ge_t<Value_wrap<T>> s;
			return s;
		}

		template <typename U> U get_value_stub()const {
			return (U) * (T*)p_instance;
		}
		template <typename U> U& get_ref_stub() {
			auto v = U();
			return v;
		}
		template <typename U> void set_value_stub(const U& arg_v) {
			*(T*)p_instance = (T)arg_v;
		}

		// ==演算、異なる型との比較は常にfalse
		template <typename U> bool eq_stub(const U& arg_v)const {
			return false;
		}
		// ==演算
		template <>bool eq_stub(const T& arg_v)const {
			return  *(T*)p_instance == arg_v;
		}



		template <> T& get_ref_stub() {
			return *(T*)p_instance;
		}

		template <> std::string get_value_stub()const {
			return std::to_string(*(T*)p_instance);
		}


	private:
	};

template <>
class Value_wrap<ScriptClassInfo> :public IValue {
public:
	Value_wrap(ScriptClassInfo* v,std::vector<IValue*> vec_member, const int ref) :IValue(ref) {
		p_instance = v; 
		int memberSize = vec_member.size();
		ary_p_member = (IValue**)malloc(sizeof(IValue*) * memberSize);
		ary_memberType = (int*)malloc(sizeof(int) * memberSize);
		for (int i = 0; i < memberSize; i++) {
			ary_p_member[i] = vec_member[i];
			ary_memberType[i] = v->GetMemberTypeIndex(i);
		}

	}
	~Value_wrap() override {
		if (ary_memberType) {
			delete ary_memberType;
		}
	}


	bool Eq(IValue* p_other)const override {
		return false;
	}
	bool Gt(IValue* p_other)const override {
		return false;
	}
	bool Ge(IValue* p_other)const override {
		return false;
	}

	void ValueCopy(IValue* p_other) const override {

	}

	IValue* Clone()const {
		return nullptr;
	}

	void Nagative()override {
		//単項マイナスは未定義
		assert(0);

	}


	virtual const IValue::get_value_base_t& get_value() const {
		static const IValue::get_value_t<Value_wrap<ScriptClassInfo>> s;
		return s;
	}
	virtual const IValue::get_ref_base_t& get_ref() const {
		static const IValue::get_ref_t<Value_wrap<ScriptClassInfo>> s;
		return s;
	}
	virtual const IValue::set_value_base_t& set_value() const {
		static const IValue::set_value_t<Value_wrap<ScriptClassInfo>> s;
		return s;
	}

	virtual const IValue::eq_base_t& eq() const {
		static const IValue::eq_t<Value_wrap<ScriptClassInfo>> s;
		return s;
	}
	virtual const IValue::gt_base_t& gt() const {
		static const IValue::gt_t<Value_wrap<ScriptClassInfo>> s;
		return s;
	}
	virtual const IValue::ge_base_t& ge() const {
		static const IValue::ge_t<Value_wrap<ScriptClassInfo>> s;
		return s;
	}

	template <typename U> U get_value_stub()const {
		return U();
	}
	template <typename U> U& get_ref_stub() {
		auto v = U();
		return v;
	}
	template <> std::string get_value_stub()const {
		return (*(ScriptClassInfo*)p_instance).ToString();
	}
	template <typename U> void set_value_stub(const U& arg_v) {

	}


	template <typename U> bool eq_stub(const U& arg_v)const {
		return  false;
	}
};

template<>
class Value_wrap<int> :public IValue {
	public:
		Value_wrap(const int v, const int ref) :IValue(ref) {
			p_instance = new int(v);
		}
		Value_wrap(const int ref) :IValue(ref) {
			p_instance = new int();
		}
		~Value_wrap() override {
			if (!isOuterMemoryRef) {
				delete ((int*)p_instance);
			}
			if (ary_memberType) {
				delete ary_memberType;
			}
		}

		bool Eq(IValue* p_other)const override {
			return p_other->Equal(*(int*)p_instance);
		}
		bool Gt(IValue* p_other)const override {
			return !p_other->GreaterThan(*(int*)p_instance);
		}
		bool Ge(IValue* p_other)const override {
			return !p_other->GreaterEq(*(int*)p_instance);
		}

		void ValueCopy(IValue* p_other) const override {
			p_other->Set<int>(*(int*)p_instance);
		}

		IValue* Clone()const {
			return new Value_wrap<int>(*(int*)p_instance, 1);
		}

		void Nagative()override {
			*(int*)p_instance = -1 * (*(int*)p_instance);
		}


		virtual const IValue::get_value_base_t& get_value() const {
			static const IValue::get_value_t<Value_wrap<int>> s;
			return s;
		}
		virtual const IValue::get_ref_base_t& get_ref() const {
			static const IValue::get_ref_t<Value_wrap<int>> s;
			return s;
		}
		virtual const IValue::set_value_base_t& set_value() const {
			static const IValue::set_value_t<Value_wrap<int>> s;
			return s;
		}

		virtual const IValue::eq_base_t& eq() const {
			static const IValue::eq_t<Value_wrap<int>> s;
			return s;
		}
		virtual const IValue::gt_base_t& gt() const {
			static const IValue::gt_t<Value_wrap<int>> s;
			return s;
		}
		virtual const IValue::ge_base_t& ge() const {
			static const IValue::ge_t<Value_wrap<int>> s;
			return s;
		}

		template <typename U> U get_value_stub()const {
			//変換不可能な型の代入
			assert(0);
			return U();
		}
		template <> int get_value_stub()const {
			return *((int*)p_instance);
		}
		template <> float get_value_stub()const {
			return (float)*((int*)p_instance);
		}
		template <> std::string get_value_stub()const {
			return std::to_string(*((int*)p_instance));
		}
		template <typename U> U& get_ref_stub() {
			auto v = U();
			return v;
		}

		template <> int& get_ref_stub() {
			return *(int*)p_instance;
		}
		template <typename U> void set_value_stub(const U& arg_v) {
			//変換不可能な型の代入
			assert(0);
		}
		template <>void set_value_stub(const int& arg_v) {
			*(int*)p_instance = (arg_v);
		}
		template <>void set_value_stub(const float& arg_v) {
			*(int*)p_instance = (int)(arg_v);
		}
		template <>void set_value_stub(const std::string& arg_v) {
			*(int*)p_instance = StrConvert::ConvertString<int>(arg_v);
		}


		template <typename U> bool eq_stub(const U& arg_v)const {
			return  false;
		}
		template <>bool eq_stub(const int& arg_v)const {
			return *(int*)p_instance == arg_v;
		}
		template <>bool eq_stub(const float& arg_v)const {
			return *(int*)p_instance == (int)arg_v;
		}

		template <typename U> bool gt_stub(const U& arg_v)const {
			return false;
		}
		template <typename U> bool ge_stub(const U& arg_v)const {
			return false;
		}
		template <> bool gt_stub(const int& arg_v)const {
			return *(int*)p_instance > arg_v;
		}
		template <> bool ge_stub(const int& arg_v)const {
			return *(int*)p_instance >= arg_v;
		}
		template <> bool gt_stub(const float& arg_v)const {
			return *(int*)p_instance > (int)arg_v;
		}
		template <> bool ge_stub(const float& arg_v)const {
			return *(int*)p_instance >= (int)arg_v;
		}

	};

template<>
class Value_wrap<float> :public IValue {
	public:
		Value_wrap(const float v, const int ref) :IValue(ref) {
			p_instance = new float(v);
		}
		Value_wrap(const float ref) :IValue(ref) {
			p_instance = new float();
		}
		Value_wrap(float* v, const int ref,const bool arg_isOuterMemoryRef) :IValue(ref) {
			p_instance = v;
			isOuterMemoryRef = arg_isOuterMemoryRef;
		}
		~Value_wrap() override {
			if (!isOuterMemoryRef) {
				delete ((float*)p_instance);
			}
			if (ary_memberType) {
				delete ary_memberType;
			}
		}

		bool Eq(IValue* p_other)const override {
			return p_other->Equal(*(float*)p_instance);
		}
		bool Gt(IValue* p_other)const override {
			return !p_other->GreaterThan(*(float*)p_instance);
		}
		bool Ge(IValue* p_other)const override {
			return !p_other->GreaterEq(*(float*)p_instance);
		}

		void ValueCopy(IValue* p_other) const override {
			p_other->Set<float>(*(float*)p_instance);
		}

		IValue* Clone()const {
			return new Value_wrap<float>(*(float*)p_instance, 1);
		}

		void Nagative()override {
			*(float*)p_instance = -1 * (*(float*)p_instance);
		}

		virtual const IValue::get_value_base_t& get_value() const {
			static const IValue::get_value_t<Value_wrap<float>> s;
			return s;
		}
		virtual const IValue::get_ref_base_t& get_ref() const {
			static const IValue::get_ref_t<Value_wrap<float>> s;
			return s;
		}
		virtual const IValue::set_value_base_t& set_value() const {
			static const IValue::set_value_t<Value_wrap<float>> s;
			return s;
		}

		virtual const IValue::eq_base_t& eq() const {
			static const IValue::eq_t<Value_wrap<float>> s;
			return s;
		}
		virtual const IValue::gt_base_t& gt() const {
			static const IValue::gt_t<Value_wrap<float>> s;
			return s;
		}
		virtual const IValue::ge_base_t& ge() const {
			static const IValue::ge_t<Value_wrap<float>> s;
			return s;
		}

		template <typename U> U get_value_stub()const {
			//変換不可能な型の代入
			assert(0);
			return U();
		}
		template <> std::string get_value_stub()const {
			return std::to_string(*((float*)p_instance));
		}
		template <> float get_value_stub()const {
			return *((float*)p_instance);
		}
		template <> int get_value_stub()const {
			return (int)*((float*)p_instance);
		}
		template <typename U> U& get_ref_stub() {
			auto v = U();
			return v;
		}

		template <> float& get_ref_stub() {
			return *(float*)p_instance;
		}
		template <typename U> void set_value_stub(const U& arg_v) {
			//変換不可能な型の代入
			assert(0);
		}
		template <>void set_value_stub(const float& arg_v) {
			*(float*)p_instance = (arg_v);
		}
		template <>void set_value_stub(const int& arg_v) {
			*(float*)p_instance = (float)(arg_v);
		}

		template <>void set_value_stub(const std::string& arg_v) {
			*(float*)p_instance = StrConvert::ConvertString<float>(arg_v);
		}

		template <typename U> bool eq_stub(const U& arg_v)const {
			return  false;
		}
		template <>bool eq_stub(const float& arg_v)const {
			return *(float*)p_instance == arg_v;
		}
		template <>bool eq_stub(const int& arg_v)const {
			return *(float*)p_instance == (float)arg_v;
		}

		template <typename U> bool gt_stub(const U& arg_v)const {
			return false;
		}
		template <typename U> bool ge_stub(const U& arg_v)const {
			return false;
		}
		template <> bool gt_stub(const float& arg_v)const {
			return *(float*)p_instance > arg_v;
		}
		template <> bool ge_stub(const float& arg_v)const {
			return *(float*)p_instance >= arg_v;
		}
		template <> bool gt_stub(const int& arg_v)const {
			return *(float*)p_instance > (float)arg_v;
		}
		template <> bool ge_stub(const int& arg_v)const {
			return *(float*)p_instance >= (float)arg_v;
		}

	};

template<>
class Value_wrap<std::string> : public IValue {
	public:
		Value_wrap(const std::string& v, const int ref) :IValue(ref) {
			p_instance = new std::string(v);
		}
		Value_wrap(const int ref) :IValue(ref) {
			p_instance = new std::string();
		}
		~Value_wrap() override{
			delete ((std::string*)p_instance);
		}

		bool Eq(IValue* p_other)const override {
			return p_other->Equal(*(std::string*)p_instance);
		}
		bool Gt(IValue* p_other)const override {
			return !p_other->GreaterThan(*(std::string*)p_instance);
		}
		bool Ge(IValue* p_other)const override {
			return !p_other->GreaterEq(*(std::string*)p_instance);
		}

		void ValueCopy(IValue* p_other) const override {
			p_other->Set<std::string>(*(std::string*)p_instance);
		}

		IValue* Clone()const {
			return new Value_wrap<std::string>(*(std::string*)p_instance, 1);
		}

		void Nagative()override {
			//文字列に単項マイナスはない
			assert(0);

		}


		virtual const IValue::get_value_base_t& get_value() const {
			static const IValue::get_value_t<Value_wrap<std::string>> s;
			return s;
		}
		virtual const IValue::get_ref_base_t& get_ref() const {
			static const IValue::get_ref_t<Value_wrap<std::string>> s;
			return s;
		}
		virtual const IValue::set_value_base_t& set_value() const {
			static const IValue::set_value_t<Value_wrap<std::string>> s;
			return s;
		}

		virtual const IValue::eq_base_t& eq() const {
			static const IValue::eq_t<Value_wrap<std::string>> s;
			return s;
		}
		virtual const IValue::gt_base_t& gt() const {
			static const IValue::gt_t<Value_wrap<std::string>> s;
			return s;
		}
		virtual const IValue::ge_base_t& ge() const {
			static const IValue::ge_t<Value_wrap<std::string>> s;
			return s;
		}

		template <typename U> U get_value_stub()const {
			return (U)StrConvert::ConvertString<U>(*(std::string*)p_instance);
		}
		template <typename U> U& get_ref_stub() {
			auto v = U();
			return v;
		}
		template <> std::string get_value_stub()const {
			return (*(std::string*)p_instance);
		}
		template <> std::string& get_ref_stub() {
			return *(std::string*)p_instance;
		}
		template <typename U> void set_value_stub(const U& arg_v) {
			*(std::string*)p_instance = std::to_string(arg_v);
		}
		template <>void set_value_stub(const std::string& arg_v) {
			*(std::string*)p_instance = (arg_v);
		}


		template <typename U> bool eq_stub(const U& arg_v)const {
			return  false;
		}

		template <>bool eq_stub(const std::string& arg_v)const {
			return *(std::string*)p_instance == arg_v;
		}

	};
template<>
class Value_wrap<ButiEngine::Vector2> : public IValue {
	public:
		Value_wrap(const ButiEngine::Vector2& v, const int ref) :IValue(ref) {
			p_instance = new ButiEngine::Vector2(v);
			ary_p_member = (IValue**)malloc(sizeof(IValue*) * 2);
			ary_memberType = (int*)malloc(sizeof(int) * 2);
			ary_p_member[0] = new Value_wrap<float>(&((ButiEngine::Vector2*)p_instance)->x, 1,true);
			ary_p_member[1] = new Value_wrap<float>(&((ButiEngine::Vector2*)p_instance)->y, 1, true);
			ary_memberType[0] = TYPE_FLOAT;
			ary_memberType[1] = TYPE_FLOAT;
		}
		Value_wrap(const int ref) :IValue(ref) {
			p_instance = new ButiEngine::Vector2();
			ary_p_member = (IValue**)malloc(sizeof(IValue*) * 2);
			ary_memberType = (int*)malloc(sizeof(int) * 2);
			ary_p_member[0] = new Value_wrap<float>(&((ButiEngine::Vector2*)p_instance)->x, 1, true);
			ary_p_member[1] = new Value_wrap<float>(&((ButiEngine::Vector2*)p_instance)->y, 1, true);
			ary_memberType[0] = TYPE_FLOAT;
			ary_memberType[1] = TYPE_FLOAT;
		}

		~Value_wrap() {
			for (int i = 0; i < 2; i++) {
				ary_p_member[i]->release();
			}

			delete ((ButiEngine::Vector2*)p_instance);
			if (ary_memberType) {
				free(ary_memberType);
			}
			if (ary_p_member) {
				free(ary_p_member);
			}
		}
		RegistSpecialization(ButiEngine::Vector2);
	};
template<>
class Value_wrap<ButiEngine::Vector3> : public IValue {
	public:
		Value_wrap(const ButiEngine::Vector3& v, const int ref) :IValue(ref) {
			p_instance = new ButiEngine::Vector3(v);

			ary_p_member = (IValue**)malloc(sizeof(IValue*) * 3);
			ary_memberType = (int*)malloc(sizeof(int) * 3);

			ary_p_member[0] = new Value_wrap<float>(&((ButiEngine::Vector3*)p_instance)->x, 1, true);
			ary_p_member[1] = new Value_wrap<float>(&((ButiEngine::Vector3*)p_instance)->y, 1, true);
			ary_p_member[2] = new Value_wrap<float>(&((ButiEngine::Vector3*)p_instance)->z, 1, true);
			ary_memberType[0] = TYPE_FLOAT;
			ary_memberType[1] = TYPE_FLOAT;
			ary_memberType[2] = TYPE_FLOAT;
		}
		Value_wrap(const int ref) :IValue(ref) {
			p_instance = new ButiEngine::Vector3();
			ary_p_member = (IValue**)malloc(sizeof(IValue*) * 3);
			ary_memberType = (int*)malloc(sizeof(int) * 3);
			ary_p_member[0] = new Value_wrap<float>(&((ButiEngine::Vector3*)p_instance)->x, 1, true);
			ary_p_member[1] = new Value_wrap<float>(&((ButiEngine::Vector3*)p_instance)->y, 1,true);
			ary_p_member[2] = new Value_wrap<float>(&((ButiEngine::Vector3*)p_instance)->z, 1,true);
			ary_memberType[0] = TYPE_FLOAT;
			ary_memberType[1] = TYPE_FLOAT;
			ary_memberType[2] = TYPE_FLOAT;
		}
		~Value_wrap() {
			for (int i = 0; i < 3; i++) {
				ary_p_member[i]->release();
			}

			delete ((ButiEngine::Vector3*)p_instance); 
			if (ary_memberType) {
				free( ary_memberType); 
			}
			if (ary_p_member) {
				free(ary_p_member);
			}
		}
		RegistSpecialization(ButiEngine::Vector3);
	};
template<>
class Value_wrap<ButiEngine::Vector4> : public IValue {
	public:
		Value_wrap(const ButiEngine::Vector4& v, const int ref) :IValue(ref) {
			p_instance = new ButiEngine::Vector4(v);
			ary_p_member = (IValue**)malloc(sizeof(IValue*) * 4);
			ary_memberType = (int*)malloc(sizeof(int) * 4);
			ary_p_member[0] = new Value_wrap<float>(&((ButiEngine::Vector4*)p_instance)->x, 1,true);
			ary_p_member[1] = new Value_wrap<float>(&((ButiEngine::Vector4*)p_instance)->y, 1,true);
			ary_p_member[2] = new Value_wrap<float>(&((ButiEngine::Vector4*)p_instance)->z, 1,true);
			ary_p_member[3] = new Value_wrap<float>(&((ButiEngine::Vector4*)p_instance)->w, 1,true);
			ary_memberType[0] = TYPE_FLOAT;
			ary_memberType[1] = TYPE_FLOAT;
			ary_memberType[2] = TYPE_FLOAT;
			ary_memberType[3] = TYPE_FLOAT;
		}
		Value_wrap(const int ref) :IValue(ref) {
			p_instance = new ButiEngine::Vector4();
			ary_p_member = (IValue**)malloc(sizeof(IValue*) * 4);
			ary_memberType = (int*)malloc(sizeof(int) * 4);
			ary_p_member[0] = new Value_wrap<float>(&((ButiEngine::Vector4*)p_instance)->x, 1,true);
			ary_p_member[1] = new Value_wrap<float>(&((ButiEngine::Vector4*)p_instance)->y, 1,true);
			ary_p_member[2] = new Value_wrap<float>(&((ButiEngine::Vector4*)p_instance)->z, 1,true);
			ary_p_member[3] = new Value_wrap<float>(&((ButiEngine::Vector4*)p_instance)->w, 1,true);
			ary_memberType[0] = TYPE_FLOAT;
			ary_memberType[1] = TYPE_FLOAT;
			ary_memberType[2] = TYPE_FLOAT;
			ary_memberType[3] = TYPE_FLOAT;
		}
		~Value_wrap() {
			for (int i = 0; i < 4; i++) {
				ary_p_member[i]->release();
			}

			delete ((ButiEngine::Vector4*)p_instance);
			if (ary_memberType) {
				free(ary_memberType);
			}
			if (ary_p_member) {
				free(ary_p_member);
			}
		}
		RegistSpecialization(ButiEngine::Vector4);
	};

//参照型など実体を持たない変数の初期化に使用
struct Type_Null {};

template<typename T>
IValue* CreateMemberInstance() {
	return new Value_wrap<T>(1);
}


using CreateMemberInstanceFunction = IValue* (*)();

template<typename T>
void PushCreateMemberInstance() {
	PushCreateMemberInstance(CreateMemberInstance<T>);
}
void PushCreateMemberInstance(CreateMemberInstanceFunction arg_function);

// 変数
class Value {

public:
	Value()
	{
		valueType = TYPE_VOID;
		v_ = nullptr;
	}

	Value(const Type_Null) {
		v_ = nullptr;
		valueType = TYPE_VOID;
	}

	//intとして初期化
	Value(const int ival)
	{
		v_ = new Value_wrap<int>(ival,1);
		valueType = TYPE_INTEGER;
	}

	//floatとして初期化
	Value(const float ival)
	{
		v_ = new Value_wrap<float>(ival,1);
		valueType = TYPE_FLOAT;
	}

	//stringとして初期化
	Value(const std::string& str)
	{
		v_ = new Value_wrap<std::string>(str, 1);
		valueType = TYPE_STRING;
	}

	//Vector2として初期化
	Value(const ButiEngine::Vector2& vec2)
	{
		v_ = new Value_wrap<ButiEngine::Vector2>(vec2, 1);
		valueType = TYPE_VOID + 1;
	}
	//Vector3として初期化
	Value(const ButiEngine::Vector3& vec3)
	{
		v_ = new Value_wrap<ButiEngine::Vector3>(vec3, 1);
		valueType = TYPE_VOID + 2;
	}
	//Vector4として初期化
	Value(const ButiEngine::Vector4& vec4)
	{
		v_ = new Value_wrap<ButiEngine::Vector4>(vec4, 1);
		valueType = TYPE_VOID + 3;
	}
	//ユーザー定義型として初期化
	Value(ScriptClassInfo& arg_info, std::vector<ButiScript::ScriptClassInfo>* arg_p_vec_scriptClassInfo);

	//変数を指定して初期化
	Value(IValue* p,const int type)
	{
		v_ = p;
		v_->addref();
		valueType = type;
	}

	~Value()
	{
		clear();
	}

	Value(const Value& a)
	{
		clear();
		Copy(a);
	}

	Value& operator=(const Value& a)
	{
		if (this == &a)
			return *this;


		Assign(a);

		return *this;
	}

	void clear()
	{
		if (v_) {
			v_->release();
			v_ = nullptr;
		}
	}

	Value Clone() {
		auto cloneV = v_->Clone();
		auto output = Value(cloneV, valueType);
		output.v_->release();
		return output;
	}

	void Copy(const Value& a)
	{
		valueType = a.valueType;
		v_ = a.v_->Clone();
	}
	void Assign(const Value& a) {
		if (!a.v_) {

			valueType = a.valueType;
			return;
		}

		if (v_) {
			v_->Set(*a.v_);
		}
		else {
			//自分が参照型の場合、相手の実体への参照を取得
			if (valueType & TYPE_REF || valueType == TYPE_VOID) {

				clear();

				v_ = a.v_;

				v_->addref();

				if (valueType == TYPE_VOID) {
					valueType = a.valueType;
				}
			}
			else {
				v_ = a.v_->Clone();
			}
		}

		


	}


	void SetType(const int arg_type) {
		valueType = arg_type;
	}
	union {
		IValue* v_=nullptr;
	};
	int valueType;
};

class StackOverflow : public std::exception {
public:
	const char* what() const throw()
	{
		return "stack overflow";
	}
};

// 固定サイズスタック
template< typename Ty, int Size >
class Stack {
public:
	Stack() : size_(0)
	{
	}

	~Stack()
	{
		resize(0);
	}

	void push(const Ty& arg_value)
	{
		if (Size <= size_) {
			throw StackOverflow();
		}
		*(::new(data_[size_++]) Ty) = arg_value;
	}

	void pop()
	{
		((Ty*)data_[--size_])->~Ty();
	}

	void pop(const int count)
	{
		resize(size_ - count);
	}

	void resize(const int newsize)
	{
		int oldsize = size_;

		if (oldsize > newsize) {
			for (int i = newsize; i < oldsize; ++i)
				((Ty*)data_[i])->~Ty();
		}
		if (oldsize < newsize) {
			if (Size < newsize)
				throw StackOverflow();
			for (int i = oldsize; i < newsize; ++i)
				::new(data_[i]) Ty;
		}
		size_ = newsize;
	}

	const Ty& top() const { return *(const Ty*)data_[size_ - 1]; }
	Ty& top() {
		return *(Ty*)data_[size_ - 1]; 
	}

	bool overflow() const { return size_ >= Size; }
	bool empty() const { return size_ == 0; }
	int size() const { 
		return 	size_;
	}

	const Ty& operator[](const int index) const { 
		return *(const Ty*)data_[index]; 
	}
	Ty& operator[](const int index) { 
		return *(Ty*)data_[index]; 
	}

protected:
	char data_[Size][sizeof(Ty)];
	int size_;
};
}

#endif