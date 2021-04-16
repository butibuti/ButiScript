#include "stdafx.h"
#include "Node.h"
#include "compiler.h"

// 変数ノードを生成
Node_t Node::make_node(const int Op, const std::string& str)
{
	if (Op == OP_IDENTIFIER)
		return Node_t(new Node_value(str));

	if (Op == OP_STRING) {
		size_t pos = str.rfind('\"');
		if (pos != std::string::npos)
			return Node_t(new Node(Op, str.substr(0, pos)));
	}
	return Node_t(new Node(Op, str));
}

// 単項演算子のノードを生成
Node_t Node::make_node(const int Op, Node_t left)
{
	switch (Op) {
	case OP_NEG:
		if (left->op_ == OP_INT) {			// 定数演算を計算する
			left->number_ = -left->number_;
			return left;
		}
		break;
	}
	return Node_t(new Node(Op, left));
}

// 二項演算子のノードを生成
Node_t Node::make_node(const int Op, Node_t left, Node_t right)
{
	// 配列ノードは、leftノードのleft_メンバに加える
	if (Op == OP_ARRAY) {
		left->left_ = right;
		return left;
	}
	auto leftOp = left->op_;
	auto rightOp = right->op_;

	// 定数演算を計算する
	if (leftOp == OP_INT && rightOp == OP_INT) {
		switch (Op) {
		case OP_LOGAND:
			left->number_ = (left->number_ && right->number_) ? 1 : 0;
			break;

		case OP_LOGOR:
			left->number_ = (left->number_ || right->number_) ? 1 : 0;
			break;

		case OP_EQ:
			left->number_ = (left->number_ == right->number_) ? 1 : 0;
			break;

		case OP_NE:
			left->number_ = (left->number_ != right->number_) ? 1 : 0;
			break;

		case OP_GT:
			left->number_ = (left->number_ > right->number_) ? 1 : 0;
			break;

		case OP_GE:
			left->number_ = (left->number_ >= right->number_) ? 1 : 0;
			break;

		case OP_LT:
			left->number_ = (left->number_ < right->number_) ? 1 : 0;
			break;

		case OP_LE:
			left->number_ = (left->number_ <= right->number_) ? 1 : 0;
			break;

		case OP_AND:
			left->number_ &= right->number_;
			break;

		case OP_OR:
			left->number_ |= right->number_;
			break;

		case OP_LSHIFT:
			left->number_ <<= right->number_;
			break;

		case OP_RSHIFT:
			left->number_ >>= right->number_;
			break;

		case OP_SUB:
			left->number_ -= right->number_;
			break;

		case OP_ADD:
			left->number_ += right->number_;
			break;

		case OP_MUL:
			left->number_ *= right->number_;
			break;

		case OP_DIV:
			if (right->number_ == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				left->number_ /= right->number_;
			}
			break;

		case OP_MOD:
			if (right->number_ == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				left->number_ %= right->number_;
			}
			break;

		default:
			return Node_t(new Node(Op, left, right));
		}
		return left;
	}

	// 定数浮動小数演算を計算する

	if (leftOp == OP_FLOAT && rightOp == OP_FLOAT) {
		switch (Op) {
		case OP_LOGAND:
			left->num_float =(float)( (left->num_float && right->num_float) ? 1 : 0);
			break;

		case OP_LOGOR:
			left->num_float = (float)((left->num_float || right->num_float) ? 1 : 0);
			break;

		case OP_EQ:
			left->num_float = (float)((left->num_float == right->num_float) ? 1 : 0);
			break;

		case OP_NE:
			left->num_float = (float)((left->num_float != right->num_float) ? 1 : 0);
			break;

		case OP_GT:
			left->num_float = (float)((left->num_float > right->num_float) ? 1 : 0);
			break;

		case OP_GE:
			left->num_float = (float)((left->num_float >= right->num_float) ? 1 : 0);
			break;

		case OP_LT:
			left->num_float = (float)((left->num_float < right->num_float) ? 1 : 0);
			break;

		case OP_LE:
			left->num_float = (float)((left->num_float <= right->num_float) ? 1 : 0);
			break;

		case OP_AND:
			left->num_float =(float)( (int)left->num_float & (int)right->num_float);
			break;

		case OP_OR:
			left->num_float = (float)((int)left->num_float | (int)right->num_float);
			break;

		case OP_LSHIFT:
			left->num_float = (float)((int)left->num_float << (int)right->num_float);
			break;

		case OP_RSHIFT:
			left->num_float = (float)((int)left->num_float >> (int)right->num_float);
			break;

		case OP_SUB:
			left->num_float -= right->num_float;
			break;

		case OP_ADD:
			left->num_float += right->num_float;
			break;

		case OP_MUL:
			left->num_float *= right->num_float;
			break;

		case OP_DIV:
			if (right->num_float == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				left->num_float /= right->num_float;
			}
			break;

		case OP_MOD:
			if (right->num_float == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				left->num_float = (float)((int)left->num_float % (int)right->num_float);
			}
			break;

		default:
			return Node_t(new Node(Op, left, right));
		}
		return left;
	}


	if (leftOp == OP_INT&& rightOp == OP_FLOAT) {
		switch (Op) {
		case OP_LOGAND:
			left->number_ = (left->number_ && (int)right->num_float) ? 1 : 0;
			break;

		case OP_LOGOR:
			left->number_ = (left->number_ || (int)right->num_float) ? 1 : 0;
			break;

		case OP_EQ:
			left->number_ = (left->number_ == (int)right->num_float) ? 1 : 0;
			break;

		case OP_NE:
			left->number_ = (left->number_ != (int)right->num_float) ? 1 : 0;
			break;

		case OP_GT:
			left->number_ = (left->number_ > (int)right->num_float) ? 1 : 0;
			break;

		case OP_GE:
			left->number_ = (left->number_ >= (int)right->num_float) ? 1 : 0;
			break;

		case OP_LT:
			left->number_ = (left->number_ < (int)right->num_float) ? 1 : 0;
			break;

		case OP_LE:
			left->number_ = (left->number_ <= (int)right->num_float) ? 1 : 0;
			break;

		case OP_AND:
			left->number_ = (int)left->number_ & (int)(int)right->num_float;
			break;

		case OP_OR:
			left->number_ = (int)left->number_ | (int)(int)right->num_float;
			break;

		case OP_LSHIFT:
			left->number_ = (int)left->number_ << (int)(int)right->num_float;
			break;

		case OP_RSHIFT:
			left->number_ = (int)left->number_ >> (int)(int)right->num_float;
			break;

		case OP_SUB:
			left->number_ -= (int)right->num_float;
			break;

		case OP_ADD:
			left->number_ += (int)right->num_float;
			break;

		case OP_MUL:
			left->number_ *= (int)right->num_float;
			break;

		case OP_DIV:
			if ((int)right->num_float == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				left->number_ /= (int)right->num_float;
			}
			break;

		case OP_MOD:
			if ((int)right->num_float == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				left->number_ = (int)left->number_ % (int)right->num_float;
			}
			break;

		default:
			return Node_t(new Node(Op, left, right));
		}
		return left;
	}

	if (leftOp == OP_FLOAT && rightOp == OP_INT) {
		switch (Op) {
		case OP_LOGAND:
			left->num_float =(float)( (left->num_float && right->number_) ? 1 : 0);
			break;

		case OP_LOGOR:
			left->num_float = (float)((left->num_float || right->number_) ? 1 : 0);
			break;

		case OP_EQ:
			left->num_float = (float)((left->num_float == right->number_) ? 1 : 0);
			break;

		case OP_NE:
			left->num_float = (float)((left->num_float != right->number_) ? 1 : 0);
			break;

		case OP_GT:
			left->num_float = (float)((left->num_float > right->number_) ? 1 : 0);
			break;

		case OP_GE:
			left->num_float = (float)((left->num_float >= right->number_) ? 1 : 0);
			break;

		case OP_LT:
			left->num_float = (float)((left->num_float < right->number_) ? 1 : 0);
			break;

		case OP_LE:
			left->num_float = (float)((left->num_float <= right->number_) ? 1 : 0);
			break;

		case OP_AND:
			left->num_float = (float)((int)left->num_float & (int)right->number_);
			break;

		case OP_OR:
			left->num_float = (float)((int)left->num_float | (int)right->number_);
			break;

		case OP_LSHIFT:
			left->num_float = (float)((int)left->num_float << (int)right->number_);
			break;

		case OP_RSHIFT:
			left->num_float = (float)((int)left->num_float >> (int)right->number_);
			break;

		case OP_SUB:
			left->num_float -= right->number_;
			break;

		case OP_ADD:
			left->num_float += right->number_;
			break;

		case OP_MUL:
			left->num_float *= right->number_;
			break;

		case OP_DIV:
			if (right->number_ == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				left->num_float /= right->number_;
			}
			break;

		case OP_MOD:
			if (right->number_ == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				left->num_float = (float)((int)left->num_float % (int)right->number_);
			}
			break;

		default:
			return Node_t(new Node(Op, left, right));
		}
		return left;
	}

	// 文字列同士の定数計算
	if (leftOp == OP_STRING && rightOp == OP_STRING) {
		if (Op == OP_ADD) {
			left->string_ += right->string_;
			return left;
		}

		int Value = 0;
		switch (Op) {
		case OP_EQ:
			if (left->string_ == right->string_)
				Value = 1;
			break;

		case OP_NE:
			if (left->string_ != right->string_)
				Value = 1;
			break;

		case OP_GT:
			if (left->string_ > right->string_)
				Value = 1;
			break;

		case OP_GE:
			if (left->string_ >= right->string_)
				Value = 1;
			break;

		case OP_LT:
			if (left->string_ < right->string_)
				Value = 1;
			break;

		case OP_LE:
			if (left->string_ <= right->string_)
				Value = 1;
			break;

		default:
			std::cerr << "文字列同士ではできない計算です。" << std::endl;
			break;
		}
		return Node_t(new Node(OP_INT, Value));
	}
	return Node_t(new Node(Op, left, right));
}

