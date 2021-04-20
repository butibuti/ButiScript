#include "stdafx.h"
#include "Parser.h"
#include "Node.h"
#include"Compiler.h"
using namespace std;
using namespace boost::spirit;

// 入力用のイテレータを定義
typedef position_iterator<string::const_iterator>	iterator_t;

// エラー処理パーサー定義
struct error_parser {
	typedef nil_t result_t;		// パーサーの結果型（nil_t）

	error_parser(char const* msg)
		: msg_(msg)
	{
	}

	template <typename ScannerT>
	int operator()(ScannerT const& scan, result_t& result) const
	{
		// 終わりまで来たら-1を返す
		if (scan.at_end()) {
			return -1;
		}

		// 改行までをスキャンし、そこまでを表示する。

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

// エラー処理パーサー
typedef functor_parser<error_parser> error_p;

// 文法エラー処理パーサー
error_p syntax_error_p = error_parser("文法エラー");

// 単項演算子ノードを生成する
struct unary_node_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef Node_t type; };

	template <typename Ty1, typename Ty2>
	Node_t operator()(Ty1 Op, const Ty2& left) const
	{
		return Node::make_node(Op, left);
	}
};

// 二項演算子ノードを生成する
struct binary_node_impl {
	template <typename Ty1, typename Ty2, typename Ty3>
	struct result { typedef Node_t type; };

	template <typename Ty1, typename Ty2, typename Ty3>
	Node_t operator()(Ty1 Op, const Ty2& left, const Ty3& right) const
	{
		return Node::make_node(Op, left, right);
	}
};

// 値を追加
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

// ノードリストを生成する
struct make_argument_impl {
	template <typename Ty>
	struct result { typedef NodeList_t type; };

	template <typename Ty>
	NodeList_t operator()(const Ty& node) const
	{
		return NodeList_t(new NodeList(node));
	}
};

// ステートメントの生成
struct make_statement_impl {
	template <typename Ty>
	struct result { typedef Statement_t type; };

	template <typename Ty>
	Statement_t operator()(Ty state) const
	{
		return Statement::make_statement(state);
	}
};

// ステートメントの生成
struct make_statement1_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef Statement_t type; };

	template <typename Ty1, typename Ty2>
	Statement_t operator()(Ty1 state, const Ty2& node) const
	{
		return Statement::make_statement(state, node);
	}
};

// ステートメントにインデックス付きで値を追加
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

// 宣言の生成
struct make_decl_impl {
	template <typename Ty>
	struct result { typedef Declaration_t type; };

	template <typename Ty>
	Declaration_t operator()(Ty type) const
	{
		return Declaration_t(new Declaration(type));
	}
};

// 宣言の生成
struct make_decl1_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef Declaration_t type; };

	template <typename Ty1, typename Ty2>
	Declaration_t operator()(Ty1 type, const Ty2& node) const
	{
		return Declaration_t(new Declaration(type, node));
	}
};

// 引数宣言の型を参照にする
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

// 引数の名前を設定
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

// 関数の生成
struct make_function_impl {
	template < typename Ty2>
	struct result { typedef Function_t type; };

	template <typename Ty2>
	Function_t operator()(const Ty2& name) const
	{
		return Function_t(new Function(name));
	}
};
// 関数の返り値設定
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

// 最終登録
struct analyze_impl {
	template <typename Ty1, typename Ty2>
	struct result { typedef void type; };

	template <typename Ty1, typename Ty2>
	void operator()(const Ty1& decl, Ty2 driver) const
	{
		decl->Analyze(driver);
	}
};

// phoenixが使用する「無名関数」用の関数
phoenix::function<binary_node_impl> const binary_node = binary_node_impl();
phoenix::function<unary_node_impl> const unary_node = unary_node_impl();
phoenix::function<push_back_impl> const push_back = push_back_impl();
phoenix::function<make_argument_impl> const make_argument = make_argument_impl();
phoenix::function<make_statement_impl> const make_statement = make_statement_impl();
phoenix::function<make_statement1_impl> const make_statement1 = make_statement1_impl();
phoenix::function<add_statement_impl> const add_statement = add_statement_impl();
phoenix::function<make_decl_impl> const make_decl = make_decl_impl();
phoenix::function<make_decl1_impl> const make_decl1 = make_decl1_impl();
phoenix::function<arg_ref_impl> const arg_ref = arg_ref_impl();
phoenix::function<arg_name_impl> const arg_name = arg_name_impl();
phoenix::function<make_function_impl> const make_function = make_function_impl();
phoenix::function<setFunctionType_impl> const set_functionType = setFunctionType_impl();
phoenix::function<analyze_impl> const analyze = analyze_impl();

real_parser<double, ureal_parser_policies<double> > const
ureal_parser = real_parser<double, ureal_parser_policies<double> >();

// 文法定義
struct script_grammer : public grammar<script_grammer> {
	script_grammer(Compiler* driver)
		:driver_(driver)
	{
	}

	Compiler* driver_;	// コンパイラ

