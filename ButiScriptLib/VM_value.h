#pragma once
#ifndef	__vm_value_h__
#define	__vm_value_h__
#include"value_type.h"
#include <iostream>
#include <exception>
#include <string>
#include"Tags.h"
#include"../../Header/Common/ButiMath.h"
//using Sample_t = std::shared_ptr<ButiScript:: Sample>;

namespace std {
template <typename T>
static std::string to_string(const T&) {
	return "このクラスはto_string()に対応していません";
	}
}
////////////////////////////////
////テンプレート継承の実装用マクロ////
////////////////////////////////
#define RegistGet(T) \
virtual T operator()(const IValueData* b, const T * p_dummy) const {\
return static_cast<const Class*>(b)->get_value_stub< T >();\
}\

#define RegistGetRef(T) \
virtual T& operator()(IValueData* b, const T * p_dummy) const {\
return static_cast<Class*>(b)->get_ref_stub< T >();\
}\

#define RegistSet(T) \
virtual void operator()(IValueData* b,const T arg_v) const {\
static_cast<Class*>(b)->set_value_stub<T>(arg_v);\
}\

#define RegistSetRef(T) \
virtual void operator()(IValueData* b,const T & arg_v) const {\
static_cast<Class*>(b)->set_value_stub<T>(arg_v);\
}\

#define RegistEq(T) \
virtual bool operator()(const IValueData* b,const T & arg_v) const {\
return static_cast<const Class*>(b)->eq_stub<T>(arg_v);\
}\

#define RegistGt(T) \
virtual bool operator()(const IValueData* b,const T & arg_v) const {\
 return static_cast<const Class*>(b)->gt_stub<T>(arg_v);\
}\

#define RegistGe(T) \
virtual bool operator()(const IValueData* b,const T & arg_v) const {\
return static_cast<const Class*>(b)->ge_stub<T>(arg_v);\
}\


//////////////////////////////////
////変数の中身の明示的特殊化用マクロ////
//////////////////////////////////
#define RegistSpecialization(Type)\
bool Eq(IValueData* p_other)const override {\
	return p_other->Equal(*( Type *)p_instance);\
}\
bool Gt(IValueData* p_other)const override {\
	return false;\
}\
bool Ge(IValueData* p_other)const override {\
	return false;\
}\
void ValueCopy(IValueData* p_other) const override {\
	p_other->Set< Type >(*( Type *)p_instance);\
}\
IValueData* Clone()const {\
	return new ValueData< Type >(*( Type *)p_instance, 1);\
}\
void Nagative()override {\
	*( Type *)p_instance = -1 * (*( Type *)p_instance);\
}\
virtual const IValueData::get_value_base_t& get_value() const {\
	static const IValueData::get_value_t<ValueData< Type >> s;\
	return s;\
}\
virtual const IValueData::get_ref_base_t& get_ref() const {\
	static const IValueData::get_ref_t<ValueData< Type >> s;\
	return s;\
}\
virtual const IValueData::set_value_base_t& set_value() const {\
	static const IValueData::set_value_t<ValueData< Type >> s;\
	return s;\
}\
virtual const IValueData::eq_base_t& eq() const {\
	static const IValueData::eq_t<ValueData< Type >> s;\
	return s;\
}\
virtual const IValueData::gt_base_t& gt() const {\
	static const IValueData::gt_t<ValueData< Type >> s;\
	return s;\
}\
virtual const IValueData::ge_base_t& ge() const {\
	static const IValueData::ge_t<ValueData< Type >> s;\
	return s;\
}\
template <typename U> U get_value_stub()const {\
	return U();\
}\
template <typename U> U& get_ref_stub() {\
	auto arg_v = U();\
	return arg_v;\
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

//参照型など実体を持たない変数の初期化に使用
struct Type_Null {};
struct Type_Enum {
	operator int() const {
		return 0;
	}
};
struct Type_Func {
	operator int() const {
		return 0;
	}
};
template<typename T>
long long int TypeSpecific() {
	static T output[1];
	return (long long int) output;
}
#ifdef IMPL_BUTIENGINE
class IValueData;
#ifndef IGLOBALVALUESAVEOBJECT_DEFINE
#define IGLOBALVALUESAVEOBJECT_DEFINE
class IGlobalValueSaveObject {
public:

	virtual void RestoreValue(IValueData** arg_v)const = 0;
	virtual void SetCompiledData(std::shared_ptr<CompiledData> arg_shp_data) {}
	virtual int GetTypeIndex()const = 0;
	virtual void SetTypeIndex(const int arg_index) = 0;
};
#endif // !IGLOBALVALUESAVEOBJECT_DEFINE

class GlobalScriptTypeValueSaveObject :public IGlobalValueSaveObject {
public:
	GlobalScriptTypeValueSaveObject() {}
	void SetCompiledData(std::shared_ptr<CompiledData> arg_shp_data)override { shp_compiledData = arg_shp_data; }
	void RestoreValue(IValueData** arg_v)const override;
	void Push(std::shared_ptr<IGlobalValueSaveObject> shp_value, int arg_index) {
		shp_value->SetTypeIndex(arg_index);
		vec_data.push_back(shp_value);
	}
	int GetTypeIndex()const override {
		return arg_type;
	}
	void SetTypeIndex(const int arg_index)override {
		arg_type = arg_index;
	}
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(vec_data);
		archive(arg_type);
	}
private:
	std::vector<std::shared_ptr<IGlobalValueSaveObject>> vec_data;
	std::shared_ptr<CompiledData>shp_compiledData;
	int arg_type;
};
template<typename T>
class GlobalValueSaveObject :public IGlobalValueSaveObject {
public:
	GlobalValueSaveObject(const T& arg_value) {
		data = arg_value;
	}
	GlobalValueSaveObject() {
	}
	void RestoreValue(IValueData** arg_v)const override;
	int GetTypeIndex()const override {
		return arg_type;
	}
	void SetTypeIndex(const int arg_index)override {
		arg_type = arg_index;
	}
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(data);
		archive(arg_type);
	}
private:
	T data;
	int arg_type;
};
template<>
class GlobalValueSaveObject <Type_Enum>:public IGlobalValueSaveObject {
public:
	GlobalValueSaveObject(const int& arg_value) {
		data = arg_value;
	}
	GlobalValueSaveObject() {
	}
	void SetCompiledData(std::shared_ptr<CompiledData> arg_shp_data) override{ shp_compiledData = arg_shp_data; }
	void RestoreValue(IValueData** arg_v)const override;
	int GetTypeIndex()const override {
		return arg_type;
	}
	void SetTypeIndex(const int arg_index)override {
		arg_type = arg_index;
	}
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(data);
		archive(arg_type);
	}