// 関数ノードの生成
Node_t Node::make_node(const int Op, Node_t left, NodeList_t right)
{
	return Node_t(new Node_function(Op, left, right));
}

// ノードのpush処理
int Node::Push(Compiler* c) const
{
	switch (op_) {
	case OP_NEG:
		if (left_->Push(c) == TYPE_STRING)
			c->error("文字列には許されない計算です。");
		c->OpNeg();
		return TYPE_INTEGER;

	case OP_INT:
		c->PushConst(number_);
		return TYPE_INTEGER;
	case OP_FLOAT:
		c->PushConstFloat(num_float);
		return TYPE_FLOAT;

	case OP_STRING:
		c->PushString(string_);
		return TYPE_STRING;

	case OP_FUNCTION:
		return Call(c, string_, NULL);
	}

	int left_type = left_->Push(c);
	int right_type = right_->Push(c);

	//右辺若しくは左辺が未定義関数
	if (left_type <= -1 || right_type <= -1) {
		return -1;
	}

	// float計算ノードの処理
	if (left_type ==TYPE_FLOAT|| right_type == TYPE_FLOAT ) {
		switch (op_) {
		case OP_LOGAND:
			c->OpLogAnd();
			break;

		case OP_LOGOR:
			c->OpLogOr();
			break;

		case OP_EQ:
			c->OpEq();
			break;

		case OP_NE:
			c->OpNe();
			break;

		case OP_GT:
			c->OpGt();
			break;

		case OP_GE:
			c->OpGe();
			break;

		case OP_LT:
			c->OpLt();
			break;

		case OP_LE:
			c->OpLe();
			break;

		case OP_AND:
			c->OpAnd();
			break;

		case OP_OR:
			c->OpOr();
			break;

		case OP_LSHIFT:
			c->OpLeftShift();
			break;

		case OP_RSHIFT:
			c->OpRightShift();
			break;

		case OP_SUB:
			c->OpFloatSub();
			break;

		case OP_ADD:
			c->OpFloatAdd();
			break;

		case OP_MUL:
			c->OpFloatMul();
			break;

		case OP_DIV:
			c->OpFloatDiv();
			break;

		case OP_MOD:
			c->OpFloatMod();
			break;

		default:
			c->error("内部エラー：処理できない計算ノードがありました。");
			break;
		}
		return TYPE_FLOAT;
	}

	// 整数計算ノードの処理
	if (left_type ==  TYPE_INTEGER && right_type==TYPE_INTEGER) {
		switch (op_) {
		case OP_LOGAND:
			c->OpLogAnd();
			break;

		case OP_LOGOR:
			c->OpLogOr();
			break;

		case OP_EQ:
			c->OpEq();
			break;

		case OP_NE:
			c->OpNe();
			break;

		case OP_GT:
			c->OpGt();
			break;

		case OP_GE:
			c->OpGe();
			break;

		case OP_LT:
			c->OpLt();
			break;

		case OP_LE:
			c->OpLe();
			break;

		case OP_AND:
			c->OpAnd();
			break;

		case OP_OR:
			c->OpOr();
			break;

		case OP_LSHIFT:
			c->OpLeftShift();
			break;

		case OP_RSHIFT:
			c->OpRightShift();
			break;

		case OP_SUB:
			c->OpSub();
			break;

		case OP_ADD:
			c->OpAdd();
			break;

		case OP_MUL:
			c->OpMul();
			break;

		case OP_DIV:
			c->OpDiv();
			break;

		case OP_MOD:
			c->OpMod();
			break;

		default:
			c->error("内部エラー：処理できない計算ノードがありました。");
			break;
		}
		return TYPE_INTEGER;
	}


	// 文字列計算ノードの処理
	switch (op_) {
	case OP_EQ:
		c->OpStrEq();
		return TYPE_INTEGER;

	case OP_NE:
		c->OpStrNe();
		return TYPE_INTEGER;

	case OP_GT:
		c->OpStrGt();
		return TYPE_INTEGER;

	case OP_GE:
		c->OpStrGe();
		return TYPE_INTEGER;

	case OP_LT:
		c->OpStrLt();
		return TYPE_INTEGER;

	case OP_LE:
		c->OpStrLe();
		return TYPE_INTEGER;

	case OP_ADD:
		c->OpStrAdd();
		break;

	default:
		c->error("文字列では計算できない式です。");
		break;
	}
	return TYPE_STRING;
}

