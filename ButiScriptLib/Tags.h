#pragma once
#ifndef TAGS_H
#define TAGS_H
#include"value_type.h"
#include<iostream>
#include<map>
#include"ButiMath/ButiMath.h"
#include"ButiMemorySystem/ButiMemorySystem/ButiPtr.h"
#include"ButiMemorySystem/ButiMemorySystem/ButiList.h"
#include"ButiUtil/ButiUtil/BinaryObject.h"
namespace ButiScript {

class Compiler;
class TypeTable;

enum class AccessModifier {
	Public, Private,Protected
};


AccessModifier StringToAccessModifier(const std::string& arg_modifierStr);

// 引数
class ArgDefine {
public:
	ArgDefine()
		: valueType(0)
	{
	}

	ArgDefine(const std::int32_t arg_type): valueType(arg_type){}
	ArgDefine(const std::int32_t arg_type, const std::string& arg_name) :valueType(arg_type), name(arg_name) {}
	ArgDefine(const std::string& arg_valueTypeName, const std::string& arg_name):valueTypeName(arg_valueTypeName), name(arg_name){}

	void SetRef()
	{
		valueType |= TYPE_REF;
	}

	void SetName(const std::string& arg_name)
	{
		name = arg_name;
	}

	void SetTypeName(const std::string& arg_name)
	{
		valueTypeName= arg_name;
	}

	std::int32_t GetType() const { return valueType; }
	const std::string& GetName() const { return name; }
	const std::string& GetTypeName() const { return valueTypeName; }
	void SpecficType(const Compiler* arg_compiler);
private:
	std::int32_t valueType;
	std::string name,valueTypeName;
};
class EnumTag {
public:
	EnumTag(){}
	EnumTag(const std::string& arg_typeName):type_Name(arg_typeName){}

	void SetValue(const std::string& arg_identiferName, const std::int32_t arg_value) {
		if (map_identifers.count(arg_identiferName)) {
			return;
		}
		map_identifers.emplace(arg_identiferName, arg_value);
	}
	std::int32_t GetValue(const std::string& arg_identiferName)const {
		return map_identifers.at(arg_identiferName);
	}
	bool ExistIdentifers(const std::string& arg_identiferName)const {
		return map_identifers.count(arg_identiferName);
	}
	const std::string& GetTypeName()const {
		return type_Name;
	}
	const std::string& GetValueName(const std::int32_t value)const {
		return map_identifer_value.at(value);
	}
	void OutputFile(std::ofstream& arg_out)const {
		std::int32_t nameSize = type_Name.size();
		arg_out.write((char*)&nameSize, sizeof(std::int32_t));
		arg_out.write(type_Name.c_str(), nameSize);

		std::int32_t identSize = map_identifers.size();
		arg_out.write((char*)&identSize, sizeof(std::int32_t));
		for (auto itr = map_identifers.begin(), end = map_identifers.end(); itr != end; itr++) {
			std::int32_t identiferSize = itr->first.size();
			arg_out.write((char*)&identiferSize, sizeof(std::int32_t));
			arg_out.write(itr->first.c_str(), identiferSize);
			arg_out.write((char*)&itr->second, sizeof(std::int32_t));

		}
	}
	void InputFile(ButiEngine::Value_ptr<ButiEngine::IBinaryReader> arg_reader) {
		std::int32_t nameSize= arg_reader->ReadVariable<std::int32_t>();
		type_Name = arg_reader->ReadCharactor(nameSize);
		std::int32_t identSize = arg_reader->ReadVariable<std::int32_t>();
		for (std::int32_t i = 0; i < identSize;i++) {
			std::int32_t identiferSize = arg_reader->ReadVariable<std::int32_t>();
			auto identifer =arg_reader->ReadCharactor(identiferSize);
			std::int32_t value = arg_reader->ReadVariable<std::int32_t>();
			map_identifers.emplace(identifer, value);
		}
	}
	void CreateEnumMap() {
		for (auto itr = map_identifers.begin(), end = map_identifers.end(); itr != end; itr++) {
			if (!map_identifer_value.count(itr->second)) {
				map_identifer_value.emplace(itr->second, itr->first);
			}
		}
	}
	const std::map<std::int32_t, std::string>& GetIdentiferMap()const {
		return map_identifer_value;
	}
	bool isSystem;
	std::int32_t typeIndex;
private:
	std::string type_Name;
	std::map<std::string, std::int32_t>map_identifers;
	std::map<std::int32_t, std::string>map_identifer_value;
};
class EnumTable {
public:
	EnumTable(){}

	void SetEnum(const EnumTag& arg_enumTag) {
		map_enumTag.emplace(arg_enumTag.GetTypeName(), arg_enumTag);
	}

