#ifndef __NODE_H__
#define	__NODE_H__

#include <iostream>
#include <string>
#include <vector>
#include"ButiMemorySystem/ButiMemorySystem/ButiPtr.h"
#include"ButiMemorySystem/ButiMemorySystem/ButiList.h"
#include"Tags.h"
namespace ButiScript {
class Compiler;
#include"value_type.h"

// ノードの命令
enum OPCODE {
	OP_NEG,
	OP_NOT,
	OP_INCREMENT,
	OP_DECREMENT,
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
	OP_NULL,
	OP_INT,
	OP_FLOAT,
	OP_IDENTIFIER,
	OP_MEMBER,
	OP_METHOD,
	OP_STRING,
	OP_FUNCTION,
	OP_ARRAY,
	OP_REFFERENCE,
};

// ノード
class ValueTag;
class Node;
class NodeList;
class Function;
using Function_t = ButiEngine::Value_ptr<Function>;
using Node_t = ButiEngine::Value_ptr<Node>;
using NodeList_t = ButiEngine::Value_ptr<NodeList>;

class Node :public ButiEngine::enable_value_from_this<Node> {
public:
	Node(const std::int32_t arg_op, const Node_t& arg_left, const Node_t& arg_right):op(arg_op), leftNode(arg_left), rightNode(arg_right), num_int(0), num_float(0)	{}

	Node(const std::int32_t arg_op, const Node_t& arg_left): op(arg_op), leftNode(arg_left), num_int(0), num_float(0){}

	Node(const std::int32_t arg_op, const std::int32_t arg_number): op(arg_op), num_int(arg_number){}

	Node(const std::int32_t arg_op, float arg_number):op(arg_op), num_float(arg_number){}

	Node(const std::int32_t arg_op, const std::string& arg_str): op(arg_op), num_int(0), strData(arg_str){}

	Node(const std::int32_t arg_op, const char* arg_buffer, const char* arg_size):op(arg_op), strData(arg_buffer, arg_size){}

	virtual ~Node(){}

	std::int32_t Op() const { return op; }
	std::int32_t GetNumber() const { return num_int; }
	const std::string& GetString() const { return strData; }
	Node_t GetLeft() const { return leftNode; }
	Node_t GetRight() const { return rightNode; }

	virtual std::int32_t Push(Compiler* arg_compiler) const;
	virtual std::int32_t PushClone(Compiler* arg_compiler) const {
		return Push(arg_compiler);
	}
	virtual std::int32_t Pop(Compiler* arg_compiler) const;

	virtual std::int32_t GetType(Compiler* arg_compiler)const;

	virtual const ValueTag* GetValueTag(Compiler* arg_compiler)const;
	virtual const ValueTag* GetValueTag(const std::string& arg_name, Compiler* arg_compiler)const;
	
	std::int32_t Assign(Compiler* arg_compiler) const;
	virtual void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const;
	static Node_t make_node(const std::int32_t arg_op, const float arg_number, const Compiler* arg_compiler)
	{
		if (arg_op == OP_FLOAT)
			return ButiEngine::make_value<Node>(arg_op, arg_number);

		return ButiEngine::make_value<Node>(arg_op, static_cast<std::int32_t>(arg_number));
	}

