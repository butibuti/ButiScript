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
using namespace std;
using namespace boost::spirit;
namespace ButiScript{
// ���͗p�̃C�e���[�^���`
typedef position_iterator<string::const_iterator>	iterator_t;

// �G���[�����p�[�T�[��`
struct error_parser {
	typedef nil_t result_t;		// �p�[�T�[�̌��ʌ^�inil_t�j

	error_parser(char const* msg)
		: msg_(msg)
	{
	}

	template <typename ScannerT>
	int operator()(ScannerT const& scan, result_t& result) const
	{
		// �I���܂ŗ�����-1��Ԃ�
		if (scan.at_end()) {
			return -1;
		}

		// ���s�܂ł��X�L�������A�����܂ł�\������B

		iterator_t b = scan.first;
		size_t length = (*(anychar_p - '\n')).parse(scan).length();
		file_position fpos = scan.first.get_position();
		cout << fpos.file << ": " << fpos.line << "." << fpos.column << ": "
			<< msg_ << " : " << string(b, scan.first) << endl;

//		return (int)length + 1;
		return -1;
	}

private:
	const char* msg_;
};

// �G���[�����p�[�T�[
typedef functor_parser<error_parser> error_p;

// ���@�G���[�����p�[�T�[
error_p syntax_error_p = error_parser("���@�G���[");

//�����o�ϐ��̏��
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

// �P�����Z�q�m�[�h�𐶐�����
struct unary_node_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef Node_t type; };

	template <typename Ty1, typename Ty2>
	Node_t operator()(Ty1 Op, const Ty2& left) const
	{
		return Node::make_node(Op, left);
	}
};

// �񍀉��Z�q�m�[�h�𐶐�����
struct binary_node_impl {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { typedef Node_t type; };

	template <typename Ty1, typename Ty2, typename Ty3>
	Node_t operator()(Ty1 Op, const Ty2& left, const Ty3& right) const
	{
		return Node::make_node(Op, left, right);
	}
};

// �񍀉��Z�q�m�[�h�𐶐�����(�R���p�C���g�p)
struct binary_node_impl_useDriver {
	template <typename Ty1, typename Ty2, typename Ty3, typename Ty4>
	struct result { typedef Node_t type; };

	template <typename Ty1, typename Ty2, typename Ty3, typename Ty4>
	Node_t operator()(Ty1 Op, const Ty2& left, const Ty3& right,const Ty4 driver) const
	{
		return Node::make_node(Op, left, right,driver);
	}
};

// �l��ǉ�
struct push_back_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef Ty1 type; };

	template <typename Ty1, typename Ty2>
	Ty1 operator()(Ty1& list, const Ty2& node) const
	{
		list->Add(node);
		return list;
	}
};
// �l��ǉ�(std::vector)
struct vector_push_back_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef void type; };

	template <typename Ty1, typename Ty2>
	void operator()(Ty1& vector, const Ty2& object) const
	{
		vector.push_back(object);
	}
};

// �m�[�h���X�g�𐶐�����
struct make_argument_impl {
	template <typename Ty>
	struct result { typedef NodeList_t type; };

	template <typename Ty>
	NodeList_t operator()(const Ty& node) const
	{
		return NodeList_t(new NodeList(node));
	}
};

// �X�e�[�g�����g�̐���
struct make_statement_impl {
	template <typename Ty>
	struct result { typedef Statement_t type; };

	template <typename Ty>
	Statement_t operator()(Ty state) const
	{
		return Statement::make_statement(state);
	}
};

// �X�e�[�g�����g�̐���
struct make_statement1_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef Statement_t type; };

	template <typename Ty1, typename Ty2>
	Statement_t operator()(Ty1 state, const Ty2& node) const
	{
		return Statement::make_statement(state, node);
	}
};

// �X�e�[�g�����g�ɃC���f�b�N�X�t���Œl��ǉ�
struct add_statement_impl {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { typedef Statement_t type; };

	template <typename Ty1, typename Ty2, typename Ty3>
	Statement_t operator()(Ty1& statement, Ty2 index, const Ty3& node) const
	{
		statement->Add(index, node);
		return statement;
	}
};

// �錾�̐���
struct make_decl_impl {
	template <typename Ty>
	struct result { typedef Declaration_t type; };

	template <typename Ty>
	Declaration_t operator()(Ty type) const
	{
		return Declaration_t(new Declaration(type));
	}
};

// �錾�̐���
struct make_decl1_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef Declaration_t type; };

	template <typename Ty1, typename Ty2>
	Declaration_t operator()(Ty1 type, const Ty2& node) const
	{
		return Declaration_t(new Declaration(type, node));
	}
};

// �����錾�̌^���Q�Ƃɂ���
struct arg_ref_impl {
	template <typename Ty1>
	struct result { typedef ArgDefine type; };

	template <typename Ty1>
	ArgDefine operator()(Ty1& decl) const
	{
		decl.set_ref();
		return decl;
	}
};

// �����̖��O��ݒ�
struct arg_name_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef ArgDefine type; };

	template <typename Ty1, typename Ty2>
	ArgDefine operator()(Ty1& decl, const Ty2& name) const
	{
		decl.set_name(name);
		return decl;
	}
};

//�֐��A�ϐ��Ăяo�����ɖ��O��Ԃ�ێ�����
struct call_namespace_impl {
	template < typename Ty2>
	struct result { typedef std::string type; };

	template <typename Ty2>
	std::string operator()(const Ty2& name) const
	{
		return name+"::";
	}
};

// �֐��̐���
struct make_function_impl {
	template < typename Ty2>
	struct result { typedef Function_t type; };

	template <typename Ty2>
	Function_t operator()(const Ty2& name) const
	{
		return Function_t(new Function(name));
	}
};
// �֐��̐���
struct make_functionWithAccess_impl {
	template < typename Ty1, typename Ty2>
	struct result { typedef Function_t type; };

	template < typename Ty1, typename Ty2>
	Function_t operator()(Ty1 accessNameFunction, const Ty2& name) const
	{
		return Function_t(new Function(accessNameFunction,name));
	}
};