	const EnumTag* FindType(const std::string& arg_typeName)const {
		if (!map_enumTag.count(arg_typeName)) {
			return nullptr;
		}

		return &map_enumTag.at(arg_typeName);
	}
	EnumTag* FindType(const std::string& arg_typeName){
		if (!map_enumTag.count(arg_typeName)) {
			return nullptr;
		}
		return &map_enumTag.at(arg_typeName);
	}
	void Clear_notSystem() {
		for (auto enumItr = map_enumTag.begin(); enumItr != map_enumTag.end(); ) {
			if (!enumItr->second.isSystem) {
				enumItr = map_enumTag.erase(enumItr);
			}
			else {
				enumItr++;
			}
		}
	}
	std::int32_t Size()const {
		return map_enumTag.size();
	}
	EnumTag* operator[](const std::int32_t arg_index) {
		auto itr = map_enumTag.begin();
		for (std::int32_t i = 0; i < arg_index; i++) { itr++; }
		return &itr->second;
	}
private:
	std::map<std::string, EnumTag> map_enumTag;
};

class ValueTag {
public:
	friend class ValueTable;
	ValueTag() : address(-1), originalAddress(-1), valueType(TYPE_INTEGER), currentSize(1), isGlobal(false)
	{
	}
	ValueTag(const std::int32_t arg_addr, const std::int32_t arg_type, const std::int32_t arg_size, const bool arg_global ,const AccessModifier arg_access)
		: address(arg_addr),originalAddress(arg_addr), valueType(arg_type), currentSize(arg_size), isGlobal(arg_global),access(arg_access)
	{
	}

	std::int32_t		valueType;
	std::int32_t		currentSize;
	bool	isGlobal;
	AccessModifier access=AccessModifier::Public;
	std::int32_t GetAddress()const { return address; }
	
private:

	std::int32_t		address,originalAddress;
};
class ValueTable {
private:
	using iter = std::map<std::string, ValueTag>::iterator;
	using const_iter = std::map<std::string, ValueTag>::const_iterator;

public:
	ValueTable(const std::int32_t arg_start_addr = 0, const bool arg_isFunctionBlockTable = false) : addr(arg_start_addr), isGlobal(false), isFunction(true)
	{
	}

	void set_global()
	{
		isGlobal = true;
	}

	bool Add(const std::int32_t arg_type, const std::string& arg_name,const AccessModifier arg_access, const std::int32_t arg_size = 1)
	{
		if (!map_variables.count(arg_name)) {
			map_variables.emplace(arg_name, ValueTag(addr, arg_type, arg_size, isGlobal,arg_access));
			list_variableTypes.Add(arg_type);
			addr += arg_size;
			return true;
		}
		return false;
	}

	const ValueTag* find(const std::string& arg_name) const
	{
		const_iter it = map_variables.find(arg_name);
		if (it != map_variables.end())
			return &it->second;
		return nullptr;
	}

	bool add_arg(const std::int32_t arg_type, const std::string& arg_name, const std::int32_t arg_addr)
	{
		auto result = map_variables.insert({ arg_name, ValueTag(arg_addr, arg_type, 1, false,AccessModifier::Public) });
		argmentsCount++;
		return result.second;
	}
	bool add_capture(const std::int32_t arg_type, const std::string& arg_name, const std::int32_t arg_addr) {

		auto result = map_variables.insert({ arg_name, ValueTag(arg_addr, arg_type, 1, false, AccessModifier::Public) });
		addr++;
		return result.second;
	}

	std::int32_t size() const { return addr; }
	std::int32_t valueCount()const { return map_variables.size(); }

	std::int32_t AddressAdd(const std::int32_t arg_v) {
		auto difference = arg_v +( map_variables.size() - 1-argmentsCount);
		for (auto itr = map_variables.begin(), end = map_variables.end(); itr != end; itr++) {
			itr->second.address = itr->second.originalAddress+ difference;
		}
		return difference+argmentsCount;
	}
	std::int32_t AddressSub(const std::int32_t arg_v) {

		auto difference = arg_v - (map_variables.size() - 1-argmentsCount);
		for (auto itr = map_variables.begin(), end = map_variables.end(); itr != end; itr++) {
			itr->second.address = itr->second.originalAddress + difference;
		}
		return difference-argmentsCount;
	}

	void Clear()
	{
		map_variables.clear();
		addr = 0;
	}

	void Alloc(Compiler* arg_comp)const;
	ValueTag& operator[] (const std::int32_t arg_index) {
		auto itr = map_variables.begin();
		for (std::int32_t i = 0; i < arg_index; i++) {
			itr++;
		}

		return itr->second;
	}

	const std::string& GetVariableName(const std::int32_t arg_index)const {
		auto itr = map_variables.begin();
		for (std::int32_t i = 0; i < arg_index; i++) {
			itr++;
		}

		return itr->first;
	}

bool IsFunctionBlock()const { return isFunction; }
#ifdef	_DEBUG

