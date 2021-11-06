#include "stdafx.h"
#ifndef BOOST_INCLUDE_H
#define BOOST_INCLUDE_H
#include <boost/bind.hpp>
#include <boost/spirit.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix1_functions.hpp>
#include <boost/spirit/include/phoenix1_new.hpp>
#include <boost/mem_fn.hpp>
#endif
#include "Parser.h"
#include "Node.h"
#include"Compiler.h"
namespace ButiScript{

using iterator_t= boost::spirit::position_iterator<std::string::const_iterator>;

// エラー処理パーサー定義
struct error_parser {
	using result_t= boost::spirit::nil_t ;		// パーサーの結果型（nil_t）

	error_parser(char const* arg_msg)
		: message(arg_msg)
	{
	}

	template <typename ScannerT>
	int operator()(ScannerT const& arg_scan, result_t& arg_result) const
	{
		// 終わりまで来たら-1を返す
		if (arg_scan.at_end()) {
			return -1;
		}

		// 改行までをスキャンし、そこまでを表示する。

		iterator_t b = arg_scan.first;
		size_t length = (*(boost::spirit::anychar_p - '\n')).parse(arg_scan).length();
		boost::spirit::file_position fpos = arg_scan.first.get_position();
		std::cout << fpos.file << ": " << fpos.line << "." << fpos.column << ": "
			<< message << " : " << std::string(b, arg_scan.first) << std::endl;

//		return (int)length + 1;
		return -1;
	}

private:
	const char* message;
};

// エラー処理パーサー
using error_p= boost::spirit::functor_parser<error_parser> ;
// 文法エラー処理パーサー
const error_p syntax_error_p = error_parser("文法エラー");

//メンバ変数の情報
struct MemberValue {
	MemberValue(const int arg_index,const std::string& arg_name,const AccessModifier arg_accessType) {
		type= arg_index;
		name = arg_name;
		if ((int)arg_accessType <= (int)AccessModifier::Protected && (int)arg_accessType >= (int)AccessModifier::Public) {
			accessType = arg_accessType;
		}
	}
	MemberValue(){}
	int type;
	std::string name;
	AccessModifier accessType=AccessModifier::Public;
};

namespace BindFunctionObject {

// 単項演算子ノードを生成する
struct unary_node_func {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { using type = Node_t; };

	template <typename Ty1, typename Ty2, typename Ty3>
	Node_t operator()(Ty1 arg_operation, const Ty2& arg_left, Ty3 arg_compiler) const
	{
		return Node::make_node(arg_operation, arg_left, arg_compiler);
	}
};

// 二項演算子ノードを生成する
struct binary_node_func {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { using type = Node_t; };

	template <typename Ty1, typename Ty2, typename Ty3>
	Node_t operator()(Ty1 arg_operation, const Ty2& arg_left, const Ty3& arg_right) const
	{
		return Node::make_node(arg_operation, arg_left, arg_right);
	}
};

// 二項演算子ノードを生成する(コンパイラ使用)
struct binary_node_func_useDriver {
	template <typename Ty1, typename Ty2, typename Ty3, typename Ty4>
	struct result { using type = Node_t; };

	template <typename Ty1, typename Ty2, typename Ty3, typename Ty4>
	Node_t operator()(Ty1 arg_operation, const Ty2& arg_left, const Ty3& arg_right, const Ty4 arg_compiler) const
	{
		return Node::make_node(arg_operation, arg_left, arg_right, arg_compiler);
	}
};

//ラムダ式を値として利用するノードを生成する
struct lambda_node_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = Node_t; };

	template <typename Ty1, typename Ty2>
	Node_t operator()(Ty1 arg_lambdaIndex, Ty2 arg_compiler) const
	{
		return Node::make_node(OP_IDENTIFIER, "@lambda:" + std::to_string(arg_lambdaIndex), arg_compiler);
	}
};

// 値を追加
struct push_back_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = Ty1; };

	template <typename Ty1, typename Ty2>
	Ty1 operator()(Ty1& arg_list, const Ty2& arg_node) const
	{
		arg_list->Add(arg_node);
		return arg_list;
	}
};
// 値を追加(std::vector)
struct vector_push_back_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = void; };

	template <typename Ty1, typename Ty2>
	void operator()(Ty1& arg_vector, const Ty2& arg_object) const
	{
		arg_vector.push_back(arg_object);
	}
};

// ノードリストを生成する
struct make_argument_func {
	template <typename Ty>
	struct result { using type = NodeList_t; };

	template <typename Ty>
	NodeList_t operator()(const Ty& arg_node) const
	{
		return NodeList_t(new NodeList(arg_node));
	}
};

// ステートメントの生成
struct make_statement_func {
	template <typename Ty>
	struct result { using type = Statement_t; };

	template <typename Ty>
	Statement_t operator()(Ty arg_vec_state) const
	{
		return Statement::make_statement(arg_vec_state);
	}
};

// ステートメントの生成
struct make_statement1_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = Statement_t; };

	template <typename Ty1, typename Ty2>
	Statement_t operator()(Ty1 arg_vec_state, const Ty2& arg_node) const
	{
		return Statement::make_statement(arg_vec_state, arg_node);
	}
};

// ステートメントにインデックス付きで値を追加
struct add_statement_func {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { using type = Statement_t; };

	template <typename Ty1, typename Ty2, typename Ty3>
	Statement_t operator()(Ty1& arg_statement, Ty2 arg_index, const Ty3& arg_node) const
	{
		arg_statement->Add(arg_index, arg_node);
		return arg_statement;
	}
};

// 宣言の生成
struct make_decl_func {
	template <typename Ty>
	struct result { using type = Declaration_t; };

	template <typename Ty>
	Declaration_t operator()(Ty arg_type) const
	{
		return Declaration_t(new Declaration(arg_type));
	}
};

// 宣言の生成
struct make_decl1_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = Declaration_t; };

	template <typename Ty1, typename Ty2>
	Declaration_t operator()(Ty1 arg_type, const Ty2& arg_node) const
	{
		return Declaration_t(new Declaration(arg_type, arg_node));
	}
};

// 引数宣言の型を参照にする
struct arg_ref_func {
	template <typename Ty1>
	struct result { using type = ArgDefine; };

