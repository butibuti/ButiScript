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
#include "Node.h"
#include "compiler.h"


namespace ButiScript {
bool CanTypeCast(const int arg_left, const int arg_right) {
	if (arg_left == TYPE_STRING || arg_right == TYPE_STRING) {
		if (arg_left != arg_right) {
			return false;
		}
	}
	return true;
}


const EnumTag* GetEnumType(const Compiler* c, Node& left_) {

	auto shp_namespace = c->GetCurrentNameSpace();
	std::string serchName;
	const  EnumTag* enumType = nullptr;
	while (!enumType)
	{
		serchName = shp_namespace->GetGlobalNameString() + left_.GetString();

		enumType = c->GetEnumTag(serchName);

		if (enumType) {
			break;
		}

		shp_namespace = shp_namespace->GetParent();
		if (!shp_namespace) {
			break;
		}

	}
	return enumType;
}
const FunctionTag* GetFunctionType(const Compiler* c, const Node& left_) {

	auto shp_namespace = c->GetCurrentNameSpace();
	std::string serchName;
	const  FunctionTag* tag = nullptr;
	while (!tag)
	{
		serchName = shp_namespace->GetGlobalNameString() + left_.GetString();

		tag = c->GetFunctionTag(serchName);

		if (tag) {
			break;
		}

		shp_namespace = shp_namespace->GetParent();
		if (!shp_namespace) {
			break;
		}

	}
	return tag;
}
const FunctionTag* GetFunctionType(const Compiler* c, const std::string& str) {

	auto shp_namespace = c->GetCurrentNameSpace();
	std::string serchName;
	const  FunctionTag* tag = nullptr;
	while (!tag)
	{
		serchName = shp_namespace->GetGlobalNameString() + str;

		tag = c->GetFunctionTag(serchName);

		if (tag) {
			break;
		}

		shp_namespace = shp_namespace->GetParent();
		if (!shp_namespace) {
			break;
		}

	}
	return tag;
}

// 変数ノードを生成
Node_t Node::make_node(const int Op, const std::string& str, const Compiler* c)
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
Node_t Node::make_node(const int Op, Node_t left, const Compiler* c)
{
	if (Op == OP_METHOD) {
		return Node_t(new Node_Method(Op, left->GetLeft(), left->GetString()));
	}
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


Node_t Node::make_node(const int Op, Node_t left, const std::string arg_memberName,const Compiler* c)
{
	if (GetEnumType(c,*left)) {
		return  Node_t(new Node_enum( left, arg_memberName));
	}

	if (Op == OP_MEMBER) {
		return Node_t(new Node_Member(Op, left, arg_memberName));
	}
	else if (Op == OP_METHOD) {
		return Node_t(new Node_Method(Op, left, arg_memberName));
	}
	return Node_t();
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
	if (Op == OP_METHOD) {
		return  Node_t(new Node_Method(Op, left, right));
	}
	return Node_t(new Node_function(Op, left, right));
}



template <typename T>
bool SetDefaultOperator(const int op_, Compiler* c) {
	switch (op_) {
	case OP_ADD:
		c->OpAdd<T>();
		break;

	case OP_SUB:
		c->OpSub<T>();
		break;

	case OP_MUL:
		c->OpMul<T>();
		break;

	case OP_DIV:
		c->OpDiv<T>();
		break;


	default:
		return false;
		break;
	}
	return true;
}
template <typename T, typename U>
bool SetDeferentTypeMulOperator(const int op_, Compiler* c) {
	if (op_ == OP_MUL) {

		c->OpMul<T, U>();
		return true;
	}
	return false;
}
template <typename T, typename U>
bool SetDeferentTypeDivOperator(const int op_, Compiler* c) {
	if (op_ == OP_DIV) {

		c->OpDiv<T, U>();
		return true;
	}
	return false;
}

template <typename T>
bool SetModOperator(const int op_, Compiler* c) {
	if (op_ == OP_MOD) {
		c->OpMod<T>();
		return true;
	}
	return false;
}
bool SetLogicalOperator(const int op_, Compiler* c) {
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

	default:
		return false;
		break;
	}
	return true;
}



template <typename T>
bool SetDefaultAssignOperator(const int op_, Compiler* c) {
	switch (op_) {
	case OP_ADD_ASSIGN:
		c->OpAdd<T>();
		break;

	case OP_SUB_ASSIGN:
		c->OpSub<T>();
		break;

	case OP_MUL_ASSIGN:
		c->OpMul<T>();
		break;

	case OP_DIV_ASSIGN:
		c->OpDiv<T>();
		break;

	case OP_ASSIGN:

		break;

	default:
		return false;
		break;
	}
	return true;
}
template <typename T, typename U>
bool SetDeferentTypeMulAssignOperator(const int op_, Compiler* c) {
	if (op_ == OP_MUL_ASSIGN) {

		c->OpMul<T, U>();
		return true;
	}
	return false;
}
template <typename T, typename U>
bool SetDeferentTypeDivAssignOperator(const int op_, Compiler* c) {
	if (op_ == OP_DIV_ASSIGN) {

		c->OpDiv<T, U>();
		return true;
	}
	return false;
}

template <typename T>
bool SetModAssignOperator(const int op_, Compiler* c) {
	if (op_ == OP_MOD_ASSIGN) {
		c->OpMod<T>();
		return true;
	}
	return false;
}
bool SetLogicalAssignOperator(const int op_, Compiler* c) {
	switch (op_) {
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

	default:
		return false;
		break;
	}
	return true;
}



// ノードのpush処理
int Node::Push(Compiler* c) const{
	if (op_ >= OP_ASSIGN && op_ <= OP_RSHIFT_ASSIGN) {
		return Assign(c);
	}

	switch (op_) {
	case OP_NEG:
		if (left_->Push(c) == TYPE_STRING)
			c->error("文字列には到底許されない計算です。");
		c->OpNeg();
		return TYPE_INTEGER;

	case OP_INT:
		c->PushConstInt(number_);
		return TYPE_INTEGER;
	case OP_FLOAT:
		c->PushConstFloat(num_float);
		return TYPE_FLOAT;

	case OP_STRING:
		c->PushString(string_);
		return TYPE_STRING;

	case OP_FUNCTION:
		return Call(c, string_, nullptr);
	}

	int left_type = left_->Push(c);
	int right_type = right_->Push(c);

	//右辺若しくは左辺が未定義関数
	if (left_type <= -1 || right_type <= -1) {
		return -1;
	}

	// float計算ノードの処理
	if ((left_type == TYPE_FLOAT && right_type == TYPE_FLOAT ) || (left_type == TYPE_INTEGER && right_type == TYPE_FLOAT) || (left_type == TYPE_FLOAT && right_type == TYPE_INTEGER)) {
		if (!SetDefaultOperator<float>(op_, c) && (!SetModOperator<float>(op_, c)) && (!SetLogicalOperator(op_, c))) {
				c->error("内部エラー：処理できない計算ノードがありました。");
		}

		return TYPE_FLOAT;
	}

	// 整数計算ノードの処理
	if (left_type ==  TYPE_INTEGER && right_type==TYPE_INTEGER) {
		if (!SetDefaultOperator<int>(op_, c)&&  (!SetModOperator<int>(op_, c))&& (!SetLogicalOperator(op_, c))) {
			c->error("内部エラー：処理できない計算ノードがありました。");	
		}
		return TYPE_INTEGER;
	}

	//Vector2計算ノード
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_VOID + 1) {
		if (!SetDefaultOperator<ButiEngine::Vector2>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulOperator<ButiEngine::Vector2, float>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector2, float>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulOperator<ButiEngine::Vector2, int>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector2, int>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_FLOAT && right_type == TYPE_VOID + 1) {
		if (!SetDeferentTypeMulOperator<float, ButiEngine::Vector2>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_INTEGER && right_type == TYPE_VOID + 1) {

		if (!SetDeferentTypeMulOperator<int, ButiEngine::Vector2>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 1;
	}
	//Vector3計算ノード
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_VOID + 2) {
		if (!SetDefaultOperator<ButiEngine::Vector3>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulOperator<ButiEngine::Vector3, float>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector3, float>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulOperator<ButiEngine::Vector3, int>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector3, int>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_FLOAT && right_type == TYPE_VOID + 2) {
		if (!SetDeferentTypeMulOperator<float, ButiEngine::Vector3>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_INTEGER && right_type == TYPE_VOID + 2) {

		if (!SetDeferentTypeMulOperator<int, ButiEngine::Vector3>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 2;
	}
	//Vector4計算ノード
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_VOID + 3) {
		if (!SetDefaultOperator<ButiEngine::Vector4>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulOperator<ButiEngine::Vector4, float>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector4, float>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_VOID +3 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulOperator<ButiEngine::Vector4, int>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector4, int>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_FLOAT && right_type == TYPE_VOID + 3) {
		if (!SetDeferentTypeMulOperator<float, ButiEngine::Vector4>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_INTEGER && right_type == TYPE_VOID + 3) {

		if (!SetDeferentTypeMulOperator<int, ButiEngine::Vector4>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		return TYPE_VOID + 3;
	}

	if (c->GetType(left_type)->p_enumTag && right_type == TYPE_INTEGER) {
		if (op_ == OP_EQ)
		{
			c->OpEq();
		}
		return left_type;
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

int Node::Pop(Compiler* c) const{
	c->error("内部エラー：計算ノードをpopしています。");
	return TYPE_INTEGER;
}

//ノードの型チェック
int Node::GetType(Compiler* c)const {
	if (op_ >= OP_ASSIGN && op_ <= OP_RSHIFT_ASSIGN) {
		return -1;
	}

	switch (op_) {
	case OP_NEG:
		if (left_->GetType(c) == TYPE_STRING)
			c->error("文字列には許されない計算です。");
		return TYPE_INTEGER;

	case OP_INT:
		return TYPE_INTEGER;
	case OP_FLOAT:
		return TYPE_FLOAT;

	case OP_STRING:
		return TYPE_STRING;

	case OP_FUNCTION:
		return GetCallType(c, string_, nullptr);
	}
	if (op_ == OP_IDENTIFIER) {
		const ValueTag* valueTag = GetValueTag(c);
		if (valueTag)  {
			return valueTag->valueType;
		}
		auto funcTag = GetFunctionType(c, *this);

		if (!funcTag) {
			c->error(GetString() + "は未定義です");
			return 0;
		}

		return c->GetfunctionTypeIndex(funcTag->args_, funcTag->valueType);

	}
	int left_type = left_->GetType(c);
	int right_type = right_->GetType(c);

	//右辺若しくは左辺が未定義関数
	if (left_type <= -1 || right_type <= -1) {
		return -1;
	}

	// float計算ノードの処理
	if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
		return TYPE_FLOAT;
	}

	// 整数計算ノードの処理
	if (left_type == TYPE_INTEGER && right_type == TYPE_INTEGER) {
		return TYPE_INTEGER;
	}


	// 文字列計算ノードの処理
	switch (op_) {
	case OP_EQ:case OP_NE:case OP_GT:case OP_GE:case OP_LT:case OP_LE:
		return TYPE_INTEGER;

	case OP_ADD:
		break;

	default:
		c->error("文字列では計算できない式です。");
		break;
	}
	return TYPE_STRING;
}
const ValueTag* Node::GetValueTag(Compiler* c) const
{
		c->error("変数ノード以外から変数を受け取ろうとしています");
		return nullptr;
}
const ValueTag* Node::GetValueTag(const std::string& arg_name, Compiler* c) const
{
	std::string  valueName;
	NameSpace_t currentSerchNameSpace = c->GetCurrentNameSpace();
	const ValueTag* valueTag = nullptr;

	while (!valueTag)
	{
		if (currentSerchNameSpace) {
			valueName = currentSerchNameSpace->GetGlobalNameString() + arg_name;
		}
		else {
			valueName = arg_name;
		}

		valueTag = c->GetValueTag(valueName);
		if (currentSerchNameSpace) {
			currentSerchNameSpace = currentSerchNameSpace->GetParent();
		}
		else {
			break;
		}

	}
	if (!valueTag) {
		//c->error("変数 " + valueName + " は定義されていません。");
	}
	return valueTag;
}
int  Node_function::GetType(Compiler* c)const {
	return GetCallType(c, left_->GetString(), &node_list_->args_);
}
//ノードの関数呼び出し型チェック
int Node::GetCallType(Compiler* c, const std::string& name, const std::vector<Node_t>* args)const {

	std::vector<int> argTypes;
	if (args) {
		auto end = args->end();
		for (auto itr = args->begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(c));
		}
	}
	std::string  functionName;
	NameSpace_t currentSerchNameSpace = c->GetCurrentNameSpace();
	const FunctionTag* tag=nullptr;

	while (!tag)
	{
		if (currentSerchNameSpace) {
			functionName = currentSerchNameSpace->GetGlobalNameString() + name;
		}
		else {
			functionName = name;
		}

		tag = c->GetFunctionTag(functionName, argTypes, true);
		if (!tag) {
			tag = c->GetFunctionTag(functionName, argTypes, false);
		}
		if (currentSerchNameSpace) {
			currentSerchNameSpace = currentSerchNameSpace->GetParent();
		}
		else {
			break;
		}

	}

	if (tag == nullptr) {
		return -1;
	}
	return tag->valueType;
}

// 代入文
int Node::Assign(Compiler* c) const{
	int left_type = -1;
	//代入のみのパターンではないので左辺をpush
	if (op_ != OP_ASSIGN) {
		left_type = left_->Push(c)&~TYPE_REF;
	}
	else {
		left_type = left_->GetType(c) & ~TYPE_REF;
	}
	int right_type = right_->Push(c) & ~TYPE_REF;



	// float計算ノードの処理
	if ((left_type == TYPE_FLOAT && right_type == TYPE_FLOAT) || (left_type == TYPE_INTEGER && right_type == TYPE_FLOAT) || (left_type == TYPE_FLOAT && right_type == TYPE_INTEGER)) {
		if (!SetDefaultAssignOperator<float>(op_, c) && (!SetModAssignOperator<float>(op_, c)) && (!SetLogicalAssignOperator(op_, c))) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c);
		return 0;
	}

	// 整数計算ノードの処理
	if (left_type == TYPE_INTEGER && right_type == TYPE_INTEGER) {
		if (!SetDefaultAssignOperator<int>(op_, c) && (!SetModAssignOperator<int>(op_, c)) && (!SetLogicalAssignOperator(op_, c))) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c);
		return 0;
	}


	//Vector2計算ノード
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_VOID + 1) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector2>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c);
		return 0;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector2, float>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector2, float>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c); 
		return 0;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector2, int>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector2, int>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c);
		return 0;
	}
	//Vector3計算ノード
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_VOID + 2) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector3>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c);
		return 0;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector3, float>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector3, float>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c);
		return 0;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector3, int>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector3, int>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c);
		return 0;
	}
	//Vector4計算ノード
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_VOID + 3) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector4>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c);
		return 0;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector4, float>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector4, float>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c);
		return 0;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector4, int>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector4, int>(op_, c)) {
			c->error("内部エラー：処理できない計算ノードがありました。");
		}
		left_->Pop(c);
		return 0;
	}

	if (left_type == TYPE_STRING&& right_type == TYPE_STRING) {
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
		left_->Pop(c);

		return 0;
	}

	if (left_type == right_type) {
		//同じ型同士なので代入可能
		left_->Pop(c);
		return 0;
	}
	else if (right_->EnumType(c) == left_type) {
		left_->Pop(c);
		return 0;
	}
	else
	{
		c->error("代入出来ない変数の組み合わせです");
	}

	return -1;
}