	// 文字列のクロージャ
	struct string_val : closure<string_val, std::string> {
		member1 str;
	};
	// 整数のクロージャ
	struct number_val : closure<number_val, unsigned int> {
		member1 number;
	};
	// 浮動小数クロージャ
	struct float_val : closure<float_val, float> {
		member1 number;
	};
	// ノードのクロージャ
	struct node_val : closure<node_val, Node_t, int> {
		member1 node;
		member2 Op;
	};
	// ノードのクロージャ
	struct nodelist_val : closure<nodelist_val, NodeList_t, int> {
		member1 node;
		member2 Op;
	};
	// 文のクロージャ
	struct state_val : closure<state_val, Statement_t> {
		member1 statement;
	};
	// 型のクロージャ
	struct type_val : closure<type_val, int> {
		member1 type;
	};

	// 変数定義のクロージャ
	struct decl_val : closure<decl_val, Declaration_t, int, Node_t> {
		member1 node;
		member2 type;
		member3 value;
	};

	// 関数定義のクロージャ
	struct func_val : closure<func_val, Function_t, int,std::string> {
		member1 node;
		member2 type;
		member3 name;
	};

	// 引数定義のクロージャ
	struct argdef_val : closure<argdef_val, ArgDefine,std::string> {
		member1 node;
		member2 name;
	};

	// 文ブロックのクロージャ
	struct block_val : closure<block_val, Block_t> {
		member1 node;
	};


	template <typename ScannerT>
	struct definition {
		rule<ScannerT, string_val::context_t>	identifier;
		rule<ScannerT, string_val::context_t>	string_node;
		rule<ScannerT, number_val::context_t>	number;
		rule<ScannerT, float_val::context_t>	floatNumber;
		rule<ScannerT, type_val::context_t>		type;
		rule<ScannerT, node_val::context_t>		func_node;
		rule<ScannerT, node_val::context_t>		Value;
		rule<ScannerT, node_val::context_t>		prime;
		rule<ScannerT, node_val::context_t>		unary;
		rule<ScannerT, node_val::context_t>		mul_expr;
		rule<ScannerT, node_val::context_t>		add_expr;
		rule<ScannerT, node_val::context_t>		shift_expr;
		rule<ScannerT, node_val::context_t>		bit_expr;
		rule<ScannerT, node_val::context_t>		equ_expr;
		rule<ScannerT, node_val::context_t>		and_expr;
		rule<ScannerT, node_val::context_t>		expr;
		rule<ScannerT, node_val::context_t>		assign;
		rule<ScannerT, nodelist_val::context_t>	argument;
		rule<ScannerT, state_val::context_t>	statement;
		rule<ScannerT, func_val::context_t>		function;
		rule<ScannerT, type_val::context_t>		arg;
		rule<ScannerT, decl_val::context_t>		decl_value;
		rule<ScannerT, decl_val::context_t>		decl_func;
		rule<ScannerT, argdef_val::context_t>	argdef;
		rule<ScannerT, block_val::context_t>	block;
		rule<ScannerT>							input;
		rule<ScannerT>							ident;

		symbols<> keywords;
		symbols<> mul_op, add_op, shift_op, bit_op, equ_op, assign_op;
		dynamic_distinct_parser<ScannerT> keyword_p;

