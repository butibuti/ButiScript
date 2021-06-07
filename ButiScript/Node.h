#ifndef __NODE_H__
#define	__NODE_H__

#include <iostream>
#include <string>
#include <vector>
#include <memory>

namespace ButiScript {



	class Compiler;

	// ノード

	// 変数、関数の型

#include"value_type.h"

	// ノードの命令
	enum OPCODE {
		OP_NEG,
		OP_ADD,
		OP_SUB,
		OP_MUL,
		OP_DIV,
		OP_MOD,
		OP_AND,
		OP_OR,
		OP_LSHIFT,
		OP_RSHIFT,
		OP_LOGAND,
		OP_LOGOR,
		OP_EQ,
		OP_NE,
		OP_GT,
		OP_GE,
		OP_LT,
		OP_LE,
		OP_ASSIGN,
		OP_ADD_ASSIGN,
		OP_SUB_ASSIGN,
		OP_MUL_ASSIGN,
		OP_DIV_ASSIGN,
		OP_MOD_ASSIGN,
		OP_AND_ASSIGN,
		OP_OR_ASSIGN,
		OP_LSHIFT_ASSIGN,
		OP_RSHIFT_ASSIGN,
		OP_INT,
		OP_FLOAT,
		OP_IDENTIFIER,
		OP_STRING,
		OP_FUNCTION,
		OP_ARRAY,
	};

	// ノード

	class Node;
	class NodeList;
	using Node_t = std::shared_ptr<Node>;
	using NodeList_t = std::shared_ptr<NodeList>;

	class Node {
	public:
		Node(const int Op, const Node_t& left, const Node_t& right)
			: op_(Op), left_(left), right_(right), number_(0), num_float(0)
		{
		}
		Node(const int Op, const Node_t& left)
			: op_(Op), left_(left), number_(0), num_float(0)
		{
		}
		Node(const int Op, const int arg_number)
			: op_(Op), number_(arg_number)
		{
		}
		Node(const int Op, float arg_number)
			: op_(Op), num_float(arg_number)
		{
		}
		Node(const int Op, const std::string& str)
			: op_(Op), number_(0), string_(str)
		{
		}
		Node(const int Op, const char* b, const char* e)
			: op_(Op), string_(b, e)
		{
		}
		virtual ~Node()
		{
		}

		int Op() const { return op_; }
		int GetNumber() const { return number_; }
		const std::string& GetString() const { return string_; }
		Node_t GetLeft() const { return left_; }
		Node_t GetRight() const { return right_; }

		virtual int Push(Compiler* c) const;
		virtual int Pop(Compiler* c) const;

		virtual int GetType(Compiler* c)const;

		int GetCallType(Compiler* c, const std::string& name, const std::vector<Node_t>* args)const;

		int Assign(Compiler* c) const;
		int Call(Compiler* c, const std::string& name, const std::vector<Node_t>* args) const;

		static Node_t make_node(const int Op, const float arg_number)
		{
			if (Op == OP_FLOAT)
				return Node_t(new Node(Op, arg_number));

			return Node_t(new Node(Op, (int)arg_number));
		}

		static Node_t make_node(const int Op, const std::string& str);
		static Node_t make_node(const int Op, Node_t left);
		static Node_t make_node(const int Op, Node_t left, Node_t right);
		static Node_t make_node(const int Op, Node_t left, NodeList_t right);

	protected:
		int op_;
		int number_;
		float num_float;
		std::string string_;
		Node_t left_;
		Node_t right_;
	};

	// 変数ノード

	class Node_value : public Node {
	public:
		Node_value(const std::string& name)
			: Node(OP_IDENTIFIER, name)
		{
		}

		int Push(Compiler* c) const;
		int Pop(Compiler* c) const;
	};

	// ノードリスト

	class NodeList {
	public:
		NodeList(Node_t node)
		{
			args_.push_back(node);
		}

		NodeList* Add(Node_t add)
		{
			args_.push_back(add);
			return this;
		}

		size_t size() const { return args_.size(); }
		Node_t get(size_t index) const { return args_[index]; }

	public:
		std::vector<Node_t> args_;
	};

	// 関数のノード

	class Node_function : public Node {
	public:
		Node_function(int Op, const Node_t& node, const NodeList_t& list)
			: Node(Op, node), node_list_(list)
		{
		}

		virtual int Push(Compiler* c) const;
		virtual int Pop(Compiler* c) const;
		int GetType(Compiler* c)const override;

	private:
		NodeList_t node_list_;
	};

	// ステートメント

	enum STATE_TYPE {
		NOP_STATE,
		ASSIGN_STATE,
		CALL_STATE,
		CASE_STATE,
		DEFAULT_STATE,
		BREAK_STATE,
		RETURN_STATE,
		IF_STATE,
		FOR_STATE,
		WHILE_STATE,
		SWITCH_STATE,
		BLOCK_STATE,
	};

	class Block;
	using Block_t = std::shared_ptr<Block>;

	class Statement;
	using Statement_t = std::shared_ptr<Statement>;

	// 文
	class Statement {
	public:
		virtual void Add(Statement_t statement)
		{
			std::cerr << "内部エラー：Add(statement)が呼ばれました" << std::endl;
		}

		virtual void Add(Node_t node)
		{
			std::cerr << "内部エラー：Add(node)が呼ばれました" << std::endl;
		}

		virtual void Add(const int index, Statement_t statement)
		{
			std::cerr << "内部エラー：Add(index, statement)が呼ばれました" << std::endl;
		}