// ノードのpop
// 計算ノードはpopできない

int Node::Pop(Compiler* c) const
{
	c->error("内部エラー：計算ノードをpopしています。");
	return TYPE_INTEGER;
}

// 代入文
int Node::Assign(Compiler* c) const
{
	if (op_ != OP_ASSIGN)
		left_->Push(c);
	auto rightType = right_->Push(c);

	//rightがまだ定義されてない関数なのでスキップ
	if (rightType == -1) {
		return -1;
	}

	if (rightType == TYPE_INTEGER) {
		switch (op_) {
		case OP_ADD_ASSIGN:
			c->OpAdd();
			break;

		case OP_SUB_ASSIGN:
			c->OpSub();
			break;

		case OP_MUL_ASSIGN:
			c->OpMul();
			break;

		case OP_DIV_ASSIGN:
			c->OpDiv();
			break;

		case OP_MOD_ASSIGN:
			c->OpMod();
			break;

		case OP_AND_ASSIGN:
			c->OpAnd();
			break;

		case OP_OR_ASSIGN:
			c->OpOr();
			break;

		case OP_LSHIFT_ASSIGN:
			c->OpLeftShift();
			break;

		case OP_RSHIFT_ASSIGN:
			c->OpRightShift();
			break;
		}
		if (left_->Pop(c) != TYPE_INTEGER)
			c->error("文字列型に整数を代入しています。");
		return 0;
	}
	if (rightType == TYPE_FLOAT) {
		switch (op_) {
		case OP_ADD_ASSIGN:
			c->OpFloatAdd();
			break;

		case OP_SUB_ASSIGN:
			c->OpFloatSub();
			break;

		case OP_MUL_ASSIGN:
			c->OpFloatMul();
			break;

		case OP_DIV_ASSIGN:
			c->OpFloatDiv();
			break;

		case OP_MOD_ASSIGN:
			c->OpFloatMod();
			break;

		case OP_AND_ASSIGN:
			c->OpAnd();
			break;

		case OP_OR_ASSIGN:
			c->OpOr();
			break;

		case OP_LSHIFT_ASSIGN:
			c->OpLeftShift();
			break;

		case OP_RSHIFT_ASSIGN:
			c->OpRightShift();
			break;
		}
		if (left_->Pop(c) != TYPE_FLOAT)
			c->error("文字列型に浮動小数を代入しています。");
		return 0;
	}

	switch (op_) {
	case OP_ADD_ASSIGN:
		c->OpStrAdd();
		break;

	case OP_ASSIGN:
		break;

	default:
		c->error("文字列では許されない計算です。");
		break;
	}
	return 0;
	if (left_->Pop(c) != TYPE_STRING)
		c->error("整数型に文字列を代入しています。");

	return -1;
}