const ValueTag* Node_value::GetValueTag(Compiler* c) const{

	if (op_ != OP_IDENTIFIER) {
		c->error("内部エラー：変数ノードに変数以外が登録されています。");
		return nullptr;
	}

	else {

		std::string  valueName;
		NameSpace_t currentSerchNameSpace = c->GetCurrentNameSpace();
		const ValueTag* valueTag = nullptr;

		while (!valueTag)
		{
			if (currentSerchNameSpace) {
				valueName = currentSerchNameSpace->GetGlobalNameString() + string_;
			}
			else {
				valueName = string_;
			}

			valueTag = c->GetValueTag(valueName);
			if (currentSerchNameSpace) {
				currentSerchNameSpace = currentSerchNameSpace->GetParent();
			}
			else {
				break;
			}

		}
		if (!valueTag) {
			//c->error("変数 " + valueName + " は定義されていません。");
		}
		return valueTag;
	}
}

// 変数ノードのpush
int Node_value::Push(Compiler* c) const{
	if (op_ != OP_IDENTIFIER) {
		c->error("内部エラー：変数ノードに変数以外が登録されています。");
	}
	else {
		const ValueTag* valueTag = GetValueTag(c);
		if(valueTag)
		{
			if (valueTag->global_) {		// グローバル変数
				if (left_) {		// 配列
					left_->Push(c);
					c->PushGlobalArrayRef(valueTag->address);
				}
				else {
					c->PushGlobalValueRef(valueTag->address);
				}
			}
			else {					// ローカル変数
				if (left_) {		// 配列
					left_->Push(c);
					c->PushLocalArrayRef(valueTag->address);
				}
				else {
					c->PushLocalRef(valueTag->address);
				}
			}
			return valueTag->valueType & ~TYPE_REF;
		}
		const auto funcTag = GetFunctionType(c, *this);

		if (!funcTag) {
			c->error(GetString() + "は未定義です");
			return -1;
		}
		else {
			c->OpPushFunctionAddress(funcTag->GetIndex());
			return c->GetfunctionTypeIndex(funcTag->args_, funcTag->valueType);
		}
	}
	return TYPE_INTEGER;
}

