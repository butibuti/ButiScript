#ifndef __COMPILER_H__
#define	__COMPILER_H__

#include "VirtualMachine.h"
#include "Node.h"

namespace ButiScript {



// 仮想マシンコード生成

class VMCode {
public:
	VMCode(const unsigned char Op)
		: size_(1), op_(Op)
	{
	}
	VMCode(const unsigned char Op, const int arg)
		: size_(5), op_(Op)
	{
		constType = TYPE_INTEGER;
		p_constValue = new int(arg);
	}
	VMCode(const unsigned char Op, const float arg)
		: size_(5), op_(Op)
	{
		constType = TYPE_FLOAT;
		p_constValue = new float(arg);
	}

	~VMCode() {
	}

	unsigned char* Get(unsigned char* p) const
	{
		if (op_ != VM_MAXCOMMAND) {			// ラベルのダミーコマンド
			*p++ = op_;
			if (size_ > 1) {
				if (p_constValue) {
					switch (constType)
					{
					case TYPE_INTEGER:
						*(int*)p = *((int*)p_constValue);
						delete(p_constValue);
						p += 4;
						break;
					case TYPE_FLOAT:
						*(float*)p = *((float*)p_constValue);
						delete(p_constValue);
						p += 4;
						break;
					default:
						break;
					}

				}
				else {
				}
			}
		}
		return p;
	}
	template<typename T>
	T  GetConstValue()const {
		assert(p_constValue);
		return *((T*)p_constValue);
	}
	template<typename T>
	void  SetConstValue(const T arg_v){
		if (p_constValue) {
			delete p_constValue;
		}
		p_constValue = new T(arg_v);
	}

public:
	unsigned char size_;
	unsigned char op_;
	int constType;
	
	
	//定数のポインタ
	void* p_constValue =nullptr;

};



//名前空間の定義
class NameSpace:std::enable_shared_from_this<NameSpace> {
public:
	NameSpace(const std::string& arg_name) :name(arg_name) {	}

	const std::string& GetNameString()const;
	std::string GetGlobalNameString()const;
	void Regist(Compiler* arg_compiler);
	void SetParent(std::shared_ptr<NameSpace>arg_parent);
	std::shared_ptr<NameSpace> GetParent()const;
private:
	std::string name;
	std::shared_ptr<NameSpace> shp_parentNamespace;
};

using NameSpace_t = std::shared_ptr<NameSpace>;


// ラベル

class Label {
public:
	Label(const int index)
		: index_(index), pos_(0)
	{
	}
	~Label()
	{
	}

public:
	int index_;
	int pos_;
};

// 変数テーブル

class ValueTag {
public:
	ValueTag() : address(-1), valueType(TYPE_INTEGER), size_(1), global_(false)
	{
	}
	ValueTag(const int addr,const int type,const int size,const bool global)
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
	using iter= std::map<std::string, ValueTag>::iterator ;
	using const_iter= std::map<std::string, ValueTag>::const_iterator ;

public:
	ValueTable(const int start_addr = 0) : addr_(start_addr), global_(false)
	{
	}

	void set_global()
	{
		global_ = true;
	}

	bool Add( const int type, const std::string& name,  const int size = 1)
	{
		std::pair<iter, bool> result = variables_.insert(make_pair(name, ValueTag(addr_, type, size, global_)));
		if (result.second) {
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

	bool add_arg( const int type, const std::string& name,  const int addr)
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
	std::map<std::string, ValueTag> variables_;
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
private:
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

	bool SetArgs(const char* args)
	{
		if (args) {
			for (int i = 0; args[i] != 0; i++) {
				switch (args[i]) {
				case 'I': case 'i':
					args_.push_back(TYPE_INTEGER);
					break;

				case 'S': case 's':
					args_.push_back(TYPE_STRING);
					break;

				case 'F': case 'f':
					args_.push_back(TYPE_FLOAT);
					break;

				default:
					return false;
				}
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
	int		valueType=0;
	int		flags_=0;
	int		index_=0;
	std::vector<unsigned char>	args_;
};

class FunctionTable {
private:
	using iter= std::map<std::string, FunctionTag>::iterator ;
	using const_iter= std::map<std::string, FunctionTag>::const_iterator ;

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

private:
	std::multimap<std::string, FunctionTag> map_functions;
};

// コンパイラ

class VirtualCPU;
using SysFunction = void (VirtualCPU::*)();
class Compiler {
public:

	class AddFunctionInfo {
	public:
		int type; 
		std::string name; 
		std::vector<ArgDefine> args; 
		Block_t block;
	};

	Compiler();
	virtual ~Compiler();

	bool Compile(const std::string& file, ButiScript::Data& Data);

#ifdef	_DEBUG
	void debug_dump();
#endif

	bool DefineSystemFunction(SysFunction arg_op,const int type, const char* name, const char* args);

	void ValueDefine(const int type, const std::vector<Node_t>& node);
	void FunctionDefine(const int type, const std::string& name, const std::vector<int>& args);
	void AddFunction(const int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block, const bool isReRegist = false);
	void RegistFunction(const int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block,const bool isReRegist=false);

	// 変数の検索、内側のブロックから検索する。
	const ValueTag* GetValueTag(const std::string& name) const
	{
		int size = (int)variables.size();
		for (int i = size - 1; i >= 0; i--) {
			const ValueTag* tag = variables[i].find(name);
			if (tag)
				return tag;
		}
		return nullptr;
	}

	// 関数の検索
	const FunctionTag* GetFunctionTag(const std::string& name,const std::vector<int>& args,const bool isStrict) const
	{
		if(isStrict)
		return functions.Find_strict(name,args);
		else {
			return functions.Find(name, args);
		}
	}
	NameSpace_t GetCurrentNameSpace()const {
		return currentNameSpace;
	}
	// for code generator.
#define	VM_CREATE
#include "VM_create.h"
#undef	VM_CREATE

	void BlockIn();
	void BlockOut();
	void AllocStack();
	int LabelSetting();

	int SetBreakLabel(const int label)
	{
		int old_index = break_index;
		break_index = label;
		return old_index;
	}
	bool JmpBreakLabel();

	int MakeLabel();

	void AddValue(const int type, const std::string& name, Node_t node);

	void SetLabel(const int label);

	void PushString(const std::string& name);
	int GetFunctionType() const { return current_function_type; }
	bool CraeteData(ButiScript::Data& Data,const int code_size);

	void PushNameSpace(NameSpace_t arg_namespace);
	void PopNameSpace();
	// Error handling.
	void error(const std::string& m);

	void ClearStatement();
	std::string GetTypeName(const int type) const;


private:
	FunctionTable functions;
	std::vector<ValueTable> variables;
	std::vector<VMCode> statement;
	std::vector<Label> labels;
	std::vector<char> text_table;
	std::vector<SysFunction> vec_sysCalls;

	NameSpace_t currentNameSpace = nullptr;
	int break_index;
	int error_count;

	std::string current_function_name;
	int current_function_type;

};
}
#endif