	template <typename Ty1>
	ArgDefine operator()(Ty1& arg_decl) const
	{
		arg_decl.SetRef();
		return arg_decl;
	}
};

// 引数の名前を設定
struct arg_name_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = ArgDefine; };

	template <typename Ty1, typename Ty2>
	ArgDefine operator()(Ty1& arg_decl, const Ty2& arg_name) const
	{
		arg_decl.SetName(arg_name);
		return arg_decl;
	}
};

//関数、変数呼び出し時に名前空間を保持する
struct call_namespace_func {
	template < typename Ty2>
	struct result { using type = std::string; };

	template <typename Ty2>
	std::string operator()(const Ty2& arg_name) const
	{
		return arg_name + "::";
	}
};

// 関数の生成
struct make_function_func {
	template < typename Ty1>
	struct result { using type = Function_t; };

	template <typename Ty1>
	Function_t operator()(const Ty1& arg_name) const
	{
		return Function_t(new Function(arg_name));
	}
};
// ラムダ式の生成
struct make_lambda_func {
	template < typename Ty1,typename Ty2>
	struct result { using type = Lambda_t; };

	template <typename Ty1,typename Ty2>
	Lambda_t operator()(const Ty1 arg_typeIndex,Ty2 arg_compiler ) const
	{
		auto output = Lambda_t(new Lambda(arg_typeIndex.first, arg_typeIndex.second,arg_compiler));
		arg_compiler->PushAnalyzeFunction(output);
		return output;
	}
};
// 関数の生成
struct make_functionWithAccess_func {
	template < typename Ty1, typename Ty2>
	struct result { using type = Function_t; };

	template < typename Ty1, typename Ty2>
	Function_t operator()(Ty1 arg_accessNameFunction, const Ty2& arg_name) const
	{
		return Function_t(new Function(arg_accessNameFunction, arg_name));
	}
};

//クラスの生成
struct make_class_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = Class_t; };

	template <typename Ty1, typename Ty2>
	Class_t operator()(const Ty1& arg_name,Ty2 arg_compiler) const
	{
		arg_compiler->PushNameSpace(NameSpace_t(new NameSpace(arg_name)));
		return Class_t(new Class(arg_name));
	}
};

//メンバ変数の生成
struct make_classMember_func {
	template < typename Ty1, typename Ty2>
	struct result { using type = void; };

	template < typename Ty1, typename Ty2>
	void operator()(Ty1 shp_class, const Ty2& type_and_name) const
	{
		shp_class->SetValue(type_and_name.name, type_and_name.type, type_and_name.accessType);
	}
};

//std::pairの生成
struct make_pair_func {
	template < typename Ty1, typename Ty2>
	struct result { using type = std::pair<Ty1, Ty2>; };

	template < typename Ty1, typename Ty2>
	std::pair<Ty1, Ty2> operator()(const Ty1 arg_first, const Ty2& arg_second) const
	{
		return { arg_first,arg_second };
	}
};

//MemberValueの生成
struct make_memberValue_func {
	template < typename Ty1, typename Ty2, typename Ty3>
	struct result { using type = MemberValue; };

	template < typename Ty1, typename Ty2, typename Ty3>
	MemberValue operator()(const Ty1 arg_index, const Ty2& arg_name, const Ty3 arg_accessType) const
	{
		return MemberValue(arg_index, arg_name, arg_accessType);
	}
};

//列挙型の生成
struct make_enum_func {
	template < typename Ty2>
	struct result { using type = Enum_t; };

	template <typename Ty2>
	Enum_t operator()(const Ty2& arg_name) const
	{
		return Enum_t(new Enum(arg_name));
	}
};
//列挙型の追加
struct add_enum_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = void; };

	template <typename Ty1, typename Ty2>
	void operator()(Ty1 arg_enum_t, const Ty2& arg_name) const
	{
		arg_enum_t->SetIdentifer(arg_name);
	}
};

// 名前空間の生成
struct make_namespace_func {
	template < typename Ty2>
	struct result { using type = NameSpace_t; };

	template <typename Ty2>
	NameSpace_t operator()(const Ty2& arg_name) const
	{
		return NameSpace_t(new NameSpace(arg_name));
	}
};

// 関数の返り値設定
struct setFunctionType_func {
	template <typename Ty1, typename Ty2,typename Ty3>
	struct result { using type = Function_t; };

	template <typename Ty1, typename Ty2,typename Ty3>
	Function_t operator()(Ty1& arg_decl, Ty2 arg_type,Ty3 arg_compiler) const
	{
		arg_decl->set_type(arg_type);
		arg_compiler->PushAnalyzeFunction(arg_decl);
		return arg_decl;
	}
};

// 関数、名前空間、クラス登録
struct regist_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = void; };

	template <typename Ty1, typename Ty2>
	void operator()(const Ty1& arg_decl, Ty2 arg_compiler) const
	{
		arg_decl->Regist(arg_compiler);
	}
};

//アクセス指定子特定
struct accessModifier_func {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { using type = std::string; };

	template <typename Ty1, typename Ty2, typename Ty3>
	std::string operator()(const Ty1& arg_modifierStr, Ty2& arg_ref_accesModifier, const Ty3& arg_ret) const
	{
		arg_ref_accesModifier = StringToAccessModifier(arg_modifierStr);
		return arg_ret;
	}
};

// メソッド登録
struct registMethod_func {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { using type = void; };

	template <typename Ty1, typename Ty2, typename Ty3>
	void operator()(const Ty1& arg_vec_decl, Ty2 arg_type, Ty3 arg_compiler) const
	{
		arg_type->RegistMethod(arg_vec_decl, arg_compiler);
	}
};

//名前空間からの離脱
struct pop_nameSpace_func {
	template < typename Ty2>
	struct result { using type = void; };

	template < typename Ty2>
	void operator()(Ty2 arg_compiler) const
	{
		arg_compiler->PopNameSpace();
	}
};

//std::cout デバッグ用
struct	cout_func {
	template <typename Ty1>
	struct result { using type = void; };

	template <typename Ty1>
	void operator()(const Ty1& arg_message) const
	{
		std::cout << arg_message << std::endl;
	}
};
//型の特定
struct	specificType_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = int; };

	template <typename Ty1, typename Ty2>
	int operator()(const Ty1& arg_key, Ty2 arg_compiler) const
	{
		return  arg_compiler->GetTypeIndex(arg_key);
	}
};
//型の特定
struct	specificFunctionType_func {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { using type = int; };

