#pragma once
#ifndef	__vm_value_h__
#define	__vm_value_h__

#include <iostream>
#include <exception>
#include <string>

namespace ButiVM {
	class VirtualString;
	class VirtualInteger;
	class VirtualFloat;
	// 参照カウンタ付き変数の基底クラス
	class RefValue {
	public:
		RefValue() : ref_(0)
		{
		}
		RefValue(int arg_ref) : ref_(arg_ref)
		{
		}

		void addref()
		{
			ref_++;
		}

		void release()
		{
			if (--ref_ == 0)
				delete this;
		}


		virtual int* GetIntPtr() {
			return nullptr;
		}
		virtual float* GetFloatPtr() {
			return nullptr;
		}

		//変数の計算処理


		//変数の比較処理
		virtual bool Eq(RefValue* arg_other) = 0;
		virtual bool EqByInt(VirtualInteger* arg_other) { return false; }
		virtual bool EqByFloat(VirtualFloat* arg_other) { return false; }
		virtual bool EqByString(VirtualString* arg_other) { return false; }
		virtual bool Gt(RefValue* arg_other) = 0;
		virtual bool GtByInt(VirtualInteger* arg_other) { return false; }
		virtual bool GtByFloat(VirtualFloat* arg_other) { return false; }
		virtual bool GtByString(VirtualString* arg_other) { return false; }
		virtual bool Ge(RefValue* arg_other) = 0;
		virtual bool GeByInt(VirtualInteger* arg_other) { return false; }
		virtual bool GeByFloat(VirtualFloat* arg_other) { return false; }
		virtual bool GeByString(VirtualString* arg_other) { return false; }


		//変数の代入処理
		virtual void Set(RefValue* arg_v) = 0;
		virtual void SetByInt(VirtualInteger*) {}
		virtual void SetByFloat(VirtualFloat*) {}
		virtual void SetByString(VirtualString*) {}

		//単項マイナス
		virtual void ToNegative()=0;

		/// 特定の型に変換
		virtual std::string ToText()=0;
		virtual int ToInt()const { return 0; }
		virtual float ToFloat()const { 
			return 0.0f;
		}

		//クローン
		virtual RefValue* Clone() = 0;
	public:
		int ref_;
	};

	// string
	class VirtualString :public RefValue {
	public:
		VirtualString() : RefValue(0)
		{
		}
		VirtualString(const std::string& str) : RefValue(1), str_(str)
		{
		}
		std::string ToText() override {
			return str_;
		}

		bool Eq(RefValue* arg_other) { return arg_other->EqByString(this); }
		bool Gt(RefValue* arg_other) { return arg_other->GtByString(this); }
		bool Ge(RefValue* arg_other) { return arg_other->GeByString(this);}

		bool EqByString(VirtualString* arg_other) override;
		bool GtByString(VirtualString* arg_other) override;
		bool GeByString(VirtualString* arg_other) override;

		void Set(RefValue* arg_v)override {arg_v->SetByString(this);}
		void SetByInt(VirtualInteger* arg_v)override;
		void SetByFloat(VirtualFloat* arg_v)override;
		void SetByString(VirtualString* arg_v)override;

		void ToNegative()override{}

		RefValue* Clone() override{
			return new VirtualString(str_);
		}

	public:
		std::string str_;
	};
	// int
	class VirtualInteger :public RefValue {
	public:
		VirtualInteger() : RefValue(0)
		{
		}
		VirtualInteger(const int arg_v) : RefValue(1), v(arg_v)
		{
		}
		std::string ToText() override {
			return std::to_string(v);
		}

		int ToInt()const {return v;}
		float ToFloat()const { 
			return (float)v; 
		}

		int* GetIntPtr() override {
			return &v;
		}

		bool Eq(RefValue* arg_other) { return arg_other->EqByInt(this); }
		bool Gt(RefValue* arg_other) { return arg_other->GtByInt(this); }
		bool Ge(RefValue* arg_other) { return arg_other->GeByInt(this); }