//�N���X�̐���
struct make_class_impl {
	template < typename Ty2>
	struct result { typedef Class_t type; };

	template <typename Ty2>
	Class_t operator()(const Ty2& name) const
	{
		return Class_t(new Class(name));
	}
};

//�����o�ϐ��̐���
struct make_classMember_impl {
	template < typename Ty1, typename Ty2>
	struct result { typedef void type; };

	template < typename Ty1, typename Ty2>
	void operator()(Ty1 shp_class, const Ty2& type_and_name) const
	{
		shp_class->SetValue(type_and_name.name, type_and_name.type, type_and_name.accessType);
	}
};

//std::pair�̐���
struct make_pair_impl {
	template < typename Ty1, typename Ty2>
	struct result { typedef std::pair<Ty1,Ty2> type; };

	template < typename Ty1, typename Ty2>
	std::pair<Ty1, Ty2> operator()(const Ty1 index, const Ty2& name) const
	{
		return {index,name};
	}
};

//MemberValue�̐���
struct make_memberValue_impl {
	template < typename Ty1, typename Ty2,typename Ty3>
	struct result { typedef MemberValue type; };

	template < typename Ty1, typename Ty2, typename Ty3>
	MemberValue operator()(const Ty1 index, const Ty2& name,const Ty3 accessType) const
	{
		return MemberValue(index, name, accessType);
	}
};

//�񋓌^�̐���
struct make_enum_impl {
	template < typename Ty2>
	struct result { typedef Enum_t type; };

	template <typename Ty2>
	Enum_t operator()(const Ty2& name) const
	{
		return Enum_t(new Enum(name));
	}
};
//�񋓌^�̒ǉ�
struct add_enum_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef void type; };

	template <typename Ty1, typename Ty2>
	void operator()(Ty1 enum_t, const Ty2& name) const
	{
		enum_t->SetIdentifer(name);
	}
};

// ���O��Ԃ̐���
struct make_namespace_impl {
	template < typename Ty2>
	struct result { typedef NameSpace_t type; };

	template <typename Ty2>
	NameSpace_t operator()(const Ty2& name) const
	{
		return NameSpace_t(new NameSpace(name));
	}
};

// �֐��̕Ԃ�l�ݒ�
struct setFunctionType_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef Function_t type; };

	template <typename Ty1, typename Ty2>
	Function_t operator()(Ty1& decl, Ty2 type) const
	{
		decl->set_type(type);
		return decl;
	}
};

// �֐��A���O��ԁA�N���X�o�^
struct regist_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef void type; };

	template <typename Ty1, typename Ty2>
	void operator()(const Ty1& decl, Ty2 driver) const
	{
		decl->Regist(driver);
	}
};

//�A�N�Z�X�w��q����
struct accessModifier_impl {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { typedef std::string type; };

	template <typename Ty1, typename Ty2, typename Ty3>
	std::string operator()(const Ty1& modifierStr,Ty2& ref_accesModifier,const Ty3& ret) const
	{
		ref_accesModifier= StringToAccessModifier(modifierStr);
		return ret;
	}
};

// ���\�b�h�o�^
struct registMethod_impl {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { typedef void type; };

	template <typename Ty1, typename Ty2, typename Ty3>
	void operator()(const Ty1& decl, Ty2 type, Ty3 driver) const
	{
		type->RegistMethod(decl, driver);
	}
};
// ���\�b�h���
struct analyzeMethod_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef void type; };

	template <typename Ty1, typename Ty2>
	void operator()( Ty1 type, Ty2 driver) const
	{
		type->AnalyzeMethod( driver);
	}
};

//���O��Ԃ���̗��E
struct pop_nameSpace_impl {
	template < typename Ty2>
	struct result { typedef void type; };

	template < typename Ty2>
	void operator()(Ty2 driver) const
	{
		driver->PopNameSpace();
	}
};

//�^�̓���
struct	specificType_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef int type; };

	template <typename Ty1, typename Ty2>
	int operator()(const Ty1& key, Ty2 driver) const
	{
		return  driver->GetTypeIndex(key);
	}
};


// �ŏI�o�^
struct analyze_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef void type; };

	template <typename Ty1, typename Ty2>
	void operator()(const Ty1& decl, Ty2 driver) const
	{
		decl->Analyze(driver);
	}
};

// phoenix���g�p���閳���֐��p�̊֐�
phoenix::function<binary_node_impl> const binary_node = binary_node_impl();
phoenix::function<binary_node_impl_useDriver> const binary_node_comp = binary_node_impl_useDriver();
phoenix::function<unary_node_impl> const unary_node = unary_node_impl();
phoenix::function<push_back_impl> const push_back = push_back_impl();
phoenix::function<vector_push_back_impl> const vec_push_back = vector_push_back_impl();
phoenix::function<make_argument_impl> const make_argument = make_argument_impl();
phoenix::function<make_statement_impl> const make_statement = make_statement_impl();
phoenix::function<make_statement1_impl> const make_statement1 = make_statement1_impl();
phoenix::function<add_statement_impl> const add_statement = add_statement_impl();
phoenix::function<make_decl_impl> const make_decl = make_decl_impl();
phoenix::function<make_decl1_impl> const make_decl1 = make_decl1_impl();
phoenix::function<arg_ref_impl> const arg_ref = arg_ref_impl();
phoenix::function<arg_name_impl> const arg_name = arg_name_impl();
phoenix::function<make_function_impl> const make_function = make_function_impl();
phoenix::function<make_functionWithAccess_impl> const make_functionWithAccess = make_functionWithAccess_impl();
phoenix::function<make_class_impl> const make_class = make_class_impl();
phoenix::function<make_classMember_impl> const make_classMember = make_classMember_impl();
phoenix::function<add_enum_impl> const add_enum = add_enum_impl();
phoenix::function<make_enum_impl> const make_enum = make_enum_impl();
phoenix::function<make_namespace_impl> const make_namespace = make_namespace_impl();
phoenix::function<make_pair_impl> const make_pair = make_pair_impl();
phoenix::function<make_memberValue_impl> const make_memberValue = make_memberValue_impl();
phoenix::function<setFunctionType_impl> const set_functionType = setFunctionType_impl();
phoenix::function<specificType_impl> const specificType = specificType_impl();
phoenix::function<accessModifier_impl> const specificAccessModifier = accessModifier_impl();
phoenix::function<analyze_impl> const analyze = analyze_impl();
phoenix::function<regist_impl> const regist = regist_impl();
phoenix::function<registMethod_impl> const registMethod = registMethod_impl();
phoenix::function<analyzeMethod_impl> const analyzeMethod = analyzeMethod_impl();
phoenix::function<pop_nameSpace_impl> const popNameSpace = pop_nameSpace_impl();
phoenix::function<call_namespace_impl> const functionCall_namespace = call_namespace_impl();