	template <typename Ty1, typename Ty2, typename Ty3>
	int operator()(const Ty1& arg_retType, const Ty2& arg_args, Ty3 arg_compiler) const
	{
		return  arg_compiler->GetfunctionTypeIndex(arg_args, arg_retType);
	}
};


// 最終登録
struct analyze_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = int; };

	template <typename Ty1, typename Ty2>
	int operator()(Ty1 arg_decl, Ty2 arg_compiler) const
	{
		return arg_decl->Analyze(arg_compiler);
	}
};
// 関数、ラムダ登録
struct pushConpiler_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = int; };

	template <typename Ty1, typename Ty2>
	int operator()(Ty1 arg_decl, Ty2 arg_compiler) const
	{
		return arg_decl->PushCompiler(arg_compiler);
	}
};
// ブロック内関数登録
struct pushConpiler_sub_func {
	template <typename Ty1, typename Ty2>
	struct result { using type = int; };

	template <typename Ty1, typename Ty2>
	int operator()(Ty1 arg_decl, Ty2 arg_compiler) const
	{
		return arg_decl->PushCompiler_sub(arg_compiler);
	}
};
//ブロック作成
struct make_block_func {
	template <typename Ty1>
	struct result { using type = Block_t; };
	template<typename Ty1>
	Block_t operator()(Ty1 arg_compiler)const {
		return std::make_shared<Block>();
	}
};

}

// phoenixが使用する無名関数用の関数
const phoenix::function<BindFunctionObject::binary_node_func>  binary_node =						BindFunctionObject::binary_node_func();
const phoenix::function<BindFunctionObject::binary_node_func_useDriver>		binary_node_comp =		BindFunctionObject::binary_node_func_useDriver();
const phoenix::function<BindFunctionObject::unary_node_func>  unary_node =							BindFunctionObject::unary_node_func();
const phoenix::function<BindFunctionObject::lambda_node_func>  lambda_node=							BindFunctionObject::lambda_node_func();
const phoenix::function<BindFunctionObject::push_back_func>  push_back =							BindFunctionObject::push_back_func();
const phoenix::function<BindFunctionObject::vector_push_back_func>  vec_push_back =					BindFunctionObject::vector_push_back_func();
const phoenix::function<BindFunctionObject::make_argument_func>  make_argument =					BindFunctionObject::make_argument_func();
const phoenix::function<BindFunctionObject::make_statement_func>  make_statement =					BindFunctionObject::make_statement_func();
const phoenix::function<BindFunctionObject::make_statement1_func>  make_statement1 =				BindFunctionObject::make_statement1_func();
const phoenix::function<BindFunctionObject::add_statement_func>  add_statement =					BindFunctionObject::add_statement_func();
const phoenix::function<BindFunctionObject::make_decl_func>  make_decl =							BindFunctionObject::make_decl_func();
const phoenix::function<BindFunctionObject::make_decl1_func>  make_decl1 =							BindFunctionObject::make_decl1_func();
const phoenix::function<BindFunctionObject::arg_ref_func>  arg_ref =								BindFunctionObject::arg_ref_func();
const phoenix::function<BindFunctionObject::arg_name_func>	arg_name =								BindFunctionObject::arg_name_func();
const phoenix::function<BindFunctionObject::make_function_func>	make_function =						BindFunctionObject::make_function_func();
const phoenix::function<BindFunctionObject::make_lambda_func>  make_lambda =							BindFunctionObject::make_lambda_func();
const phoenix::function<BindFunctionObject::make_functionWithAccess_func>make_functionWithAccess=	BindFunctionObject::make_functionWithAccess_func();
const phoenix::function<BindFunctionObject::make_class_func>  make_class =							BindFunctionObject::make_class_func();
const phoenix::function<BindFunctionObject::make_classMember_func>  make_classMember =				BindFunctionObject::make_classMember_func();
const phoenix::function<BindFunctionObject::add_enum_func>  add_enum =								BindFunctionObject::add_enum_func();
const phoenix::function<BindFunctionObject::make_enum_func>  make_enum =							BindFunctionObject::make_enum_func();
const phoenix::function<BindFunctionObject::make_namespace_func>  make_namespace =					BindFunctionObject::make_namespace_func();
const phoenix::function<BindFunctionObject::make_pair_func>  make_pair =							BindFunctionObject::make_pair_func();
const phoenix::function<BindFunctionObject::make_memberValue_func>  make_memberValue =				BindFunctionObject::make_memberValue_func();
const phoenix::function<BindFunctionObject::setFunctionType_func>  set_functionType =				BindFunctionObject::setFunctionType_func();
const phoenix::function<BindFunctionObject::specificType_func>  specificType =						BindFunctionObject::specificType_func();
const phoenix::function<BindFunctionObject::specificFunctionType_func>  specificFunctionType =		BindFunctionObject::specificFunctionType_func();
const phoenix::function<BindFunctionObject::accessModifier_func>  specificAccessModifier =			BindFunctionObject::accessModifier_func();
const phoenix::function<BindFunctionObject::analyze_func>  analyze =								BindFunctionObject::analyze_func();
const phoenix::function<BindFunctionObject::pushConpiler_sub_func>  pushCompiler_sub =				BindFunctionObject::pushConpiler_sub_func();
const phoenix::function<BindFunctionObject::pushConpiler_func>  pushCompiler =						BindFunctionObject::pushConpiler_func();
const phoenix::function<BindFunctionObject::regist_func>  regist =									BindFunctionObject::regist_func();
const phoenix::function<BindFunctionObject::registMethod_func>  registMethod =						BindFunctionObject::registMethod_func();
const phoenix::function<BindFunctionObject::pop_nameSpace_func>  popNameSpace =						BindFunctionObject::pop_nameSpace_func();
const phoenix::function<BindFunctionObject::call_namespace_func>  functionCall_namespace =			BindFunctionObject::call_namespace_func();
const phoenix::function<BindFunctionObject::cout_func>  cout =										BindFunctionObject::cout_func();
const phoenix::function<BindFunctionObject::make_block_func>  make_block=							BindFunctionObject::make_block_func();