		bool EqByInt(VirtualInteger* arg_other) override;
		bool GtByInt(VirtualInteger* arg_other) override;
		bool GeByInt(VirtualInteger* arg_other) override;

		bool EqByFloat(VirtualFloat* arg_other) override;
		bool GtByFloat(VirtualFloat* arg_other) override;
		bool GeByFloat(VirtualFloat* arg_other) override;


		void Set(RefValue* arg_v)override { arg_v->SetByInt(this); }
		void SetByInt(VirtualInteger* arg_v)override;
		void SetByFloat(VirtualFloat* arg_v)override;


		void ToNegative()override {v = -v;}
		RefValue* Clone() override {
			return new VirtualInteger(v);
		}
	public:
		int v;
	};

	// float
	class VirtualFloat :public RefValue {
	public:
		VirtualFloat() : RefValue(0)
		{
		}
		VirtualFloat(const float arg_v) : RefValue(1), v(arg_v)
		{
		}
		std::string ToText() override {
			return std::to_string(v);
		}

		int ToInt()const { return (int)v; }
		float ToFloat()const { 
			return v; 
		}
		float* GetFloatPtr() override {
			return &v;
		}

		bool Eq(RefValue* arg_other) { return  arg_other->EqByFloat(this); }
		bool Gt(RefValue* arg_other) { return  arg_other->GtByFloat(this); }
		bool Ge(RefValue* arg_other) { return  arg_other->GeByFloat(this); }

		bool EqByInt(VirtualInteger* arg_other) override;
		bool GtByInt(VirtualInteger* arg_other) override;
		bool GeByInt(VirtualInteger* arg_other) override;

		bool EqByFloat(VirtualFloat* arg_other) override;
		bool GtByFloat(VirtualFloat* arg_other) override;
		bool GeByFloat(VirtualFloat* arg_other) override;

		void SetByInt(VirtualInteger* arg_v)override;
		void SetByFloat(VirtualFloat* arg_v)override;
		void Set(RefValue* arg_v)override {
			arg_v->SetByFloat(this);
		}


		void ToNegative()override {
			v = -v;
		}

		RefValue* Clone() override {
			return new VirtualFloat(v);
		}
	public:
		float v;
	};

	// 変数
	class Value {
		enum {
			type_integer,
			type_float,
			type_string,
		};

	public:
		Value()
		{
			v_ = new VirtualInteger(0);
			type_ = type_integer;
		}

		//intとして初期化
		Value(int ival)
		{
			v_ = new VirtualInteger(ival);
			type_ = type_integer;
		}

		//floatとして初期化
		Value(float ival)
		{
			v_ = new VirtualFloat(ival);
			type_ = type_float;
		}

		//stringとして初期化
		Value(const std::string& str)
		{
			v_ = new VirtualString(str);
			type_ = type_string;
		}

		//stringとして初期化
		Value(VirtualString* p)
		{
			v_ = p;
			type_ = type_string;
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
			Copy(a);

			return *this;
		}

		void clear()
		{
			if (type_ == type_string)
				v_->release();
		}

		void Copy(const Value& a)
		{
			type_ = a.type_;
			if (type_ == type_string) {

				v_ = a.v_;
				v_->addref();
			}
			else {
				v_ = a.v_->Clone();
			}
		}

		union {
			RefValue* v_;
		};
		char type_;
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
			if (Size <= size_)
				throw StackOverflow();
			*(::new(data_[size_++]) Ty) = arg_value;
		}

		void pop()
		{
			((Ty*)data_[--size_])->~Ty();
		}

		void pop(int count)
		{
			resize(size_ - count);
		}

		void resize(int newsize)
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
		Ty& top() { return *(Ty*)data_[size_ - 1]; }

		bool overflow() const { return size_ >= Size; }
		bool empty() const { return size_ == 0; }
		int size() const { return size_; }

		const Ty& operator[](int index) const { return *(const Ty*)data_[index]; }
		Ty& operator[](int index) { return *(Ty*)data_[index]; }

	protected:
		char data_[Size][sizeof(Ty)];
		int size_;
	};

}	// namespace

#endif