real_parser<double, ureal_parser_policies<double> > const ureal_parser = real_parser<double, ureal_parser_policies<double> >();

namespace ButiClosure {

	// ������̃N���[�W��
	struct string_val : closure<string_val, std::string> {
		member1 str;
	};
	// �����̃N���[�W��
	struct number_val : closure<number_val, unsigned int> {
		member1 number;
	};
	// ���������N���[�W��
	struct float_val : closure<float_val, float> {
		member1 number;
	};

	// �m�[�h�̃N���[�W��
	struct node_val : closure<node_val, Node_t, int, std::string> {
		member1 node;
		member2 Op;
		member3 name;
	};
	// �����o�Ăяo���̃N���[�W��
	struct callmember_val : closure<callmember_val, Node_t, int, std::string> {
		member1 memberNode;
		member1 valueNode;
		member2 Op;
		member3 name;
	};

	// ���O��Ԃ̃N���[�W��
	struct namespace_val : closure<namespace_val,  std::string> {
		member1 name;
	};
	// �m�[�h�̃N���[�W��
	struct nodelist_val : closure<nodelist_val, NodeList_t, int> {
		member1 node;
		member2 Op;
	};
	// ���̃N���[�W��
	struct state_val : closure<state_val, Statement_t> {
		member1 statement;
	};
	// �^�̃N���[�W��
	struct type_val : closure<type_val, int> {
		member1 type;
	};

	// �ϐ���`�̃N���[�W��
	struct decl_val : closure<decl_val, Declaration_t, int,std::vector< Node_t>,AccessModifier> {
		member1 node;
		member2 type;
		member3 value;
		member4 access;
	};

	// �֐���`�̃N���[�W��
	struct func_val : closure<func_val, Function_t, int, std::string> {
		member1 node;
		member2 type;
		member3 name;
	};
	// �N���X�����o��`�̃N���[�W��
	struct classMember_val : closure<classMember_val, MemberValue,std::string, AccessModifier> {
		member1 memberValue;
		member2 name;
		member3 accessModifier;
	};
	// �N���X��`�̃N���[�W��
	struct class_val : closure<class_val, Class_t, std::string> {
		member1 Class;
		member2 name;
	};

	struct enum_val : closure<enum_val, Enum_t> {
		member1 enum_t;
	};

	// ������`�̃N���[�W��
	struct argdef_val : closure<argdef_val, ArgDefine, std::string> {
		member1 node;
		member2 name;
	};

	// ���u���b�N�̃N���[�W��
	struct block_val : closure<block_val, Block_t> {
		member1 node;
	};
}

//�^�o�^
struct typeRegist_grammer : public grammar<typeRegist_grammer> {
	typeRegist_grammer(Compiler* driver)
		:driver_(driver)
	{
	}
	Compiler* driver_;
	template <typename ScannerT>
	struct definition {
		rule<ScannerT, ButiClosure::string_val::context_t>	identifier;
		rule<ScannerT, ButiClosure::namespace_val::context_t>	nameSpace;
		rule<ScannerT, ButiClosure::namespace_val::context_t>	nameSpace_call;
		rule<ScannerT, ButiClosure::enum_val::context_t>		Enum;
		rule<ScannerT, ButiClosure::class_val::context_t>		define_class;
		rule<ScannerT>	decl_value,decl_classMember,Value,function, argdef, type,string_node, number, floatNumber, func_node, prime, unary, mul_expr, add_expr, shift_expr, bit_expr, equ_expr,
			and_expr, expr, assign, argument, statement, arg, decl_func, callMember, block, input, ident;

		symbols<> keywords;
		symbols<> mul_op, add_op, shift_op, bit_op, equ_op, assign_op;
		dynamic_distinct_parser<ScannerT> keyword_p;