// 変数ノードのpush
int Node_value::Push(Compiler* c) const
{
	if (op_ != OP_IDENTIFIER) {
		c->error("内部エラー：変数ノードに変数以外が登録されています。");
	}
	else {
		const ValueTag* tag = c->GetValueTag(string_);
		if (tag == 0) {
			c->error("変数 " + string_ + " は定義されていません。");
		}
		else {
			// 参照型変数は、引数にしか存在しない
			if ((tag->type_ & TYPE_REF) != 0) {
				if (left_) {		// 配列
					left_->Push(c);
					c->PushLocalArrayRef(tag->addr_);
				}
				else {
					c->PushLocalRef(tag->addr_);
				}
				return tag->type_ & ~TYPE_REF;
			}
			if (tag->global_) {		// 外部変数
				if (left_) {		// 配列
					left_->Push(c);
					c->PushArray(tag->addr_);
				}
				else {
					c->PushValue(tag->addr_);
				}
			}
			else {					// ローカル変数
				if (left_) {		// 配列
					left_->Push(c);
					c->PushLocalArray(tag->addr_);
				}
				else {
					c->PushLocal(tag->addr_);
				}
			}
			return tag->type_;
		}
	}
	return TYPE_INTEGER;
}

// 変数ノードのpop
int Node_value::Pop(Compiler* c) const
{
	if (op_ != OP_IDENTIFIER) {
		c->error("内部エラー：変数ノードに変数以外が登録されています。");
	}
	else {
		const ValueTag* tag = c->GetValueTag(string_);
		if (tag == 0) {
			c->error("変数 " + string_ + " は定義されていません。");
		}
		else {
			// 参照型変数は、引数にしか存在しない
			if ((tag->type_ & TYPE_REF) != 0) {
				if (left_) {		// 配列
					left_->Push(c);
					c->PopLocalArrayRef(tag->addr_);
				}
				else {
					c->PopLocalRef(tag->addr_);
				}
				return tag->type_ & ~TYPE_REF;
			}
			if (tag->global_) {		// 外部変数
				if (left_) {		// 配列
					left_->Push(c);
					c->PopArray(tag->addr_);
				}
				else {
					c->PopValue(tag->addr_);
				}
			}
			else {					// ローカル変数
				if (left_) {		// 配列
					left_->Push(c);
					c->PopLocalArray(tag->addr_);
				}
				else {
					c->PopLocal(tag->addr_);
				}
			}
			return tag->type_;
		}
	}
	return TYPE_INTEGER;
}