	static Node_t make_node(const std::int32_t arg_op, const std::string& arg_str, const Compiler* arg_compiler);
	static Node_t make_node(const std::int32_t arg_op, Node_t arg_left, const Compiler* arg_compiler);
	static Node_t make_node(const std::int32_t arg_op, Node_t arg_left, Node_t arg_right);
	static Node_t make_node(const std::int32_t arg_op, Node_t arg_left, const std::string& arg_str);
	static Node_t make_node(const std::int32_t arg_op, Node_t arg_left, NodeList_t arg_list);
	static Node_t make_node(const std::int32_t arg_op, Node_t arg_left, const ButiEngine::List<std::string>& arg_templateTypes);
	std::string ToNameSpaceString()const { return m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData+"::" : strData+"::"; }
	virtual bool IsNameSpaceNode(Compiler* arg_compiler)const { return false; }
	void SetLeftNode(Node_t arg_node) { leftNode = arg_node; }
	void SetRightNode(Node_t arg_node) { rightNode = arg_node; }
	void SetRefferenceNode(Node_t arg_node) { m_vlp_refference = arg_node; }
	virtual Node_t ToFunctionCall()const {assert(0 && "不明なノードを関数呼び出しに変更しようとしています");return nullptr; }
	virtual void SetArgmentList(NodeList_t arg_List) {}
	void SetTemplateList(const ButiEngine::List<std::string>& arg_list_templateTypeNames){m_list_templateTypeNames = arg_list_templateTypeNames;}
protected:
	std::int32_t op;
	std::int32_t num_int;
	float num_float;
	std::string strData;
	Node_t leftNode;
	Node_t rightNode;
	Node_t m_vlp_refference;
	ButiEngine::List<std::string> m_list_templateTypeNames;
};

//Nullのノード
class Node_Null :public Node {
public:
	Node_Null() :Node(OP_NULL, "null") {}
	std::int32_t Push(Compiler* arg_compiler)const override;
};

// 値ノード
class Node_value : public Node {
public:
	Node_value(const std::string& arg_name) :Node(OP_IDENTIFIER, arg_name) {};
	Node_value(const std::string& arg_name, Node_t arg_refference) : Node(OP_REFFERENCE, arg_name)
	{
		m_vlp_refference = arg_refference;
	};
	const ValueTag* GetValueTag(const std::string& arg_searchStr,Compiler* arg_compiler)const override;
	std::int32_t Push(Compiler* arg_compiler) const override;
	std::int32_t PushClone(Compiler* arg_compiler) const override;
	std::int32_t Pop(Compiler* arg_compiler) const override;
	std::int32_t GetType(Compiler* arg_compiler)const override;
	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const override;
	Node_t ToFunctionCall()const override;
	bool IsNameSpaceNode(Compiler* arg_compiler)const override;
};

// 関数呼び出しノード
class Node_functionCall : public Node {
public:
	Node_functionCall(const std::string& arg_name, Node_t arg_refference,const ButiEngine::List<std::string>& arg_templateList) : Node(OP_FUNCTION, arg_name)
	{
		m_vlp_refference = arg_refference;
		m_list_templateTypeNames = arg_templateList;
	};
	std::int32_t Push(Compiler* arg_compiler) const override;
	std::int32_t PushClone(Compiler* arg_compiler) const override;
	std::int32_t Pop(Compiler* arg_compiler) const override;
	std::int32_t GetType(Compiler* arg_compiler)const override;
	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const override;

	void SetArgmentList(NodeList_t arg_List)override;
private:
	NodeList_t m_nodeList;
};

// ノードリスト

class NodeList {
public:
	NodeList(Node_t arg_node)
	{
		if (arg_node) {
			list_args.Add(arg_node);
		}
	}

	NodeList* Add(Node_t arg_add)
	{
		if (arg_add) {
			list_args.Add(arg_add);
		}
		return this;
	}

	std::uint64_t size() const { return list_args.GetSize(); }
	Node_t get(std::uint64_t arg_index) const { return list_args[arg_index]; }

	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const;
	void Reverse() {
		std::reverse(list_args.begin(), list_args.end());
	}
public:
	ButiEngine::List<Node_t> list_args;
};

//関数オブジェクトのノード
class Node_FunctionObject :public Node {
public:
	Node_FunctionObject(const std::string& arg_identiferName) : Node(OP_INT, arg_identiferName) {
		strData = arg_identiferName;
	}
	virtual std::int32_t Push(Compiler* arg_compiler) const;
	virtual std::int32_t Pop(Compiler* arg_compiler) const;
	std::int32_t GetType(Compiler* arg_compiler)const override;
};

// ステートメント

enum STATE_TYPE {
	NOP_STATE,
	ASSIGN_STATE,
	UNARY_STATE,
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
using Block_t = ButiEngine::Value_ptr <Block>;

class Statement;
using Statement_t = ButiEngine::Value_ptr<Statement>;

// 文
class Statement {
public:
	virtual void Add(Statement_t arg_statement)
	{
		std::cerr << "内部エラー：Add(statement)が呼ばれました" << std::endl;
	}