		definition(typeRegist_grammer const& self)
			:keyword_p(alnum_p | '_')
		{
			using phoenix::arg1;
			using phoenix::arg2;
			using phoenix::var;
			using phoenix::new_;
			using phoenix::construct_;

			keywords = "if", "for", "while", "switch", "case", "default", "break", "return", "namespace";


			// ���ʎq
			ident = lexeme_d[
				((alpha_p | '_') >> *(alnum_p | '_')) - (keywords >> anychar_p - (alnum_p | '_'))
			];
			// ���ʎq�i�N���[�W���ɓo�^�j
			identifier = ident[identifier.str = construct_<string>(arg1, arg2)];


			Enum = "enum" >> identifier[Enum.enum_t = make_enum(arg1)] >> "{" >>
				!identifier[add_enum(Enum.enum_t, arg1)]
				>> *(',' >> identifier[add_enum(Enum.enum_t, arg1)]) >>
				"}";

			//����
			number = uint_p;

			//��������
			floatNumber = strict_real_p;

			// ������
			string_node = lexeme_d[
				confix_p(ch_p('"'), *c_escape_ch_p, '"')
			];


			//���O��Ԃ���̌Ăяo��
			nameSpace_call = identifier[nameSpace_call.name = arg1] >> "::";

			// �ϐ�
			Value = (*(nameSpace_call)) >>
				identifier;

			// �֐��̈���
			argument = expr
				>> *(',' >> expr);

			// �֐��Ăяo��
			func_node = *(identifier >> "::") >> identifier >>
				'(' >> !argument >> ')';

			//�����o�ϐ��Ăяo��
			callMember = Value >> "."
				>> identifier
				>> !('(' >> !argument >> ')')
				>> *("." >>
					identifier >>
					!('(' >> !argument >> ')')
					);

			// �v�Z��prime�m�[�h
			prime = callMember
				| func_node
				| Value
				| floatNumber
				| number
				| string_node
				| '(' >> expr >> ')'
				;

			// �P�����Z�q
			unary = prime
				| '-' >> prime;

			// �񍀉��Z�q�i*, /, %�j
			mul_op.add("*", OP_MUL)("/", OP_DIV)("%", OP_MOD);
			mul_expr = unary
				>> *(mul_op
					>> unary);

			// �񍀉��Z�q�i+, -�j
			add_op.add("+", OP_ADD)("-", OP_SUB);
			add_expr = mul_expr
				>> *(add_op
					>> mul_expr);

			// �񍀉��Z�q�i<<, >>�j
			shift_op.add("<<", OP_LSHIFT)(">>", OP_RSHIFT);
			shift_expr = add_expr
				>> *(shift_op
					>> add_expr);

			// �񍀉��Z�q�i&, |�j
			bit_op.add("&", OP_AND)("|", OP_OR);
			bit_expr = shift_expr
				>> *(bit_op
					>> shift_expr);

			// �񍀉��Z�q�i��r�j
			equ_op.add("==", OP_EQ)("!=", OP_NE)(">=", OP_GE)(">", OP_GT)("<=", OP_LE)("<", OP_LT);
			equ_expr = bit_expr
				>> !(equ_op
					>> bit_expr);

			// �񍀉��Z�q�i&&�j
			and_expr = equ_expr
				>> *("&&" >> equ_expr);

			// �񍀉��Z�q�i||�j
			expr = and_expr
				>> *("||" >> and_expr);

			// ���
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
				>> expr;

			// �ϐ��錾
			decl_value = !(str_p("private") | str_p("public")) >> "var" >> Value % ',' >> ':' >> type >> ';';

			// �^��
			type = identifier >> !ch_p('&');

			// �֐��錾�̈���
			arg = identifier >> ':'
				>> type
				>> !str_p("[]");

			// �֐��錾
			decl_func = identifier
				>> '(' >> !(arg % ',') >> ')' >> ":" >> type >> ';';

			// �֐���`�̈���
			argdef = identifier >> ':'
				>> type
				>> !str_p("[]");

			// �֐���`
			function = identifier >> !identifier
				>> '(' >> !(argdef% ',') >> ')' >>
				':' >> type
				>> block;

			// ���u���b�N
			block = ch_p('{')
				>> *(statement
					| decl_value)
				>> '}';
			//�N���X�̃����o�[��`
			decl_classMember = identifier >> !identifier
				>> ':' >> type >> ';';

			//�N���X��`
			define_class = "class" >> identifier[define_class.Class = make_class(arg1)] >> "{" >>
				*(decl_classMember
					| function)
				>> "}";
			// ��
			statement = ch_p(';')
				| assign >> ';'
				| str_p("case") >> expr >> ':'
				| str_p("default") >> ':'
				| str_p("break") >> ';'
				| str_p("return")
				>> !expr >> ';'
				| str_p("if")
				>> '(' >> expr >> ')'
				>> statement
				>> !("else"
					>> statement)

				| str_p("for") >> '('
				>> !(assign) >> ';'
				>> expr >> ';'
				>> !(assign || func_node || callMember) >> ')'
				>> statement

				| str_p("while") >> '('
				>> expr >> ')'
				>> statement
				| str_p("switch") >> '('
				>> expr >> ')'
				>> '{'
				>> *statement
				>> '}'
				| func_node >> ';'
				| callMember >> ';'
				| block
				;

			nameSpace = str_p("namespace") >> identifier[regist(make_namespace(arg1), self.driver_)] >> "{"
				>> *(define_class[regist(arg1, self.driver_)]
					| Enum[analyze(arg1, self.driver_)]
					| function
					| decl_func
					| decl_value
					| nameSpace[popNameSpace(self.driver_)]) >> "}";

			// ���͂��ꂽ�\��
			input = *(define_class[regist(arg1, self.driver_)]
				| Enum[analyze(arg1, self.driver_)]
				| function
				| decl_func
				| decl_value
				| nameSpace[popNameSpace(self.driver_)]
				| syntax_error_p
				);
		}

		rule<ScannerT> const& start() const
		{
			return input;
		}
	};
};


// �֐��A�O���[�o���ϐ��o�^�A�N���X���
struct registFunc_classAnalyze_grammer : public grammar<registFunc_classAnalyze_grammer> {
	registFunc_classAnalyze_grammer(Compiler* driver)
		:driver_(driver)
	{
	}
	Compiler* driver_;	
	template <typename ScannerT>
	struct definition {
		rule<ScannerT, ButiClosure::string_val::context_t>	identifier;
		rule<ScannerT, ButiClosure::type_val::context_t>		type;
		rule<ScannerT, ButiClosure::func_val::context_t>		function;
		rule<ScannerT, ButiClosure::argdef_val::context_t>	argdef;
		rule<ScannerT, ButiClosure::namespace_val ::context_t>	nameSpace;
		rule<ScannerT, ButiClosure::namespace_val::context_t>	nameSpace_call;
		rule<ScannerT, ButiClosure::node_val::context_t>		Value;
		rule<ScannerT, ButiClosure::enum_val::context_t>		Enum;
		rule<ScannerT, ButiClosure::decl_val::context_t>		decl_value;
		rule<ScannerT, ButiClosure::class_val::context_t>		define_class;
		rule<ScannerT, ButiClosure::classMember_val::context_t>		decl_classMember;
		rule<ScannerT>	string_node,number,floatNumber,	func_node,prime,unary,mul_expr,add_expr,shift_expr,bit_expr,equ_expr,	
			and_expr,expr,assign,argument,statement,arg,decl_func,callMember ,block,input,ident;

