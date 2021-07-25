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
			: valueType(arg_type), name_(arg_name)
		{
		}

		void set_ref()
		{
			valueType |= TYPE_REF;
		}

		void set_name(const std::string& arg_name)
		{
			name_ = arg_name;
		}

		int type() const { return valueType; }
		const std::string& name() const { return name_; }

	private:
		int valueType;
		std::string name_;
	};
	class EnumTag {
	public:
		EnumTag(){}
		EnumTag(const std::string& arg_typeName):type_Name(arg_typeName){}

		void SetValue(const std::string& arg_identiferName, const int value) {
			if (map_identifers.count(arg_identiferName)) {
				return;
			}
			map_identifers.emplace(arg_identiferName, value);
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
		bool isSystem;
	private:
		std::string type_Name;
		std::map<std::string, int>map_identifers;
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
	private:
		std::map<std::string, EnumTag> map_enumTag;
	};

	class ValueTag {
	public:
		ValueTag() : address(-1), valueType(TYPE_INTEGER), size_(1), global_(false)
		{
		}
		ValueTag(const int addr, const int type, const int size, const bool global ,const AccessModifier arg_access)
			: address(addr), valueType(type), size_(size), global_(global),access(arg_access)
		{
		}

	public:
		int		address;
		int		valueType;
		int		size_;
		bool	global_;
		AccessModifier access=AccessModifier::Public;
	};
	class ValueTable {
	private:
		using iter = std::map<std::string, ValueTag>::iterator;
		using const_iter = std::map<std::string, ValueTag>::const_iterator;

	public:
		ValueTable(const int start_addr = 0) : addr_(start_addr), global_(false)
		{
		}

		void set_global()
		{
			global_ = true;
		}

		bool Add(const int type, const std::string& name,const AccessModifier access, const int size = 1)
		{
			if (!variables_.count(name)) {
				variables_.emplace(name, ValueTag(addr_, type, size, global_,access));
				vec_variableTypes.push_back(type);
				addr_ += size;
				return true;
			}
			return false;
		}

		const ValueTag* find(const std::string& name) const
		{
			const_iter it = variables_.find(name);
			if (it != variables_.end())
				return &it->second;
			return nullptr;
		}

		bool add_arg(const int type, const std::string& name, const int addr)
		{
			std::pair<iter, bool> result = variables_.insert(make_pair(name, ValueTag(addr, type, 1, false,AccessModifier::Public)));
			return result.second;
		}

		int size() const { return addr_; }
		void Clear()
		{
			variables_.clear();
			addr_ = 0;
		}

		void Alloc(Compiler* arg_comp)const;
		ValueTag& operator[] (const int index) {
			auto itr = variables_.begin();
			for (int i = 0; i < index; i++) {
				itr++;
			}

			return itr->second;
		}

		const std::string& GetVariableName(const int index)const {
			auto itr = variables_.begin();
			for (int i = 0; i < index; i++) {
				itr++;
			}

			return itr->first;
		}

#ifdef	_DEBUG
		struct DumpAction {
			void operator()(const std::pair<std::string, ValueTag>& it)
			{
				std::cout << it.first << ", addr = " << it.second.address << ", type = " << it.second.valueType << ", size = " << it.second.size_ << ", global = " << it.second.global_ << std::endl;
			}
		};

		void dump() const
		{
			std::cout << "-------- value --------" << std::endl;
			std::for_each(variables_.begin(), variables_.end(), DumpAction());
		}
#endif

	private:
		std::map <std::string, ValueTag> variables_;
		std::vector < int> vec_variableTypes;
		int		addr_;
		bool	global_;
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
		FunctionTag(const int type,const std::string &arg_name) 
			: valueType(type), flags_(0), index_(0), name(arg_name)
		{
		}
		void SetArg(const int type)
		{
			args_.push_back(type);
		}

		void SetArgs(const std::vector<ArgDefine>& args)
		{
			size_t size = args.size();
			for (size_t i = 0; i < size; i++) {
				args_.push_back(args[i].type());
			}
		}

		void SetArgs(const std::vector<int>& args)
		{
			size_t size = args.size();
			for (size_t i = 0; i < size; i++) {
				args_.push_back(args[i]);
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

					args_.push_back(arg_map_argmentChars.at(splited[i]));
				}

			}
			return true;
		}


		bool CheckArgList(const std::vector<ArgDefine>& args) const
		{
			// 引数が無い場合
			if (args.empty())
				return args_.empty();

			// 引数の個数が異なる
			if (args.size() != args_.size())
				return false;

			// 全引数の型をチェック

			//厳密チェック
			size_t size = args_.size();
			for (size_t i = 0; i < size; i++) {
				if (!TypeCheck(args[i].type(), (int)args_[i]))
					return false;
			}

			return true;
		}

		bool CheckArgList(const std::vector<int>& args) const
		{
			// 引数が無い場合
			if (args.empty())
				return args_.empty();

			// 引数の個数が異なる
			if (args.size() != args_.size())
				return false;

			// 全引数の型をチェック
			//厳密チェック
			size_t size = args_.size();
			for (size_t i = 0; i < size; i++) {
				if (!TypeCheck(args[i], (int)args_[i]))
					return false;
			}
			return true;
		}


		bool CheckArgList_strict(const std::vector<ArgDefine>& args) const
		{
			// 引数が無い場合
			if (args.empty())
				return args_.empty();

			// 引数の個数が異なる
			if (args.size() != args_.size())
				return false;

			// 全引数の型をチェック

			//厳密チェック
			size_t size = args_.size();
			for (size_t i = 0; i < size; i++) {
				if (!TypeCheck_strict(args[i].type(), (int)args_[i]))
					return false;
			}

			return true;
		}

		bool CheckArgList_strict(const std::vector<int>& args) const
		{
			// 引数が無い場合
			if (args.empty())
				return args_.empty();

			// 引数の個数が異なる
			if (args.size() != args_.size())
				return false;

			// 全引数の型をチェック
			//厳密チェック
			size_t size = args_.size();
			for (size_t i = 0; i < size; i++) {
				if (!TypeCheck_strict(args[i], (int)args_[i]))
					return false;
			}
			return true;
		}

		// 指定の引数の型を得る

		int GetArg(const int index) const
		{
			return args_[index];
		}

		int ArgSize() const { return (int)args_.size(); }

		void SetIndex(const int index) { index_ = index; }
		void SetDeclaration() { flags_ |= flag_declaration; }	// 宣言
		void SetDefinition() { flags_ |= flag_definition; }		// 定義
		void SetSystem() { flags_ |= flag_system; }

		int GetIndex() const { return index_; }
		bool IsDeclaration() const { return (flags_ & flag_declaration) != 0; }
		bool IsDefinition() const { return (flags_ & flag_definition) != 0; }
		bool IsSystem() const { return (flags_ & flag_system) != 0; }
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
			arg_fOut.write((char*)&flags_, sizeof(flags_));
			arg_fOut.write((char*)&index_, sizeof(index_));
			int argsSize = args_.size();
			arg_fOut.write((char*)&argsSize, sizeof(argsSize));
			for (int i = 0; i < argsSize; i++) {
				arg_fOut.write((char*)&args_[i], sizeof(args_[i]));
			}
			int size = name.size();
			arg_fOut.write((char*)&size, sizeof(size));
			arg_fOut.write(name.c_str(), (size));
		}
		void FileInput(std::ifstream& arg_fIn) {
			arg_fIn.read((char*)&valueType, sizeof(valueType));
			arg_fIn.read((char*)&flags_, sizeof(flags_));
			arg_fIn.read((char*)&index_, sizeof(index_));
			int argsSize = 0;
			arg_fIn.read((char*)&argsSize, sizeof(argsSize));
			for (int i = 0; i < argsSize; i++) {
				int arg;
				arg_fIn.read((char*)&arg, sizeof(arg));
				args_.push_back(arg);
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
		int		flags_ = 0;
		int		index_ = 0;
		std::vector<int>	args_;
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


		FunctionTag* Add(const std::string& name, const FunctionTag& tag)
		{
			auto key = name;
			auto result = map_functions.emplace(key, tag);

			return &result->second;
		}

		const FunctionTag* Find_strict(const std::string& name, const std::vector<int>& args) const
		{
			const_iter itr = map_functions.find(name);
			if (itr == map_functions.end()) {
				return nullptr;
			}
			auto end = map_functions.upper_bound(name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList_strict(args)) {

					return &itr->second;
				}
			}
			return nullptr;
		}

		FunctionTag* Find_strict(const std::string& name, const std::vector<ArgDefine>& vec_args)
		{
			iter itr = map_functions.find(name);

			if (itr == map_functions.end()) {
				return nullptr;
			}

			auto end = map_functions.upper_bound(name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList_strict(vec_args)) {

					return &itr->second;
				}
			}
			return nullptr;
		}

		const FunctionTag* Find(const std::string& name, const std::vector<int>& args) const
		{
			const_iter itr = map_functions.find(name);
			if (itr == map_functions.end()) {
				return nullptr;
			}
			auto end = map_functions.upper_bound(name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList(args)) {

					return &itr->second;
				}
			}
			return nullptr;
		}

		FunctionTag* Find(const std::string& name, const std::vector<ArgDefine>& vec_args)
		{
			iter itr = map_functions.find(name);

			if (itr == map_functions.end()) {
				return nullptr;
			}

			auto end = map_functions.upper_bound(name);
			for (; itr != end; itr++) {
				if (itr->second.CheckArgList(vec_args)) {

					return &itr->second;
				}
			}
			return nullptr;
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

		FunctionTag* operator[](const int index) {
			auto itr = map_functions.begin();
			int maxCount = min(map_functions.size(),index);
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
		int GetMemberTypeIndex(const int index)const {
			return vec_memberTypes[index];
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

	struct TypeTag {
		TypeTag() {}

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
		bool isSystem;
		bool isShared=false;

		ScriptClassInfo GetScriptTypeInfo()const {
			if (isSystem) {
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
		const TypeTag* GetType(const int index) const {
			if (vec_types.size() <= index) {
				return nullptr;
			}
			return vec_types[index];
		}
		TypeTag* GetType(const int index) {
			if (vec_types.size() <= index) {
				return nullptr;
			}
			return vec_types[index];
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
			for (auto typeItr = map_types.begin(); typeItr != map_types.end(); ) {
				if (!typeItr->second.isSystem) {
					typeItr = map_types.erase(typeItr);
					map_argmentChars.erase(typeItr->first);
					auto removeItr = vec_types.begin();
					for (int i = 0; i < typeItr->second.typeIndex; i++) {
						removeItr++;
					}
					vec_types.erase(removeItr);
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
				if (vec_types[i]->isSystem){ continue; }
					
				output.push_back(vec_types[i]->GetScriptTypeInfo());

			}

			return output;
		}
	private:

		std::vector<TypeTag* > vec_types;
		std::map<std::string, int> map_argmentChars;
		std::map<std::string, TypeTag> map_types;
		int systemTypeCount;
	};
	
}
inline std::string ButiScript::FunctionTag::GetNameWithArgment(const TypeTable& arg_typeTable) const
{
	std::string output = name;

	if (args_.size()) {
		output += ":";
	}

	for (int i = 0; i < args_.size(); i++) {
		output += arg_typeTable.GetType(args_[i] & ~TYPE_REF)->argName;
		if (i + 1 != args_.size()) {
			output += ",";
		}
	}
	return output;
}
#endif // !TAGS_H