		virtual void Add(const int index, Node_t node)
		{
			std::cerr << "内部エラー：Add(index, node)が呼ばれました" << std::endl;
		}

		virtual int Analyze(Compiler* c) = 0;
		virtual int Regist(Compiler* c) { return 0; };

		virtual int Case_Analyze(Compiler* c, int* default_label)
		{
			return 0;
		}

		static Statement_t make_statement(const int state);
		static Statement_t make_statement(const int state, Node_t node);
		static Statement_t make_statement(const int state, Block_t block);
	};

	// nop
	class Statement_nop : public Statement {
	public:
		int Analyze(Compiler* c);
	};

	// 代入
	class Statement_assign : public Statement {
	public:
		Statement_assign(Node_t node)
			: vec_node(node)
		{
		}

		int Analyze(Compiler* c);

		int ReAnalyze(Compiler* c);

	private:
		Node_t vec_node;
	};

	// 関数呼び出し
	class ccall_statement : public Statement, public  std::enable_shared_from_this<ccall_statement> {
	public:
		ccall_statement(Node_t node)
			: vec_node(node)
		{
		}

		int Analyze(Compiler* c);
		void ReAnalyze(Compiler* c);
	private:
		Node_t vec_node;
	};

	// case
	class Statement_case : public Statement {
	public:
		Statement_case(Node_t node)
			: vec_node(node)
		{
		}

		int Analyze(Compiler* c);
		int case_Analyze(Compiler* c, int* default_label);

	private:
		Node_t vec_node;
		int label_;
	};

	// default
	class Statement_default : public Statement {
	public:
		int Analyze(Compiler* c);
		int case_Analyze(Compiler* c, int* default_label);

	private:
		int label_;
	};

	// break
	class Statement_break : public Statement {
	public:
		int Analyze(Compiler* c);
	};

	// return
	class Statement_return : public Statement {
	public:
		void Add(Node_t node)
		{
			vec_node = node;
		}

		int Analyze(Compiler* c);

	private:
		Node_t vec_node;
	};

	// if
	class Statement_if : public Statement {
	public:
		Statement_if()
		{
		}

		void Add(Node_t node)
		{
			vec_node = node;
		}

		void Add(const int index, Statement_t statement)
		{
			statement_[index] = statement;
		}

		int Analyze(Compiler* c);

	private:
		Node_t vec_node;
		Statement_t statement_[2];
	};

	// for
	class Statement_for : public Statement {
	public:
		void Add(Statement_t statement)
		{
			statement_ = statement;
		}

		void Add(const int index, Node_t node)
		{
			vec_node[index] = node;
		}

		int Analyze(Compiler* c);

	private:
		Statement_t statement_;
		Node_t vec_node[3];
	};

	// while
	class Statement_while : public Statement {
	public:
		void Add(Statement_t statement)
		{
			statement_ = statement;
		}

		void Add(Node_t node)
		{
			vec_node = node;
		}

		int Analyze(Compiler* c);

	private:
		Node_t vec_node;
		Statement_t statement_;
	};

	// switch
	class Statement_switch : public Statement {
	public:
		Statement_switch(Node_t node)
			: vec_node(node)
		{
		}

		void Add(Statement_t statement)
		{
			statement_.push_back(statement);
		}

		int Analyze(Compiler* c);

	private:
		Node_t vec_node;
		std::vector<Statement_t> statement_;
	};

	// block
	class Statement_block : public Statement {
	public:
		Statement_block(Block_t block)
			: block_(block)
		{
		}

		int Analyze(Compiler* c);

	private:
		Block_t block_;
	};

	// 宣言

	class Declaration : public Statement {
	public:
		Declaration(const int arg_type)
			:valueType(arg_type), isFunction(false)
		{
		}

		Declaration(const int arg_type, const std::string& arg_name)
			:valueType(arg_type), name_(arg_name), isFunction(true)
		{
		}

		void Add(Node_t node)
		{
			vec_node.push_back(node);
		}

		void Add(const int arg_type)
		{
			args.push_back(arg_type);
		}

		int Analyze(Compiler* c);

		void Define(Compiler* c);
		bool IsFunction()const {
			return isFunction;
		}
	private:
		int valueType;					// 型
		bool isFunction;				// 関数か変数か
		std::vector<Node_t> vec_node;	// 変数
		std::string name_;			// 関数名
		std::vector<int> args;		// 関数の引数
	};

	using Declaration_t = std::shared_ptr<Declaration>;

	// ブロック

	class Block {
	public:
		void Add(const Declaration_t& decl)
		{
			decl_.push_back(decl);
		}
		void Add(const Statement_t& state)
		{
			state_.push_back(state);
		}

		int Analyze(Compiler* c);

	private:
		std::vector<Declaration_t> decl_;
		std::vector<Statement_t> state_;
	};

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

	// 関数

	class Function {
	public:
		Function(const std::string& arg_name)
			: name_(arg_name)
		{
		}

		void Add(ArgDefine arg)
		{
			args_.push_back(arg);
		}

		void Add(Block_t block)
		{
			block_ = block;
		}

		int Analyze(Compiler* c);
		int Regist(Compiler* c);
		void set_type(const int arg_type) {
			valueType = arg_type;
		}

	private:
		int valueType;
		std::string name_;
		std::vector<ArgDefine> args_;
		Block_t block_;
	};
	using Function_t = std::shared_ptr<Function>;

}
#endif