		symbols<> keywords;
		symbols<> mul_op, add_op, shift_op, bit_op, equ_op, assign_op;
		dynamic_distinct_parser<ScannerT> keyword_p;

		definition(registFunc_classAnalyze_grammer const& self)
			:keyword_p(alnum_p | '_')
		{
			using phoenix::arg1;
			using phoenix::arg2;
			using phoenix::var;
			using phoenix::new_;
			using phoenix::construct_;

			keywords = "if", "for", "while", "switch", "case", "default", "break", "return","namespace";


			// ���ʎq
			ident = lexeme_d[
				((alpha_p | '_') >> *(alnum_p | '_')) - (keywords >> anychar_p - (alnum_p | '_'))
			];
			// ���ʎq�i�N���[�W���ɓo�^�j
			identifier = ident[identifier.str = construct_<string>(arg1, arg2)];


			Enum = "enum">>identifier >> "{" >>
				!identifier
				>> *(',' >> identifier)>>
				"}";

			//����
			number = uint_p;

			//��������
			floatNumber = strict_real_p;

			// ������
			string_node = lexeme_d[
				confix_p(ch_p('"'), *c_escape_ch_p, '"')
			];


			//���O��Ԃ���̌Ăяo��
			nameSpace_call = identifier[nameSpace_call.name = arg1] >> "::";

			// �ϐ�
			Value = (*(nameSpace_call[Value.name += functionCall_namespace(arg1)])) >>
				identifier[Value.node = unary_node(OP_IDENTIFIER,  arg1)];

			// �֐��̈���
			argument = expr
				>> *(',' >> expr);

			// �֐��Ăяo��
			func_node = *(identifier >> "::") >> identifier>>
				'(' >> !argument >> ')';

			//�����o�ϐ��Ăяo��
			callMember = Value >> "."
				>> identifier
				>> !('(' >> !argument >> ')')
				>> *("." >>
					identifier >>
					!('(' >> !argument >> ')')
					);

			// �v�Z��prime�m�[�h
			prime =callMember
				|func_node
				| Value
				| floatNumber
				| number
				| string_node
				| '(' >> expr >> ')'
				;

			// �P�����Z�q
			unary = prime
				| '-' >> prime;

			// �񍀉��Z�q�i*, /, %�j
			mul_op.add("*", OP_MUL)("/", OP_DIV)("%", OP_MOD);
			mul_expr = unary
				>> *(mul_op
					>> unary);

			// �񍀉��Z�q�i+, -�j
			add_op.add("+", OP_ADD)("-", OP_SUB);
			add_expr = mul_expr
				>> *(add_op
					>> mul_expr);

			// �񍀉��Z�q�i<<, >>�j
			shift_op.add("<<", OP_LSHIFT)(">>", OP_RSHIFT);
			shift_expr = add_expr
				>> *(shift_op
					>> add_expr);

			// �񍀉��Z�q�i&, |�j
			bit_op.add("&", OP_AND)("|", OP_OR);
			bit_expr = shift_expr
				>> *(bit_op
					>> shift_expr);

			// �񍀉��Z�q�i��r�j
			equ_op.add("==", OP_EQ)("!=", OP_NE)(">=", OP_GE)(">", OP_GT)("<=", OP_LE)("<", OP_LT);
			equ_expr = bit_expr
				>> !(equ_op
					>> bit_expr);

			// �񍀉��Z�q�i&&�j
			and_expr = equ_expr
				>> *("&&" >> equ_expr);

			// �񍀉��Z�q�i||�j
			expr = and_expr
				>> *("||" >> and_expr);

			// ���
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
			assign = (callMember|Value)
				>> assign_op
				>> expr;

			// �ϐ��錾
			decl_value = !(str_p("private")[decl_value.access=AccessModifier::Private] | str_p("public")[decl_value.access = AccessModifier::Public]) 
				>> "var" >> Value[vec_push_back (decl_value.value ,arg1)] % ',' >> ':' >> type[decl_value.node = push_back(make_decl1(arg1, decl_value.access), decl_value.value)] >> ';';

			// �^��
			type = identifier[type.type = specificType(arg1,self.driver_)] >> !ch_p('&')[type.type |= TYPE_REF];

			// �֐��錾�̈���
			arg = identifier >> ':'
				>> type
				>> !str_p("[]");

			// �֐��錾
			decl_func = identifier
				>> '(' >> !(arg % ',') >> ')'>>":" >>type >> ';';

			// �֐���`�̈���
			argdef = identifier[argdef.name = arg1] >> ':'
				>> type[argdef.node = construct_<ArgDefine>(arg1, argdef.name)]
				>> !str_p("[]")[argdef.node = arg_ref(argdef.node)];

			// �֐���`
			function = identifier[function.node = make_function(arg1)]>>!identifier[function.node = make_functionWithAccess(function.node,arg1)]
				>> '(' >> !(argdef[function.node = push_back(function.node, arg1)] % ',') >> ')' >>
				':' >> type[function.node = set_functionType(function.node, arg1)]
				>> block;

			// ���u���b�N
			block = ch_p('{')
				>> *(statement
					| decl_value)
				>> '}';
			//�N���X�̃����o�[��`
			decl_classMember = identifier[decl_classMember.name = arg1]>> !identifier[decl_classMember.name= specificAccessModifier(decl_classMember.name,decl_classMember.accessModifier, arg1)]
				>> ':' >> type[decl_classMember.memberValue= make_memberValue( arg1,decl_classMember.name,decl_classMember.accessModifier)] >> ';';

			//�N���X��`
			define_class = "class" >> identifier[define_class.Class= make_class(arg1)] >> "{" >>
				*(decl_classMember[make_classMember(define_class.Class,arg1)]
					| function[registMethod(arg1,define_class.Class, self.driver_)])
				>> "}";
			// ��
			statement = ch_p(';')
				| assign >> ';'
				| str_p("case") >> expr >> ':'
				| str_p("default")>> ':'
				| str_p("break") >> ';'
				| str_p("return")
				>> !expr>> ';'
				| str_p("if")
				>> '(' >> expr>> ')'
				>> statement
				>> !("else"
					>> statement)

				| str_p("for")>> '('
				>> !(assign) >> ';'
				>> expr >> ';'
				>> !(assign|| func_node|| callMember) >> ')'
				>> statement

				| str_p("while") >> '('
				>> expr >> ')'
				>> statement
				| str_p("switch") >> '('
				>> expr >> ')'
				>> '{'
				>> *statement
				>> '}'
				| func_node >> ';'
				| callMember >>';'
				| block
				;

			nameSpace = str_p("namespace") >> identifier[regist(make_namespace(arg1), self.driver_)] >> "{"
				>> *(define_class[analyze(arg1,self.driver_)]
					|Enum
					|function[regist(arg1, self.driver_)]
					| decl_func
					| decl_value[analyze(arg1, self.driver_)]
					| nameSpace[popNameSpace(self.driver_)]) >> "}";

			// ���͂��ꂽ�\��
			input = *(define_class[analyze(arg1, self.driver_)]
				|Enum
				|function[regist(arg1, self.driver_)]
				| decl_func
				| decl_value[analyze(arg1, self.driver_)]
				| nameSpace[popNameSpace(self.driver_)]
				| syntax_error_p
				);
		}

