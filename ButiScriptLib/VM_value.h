#pragma once
#ifndef	__vm_value_h__
#define	__vm_value_h__
#include"value_type.h"
#include <iostream>
#include <exception>
#include <string>
#include"Tags.h"
#include<mutex>
#include"ButiMath/ButiMath.h"

namespace ButiScript {
template <typename T>class Type_hasMember;
}
#include"ButiMemorySystem/ButiMemorySystem/ButiPtr.h"
namespace std {
template<typename T>
inline std::string to_string(const ButiScript::Type_hasMember<T>& arg_v) {
	return std::to_string(static_cast<T>(arg_v));
}
}

namespace ButiScript {
constexpr std::int32_t vTableSize = 0b00001000;
class IType_hasMember {
public:
	virtual inline ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index) = 0;
	virtual inline const ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)const = 0;
	virtual inline const std::int32_t GetMemberType(const std::int32_t arg_index)const = 0;
	virtual inline void SetMember(ButiEngine::Value_ptr<void> arg_data, const std::int32_t arg_index) = 0;
	template<typename T>
	void DoSome() {}
	template <typename MemberType, std::int32_t TypeIndex>
	inline void SetMemberType(const std::int32_t ByteOffset) {
		SetMemberType_({ ButiEngine::to_value(reinterpret_cast<MemberType*> (reinterpret_cast<std::int8_t*>(this) + ByteOffset)),TypeIndex });
	}
	template <typename MemberType, std::int32_t ByteOffset, std::int32_t TypeIndex>
	inline void SetMemberType() {
		SetMemberType_({ ButiEngine::to_value(reinterpret_cast<MemberType*> (reinterpret_cast<std::int8_t*>(this) + ByteOffset)),TypeIndex });
	}
protected:
	virtual inline void SetMemberType_(std::pair< ButiEngine::Value_ptr<void>, std::int32_t> arg_memberType) = 0;
};

template<typename T>
class Type_hasMember :public T, public IType_hasMember {
public:
	inline Type_hasMember(const T& v) :T(v) {
	}
	inline Type_hasMember() {}
	inline Type_hasMember(const Type_hasMember<T>& arg_v) : T(static_cast<T>(arg_v)) {
		list_member.reserve(arg_v.list_member.size());
		for (auto& member : arg_v.list_member) {
			list_member.push_back({ ButiEngine::CreateSameTypeValuePtr(member.first,reinterpret_cast<std::int8_t*>(this) + (reinterpret_cast<std::int8_t>(member.first.get()) - reinterpret_cast<std::int8_t>(&arg_v))) ,member.second });
		}
	}
	inline ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)override {
		return list_member.at(arg_index).first;
	}
	inline const ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)const override {
		return list_member.at(arg_index).first;
	}
	inline const std::int32_t GetMemberType(const std::int32_t arg_index)const {
		return list_member.at(arg_index).second;
	}
	inline void SetMember(ButiEngine::Value_ptr<void> arg_data, const std::int32_t arg_index)override {
		list_member.at(arg_index).first = arg_data;
	}
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(static_cast<T&>(*this));
	}
protected:
	inline void SetMemberType_(std::pair< ButiEngine::Value_ptr<void>, std::int32_t>& arg_memberType)override {
		list_member.push_back(arg_memberType);
	}
private:
	ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>> list_member;
};
template<>
class Type_hasMember< ButiEngine::Vector2> :public ButiEngine::Vector2, public IType_hasMember {
public:
	inline Type_hasMember(const  ButiEngine::Vector2& v) : ButiEngine::Vector2(v) {
		SetMembers();
	}
	inline Type_hasMember() {
		SetMembers();
	}
	inline Type_hasMember(const Type_hasMember< ButiEngine::Vector2>& arg_v) : ButiEngine::Vector2(static_cast<ButiEngine::Vector2>(arg_v)) {
		SetMembers();
	}
	inline ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)override {
		return list_member.At(arg_index).first;
	}
	inline const ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)const override {
		return list_member.At(arg_index).first;
	}
	inline const std::int32_t GetMemberType(const std::int32_t arg_index)const {
		return list_member.At(arg_index).second;
	}
	inline void SetMember(ButiEngine::Value_ptr<void> arg_data, const std::int32_t arg_index)override {
		list_member.At(arg_index).first = arg_data;
	}
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(static_cast<ButiEngine::Vector2&>(*this));
	}