		definition(script_grammer const& self)
			:keyword_p(alnum_p | '_')
		{
			using phoenix::arg1;
			using phoenix::arg2;
			using phoenix::var;
			using phoenix::new_;
			using phoenix::construct_;

			keywords = "if", "for", "while", "switch", "case", "default", "break", "return";
			// 識別子
			ident = lexeme_d[
				((alpha_p | '_') >> *(alnum_p | '_')) - (keywords >> anychar_p - (alnum_p | '_'))
			];
			// 識別子（クロージャに登録）
			identifier = ident[identifier.str = construct_<string>(arg1, arg2)];

			//整数
			number = uint_p[number.number = arg1];

			//浮動小数
			floatNumber = strict_real_p[floatNumber.number = arg1];

			// 文字列
			string_node = lexeme_d[
				confix_p(ch_p('"')[string_node.str = ""], *c_escape_ch_p[string_node.str += arg1], '"')
			];

			// 変数
			Value = identifier[Value.node = unary_node(OP_IDENTIFIER, arg1)]
				>> !('[' >> expr[Value.node = binary_node(OP_ARRAY, Value.node, arg1)] >> ']');

			// 関数の引数
			argument = expr[argument.node = make_argument(arg1)]
				>> *(',' >> expr[argument.node = push_back(argument.node, arg1)]);

			// 関数呼び出し
			func_node = identifier[func_node.node = unary_node(OP_FUNCTION, arg1)] >>
				'(' >> !argument[func_node.node = binary_node(OP_FUNCTION, func_node.node, arg1)] >> ')';

			// 計算のprimeノード
			prime = func_node[prime.node = arg1]
				| Value[prime.node = arg1]
				| floatNumber[prime.node = unary_node(OP_FLOAT, arg1)]
				| number[prime.node = unary_node(OP_INT, arg1)] 
				| string_node[prime.node = unary_node(OP_STRING, arg1)]
				| '(' >> expr[prime.node = arg1] >> ')'
				;

			// 単項演算子
			unary = prime[unary.node = arg1]
				| '-' >> prime[unary.node = unary_node(OP_NEG, arg1)];

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
			assign = Value[assign.node = arg1]
				>> assign_op[assign.Op = arg1]
				>> expr[assign.node = binary_node(assign.Op, assign.node, arg1)];

			// 変数宣言
			decl_value = "var" >> Value[decl_value.value = arg1] % ',' >> ':' >> type[decl_value.node = push_back(make_decl(arg1), decl_value.value)] >> ';';

			// 型宣言
			type = keyword_p("int")[type.type = TYPE_INTEGER] >> !ch_p('&')[type.type |= TYPE_REF]
				| keyword_p("float")[type.type = TYPE_FLOAT] >> !ch_p('&')[type.type |= TYPE_REF]
				| keyword_p("string")[type.type = TYPE_STRING] >> !ch_p('&')[type.type |= TYPE_REF]
				| keyword_p("void")[type.type = TYPE_VOID]
				;

			// 関数宣言の引数
			arg = identifier>>':'
				>>type[arg.type = arg1]
				>> !str_p("[]")[arg.type |= TYPE_REF];

			// 関数宣言
			decl_func = type[decl_func.type = arg1]
				>> identifier[decl_func.node = make_decl1(decl_func.type, arg1)]
				>> '(' >> !(arg[decl_func.node = push_back(decl_func.node, arg1)] % ',') >> ')' >> ';';

			// 関数定義の引数
			argdef = identifier[argdef.name = arg1]>>':'
				>>type[argdef.node = construct_<ArgDefine>(arg1, argdef.name)]
				>> !str_p("[]")[argdef.node = arg_ref(argdef.node)];

			// 関数定義
			function =  identifier[function.node = make_function(arg1)]
				>> '(' >> !(argdef[function.node = push_back(function.node, arg1)] % ',') >> ')'>>
				':' >> type[function.node = set_functionType(function.node, arg1)]
				>> block[function.node = push_back(function.node, arg1)];

			// 文ブロック
			block = ch_p('{')[block.node = construct_<Block_t>(new_<Block>())]
				>> *(statement[block.node = push_back(block.node, arg1)]
					| decl_value[block.node = push_back(block.node, arg1)])
				>> '}';

			// 文
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
				>> assign[statement.statement = add_statement(statement.statement, 0, arg1)] >> ';'
				>> expr[statement.statement = add_statement(statement.statement, 1, arg1)] >> ';'
				>> assign[statement.statement = add_statement(statement.statement, 2, arg1)] >> ')'
				>> statement[statement.statement = push_back(statement.statement, arg1)]
				| str_p("while")[statement.statement = make_statement(WHILE_STATE)] >> '('
				>> expr[statement.statement = push_back(statement.statement, arg1)] >> ')'
				>> statement[statement.statement = push_back(statement.statement, arg1)]
				| str_p("switch") >> '('
				>> expr[statement.statement = make_statement1(SWITCH_STATE, arg1)] >> ')'
				>> '{'
				>> *statement[statement.statement = push_back(statement.statement, arg1)]
				>> '}'
				| func_node[statement.statement = make_statement1(CALL_STATE, arg1)] >> ';'
				| block[statement.statement = make_statement1(BLOCK_STATE, arg1)]
				;

			// 入力された構文
			input = *( decl_func[analyze(arg1, self.driver_)]
				| function[analyze(arg1, self.driver_)]
				| decl_value[analyze(arg1, self.driver_)]
				| syntax_error_p
				);
		}

		rule<ScannerT> const& start() const
		{
			return input;
		}
	};
};

// スキップイテレーター
// コメントのスキップを加える
struct skip_parser : public grammar<skip_parser> {
	template <typename ScannerT>
	struct definition {
		rule<ScannerT> skip_p;

		definition(skip_parser const& self)
		{
			skip_p = space_p
				| comment_p("//")			// C++コメント用
				| comment_p("/*", "*/")		// Cコメント用
				;
		}
		rule<ScannerT> const& start() const
		{
			return skip_p;
		}
	};
};

// 全部スキップしてみて、EOFまで来たか？
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

// 構文解析
bool ScriptParser(const string& path, Compiler* driver)
{
	ifstream fin(path.c_str());

	if (!fin) {
		driver->error("ファイル" + path + "がオープンできません。");
		return false;
	}

	// ファイルを一度メモリーに読み込む
	istreambuf_iterator<char> fbegin(fin);
	istreambuf_iterator<char> fend;
	string input(fbegin, fend);

	// ポジションイテレータ
	iterator_t begin(input.begin(), input.end(), path);
	iterator_t end;
	begin.set_tabchars(4);	// tab=4に設定

	script_grammer	gr(driver);
	skip_parser skip_p;
	parse_info<iterator_t> info = parse(begin, end, gr, skip_p);
	driver->ReRegistFunctions();
	if (info.hit && (info.full || skip_all(info.stop, end, skip_p))) {
		return true;
	}

	driver->error("構文解析失敗");

	return false;
}