int Node_value::PushClone(Compiler* c) const{
	if (op_ != OP_IDENTIFIER) {
		c->error("内部エラー：変数ノードに変数以外が登録されています。");
	}
	else {
		const ValueTag* valueTag = GetValueTag(c);
		if(valueTag)
		{
			if (valueTag->global_) {		// グローバル変数
				if (left_) {		// 配列
					left_->Push(c);
					c->PushGlobalArray(valueTag->address);
				}
				else {
					c->PushGlobalValue(valueTag->address);
				}
			}
			else {					// ローカル変数
				if (left_) {		// 配列
					left_->Push(c);
					c->PushLocalArray(valueTag->address);
				}
				else {
					c->PushLocal(valueTag->address);
				}
			}
			return valueTag->valueType & ~TYPE_REF;
		}

		const auto funcTag = GetFunctionType(c, *this);

		if (!funcTag) {
			c->error(GetString() + "は未定義です");
			return -1;
		}
		else {
			c->OpPushFunctionAddress (funcTag->GetIndex());
			return c->GetfunctionTypeIndex(funcTag->args_, funcTag->valueType);
		}
	}
	return TYPE_INTEGER;
}

// 変数ノードのpop
int Node_value::Pop(Compiler* c) const{
	if (op_ != OP_IDENTIFIER) {
		c->error("内部エラー：変数ノードに変数以外が登録されています。");
	}
	else {
		const ValueTag* valueTag = GetValueTag(c);
		if(valueTag)
		{
			if (valueTag->global_) {		// グローバル変数
				if (left_) {		// 配列
					left_->Push(c);
					c->PopArray(valueTag->address);
				}
				else {
					c->PopValue(valueTag->address);
				}
			}
			else {					// ローカル変数
				if (left_) {		// 配列
					left_->Push(c);
					c->PopLocalArray(valueTag->address);
				}
				else {
					c->PopLocal(valueTag->address);
				}
			}
			return valueTag->valueType & ~TYPE_REF;
		}
		const auto funcTag = GetFunctionType(c, *this);

		if (!funcTag) {
			c->error(GetString() + "は未定義です");
			return -1;
		}
		else {
			c->error(GetString() + "は定数です");
			return -1;
		}
	}
	return TYPE_INTEGER;
}