		rule<ScannerT> const& start() const
		{
			return input;
		}
	};
};

// �֐����
struct funcAnalyze_grammer : public grammar<funcAnalyze_grammer> {
	funcAnalyze_grammer(Compiler* driver)
		:driver_(driver)
	{
	}
	Compiler* driver_;
	template <typename ScannerT>
	struct definition {
		rule<ScannerT, ButiClosure::string_val::context_t>	identifier;
		rule<ScannerT, ButiClosure::string_val::context_t>	string_node;
		rule<ScannerT, ButiClosure::number_val::context_t>	number;
		rule<ScannerT, ButiClosure::float_val::context_t>	floatNumber;
		rule<ScannerT, ButiClosure::type_val::context_t>		type;
		rule<ScannerT, ButiClosure::node_val::context_t>		func_node;
		rule<ScannerT, ButiClosure::node_val::context_t>		Value;
		rule<ScannerT, ButiClosure::callmember_val::context_t> callMember;
		rule<ScannerT, ButiClosure::node_val::context_t>		prime;
		rule<ScannerT, ButiClosure::node_val::context_t>		unary;
		rule<ScannerT, ButiClosure::node_val::context_t>		mul_expr;
		rule<ScannerT, ButiClosure::node_val::context_t>		add_expr;
		rule<ScannerT, ButiClosure::node_val::context_t>		shift_expr;
		rule<ScannerT, ButiClosure::node_val::context_t>		bit_expr;
		rule<ScannerT, ButiClosure::node_val::context_t>		equ_expr;
		rule<ScannerT, ButiClosure::node_val::context_t>		and_expr;
		rule<ScannerT, ButiClosure::node_val::context_t>		expr;
		rule<ScannerT, ButiClosure::node_val::context_t>		assign;
		rule<ScannerT, ButiClosure::nodelist_val::context_t>	argument;
		rule<ScannerT, ButiClosure::state_val::context_t>	statement;
		rule<ScannerT, ButiClosure::func_val::context_t>		function;
		rule<ScannerT, ButiClosure::class_val::context_t>		define_class;
		rule<ScannerT, ButiClosure::type_val::context_t>		arg;
		rule<ScannerT, ButiClosure::decl_val::context_t>		decl_value;
		rule<ScannerT, ButiClosure::decl_val::context_t>		decl_func;
		rule<ScannerT, ButiClosure::argdef_val::context_t>	argdef;
		rule<ScannerT, ButiClosure::block_val::context_t>	block;
		rule<ScannerT, ButiClosure::namespace_val::context_t>	nameSpace;
		rule<ScannerT, ButiClosure::namespace_val::context_t>	nameSpace_call;
		rule<ScannerT>							input, Enum,decl_classMember;
		rule<ScannerT>							ident;

		symbols<> keywords;
		symbols<> mul_op, add_op, shift_op, bit_op, equ_op, assign_op;
		dynamic_distinct_parser<ScannerT> keyword_p;