const boost::spirit::real_parser<double, boost::spirit::ureal_parser_policies<double> >  ureal_parser = boost::spirit::real_parser<double, boost::spirit::ureal_parser_policies<double> >();

namespace ButiClosure {

	// 文字列のクロージャ
	struct string_val : boost::spirit::closure<string_val, std::string> {
		member1 str;
	};
	// 整数のクロージャ
	struct number_val : boost::spirit::closure<number_val, unsigned int> {
		member1 number;
	};
	// 浮動小数クロージャ
	struct float_val : boost::spirit::closure<float_val, float> {
		member1 number;
	};

	// ノードのクロージャ
	struct node_val : boost::spirit::closure<node_val, Node_t, int, std::string> {
		member1 node;
		member2 Op;
		member3 name;
	};
	// メンバ呼び出しのクロージャ
	struct callmember_val : boost::spirit::closure<callmember_val, Node_t, int, std::string> {
		member1 memberNode;
		member1 valueNode;
		member2 Op;
		member3 name;
	};

	// 名前空間のクロージャ
	struct namespace_val : boost::spirit::closure<namespace_val,  std::string> {
		member1 name;
	};
	// ノードのクロージャ
	struct nodelist_val : boost::spirit::closure<nodelist_val, NodeList_t, int> {
		member1 node;
		member2 Op;
	};
	// 文のクロージャ
	struct state_val : boost::spirit::closure<state_val, Statement_t> {
		member1 statement;
	};
	// 型のクロージャ
	struct type_val : boost::spirit::closure<type_val, int> {
		member1 type;
	};
	// 関数型のクロージャ
	struct type_func_val : boost::spirit::closure<type_func_val, std::pair< int, std::vector<ArgDefine>>,std::vector<ArgDefine>> {
		member1 type;
		member2 argments;
	};
	//ラムダ式定義のクロージャ
	struct lambda_val :boost::spirit::closure<lambda_val, Lambda_t, int, std::vector<int>> {
		member1 node;
		member2 type;
	};
	//ラムダ式利用のクロージャ
	struct lambda_prime_val :boost::spirit::closure<lambda_prime_val, Node_t> {
		member1 node;
	};

	// 変数定義のクロージャ
	struct decl_val : boost::spirit::closure<decl_val, Declaration_t, int,std::vector< Node_t>,AccessModifier> {
		member1 node;
		member2 type;
		member3 value;
		member4 access;
	};

	// 関数定義のクロージャ
	struct func_val : boost::spirit::closure<func_val, Function_t, int, std::string> {
		member1 node;
		member2 type;
		member3 name;
	};
	// クラスメンバ定義のクロージャ
	struct classMember_val : boost::spirit::closure<classMember_val, MemberValue,std::string, AccessModifier> {
		member1 memberValue;
		member2 name;
		member3 accessModifier;
	};
	// クラス定義のクロージャ
	struct class_val : boost::spirit::closure<class_val, Class_t, std::string> {
		member1 Class;
		member2 name;
	};
	//列挙型定義のクロージャ
	struct enum_val : boost::spirit::closure<enum_val, Enum_t> {
		member1 enum_t;
	};

	// 引数定義のクロージャ
	struct argdef_val : boost::spirit::closure<argdef_val, ArgDefine, std::string> {
		member1 node;
		member2 name;
	};

	// 文ブロックのクロージャ
	struct block_val : boost::spirit::closure<block_val, Block_t> {
		member1 node;
	};
}

//型登録
struct typeRegist_grammer : public boost::spirit::grammar<typeRegist_grammer> {
	typeRegist_grammer(Compiler* arg_compiler)
		:compiler(arg_compiler)
	{
	}
	Compiler* compiler;
	template <typename ScannerT>
	struct definition {

		boost::spirit::rule<ScannerT, ButiClosure::string_val::context_t>	identifier;
		boost::spirit::rule<ScannerT, ButiClosure::namespace_val::context_t>	nameSpace;
		boost::spirit::rule<ScannerT, ButiClosure::namespace_val::context_t>	nameSpace_call;
		boost::spirit::rule<ScannerT, ButiClosure::enum_val::context_t>		Enum;
		boost::spirit::rule<ScannerT, ButiClosure::class_val::context_t>		define_class;
		boost::spirit::rule<ScannerT>	decl_value,decl_classMember,Value,function, argdef, type,funcType,string_node, number, floatNumber, func_node, prime, unary, mul_expr, add_expr, shift_expr, bit_expr, equ_expr,
			and_expr, expr, assign, argument, statement, arg, decl_func, callMember, block, input, ident,lambda;