// 関数呼び出し
struct set_arg {
	Compiler* comp_;
	const FunctionTag* func_;
	mutable int index_;
	set_arg(Compiler* comp, const FunctionTag* func) : comp_(comp), func_(func), index_(0)
	{
	}

	void operator()(Node_t node) const
	{
		int type = func_->GetArg(index_++);
		if ((type & TYPE_REF) != 0) {		// 参照
			if (node->Op() != OP_IDENTIFIER) {
				comp_->error("参照型引数に、変数以外は指定できません。");
			}
			else {
				const ValueTag* tag = comp_->GetValueTag(node->GetString());
				if (tag == 0) {
					comp_->error("変数 " + node->GetString() + " は定義されていません。");
				}
				else if (tag->type_ >= TYPE_INTEGER_REF) {		// 参照
					// 参照型変数は、ローカルしかない
					if (node->GetLeft()) {
						node->GetLeft()->Push(comp_);
						comp_->PushLocal(tag->addr_);
						comp_->OpAdd();
					}
					else {
						comp_->PushLocal(tag->addr_);
					}
				}
				else {
					if ((tag->type_ | TYPE_REF) != type) {
						comp_->error("引数の型が合いません。");
					}
					int addr = tag->addr_;
					if (tag->global_)			// 外部変数
						addr |= ButiVM::VirtualCPU::global_flag;
					// アドレスをpush
					if (node->GetLeft()) {			// 配列
						if (node->GetLeft()->Op() == OP_INT) {
							comp_->PushAddr(addr + node->GetLeft()->GetNumber());
						}
						else {
							node->GetLeft()->Push(comp_);
							comp_->PushArrayAddr(addr);
						}
					}
					else {
						comp_->PushAddr(addr);
					}
				}
			}
		}
		else {
			if (node->Push(comp_) != type) {
				comp_->error("引数の型が合いません。");
			}
		}
	}
};