private:
	int data;
	int arg_type;
	std::shared_ptr<CompiledData>shp_compiledData;
};
template<typename T>
class GlobalSharedPtrValueSaveObject :public IGlobalValueSaveObject {
public:
	GlobalSharedPtrValueSaveObject(std::shared_ptr<T> arg_value) {
		data = arg_value;
	}
	GlobalSharedPtrValueSaveObject() {
	}
	void RestoreValue(IValueData** arg_v)const override;
	int GetTypeIndex()const override {
		return arg_type;
	}
	void SetTypeIndex(const int arg_index)override {
		arg_type = arg_index;
	}
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(data);
		archive(arg_type);
	}
private:
	std::shared_ptr<T> data;
	int arg_type;
};

#endif //IMPL_BUTIENGINE
class IValueData {
public:
	IValueData(const int arg_ref) {
		ref_ = arg_ref;
	}

	virtual ~IValueData() {
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
	template <> void Set(const IValueData& arg_v) {
		arg_v.ValueCopy(this);
	}


	IValueData* GetMember(const int arg_index)const {
		return ary_p_member[arg_index];
	}
	int GetMemberType(const int arg_index)const {
		return ary_memberType[arg_index];
	}

	void SetMember(IValueData* arg_v,const int arg_index) {
		if (ary_p_member[arg_index]) {
			ary_p_member[arg_index]->release();
		}
		ary_p_member[arg_index] = arg_v;
		ary_p_member[arg_index]->addref();
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
	virtual bool Eq(IValueData* p_other)const = 0;
	virtual bool Gt(IValueData* p_other)const = 0;
	virtual bool Ge(IValueData* p_other)const = 0;

	//値のコピー
	virtual void ValueCopy(IValueData* p_other) const = 0;

	//変数そのもののコピー
	virtual IValueData* Clone()const = 0;

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
#ifdef IMPL_BUTIENGINE
	virtual std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const = 0;
	virtual void ShowGUI(const std::string& arg_label) = 0;
#endif // IMPL_BUTIENGINE

protected:
	template <typename Class, typename Super = IValueData>
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
			RegistGet(ButiEngine::Matrix4x4);
			RegistGet(Type_Null);
			RegistGet(Type_Enum);
			RegistGet(Type_Func);
		};
	template <typename Class, typename Super = IValueData>
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
			RegistGetRef(ButiEngine::Matrix4x4);
			RegistGetRef(Type_Null);
			RegistGetRef(Type_Enum);
			RegistGetRef(Type_Func);
		};
	template <typename Class, typename Super = IValueData>
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
			RegistSetRef(ButiEngine::Matrix4x4);
			RegistSetRef(Type_Null);
			RegistSetRef(Type_Enum);
			RegistSetRef(Type_Func);
		};
	template <typename Class, typename Super = IValueData>
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
			RegistEq(ButiEngine::Matrix4x4);
			RegistEq(Type_Null);
			RegistEq(Type_Enum);
			RegistEq(Type_Func);

		};
	template <typename Class, typename Super = IValueData>
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
			RegistGt(ButiEngine::Matrix4x4);
			RegistGt(Type_Null);
			RegistGt(Type_Enum);
			RegistGt(Type_Func);
		};
	template <typename Class, typename Super = IValueData>
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
			RegistGe(ButiEngine::Matrix4x4);
			RegistGe(Type_Null);
			RegistGe(Type_Enum);
			RegistGe(Type_Func);

		};


	struct empty_class { struct get_value_base_t {}; struct get_ref_base_t {}; struct set_value_base_t {}; struct eq_base_t {}; struct gt_base_t {}; struct ge_base_t {}; };
	using get_value_base_t=get_value_t<const IValueData, empty_class> ;
	using get_ref_base_t=get_ref_t<IValueData, empty_class> ;
	using set_value_base_t=set_value_t<IValueData, empty_class> ;
	using eq_base_t=eq_t<const IValueData, empty_class> ;
	using gt_base_t=gt_t<const IValueData, empty_class> ;
	using ge_base_t=ge_t<const IValueData, empty_class> ;
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
	IValueData** ary_p_member=nullptr;
	//メンバ変数の型
	int* ary_memberType=nullptr;
	bool isOuterMemoryRef = false;



private:
	int ref_ = 0;
};


template <typename T>
class ValueData : public IValueData {
	public:
		ValueData(const T arg_v, const int arg_ref) :IValueData(arg_ref) {
			p_instance = new T(arg_v);
		}
		ValueData(T* arg_v, const int arg_ref) :IValueData(arg_ref) {
			p_instance = arg_v;
			isOuterMemoryRef = true;
		}
		ValueData(const int arg_ref) :IValueData(arg_ref) {
			p_instance = new T();
		}
		~ValueData() {
			if (!isOuterMemoryRef) {
				delete ((T*)p_instance);
			}
			if (ary_memberType) {
				delete ary_memberType;
			}
		}
		IValueData* Clone()const override {
			return new ValueData<T>(*(T*)p_instance, 1);
		}

		bool Eq(IValueData* p_other)const override {
			return p_other->Equal(*(T*)p_instance);
		}
		bool Gt(IValueData* p_other)const override {
			return !p_other->GreaterThan(*(T*)p_instance);
		}
		bool Ge(IValueData* p_other)const override {
			return !p_other->GreaterEq(*(T*)p_instance);
		}

		void ValueCopy(IValueData* p_other) const override {
			p_other->Set<T>(*(T*)p_instance);
		}


		void Nagative()override {
			//単項マイナスはない
			assert(0);

		}


		virtual const IValueData::get_value_base_t& get_value() const {
			static const IValueData::get_value_t<ValueData<T>> s;
			return s;
		}
		virtual const IValueData::get_ref_base_t& get_ref() const {
			static const IValueData::get_ref_t<ValueData<T>> s;
			return s;
		}
		virtual const IValueData::set_value_base_t& set_value() const {
			static const IValueData::set_value_t<ValueData<T>> s;
			return s;
		}

		virtual const IValueData::eq_base_t& eq() const {
			static const IValueData::eq_t<ValueData<T>> s;
			return s;
		}
		virtual const IValueData::gt_base_t& gt() const {
			static const IValueData::gt_t<ValueData<T>> s;
			return s;
		}
		virtual const IValueData::ge_base_t& ge() const {
			static const IValueData::ge_t<ValueData<T>> s;
			return s;
		}

		template <typename U> U get_value_stub()const {
			return U();
		}
		template <typename U> U& get_ref_stub() {
			auto v = U();
			return v;
		}
		template <> T get_value_stub()const {
			return (*(T*)p_instance);
		}
		template <> std::string get_value_stub()const {
			return std::to_string(*(T*)p_instance);
		}
		template <> T& get_ref_stub() {
			return *(T*)p_instance;
		}
		template <typename U> void set_value_stub(const U& arg_v) {

		}
		template <>void set_value_stub(const T& arg_v) {
			*(T*)p_instance = (arg_v);
		}


		template <typename U> bool eq_stub(const U& arg_v)const {
			return  false;
		}