		boost::spirit::symbols<> keywords;
		boost::spirit::symbols<> mul_op, add_op, shift_op, bit_op, equ_op, assign_op;
		boost::spirit::dynamic_distinct_parser<ScannerT> keyword_p;
		definition(typeRegist_grammer const& self)
			:keyword_p(boost::spirit::alnum_p | '_')
		{
			using phoenix::arg1;
			using phoenix::arg2;
			using phoenix::var;
			using phoenix::new_;
			using phoenix::construct_;

			keywords = "if", "for", "while", "switch", "case", "default", "break", "return", "namespace";


			// 識別子
			ident = boost::spirit::lexeme_d[
				((boost::spirit::alpha_p | '_') >> *(boost::spirit::alnum_p | '_')) - (keywords >> boost::spirit::anychar_p - (boost::spirit::alnum_p | '_'))
			];
			// 識別子（クロージャに登録）
			identifier = ident[identifier.str = construct_<std::string>(arg1, arg2)];


			Enum = "enum" >> identifier[Enum.enum_t = make_enum(arg1)] >> "{" >>
				!identifier[add_enum(Enum.enum_t, arg1)]
				>> *(',' >> identifier[add_enum(Enum.enum_t, arg1)]) >>
				"}";

			//整数
			number = boost::spirit::uint_p;

			//浮動小数
			floatNumber = boost::spirit::strict_real_p;

			// 文字列
			string_node = boost::spirit::lexeme_d[
				boost::spirit::confix_p(boost::spirit::ch_p('"'), *boost::spirit::c_escape_ch_p, '"')
			];


			//名前空間からの呼び出し
			nameSpace_call = identifier[nameSpace_call.name = arg1] >> "::";

			// 変数
			Value = (*(nameSpace_call)) >>
				identifier;

			// 関数の引数
			argument = expr
				>> *(',' >> expr);

			// 関数呼び出し
			func_node = *(identifier >> "::") >> identifier >>
				'(' >> !argument >> ')';

			//メンバ変数呼び出し
			callMember = Value >> "."
				>> identifier
				>> !('(' >> !argument >> ')')
				>> *("." >>
					identifier >>
					!('(' >> !argument >> ')')
					);

			// 計算のprimeノード
			prime = lambda
				|callMember
				| func_node
				| Value
				| floatNumber
				| number
				| string_node
				| '(' >> expr >> ')'
			
				;

			// 単項演算子
			unary = prime
				| '-' >> prime;

			// 二項演算子（*, /, %）
			mul_op.add("*", OP_MUL)("/", OP_DIV)("%", OP_MOD);
			mul_expr = unary
				>> *(mul_op
					>> unary);

			// 二項演算子（+, -）
			add_op.add("+", OP_ADD)("-", OP_SUB);
			add_expr = mul_expr
				>> *(add_op
					>> mul_expr);

			// 二項演算子（<<, >>）
			shift_op.add("<<", OP_LSHIFT)(">>", OP_RSHIFT);
			shift_expr = add_expr
				>> *(shift_op
					>> add_expr);

			// 二項演算子（&, |）
			bit_op.add("&", OP_AND)("|", OP_OR);
			bit_expr = shift_expr
				>> *(bit_op
					>> shift_expr);

			// 二項演算子（比較）
			equ_op.add("==", OP_EQ)("!=", OP_NE)(">=", OP_GE)(">", OP_GT)("<=", OP_LE)("<", OP_LT);
			equ_expr = bit_expr
				>> !(equ_op
					>> bit_expr);

			// 二項演算子（&&）
			and_expr = equ_expr
				>> *("&&" >> equ_expr);

			// 二項演算子（||）
			expr = and_expr
				>> *("||" >> and_expr);

			// 代入
			assign_op.add
			("=", OP_ASSIGN)
				("+=", OP_ADD_ASSIGN)
				("-=", OP_SUB_ASSIGN)
				("*=", OP_MUL_ASSIGN)
				("/=", OP_DIV_ASSIGN)
				("%=", OP_MOD_ASSIGN)
				("&=", OP_AND_ASSIGN)
				("|=", OP_OR_ASSIGN)
				("<<=", OP_LSHIFT_ASSIGN)
				(">>=", OP_RSHIFT_ASSIGN);
			assign = (callMember | Value)
				>> assign_op
				>> (expr);

			// 変数宣言
			decl_value = !(boost::spirit::str_p("private") | boost::spirit::str_p("public")) >> "var" >> Value % ',' >> ':' >> type >> ';';

			// 型名
			type = identifier >> !boost::spirit::ch_p('&')
				| funcType;
			//関数型名
			funcType = '(' >> !(arg % ',') >> ')' >> "=>" >> type;

			lambda = funcType >> block;

			// 関数宣言の引数
			arg = identifier >> ':'
				>> type
				>> !boost::spirit::str_p("[]");

			// 関数宣言
			decl_func = identifier
				>> '(' >> !(arg % ',') >> ')' >> ":" >> type >> ';';

			// 関数定義の引数
			argdef = identifier >> ':'
				>> type
				>> !boost::spirit::str_p("[]");

			// 関数定義
			function = identifier >> !identifier
				>> '(' >> !(argdef% ',') >> ')' >>
				':' >> type
				>> block;

			// 文ブロック
			block = boost::spirit::ch_p('{')
				>> *(statement
					| decl_value)
				>> '}';
			//クラスのメンバー定義
			decl_classMember = identifier >> !identifier
				>> ':' >> type >> ';';

			//クラス定義
			define_class = "class" >> identifier[define_class.Class = make_class(arg1,self.compiler)] >> "{" >>
				*(decl_classMember
					| function)
				>> boost::spirit::ch_p('}')[popNameSpace(self.compiler)];
			// 文
			statement = boost::spirit::ch_p(';')
				| assign >> ';'
				| boost::spirit::str_p("case") >> expr >> ':'
				| boost::spirit::str_p("default") >> ':'
				| boost::spirit::str_p("break") >> ';'
				| boost::spirit::str_p("return")
				>> !expr >> ';'
				| boost::spirit::str_p("if")
				>> '(' >> expr >> ')'
				>> statement
				>> !("else"
					>> statement)

				| boost::spirit::str_p("for") >> '('
				>> !(assign) >> ';'
				>> expr >> ';'
				>> !(assign || func_node || callMember) >> ')'
				>> statement

				| boost::spirit::str_p("while") >> '('
				>> expr >> ')'
				>> statement
				| boost::spirit::str_p("switch") >> '('
				>> expr >> ')'
				>> '{'
				>> *statement
				>> '}'
				| func_node >> ';'
				| callMember >> ';'
				| function
				| block
				;

			nameSpace = boost::spirit::str_p("namespace") >> identifier[regist(make_namespace(arg1), self.compiler)] >> "{"
				>> *(define_class[regist(arg1, self.compiler)]
					| Enum[analyze(arg1, self.compiler)]
					| function
					| decl_func
					| decl_value
					| nameSpace[popNameSpace(self.compiler)]) >> "}";

			// 入力された構文
			input = *(define_class[regist(arg1, self.compiler)]
				| Enum[analyze(arg1, self.compiler)]
				| function
				| decl_func
				| decl_value
				| nameSpace[popNameSpace(self.compiler)]
				|lambda
				| syntax_error_p
				);
		}

		boost::spirit::rule<ScannerT> const& start() const
		{
			return input;
		}
	};
};


// 関数、クラス解析
struct funcAnalyze_grammer : public boost::spirit::grammar<funcAnalyze_grammer> {
	funcAnalyze_grammer(Compiler* arg_compiler)
		:compiler(arg_compiler)
	{
	}
	Compiler* compiler;
	template <typename ScannerT>
	struct definition {