// 関数呼び出し
int Node::Call(Compiler* c, const std::string& name, const std::vector<Node_t>* args) const
{
	const FunctionTag* tag = c->GetFunctionTag(name);
	if (tag == NULL) {
		return -1;
	}

	int arg_size = (args) ? (int)args->size() : 0;
	if (tag->ArgSize() != arg_size) {
		c->error("引数の数が合いません。");
	}

	// 引数をpush
	if (args && tag->ArgSize() == arg_size) {
		std::for_each(args->begin(), args->end(), set_arg(c, tag));
	}

	// 引数の数をpush
	c->PushConst(arg_size);

	if (tag->IsSystem()) {
		c->OpSysCall(tag->GetIndex());		// 組み込み関数
	}
	else {
		c->OpCall(tag->GetIndex());			// スクリプト上の関数
	}

	return tag->type_;
}

int Node_function::Push(Compiler* c) const
{
	return Call(c, left_->GetString(), &node_list_->args_);
}

// 関数にpopはできないのでエラーメッセージを出す
int Node_function::Pop(Compiler* c) const
{
	c->error("内部エラー：関数ノードをpopした");
	return TYPE_INTEGER;
}

// 文ノード生成
Statement_t Statement::make_statement(const int state)
{
	switch (state) {
	case NOP_STATE:
		return Statement_t(new Statement_nop());

	case DEFAULT_STATE:
		return Statement_t(new Statement_default());

	case BREAK_STATE:
		return Statement_t(new Statement_break());

	case RETURN_STATE:
		return Statement_t(new Statement_return());

	case IF_STATE:
		return Statement_t(new Statement_if());

	case FOR_STATE:
		return Statement_t(new Statement_for());

	case WHILE_STATE:
		return Statement_t(new Statement_while());
	}

	std::cerr << "内部エラー：文ノードミス" << std::endl;
	return Statement_t(new Statement_nop());
}

Statement_t Statement::make_statement(const int state, Node_t node)
{
	switch (state) {
	case ASSIGN_STATE:
		return Statement_t(new Statement_assign(node));

	case CASE_STATE:
		return Statement_t(new Statement_case(node));

	case SWITCH_STATE:
		return Statement_t(new Statement_switch(node));

	case CALL_STATE:
		return Statement_t(new ccall_statement(node));
	}

	std::cerr << "内部エラー：文ノードミス" << std::endl;
	return Statement_t(new Statement_nop());
}

Statement_t Statement::make_statement(const int state, Block_t block)
{
	switch (state) {
	case BLOCK_STATE:
		return Statement_t(new Statement_block(block));
	}

	std::cerr << "内部エラー：文ノードミス" << std::endl;
	return Statement_t(new Statement_nop());
}

// nop文
int Statement_nop::Analyze(Compiler* c) 
{
	return 0;
}

// 代入文
int Statement_assign::Analyze(Compiler* c) 
{
	return node_->Assign(c);
	
}

int Statement_assign::ReAnalyze(Compiler* c)
{
	return node_->Assign(c);
	
}

// 関数呼び出し文
int ccall_statement::Analyze(Compiler* c) 
{
	int type = node_->Push(c);

	if (type == -1) {

		return -1;
	}

	if (type != TYPE_VOID)
		c->OpPop();			// 戻り値を捨てるためのpop

	return 0;
}

void ccall_statement::ReAnalyze(Compiler* c) 
{
	int type = node_->Push(c);

	if (type == -1) {

		c->error("定義されていない関数を参照しています");
	}

	if (type != TYPE_VOID)
		c->OpPop();
}

// case文
int Statement_case::Analyze(Compiler* c) 
{
	c->SetLabel(label_);
	return 0;
}

// case文の前処理
int Statement_case::case_Analyze(Compiler* c, int* default_label)
{
	label_ = c->MakeLabel();
	if (node_->Op() != OP_INT)
		c->error("case 文には定数のみ指定できます。");
	node_->Push(c);
	c->OpTest(label_);
	return 0;
}

// default文
int Statement_default::Analyze(Compiler* c) 
{
	c->SetLabel(label_);
	return 0;
}

// default文の前処理
int Statement_default::case_Analyze(Compiler* c, int* default_label)
{
	label_ = c->MakeLabel();
	*default_label = label_;

	return 0;
}