	void DumpAction(const std::pair<std::string, ValueTag>& it)const
	{
#ifdef BUTIGUI_H
		ButiEngine::GUI::Console(it.first + ", addr = " + std::to_string(it.second.address) + ", type = " + std::to_string(it.second.valueType) + ", size = " + std::to_string(it.second.currentSize) + ", global = " + std::to_string(it.second.isGlobal));
#else
		std::cout << it.first << ", addr = " << it.second.GetAddress() << ", type = " << it.second.valueType << ", size = " << it.second.currentSize << ", global = " << it.second.isGlobal << std::endl;

#endif // BUTIGUI_H
	}
	void Dump() const
	{
#ifdef BUTIGUI_H
		ButiEngine::GUI::Console("-------- value --------\n");
#else
		std::cout << "-------- value --------" << std::endl;
#endif // BUTIGUI_H
		for (auto& variable : map_variables) {
			DumpAction(variable);
		}
	}
#endif

private:
	std::map <std::string, ValueTag> map_variables;
	ButiEngine::List< std::int32_t> list_variableTypes;
	std::int32_t addr,argmentsCount=0;
	bool	isGlobal,isFunction;
};
class TypeTable;

static bool TypeCheck(const std::int32_t arg_left, const std::int32_t arg_right, const TypeTable* arg_table);
static bool TypeCheck_strict(const std::int32_t arg_left, const std::int32_t arg_right) {
	std::int32_t left = arg_left & ~TYPE_REF;
	std::int32_t right = arg_right & ~TYPE_REF;
	//同じ型のチェック
	if (left == right) {
		return true;
	}

	return false;
}


// 関数定義用

class FunctionTag {
	protected:
		enum {
			flag_declaration = 1 << 0,
			flag_definition = 1 << 1,
			flag_system = 1 << 2,
		};

	public:
		FunctionTag(const std::string& arg_name):name(arg_name)
		{
		}
		FunctionTag(const std::int32_t arg_type,const std::string &arg_name) 
			: valueType(arg_type), flags(0), index(0), name(arg_name)
		{
		}
		void SetArg(const std::int32_t arg_type)
		{
			list_args.Add(arg_type);
		}

		void SetArgs(const ButiEngine::List<ArgDefine>& arg_list_args)
		{
			std::uint64_t size = arg_list_args.GetSize();
			for (std::uint64_t i = 0; i < size; i++) {
				list_args.Add(arg_list_args[i].GetType());
			}
		}

		void SetArgs(const ButiEngine::List<std::int32_t>& arg_list_args)
		{
			std::uint64_t size = arg_list_args.GetSize();
			for (std::uint64_t i = 0; i < size; i++) {
				list_args.Add(arg_list_args[i]);
			}
		}

		bool SetArgs(const std::string& args, const std::map<std::string, std::int32_t>& arg_map_argmentChars)
		{
			if (args.size() == 0) {
				return true;
			}

			auto splited = ButiEngine::List<std::string>();
			std::int32_t first = 0;
			std::int32_t last = args.find_first_of(",");
			if (last == std::string::npos) {

				splited.Add(args);
			}
			else {
				while (first < args.size())
				{
					auto subString = args.substr(first, last - first);
					splited.Add(subString);
					first = last + 1;
					last = args.find_first_of(",", first);
					if (last == std::string::npos) {
						last = args.size();
					}
				}
			}

			for (std::int32_t i = 0; i < splited.GetSize(); i++) {
				if (!arg_map_argmentChars.count(splited[i])) {
					return false;
				}
				else {

					list_args.Add(arg_map_argmentChars.at(splited[i]));
				}

			}
			return true;
		}


		bool CheckArgList(const ButiEngine::List<ArgDefine>& arg_list_args,const TypeTable* arg_typeTable) const
		{
			// 引数が無い場合
			if (arg_list_args.IsEmpty())
				return list_args.IsEmpty();

			// 引数の個数が異なる
			if (arg_list_args.GetSize() != list_args.GetSize())
				return false;

			// 全引数の型をチェック

			//厳密チェック
			std::uint64_t size = list_args.GetSize();
			for (std::uint64_t i = 0; i < size; i++) {
				if (!TypeCheck(arg_list_args[i].GetType(), (std::int32_t)list_args[i],arg_typeTable))
					return false;
			}

			return true;
		}

		bool CheckArgList(const ButiEngine::List<std::int32_t>& arg_list_args, const TypeTable* arg_typeTable) const
		{
			// 引数が無い場合
			if (arg_list_args.IsEmpty())
				return list_args.IsEmpty();

			// 引数の個数が異なる
			if (arg_list_args.GetSize() != list_args.GetSize())
				return false;

			// 全引数の型をチェック
			//厳密チェック
			std::uint64_t size = list_args.GetSize();
			for (std::uint64_t i = 0; i < size; i++) {
				if (!TypeCheck(arg_list_args[i], static_cast<std::int32_t>(list_args[i]),arg_typeTable))
					return false;
			}
			return true;
		}