		boost::spirit::rule<ScannerT, ButiClosure::string_val::context_t>	identifier;
		boost::spirit::rule<ScannerT, ButiClosure::string_val::context_t>	string_node;
		boost::spirit::rule<ScannerT, ButiClosure::number_val::context_t>	number;
		boost::spirit::rule<ScannerT, ButiClosure::float_val::context_t>	floatNumber;
		boost::spirit::rule<ScannerT, ButiClosure::type_val::context_t>		type;
		boost::spirit::rule<ScannerT, ButiClosure::type_func_val::context_t>		funcType;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		func_node;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		Value;
		boost::spirit::rule<ScannerT, ButiClosure::callmember_val::context_t> callMember;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		prime;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		unary;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		mul_expr;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		add_expr;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		shift_expr;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		bit_expr;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		equ_expr;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		and_expr;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		expr;
		boost::spirit::rule<ScannerT, ButiClosure::node_val::context_t>		assign;
		boost::spirit::rule<ScannerT, ButiClosure::nodelist_val::context_t>	argument;
		boost::spirit::rule<ScannerT, ButiClosure::state_val::context_t>	statement;
		boost::spirit::rule<ScannerT, ButiClosure::func_val::context_t>		function;
		boost::spirit::rule<ScannerT, ButiClosure::class_val::context_t>		define_class;
		boost::spirit::rule<ScannerT, ButiClosure::type_val::context_t>		arg;
		boost::spirit::rule<ScannerT, ButiClosure::decl_val::context_t>		decl_value;
		boost::spirit::rule<ScannerT, ButiClosure::decl_val::context_t>		decl_func;
		boost::spirit::rule<ScannerT, ButiClosure::argdef_val::context_t>	argdef;
		boost::spirit::rule<ScannerT, ButiClosure::block_val::context_t>	block;
		boost::spirit::rule<ScannerT, ButiClosure::namespace_val::context_t>	nameSpace;
		boost::spirit::rule<ScannerT, ButiClosure::namespace_val::context_t>	nameSpace_call;
		boost::spirit::rule<ScannerT, ButiClosure::lambda_prime_val::context_t>	lambda_prime;
		boost::spirit::rule<ScannerT, ButiClosure::lambda_val::context_t>		lambda;
		boost::spirit::rule<ScannerT, ButiClosure::classMember_val::context_t>		decl_classMember;
		boost::spirit::rule<ScannerT>							input, Enum;
		boost::spirit::rule<ScannerT>							ident;

		boost::spirit::symbols<> keywords;
		boost::spirit::symbols<> mul_op, add_op, shift_op, bit_op, equ_op, assign_op;
		boost::spirit::dynamic_distinct_parser<ScannerT> keyword_p;