// 関数呼び出し
struct set_arg {
	Compiler* comp_;
	const std::vector<int>* argTypes_;
	mutable int index_;
	set_arg(Compiler* comp, const FunctionTag* func) : comp_(comp), argTypes_(&func->args_), index_(0) {}
	set_arg(Compiler* comp, const std::vector<int>* arg_argTypes) : comp_(comp), argTypes_(arg_argTypes), index_(0){}

	void operator()(Node_t node) const
	{
		int type = (*argTypes_)[index_++];
		if ((type & TYPE_REF) != 0) {		// 参照
			if (node->Op() != OP_IDENTIFIER) {
				comp_->error("参照型引数に、変数以外は指定できません。");
			}
			else {
				std::string  valueName;
				NameSpace_t currentSerchNameSpace = comp_->GetCurrentNameSpace();
				const ValueTag* tag = node->GetValueTag(comp_);
				if (tag == nullptr) {
					comp_->error("変数 " + node->GetString() + " は定義されていません。");
				}
				else {
					if (!TypeCheck(tag->valueType ,type) ){
						comp_->error("引数の型が合いません。");
					}

					if (tag->global_) {
						if (node->GetLeft()) {
							node->GetLeft()->Push(comp_);
							comp_->PushGlobalArrayRef(tag->address);
						}
						else {
							comp_->PushGlobalValueRef(tag->address);
						}
					}
					else {
						if (node->GetLeft()) {
							node->GetLeft()->Push(comp_);
							comp_->PushLocalArrayRef(tag->address);
						}
						else {
							comp_->PushLocalRef(tag->address);
						}
					}
				}
			}
		}
		else {
			if (!TypeCheck( node->PushClone(comp_), type)) {
				comp_->error("引数の型が合いません。");
			}
		}
	}
};