		template <>bool eq_stub(const T& arg_v)const {
			return (T*)p_instance == &arg_v;
		}

#ifdef IMPL_BUTIENGINE
		std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
			return std::make_shared<GlobalValueSaveObject<T>>(*(T*)p_instance);
		}
		void ShowGUI(const std::string& arg_label) override {
			ButiEngine::GUI::Text("GUIで操作できない型です");
		}
#endif // IMPL_BUTIENGINE
	private:
	};

template <>
class ValueData<ScriptClassInfo> :public IValueData {
public:
	ValueData(ScriptClassInfo* arg_v,std::vector<IValueData*> arg_vec_member, const int arg_ref) :IValueData(arg_ref) {
		p_instance = arg_v; 
		int memberSize = arg_vec_member.size();
		ary_p_member = (IValueData**)malloc(sizeof(IValueData*) * memberSize);
		ary_memberType = (int*)malloc(sizeof(int) * memberSize);
		for (int i = 0; i < memberSize; i++) {
			ary_p_member[i] = arg_vec_member[i];
			ary_memberType[i] = arg_v->GetMemberTypeIndex(i);
		}

	}
	~ValueData() override {
		if (ary_memberType) {
			delete ary_memberType;
		}
		auto memberSize = ((ScriptClassInfo*)p_instance)->GetMemberSize();
		for (int i = 0; i < memberSize; i++) {
			ary_p_member[i]->release();
		}
		if (ary_p_member) {
			delete ary_p_member;
		}
	}


	bool Eq(IValueData* p_other)const override {
		return false;
	}
	bool Gt(IValueData* p_other)const override {
		return false;
	}
	bool Ge(IValueData* p_other)const override {
		return false;
	}

	void ValueCopy(IValueData* p_other) const override {

		*p_other =* Clone();
	}

	IValueData* Clone()const {
		std::vector<IValueData*> vec_clonedMember;
		auto memberSize = ((ScriptClassInfo*)p_instance)->GetMemberSize();
		for (int i = 0; i < memberSize; i++) {
			if (ary_memberType[i] & TYPE_REF) {
				vec_clonedMember.push_back(ary_p_member[i]);
			}
			else {
				vec_clonedMember.push_back(ary_p_member[i]->Clone());
			}
		}

		return new ValueData<ScriptClassInfo>((ScriptClassInfo*)p_instance, vec_clonedMember,1);
	}

	void Nagative()override {
		//単項マイナスは未定義
		assert(0);

	}


	virtual const IValueData::get_value_base_t& get_value() const {
		static const IValueData::get_value_t<ValueData<ScriptClassInfo>> s;
		return s;
	}
	virtual const IValueData::get_ref_base_t& get_ref() const {
		static const IValueData::get_ref_t<ValueData<ScriptClassInfo>> s;
		return s;
	}
	virtual const IValueData::set_value_base_t& set_value() const {
		static const IValueData::set_value_t<ValueData<ScriptClassInfo>> s;
		return s;
	}

	virtual const IValueData::eq_base_t& eq() const {
		static const IValueData::eq_t<ValueData<ScriptClassInfo>> s;
		return s;
	}
	virtual const IValueData::gt_base_t& gt() const {
		static const IValueData::gt_t<ValueData<ScriptClassInfo>> s;
		return s;
	}
	virtual const IValueData::ge_base_t& ge() const {
		static const IValueData::ge_t<ValueData<ScriptClassInfo>> s;
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

#ifdef IMPL_BUTIENGINE
	std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
		auto ret = std::make_shared<GlobalScriptTypeValueSaveObject>();

		auto memberSize = ((ScriptClassInfo*)p_instance)->GetMemberSize();
		for (int i = 0; i < memberSize; i++) {
			ret->Push(ary_p_member[i]->GetSaveObject(), ary_memberType[i]);
			
		}
		return  ret;
	}

	void ShowGUI(const std::string& arg_label) override {
		if (ButiEngine::GUI::TreeNode(arg_label)) {
			auto memberSize = ((ScriptClassInfo*)p_instance)->GetMemberSize();
			auto memberNameItr = ((ScriptClassInfo*)p_instance)->GetMamberName().begin();
			for (int i = 0; i < memberSize; i++, memberNameItr++)
			{
				ary_p_member[i]->ShowGUI(*memberNameItr);
			}
			ButiEngine::GUI::TreePop();
		}
	}
#endif // IMPL_BUTIENGINE
};

template<>
class ValueData<int> :public IValueData {
public:
	ValueData(const int arg_v, const int arg_ref) :IValueData(arg_ref) {
		p_instance = new int(arg_v);
	}
	ValueData(const int arg_ref) :IValueData(arg_ref) {
		p_instance = new int();
	}
	~ValueData() override {
		if (!isOuterMemoryRef) {
			delete ((int*)p_instance);
		}
		if (ary_memberType) {
			delete ary_memberType;
		}
	}

	bool Eq(IValueData* p_other)const override {
		return p_other->Equal(*(int*)p_instance);
	}
	bool Gt(IValueData* p_other)const override {
		return !p_other->GreaterThan(*(int*)p_instance);
	}
	bool Ge(IValueData* p_other)const override {
		return !p_other->GreaterEq(*(int*)p_instance);
	}

	void ValueCopy(IValueData* p_other) const override {
		p_other->Set<int>(*(int*)p_instance);
	}

	IValueData* Clone()const {
		return new ValueData<int>(*(int*)p_instance, 1);
	}

	void Nagative()override {
		*(int*)p_instance = -1 * (*(int*)p_instance);
	}


	virtual const IValueData::get_value_base_t& get_value() const {
		static const IValueData::get_value_t<ValueData<int>> s;
		return s;
	}
	virtual const IValueData::get_ref_base_t& get_ref() const {
		static const IValueData::get_ref_t<ValueData<int>> s;
		return s;
	}
	virtual const IValueData::set_value_base_t& set_value() const {
		static const IValueData::set_value_t<ValueData<int>> s;
		return s;
	}

	virtual const IValueData::eq_base_t& eq() const {
		static const IValueData::eq_t<ValueData<int>> s;
		return s;
	}
	virtual const IValueData::gt_base_t& gt() const {
		static const IValueData::gt_t<ValueData<int>> s;
		return s;
	}
	virtual const IValueData::ge_base_t& ge() const {
		static const IValueData::ge_t<ValueData<int>> s;
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

#ifdef IMPL_BUTIENGINE
	std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
		return std::make_shared<GlobalValueSaveObject<int>>(*(int*)p_instance);
	}