protected:
	inline void SetMemberType_(std::pair< ButiEngine::Value_ptr<void>, std::int32_t> arg_memberType)override {
		list_member.Add(arg_memberType);
	}
private:
	inline void SetMembers() {
		this->SetMemberType<float, vTableSize + sizeof(float) * 0, TYPE_FLOAT>();
		this->SetMemberType<float, vTableSize + sizeof(float) * 1, TYPE_FLOAT>();
	}
	ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>> list_member;
};
template<>
class Type_hasMember< ButiEngine::Vector3> :public ButiEngine::Vector3, public IType_hasMember {
public:
	inline Type_hasMember(const  ButiEngine::Vector3& v) : ButiEngine::Vector3(v) {
		SetMembers();
	}
	inline Type_hasMember() {
		SetMembers();
	}
	inline Type_hasMember(const Type_hasMember< ButiEngine::Vector3>& arg_v) : ButiEngine::Vector3(static_cast<ButiEngine::Vector3>(arg_v)) {
		SetMembers();
	}
	inline ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)override {
		return list_member.At(arg_index).first;
	}
	inline const ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)const override {
		return list_member.At(arg_index).first;
	}
	inline const std::int32_t GetMemberType(const std::int32_t arg_index)const {
		return list_member.At(arg_index).second;
	}
	inline void SetMember(ButiEngine::Value_ptr<void> arg_data, const std::int32_t arg_index)override {
		list_member.At(arg_index).first = arg_data;
	}
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(static_cast<ButiEngine::Vector3&>(*this));
	}
protected:
	inline void SetMemberType_(std::pair< ButiEngine::Value_ptr<void>, std::int32_t> arg_memberType)override {
		list_member.Add(arg_memberType);
	}
private:
	inline void SetMembers() {
		this->SetMemberType<float, vTableSize + sizeof(float) * 0, TYPE_FLOAT>();
		this->SetMemberType<float, vTableSize + sizeof(float) * 1, TYPE_FLOAT>();
		this->SetMemberType<float, vTableSize + sizeof(float) * 2, TYPE_FLOAT>();
	}
	ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>> list_member;
};

template<>
class Type_hasMember< ButiEngine::Vector4> :public ButiEngine::Vector4, public IType_hasMember {
public:
	inline Type_hasMember(const  ButiEngine::Vector4& v) : ButiEngine::Vector4(v) {
		SetMembers();
	}
	inline Type_hasMember() {
		SetMembers();
	}
	inline Type_hasMember(const Type_hasMember< ButiEngine::Vector4>& arg_v) : ButiEngine::Vector4(static_cast<ButiEngine::Vector4>(arg_v)) {
		SetMembers();
	}
	inline ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)override {
		return list_member.At(arg_index).first;
	}
	inline const ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)const override {
		return list_member.At(arg_index).first;
	}
	inline const std::int32_t GetMemberType(const std::int32_t arg_index)const {
		return list_member.At(arg_index).second;
	}
	inline void SetMember(ButiEngine::Value_ptr<void> arg_data, const std::int32_t arg_index)override {
		list_member.At(arg_index).first = arg_data;
	}

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(static_cast<ButiEngine::Vector4&>(*this));
	}
protected:
	inline void SetMemberType_(std::pair< ButiEngine::Value_ptr<void>, std::int32_t> arg_memberType)override {
		list_member.Add(arg_memberType);
	}
private:
	inline void SetMembers() {
		this->SetMemberType<float, vTableSize + sizeof(float) * 0, TYPE_FLOAT>();
		this->SetMemberType<float, vTableSize + sizeof(float) * 1, TYPE_FLOAT>();
		this->SetMemberType<float, vTableSize + sizeof(float) * 2, TYPE_FLOAT>();
		this->SetMemberType<float, vTableSize + sizeof(float) * 3, TYPE_FLOAT>();
	}
	ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>> list_member;
};