	virtual void Add(Node_t arg_node)
	{
		std::cerr << "内部エラー：Add(node)が呼ばれました" << std::endl;
	}

	virtual void Add(const ButiEngine::List<Node_t >& arg_list_node)
	{
		std::cerr << "内部エラー：Add(ButiEngine::List< node>)が呼ばれました" << std::endl;
	}

	virtual void Add(const std::int32_t arg_index, Statement_t arg_statement)
	{
		std::cerr << "内部エラー：Add(index, statement)が呼ばれました" << std::endl;
	}

	virtual void Add(const std::int32_t arg_index, Node_t arg_node)
	{
		std::cerr << "内部エラー：Add(index, node)が呼ばれました" << std::endl;
	}

	virtual std::int32_t Analyze(Compiler* arg_compiler) = 0;
	virtual std::int32_t Regist(Compiler* arg_compiler) { return 0; };
	virtual void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) {}
	virtual std::int32_t Case_Analyze(Compiler* arg_compiler, std::int32_t* arg_default_label)
	{
		return 0;
	}

	static Statement_t make_statement(const std::int32_t arg_list_state);
	static Statement_t make_statement(const std::int32_t arg_list_state, const std::int32_t);
	static Statement_t make_statement(const std::int32_t arg_list_state, Node_t arg_node);
	static Statement_t make_statement(const std::int32_t arg_list_state, Block_t arg_block);

};

// nop
class Statement_nop : public Statement {
public:
	std::int32_t Analyze(Compiler* arg_compiler);
};

class Statement_unary :public Statement {
public:
	Statement_unary(Node_t arg_node)
		: node(arg_node)
	{
	}
	std::int32_t Analyze(Compiler* arg_compiler)override;
private:
	Node_t node;
};

// 代入
class Statement_assign : public Statement {
public:
	Statement_assign(Node_t arg_node)
		: node(arg_node)
	{
	}

	std::int32_t Analyze(Compiler* arg_compiler);

	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) override;
private:
	Node_t node;
};

// 関数呼び出し
class ccall_statement : public Statement, public ButiEngine::enable_value_from_this<ccall_statement> {
public:
	ccall_statement(Node_t arg_node)
		: node(arg_node)
	{
	}

	std::int32_t Analyze(Compiler* arg_compiler);

	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) override;
private:
	Node_t node;
};

// case
class Statement_case : public Statement {
public:
	Statement_case(Node_t arg_node)
		: node(arg_node)
	{
	}

	std::int32_t Analyze(Compiler* arg_compiler);
	std::int32_t case_Analyze(Compiler* arg_compiler, std::int32_t* arg_default_label);
	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) override;

private:
	Node_t node;
	std::int32_t label_;
};

// default
class Statement_default : public Statement {
public:
	std::int32_t Analyze(Compiler* arg_compiler);
	std::int32_t case_Analyze(Compiler* arg_compiler, std::int32_t* arg_default_label);

private:
	std::int32_t label_;
};

// break
class Statement_break : public Statement {
public:
	std::int32_t Analyze(Compiler* arg_compiler);
};

// return
class Statement_return : public Statement {
public:
	void Add(Node_t arg_node)
	{
		node = arg_node;
	}

	std::int32_t Analyze(Compiler* arg_compiler);
	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) override;

private:
	Node_t node;
};

// if
class Statement_if : public Statement {
public:
	Statement_if()
	{
	}

