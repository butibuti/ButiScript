#pragma once
#ifndef TAGS_H
#define TAGS_H
#include"value_type.h"
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

		ArgDefine(const int arg_type)
			: valueType(arg_type)
		{
		}
		ArgDefine(const int arg_type, const std::string& arg_name)
			: valueType(arg_type), name(arg_name)
		{
		}

		void SetRef()
		{
			valueType |= TYPE_REF;
		}

		void SetName(const std::string& arg_name)
		{
			name = arg_name;
		}

		int GetType() const { return valueType; }
		const std::string& GetName() const { return name; }

	private:
		int valueType;
		std::string name;
	};
	class EnumTag {
	public:
		EnumTag(){}
		EnumTag(const std::string& arg_typeName):type_Name(arg_typeName){}

		void SetValue(const std::string& arg_identiferName, const int arg_value) {
			if (map_identifers.count(arg_identiferName)) {
				return;
			}
			map_identifers.emplace(arg_identiferName, arg_value);
		}
		int GetValue(const std::string& arg_identiferName)const {
			return map_identifers.at(arg_identiferName);
		}
		bool ExistenceIdentifers(const std::string& arg_identiferName)const {
			return map_identifers.count(arg_identiferName);
		}
		const std::string& GetTypeName()const {
			return type_Name;
		}
		void OutputFile(std::ofstream& arg_out)const {
			int nameSize = type_Name.size();
			arg_out.write((char*)&nameSize, sizeof(int));
			arg_out.write(type_Name.c_str(), nameSize);

			int identSize = map_identifers.size();
			arg_out.write((char*)&identSize, sizeof(int));
			for (auto itr = map_identifers.begin(), end = map_identifers.end(); itr != end; itr++) {
				int identiferSize = itr->first.size();
				arg_out.write((char*)&identiferSize, sizeof(int));
				arg_out.write(itr->first.c_str(), identiferSize);
				arg_out.write((char*)&itr->second, sizeof(int));

			}
		}
		void InputFile(std::ifstream& arg_in) {
			int nameSize = 0;
			arg_in.read((char*)&nameSize, sizeof(int));
			char* buff =(char*) malloc(nameSize);
			arg_in.read(buff, nameSize);
			type_Name = std::string(buff);
			delete buff;
			int identSize = 0;
			arg_in.read((char*)&identSize, sizeof(int));
			for (int i = 0; i < identSize;i++) {
				int identiferSize = 0;
				arg_in.read((char*)&identiferSize, sizeof(int));
				buff = (char*)malloc(identiferSize);
				arg_in.read(buff, identiferSize);
				auto identifer = std::string(buff);
				delete buff;
				int value = 0;
				arg_in.read((char*)&value, sizeof(int));
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
		const std::map<int, std::string>& GetIdentiferMap()const {
			return map_identifer_value;
		}
		bool isSystem;
		int typeIndex;
	private:
		std::string type_Name;
		std::map<std::string, int>map_identifers;
		std::map<int, std::string>map_identifer_value;
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
		int Size()const {
			return map_enumTag.size();
		}
		EnumTag* operator[](const int arg_index) {
			auto itr = map_enumTag.begin();
			for (int i = 0; i < arg_index; i++) { itr++; }
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
		ValueTag(const int arg_addr, const int arg_type, const int arg_size, const bool arg_global ,const AccessModifier arg_access)
			: address(arg_addr),originalAddress(arg_addr), valueType(arg_type), currentSize(arg_size), isGlobal(arg_global),access(arg_access)
		{
		}

		int		valueType;
		int		currentSize;
		bool	isGlobal;
		AccessModifier access=AccessModifier::Public;
		int GetAddress()const { return address; }
		
	private:

		int		address,originalAddress;
	};
	class ValueTable {
	private:
		using iter = std::map<std::string, ValueTag>::iterator;
		using const_iter = std::map<std::string, ValueTag>::const_iterator;

	public:
		ValueTable(const int arg_start_addr = 0, const bool arg_isFunctionBlockTable = false) : addr(arg_start_addr), isGlobal(false), isFunction(true)
		{
		}

		void set_global()
		{
			isGlobal = true;
		}

		bool Add(const int arg_type, const std::string& arg_name,const AccessModifier arg_access, const int arg_size = 1)
		{
			if (!map_variables.count(arg_name)) {
				map_variables.emplace(arg_name, ValueTag(addr, arg_type, arg_size, isGlobal,arg_access));
				vec_variableTypes.push_back(arg_type);
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

		bool add_arg(const int arg_type, const std::string& arg_name, const int arg_addr)
		{
			std::pair<iter, bool> result = map_variables.insert(make_pair(arg_name, ValueTag(arg_addr, arg_type, 1, false,AccessModifier::Public)));
			argmentsCount++;
			return result.second;
		}

		int size() const { return addr; }
		int valueCount()const { return map_variables.size(); }

		int AddressAdd(const int arg_v) {
			auto difference = arg_v +( map_variables.size() - 1-argmentsCount);
			for (auto itr = map_variables.begin(), end = map_variables.end(); itr != end; itr++) {
				itr->second.address = itr->second.originalAddress+ difference;
			}
			return difference+argmentsCount;
		}
		int AddressSub(const int arg_v) {

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
		ValueTag& operator[] (const int arg_index) {
			auto itr = map_variables.begin();
			for (int i = 0; i < arg_index; i++) {
				itr++;
			}

			return itr->second;
		}

		const std::string& GetVariableName(const int arg_index)const {
			auto itr = map_variables.begin();
			for (int i = 0; i < arg_index; i++) {
				itr++;
			}

			return itr->first;
		}

		bool IsFunctionBlock()const { return isFunction; }
#ifdef	_DEBUG
		struct DumpAction {
			void operator()(const std::pair<std::string, ValueTag>& it)
			{
#ifdef BUTIGUI_H
				ButiEngine::GUI::Console(it.first + ", addr = " + std::to_string( it.second.address )+ ", type = " + std::to_string(it.second.valueType )+ ", size = " +std::to_string( it.second.currentSize )+ ", global = " + std::to_string( it.second.isGlobal));
#else
				std::cout << it.first << ", addr = " << it.second.GetAddress() << ", type = " << it.second.valueType << ", size = " << it.second.currentSize << ", global = " << it.second.isGlobal << std::endl;

#endif // BUTIGUI_H
				}
		};
		void Dump() const
		{

#ifdef BUTIGUI_H
			ButiEngine::GUI::Console("-------- value --------\n");
#else
			std::cout << "-------- value --------" << std::endl;
#endif // BUTIGUI_H
			std::for_each(map_variables.begin(), map_variables.end(), DumpAction());
		}
#endif

	private:
		std::map <std::string, ValueTag> map_variables;
		std::vector < int> vec_variableTypes;
		int		addr,argmentsCount=0;
		bool	isGlobal,isFunction;
	};


	static bool TypeCheck(const int arg_left, const int arg_right) {
		int left = arg_left & ~TYPE_REF;
		int right = arg_right & ~TYPE_REF;
		//同じ型のチェック
		if (left == right) {
			return true;
		}

		//暗黙キャスト可能か
		if ((left == TYPE_INTEGER && right == TYPE_FLOAT) || (left == TYPE_FLOAT && right == TYPE_INTEGER)) {
			return true;
		}

		return false;
	}
	static bool TypeCheck_strict(const int arg_left, const int arg_right) {
		int left = arg_left & ~TYPE_REF;
		int right = arg_right & ~TYPE_REF;
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
		FunctionTag(const int arg_type,const std::string &arg_name) 
			: valueType(arg_type), flags(0), index(0), name(arg_name)
		{
		}
		void SetArg(const int arg_type)
		{
			vec_args.push_back(arg_type);
		}

		void SetArgs(const std::vector<ArgDefine>& arg_vec_args)
		{
			size_t size = arg_vec_args.size();
			for (size_t i = 0; i < size; i++) {
				vec_args.push_back(arg_vec_args[i].GetType());
			}
		}

		void SetArgs(const std::vector<int>& arg_vec_args)
		{
			size_t size = arg_vec_args.size();
			for (size_t i = 0; i < size; i++) {
				vec_args.push_back(arg_vec_args[i]);
			}
		}

		bool SetArgs(const std::string& args, const std::map<std::string, int>& arg_map_argmentChars)
		{
			if (args.size() == 0) {
				return true;
			}

			auto splited = std::vector<std::string>();
			int first = 0;
			int last = args.find_first_of(",");
			if (last == std::string::npos) {

				splited.push_back(args);
			}
			else {
				while (first < args.size())
				{
					auto subString = args.substr(first, last - first);
					splited.push_back(subString);
					first = last + 1;
					last = args.find_first_of(",", first);
					if (last == std::string::npos) {
						last = args.size();
					}
				}
			}

			for (int i = 0; i < splited.size(); i++) {
				if (!arg_map_argmentChars.count(splited[i])) {
					return false;
				}
				else {

					vec_args.push_back(arg_map_argmentChars.at(splited[i]));
				}

			}
			return true;
		}


		bool CheckArgList(const std::vector<ArgDefine>& arg_vec_args) const
		{
			// 引数が無い場合
			if (arg_vec_args.empty())
				return vec_args.empty();

			// 引数の個数が異なる
			if (arg_vec_args.size() != vec_args.size())
				return false;

			// 全引数の型をチェック

			//厳密チェック
			size_t size = vec_args.size();
			for (size_t i = 0; i < size; i++) {
				if (!TypeCheck(arg_vec_args[i].GetType(), (int)vec_args[i]))
					return false;
			}

			return true;
		}

		bool CheckArgList(const std::vector<int>& arg_vec_args) const
		{
			// 引数が無い場合
			if (arg_vec_args.empty())
				return vec_args.empty();

			// 引数の個数が異なる
			if (arg_vec_args.size() != vec_args.size())
				return false;

			// 全引数の型をチェック
			//厳密チェック
			size_t size = vec_args.size();
			for (size_t i = 0; i < size; i++) {
				if (!TypeCheck(arg_vec_args[i], (int)vec_args[i]))
					return false;
			}
			return true;
		}


		bool CheckArgList_strict(const std::vector<ArgDefine>& arg_vec_args) const
		{
			// 引数が無い場合
			if (arg_vec_args.empty())
				return vec_args.empty();

			// 引数の個数が異なる
			if (arg_vec_args.size() != vec_args.size())
				return false;

			// 全引数の型をチェック

			//厳密チェック
			size_t size = vec_args.size();
			for (size_t i = 0; i < size; i++) {
				if (!TypeCheck_strict(arg_vec_args[i].GetType(), (int)vec_args[i]))
					return false;
			}

			return true;
		}

		bool CheckArgList_strict(const std::vector<int>& arg_vec_args) const
		{
			// 引数が無い場合
			if (arg_vec_args.empty())
				return vec_args.empty();

			// 引数の個数が異なる
			if (arg_vec_args.size() != vec_args.size())
				return false;

			// 全引数の型をチェック
			//厳密チェック
			size_t size = vec_args.size();
			for (size_t i = 0; i < size; i++) {
				if (!TypeCheck_strict(arg_vec_args[i], (int)vec_args[i]))
					return false;
			}
			return true;
		}

		// 指定の引数の型を得る

		int GetArg(const int arg_index) const
		{
			return vec_args[arg_index];
		}

		int ArgSize() const { return (int)vec_args.size(); }

		void SetIndex(const int arg_index) { index = arg_index; }
		void SetDeclaration() { flags |= flag_declaration; }	// 宣言
		void SetDefinition() { flags |= flag_definition; }		// 定義
		void SetSystem() { flags |= flag_system; }

		int GetIndex() const { return index; }
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
			int argsSize = vec_args.size();
			arg_fOut.write((char*)&argsSize, sizeof(argsSize));
			for (int i = 0; i < argsSize; i++) {
				arg_fOut.write((char*)&vec_args[i], sizeof(vec_args[i]));
			}
			int size = name.size();
			arg_fOut.write((char*)&size, sizeof(size));
			arg_fOut.write(name.c_str(), (size));
		}
		void FileInput(std::ifstream& arg_fIn) {
			arg_fIn.read((char*)&valueType, sizeof(valueType));
			arg_fIn.read((char*)&flags, sizeof(flags));
			arg_fIn.read((char*)&index, sizeof(index));
			int argsSize = 0;
			arg_fIn.read((char*)&argsSize, sizeof(argsSize));
			for (int i = 0; i < argsSize; i++) {
				int arg;
				arg_fIn.read((char*)&arg, sizeof(arg));
				vec_args.push_back(arg);
			}
			int size = 0;
			arg_fIn.read((char*)&size, sizeof(size));
			char* buff=(char*)malloc(size);
			arg_fIn.read(buff, (size));
			name = std::string(buff, size);
			free(buff);
		}

	public:
		int		valueType = 0;
		int		flags = 0;
		int		index = 0;
		std::vector<int>	vec_args;
		std::string name;
		AccessModifier accessType = AccessModifier::Public;
	};

	class FunctionTable {
	private:
		using iter = std::map<std::string, FunctionTag>::iterator;
		using const_iter = std::map<std::string, FunctionTag>::const_iterator;

	public:
		FunctionTable()
		{
		}


		FunctionTag* Add(const std::string& arg_name, const FunctionTag& arg_tag)
		{
			auto key = arg_name;
			auto result = map_functions.emplace(key, arg_tag);

			return &result->second;
		}

		const FunctionTag* Find_strict(const std::string& arg_name, const std::vector<int>& arg_vec_args) const
		{
			const_iter itr = map_functions.find(arg_name);
			if (itr == map_functions.end()) {
				return nullptr;
			}
			auto end = map_functions.upper_bound(arg_name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList_strict(arg_vec_args)) {

					return &itr->second;
				}
			}
			return nullptr;
		}

		FunctionTag* Find_strict(const std::string& arg_name, const std::vector<ArgDefine>& arg_vec_args)
		{
			iter itr = map_functions.find(arg_name);

			if (itr == map_functions.end()) {
				return nullptr;
			}

			auto end = map_functions.upper_bound(arg_name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList_strict(arg_vec_args)) {

					return &itr->second;
				}
			}
			return nullptr;
		}

		const FunctionTag* Find(const std::string& arg_name, const std::vector<int>& arg_vec_args) const
		{
			const_iter itr = map_functions.find(arg_name);
			if (itr == map_functions.end()) {
				return nullptr;
			}
			auto end = map_functions.upper_bound(arg_name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList(arg_vec_args)) {

					return &itr->second;
				}
			}
			return nullptr;
		}

		FunctionTag* Find(const std::string& arg_name, const std::vector<ArgDefine>& arg_vec_args)
		{
			iter itr = map_functions.find(arg_name);

			if (itr == map_functions.end()) {
				return nullptr;
			}

			auto end = map_functions.upper_bound(arg_name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList(arg_vec_args)) {

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
		int Size()const {
			return map_functions.size();
		}

		FunctionTag* operator[](const int arg_index) {
			auto itr = map_functions.begin();
			int maxCount = min(map_functions.size(),arg_index);
			for (int i = 0; i < maxCount; i++) {
				itr++;
			}

			return &(itr->second);
		}
		
		void FileOutput(std::ofstream& arg_fOut) const{
			int functionsSize = map_functions.size();
			arg_fOut.write((char*)&functionsSize, sizeof(functionsSize));
			auto end = map_functions.end();
			for (auto itr = map_functions.begin(); itr !=end;itr++) {
				int size = itr->first.size();
				arg_fOut.write((char*)&size, sizeof(size));
				arg_fOut.write(itr->first.c_str(), size);
				itr->second.FileOutput(arg_fOut);
			}

		}

		void FileInput(std::ifstream& arg_fIn) {
			int functionsSize = 0;
			arg_fIn.read((char*)&functionsSize, sizeof(functionsSize));

			for (int i = 0; i < functionsSize; i++) {
				int size = 0;
				std::string functionStr;
				arg_fIn.read((char*)&size, sizeof(size));
				char* p_buff = (char*)malloc(size);
				arg_fIn.read(p_buff, size);
				functionStr =std::string( p_buff,size);
				free(p_buff);
				FunctionTag tag(functionStr);
				tag.FileInput(arg_fIn);
				map_functions.emplace(functionStr, tag);
			}

		}

	protected:
		std::multimap<std::string, FunctionTag> map_functions;
	};

	class VirtualCPU;
	using OperationFunction = void (VirtualCPU::*)();

	/// <summary>
	/// スクリプト側で定義するクラスの情報
	/// </summary>
	class ScriptClassInfo {
	public:
		std::string ToString()const {
			return "ScriptClass!";
		}
		int GetTypeIndex()const {
			return typeIndex;
		}
		int GetMemberTypeIndex(const int arg_index)const {
			return vec_memberTypes[arg_index];
		}
		int GetMemberSize()const {
			return vec_memberTypes.size();
		}
		void SetTypeIndex(const int arg_index) {
			typeIndex = arg_index;
		}
		void SetMemberTypes(const std::vector<int> arg_vec_types) {
			vec_memberTypes = arg_vec_types;
		}
		void SetMemberNames(const std::vector<std::string> arg_vec_names) {
			vec_memberName = arg_vec_names;
		}
		const std::vector<std::string>& GetMamberName()const {
			return vec_memberName;
		}
		void SetClassName(const std::string& arg_className) {
			className = arg_className;
		}
		void OutputFile(std::ofstream& arg_fOut) const {
			int size = className.size();
			arg_fOut.write((char*)&size, sizeof(int));
			arg_fOut.write(className.c_str(), size);
			arg_fOut.write((char*)&typeIndex, sizeof(int));
			int memberSize = vec_memberTypes.size();
			arg_fOut.write((char*)&memberSize, sizeof(int));
			for (int i = 0; i < memberSize; i++) {
				arg_fOut.write((char*)&vec_memberTypes[i], sizeof(int));
			}
			for (int i = 0; i < memberSize; i++) {
				int size = vec_memberName[i].size();
				arg_fOut.write((char*)&size, sizeof(int));
				arg_fOut.write(vec_memberName[i].c_str(), size);
			}

		}
		void InputFile(std::ifstream& arg_fIn) {
			int size = 0;
			arg_fIn.read((char*)&size, sizeof(int));
			char* nameBuff = (char*)malloc(size);
			arg_fIn.read(nameBuff, size);
			className = std::string(nameBuff, size);
			free(nameBuff);

			arg_fIn.read((char*)&typeIndex, sizeof(int));
			int memberSize = 0;
			arg_fIn.read((char*)&memberSize, sizeof(int));
			vec_memberTypes.resize(memberSize);
			for (int i = 0; i < memberSize; i++) {
				arg_fIn.read((char*)&vec_memberTypes[i], sizeof(int));
			}
			for (int i = 0; i < memberSize; i++) {
				int size = 0;
				arg_fIn.read((char*)&size, sizeof(int));
				char* nameBuff = (char*)malloc(size);
				arg_fIn.read(nameBuff, size);
				vec_memberName.push_back(std::string(nameBuff, size));
				free(nameBuff);
			}

		}
	private:
		int typeIndex;
		std::vector<int> vec_memberTypes;
		std::vector<std::string> vec_memberName;
		std::string className;
	};
	struct MemberValueInfo {
		int index;
		int type;
		AccessModifier access = AccessModifier::Public;
	};
	struct FunctionObjectTypeData {
		FunctionObjectTypeData(const int arg_retType,const std::vector<int>& arg_argTypes):returnType(arg_retType),vec_argTypes(arg_argTypes){}
		int returnType;
		std::vector<int> vec_argTypes;
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
		int typeIndex;
		//型名
		std::string typeName;
		//引数記号
		std::string argName;
		//メンバ変数
		std::map<std::string, MemberValueInfo> map_memberValue;
		//メソッド
		FunctionTable methods;

		FunctionTag* AddMethod(const std::string& arg_methodName, const FunctionTag& arg_method) {
			return methods.Add(arg_methodName, arg_method);
		}
		bool isSystem = false, isShared = false;
		bool IsFunctionObjectType()const { return p_functionObjectData; }
		EnumTag* p_enumTag = nullptr;
		FunctionObjectTypeData* p_functionObjectData=nullptr;
		int GetFunctionObjectReturnType()const;
		int GetFunctionObjectArgSize()const;
		const std::vector<int>& GetFunctionObjectArgment()const;
		ScriptClassInfo GetScriptTypeInfo()const {
			if (isSystem||p_functionObjectData) {
				//組み込み型なのでスクリプト型定義は作れない
				assert(0);
				return ScriptClassInfo();
			}
			ScriptClassInfo output;
			output.SetClassName(typeName); 
			output.SetTypeIndex(typeIndex);
			std::vector<int> vec_types;
			std::vector<std::string> vec_memberNames;
			vec_types.resize(map_memberValue.size());
			vec_memberNames.resize(map_memberValue.size());
			auto end = map_memberValue.end();
			for (auto itr = map_memberValue.begin(); itr !=end ; itr++) {
				vec_types[itr->second.index] = itr->second.type;
				vec_memberNames[itr->second.index] = itr->first;
			}

			output.SetMemberTypes(vec_types);
			output.SetMemberNames(vec_memberNames);

			return output;

		}

	};

	//型定義用

	class TypeTable {
	public:
		void Release();
		const TypeTag* GetType(const int arg_index) const {
			if (vec_types.size() <= arg_index) {
				return nullptr;
			}
			return vec_types[arg_index];
		}
		TypeTag* GetType(const int arg_index) {
			if (vec_types.size() <= arg_index) {
				return nullptr;
			}
			return vec_types[arg_index];
		}
		const std::map<std::string, int>& GetArgmentKeyMap()const {
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
		const TypeTag* GetFunctionType(const std::vector<int>& arg_argmentTypes, const int arg_retType)const {
			auto functionTypeName = "FunctionType:" + std::to_string(arg_retType);
			for (auto i = 0; i < arg_argmentTypes.size(); i++) {
				functionTypeName += "," + std::to_string(arg_argmentTypes[i]);
			}
			if (!map_types.count(functionTypeName)) {
				return nullptr;
			}
			return &map_types.at(functionTypeName);
		}
		TypeTag* GetFunctionType(const std::vector<int>& arg_argmentTypes, const int arg_retType) {
			auto functionTypeName = "FunctionType:" + std::to_string(arg_retType);
			for (auto i = 0; i < arg_argmentTypes.size(); i++) {
				functionTypeName += "," + std::to_string(arg_argmentTypes[i]);
			}
			if (!map_types.count(functionTypeName)) {
				return nullptr;
			}
			return &map_types.at(functionTypeName);
		}
		TypeTag* CreateFunctionType(const std::vector<int>& arg_argmentTypes, const int arg_retType) {
			auto functionTypeName = "FunctionType:" + std::to_string(arg_retType);
			for (auto i = 0; i < arg_argmentTypes.size(); i++) {
				functionTypeName += "," + std::to_string(arg_argmentTypes[i]);
			}
			if (map_types.count(functionTypeName)) {
				return &map_types.at(functionTypeName);
			}
			TypeTag functionType;
			functionType.typeName = functionTypeName;
			functionType.argName = functionTypeName;

			functionType.typeIndex = vec_types.size();
			functionType.isSystem = false;
			if (vec_types.size() <= functionType.typeIndex) {
				vec_types.resize(functionType.typeIndex + 1);
			}
			map_argmentChars.emplace(functionTypeName, functionType.typeIndex);
			map_types.emplace(functionTypeName, functionType);
			vec_types[functionType.typeIndex] = &map_types.at(functionTypeName);
			map_types.at(functionTypeName).p_functionObjectData = new FunctionObjectTypeData(arg_retType,arg_argmentTypes);

			return &map_types.at(functionTypeName);
		}


		void RegistType(const TypeTag& arg_type) {


			if (vec_types.size() <= arg_type.typeIndex) {
				vec_types.resize(arg_type.typeIndex + 1);
			}
			map_argmentChars.emplace(arg_type.argName, arg_type.typeIndex);
			map_types.emplace(arg_type.typeName, arg_type);
			vec_types[arg_type.typeIndex] = &map_types.at(arg_type.typeName);
			if (arg_type.isSystem) {
				systemTypeCount++;
			}
		}
		void RegistType(EnumTag& arg_type) {
			TypeTag enumType;
			enumType.typeName = arg_type.GetTypeName();
			enumType.argName = arg_type.GetTypeName();
			enumType.p_enumTag = &arg_type;
			enumType.typeIndex = vec_types.size();
			arg_type.typeIndex = enumType.typeIndex;
			enumType.isSystem = arg_type.isSystem;
			if (vec_types.size() <= enumType.typeIndex) {
				vec_types.resize(enumType.typeIndex + 1);
			}
			map_argmentChars.emplace(arg_type.GetTypeName(), enumType.typeIndex);
			map_types.emplace(arg_type.GetTypeName(), enumType);
			vec_types[enumType.typeIndex] = &map_types.at(arg_type.GetTypeName());
			if (arg_type.isSystem) {
				systemTypeCount++;
			}
		}

		const std::vector<TypeTag* >& GetSystemType()const {
			return vec_types;
		}

		void CreateTypeVec(std::vector<TypeTag>& arg_ref_types) const {
			arg_ref_types.reserve(vec_types.size());
			auto end = vec_types.end();
			for (auto itr = vec_types.begin(); itr != end; itr++) {
				arg_ref_types.push_back(*(*itr));
			}

		}


		void Clear_notSystem() {
			for (auto itr = vec_types.begin(); itr != vec_types.end();) {
				if (!(*itr)->isSystem) {
					itr = vec_types.erase(itr);
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
		}
		int GetSize()const {
			return vec_types.size();
		}
		int GetSystemTypeSize()const {
			return systemTypeCount;
		}
		int GetScriptTypeSize()const {
			return vec_types.size()-systemTypeCount;
		}
		std::vector<ScriptClassInfo> GetScriptClassInfo()const {
			std::vector<ScriptClassInfo> output;
			for (int i = 0; i < vec_types.size(); i++) {
				if (vec_types[i]->isSystem|| vec_types[i]->p_functionObjectData){ continue; }
					
				output.push_back(vec_types[i]->GetScriptTypeInfo());

			}

			return output;
		}
	private:

		std::vector<TypeTag* > vec_types;
		std::map<std::string, int> map_argmentChars;
		std::map<std::string, TypeTag> map_types;
		int systemTypeCount=0;
	};
	
}
inline std::string ButiScript::FunctionTag::GetNameWithArgment(const TypeTable& arg_typeTable) const
{
	std::string output = name;

	if (vec_args.size()) {
		output += ":";
	}

	for (int i = 0; i < vec_args.size(); i++) {
		output += arg_typeTable.GetType(vec_args[i] & ~TYPE_REF)->argName;
		if (i + 1 != vec_args.size()) {
			output += ",";
		}
	}
	return output;
}
#endif // !TAGS_H