	void ShowGUI(const std::string& arg_label) override {
		ButiEngine::GUI::Input(arg_label, *(int*)p_instance);
	}
#endif // IMPL_BUTIENGINE
};
template<>
class ValueData<Type_Func> :public IValueData {
public:
	ValueData(const int arg_v, const int arg_ref, std::map <  int,const std::string*> * arg_p_funcEntryTable) :IValueData(arg_ref), p_functionJumpTable(arg_p_funcEntryTable) {
		p_instance = new int(arg_v);
	}
	ValueData(const int arg_ref, std::map <  int, std::string*>* arg_p_funcTag) :IValueData(arg_ref) {
		p_instance = new int();
	}
	~ValueData() override {
		if (!isOuterMemoryRef) {
			delete ((int*)p_instance);
		}
		if (ary_memberType) {
			delete ary_memberType;
		}
	}

	bool Eq(IValueData* p_other)const override {
		return p_other->Equal(*(int*)p_instance);
	}
	bool Gt(IValueData* p_other)const override {
		return !p_other->GreaterThan(*(int*)p_instance);
	}
	bool Ge(IValueData* p_other)const override {
		return !p_other->GreaterEq(*(int*)p_instance);
	}

	void ValueCopy(IValueData* p_other) const override {
		p_other->Set<int>(*(int*)p_instance);
	}

	IValueData* Clone()const {
		return new ValueData<Type_Func>(*(int*)p_instance, 1, p_functionJumpTable);
	}

	void Nagative()override {
		assert(0 && "関数型に単項マイナスはありません");
		*(int*)p_instance = -1 * (*(int*)p_instance);
	}


	virtual const IValueData::get_value_base_t& get_value() const {
		static const IValueData::get_value_t<ValueData<Type_Func>> s;
		return s;
	}
	virtual const IValueData::get_ref_base_t& get_ref() const {
		static const IValueData::get_ref_t<ValueData<Type_Func>> s;
		return s;
	}
	virtual const IValueData::set_value_base_t& set_value() const {
		static const IValueData::set_value_t<ValueData<Type_Func>> s;
		return s;
	}

	virtual const IValueData::eq_base_t& eq() const {
		static const IValueData::eq_t<ValueData<Type_Func>> s;
		return s;
	}
	virtual const IValueData::gt_base_t& gt() const {
		static const IValueData::gt_t<ValueData<Type_Func>> s;
		return s;
	}
	virtual const IValueData::ge_base_t& ge() const {
		static const IValueData::ge_t<ValueData<Type_Func>> s;
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

#ifdef IMPL_BUTIENGINE
	std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
		return std::make_shared<GlobalValueSaveObject<Type_Enum>>(*(int*)p_instance);
	}

	void ShowGUI(const std::string& arg_label) override {
		//ButiEngine::GUI::Input(arg_label, *(int*)p_instance, p_enumTag->GetIdentiferMap());
	}
#endif // IMPL_BUTIENGINE

	std::map <  int,const std::string*>* p_functionJumpTable;
};

template<>
class ValueData<Type_Enum> :public IValueData {
public:
	ValueData(const int arg_v, const int arg_ref, EnumTag* arg_p_enumTag) :IValueData(arg_ref), p_enumTag(arg_p_enumTag) {
		p_instance = new int(arg_v);
	}
	ValueData(const int arg_ref, EnumTag* arg_p_enumTag) :IValueData(arg_ref) {
		p_instance = new int();
	}
	~ValueData() override {
		if (!isOuterMemoryRef) {
			delete ((int*)p_instance);
		}
		if (ary_memberType) {
			delete ary_memberType;
		}
	}

	bool Eq(IValueData* p_other)const override {
		return p_other->Equal(*(int*)p_instance);
	}
	bool Gt(IValueData* p_other)const override {
		return !p_other->GreaterThan(*(int*)p_instance);
	}
	bool Ge(IValueData* p_other)const override {
		return !p_other->GreaterEq(*(int*)p_instance);
	}

	void ValueCopy(IValueData* p_other) const override {
		p_other->Set<int>(*(int*)p_instance);
	}

	IValueData* Clone()const {
		return new ValueData<Type_Enum>(*(int*)p_instance, 1, p_enumTag);
	}

	void Nagative()override {
		assert(0 && "列挙型に単項マイナスはありません");
		*(int*)p_instance = -1 * (*(int*)p_instance);
	}


	virtual const IValueData::get_value_base_t& get_value() const {
		static const IValueData::get_value_t<ValueData<Type_Enum>> s;
		return s;
	}
	virtual const IValueData::get_ref_base_t& get_ref() const {
		static const IValueData::get_ref_t<ValueData<Type_Enum>> s;
		return s;
	}
	virtual const IValueData::set_value_base_t& set_value() const {
		static const IValueData::set_value_t<ValueData<Type_Enum>> s;
		return s;
	}

	virtual const IValueData::eq_base_t& eq() const {
		static const IValueData::eq_t<ValueData<Type_Enum>> s;
		return s;
	}
	virtual const IValueData::gt_base_t& gt() const {
		static const IValueData::gt_t<ValueData<Type_Enum>> s;
		return s;
	}
	virtual const IValueData::ge_base_t& ge() const {
		static const IValueData::ge_t<ValueData<Type_Enum>> s;
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

#ifdef IMPL_BUTIENGINE
	std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
		return std::make_shared<GlobalValueSaveObject<Type_Enum>>(*(int*)p_instance);
	}

	void ShowGUI(const std::string& arg_label) override {
		ButiEngine::GUI::Input(arg_label, *(int*)p_instance, p_enumTag->GetIdentiferMap());
	}
#endif // IMPL_BUTIENGINE

	EnumTag* p_enumTag;
};

template<>
class ValueData<float> :public IValueData {
	public:
		ValueData(const float arg_v, const int arg_ref) :IValueData(arg_ref) {
			p_instance = new float(arg_v);
		}
		ValueData(const float arg_ref) :IValueData(arg_ref) {
			p_instance = new float();
		}
		ValueData(float* arg_v, const int arg_ref,const bool arg_isOuterMemoryRef) :IValueData(arg_ref) {
			p_instance = arg_v;
			isOuterMemoryRef = arg_isOuterMemoryRef;
		}
		~ValueData() override {
			if (!isOuterMemoryRef) {
				delete ((float*)p_instance);
			}
			if (ary_memberType) {
				delete ary_memberType;
			}
		}

		bool Eq(IValueData* p_other)const override {
			return p_other->Equal(*(float*)p_instance);
		}
		bool Gt(IValueData* p_other)const override {
			return !p_other->GreaterThan(*(float*)p_instance);
		}
		bool Ge(IValueData* p_other)const override {
			return !p_other->GreaterEq(*(float*)p_instance);
		}

		void ValueCopy(IValueData* p_other) const override {
			p_other->Set<float>(*(float*)p_instance);
		}

		IValueData* Clone()const {
			return new ValueData<float>(*(float*)p_instance, 1);
		}

		void Nagative()override {
			*(float*)p_instance = -1 * (*(float*)p_instance);
		}