// 関数呼び出し
int Node::Call(Compiler* c, const std::string& name, const std::vector<Node_t>* args) const
{
	std::string  functionName;
	NameSpace_t currentSerchNameSpace = c->GetCurrentNameSpace();

	std::vector<int> argTypes;
	if (args) {
		auto end = args->end();
		for (auto itr = args->begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(c));
		}
	}

	int argSize = argTypes.size();
	const FunctionTag* functionTag=nullptr;
	
	while (!functionTag)
	{
		if (currentSerchNameSpace) {
			functionName = currentSerchNameSpace->GetGlobalNameString() + name;
		}
		else {
			functionName = name;
		}
		
		functionTag = c->GetFunctionTag(functionName, argTypes, true);
		if (!functionTag) {
			functionTag = c->GetFunctionTag(functionName, argTypes, false);
		}
		if (currentSerchNameSpace) {
			currentSerchNameSpace = currentSerchNameSpace->GetParent();
		}
		else {
			break;
		}

	}


	if (functionTag ) {
		// 引数をpush
		if (args && functionTag->ArgSize() == argSize) {
			std::for_each(args->begin(), args->end(), set_arg(c, functionTag));
		}

		// 引数の数をpush
		c->PushConstInt(argSize);

		if (functionTag->IsSystem()) {
			c->OpSysCall(functionTag->GetIndex());		// 組み込み関数
		}
		else {
			c->OpCall(functionTag->GetIndex());			// スクリプト上の関数
		}

		return functionTag->valueType;
	}
	//関数型変数からの呼び出し
	auto valueTag = GetValueTag(name, c);
	if (valueTag) {
		auto valueType=c->GetType(valueTag->valueType);
		if (valueType->isFunctionObject) {
			// 引数をpush
			if (args && valueType->GetFunctionObjectArgSize() == argSize) {
				auto valueArgTypes = valueType->GetFunctionObjectArgment();
				std::for_each(args->begin(), args->end(), set_arg(c,&valueArgTypes ));
			}

			// 引数の数をpush
			c->PushConstInt(argSize);
			if (valueTag->global_) {		// グローバル変数

				c->PushGlobalValue(valueTag->address);
			}
			else {		

				c->PushLocal(valueTag->address);
			}
			c->OpCallByVariable();
			return valueType->GetFunctionObjectReturnType();
		}
	}


	std::string message = "";
	if (argSize) {
		for (int i = 0; i < argSize; i++) {
			message += c->GetTypeName(argTypes[i]) + " ";
		}
		message += "を引数にとる";
	}
	message += "関数" + functionName + "は未宣言です";
	c->error(message);
	return -1;
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

	case RETURN_STATE: {
		auto ret = Statement_t(new Statement_return());

		return ret;
	}


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
	return vec_node->Assign(c);
	
}