		bool CheckArgList_strict(const ButiEngine::List<ArgDefine>& arg_list_args) const
		{
			// 引数が無い場合
			if (arg_list_args.IsEmpty())
				return list_args.IsEmpty();

			// 引数の個数が異なる
			if (arg_list_args.GetSize() != list_args.GetSize())
				return false;

			// 全引数の型をチェック

			//厳密チェック
			std::uint64_t size = list_args.GetSize();
			for (std::uint64_t i = 0; i < size; i++) {
				if (!TypeCheck_strict(arg_list_args[i].GetType(), (std::int32_t)list_args[i]))
					return false;
			}

			return true;
		}

		bool CheckArgList_strict(const ButiEngine::List<std::int32_t>& arg_list_args) const
		{
			// 引数が無い場合
			if (arg_list_args.IsEmpty())
				return list_args.IsEmpty();

			// 引数の個数が異なる
			if (arg_list_args.GetSize() != list_args.GetSize())
				return false;

			// 全引数の型をチェック
			//厳密チェック
			std::uint64_t size = list_args.GetSize();
			for (std::uint64_t i = 0; i < size; i++) {
				if (!TypeCheck_strict(arg_list_args[i], (std::int32_t)list_args[i]))
					return false;
			}
			return true;
		}

		// 指定の引数の型を得る

		std::int32_t GetArg(const std::int32_t arg_index) const
		{
			return list_args[arg_index];
		}

		std::int32_t ArgSize() const { return (std::int32_t)list_args.GetSize(); }

		void SetIndex(const std::int32_t arg_index) { index = arg_index; }
		void SetDeclaration() { flags |= flag_declaration; }	// 宣言
		void SetDefinition() { flags |= flag_definition; }		// 定義
		void SetSystem() { flags |= flag_system; }

		std::int32_t GetIndex() const { 
			return index; 
		}
		bool IsDeclaration() const { return (flags & flag_declaration) != 0; }
		bool IsDefinition() const { return (flags & flag_definition) != 0; }
		bool IsSystem() const { return (flags & flag_system) != 0; }
		const std::string& GetName() const{ return name; }
		inline std::string GetNameWithArgment(const TypeTable& arg_typeTable)const;
		AccessModifier GetAccessType()const {
			return accessType;
		}
		void SetAccessType(AccessModifier arg_modifier) {
			accessType = arg_modifier;
		}
		void FileOutput(std::ofstream& arg_fOut)const {
			arg_fOut.write((char*)&valueType, sizeof(valueType));
			arg_fOut.write((char*)&flags, sizeof(flags));
			arg_fOut.write((char*)&index, sizeof(index));
			std::int32_t argsSize = list_args.GetSize();
			arg_fOut.write((char*)&argsSize, sizeof(argsSize));
			for (std::int32_t i = 0; i < argsSize; i++) {
				arg_fOut.write((char*)&list_args[i], sizeof(list_args[i]));
			}
			std::int32_t size = name.size();
			arg_fOut.write((char*)&size, sizeof(size));
			arg_fOut.write(name.c_str(), (size));
		}
		void FileInput(ButiEngine::Value_ptr<ButiEngine::IBinaryReader> arg_reader) {
			valueType = arg_reader->ReadVariable<std::int32_t>();
			flags = arg_reader->ReadVariable<std::int32_t>();
			index = arg_reader->ReadVariable<std::int32_t>();
			std::int32_t argsSize = arg_reader->ReadVariable<std::int32_t>();
			for (std::int32_t i = 0; i < argsSize; i++) {
				std::int32_t arg=arg_reader->ReadVariable<std::int32_t>();
				list_args.Add(arg);
			}
			std::int32_t size = arg_reader->ReadVariable<std::int32_t>();
			name = arg_reader->ReadCharactor(size);
		}
		void SetTemplateType(const ButiEngine::List<std::int32_t>& arg_list_template) {list_templateTypes = arg_list_template; }
		bool IsTemplate()const { return list_templateTypes.GetSize(); }
		std::string GetTemplateNames(const TypeTable* arg_table)const;
		std::int32_t		valueType = 0;
		std::int32_t		flags = 0;
		std::int32_t		index = 0;
		ButiEngine::List<std::int32_t>	list_args;
		std::string name;
		AccessModifier accessType = AccessModifier::Public;
		bool isLambda=false;
		ButiEngine::List<std::int32_t> list_captureList;
		ButiEngine::List<std::int32_t> list_templateTypes;
	};

class FunctionTable {
	private:
		using iter = std::map<std::string, FunctionTag>::iterator;
		using const_iter = std::map<std::string, FunctionTag>::const_iterator;

	public:
		FunctionTable()
		{
		}


