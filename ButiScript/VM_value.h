#pragma once
#ifndef	__vm_value_h__
#define	__vm_value_h__
#include"value_type.h"
#include <iostream>
#include <exception>
#include <string>
#include"ButiMath.h"
#define RegistGet(T) \
virtual T operator()(const IValue* b, const T * p_dummy) const {\
return static_cast<const Class*>(b)->get_value_stub< T >();\
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

namespace ButiScript {


	class IValue {
	public:
		IValue(const int arg_ref) {
			ref_ = arg_ref;
		}

		virtual ~IValue() {
			delete p_instance;
		}

		template <typename T> T Get()const {
			static T* p_dummy = nullptr;
			return get_value()(this, p_dummy);
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
			RegistGe(std::string);

		};


		struct empty_class { struct get_value_base_t {}; struct set_value_base_t {}; struct eq_base_t {}; struct gt_base_t {}; struct ge_base_t {}; };
		typedef get_value_t<const IValue, empty_class> get_value_base_t;
		typedef set_value_t<IValue, empty_class> set_value_base_t;
		typedef eq_t<const IValue, empty_class> eq_base_t;
		typedef gt_t<const IValue, empty_class> gt_base_t;
		typedef ge_t<const IValue, empty_class> ge_base_t;
		virtual const get_value_base_t& get_value() const = 0;
		virtual const set_value_base_t& set_value() const = 0;
		virtual const eq_base_t& eq() const = 0;
		virtual const gt_base_t& gt() const = 0;
		virtual const ge_base_t& ge() const = 0;


		template <typename T> T get_value_stub()const {
			assert(0);
			return T();
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
		void* p_instance;
		//メンバ変数
		IValue** ary_p_member;
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
		}
		Value_wrap(const int ref) :IValue(ref) {
			p_instance = new T();
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
		template <typename U> void set_value_stub(const U& arg_v) {
			*(T*)p_instance = (T)arg_v;
		}

		template <typename U> bool eq_stub(const U& arg_v)const {
			return  *(T*)p_instance == arg_v;
		}
		template <typename U> bool gt_stub(const U& arg_v)const {
			return *(T*)p_instance > arg_v;
		}
		template <typename U> bool ge_stub(const U& arg_v)const {
			return *(T*)p_instance >= arg_v;
		}


		template <> std::string get_value_stub()const {
			return std::to_string(*(T*)p_instance);
		}
		template <>void set_value_stub(const std::string& arg_v) {
			*(T*)p_instance = StrConvert::ConvertString<T>(arg_v);
		}

		template <> ButiEngine::Vector2 get_value_stub()const {
			return ButiEngine::Vector2();
		}
		template <>void set_value_stub(const ButiEngine::Vector2& arg_v) {
			*(T*)p_instance = arg_v.x;
		}

		template <>bool eq_stub(const std::string& arg_v)const {
			return false;
		}
		template <>bool gt_stub(const std::string& arg_v)const {
			return false;
		}
		template <>bool ge_stub(const std::string& arg_v)const {
			return false;
		}
		template <>bool eq_stub(const ButiEngine::Vector2& arg_v)const {
			return false;
		}
		template <>bool gt_stub(const ButiEngine::Vector2& arg_v)const {
			return false;
		}
		template <>bool ge_stub(const ButiEngine::Vector2& arg_v)const {
			return false;
		}

	private:
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
		template <> std::string get_value_stub()const {
			return (*(std::string*)p_instance);
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
			ary_p_member = (IValue**)malloc(sizeof(IValue*)*2);
			ary_p_member[0] = new Value_wrap<float>(&((ButiEngine::Vector2*)p_instance)->x, 1);
			ary_p_member[1] = new Value_wrap<float>(&((ButiEngine::Vector2*)p_instance)->y, 1);
		}
		Value_wrap(const int ref) :IValue(ref) {
			p_instance = new ButiEngine::Vector2();
		}


		bool Eq(IValue* p_other)const override {
			return p_other->Equal(*(ButiEngine::Vector2 *)p_instance);
		}
		bool Gt(IValue* p_other)const override {
			return false;
		}
		bool Ge(IValue* p_other)const override {
			return false;
		}

		void ValueCopy(IValue* p_other) const override {
			p_other->Set<ButiEngine::Vector2>(*(ButiEngine::Vector2*)p_instance);
		}

		IValue* Clone()const {
			return new Value_wrap<ButiEngine::Vector2>(*(ButiEngine::Vector2*)p_instance,1);
		}

		void Nagative()override {
			*(ButiEngine::Vector2*)p_instance = -1 * (*(ButiEngine::Vector2*)p_instance);

		}


		virtual const IValue::get_value_base_t& get_value() const {
			static const IValue::get_value_t<Value_wrap<ButiEngine::Vector2>> s;
			return s;
		}
		virtual const IValue::set_value_base_t& set_value() const {
			static const IValue::set_value_t<Value_wrap<ButiEngine::Vector2>> s;
			return s;
		}

		virtual const IValue::eq_base_t& eq() const {
			static const IValue::eq_t<Value_wrap<ButiEngine::Vector2>> s;
			return s;
		}
		virtual const IValue::gt_base_t& gt() const {
			static const IValue::gt_t<Value_wrap<ButiEngine::Vector2>> s;
			return s;
		}
		virtual const IValue::ge_base_t& ge() const {
			static const IValue::ge_t<Value_wrap<ButiEngine::Vector2>> s;
			return s;
		}

		template <typename U> U get_value_stub()const {
			return U();
		}
		template <> ButiEngine::Vector2 get_value_stub()const {
			return (*(ButiEngine::Vector2*)p_instance);
		}
		template <> std::string get_value_stub()const {
			return std::to_string(*(ButiEngine::Vector2*)p_instance);
		}

		template <typename U> void set_value_stub(const U& arg_v) {
			*(std::string*)p_instance = std::to_string(arg_v);
		}
		template <>void set_value_stub(const ButiEngine::Vector2& arg_v) {
			*(ButiEngine::Vector2*)p_instance = (arg_v);
		}
		template <>void set_value_stub(const std::string& arg_v) {
			*(ButiEngine::Vector2 *)p_instance = StrConvert::ConvertString<ButiEngine::Vector2>(arg_v);
		}


		template <typename U> bool eq_stub(const U& arg_v)const {
			return  false;
		}

		template <>bool eq_stub(const ButiEngine::Vector2& arg_v)const {
			return *(ButiEngine::Vector2*)p_instance == arg_v;
		}

	};

	//参照型など実体を持たない変数の初期化に使用
	struct Type_Null {};

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
		Value(const ButiEngine::Vector2 vec2)
		{
			v_ = new Value_wrap<ButiEngine::Vector2>(vec2, 1);
			valueType = TYPE_VOID+1;
		}

		//変数を指定して初期化
		Value(IValue* p,const int type)
		{
			v_ = p->Clone();
			valueType = type;
		}

		~Value()
		{
			clear();
		}

		Value(const Value& a)
		{
			Copy(a);
		}

		Value& operator=(const Value& a)
		{
			if (this == &a)
				return *this;

			clear();

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

		void Copy(const Value& a)
		{
			valueType = a.valueType;
			if (valueType == TYPE_STRING) {

				v_ = a.v_;
				v_->addref();
			}
			else {
				v_ = a.v_->Clone();
			}
		}
		void Assign(const Value& a) {
			//自分が参照型の場合、相手の実体への参照を取得
			if (valueType & TYPE_REF|| a.valueType == TYPE_STRING) {
				v_ = a.v_;
				v_->addref();

				if (valueType == TYPE_VOID) {
					valueType = a.valueType;
				}
				return;
			}

			{
				if (v_) {
					v_->Set(*a.v_);
				}
				else {
					if (a.v_) {
						v_ = a.v_->Clone();
					}
				}
			}
			if (valueType == TYPE_VOID) {
				valueType = a.valueType;
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
		int size() const { return size_; }

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

}	// namespace

#endif