template<>
class Type_hasMember< ButiEngine::Matrix4x4> :public ButiEngine::Matrix4x4, public IType_hasMember {
public:
	inline Type_hasMember(const  ButiEngine::Matrix4x4& v) : ButiEngine::Matrix4x4(v) {
		SetMembers();
	}
	inline Type_hasMember() {
		SetMembers();
	}
	inline Type_hasMember(const Type_hasMember< ButiEngine::Matrix4x4>& arg_v) : ButiEngine::Matrix4x4(static_cast<ButiEngine::Matrix4x4>(arg_v)) {
		SetMembers();
	}
	inline ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)override {
		return list_member.At(arg_index).first;
	}
	inline const ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)const override {
		return list_member.At(arg_index).first;
	}
	inline const std::int32_t GetMemberType(const std::int32_t arg_index)const {
		return list_member.At(arg_index).second;
	}
	inline void SetMember(ButiEngine::Value_ptr<void> arg_data, const std::int32_t arg_index)override {
		list_member.At(arg_index).first = arg_data;
	}

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(static_cast<ButiEngine::Matrix4x4&>(*this) );
	}
protected:
	inline void SetMemberType_(std::pair< ButiEngine::Value_ptr<void>, std::int32_t> arg_memberType)override {
		list_member.Add(arg_memberType);
	}
private:
	inline void SetMembers() {

		this->SetMemberType<float, vTableSize + sizeof(float) * 0, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 1, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 2, TYPE_FLOAT>();	this->SetMemberType<float, vTableSize + sizeof(float) * 3, TYPE_FLOAT>();
		this->SetMemberType<float, vTableSize + sizeof(float) * 4, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 5, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 6, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 7, TYPE_FLOAT>();
		this->SetMemberType<float, vTableSize + sizeof(float) * 8, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 9, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 10, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 11, TYPE_FLOAT>();
		this->SetMemberType<float, vTableSize + sizeof(float) * 12, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 13, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 14, TYPE_FLOAT>(); this->SetMemberType<float, vTableSize + sizeof(float) * 15, TYPE_FLOAT>();

	}
	ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>> list_member;
};
//参照型など実体を持たない変数の初期化に使用
struct Type_Null {};
struct Type_Enum {
	Type_Enum(const std::int32_t arg_value, const EnumTag* arg_p_enumTag)noexcept :value(arg_value), p_enumTag(arg_p_enumTag) {}
	Type_Enum(const EnumTag* arg_p_enumTag)noexcept :p_enumTag(arg_p_enumTag) {}
	Type_Enum(){}
	std::int32_t value;
	const EnumTag* p_enumTag;
};
struct Type_Function {
	Type_Function(const std::int32_t arg_address, const FunctionTag* arg_p_functionTag, const std::map <  std::int32_t, const std::string*>* arg_jumpTable, const ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>>& arg_capturedValue)noexcept
		:address(arg_address), p_functionTag(arg_p_functionTag), p_functionJumpTable(arg_jumpTable), list_capturedValue(arg_capturedValue)
	{}
	Type_Function(const std::int32_t arg_address, const std::map <  std::int32_t, const std::string*>* arg_jumpTable, const ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>>& arg_capturedValue)noexcept
		:address(arg_address), p_functionJumpTable(arg_jumpTable), list_capturedValue(arg_capturedValue)
	{}
	Type_Function(const FunctionTag* arg_p_functionTag, const std::map <  std::int32_t, const std::string*>* arg_jumpTable, const ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>>& arg_capturedValue)noexcept
		: p_functionTag(arg_p_functionTag), p_functionJumpTable(arg_jumpTable), list_capturedValue(arg_capturedValue)
	{}
	Type_Function(const std::map <  std::int32_t, const std::string*>* arg_jumpTable, const ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>>& arg_capturedValue)noexcept
		: p_functionJumpTable(arg_jumpTable), list_capturedValue(arg_capturedValue)
	{}
	Type_Function(){}
	inline void AddCapture(const ButiEngine::Value_ptr<void> arg_value, const std::int32_t arg_type) {
		list_capturedValue.Add({ arg_value,arg_type });
	}
	std::int32_t address;
	const FunctionTag* p_functionTag;
	const std::map <  std::int32_t, const std::string*>* p_functionJumpTable;
	ButiEngine::List<std::pair< ButiEngine::Value_ptr<void>, std::int32_t>> list_capturedValue;
};
struct Type_ScriptClass :public IType_hasMember
{
	Type_ScriptClass(const ScriptClassInfo* arg_info) :p_classInfo(arg_info) {
		list_member.Resize(arg_info->GetMemberSize());
	}
	Type_ScriptClass(const ButiEngine::List<ButiEngine::Value_ptr<void>>& arg_list_member, const ScriptClassInfo* arg_info)noexcept :list_member(arg_list_member), p_classInfo(arg_info) {}
	Type_ScriptClass(const Type_ScriptClass& arg_v) :p_classInfo(arg_v.p_classInfo) {
		list_member.Reserve(arg_v.list_member.GetSize());
		for (auto& member : arg_v.list_member) {
			list_member.Add(member.Clone());
		}
	}
	Type_ScriptClass()noexcept:list_member(),p_classInfo(nullptr){}
	inline ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)override {
		return list_member.At(arg_index);
	}
	inline const ButiEngine::Value_ptr<void>& GetMember(const std::int32_t arg_index)const override {
		return list_member.At(arg_index);
	}
	inline const std::int32_t GetMemberType(const std::int32_t arg_index)const {
		return p_classInfo->GetMemberTypeIndex(arg_index);;
	}
	inline void SetMember(ButiEngine::Value_ptr<void> arg_data, const std::int32_t arg_index)override {
		list_member.At(arg_index) = arg_data;
	}
	inline void SetClassInfoUpdate(const ButiEngine::List<TypeTag>& arg_types, const ButiEngine::List<ScriptClassInfo>& arg_scriptClassInfos, const std::int32_t arg_sysTypeCount, const std::int32_t arg_typeIndex) {

	}
	bool ShowGUI(const std::string& arg_label) {
		bool output = false;
#ifdef _BUTIENGINEBUILD


		if (ButiEngine::GUI::TreeNode(arg_label)) {
			std::int32_t index = 0;
			for (auto& member : list_member)
			{
				auto guiRet= member.ShowGUI(p_classInfo->GetMamberName()[index]);
				output = guiRet ? guiRet : output;
				index++;
			}
			ButiEngine::GUI::TreePop();
		}
#endif // _BUTIENGINEBUILD
		return output;
	}
	ButiEngine::List<ButiEngine::Value_ptr<void>> list_member;
	const ScriptClassInfo* p_classInfo;