		FunctionTag* Add(const std::string& arg_name, const FunctionTag& arg_tag,const TypeTable* arg_table)
		{
			auto key = arg_name+arg_tag.GetTemplateNames(arg_table);
			auto result = map_functions.emplace(key, arg_tag);

			return &result->second;
		}

		const FunctionTag* Find_strict(const std::string& arg_name, const ButiEngine::List<std::int32_t>& arg_list_args, const TypeTable* arg_typeTable) const
		{
			const_iter itr = map_functions.find(arg_name);
			if (itr == map_functions.end()) {
				return nullptr;
			}
			auto end = map_functions.upper_bound(arg_name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList_strict(arg_list_args)) {

					return &itr->second;
				}
			}
			return nullptr;
		}

		FunctionTag* Find_strict(const std::string& arg_name, const ButiEngine::List<ArgDefine>& arg_list_args, const TypeTable* arg_typeTable)
		{
			iter itr = map_functions.find(arg_name);

			if (itr == map_functions.end()) {
				return nullptr;
			}

			auto end = map_functions.upper_bound(arg_name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList_strict(arg_list_args)) {

					return &itr->second;
				}
			}
			return nullptr;
		}

		const FunctionTag* Find(const std::string& arg_name, const ButiEngine::List<std::int32_t>& arg_list_args,const TypeTable* arg_typeTable) const
		{
			const_iter itr = map_functions.find(arg_name);
			if (itr == map_functions.end()) {
				return nullptr;
			}
			auto end = map_functions.upper_bound(arg_name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList(arg_list_args,arg_typeTable)) {
					return &itr->second;
				}
			}
			return nullptr;
		}

		FunctionTag* Find(const std::string& arg_name, const ButiEngine::List<ArgDefine>& arg_list_args, const TypeTable* arg_typeTable)
		{
			iter itr = map_functions.find(arg_name);

			if (itr == map_functions.end()) {
				return nullptr;
			}

			auto end = map_functions.upper_bound(arg_name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList(arg_list_args,arg_typeTable)) {

					return &itr->second;
				}
			}
			return nullptr;
		}

		const FunctionTag* Find(const std::string& arg_name) const
		{
			const_iter itr = map_functions.find(arg_name);
			if (itr == map_functions.end()) {
				return nullptr;
			}
			return &itr->second;
		}

		FunctionTag* Find(const std::string& arg_name)
		{
			iter itr = map_functions.find(arg_name);
			if (itr == map_functions.end()) {
				return nullptr;
			}
			return &itr->second;
		}

		void Clear()
		{
			map_functions.clear();
		}

		void Clear_notSystem() {
			for (auto funcItr = map_functions.begin(); funcItr != map_functions.end(); ) {
				if (!funcItr->second.IsSystem()) {
					funcItr = map_functions.erase(funcItr);
				}
				else {
					funcItr++;
				}
			}
		}
		std::int32_t Size()const {
			return map_functions.size();
		}

		FunctionTag* operator[](const std::int32_t arg_index) {
			auto itr = map_functions.begin();
			std::int32_t maxCount = min(map_functions.size(),arg_index);
			for (std::int32_t i = 0; i < maxCount; i++) {
				itr++;
			}

			return &(itr->second);
		}
		
		void FileOutput(std::ofstream& arg_fOut) const{
			std::int32_t functionsSize = map_functions.size();
			arg_fOut.write((char*)&functionsSize, sizeof(functionsSize));
			auto end = map_functions.end();
			for (auto itr = map_functions.begin(); itr !=end;itr++) {
				std::int32_t size = itr->first.size();
				arg_fOut.write((char*)&size, sizeof(size));
				arg_fOut.write(itr->first.c_str(), size);
				itr->second.FileOutput(arg_fOut);
			}

		}

		void FileInput(ButiEngine::Value_ptr<ButiEngine::IBinaryReader> arg_reader) {
			std::int32_t functionsSize = arg_reader->ReadVariable<std::int32_t>();
			for (std::int32_t i = 0; i < functionsSize; i++) {
				std::int32_t size = arg_reader->ReadVariable<std::int32_t>();
				std::string functionStr = arg_reader->ReadCharactor(size);
				FunctionTag tag(functionStr);
				tag.FileInput(arg_reader);
				map_functions.emplace(functionStr, tag);
			}

		}

	protected:
		std::multimap<std::string, FunctionTag> map_functions;
	};

class VirtualMachine;
using OperationFunction = void (VirtualMachine::*)();

/// <summary>
/// スクリプト側で定義するクラスの情報
/// </summary>
class ScriptClassInfo {
public:
	std::string ToString()const {
		return "ScriptClass!";
	}
	std::int32_t GetTypeIndex()const {
		return typeIndex;
	}
	std::int32_t GetMemberTypeIndex(const std::int32_t arg_index)const {
		return list_memberTypes[arg_index];
	}
	std::int32_t GetMemberSize()const {
		return list_memberTypes.GetSize();
	}
	void SetTypeIndex(const std::int32_t arg_index) {
		typeIndex = arg_index;
	}
	void SetMemberTypes(const ButiEngine::List<std::int32_t> arg_list_types) {
		list_memberTypes = arg_list_types;
	}
	void SetMemberNames(const ButiEngine::List<std::string> arg_list_names) {
		list_memberName = arg_list_names;
	}
	const ButiEngine::List<std::string>& GetMamberName()const {
		return list_memberName;
	}
	void SetClassName(const std::string& arg_className) {
		className = arg_className;
	}