// break文
int Statement_break::Analyze(Compiler* c) 
{
	if (!c->JmpBreakLabel()) {
		c->error("breakがswitch/for/while外に有ります");
	}
	return 0;
}

// return文
int Statement_return::Analyze(Compiler* c) 
{
	if (c->GetFunctionType() == TYPE_VOID) {	// 戻り値無し
		if (node_ != 0) {
			c->error("void関数に戻り値が設定されています");
		}
		c->OpReturn();
	}
	else {
		if (node_ == 0) {
			c->error("関数の戻り値がありません");
		}
		else {
			int node_type = node_->Push(c);		// 戻り値をpush

			if (node_type == -1) {//宣言されていない関数参照してたらスキップ
				return -1;
			}

			if (node_type != c->GetFunctionType()) {
				c->error("戻り値の型が合いません");
			}
		}
		c->OpReturnV();
	}
	return 0;
}

// if文
int Statement_if::Analyze(Compiler* c) 
{
	node_->Push(c);
	int label1 = c->MakeLabel();
	c->OpJmpNC(label1);
	statement_[0]->Analyze(c);

	if (statement_[1]) {
		int label2 = c->MakeLabel();
		c->OpJmp(label2);
		c->SetLabel(label1);
		statement_[1]->Analyze(c);
		c->SetLabel(label2);
	}
	else {
		c->SetLabel(label1);
	}
	return 0;
}

// for文
int Statement_for::Analyze(Compiler* c) 
{
	int label1 = c->MakeLabel();
	int label2 = c->MakeLabel();

	int break_label = c->SetBreakLabel(label2);

	node_[0]->Assign(c);
	c->SetLabel(label1);
	node_[1]->Push(c);
	c->OpJmpNC(label2);
	statement_->Analyze(c);
	node_[2]->Assign(c);
	c->OpJmp(label1);
	c->SetLabel(label2);

	c->SetBreakLabel(break_label);
	return 0;
}

// while文
int Statement_while::Analyze(Compiler* c) 
{
	int label1 = c->MakeLabel();
	int label2 = c->MakeLabel();

	int break_label = c->SetBreakLabel(label2);

	c->SetLabel(label1);
	node_->Push(c);
	c->OpJmpNC(label2);
	statement_->Analyze(c);
	c->OpJmp(label1);
	c->SetLabel(label2);

	c->SetBreakLabel(break_label);
	return 0;
}

// switch文
int Statement_switch::Analyze(Compiler* c) 
{
	if (!statement_.empty()) {
		node_->Push(c);

		int label = c->MakeLabel();		// L0ラベル作成
		int break_label = c->SetBreakLabel(label);
		int default_label = label;

		std::for_each(statement_.begin(), statement_.end(),
			boost::bind(&Statement::Case_Analyze, _1, c, &default_label));

		c->OpPop();
		c->OpJmp(default_label);

		std::for_each(statement_.begin(), statement_.end(), boost::bind(&Statement::Analyze, _1, c));
		c->SetLabel(label);

		c->SetBreakLabel(break_label);
	}
	return 0;
}

// block文
int Statement_block::Analyze(Compiler* c) 
{
	c->BlockIn();
	block_->Analyze(c);
	c->BlockOut();
	return 0;
}

// 文ブロック
int Block::Analyze(Compiler* c) 
{
	auto ret = 0;
	{

		auto endItr = decl_.end();

		for (auto itr = decl_.begin(); itr != endItr; itr++) {
			if ((ret=(*itr)->Analyze(c))!=0) {
				return ret;
			}
		}
	}
	if (!decl_.empty())
		c->AllocStack();	// スタックフレーム確保

	{

		auto endItr = state_.end();

		for (auto itr = state_.begin(); itr != endItr; itr++) {
			if ((ret = (*itr)->Analyze(c)) != 0) {
				return ret;
			}
		}
	}

	return ret;
}

// 宣言の解析
int Declaration::Analyze(Compiler* c) 
{
	if (is_func_) {		// 関数
		c->FunctionDefine(type_, name_, arg_);
	}
	else {
		c->ValueDefine(type_, node_);
	}
	return 0;
}

// 関数の解析
int Function::Analyze(Compiler* c) 
{
	c->AddFunction(type_, name_, args_, block_);

	return 0;
}