		definition(funcAnalyze_grammer const& self)
			:keyword_p(boost::spirit::alnum_p | '_')
		{
			using phoenix::arg1;
			using phoenix::arg2;
			using phoenix::var;
			using phoenix::new_;
			using phoenix::construct_;
			keywords = "if", "for", "while", "switch", "case", "default", "break", "return", "namespace";
			// 識別子
			ident = boost::spirit::lexeme_d[
				((boost::spirit::alpha_p | '_') >> *(boost::spirit::alnum_p | '_')) - (keywords >> boost::spirit::anychar_p - (boost::spirit::alnum_p | '_'))
			];
			// 識別子（クロージャに登録）
			identifier = ident[identifier.str = construct_<std::string>(arg1, arg2)];
			

			Enum = "enum" >> identifier >> "{" >>
				identifier
				>> *(',' >> identifier) >>
				"}";

			//名前空間からの呼び出し
			nameSpace_call = identifier[nameSpace_call.name = arg1] >> "::";

			//整数
			number = boost::spirit::uint_p[number.number = arg1];

			//浮動小数
			floatNumber = boost::spirit::strict_real_p[floatNumber.number = arg1];

			// 文字列
			string_node = boost::spirit::lexeme_d[
				boost::spirit::confix_p(boost::spirit::ch_p('"')[string_node.str = ""], *boost::spirit::c_escape_ch_p[string_node.str += arg1], '"')
			];

			// 変数
			Value = (*(nameSpace_call[Value.name += functionCall_namespace(arg1)])) >>
				identifier[Value.node = unary_node(OP_IDENTIFIER, Value.name+ arg1, self.compiler)]
				>> !('[' >> expr[Value.node = binary_node(OP_ARRAY, Value.node, arg1)] >> ']');

			// 関数の引数
			argument = expr[argument.node = make_argument(arg1)]
				>> *(',' >> expr[argument.node = push_back(argument.node, arg1)]);

			// 関数呼び出し
			func_node = (*(nameSpace_call[func_node.name += functionCall_namespace(arg1) ]))>>
				identifier[func_node.node = unary_node(OP_FUNCTION, func_node.name+arg1, self.compiler)] >>
				'(' >> !argument[func_node.node = binary_node(OP_FUNCTION, func_node.node, arg1)] >> ')';


			//メンバ呼び出し
			callMember =Value[ callMember.valueNode=arg1]>>"."
				>> identifier[callMember.memberNode = binary_node_comp(OP_MEMBER, callMember.valueNode,arg1, self.compiler)]
				>>!(boost::spirit::ch_p('(' )[callMember.memberNode = unary_node(OP_METHOD, callMember.memberNode, self.compiler)]>> !argument[callMember.memberNode = binary_node(OP_METHOD, callMember.memberNode, arg1)] >> ')')
				>>* (".">> 
					identifier[callMember.memberNode = binary_node_comp(OP_MEMBER, callMember.memberNode, arg1, self.compiler)]  >>  
					!(boost::spirit::ch_p('(')[callMember.memberNode = unary_node(OP_METHOD, callMember.memberNode, self.compiler)] >> !argument[callMember.memberNode = binary_node(OP_METHOD, callMember.memberNode, arg1)] >> ')')
					)
				
				;

			// 計算のprimeノード
			prime = lambda_prime[prime.node=arg1]
				| callMember[prime.node = arg1]
				|func_node[prime.node = arg1]
				| Value[prime.node = arg1]
				| floatNumber[prime.node = unary_node(OP_FLOAT, arg1, self.compiler)]
				| number[prime.node = unary_node(OP_INT, arg1, self.compiler)]
				| string_node[prime.node = unary_node(OP_STRING, arg1, self.compiler)]
				| '(' >> expr[prime.node = arg1] >> ')'
				;

			// 単項演算子
			unary = prime[unary.node = arg1]
				| '-' >> prime[unary.node = unary_node(OP_NEG, arg1, self.compiler)];

			// 二項演算子（*, /, %）
			mul_op.add("*", OP_MUL)("/", OP_DIV)("%", OP_MOD);
			mul_expr = unary[mul_expr.node = arg1]
				>> *(mul_op[mul_expr.Op = arg1]
					>> unary[mul_expr.node = binary_node(mul_expr.Op, mul_expr.node, arg1)]);

			// 二項演算子（+, -）
			add_op.add("+", OP_ADD)("-", OP_SUB);
			add_expr = mul_expr[add_expr.node = arg1]
				>> *(add_op[add_expr.Op = arg1]
					>> mul_expr[add_expr.node = binary_node(add_expr.Op, add_expr.node, arg1)]);

			// 二項演算子（<<, >>）
			shift_op.add("<<", OP_LSHIFT)(">>", OP_RSHIFT);
			shift_expr = add_expr[shift_expr.node = arg1]
				>> *(shift_op[shift_expr.Op = arg1]
					>> add_expr[shift_expr.node = binary_node(shift_expr.Op, shift_expr.node, arg1)]);

			// 二項演算子（&, |）
			bit_op.add("&", OP_AND)("|", OP_OR);
			bit_expr = shift_expr[bit_expr.node = arg1]
				>> *(bit_op[bit_expr.Op = arg1]
					>> shift_expr[bit_expr.node = binary_node(bit_expr.Op, bit_expr.node, arg1)]);

			// 二項演算子（比較）
			equ_op.add("==", OP_EQ)("!=", OP_NE)(">=", OP_GE)(">", OP_GT)("<=", OP_LE)("<", OP_LT);
			equ_expr = bit_expr[equ_expr.node = arg1]
				>> !(equ_op[equ_expr.Op = arg1]
					>> bit_expr[equ_expr.node = binary_node(equ_expr.Op, equ_expr.node, arg1)]);

			// 二項演算子（&&）
			and_expr = equ_expr[and_expr.node = arg1]
				>> *("&&" >> equ_expr[and_expr.node = binary_node(OP_LOGAND, and_expr.node, arg1)]);

			// 二項演算子（||）
			expr = and_expr[expr.node = arg1]
				>> *("||" >> and_expr[expr.node = binary_node(OP_LOGOR, expr.node, arg1)]);

			// 代入
			assign_op.add
			("=", OP_ASSIGN)
				("+=", OP_ADD_ASSIGN)
				("-=", OP_SUB_ASSIGN)
				("*=", OP_MUL_ASSIGN)
				("/=", OP_DIV_ASSIGN)
				("%=", OP_MOD_ASSIGN)
				("&=", OP_AND_ASSIGN)
				("|=", OP_OR_ASSIGN)
				("<<=", OP_LSHIFT_ASSIGN)
				(">>=", OP_RSHIFT_ASSIGN);
			assign = (callMember[assign.node = arg1]|Value[assign.node = arg1] )
				>> assign_op[assign.Op = arg1]
				>> expr[assign.node = binary_node(assign.Op, assign.node, arg1)];

			// 変数宣言
			decl_value = !(boost::spirit::str_p("private")[decl_value.access = AccessModifier::Private] | boost::spirit::str_p("public")[decl_value.access = AccessModifier::Public])
				>> "var" >> Value[vec_push_back(decl_value.value, arg1)] % ',' >> ':' >> type[decl_value.node = push_back(make_decl1(arg1, decl_value.access), decl_value.value)] >> ';';


			// 型名
			type = (identifier[type.type = specificType(arg1, self.compiler)] >> !boost::spirit::ch_p('&')[type.type |= TYPE_REF])
				|funcType[type.type= specificType(arg1,self.compiler)];
			//関数型名
			funcType = '(' >> !(argdef[vec_push_back(funcType.argments,arg1)] % ',') >> ')' >> "=>" >> type[funcType.type = make_pair(specificFunctionType(arg1, funcType.argments, self.compiler), funcType.argments)];

			lambda = funcType[lambda.node = make_lambda(arg1,self.compiler)] >> block[lambda.node=push_back(lambda.node,arg1)];

			lambda_prime = lambda[lambda_prime.node= lambda_node(pushCompiler(arg1, self.compiler),self.compiler)];

			// 関数宣言の引数
			arg = identifier >> ':'
				>> type[arg.type = arg1]
				>> !boost::spirit::str_p("[]")[arg.type |= TYPE_REF];

			// 関数宣言
			decl_func =  identifier[decl_func.node = make_decl1(decl_func.type, arg1)]
				>> '(' >> !(arg[decl_func.node = push_back(decl_func.node, arg1)] % ',') >> ')'>>":">>type[decl_func.type = arg1] >> ';';

			// 関数定義の引数
			argdef = identifier[argdef.name = arg1] >> ':'
				>> type[argdef.node = construct_<ArgDefine>(arg1, argdef.name)]
				>> !boost::spirit::str_p("[]")[argdef.node = arg_ref(argdef.node)];

			// 関数定義
			function = identifier[function.node = make_function(arg1)]>>!(identifier[function.node = make_functionWithAccess(function.node, arg1)])
				>> '(' >> !(argdef[function.node = push_back(function.node, arg1)] % ',') >> ')' >>
				':' >> type[function.node = set_functionType(function.node, arg1,self.compiler)]
				>> block[function.node = push_back(function.node, arg1)];

			//クラスのメンバー定義
			decl_classMember = identifier[decl_classMember.name = arg1] >> !identifier[decl_classMember.name = specificAccessModifier(decl_classMember.name, decl_classMember.accessModifier, arg1)]
				>> ':' >> type[decl_classMember.memberValue = make_memberValue(arg1, decl_classMember.name, decl_classMember.accessModifier)] >> ';';


			//クラス定義
			define_class = "class" >> identifier[define_class.Class = make_class(arg1, self.compiler)]>> "{" >>
				*(decl_classMember[make_classMember(define_class.Class, arg1)] 
					|  function[registMethod(arg1, define_class.Class, self.compiler)])
				>>boost::spirit::ch_p( '}')[popNameSpace(self.compiler)];

			// 文ブロック
			block = boost::spirit::ch_p('{')[block.node = make_block(self.compiler)]
				>> *(statement[block.node = push_back(block.node, arg1)]
					| decl_value[block.node = push_back(block.node, arg1)])
				>> boost::spirit::ch_p('}');

			// 文
			statement = boost::spirit::ch_p(';')[statement.statement = make_statement(NOP_STATE)]
				| assign[statement.statement = make_statement1(ASSIGN_STATE, arg1)] >> ';'
				| boost::spirit::str_p("case") >> expr[statement.statement = make_statement1(CASE_STATE, arg1)] >> ':'
				| boost::spirit::str_p("default")[statement.statement = make_statement(DEFAULT_STATE)] >> ':'
				| boost::spirit::str_p("break")[statement.statement = make_statement(BREAK_STATE)] >> ';'
				| boost::spirit::str_p("return")[statement.statement = make_statement(RETURN_STATE)]
				>> !expr[statement.statement = push_back(statement.statement, arg1)] >> ';'

				| boost::spirit::str_p("if")[statement.statement = make_statement(IF_STATE)]
				>> '(' >> expr[statement.statement = push_back(statement.statement, arg1)] >> ')'
				>> statement[statement.statement = add_statement(statement.statement, 0, arg1)]
				>> !("else"
					>> statement[statement.statement = add_statement(statement.statement, 1, arg1)])

				| boost::spirit::str_p("for")[statement.statement = make_statement(FOR_STATE)] >> '('
				>> !(assign[statement.statement = add_statement(statement.statement, 0, arg1)] ) >> ';'
				>> expr[statement.statement = add_statement(statement.statement, 1, arg1)] >> ';'
				>> !(assign[statement.statement = add_statement(statement.statement, 2, arg1)] || func_node[statement.statement = add_statement(statement.statement, 2, arg1)]) >> ')'
				>> statement[statement.statement = push_back(statement.statement, arg1)]


				| (boost::spirit::str_p("while")[statement.statement = make_statement(WHILE_STATE)] >> '(')
				>> (expr[statement.statement = push_back(statement.statement, arg1)] >> ')')
				>> statement[statement.statement = push_back(statement.statement, arg1)]
				| boost::spirit::str_p("switch") >> '('
				>> (expr[statement.statement = make_statement1(SWITCH_STATE, arg1)] >> ')')
				>> '{'
				>> *statement[statement.statement = push_back(statement.statement, arg1)]
				>> '}'
				| (callMember[statement.statement = make_statement1(CALL_STATE, arg1)] >> ';')
				| function[statement.statement = make_statement1(NOP_STATE, pushCompiler_sub(arg1, self.compiler))]
				| (func_node[statement.statement = make_statement1(CALL_STATE, arg1)] >> ';')
				| block[statement.statement = make_statement1(BLOCK_STATE, arg1)]
				;


			nameSpace = boost::spirit::str_p("namespace") >> identifier[regist(make_namespace(arg1), self.compiler)] >> "{"
				>> *(define_class[pushCompiler(arg1,self.compiler)]
					|Enum
					|function[pushCompiler(arg1, self.compiler)]
					| decl_func[analyze(arg1, self.compiler)]
					| decl_value[analyze(arg1, self.compiler)]
					| nameSpace[popNameSpace(self.compiler)]) >> "}";

			// 入力された構文
			input = *(define_class[pushCompiler(arg1, self.compiler)]
				|Enum
				|function[pushCompiler(arg1, self.compiler)]
				| decl_func[analyze(arg1, self.compiler)]
				| decl_value[analyze(arg1, self.compiler)]
				| nameSpace[popNameSpace(self.compiler)]
				| syntax_error_p
				);
		}