	bool operator ==(const ScriptClassInfo& other) const{
		if ((other.typeIndex != typeIndex) ||(other.systemTypeCount!=systemTypeCount)||(other.list_memberTypes.GetSize()!=list_memberTypes.GetSize() )) {
			return false;
		}
		for (auto itr = list_memberTypes.begin(), end = list_memberTypes.end(), otherItr = other.list_memberTypes.begin(); itr != end; itr++, otherItr++) {
			if ((*itr) != (*otherItr)) {
				return false;
			}
		}
		return true;
	}

	bool operator !=(const ScriptClassInfo& other)const {
		return !(*this == other);
	}

	void OutputFile(std::ofstream& arg_fOut) const {
		std::int32_t size = className.size();
		arg_fOut.write((char*)&size, sizeof(std::int32_t));
		arg_fOut.write(className.c_str(), size);
		arg_fOut.write((char*)&typeIndex, sizeof(std::int32_t));
		std::int32_t memberSize = list_memberTypes.GetSize();
		arg_fOut.write((char*)&memberSize, sizeof(std::int32_t));
		for (std::int32_t i = 0; i < memberSize; i++) {
			arg_fOut.write((char*)&list_memberTypes[i], sizeof(std::int32_t));
		}
		for (std::int32_t i = 0; i < memberSize; i++) {
			std::int32_t size = list_memberName[i].size();
			arg_fOut.write((char*)&size, sizeof(std::int32_t));
			arg_fOut.write(list_memberName[i].c_str(), size);
		}

	}
	void InputFile(ButiEngine::Value_ptr<ButiEngine::IBinaryReader> arg_reader) {
		std::int32_t size = arg_reader->ReadVariable<std::int32_t>();
		className = arg_reader->ReadCharactor(size);

		typeIndex= arg_reader->ReadVariable<std::int32_t>();
		std::int32_t memberSize = arg_reader->ReadVariable<std::int32_t>();
		list_memberTypes.Resize(memberSize);
		for (std::int32_t i = 0; i < memberSize; i++) {
			list_memberTypes[i] = arg_reader->ReadVariable<std::int32_t>();
		}
		for (std::int32_t i = 0; i < memberSize; i++) {
			std::int32_t size = arg_reader->ReadVariable<std::int32_t>();
			list_memberName.Add(arg_reader->ReadCharactor(size));
		}

	}
	std::int32_t GetSystemTypeCount()const { return typeIndex; }
	void SetSystemTypeCount(const std::int32_t arg_systemTypeCount) { systemTypeCount = arg_systemTypeCount; }
private:
	std::int32_t typeIndex;
	std::int32_t systemTypeCount;
	ButiEngine::List<std::int32_t> list_memberTypes;
	ButiEngine::List<std::string> list_memberName;
	std::string className;
};
struct MemberValueInfo {
	std::int32_t index;
	std::int32_t type;
	AccessModifier access = AccessModifier::Public;
};
struct FunctionObjectTypeData {
	FunctionObjectTypeData(const std::int32_t arg_retType,const ButiEngine::List<std::int32_t>& arg_argTypes):returnType(arg_retType),list_argTypes(arg_argTypes){}
	std::int32_t returnType;
	ButiEngine::List<std::int32_t> list_argTypes;
};
struct TypeTag {
	TypeTag() {}
	void Release() { 
		if (p_functionObjectData){
		delete p_functionObjectData; 
		}
	}
	//生成用アドレス
	OperationFunction typeFunc;
	//参照型生成用アドレス
	OperationFunction refTypeFunc;
	//型情報
	std::int32_t typeIndex;
	//型名
	std::string typeName;
	//引数記号
	std::string argName;
	//メンバ変数
	std::map<std::string, MemberValueInfo> map_memberValue;
	//メソッド
	FunctionTable methods;