		virtual const IValueData::get_value_base_t& get_value() const {
			static const IValueData::get_value_t<ValueData<float>> s;
			return s;
		}
		virtual const IValueData::get_ref_base_t& get_ref() const {
			static const IValueData::get_ref_t<ValueData<float>> s;
			return s;
		}
		virtual const IValueData::set_value_base_t& set_value() const {
			static const IValueData::set_value_t<ValueData<float>> s;
			return s;
		}

		virtual const IValueData::eq_base_t& eq() const {
			static const IValueData::eq_t<ValueData<float>> s;
			return s;
		}
		virtual const IValueData::gt_base_t& gt() const {
			static const IValueData::gt_t<ValueData<float>> s;
			return s;
		}
		virtual const IValueData::ge_base_t& ge() const {
			static const IValueData::ge_t<ValueData<float>> s;
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

#ifdef IMPL_BUTIENGINE
		std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
			return std::make_shared<GlobalValueSaveObject<float>>(*(float*)p_instance);
		}
		void ShowGUI(const std::string& arg_label) override {
			ButiEngine::GUI::Input(arg_label,*(float*)p_instance);
		}
#endif // IMPL_BUTIENGINE
	};

template<>
class ValueData<Type_Null> : public IValueData {
	public:
		ValueData(const Type_Null& arg_v, const int arg_ref) :IValueData(arg_ref) {
			p_instance = new Type_Null(arg_v);
			assert(0);
			//void型のオブジェクトが生成されています
		}
		ValueData(const int arg_ref) :IValueData(arg_ref) {
			p_instance = new Type_Null();
			//void型のオブジェクトが生成されています
		}
		~ValueData() override{
			delete ((Type_Null*)p_instance);
		}

		bool Eq(IValueData* p_other)const override {
			return p_other->Equal(*(Type_Null*)p_instance);
		}
		bool Gt(IValueData* p_other)const override {
			return !p_other->GreaterThan(*(Type_Null*)p_instance);
		}
		bool Ge(IValueData* p_other)const override {
			return !p_other->GreaterEq(*(Type_Null*)p_instance);
		}

		void ValueCopy(IValueData* p_other) const override {
			p_other->Set<Type_Null>(*(Type_Null*)p_instance);
		}

		IValueData* Clone()const {
			return new ValueData<Type_Null>(*(Type_Null*)p_instance, 1);
		}

		void Nagative()override {
			//単項マイナスはない
			assert(0);

		}


		virtual const IValueData::get_value_base_t& get_value() const {
			static const IValueData::get_value_t<ValueData<Type_Null>> s;
			return s;
		}
		virtual const IValueData::get_ref_base_t& get_ref() const {
			static const IValueData::get_ref_t<ValueData<Type_Null>> s;
			return s;
		}
		virtual const IValueData::set_value_base_t& set_value() const {
			static const IValueData::set_value_t<ValueData<Type_Null>> s;
			return s;
		}

		virtual const IValueData::eq_base_t& eq() const {
			static const IValueData::eq_t<ValueData<Type_Null>> s;
			return s;
		}
		virtual const IValueData::gt_base_t& gt() const {
			static const IValueData::gt_t<ValueData<Type_Null>> s;
			return s;
		}
		virtual const IValueData::ge_base_t& ge() const {
			static const IValueData::ge_t<ValueData<Type_Null>> s;
			return s;
		}

		template <typename U> U get_value_stub()const {
			return U();
		}
		template <typename U> U& get_ref_stub() {
			auto v = U();
			return v;
		}
		template <typename U> void set_value_stub(const U& arg_v) {
		}


		template <typename U> bool eq_stub(const U& arg_v)const {
			return  false;
		}

#ifdef IMPL_BUTIENGINE
		std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
			assert(0);
			//参照型の保存は未定義
			return nullptr;
		}

		void ShowGUI(const std::string& arg_label)override {
			ButiEngine::GUI::Text("GUIで操作できない型です");
		}
#endif // IMPL_BUTIENGINE

	};

template<>
class ValueData<std::string> : public IValueData {
public:
	ValueData(const std::string& arg_v, const int arg_ref) :IValueData(arg_ref) {
		p_instance = new std::string(arg_v);
	}
	ValueData(const int arg_ref) :IValueData(arg_ref) {
		p_instance = new std::string();
	}
	~ValueData() override {
		delete ((std::string*)p_instance);
	}

	bool Eq(IValueData* p_other)const override {
		return p_other->Equal(*(std::string*)p_instance);
	}
	bool Gt(IValueData* p_other)const override {
		return !p_other->GreaterThan(*(std::string*)p_instance);
	}
	bool Ge(IValueData* p_other)const override {
		return !p_other->GreaterEq(*(std::string*)p_instance);
	}

	void ValueCopy(IValueData* p_other) const override {
		p_other->Set<std::string>(*(std::string*)p_instance);
	}

	IValueData* Clone()const {
		return new ValueData<std::string>(*(std::string*)p_instance, 1);
	}

	void Nagative()override {
		//文字列に単項マイナスはない
		assert(0);

	}


	virtual const IValueData::get_value_base_t& get_value() const {
		static const IValueData::get_value_t<ValueData<std::string>> s;
		return s;
	}
	virtual const IValueData::get_ref_base_t& get_ref() const {
		static const IValueData::get_ref_t<ValueData<std::string>> s;
		return s;
	}
	virtual const IValueData::set_value_base_t& set_value() const {
		static const IValueData::set_value_t<ValueData<std::string>> s;
		return s;
	}

	virtual const IValueData::eq_base_t& eq() const {
		static const IValueData::eq_t<ValueData<std::string>> s;
		return s;
	}
	virtual const IValueData::gt_base_t& gt() const {
		static const IValueData::gt_t<ValueData<std::string>> s;
		return s;
	}
	virtual const IValueData::ge_base_t& ge() const {
		static const IValueData::ge_t<ValueData<std::string>> s;
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

#ifdef IMPL_BUTIENGINE
	std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
		return std::make_shared<GlobalValueSaveObject<std::string>>(*(std::string*)p_instance);
	}
	void ShowGUI(const std::string& arg_label)override {
		ButiEngine::GUI::Input(arg_label, *(std::string*)p_instance);
	}
#endif // IMPL_BUTIENGINE
};
template<>
class ValueData<ButiEngine::Vector2> : public IValueData {
	public:
		ValueData(const ButiEngine::Vector2& arg_v, const int arg_ref) :IValueData(arg_ref) {
			p_instance = new ButiEngine::Vector2(arg_v);
			ary_p_member = (IValueData**)malloc(sizeof(IValueData*) * sizeof(ButiEngine::Vector2) / sizeof(float));
			ary_memberType = (int*)malloc(sizeof(int) * sizeof(ButiEngine::Vector2) / sizeof(float));
			ary_p_member[0] = new ValueData<float>(&((ButiEngine::Vector2*)p_instance)->x, 1,true);
			ary_p_member[1] = new ValueData<float>(&((ButiEngine::Vector2*)p_instance)->y, 1, true);
			ary_memberType[0] = TYPE_FLOAT;
			ary_memberType[1] = TYPE_FLOAT;
		}
		ValueData(const int arg_ref) :IValueData(arg_ref) {
			p_instance = new ButiEngine::Vector2();
			ary_p_member = (IValueData**)malloc(sizeof(IValueData*) * sizeof(ButiEngine::Vector2) / sizeof(float));
			ary_memberType = (int*)malloc(sizeof(int) * sizeof(ButiEngine::Vector2) / sizeof(float));
			ary_p_member[0] = new ValueData<float>(&((ButiEngine::Vector2*)p_instance)->x, 1, true);
			ary_p_member[1] = new ValueData<float>(&((ButiEngine::Vector2*)p_instance)->y, 1, true);
			ary_memberType[0] = TYPE_FLOAT;
			ary_memberType[1] = TYPE_FLOAT;
		}