		definition(funcAnalyze_grammer const& self)
			:keyword_p(alnum_p | '_')
		{
			using phoenix::arg1;
			using phoenix::arg2;
			using phoenix::var;
			using phoenix::new_;
			using phoenix::construct_;
			keywords = "if", "for", "while", "switch", "case", "default", "break", "return", "namespace";
			// ���ʎq
			ident = lexeme_d[
				((alpha_p | '_') >> *(alnum_p | '_')) - (keywords >> anychar_p - (alnum_p | '_'))
			];
			// ���ʎq�i�N���[�W���ɓo�^�j
			identifier = ident[identifier.str = construct_<string>(arg1, arg2)];
			

			Enum = "enum" >> identifier >> "{" >>
				identifier
				>> *(',' >> identifier) >>
				"}";

			//���O��Ԃ���̌Ăяo��
			nameSpace_call = identifier[nameSpace_call.name = arg1] >> "::";

			//����
			number = uint_p[number.number = arg1];

			//��������
			floatNumber = strict_real_p[floatNumber.number = arg1];

			// ������
			string_node = lexeme_d[
				confix_p(ch_p('"')[string_node.str = ""], *c_escape_ch_p[string_node.str += arg1], '"')
			];

			// �ϐ�
			Value = (*(nameSpace_call[Value.name += functionCall_namespace(arg1)])) >>
				identifier[Value.node = unary_node(OP_IDENTIFIER, Value.name+ arg1)]
				>> !('[' >> expr[Value.node = binary_node(OP_ARRAY, Value.node, arg1)] >> ']');

			// �֐��̈���
			argument = expr[argument.node = make_argument(arg1)]
				>> *(',' >> expr[argument.node = push_back(argument.node, arg1)]);

			// �֐��Ăяo��
			func_node = (*(nameSpace_call[func_node.name += functionCall_namespace(arg1) ]))>>
				identifier[func_node.node = unary_node(OP_FUNCTION, func_node.name+arg1)] >>
				'(' >> !argument[func_node.node = binary_node(OP_FUNCTION, func_node.node, arg1)] >> ')';


			//�����o�Ăяo��
			callMember =Value[ callMember.valueNode=arg1]>>"."
				>> identifier[callMember.memberNode = binary_node_comp(OP_MEMBER, callMember.valueNode,arg1, self.driver_)]
				>>!(ch_p('(' )[callMember.memberNode = unary_node(OP_METHOD, callMember.memberNode)]>> !argument[callMember.memberNode = binary_node(OP_METHOD, callMember.memberNode, arg1)] >> ')')
				>>* (".">> 
					identifier[callMember.memberNode = binary_node_comp(OP_MEMBER, callMember.memberNode, arg1, self.driver_)]  >>  
					!(ch_p('(')[callMember.memberNode = unary_node(OP_METHOD, callMember.memberNode)] >> !argument[callMember.memberNode = binary_node(OP_METHOD, callMember.memberNode, arg1)] >> ')')
					)
				
				;

			// �v�Z��prime�m�[�h
			prime = callMember[prime.node = arg1]
				|func_node[prime.node = arg1]
				| Value[prime.node = arg1]
				| floatNumber[prime.node = unary_node(OP_FLOAT, arg1)]
				| number[prime.node = unary_node(OP_INT, arg1)]
				| string_node[prime.node = unary_node(OP_STRING, arg1)]
				| '(' >> expr[prime.node = arg1] >> ')'
				;

			// �P�����Z�q
			unary = prime[unary.node = arg1]
				| '-' >> prime[unary.node = unary_node(OP_NEG, arg1)];

			// �񍀉��Z�q�i*, /, %�j
			mul_op.add("*", OP_MUL)("/", OP_DIV)("%", OP_MOD);
			mul_expr = unary[mul_expr.node = arg1]
				>> *(mul_op[mul_expr.Op = arg1]
					>> unary[mul_expr.node = binary_node(mul_expr.Op, mul_expr.node, arg1)]);

			// �񍀉��Z�q�i+, -�j
			add_op.add("+", OP_ADD)("-", OP_SUB);
			add_expr = mul_expr[add_expr.node = arg1]
				>> *(add_op[add_expr.Op = arg1]
					>> mul_expr[add_expr.node = binary_node(add_expr.Op, add_expr.node, arg1)]);

			// �񍀉��Z�q�i<<, >>�j
			shift_op.add("<<", OP_LSHIFT)(">>", OP_RSHIFT);
			shift_expr = add_expr[shift_expr.node = arg1]
				>> *(shift_op[shift_expr.Op = arg1]
					>> add_expr[shift_expr.node = binary_node(shift_expr.Op, shift_expr.node, arg1)]);

			// �񍀉��Z�q�i&, |�j
			bit_op.add("&", OP_AND)("|", OP_OR);
			bit_expr = shift_expr[bit_expr.node = arg1]
				>> *(bit_op[bit_expr.Op = arg1]
					>> shift_expr[bit_expr.node = binary_node(bit_expr.Op, bit_expr.node, arg1)]);

			// �񍀉��Z�q�i��r�j
			equ_op.add("==", OP_EQ)("!=", OP_NE)(">=", OP_GE)(">", OP_GT)("<=", OP_LE)("<", OP_LT);
			equ_expr = bit_expr[equ_expr.node = arg1]
				>> !(equ_op[equ_expr.Op = arg1]
					>> bit_expr[equ_expr.node = binary_node(equ_expr.Op, equ_expr.node, arg1)]);

			// �񍀉��Z�q�i&&�j
			and_expr = equ_expr[and_expr.node = arg1]
				>> *("&&" >> equ_expr[and_expr.node = binary_node(OP_LOGAND, and_expr.node, arg1)]);

			// �񍀉��Z�q�i||�j
			expr = and_expr[expr.node = arg1]
				>> *("||" >> and_expr[expr.node = binary_node(OP_LOGOR, expr.node, arg1)]);

			// ���
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

			// �ϐ��錾
			decl_value = !(str_p("private")[decl_value.access = AccessModifier::Private] | str_p("public")[decl_value.access = AccessModifier::Public])
				>> "var" >> Value[vec_push_back(decl_value.value, arg1)] % ',' >> ':' >> type[decl_value.node = push_back(make_decl1(arg1, decl_value.access), decl_value.value)] >> ';';


			// �^��
			type = identifier[type.type = specificType(arg1, self.driver_)] >> !ch_p('&')[type.type |= TYPE_REF];

			// �֐��錾�̈���
			arg = identifier >> ':'
				>> type[arg.type = arg1]
				>> !str_p("[]")[arg.type |= TYPE_REF];

			// �֐��錾
			decl_func =  identifier[decl_func.node = make_decl1(decl_func.type, arg1)]
				>> '(' >> !(arg[decl_func.node = push_back(decl_func.node, arg1)] % ',') >> ')'>>":">>type[decl_func.type = arg1] >> ';';

			// �֐���`�̈���
			argdef = identifier[argdef.name = arg1] >> ':'
				>> type[argdef.node = construct_<ArgDefine>(arg1, argdef.name)]
				>> !str_p("[]")[argdef.node = arg_ref(argdef.node)];

			// �֐���`
			function = identifier[function.node = make_function(arg1)]>>!(identifier[function.node = make_functionWithAccess(function.node, arg1)])
				>> '(' >> !(argdef[function.node = push_back(function.node, arg1)] % ',') >> ')' >>
				':' >> type[function.node = set_functionType(function.node, arg1)]
				>> block[function.node = push_back(function.node, arg1)];


			//�N���X�̃����o�[��`
			decl_classMember= identifier>> (!identifier) >> ':' >> type >> ';';

			//�N���X��`
			define_class = "class" >> identifier[define_class.Class = make_class(arg1)]>> "{" >>
				*(decl_classMember|  function[registMethod(arg1, define_class.Class, self.driver_)])
				>>"}";

			// ���u���b�N
			block = ch_p('{')[block.node = construct_<Block_t>(new_<Block>())]
				>> *(statement[block.node = push_back(block.node, arg1)]
					| decl_value[block.node = push_back(block.node, arg1)])
				>> '}';

			// ��
			statement = ch_p(';')[statement.statement = make_statement(NOP_STATE)]
				| assign[statement.statement = make_statement1(ASSIGN_STATE, arg1)] >> ';'
				| str_p("case") >> expr[statement.statement = make_statement1(CASE_STATE, arg1)] >> ':'
				| str_p("default")[statement.statement = make_statement(DEFAULT_STATE)] >> ':'
				| str_p("break")[statement.statement = make_statement(BREAK_STATE)] >> ';'
				| str_p("return")[statement.statement = make_statement(RETURN_STATE)]
				>> !expr[statement.statement = push_back(statement.statement, arg1)] >> ';'

				| str_p("if")[statement.statement = make_statement(IF_STATE)]
				>> '(' >> expr[statement.statement = push_back(statement.statement, arg1)] >> ')'
				>> statement[statement.statement = add_statement(statement.statement, 0, arg1)]
				>> !("else"
					>> statement[statement.statement = add_statement(statement.statement, 1, arg1)])

				| str_p("for")[statement.statement = make_statement(FOR_STATE)] >> '('
				>> !(assign[statement.statement = add_statement(statement.statement, 0, arg1)] ) >> ';'
				>> expr[statement.statement = add_statement(statement.statement, 1, arg1)] >> ';'
				>> !(assign[statement.statement = add_statement(statement.statement, 2, arg1)] || func_node[statement.statement = add_statement(statement.statement, 2, arg1)]) >> ')'
				>> statement[statement.statement = push_back(statement.statement, arg1)]


				| str_p("while")[statement.statement = make_statement(WHILE_STATE)] >> '('
				>> expr[statement.statement = push_back(statement.statement, arg1)] >> ')'
				>> statement[statement.statement = push_back(statement.statement, arg1)]
				| str_p("switch") >> '('
				>> expr[statement.statement = make_statement1(SWITCH_STATE, arg1)] >> ')'
				>> '{'
				>> *statement[statement.statement = push_back(statement.statement, arg1)]
				>> '}'
				| callMember[statement.statement = make_statement1(CALL_STATE, arg1)] >> ';'
				| func_node[statement.statement = make_statement1(CALL_STATE, arg1)] >> ';'
				| block[statement.statement = make_statement1(BLOCK_STATE, arg1)]
				;


			nameSpace = str_p("namespace") >> identifier[regist(make_namespace(arg1), self.driver_)] >> "{"
				>> *(define_class[analyzeMethod(arg1,self.driver_)]
					|Enum
					|function[analyze(arg1, self.driver_)]
					| decl_func[analyze(arg1, self.driver_)]
					| decl_value
					| nameSpace[popNameSpace(self.driver_)]) >> "}";

			// ���͂��ꂽ�\��
			input = *(define_class[analyzeMethod(arg1, self.driver_)]
				|Enum
				|function[analyze(arg1, self.driver_)]
				| decl_func[analyze(arg1, self.driver_)]
				| decl_value
				| nameSpace[popNameSpace(self.driver_)]
				| syntax_error_p
				);
		}