int Statement_assign::ReAnalyze(Compiler* c)
{
	return vec_node->Assign(c);
	
}

// 関数呼び出し文
int ccall_statement::Analyze(Compiler* c) 
{
	int type = vec_node->Push(c);

	if (type == -1) {

		return -1;
	}

	if (type != TYPE_VOID)
		c->OpPop();			// 戻り値を捨てるためのpop

	return 0;
}

void ccall_statement::ReAnalyze(Compiler* c) 
{
	int type = vec_node->Push(c);

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
	if (vec_node->Op() != OP_INT)
		c->error("case 文には定数のみ指定できます。");
	vec_node->Push(c);
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
		if (vec_node != 0) {
			c->error("void関数に戻り値が設定されています");
		}
		c->OpReturn();
	}
	else {
		if (vec_node == 0) {
			c->error("関数の戻り値がありません");
		}
		else {
			int node_type = vec_node->Push(c);		// 戻り値をpush

			if (!CanTypeCast( node_type ,c->GetFunctionType())) {
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
	vec_node->Push(c);
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
	if(vec_node[0])
		vec_node[0]->Push(c);
	c->SetLabel(label1);
	vec_node[1]->Push(c);
	c->OpJmpNC(label2);
	statement_->Analyze(c);
	if (vec_node[2])
		vec_node[2]->Push(c);
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
	vec_node->Push(c);
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
		vec_node->Push(c);

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


		for (auto itr = decl_.begin(), endItr = decl_.end(); itr != endItr; itr++) {
			(*itr)->Define(c);
		}
	}
	if (!decl_.empty())
		c->AllocStack();	// スタックフレーム確保

	{

		

		for (auto itr = state_.begin(), endItr = state_.end(); itr != endItr; itr++) {
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
	if (isFunction) {		// 関数
		c->FunctionDefine(valueType, name_, args);
	}
	else {
		c->ValueDefine(valueType, vec_node,accessType);

		auto type = c->GetType(valueType&~TYPE_REF);
		int size = vec_node.size();
		if (valueType & TYPE_REF) {
			if (type->isSystem) {
				for (int i = 0; i < size; i++) {
					c->OpAllocStack_Ref(valueType);
				}
			}
			else if (type->p_enumTag) {
				for (int i = 0; i < size; i++) {
					c->OpAllocStack_Ref_EnumType(valueType);
				}
			}
			else if (type->isFunctionObject) {
				for (int i = 0; i < size; i++) {
					c->OpAllocStack_Ref_FunctionType(valueType);
				}
			}
			else {
				for (int i = 0; i < size; i++) {
					c->OpAllocStack_Ref_ScriptType((valueType & ~TYPE_REF) - c->GetSystemTypeSize());
				}
			}
		}
		else {
			if (type->isSystem) {
				for (int i = 0; i < size; i++) {
					c->OpAllocStack(valueType);
				}
			}
			else if (type->p_enumTag) {
				for (int i = 0; i < size; i++) {
					c->OpAllocStackEnumType(valueType);
				}
			}
			else if (type->isFunctionObject) {
				for (int i = 0; i < size; i++) {
					c->OpAllocStackFunctionType(valueType);
				}
			}
			else {
				for (int i = 0; i < size; i++) {
					c->OpAllocStack_ScriptType(valueType - c->GetSystemTypeSize());
				}
			}
		}
	}
	return 0;
}


void Declaration::Define(Compiler* c)
{
	if (isFunction) {		// 関数
		c->FunctionDefine(valueType, name_, args);
	}
	else {
		c->ValueDefine(valueType, vec_node,accessType);
	}
}
int Node_Member::Push(Compiler* c) const
{
	if (op_ != OP_MEMBER) {
		c->error("内部エラー：メンバ変数ノードにメンバ変数以外が登録されています。");
	}
	else {
		left_->Push(c);
		auto typeTag = c->GetType(left_->GetType(c) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(string_).access != AccessModifier::Public && c->GetCurrentThisType() != typeTag) {
			c->error(typeTag->typeName+"の" + string_ + "にアクセス出来ません");
		}

		c->PushMemberRef(typeTag->map_memberValue.at(string_).index);
		return   typeTag->map_memberValue.at(string_).type & ~TYPE_REF;
	}
	return -1;
}
int Node_Member::PushClone(Compiler* c) const
{
	if (op_ != OP_MEMBER) {
		c->error("内部エラー：メンバ変数ノードにメンバ変数以外が登録されています。");
	}
	else {
		left_->Push(c);
		auto typeTag = c->GetType(left_->GetType(c) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(string_).access != AccessModifier::Public && c->GetCurrentThisType() != typeTag) {
			c->error(typeTag->typeName + "の" + string_ + "にアクセス出来ません");
		}
		c->PushMember(typeTag->map_memberValue.at(string_).index);
		return   typeTag->map_memberValue.at(string_).type & ~TYPE_REF;

	}
	return -1;
}
int Node_Member::Pop(Compiler* c) const
{
	if (op_ != OP_MEMBER) {
		c->error("内部エラー：メンバ変数ノードにメンバ変数以外が登録されています。");
	}
	else {

		//型
		auto typeTag = c->GetType(left_->GetType(c));
		if (typeTag->map_memberValue.at(string_).access != AccessModifier::Public && c->GetCurrentThisType() != typeTag) {
			c->error(typeTag->typeName + "の" + string_ + "にアクセス出来ません");
		}
		
		left_->Push(c);
		auto type = typeTag->map_memberValue.at(string_).type;
		if (type & TYPE_REF) {
			c->PopMemberRef(typeTag->map_memberValue.at(string_).index);
		}
		else {
			c->PopMember(typeTag->map_memberValue.at(string_).index);
		}

		return   typeTag->map_memberValue.at(string_).type & ~TYPE_REF;
		
	}
	return TYPE_INTEGER;
}
int Node_Member::GetType(Compiler* c) const
{
	if (op_ != OP_MEMBER) {
		c->error("内部エラー：メンバ変数ノードにメンバ変数以外が登録されています。");
	}
	else {
		//変数のメンバ変数
		if (left_->Op() == OP_IDENTIFIER|| left_->Op() == OP_MEMBER) {
			{

				//型
				auto typeTag = c->GetType(left_->GetType(c));
				if (!typeTag->map_memberValue.count(string_)) {
					c->error("構文エラー："+typeTag->typeName+"にメンバ変数"+string_+"は存在しません");
					return -1;
				}
				return typeTag->map_memberValue.at(string_).type;
			}
		}
	}
}
int Node_Method::Push(Compiler* c) const
{
	if (op_ != OP_METHOD) {
		c->error("内部エラー：メンバ関数ノードにメンバ関数以外が登録されています。");
	}

	std::vector<int> argTypes;
	if (node_list_) {
		auto end = node_list_->args_.end();
		for (auto itr = node_list_->args_.begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(c));
		}
	}

	int argSize = argTypes.size();
	const TypeTag* typeTag = nullptr;
	const FunctionTag* methodTag = nullptr;


	typeTag = c->GetType(left_->GetType(c) & ~TYPE_REF);
	methodTag = typeTag->methods.Find(string_, argTypes);
	if (methodTag->accessType != AccessModifier::Public&&c->GetCurrentThisType()!=typeTag) {

		c->error(typeTag->typeName + "　の" + methodTag->name + "()はアクセス出来ません");
	}

	if (methodTag == nullptr) {
		std::string message = "";
		if (argSize) {
			for (int i = 0; i < argSize; i++) {
				message += c->GetTypeName(argTypes[i]) + " ";
			}
			message += "を引数にとる";
		}
		message += "関数" + string_ + "は未宣言です";
		c->error(message);
		return -1;
	}

	// 引数をpush
	if (node_list_&& methodTag->ArgSize() == argSize) {
		std::for_each(node_list_->args_.begin(), node_list_->args_.end(), set_arg(c, methodTag));
	}


	left_->Push(c);


	if (methodTag->IsSystem()) {
		// 引数の数をpush
		c->PushConstInt(argSize);
		c->OpSysMethodCall(methodTag->GetIndex());		// 組み込みメソッド
	}
	else {
		// 引数の数+thisをpush
		c->PushConstInt(argSize+1);
		c->OpCall(methodTag->GetIndex());			// スクリプト上のメソッド
	}


	return methodTag->valueType;
}
int Node_Method::Pop(Compiler* c) const
{
	c->error("内部エラー：メンバ関数ノードをpop");
	return TYPE_INTEGER;
}
int Node_Method::GetType(Compiler* c) const
{
	if (op_ != OP_METHOD) {
		c->error("内部エラー：メンバ関数ノードにメンバ関数以外が登録されています。");
	}

	std::vector<int> argTypes;

	if (node_list_) {
		auto end = node_list_->args_.end();
		for (auto itr = node_list_->args_.begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(c));
		}
	}


	const TypeTag* typeTag = c->GetType(left_->GetType(c) & ~TYPE_REF);
	const FunctionTag* tag = typeTag->methods.Find(string_, argTypes);

	return tag->valueType;
}
int Node_enum::Push(Compiler* c) const
{

	auto enumType = GetEnumType(c,*left_);
	if (enumType == nullptr) {

		c->error("列挙型　" + left_->GetString() + "は未定義です");
		return -1;
	}
	if (!enumType->ExistenceIdentifers(string_)) {

		c->error("列挙型　" + left_->GetString()+"."+string_ + "は未定義です");
		return -1;
	}
	c->PushConstInt( enumType->GetValue(string_));

	return TYPE_INTEGER;
}
int Node_enum::Pop(Compiler* c) const
{
	c->error("内部エラー：列挙型ノードをpop");
	return -1;
}
int Node_enum::GetType(Compiler* c) const
{
	return TYPE_INTEGER;
}
int Node_enum::EnumType(Compiler* c) const
{
	auto type=c->GetType(left_->GetString());
	if (!type) {
		c->error("列挙型　" + left_->GetString() + "." + string_ + "は未定義です");
		return 0;
	}
	return type->typeIndex;
}

int Node_FunctionObject::Push(Compiler* c) const
{
	auto funcTag = GetFunctionType(c, *this);

	if (!funcTag) {
		c->error(GetString() + "は未定義です");
		return -1;
	}
	c->PushConstInt(funcTag->GetIndex());
	return c->GetfunctionTypeIndex(funcTag->args_, funcTag->valueType);
}
int Node_FunctionObject::Pop(Compiler* c) const
{
	c->error("内部エラー：関数型ノードをpop");
	return -1;
}
int Node_FunctionObject::GetType(Compiler* c) const
{
	auto funcTag = GetFunctionType(c, *this);
	
	if (!funcTag) {
		c->error(GetString() + "は未定義です");
		return 0;
	}

	return c->GetfunctionTypeIndex(funcTag->args_, funcTag->valueType);
}
void Enum::SetIdentifer(const std::string& arg_name)
{
	map_identifer.emplace(arg_name, map_identifer.size());
}
void Enum::SetIdentifer(const std::string& arg_name, const int value)
{
	map_identifer.emplace(arg_name, value);
}
int Enum::Analyze(Compiler* c)
{
	c->RegistEnumType(typeName);
	auto tag = c->GetEnumTag(c->GetCurrentNameSpace()->GetGlobalNameString()+ typeName);
	auto end = map_identifer.end();
	for (auto itr = map_identifer.begin(); itr != end; itr++) {
		tag->SetValue(itr->first, itr->second);
	}
	return 0;
}
int Class::Analyze(Compiler* c)
{
	c->AnalyzeScriptType(name_, map_values);
	auto typeTag = c->GetType(name_);
	auto methodTable = &typeTag->methods;
	auto end = vec_methods.end();
	for (auto itr = vec_methods.begin(); itr != end; itr++) {
		(*itr)->Regist(c, methodTable);
	}
	vec_methods.clear();
	return 0;
}
int Class::AnalyzeMethod(Compiler* c)
{
	auto typeTag = c->GetType(name_);
	auto methodTable = &typeTag->methods;
	auto end = vec_methods.end();
	c->PushCurrentThisType(typeTag);
	for (auto itr = vec_methods.begin(); itr != end; itr++) {
		(*itr)->Analyze(c,methodTable);
	}
	c->PopCurrentThisType();
	vec_methods.clear();
	return 0;
}
int Class::Regist(Compiler* c)
{
	c->RegistScriptType(name_);
	return 0;
}
void Class::RegistMethod(Function_t method, Compiler* c)
{
	vec_methods.push_back(method);
	
}
void Class::SetValue(const std::string& arg_name, const int arg_type, const AccessModifier accessType)
{
	std::pair<int, AccessModifier> v = { arg_type,accessType };
	map_values.emplace(arg_name,v);
}

// 関数の解析
int Function::Analyze(Compiler* c, FunctionTable* funcTable)
{

	c->AddFunction(valueType, name_, args_, block_, accessType, funcTable);

	return 0;
}

int Function::Regist(Compiler* c, FunctionTable* funcTable)
{
	c->RegistFunction(valueType, name_, args_, block_, accessType, funcTable);
	return 0;
}

Ramda::Ramda(const int arg_type,const std::vector<ArgDefine>& arg_args)
{
	valueType = arg_type;
	args_ = arg_args;
}
int Ramda::Analyze(Compiler* c, FunctionTable* funcTable)
{
	auto typeTag = c->GetType(valueType);
	auto ramdaCount = c->GetRamdaCount();
	c->AddRamda (typeTag->GetFunctionObjectReturnType(), args_, block_, funcTable);
	return ramdaCount;
}
int Ramda::Regist(Compiler* c, FunctionTable* funcTable)
{
	auto typeTag = c->GetType(valueType);
	c->RegistRamda(  typeTag->GetFunctionObjectReturnType(),  args_,funcTable);
	return 0;
}
}