		~ValueData() {
			for (int i = 0; i < sizeof(ButiEngine::Vector2) / sizeof(float); i++) {
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
#ifdef IMPL_BUTIENGINE
		std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
			return std::make_shared<GlobalValueSaveObject<ButiEngine::Vector2>>(*(ButiEngine::Vector2*)p_instance);
		}

		void ShowGUI(const std::string& arg_label)override {
			ButiEngine::GUI::Input(arg_label, *(ButiEngine::Vector2*)p_instance);
		}
#endif // IMPL_BUTIENGINE
	};
template<>
class ValueData<ButiEngine::Vector3> : public IValueData {
	public:
		ValueData(const ButiEngine::Vector3& arg_v, const int arg_ref) :IValueData(arg_ref) {
			p_instance = new ButiEngine::Vector3(arg_v);

			ary_p_member = (IValueData**)malloc(sizeof(IValueData*) * sizeof(ButiEngine::Vector3) / sizeof(float));
			ary_memberType = (int*)malloc(sizeof(int) * sizeof(ButiEngine::Vector3) / sizeof(float));

			ary_p_member[0] = new ValueData<float>(&((ButiEngine::Vector3*)p_instance)->x, 1, true);
			ary_p_member[1] = new ValueData<float>(&((ButiEngine::Vector3*)p_instance)->y, 1, true);
			ary_p_member[2] = new ValueData<float>(&((ButiEngine::Vector3*)p_instance)->z, 1, true);
			ary_memberType[0] = TYPE_FLOAT;
			ary_memberType[1] = TYPE_FLOAT;
			ary_memberType[2] = TYPE_FLOAT;
		}
		ValueData(const int arg_ref) :IValueData(arg_ref) {
			p_instance = new ButiEngine::Vector3();
			ary_p_member = (IValueData**)malloc(sizeof(IValueData*) * sizeof(ButiEngine::Vector3) / sizeof(float));
			ary_memberType = (int*)malloc(sizeof(int) * sizeof(ButiEngine::Vector3) / sizeof(float));
			ary_p_member[0] = new ValueData<float>(&((ButiEngine::Vector3*)p_instance)->x, 1, true);
			ary_p_member[1] = new ValueData<float>(&((ButiEngine::Vector3*)p_instance)->y, 1,true);
			ary_p_member[2] = new ValueData<float>(&((ButiEngine::Vector3*)p_instance)->z, 1,true);
			ary_memberType[0] = TYPE_FLOAT;
			ary_memberType[1] = TYPE_FLOAT;
			ary_memberType[2] = TYPE_FLOAT;
		}
		~ValueData() {
			for (int i = 0; i < sizeof(ButiEngine::Vector3) / sizeof(float); i++) {
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

#ifdef IMPL_BUTIENGINE
		std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
			return std::make_shared<GlobalValueSaveObject<ButiEngine::Vector3>>(*(ButiEngine::Vector3*)p_instance);
		}
		void ShowGUI(const std::string& arg_label)override {
			ButiEngine::GUI::Input(arg_label,*(ButiEngine::Vector3*)p_instance);
		}
#endif // IMPL_BUTIENGINE
	};
template<>
class ValueData<ButiEngine::Vector4> : public IValueData {
public:
	ValueData(const ButiEngine::Vector4& arg_v, const int arg_ref) :IValueData(arg_ref) {
		p_instance = new ButiEngine::Vector4(arg_v);
		ary_p_member = (IValueData**)malloc(sizeof(IValueData*) * sizeof(ButiEngine::Vector4) / sizeof(float));
		ary_memberType = (int*)malloc(sizeof(int) * sizeof(ButiEngine::Vector4) / sizeof(float));
		ary_p_member[0] = new ValueData<float>(&((ButiEngine::Vector4*)p_instance)->x, 1, true);
		ary_p_member[1] = new ValueData<float>(&((ButiEngine::Vector4*)p_instance)->y, 1, true);
		ary_p_member[2] = new ValueData<float>(&((ButiEngine::Vector4*)p_instance)->z, 1, true);
		ary_p_member[3] = new ValueData<float>(&((ButiEngine::Vector4*)p_instance)->w, 1, true);
		ary_memberType[0] = TYPE_FLOAT;
		ary_memberType[1] = TYPE_FLOAT;
		ary_memberType[2] = TYPE_FLOAT;
		ary_memberType[3] = TYPE_FLOAT;
	}
	ValueData(const int arg_ref) :IValueData(arg_ref) {
		p_instance = new ButiEngine::Vector4();
		ary_p_member = (IValueData**)malloc(sizeof(IValueData*) * sizeof(ButiEngine::Vector4) / sizeof(float));
		ary_memberType = (int*)malloc(sizeof(int) * sizeof(ButiEngine::Vector4) / sizeof(float));
		ary_p_member[0] = new ValueData<float>(&((ButiEngine::Vector4*)p_instance)->x, 1, true);
		ary_p_member[1] = new ValueData<float>(&((ButiEngine::Vector4*)p_instance)->y, 1, true);
		ary_p_member[2] = new ValueData<float>(&((ButiEngine::Vector4*)p_instance)->z, 1, true);
		ary_p_member[3] = new ValueData<float>(&((ButiEngine::Vector4*)p_instance)->w, 1, true);
		ary_memberType[0] = TYPE_FLOAT;
		ary_memberType[1] = TYPE_FLOAT;
		ary_memberType[2] = TYPE_FLOAT;
		ary_memberType[3] = TYPE_FLOAT;
	}
	~ValueData() {
		for (int i = 0; i < sizeof(ButiEngine::Vector4) / sizeof(float); i++) {
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

#ifdef IMPL_BUTIENGINE
	std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
		return std::make_shared<GlobalValueSaveObject<ButiEngine::Vector4>>(*(ButiEngine::Vector4*)p_instance);
	}

	void ShowGUI(const std::string& arg_label)override {
		ButiEngine::GUI::Input(arg_label, *(ButiEngine::Vector4*)p_instance);
	}
#endif // IMPL_BUTIENGINE
};

template<>
class ValueData<ButiEngine::Matrix4x4> : public IValueData {
public:
	ValueData(const ButiEngine::Matrix4x4& arg_v, const int arg_ref) :IValueData(arg_ref) {
		p_instance = new ButiEngine::Matrix4x4(arg_v);
		ary_p_member = (IValueData**)malloc(sizeof(IValueData*) * sizeof(ButiEngine::Matrix4x4) / sizeof(float));
		ary_memberType = (int*)malloc(sizeof(int) * sizeof(ButiEngine::Matrix4x4) / sizeof(float));
		for (int i = 0; i < sizeof(ButiEngine::Matrix4x4) / sizeof(float); i++) {
			ary_p_member[i] = new ValueData<float>(&((ButiEngine::Matrix4x4*)p_instance)->m[i/4][i%4], 1, true);
			ary_memberType[i] = TYPE_FLOAT;
		}
	}
	ValueData(const int arg_ref) :IValueData(arg_ref) {
		p_instance = new ButiEngine::Matrix4x4();
		ary_p_member = (IValueData**)malloc(sizeof(IValueData*) * sizeof(ButiEngine::Matrix4x4) / sizeof(float));
		ary_memberType = (int*)malloc(sizeof(int) * sizeof(ButiEngine::Matrix4x4) / sizeof(float));
		for (int i = 0; i < sizeof(ButiEngine::Matrix4x4) / sizeof(float); i++) {
			ary_p_member[i] = new ValueData<float>(&((ButiEngine::Matrix4x4*)p_instance)->m[i / 4][i % 4], 1, true);
			ary_memberType[i] = TYPE_FLOAT;
		}
	}
	~ValueData() {
		for (int i = 0; i < sizeof(ButiEngine::Matrix4x4)/sizeof(float); i++) {
			ary_p_member[i]->release();
		}

		delete ((ButiEngine::Matrix4x4*)p_instance);
		if (ary_memberType) {
			free(ary_memberType);
		}
		if (ary_p_member) {
			free(ary_p_member);
		}
	}