	void Add(Node_t arg_node)
	{
		node = arg_node;
	}

	void Add(const std::int32_t arg_index, Statement_t arg_statement)
	{
		list_statement[arg_index] = arg_statement;
	}

	std::int32_t Analyze(Compiler* arg_compiler);
	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) override;

private:
	Node_t node;
	Statement_t list_statement[2];
};

// for
class Statement_for : public Statement {
public:
	void Add(Statement_t arg_statement)
	{
		list_statement = arg_statement;
	}

	void Add(const std::int32_t arg_index, Node_t arg_node)
	{
		node[arg_index] = arg_node;
	}

	std::int32_t Analyze(Compiler* arg_compiler);
	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) override;

private:
	Statement_t list_statement;
	Node_t node[3];
};

// while
class Statement_while : public Statement {
public:
	void Add(Statement_t arg_statement)
	{
		list_statement = arg_statement;
	}

	void Add(Node_t arg_node)
	{
		node = arg_node;
	}

	std::int32_t Analyze(Compiler* arg_compiler);

	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) override;
private:
	Node_t node;
	Statement_t list_statement;
};

// switch
class Statement_switch : public Statement {
public:
	Statement_switch(Node_t arg_node)
		: node(arg_node)
	{
	}

	void Add(Statement_t arg_statement)
	{
		list_statement.Add(arg_statement);
	}

	std::int32_t Analyze(Compiler* arg_compiler);

	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) override;
private:
	Node_t node;
	ButiEngine::List<Statement_t> list_statement;
};

// block
class Statement_block : public Statement {
public:
	Statement_block(Block_t arg_block)
		: block_(arg_block)
	{
	}

	std::int32_t Analyze(Compiler* arg_compiler);
	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) override;

private:
	Block_t block_;
};

// 宣言
class Declaration : public Statement {
public:
	Declaration(const std::string& arg_typeName):valueTypeName(arg_typeName), isFunction(false){}
	Declaration(const std::string& arg_typeName, const AccessModifier arg_access)
		:valueTypeName(arg_typeName), isFunction(false)
	{
		if (arg_access == AccessModifier::Public || arg_access == AccessModifier::Private || arg_access == AccessModifier::Protected) {
			accessType = arg_access;
		}
	}

	Declaration(const std::string& arg_typeName, const std::string& arg_name):valueTypeName(arg_typeName), isFunction(true)
	{
		list_names.Add(arg_name);
	}

	void Add(const ButiEngine::List<std::string >& arg_list_str)
	{
		list_names = (arg_list_str);
	}

	void Add(const std::string arg_typeName)
	{
		list_argTypeNames.Add(arg_typeName);
	}

	std::int32_t PushCompiler(Compiler* arg_compiler);
	std::int32_t Analyze(Compiler* arg_compiler);

	void Define(Compiler* arg_compiler);
	bool IsFunction()const {
		return isFunction;
	}
private:
	std::string valueTypeName;
	bool isFunction;
	ButiEngine::List<std::string> list_names,list_argTypeNames;
	ButiEngine::List<std::int32_t> list_argType;
	AccessModifier accessType = AccessModifier::Public;
};

using Declaration_t = ButiEngine::Value_ptr<Declaration>;

// ブロック

class Block {
public:
	void Add(const Declaration_t& arg_decl)
	{
		list_decl.Add(arg_decl);
	}
	void Add(const Statement_t& arg_state)
	{
		list_state.Add(arg_state);
	}

	std::int32_t Analyze(Compiler* arg_compiler, ButiEngine::List<Function_t>& arg_captureCheck);
	inline std::int32_t Analyze(Compiler* arg_compiler) {
		static ButiEngine::List<Function_t> captureCheckDummy;
		return Analyze(arg_compiler, captureCheckDummy);
	}
	void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler);
private:
	ButiEngine::List<Declaration_t> list_decl;
	ButiEngine::List<Statement_t> list_state;
};


}
#endif