protected:

	inline void SetMemberType_(std::pair< ButiEngine::Value_ptr<void>, std::int32_t> arg_memberType)override {	}
};
template<typename T>
std::int64_t TypeSpecific() {
	static T* output[1];
	return reinterpret_cast<std::int64_t>(output);
}
#ifdef _BUTIENGINEBUILD

}
namespace ButiEngine{
class IUseCompiledData {
public: 
	virtual void SetCompiledData(Value_ptr<ButiScript::CompiledData> arg_vlp_data) = 0;
};

template<>
class ValuePtrRestoreObject <ButiScript::Type_ScriptClass> :public IValuePtrRestoreObject, public IUseCompiledData {
public:
	ValuePtrRestoreObject(const ButiScript::Type_ScriptClass& arg_value) {
		type = arg_value.p_classInfo->GetTypeIndex();
		std::int32_t memberIndex =0;
		for (auto& v : arg_value.list_member) {
			list_data.push_back({ v.GetRestoreObject(),arg_value.p_classInfo->GetMemberTypeIndex(memberIndex) });
			memberIndex++;
		}
	}
	ValuePtrRestoreObject() {}
	void SetCompiledData(Value_ptr<ButiScript::CompiledData> arg_vlp_data)override { vlp_compiledData = arg_vlp_data; }
	void RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const override;
	void Push(std::pair< ButiEngine::Value_ptr<ButiEngine::IValuePtrRestoreObject>, std::int32_t> arg_vlp_data) {
		list_data.push_back(arg_vlp_data);
	}
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(list_data);
		archive(type);
	}
private:
	std::vector<std::pair< ButiEngine::Value_ptr<ButiEngine::IValuePtrRestoreObject>, std::int32_t>> list_data;
	std::int32_t type;
	Value_ptr<ButiScript::CompiledData>vlp_compiledData;
};
template<>
class ValuePtrRestoreObject <ButiScript::Type_Enum> :public IValuePtrRestoreObject,public IUseCompiledData {
public:
	ValuePtrRestoreObject(const ButiScript::Type_Enum& arg_value) {
		data = arg_value.value;
		type = arg_value.p_enumTag->typeIndex;
	}
	ValuePtrRestoreObject() {
	}
	void RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const override;
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(data);
		archive(type);
	}
	void SetCompiledData(Value_ptr<ButiScript::CompiledData> arg_vlp_data)override { vlp_compiledData = arg_vlp_data; }
