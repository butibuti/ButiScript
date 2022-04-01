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

	// ノード

	// 変数、関数の型

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
	};

	// ノード
	class ValueTag;
	class Node;
	class NodeList;
	class Function;
	using Function_t = ButiEngine::Value_ptr<Function>;
	using Node_t = ButiEngine::Value_ptr<Node>;
	using NodeList_t = ButiEngine::Value_ptr<NodeList>;

	class Node :public ButiEngine::enable_value_from_this<Node>{
	public:
		Node(const std::int32_t arg_op, const Node_t& arg_left, const Node_t& arg_right)
			: op(arg_op), leftNode(arg_left), rightNode(arg_right), num_int(0), num_float(0)
		{
		}
		Node(const std::int32_t arg_op, const Node_t& arg_left)
			: op(arg_op), leftNode(arg_left), num_int(0), num_float(0)
		{
		}
		Node(const std::int32_t arg_op, const std::int32_t arg_number)
			: op(arg_op), num_int(arg_number)
		{
		}
		Node(const std::int32_t arg_op, float arg_number)
			: op(arg_op), num_float(arg_number)
		{
		}
		Node(const std::int32_t arg_op, const std::string& arg_str)
			: op(arg_op), num_int(0), strData(arg_str)
		{
		}
		Node(const std::int32_t arg_op, const char* arg_buffer, const char* arg_size)
			: op(arg_op), strData(arg_buffer, arg_size)
		{
		}
		virtual ~Node()
		{
		}

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
		virtual std::int32_t EnumType(Compiler* arg_compiler)const {return TYPE_INTEGER;}
		std::int32_t GetCallType(Compiler* arg_compiler, const std::string& arg_name, const std::vector<Node_t>* arg_vec_arg,const std::vector<std::int32_t>& arg_vec_temps)const;

		std::int32_t Assign(Compiler* arg_compiler) const;
		std::int32_t Call(Compiler* arg_compiler, const std::string& arg_name, const std::vector<Node_t>* arg_vec_arg, const std::vector<std::int32_t>& arg_vec_temps) const;

		virtual Node_t CreateMethod(Node_t arg_node);

		virtual void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) const;
		static Node_t make_node(const std::int32_t arg_op, const float arg_number, const Compiler* arg_compiler)
		{
			if (arg_op == OP_FLOAT)
				return ButiEngine::make_value<Node>(arg_op, arg_number);

			return ButiEngine::make_value<Node>(arg_op, static_cast<std::int32_t>(arg_number));
		}

		static Node_t make_node(const std::int32_t arg_op, const std::string& arg_str, const Compiler* arg_compiler);
		static Node_t make_node(const std::int32_t arg_op, Node_t arg_left, const Compiler* arg_compiler);
		static Node_t make_node(const std::int32_t arg_op, Node_t arg_left,const std::string arg_memberName,const Compiler* arg_compiler);
		static Node_t make_node(const std::int32_t arg_op, Node_t arg_left, Node_t arg_right);
		static Node_t make_node(const std::int32_t arg_op, Node_t arg_left, Node_t arg_right,const Compiler* arg_compiler);
		static Node_t make_node(const std::int32_t arg_op, Node_t arg_left, NodeList_t arg_list);
		static Node_t make_node(const std::int32_t arg_op, Node_t arg_left, const std::vector<std::int32_t>& arg_templateTypes);

		void SetLeftNode(Node_t arg_node) { leftNode = arg_node; }
		void SetRightNode(Node_t arg_node) { rightNode = arg_node; }

	protected:
		std::int32_t op;
		std::int32_t num_int;
		float num_float;
		std::string strData;
		Node_t leftNode;
		Node_t rightNode;
	};

	//Nullのノード
	class Node_Null:public Node {
	public:
		Node_Null():Node(OP_NULL,"null") {
		}
		std::int32_t Push(Compiler* arg_compiler)const override;
	};

	// 変数ノード

	class Node_value : public Node {
	public:
		Node_value(const std::string& arg_name)
			: Node(OP_IDENTIFIER, arg_name)
		{
		};
		const ValueTag* GetValueTag(Compiler* arg_compiler)const override;
		std::int32_t Push(Compiler* arg_compiler) const override;
		std::int32_t PushClone(Compiler* arg_compiler) const override;
		std::int32_t Pop(Compiler* arg_compiler) const override;
		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const override;
	};

	// ノードリスト

	class NodeList {
	public:
		NodeList(Node_t arg_node)
		{
			vec_args.push_back(arg_node);
		}

		NodeList* Add(Node_t arg_add)
		{
			vec_args.push_back(arg_add);
			return this;
		}

		std::uint64_t size() const { return vec_args.size(); }
		Node_t get(std::uint64_t arg_index) const { return vec_args[arg_index]; }

		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const ;
	public:
		std::vector<Node_t> vec_args;
	};

	// 関数のノード
	class Node_Member;
	class Node_function : public Node {
	public:
		Node_function(std::int32_t arg_op, const Node_t& arg_node, const NodeList_t arg_list)
			: Node(arg_op, arg_node), nodeList(arg_list)
		{
		}
		Node_function(std::int32_t arg_op, const Node_t& arg_node, const std::vector<std::int32_t>& arg_templateTypes)
			: Node(arg_op, arg_node), vec_templateTypes(arg_templateTypes)
		{
		}
		void SetArgmentList(NodeList_t arg_List) {
			nodeList = arg_List;
		}
		virtual std::int32_t Push(Compiler* arg_compiler) const;
		virtual std::int32_t Pop(Compiler* arg_compiler) const;
		std::int32_t GetType(Compiler* arg_compiler)const override;

		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const override;
		Node_t CreateMethod(Node_t arg_node)override;
	private:
		NodeList_t nodeList;
		std::vector<std::int32_t> vec_templateTypes;
	};

	//メンバ変数へのアクセスノード
	class Node_Member :public Node {
	public:
		Node_Member(const std::int32_t arg_op, const Node_t& arg_valueNode,const std::string& arg_memberName)
			:Node(arg_op, arg_valueNode)
		{
			strData = (arg_memberName);
		}
		virtual std::int32_t Push(Compiler* arg_compiler) const;
		std::int32_t PushClone(Compiler* arg_compiler) const;
		virtual std::int32_t Pop(Compiler* arg_compiler) const;
		std::int32_t GetType(Compiler* arg_compiler)const override;

		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const override;
	private:
	};

	//メンバ関数のノード
	class Node_Method :public Node {
	public:
		
		Node_Method(const std::int32_t arg_op, const Node_t arg_funcNode, const Node_t arg_valueNode, const NodeList_t list, const std::vector<std::int32_t>& arg_templateTypes)
			:Node(arg_op, arg_funcNode,arg_valueNode), nodeList(list),vec_templateTypes(arg_templateTypes)
		{
		}
		virtual std::int32_t Push(Compiler* arg_compiler) const;
		virtual std::int32_t Pop(Compiler* arg_compiler) const;
		std::int32_t GetType(Compiler* arg_compiler)const override;

		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const override;
	private:
		NodeList_t nodeList;
		std::vector<std::int32_t> vec_templateTypes;
	};

	//enum呼び出しのノード
	class Node_enum:public Node {
	public:
		Node_enum(const Node_t arg_enumTypeNode, const std::string& arg_identiferName): Node(OP_INT, arg_enumTypeNode) {
			strData = arg_identiferName;
		}

		virtual std::int32_t Push(Compiler* arg_compiler) const;
		virtual std::int32_t Pop(Compiler* arg_compiler) const;
		std::int32_t GetType(Compiler* arg_compiler)const override;
		std::int32_t EnumType(Compiler* arg_compiler)const override;
	};

	//関数オブジェクトのノード
	class Node_FunctionObject :public Node {
	public:
		Node_FunctionObject(const std::string& arg_identiferName) : Node(OP_INT,arg_identiferName) {
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

		virtual void Add(const std::vector<Node_t >& arg_vec_node)
		{
			std::cerr << "内部エラー：Add(std::vector< node>)が呼ばれました" << std::endl;
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
		virtual void LambdaCapture(std::map<std::string,const ValueTag*>& arg_captureList, Compiler* arg_compiler){}
		virtual std::int32_t Case_Analyze(Compiler* arg_compiler, std::int32_t* arg_default_label)
		{
			return 0;
		}

		static Statement_t make_statement(const std::int32_t arg_vec_state);
		static Statement_t make_statement(const std::int32_t arg_vec_state,const std::int32_t );
		static Statement_t make_statement(const std::int32_t arg_vec_state, Node_t arg_node);
		static Statement_t make_statement(const std::int32_t arg_vec_state, Block_t arg_block);

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

		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) override;
	private:
		Node_t node;
	};

	// 関数呼び出し
	class ccall_statement : public Statement, public ButiEngine::enable_value_from_this<ccall_statement> {
	public:
		ccall_statement(Node_t arg_node)
			:  node(arg_node)
		{
		}

		std::int32_t Analyze(Compiler* arg_compiler);

		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) override;
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
		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) override;

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
		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) override;

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
			vec_statement[arg_index] = arg_statement;
		}

		std::int32_t Analyze(Compiler* arg_compiler);
		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) override;

	private:
		Node_t node;
		Statement_t vec_statement[2];
	};

	// for
	class Statement_for : public Statement {
	public:
		void Add(Statement_t arg_statement)
		{
			vec_statement = arg_statement;
		}

		void Add(const std::int32_t arg_index, Node_t arg_node)
		{
			node[arg_index] = arg_node;
		}

		std::int32_t Analyze(Compiler* arg_compiler);
		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) override;

	private:
		Statement_t vec_statement;
		Node_t node[3];
	};

	// while
	class Statement_while : public Statement {
	public:
		void Add(Statement_t arg_statement)
		{
			vec_statement = arg_statement;
		}

		void Add(Node_t arg_node)
		{
			node = arg_node;
		}

		std::int32_t Analyze(Compiler* arg_compiler);

		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) override;
	private:
		Node_t node;
		Statement_t vec_statement;
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
			vec_statement.push_back(arg_statement);
		}

		std::int32_t Analyze(Compiler* arg_compiler);

		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) override;
	private:
		Node_t node;
		std::vector<Statement_t> vec_statement;
	};

	// block
	class Statement_block : public Statement {
	public:
		Statement_block(Block_t arg_block)
			: block_(arg_block)
		{
		}

		std::int32_t Analyze(Compiler* arg_compiler);
		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) override;

	private:
		Block_t block_;
	};

	// 宣言

	class Declaration : public Statement {
	public:
		Declaration(const std::int32_t arg_type)
			:valueType(arg_type), isFunction(false)
		{
		}
		Declaration(const std::int32_t arg_type,const AccessModifier arg_access)
			:valueType(arg_type), isFunction(false)
		{
			if (arg_access == AccessModifier::Public || arg_access == AccessModifier::Private||arg_access==AccessModifier::Protected) {
				accessType = arg_access;
			}
		}

		Declaration(const std::int32_t arg_type, const std::string& arg_name)
			:valueType(arg_type), name(arg_name), isFunction(true)
		{
		}

		void Add(Node_t arg_node)
		{
			vec_node.push_back(arg_node);
		}
		void Add(const std::vector<Node_t >& arg_node)
		{
			vec_node=(arg_node);
		}

		void Add(const std::int32_t arg_type)
		{
			vec_argType.push_back(arg_type);
		}

		std::int32_t PushCompiler(Compiler* arg_compiler);
		std::int32_t Analyze(Compiler* arg_compiler);

		void Define(Compiler* arg_compiler);
		bool IsFunction()const {
			return isFunction;
		}
	private:
		std::int32_t valueType;					
		bool isFunction;				
		std::vector<Node_t> vec_node;	
		std::string name;			
		std::vector<std::int32_t> vec_argType;		
		AccessModifier accessType=AccessModifier::Public;
	};

	using Declaration_t = ButiEngine::Value_ptr<Declaration>;

	// ブロック

	class Block {
	public:
		void Add(const Declaration_t& arg_decl)
		{
			vec_decl.push_back(arg_decl);
		}
		void Add(const Statement_t& arg_state)
		{
			vec_state.push_back(arg_state);
		}

		std::int32_t Analyze(Compiler* arg_compiler, std::vector<Function_t>& arg_captureCheck);
		inline std::int32_t Analyze(Compiler* arg_compiler) {
			static std::vector<Function_t> captureCheckDummy;
			return Analyze(arg_compiler, captureCheckDummy);
		}
		void LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) ;
	private:
		std::vector<Declaration_t> vec_decl;
		std::vector<Statement_t> vec_state;
	};

	// 引数

	class NameSpace;
	using NameSpace_t = ButiEngine::Value_ptr<NameSpace>;
	// 関数
	class Function :public ButiEngine::enable_value_from_this<Function>{
	public:
		Function(const std::string& arg_name)
			: name(arg_name),searchName(arg_name)
		{
			accessType = AccessModifier::Public;
		}
		Function(const std::string& arg_name,const AccessModifier arg_access)
			: name(arg_name)
		{
			if (arg_access == AccessModifier::Public || arg_access == AccessModifier::Private || arg_access == AccessModifier::Protected) {
				accessType = arg_access;
			}
		}

		void Add(ArgDefine arg_argDefine)
		{
			args.push_back(arg_argDefine);
		}

		void Add(Block_t arg_block)
		{
			block = arg_block;
		}
		std::int32_t PushCompiler(Compiler* arg_compiler, FunctionTable* arg_p_funcTable = nullptr);
		std::int32_t PushCompiler_sub(Compiler* arg_compiler);
		virtual std::int32_t Analyze(Compiler* arg_compiler,FunctionTable* arg_p_funcTable =nullptr);
		void set_type(const std::int32_t arg_type) {
			returnType = arg_type;
		}
		void AddSubFunction(Function_t);
		const std::string& GetName()const {
			return name;
		}
		void SetParent(Function_t arg_function) { parentFunction = arg_function; }
		virtual void LambdaCapture( Compiler* arg_compiler){}
		void SetAccess(const AccessModifier arg_access) {
			if (arg_access == AccessModifier::Public || arg_access == AccessModifier::Private || arg_access == AccessModifier::Protected) {
				accessType = arg_access;
			}
		}
	protected:
		Function(){}
		std::int32_t returnType;
		std::string name,searchName;
		std::vector<ArgDefine> args;
		AccessModifier accessType=AccessModifier::Public;
		Block_t block;
		std::vector<Function_t> vec_subFunctions;
		Function_t parentFunction;
		NameSpace_t ownNameSpace;
	};

	class Lambda :public Function {
	public:
		Lambda(const std::int32_t arg_type,const std::vector<ArgDefine>& arg_args,Compiler* arg_compiler);
		std::int32_t PushCompiler(Compiler* arg_compiler);
		std::int32_t Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable = nullptr);
		void LambdaCapture(Compiler* arg_compiler)override;
	private:
		std::int32_t lambdaIndex;
		std::map<std::string, const ValueTag*> map_lambdaCapture;
	};
	using Lambda_t = ButiEngine::Value_ptr<Lambda>;

	class Class:public ButiEngine::enable_value_from_this<Class> {
	public:
		Class(const std::string& arg_name)
			: name(arg_name)
		{
		}
		std::int32_t Analyze(Compiler* arg_compiler);
		std::int32_t Regist(Compiler* arg_compiler);
		std::int32_t PushCompiler(Compiler* arg_compiler);
		void RegistMethod(Function_t arg_method, Compiler* arg_compiler);
		void SetValue(const std::string& arg_name, const std::int32_t arg_type,const AccessModifier arg_accessType);
	private:
		std::map < std::string, std::pair< std::int32_t,AccessModifier>> map_values;
		std::vector<Function_t> vec_methods;
		std::string name;
	};
	using Class_t = ButiEngine::Value_ptr<Class>;



	//列挙型

	class Enum {
	public:
		Enum(const std::string& arg_typeName):typeName(arg_typeName){}

		void SetIdentifer(const std::string& arg_name);
		void SetIdentifer(const std::string& arg_name,const std::int32_t arg_value);
		std::int32_t Analyze(Compiler* arg_compiler);
	private:
		std::string typeName;
		std::map<std::string, std::int32_t> map_identifer;
	};
	using Enum_t = ButiEngine::Value_ptr<Enum>;
}
#endif
