#pragma once
#ifndef TAGS_H
#define TAGS_H
#include"value_type.h"
namespace ButiScript {

	class Compiler;
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

	class ValueTag {
	public:
		ValueTag() : address(-1), valueType(TYPE_INTEGER), size_(1), global_(false)
		{
		}
		ValueTag(const int addr, const int type, const int size, const bool global)
			: address(addr), valueType(type), size_(size), global_(global)
		{
		}

	public:
		int		address;
		int		valueType;
		int		size_;
		bool	global_;
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

		bool Add(const int type, const std::string& name, const int size = 1)
		{
			if (!variables_.count(name)) {
				variables_.emplace(name, ValueTag(addr_, type, size, global_));
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
			std::pair<iter, bool> result = variables_.insert(make_pair(name, ValueTag(addr, type, 1, false)));
			return result.second;
		}

		int size() const { return addr_; }
		void Clear()
		{
			variables_.clear();
			addr_ = 0;
		}

		void Alloc(Compiler* arg_comp)const;

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
		FunctionTag()
		{
		}
		FunctionTag(const int type)
			: valueType(type), flags_(0), index_(0)
		{
		}

		void SetArg(const int type)
		{
			args_.push_back((unsigned char)type);
		}

		void SetArgs(const std::vector<ArgDefine>& args)
		{
			size_t size = args.size();
			for (size_t i = 0; i < size; i++) {
				args_.push_back((unsigned char)args[i].type());
			}
		}

		void SetArgs(const std::vector<int>& args)
		{
			size_t size = args.size();
			for (size_t i = 0; i < size; i++) {
				args_.push_back((unsigned char)args[i]);
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
				if (!arg_map_argmentChars.count(args)) {
					return false;
				}
				else {

					args_.push_back(arg_map_argmentChars.at(args));
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

	public:
		int		valueType = 0;
		int		flags_ = 0;
		int		index_ = 0;
		std::vector<unsigned char>	args_;

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

	protected:
		std::multimap<std::string, FunctionTag> map_functions;
	};

	class VirtualCPU;
	using OperationFunction = void (VirtualCPU::*)();

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

		//メンバ変数へのアクセス名とインデックス
		std::map<std::string, int> map_memberIndex;
		//メンバ変数へのアクセス名と型
		std::map<std::string, int> map_memberType;
		//メソッド
		FunctionTable methods;

		FunctionTag* AddMethod(const std::string& arg_methodName, const FunctionTag& arg_method) {
			return methods.Add(arg_methodName, arg_method);
		}
	};

	//型定義用

	class TypeTable {
	public:
		const TypeTag* GetType(const int index) const {
			if (vec_systemTypes.size() <= index) {
				return nullptr;
			}
			return vec_systemTypes[index];
		}
		TypeTag* GetType(const int index) {
			if (vec_systemTypes.size() <= index) {
				return nullptr;
			}
			return vec_systemTypes[index];
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

		void RegistType(const TypeTag& arg_type) {


			if (vec_systemTypes.size() <= arg_type.typeIndex) {
				vec_systemTypes.resize(arg_type.typeIndex + 1);
			}
			map_argmentChars.emplace(arg_type.argName, arg_type.typeIndex);
			map_types.emplace(arg_type.typeName, arg_type);
			vec_systemTypes[arg_type.typeIndex] = &map_types.at(arg_type.typeName);
		}

		const std::vector<TypeTag* >& GetSystemType()const {
			return vec_systemTypes;
		}

		void CreateTypeVec(std::vector<TypeTag>& arg_ref_types) const {
			arg_ref_types.reserve(vec_systemTypes.size());
			auto end = vec_systemTypes.end();
			for (auto itr = vec_systemTypes.begin(); itr != end; itr++) {
				arg_ref_types.push_back(*(*itr));
			}

		}


	private:

		std::vector<TypeTag* > vec_systemTypes;
		std::map<std::string, int> map_argmentChars;
		std::map<std::string, TypeTag> map_types;

	};
}
#endif // !TAGS_H