private:
	std::int32_t data;
	std::int32_t type;
	Value_ptr<ButiScript::CompiledData>vlp_compiledData;
};
template<>
class ValuePtrRestoreObject <ButiScript::Type_Function> :public IValuePtrRestoreObject, public IUseCompiledData {
public:
	ValuePtrRestoreObject(const ButiScript::Type_Function& arg_value) {
		address = arg_value.address;
	}
	ValuePtrRestoreObject(){}
	void RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const override;
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(address);
	}
	void SetCompiledData(Value_ptr<ButiScript::CompiledData> arg_vlp_data)override { vlp_compiledData = arg_vlp_data; }
private:
	std::int32_t address;
	Value_ptr<ButiScript::CompiledData>vlp_compiledData;
};
template<>
class ValuePtrRestoreObject <ButiScript::Type_Null> :public IValuePtrRestoreObject {
public:
	ValuePtrRestoreObject(const ButiScript::Type_Null&){}
	ValuePtrRestoreObject(){}
	void RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const override;
	template<class Archive>
	void serialize(Archive& archive)
	{
	}
private:
};

}

namespace ButiScript{
#endif //_BUTIENGINEBUILD


template<typename T>
inline ButiEngine::Value_ptr<void> CreateMemberInstance() {
	return ButiEngine::make_value<T>();
}

using CreateMemberInstanceFunction = ButiEngine::Value_ptr<void>(*)();

void PushCreateMemberInstance(CreateMemberInstanceFunction arg_function);

template<typename T>
inline void PushCreateMemberInstance() {
	PushCreateMemberInstance(CreateMemberInstance<T>);
}

// 変数
class Value {

public:
	constexpr Value()noexcept : valueType(TYPE_VOID) {}

	template<typename T>
	inline Value(const T& arg_v) {
		valueData = ButiEngine::make_value<T>(arg_v);
		valueType = TYPE_VOID;
	}
	template<>
	inline Value(const ButiEngine::Vector2& arg_v) {
		static Type_hasMember<ButiEngine::Vector2> t;
		auto data = ButiEngine::make_value<Type_hasMember<ButiEngine::Vector2>>(arg_v);
		valueData = data;
		valueType = TYPE_VOID;
	}
	template<>
	inline Value(const ButiEngine::Vector3& arg_v) {
		static Type_hasMember<ButiEngine::Vector3> t;
		auto data = ButiEngine::make_value<Type_hasMember<ButiEngine::Vector3>>(arg_v);
		valueData = data;
		valueType = TYPE_VOID;
	}
	template<>
	inline Value(const ButiEngine::Vector4& arg_v) {
		static Type_hasMember<ButiEngine::Vector4> t;
		auto data = ButiEngine::make_value<Type_hasMember<ButiEngine::Vector4>>(arg_v);

		valueData = data;
		valueType = TYPE_VOID;
	}
	template<>
	inline Value(const ButiEngine::Matrix4x4& arg_v) {
		static Type_hasMember<ButiEngine::Matrix4x4> t;
		auto data = ButiEngine::make_value<Type_hasMember<ButiEngine::Matrix4x4>>(arg_v);
		valueData = data;
		valueType = TYPE_VOID;
	}
	constexpr Value(const Type_Null)noexcept:valueType(TYPE_VOID) {}

	//ユーザー定義型として初期化
	Value(ScriptClassInfo& arg_info, ButiEngine::List<ButiScript::ScriptClassInfo>* arg_p_list_scriptClassInfo);