	bool Eq(IValueData* p_other)const override {
		
			return p_other->Equal(*(ButiEngine::Matrix4x4*)p_instance); 
	}
	bool Gt(IValueData* p_other)const override {

		return false;
	}
	bool Ge(IValueData* p_other)const override {

		return false;
	}
	void ValueCopy(IValueData* p_other) const override {

		p_other->Set< ButiEngine::Matrix4x4 >(*(ButiEngine::Matrix4x4*)p_instance);
	}
	IValueData* Clone()const {

		return new ValueData< ButiEngine::Matrix4x4 >(*(ButiEngine::Matrix4x4*)p_instance, 1);
	}
	void Nagative()override {

		*(ButiEngine::Matrix4x4*)p_instance = - (*(ButiEngine::Matrix4x4*)p_instance);
	}
	virtual const IValueData::get_value_base_t& get_value() const {

		static const IValueData::get_value_t<ValueData< ButiEngine::Matrix4x4 >> s;
		return s;
	}
	virtual const IValueData::get_ref_base_t& get_ref() const {

		static const IValueData::get_ref_t<ValueData< ButiEngine::Matrix4x4 >> s;
		return s;
	}
	virtual const IValueData::set_value_base_t& set_value() const {

		static const IValueData::set_value_t<ValueData< ButiEngine::Matrix4x4 >> s;
		return s;
	}
	virtual const IValueData::eq_base_t& eq() const {

		static const IValueData::eq_t<ValueData< ButiEngine::Matrix4x4 >> s;
		return s;
	}
	virtual const IValueData::gt_base_t& gt() const {

		static const IValueData::gt_t<ValueData< ButiEngine::Matrix4x4 >> s;
		return s;
	}
	virtual const IValueData::ge_base_t& ge() const {

		static const IValueData::ge_t<ValueData< ButiEngine::Matrix4x4 >> s;
		return s;
	}
	template <typename U> U get_value_stub()const {

		return U();
	}
	template <typename U> U& get_ref_stub() {

		auto v = U();
		return v;
	}
	template <>  ButiEngine::Matrix4x4& get_ref_stub() {

		return *(ButiEngine::Matrix4x4*)p_instance;
	}
	template <>  ButiEngine::Matrix4x4  get_value_stub()const {

		return (*(ButiEngine::Matrix4x4*)p_instance);
	}
	template <> std::string get_value_stub()const {

		return std::to_string(*(ButiEngine::Matrix4x4*)p_instance);
	}
	template <typename U> void set_value_stub(const U& arg_v) {

	}
	template <>void set_value_stub(const  ButiEngine::Matrix4x4& arg_v) {

		*(ButiEngine::Matrix4x4*)p_instance = (arg_v);
	}
	template <>void set_value_stub(const std::string& arg_v) {

		*(ButiEngine::Matrix4x4*)p_instance = StrConvert::ConvertString< ButiEngine::Matrix4x4 >(arg_v);
	}
	template <typename U> bool eq_stub(const U& arg_v)const {

		return  false;
	}
	template <>bool eq_stub(const  ButiEngine::Matrix4x4& arg_v)const {

		return *(ButiEngine::Matrix4x4*)p_instance == arg_v;
	}

#ifdef IMPL_BUTIENGINE
	std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
		return std::make_shared<GlobalValueSaveObject<ButiEngine::Matrix4x4>>(*(ButiEngine::Matrix4x4*)p_instance);
	}

	void ShowGUI(const std::string& arg_label)override {
		ButiEngine::GUI::Input(arg_label, *(ButiEngine::Matrix4x4*)p_instance);
	}
#endif // IMPL_BUTIENGINE
};

template<typename T>
class Value_Shared :public IValueData {
public:
	Value_Shared(const int arg_ref) :IValueData(arg_ref) {
		int i = 0;
	}
	Value_Shared(std::shared_ptr<T> arg_instance, const int arg_ref) :IValueData(arg_ref) {
		shp = arg_instance;
	}

	std::shared_ptr<T> Get() {
		if (!shp) {
			std::cout<<"インスタンスの生成されていないオブジェクトの関数を呼び出そうとしています" << std::endl;
		}
		return shp;
	}
	void Set(std::shared_ptr<T> arg_shp) {
		shp = arg_shp;
	}

	//比較
	bool Eq(IValueData* p_other)const {return ((Value_Shared<T>*)p_other)->shp == shp;}
	bool Gt(IValueData* p_other)const {return false;}
	bool Ge(IValueData* p_other)const {return false;}

	//値のコピー
	void ValueCopy(IValueData* p_other) const {
		((Value_Shared<T>*)p_other)->shp = shp;
	}

	//変数そのもののコピー
	IValueData* Clone()const {
		return new Value_Shared(shp, 1);
	}