	FunctionTag* AddMethod(const std::string& arg_methodName, const FunctionTag& arg_method,const TypeTable* arg_table) {
		return methods.Add(arg_methodName, arg_method, arg_table);
	}
	bool isSystem = false, isShared = false;
	bool IsFunctionObjectType()const { return p_functionObjectData; }
	EnumTag* p_enumTag = nullptr;
	FunctionObjectTypeData* p_functionObjectData=nullptr;
	std::int32_t GetFunctionObjectReturnType()const;
	std::int32_t GetFunctionObjectArgSize()const;
	const ButiEngine::List<std::int32_t>& GetFunctionObjectArgment()const;
	ScriptClassInfo GetScriptTypeInfo()const {
		if (isSystem||p_functionObjectData) {
			//組み込み型なのでスクリプト型定義は作れない
			assert(0);
			return ScriptClassInfo();
		}
		ScriptClassInfo output;
		output.SetClassName(typeName); 
		output.SetTypeIndex(typeIndex);
		ButiEngine::List<std::int32_t> list_types;
		ButiEngine::List<std::string> list_memberNames;
		list_types.Resize(map_memberValue.size());
		list_memberNames.Resize(map_memberValue.size());
		auto end = map_memberValue.end();
		for (auto itr = map_memberValue.begin(); itr !=end ; itr++) {
			list_types[itr->second.index] = itr->second.type;
			list_memberNames[itr->second.index] = itr->first;
		}

		output.SetMemberTypes(list_types);
		output.SetMemberNames(list_memberNames);

		return output;

	}

};

//型定義用

class TypeTable {
public:
	void Release();
	const TypeTag* GetType(const std::int32_t arg_index) const {
		if (list_types.GetSize() <= arg_index||arg_index<0) {
			return nullptr;
		}
		return list_types[arg_index];
	}
	TypeTag* GetType(const std::int32_t arg_index) {
		if (list_types.GetSize() <= arg_index || arg_index < 0) {
			return nullptr;
		}
		return list_types[arg_index];
	}
	const std::map<std::string, std::int32_t>& GetArgmentKeyMap()const {
		return map_argmentChars;
	}

	const TypeTag* GetType(const std::string& arg_typename)const {
		if (!map_types.count(arg_typename)) {
			return nullptr;
		}
		return &map_types.at(arg_typename);
	}
	TypeTag* GetType(const std::string& arg_typename) {
		if (!map_types.count(arg_typename)) {
			return nullptr;
		}
		return &map_types.at(arg_typename);
	}
	const TypeTag* GetFunctionType(const ButiEngine::List<std::int32_t>& arg_argmentTypes, const std::int32_t arg_retType)const {
		auto functionTypeName = "FunctionType:" + std::to_string(arg_retType);
		for (auto i = 0; i < arg_argmentTypes.GetSize(); i++) {
			functionTypeName += "," + std::to_string(arg_argmentTypes[i]);
		}
		if (!map_types.count(functionTypeName)) {
			return nullptr;
		}
		return &map_types.at(functionTypeName);
	}
	TypeTag* GetFunctionType(const ButiEngine::List<std::int32_t>& arg_argmentTypes, const std::int32_t arg_retType) {
		auto functionTypeName = "FunctionType:" + std::to_string(arg_retType);
		for (auto i = 0; i < arg_argmentTypes.GetSize(); i++) {
			functionTypeName += "," + std::to_string(arg_argmentTypes[i]);
		}
		if (!map_types.count(functionTypeName)) {
			return nullptr;
		}
		return &map_types.at(functionTypeName);
	}
	TypeTag* CreateFunctionType(const ButiEngine::List<std::int32_t>& arg_argmentTypes, const std::int32_t arg_retType) {
		auto functionTypeName = "FunctionType:" + std::to_string(arg_retType);
		for (auto i = 0; i < arg_argmentTypes.GetSize(); i++) {
			functionTypeName += "," + std::to_string(arg_argmentTypes[i]);
		}
		if (map_types.count(functionTypeName)) {
			return &map_types.at(functionTypeName);
		}
		TypeTag functionType;
		functionType.typeName = functionTypeName;
		functionType.argName = functionTypeName;

		functionType.typeIndex = list_types.GetSize();
		functionType.isSystem = false;
		if (list_types.GetSize() <= functionType.typeIndex) {
			list_types.Resize(functionType.typeIndex + 1);
		}
		map_argmentChars.emplace(functionTypeName, functionType.typeIndex);
		map_types.emplace(functionTypeName, functionType);
		list_types[functionType.typeIndex] = &map_types.at(functionTypeName);
		map_types.at(functionTypeName).p_functionObjectData = new FunctionObjectTypeData(arg_retType,arg_argmentTypes);

		functionTypeCount++;
		return &map_types.at(functionTypeName);
	}