	//変数を指定して初期化
	Value( ButiEngine::Value_ptr<void> arg_p_data,const std::int32_t arg_type)
	{
		valueData = arg_p_data;
		valueType = arg_type;
	}
	Value(ButiEngine::Value_ptr<void> arg_p_data)
	{
		valueData = arg_p_data;
		valueType = TYPE_VOID;
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
		if (this != &other) {
			Assign(other);
		}
		return *this;
	}

	static std::int32_t SetTypeIndex(std::int64_t arg_typeFunc);
	static std::int32_t GetTypeIndex(std::int64_t arg_typeFunc);

	void clear()
	{
		valueData = nullptr;
	}
	Value Clone() {
		auto output = Value(valueData.Clone(), valueType);
		return output;
	}

	void Copy(const Value& other)
	{
		valueType = other.valueType;
		valueData = other.valueData;
	}
	inline void Assign(const Value& other) {
		clear();
		valueData = other.valueData;
		if (valueType == TYPE_VOID) {
			valueType = other.valueType;
		}
	}
	template <typename OwnType,typename CastType>
	inline Value Cast()const {
		return Value(valueData.StaticCast<OwnType, CastType>(), GetTypeIndex(TypeSpecific<CastType>()));
	}
	template <typename T>
	inline const T& Get()const {
		return *valueData.get<T>();
	}
	template <typename T>
	inline T& Get(){
		return *valueData.get<T>();
	}
	template <typename T>
	inline void Negative() {
		*valueData.get<T>() = -1 * *valueData.get<T>();
	}
	template <typename T>
	inline void Not() {
		*valueData.get<T>() = ! *valueData.get<T>();
	}
	template<typename T>
	inline void Increment() {
		(*valueData.get<T>())++;
	}
	template<typename T>
	inline void Decrement() {
		(*valueData.get<T>())--;
	}
	void SetType(const std::int32_t arg_type) {
		valueType = arg_type;
	}