	//単項マイナス
	void Nagative() {}
	const get_value_base_t& get_value() const {
		static const IValueData::get_value_t<ValueData<int>> s;
		return s;

	}
	const get_ref_base_t& get_ref() const {
		static const IValueData::get_ref_t<ValueData<int>> s;
		return s;
	}
	const set_value_base_t& set_value() const {
		static const IValueData::set_value_t<ValueData<int>> s;
		return s;
	}
	const eq_base_t& eq() const {
		static const IValueData::eq_t<ValueData<int>> s;
		return s;
	}
	const gt_base_t& gt() const {
		static const IValueData::gt_t<ValueData<int>> s;
		return s;
	}
	const ge_base_t& ge() const {
		static const IValueData::ge_t<ValueData<int>> s;
		return s;
	}
#ifdef IMPL_BUTIENGINE
	std::shared_ptr<ButiScript::IGlobalValueSaveObject> GetSaveObject() const override {
		return std::make_shared<ButiScript::GlobalSharedPtrValueSaveObject<T>>(shp);
	}

	void ShowGUI(const std::string& arg_label)override {
		ButiEngine::GUI::Text("GUIで操作できない型です");
	}
#endif // IMPL_BUTIENGINE
private:
	std::shared_ptr<T> shp=nullptr;
};


template<typename T>
IValueData* CreateMemberInstance() {
	return new ValueData<T>(1);
}
template<typename T>
IValueData* CreateSharedMemberInstance() {
	return new Value_Shared<T>(1);
}


using CreateMemberInstanceFunction = IValueData* (*)();

template<typename T>
void PushCreateMemberInstance() {
	PushCreateMemberInstance(CreateMemberInstance<T>);
}
template<typename T>
void PushCreateSharedMemberInstance() {
	PushCreateMemberInstance(CreateSharedMemberInstance<T>);
}
void PushCreateMemberInstance(CreateMemberInstanceFunction arg_function);

// 変数
class Value {

public:
	Value()
	{
		valueData = nullptr;
		valueType = TYPE_VOID;
	}

	template<typename T>
	Value(const T arg_v) {
		valueData = new ValueData<T>(arg_v, 1);
		valueType = TYPE_VOID;
	}
	template<typename T>
	Value(std::shared_ptr<T> arg_instance) {
		valueData = new Value_Shared<T>(arg_instance, 1);
		valueType = TYPE_VOID;
	}

	Value(const Type_Null) {
		valueData = nullptr;
		valueType = TYPE_VOID;
	}
	Value(const Type_Enum,EnumTag* arg_enumTag) {
		valueData = new ValueData<Type_Enum>(0, 1,arg_enumTag);
		valueType = TYPE_VOID;
	}
	Value(const Type_Func, std::map<int,const std::string*>* arg_entryPointTable) {
		valueData = new ValueData<Type_Func>(0, 1, arg_entryPointTable);
		valueType = TYPE_VOID;
	}

	//ユーザー定義型として初期化
	Value(ScriptClassInfo& arg_info, std::vector<ButiScript::ScriptClassInfo>* arg_p_vec_scriptClassInfo);

	//変数を指定して初期化
	Value(IValueData* arg_p_data,const int arg_type)
	{
		valueData = arg_p_data;
		valueData->addref();
		valueType = arg_type;
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

	Value& operator=(const Value& other)
	{
		if (this == &other)
			return *this;


		Assign(other);

		return *this;
	}

	static int SetTypeIndex(long long int arg_typeFunc);
	static int GetTypeIndex(long long int arg_typeFunc);

	void clear()
	{
		if (valueData) {
			valueData->release();
			valueData = nullptr;
		}
	}

	template <typename T>
	void SetSharedType() {
		valueData = new Value_Shared<T>(1);
	}

	Value Clone() {
		auto cloneV = valueData->Clone();
		auto output = Value(cloneV, valueType);
		output.valueData->release();
		return output;
	}

	void Copy(const Value& other)
	{
		valueType = other.valueType;
		valueData = other.valueData->Clone();
	}
	void Assign(const Value& other) {
		if (!other.valueData) {

			valueType = other.valueType;
			return;
		}

		if (valueData) {
			valueData->Set(*other.valueData);
		}
		else {
			//自分が参照型の場合、相手の実体への参照を取得
			if (valueType & TYPE_REF || valueType == TYPE_VOID) {

				clear();

				valueData = other.valueData;

				valueData->addref();

				if (valueType == TYPE_VOID) {
					valueType = other.valueType;
				}
			}
			else {
				valueData = other.valueData->Clone();
			}
		}

		


	}


	void SetType(const int arg_type) {
		valueType = arg_type;
	}
	IValueData* valueData = nullptr;
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
template< typename T, int Size >
class Stack {
public:
	Stack() : currentSize(0)
	{
	}

	~Stack()
	{
		resize(0);
	}

	void push(const T& arg_value)
	{
		if (Size <= currentSize) {
			throw StackOverflow();
		}
		*(::new(data_[currentSize++]) T) = arg_value;
	}

	void pop()
	{
		((T*)data_[--currentSize])->~T();
	}

	void pop(const int count)
	{
		resize(currentSize - count);
	}

	void resize(const int newsize)
	{
		int oldsize = currentSize;

		if (oldsize > newsize) {
			for (int i = newsize; i < oldsize; ++i)
				((T*)data_[i])->~T();
		}
		if (oldsize < newsize) {
			if (Size < newsize)
				throw StackOverflow();
			for (int i = oldsize; i < newsize; ++i)
				::new(data_[i]) T;
		}
		currentSize = newsize;
	}

	const T& top() const { return *(const T*)data_[currentSize - 1]; }
	T& top() {
		return *(T*)data_[currentSize - 1]; 
	}

	bool overflow() const { return currentSize >= Size; }
	bool empty() const { return currentSize == 0; }
	int size() const { 
		return 	currentSize;
	}

	const T& operator[](const int index) const { 
		return *(const T*)data_[index]; 
	}
	T& operator[](const int index) { 
		return *(T*)data_[index]; 
	}

protected:
	char data_[Size][sizeof(T)];
	int currentSize;
};
}
namespace ButiScript {
class Sample {
public:
	Sample() {
		static int incr = 0;
		i = incr++;
	}
	void SampleMethod() {
		std::cout << "SampleMethod:" << i << std::endl;
	}
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(i);
	}
private:
	int i;
};

#ifdef IMPL_BUTIENGINE
template<typename T>
void  ButiScript::GlobalValueSaveObject<T>::RestoreValue(ButiScript::IValueData** arg_v) const
{
	*arg_v = new ButiScript::ValueData<T>(data, 1);
}
template<typename T>
void ButiScript::GlobalSharedPtrValueSaveObject<T>::RestoreValue(ButiScript::IValueData** arg_v) const
{
	*arg_v = new ButiScript::Value_Shared<T>(data, 1);
}
#endif // IMPL_BUTIENGINE
}

#endif