	void RegistType(const TypeTag& arg_type) {


		if (list_types.GetSize() <= arg_type.typeIndex) {
			list_types.Resize(arg_type.typeIndex + 1);
		}
		map_argmentChars.emplace(arg_type.argName, arg_type.typeIndex);
		map_types.emplace(arg_type.typeName, arg_type);
		list_types[arg_type.typeIndex] = &map_types.at(arg_type.typeName);
		if (arg_type.isSystem) {
			systemTypeCount++;
		}
	}
	void RegistType(EnumTag& arg_type) {
		TypeTag enumType;
		enumType.typeName = arg_type.GetTypeName();
		enumType.argName = arg_type.GetTypeName();
		enumType.p_enumTag = &arg_type;
		enumType.typeIndex = list_types.GetSize();
		arg_type.typeIndex = enumType.typeIndex;
		enumType.isSystem = arg_type.isSystem;
		if (list_types.GetSize() <= enumType.typeIndex) {
			list_types.Resize(enumType.typeIndex + 1);
		}
		map_argmentChars.emplace(arg_type.GetTypeName(), enumType.typeIndex);
		map_types.emplace(arg_type.GetTypeName(), enumType);
		list_types[enumType.typeIndex] = &map_types.at(arg_type.GetTypeName());
		if (arg_type.isSystem) {
			systemTypeCount++;
		}
	}

	const ButiEngine::List<TypeTag* >& GetSystemType()const {
		return list_types;
	}

	void CreateTypeVec(ButiEngine::List<TypeTag>& arg_ref_types) const {
		arg_ref_types.Reserve(list_types.GetSize());
		auto end = list_types.end();
		for (auto itr = list_types.begin(); itr != end; itr++) {
			arg_ref_types.Add(*(*itr));
		}

	}


	void Clear_notSystem() {
		for (auto itr = list_types.begin(); itr != list_types.end();) {
			if (!(*itr)->isSystem) {
				itr = list_types.erase(itr);
			}
			else {
				itr++;
			}
		}
		for (auto typeItr = map_types.begin(); typeItr != map_types.end(); ) {
			if (!typeItr->second.isSystem) {
				map_argmentChars.erase(typeItr->first);
				typeItr->second.Release();
				typeItr = map_types.erase(typeItr);
			}
			else {
				typeItr++;
			}
		}
		functionTypeCount = 0;
	}
	std::int32_t GetSize()const {
		return list_types.GetSize();
	}
	std::int32_t GetSystemTypeSize()const {
		return systemTypeCount;
	}
	std::int32_t GetFunctionTypeSize()const {
		return functionTypeCount;
	}
	std::int32_t GetScriptTypeSize()const {
		return list_types.GetSize()-systemTypeCount-functionTypeCount;
	}
	ButiEngine::List<ScriptClassInfo> GetScriptClassInfo()const {
		ButiEngine::List<ScriptClassInfo> output;
		for (std::int32_t i = 0; i < list_types.GetSize(); i++) {
			if (list_types[i]->isSystem|| list_types[i]->p_functionObjectData){ continue; }
				
			output.Add(list_types[i]->GetScriptTypeInfo());

		}

		return output;
	}
private:

	ButiEngine::List<TypeTag* > list_types;
	std::map<std::string, std::int32_t> map_argmentChars;
	std::map<std::string, TypeTag> map_types;
	std::int32_t systemTypeCount=0,functionTypeCount=0;
};

bool TypeCheck(const std::int32_t arg_left, const std::int32_t arg_right, const TypeTable* arg_table) {
	std::int32_t left = arg_left & ~TYPE_REF;
	std::int32_t right = arg_right & ~TYPE_REF;
	if (!arg_table->GetType(left) || !arg_table->GetType(right)) {
		return false;
	}
	//同じ型のチェック
	if (left == right) {
		return true;
	}

	//暗黙キャスト可能か
	if ((left == TYPE_INTEGER && right == TYPE_FLOAT) || (left == TYPE_FLOAT && right == TYPE_INTEGER) ||
		(left == TYPE_INTEGER && arg_table->GetType(right)->p_enumTag) || (right == TYPE_INTEGER && arg_table->GetType(left)->p_enumTag) || (arg_table->GetType(left)->p_enumTag && arg_table->GetType(right)->p_enumTag)) {
		return true;
	}

	return false;
}
}
inline std::string ButiScript::FunctionTag::GetNameWithArgment(const TypeTable& arg_typeTable) const
{
	std::string output = name;

	if (list_args.GetSize()) {
		output += ":";
	}

	for (std::int32_t i = 0; i < list_args.GetSize(); i++) {
		output += arg_typeTable.GetType(list_args[i] & ~TYPE_REF)->argName;
		if (i + 1 != list_args.GetSize()) {
			output += ",";
		}
	}
	return output;
}

static std::string GetTemplateName(const ButiEngine::List<std::int32_t>& arg_list_temps, const ButiScript::TypeTable* arg_table) {


	if (!arg_list_temps.GetSize()) {
		return "";
	}
	std::string output = "<";
	for (std::int32_t i = 0; i < arg_list_temps.GetSize(); i++) {
		if (i != 0) {
			output += ",";
		}
		output += arg_table->GetType(arg_list_temps[i])->typeName;
	}
	output += ">";
	return output;
}

inline std::string ButiScript::FunctionTag::GetTemplateNames(const TypeTable* arg_table) const
{
	return GetTemplateName(list_templateTypes,arg_table);
}
#endif // !TAGS_H