	///Vector2等の組み込みクラスの取り出し
	template <>	inline const ButiEngine::Vector2& Get()const {
		return static_cast<const ButiEngine::Vector2&>(*valueData.get<Type_hasMember<ButiEngine::Vector2>>());
	}
	template <>	inline ButiEngine::Vector2& Get() {
		return static_cast<ButiEngine::Vector2&>(*valueData.get<Type_hasMember<ButiEngine::Vector2>>());
	}
	template <>	inline void Negative<ButiEngine::Vector2>() {
		static_cast<ButiEngine::Vector2&>(*valueData.get<Type_hasMember<ButiEngine::Vector2>>()) = -1 * static_cast<ButiEngine::Vector2&>(*valueData.get<Type_hasMember<ButiEngine::Vector2>>());
	}
	template <>	inline const ButiEngine::Vector3& Get()const {
		return static_cast<const ButiEngine::Vector3&>(*valueData.get<Type_hasMember<ButiEngine::Vector3>>());
	}
	template <>	inline ButiEngine::Vector3& Get() {
		return static_cast<ButiEngine::Vector3&>(*valueData.get<Type_hasMember<ButiEngine::Vector3>>());
	}
	template <>	inline void Negative<ButiEngine::Vector3>() {
		static_cast<ButiEngine::Vector3&>(*valueData.get<Type_hasMember<ButiEngine::Vector3>>()) = -1 * static_cast<ButiEngine::Vector3&>(*valueData.get<Type_hasMember<ButiEngine::Vector3>>());
	}
	template <>	inline const ButiEngine::Vector4& Get()const {
		return static_cast<const ButiEngine::Vector4&>(*valueData.get<Type_hasMember<ButiEngine::Vector4>>());
	}
	template <>	inline ButiEngine::Vector4& Get() {
		return static_cast<ButiEngine::Vector4&>(*valueData.get<Type_hasMember<ButiEngine::Vector4>>());
	}
	template <>	inline void Negative<ButiEngine::Vector4>() {
		static_cast<ButiEngine::Vector4&>(*valueData.get<Type_hasMember<ButiEngine::Vector4>>()) = -1 * static_cast<ButiEngine::Vector4&>(*valueData.get<Type_hasMember<ButiEngine::Vector4>>());
	}
	template <>	inline const ButiEngine::Matrix4x4& Get()const {
		return static_cast<const ButiEngine::Matrix4x4&>(*valueData.get<Type_hasMember<ButiEngine::Matrix4x4>>());
	}
	template <>	inline ButiEngine::Matrix4x4& Get() {
		return static_cast<ButiEngine::Matrix4x4&>(*valueData.get<Type_hasMember<ButiEngine::Matrix4x4>>());
	}
	template <>	inline void Negative<ButiEngine::Matrix4x4>() {
		static_cast<ButiEngine::Matrix4x4&>(*valueData.get<Type_hasMember<ButiEngine::Matrix4x4>>()) = -1 * static_cast<ButiEngine::Matrix4x4&>(*valueData.get<Type_hasMember<ButiEngine::Matrix4x4>>());
	}
	ButiEngine::Value_ptr<void> valueData;
	std::int32_t valueType;
};
namespace ButiValueOperator {
template<typename LeftType,typename RightType, typename RetType=LeftType>
inline RetType Addition(const Value& arg_left, const Value& arg_right) {
	return arg_left.Get<LeftType>() + arg_right.Get<RightType>();
}
template<typename LeftType, typename RightType, typename RetType = LeftType>
inline RetType Subtract(const Value& arg_left, const Value& arg_right) {
	return arg_left.Get<LeftType>() - arg_right.Get<RightType>();
}
template<typename LeftType, typename RightType, typename RetType = LeftType>
inline RetType Multiple(const Value& arg_left, const Value& arg_right) {
	return arg_left.Get<LeftType>() * arg_right.Get<RightType>();
}
template<typename LeftType, typename RightType, typename RetType = LeftType>
inline RetType Division(const Value& arg_left, const Value& arg_right) {
	return arg_left.Get<LeftType>() / arg_right.Get<RightType>();
}
template<typename LeftType, typename RightType>
inline std::int32_t Mod(const Value& arg_left, const Value& arg_right) {
	std::int32_t leftInt = static_cast<std::int32_t>(arg_left.Get<LeftType>());
	std::int32_t rightInt = static_cast<std::int32_t>(arg_left.Get<RightType>());
	return  leftInt % rightInt;
}
template<typename LeftType, typename RightType>
inline bool Equal(const Value& arg_left, const Value& arg_right) {
	return arg_left.Get<LeftType>() == arg_right.Get<RightType>();
}
template<typename LeftType, typename RightType>
inline bool GreaterThan(const Value& arg_left, const Value& arg_right) {
	return arg_left.Get<LeftType>() > arg_right.Get<RightType>();
}
template<typename LeftType, typename RightType>
inline bool GreaterEqual(const Value& arg_left, const Value& arg_right) {
	return arg_left.Get<LeftType>() >= arg_right.Get<RightType>();
}
}

class StackOverflow : public std::exception {
public:
	const char* what() const throw()
	{
		return "stack overflow";
	}
};

// 固定サイズスタック
template< typename T, std::int32_t Size >
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
		*(::new(data[currentSize++]) T) = arg_value;
	}

	void pop()
	{
		((T*)data[--currentSize])->~T();
	}

	void pop(const std::int32_t count)
	{
		resize(currentSize - count);
	}

	void resize(const std::int32_t newsize)
	{
		std::int32_t oldsize = currentSize;

		if (oldsize > newsize) {
			for (std::int32_t i = newsize; i < oldsize; ++i)
				((T*)data[i])->~T();
		}
		if (oldsize < newsize) {
			if (Size < newsize)
				throw StackOverflow();
			for (std::int32_t i = oldsize; i < newsize; ++i)
				::new(data[i]) T;
		}
		currentSize = newsize;
	}

	const T& top() const { return *(const T*)data[currentSize - 1]; }
	T& top() {
		return *(T*)data[currentSize - 1];
	}

	bool overflow() const { return currentSize >= Size; }
	bool empty() const { return currentSize == 0; }
	std::int32_t size() const { 
		return 	currentSize;
	}

	const T& operator[](const std::int32_t index) const { 
		return *(const T*)data[index];
	}
	T& operator[](const std::int32_t index) { 
		return *(T*)data[index];
	}

protected:
	char data[Size][sizeof(T)];
	std::int32_t currentSize;
};
}
#endif