		rule<ScannerT> const& start() const
		{
			return input;
		}
	};
};

// �X�L�b�v
struct skip_parser : public grammar<skip_parser> {
	template <typename ScannerT>
	struct definition {
		rule<ScannerT> skip_p;

		definition(skip_parser const& self)
		{
			skip_p = space_p
				| comment_p("//")			// C++�R�����g�p
				| comment_p("/*", "*/")		// C�R�����g�p
				;
		}
		rule<ScannerT> const& start() const
		{
			return skip_p;
		}
	};
};

// EOF�܂Ői�񂾂̂��̔���
template <typename IteratorT, typename DerivedT>
bool skip_all(IteratorT first, IteratorT last, parser<DerivedT> const& p)
{
	for (;;) {
		parse_info<IteratorT> info = parse(first, last, p);
		if (info.full)
			return true;
		if (!info.hit)
			return false;
		first = info.stop;
	}
}
}
// �\�����
bool ButiScript::ScriptParser(const string& path, Compiler* driver)
{
	ifstream fin(path.c_str());

	if (!fin) {
		driver->error("�t�@�C��" + path + "���I�[�v���ł��܂���B");
		return false;
	}

	// �t�@�C����ǂݍ���
	istreambuf_iterator<char> fbegin(fin);
	istreambuf_iterator<char> fend;
	string input(fbegin, fend);

	// �|�W�V�����C�e���[�^
	iterator_t begin(input.begin(), input.end(), path);
	iterator_t end;
	begin.set_tabchars(4);	// tab=4�ɐݒ�

	funcAnalyze_grammer	gr(driver);
	registFunc_classAnalyze_grammer	gr_regist(driver);
	typeRegist_grammer	gr_typeRegist(driver);
	skip_parser skip_p;
	parse_info<iterator_t> info = parse(begin, end, gr_typeRegist, skip_p);
	if (!(info.hit && (info.full || skip_all(info.stop, end, skip_p)))) {
		driver->error("�\����͎��s");
		return false;
	}
	
	info = parse(begin, end, gr_regist, skip_p);
	if (!(info.hit && (info.full || skip_all(info.stop, end, skip_p)))) {
		driver->error("�\����͎��s");
		return false;
	}
	driver-> OpHalt();
	info = parse(begin, end, gr, skip_p);

	if (info.hit && (info.full || skip_all(info.stop, end, skip_p))) {
		return true;
	}

	driver->error("�\����͎��s");
	return false;
}