		boost::spirit::rule<ScannerT> const& start() const
		{
			return input;
		}
	};
};
// スキップ
struct skip_parser : public boost::spirit::grammar<skip_parser> {

	template <typename ScannerT>
	struct definition {

		boost::spirit::rule<ScannerT> skip_p;

		definition(skip_parser const& self)
		{
			skip_p = boost::spirit::space_p
				| boost::spirit::comment_p("//")			
				| boost::spirit::comment_p("/*", "*/")		
				;
		}
		boost::spirit::rule<ScannerT> const& start() const
		{
			return skip_p;
		}
	};
};

// EOFまで進んだのかの判定
template <typename IteratorT, typename DerivedT>
bool skip_all(IteratorT arg_first, IteratorT arg_last, boost::spirit::parser<DerivedT> const& arg_perser)
{
	using namespace boost::spirit;
	for (;;) {
		parse_info<IteratorT> info = parse(arg_first, arg_last, arg_perser);
		if (info.full)
			return true;
		if (!info.hit)
			return false;
		arg_first = info.stop;
	}
}
}
bool ButiScript::ScriptParse(const std::string& arg_filePath, Compiler* arg_compiler)
{
	std::ifstream fin(arg_filePath.c_str());

	if (!fin) {
		arg_compiler->error("ファイル" + arg_filePath + "がオープンできません。");
		return false;
	}

	// ファイルを読み込む
	std:: istreambuf_iterator<char> fbegin(fin);
	std::istreambuf_iterator<char> fend;
	std::string input(fbegin, fend);

	// ポジションイテレータ
	iterator_t begin(input.begin(), input.end(), arg_filePath);
	iterator_t end;
	begin.set_tabchars(4);

	funcAnalyze_grammer	gr(arg_compiler);
	typeRegist_grammer	gr_typeRegist(arg_compiler);
	skip_parser skip_p;
	boost::spirit::parse_info<iterator_t> info = parse(begin, end, gr_typeRegist, skip_p);
	if (!(info.hit && (info.full || skip_all(info.stop, end, skip_p)))) {
		arg_compiler->error("構文解析失敗");

		arg_compiler->ClearGlobalNameSpace();
		return false;
	}
	arg_compiler->ClearNameSpace();
	info = parse(begin, end, gr, skip_p);
	arg_compiler->OpHalt();
	arg_compiler->LambdaCountReset();
	arg_compiler->Analyze();
	arg_compiler->ClearNameSpace();
	arg_compiler->LambdaCountReset();
	arg_compiler->ClearGlobalNameSpace();
	if (!(info.hit && (info.full || skip_all(info.stop, end, skip_p))) ){
		arg_compiler->error("構文解析失敗");
		return false;
	}

	return true;
}
