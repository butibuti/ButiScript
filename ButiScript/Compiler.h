#ifndef __COMPILER_H__
#define	__COMPILER_H__

#include "VirtualMachine.h"
#include "Node.h"

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

public:
	unsigned char size_;
	unsigned char op_;
	int constType;
	
	//
	int codeValue =0;
	
	//定数のポインタ
	void* p_constValue =nullptr;

};

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
	ValueTag() : addr_(-1), type_(TYPE_INTEGER), size_(1), global_(false)
	{
	}
	ValueTag(const int addr,const int type,const int size,const bool global)
		: addr_(addr), type_(type), size_(size), global_(global)
	{
	}

public:
	int		addr_;
	int		type_;
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

#ifdef	_DEBUG
	struct DumpAction {
		void operator()(const std::pair<std::string, ValueTag>& it)
		{
			std::cout << it.first << ", addr = " << it.second.addr_ << ", type = " << it.second.type_ << ", size = " << it.second.size_ << ", global = " << it.second.global_ << std::endl;
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
		: type_(type), flags_(0), index_(0)
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

	bool ChkArgList(const std::vector<ArgDefine>& args) const
	{
		// 引数が無い場合
		if (args.empty())
			return args_.empty();

		// 引数の個数が異なる
		if (args.size() != args_.size())
			return false;

		// 全引数の型をチェック
		size_t size = args_.size();
		for (size_t i = 0; i < size; i++) {
			if (args[i].type() != (int)args_[i])
				return false;
		}
		return true;
	}

	bool ChkArgList(const std::vector<int>& args) const
	{
		// 引数が無い場合
		if (args.empty())
			return args_.empty();

		// 引数の個数が異なる
		if (args.size() != args_.size())
			return false;

		// 全引数の型をチェック
		size_t size = args_.size();
		for (size_t i = 0; i < size; i++) {
			if (args[i] != (int)args_[i])
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
	int		type_;
	int		flags_;
	int		index_;
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
		std::pair<iter, bool> result = functions_.insert(make_pair(name, tag));
		if (result.second)
			return &result.first->second;
		return nullptr;
	}

	const FunctionTag* find(const std::string& name) const
	{
		const_iter it = functions_.find(name);
		if (it != functions_.end())
			return &it->second;
		return nullptr;
	}

	FunctionTag* find(const std::string& name)
	{
		iter it = functions_.find(name);
		if (it != functions_.end())
			return &it->second;
		return nullptr;
	}

	void Clear()
	{
		functions_.clear();
	}

private:
	std::map<std::string, FunctionTag> functions_;
};

// コンパイラ

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

	bool Compile(const std::string& file, ButiVM::Data& Data);

#ifdef	_DEBUG
	void debug_dump();
#endif

	bool DefineSystemFunction(const int index,const int type, const char* name, const char* args);

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
	const FunctionTag* GetFunctionTag(const std::string& name) const
	{
		return functions.find(name);
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
	bool CraeteData(ButiVM::Data& Data,const int code_size);

	//まだ定義されていない関数を呼び出している関数の登録
	void AddCallingNonDeclaredFunction(const int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block);

	//関数マッチングの二週目
	void ReRegistFunctions();

	// Error handling.
	void error(const std::string& m);

	void ClearStatement();
private:
	FunctionTable functions;
	std::vector<AddFunctionInfo> vec_callingNonDeclaredFunctions;
	std::vector<ValueTable> variables;
	std::vector<VMCode> statement;
	std::vector<Label> labels;
	std::vector<char> text_table;

	int break_index;
	int error_count;

	std::string current_function_name;
	int current_function_type;
};

#endif
