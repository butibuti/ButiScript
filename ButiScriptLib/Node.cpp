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
const char* thisPtrName = "this";
const std::int32_t argmentAddressStart=-4;
bool CanTypeCast(const std::int32_t arg_left, const std::int32_t arg_right) {
	if (arg_left == TYPE_STRING || arg_right == TYPE_STRING) {
		if (arg_left != arg_right) {
			return false;
		}
	}
	return true;
}


const EnumTag* GetEnumType(const Compiler* arg_compiler,const Node& arg_leftNode) {

	auto vlp_namespace = arg_compiler->GetCurrentNameSpace();
	std::string searchName;
	const  EnumTag* enumType = nullptr;
	while (!enumType)
	{
		searchName = vlp_namespace->GetGlobalNameString() + arg_leftNode.GetString();

		enumType = arg_compiler->GetEnumTag(searchName);

		if (enumType) {
			break;
		}

		vlp_namespace = vlp_namespace->GetParent();
		if (!vlp_namespace) {
			break;
		}

	}
	return enumType;
}
const FunctionTag* GetFunctionType(const Compiler* arg_compiler, const std::string& arg_str) {
	return arg_compiler->GetFunctionTag(arg_str);
}

const FunctionTag* GetFunctionType(const Compiler* arg_compiler, const Node& leftNode) {
	return GetFunctionType(arg_compiler, leftNode.GetString());
}
// 変数ノードを生成
Node_t Node::make_node(const std::int32_t arg_op, const std::string& arg_str, const Compiler* arg_compiler)
{
	if (arg_op == OP_IDENTIFIER)
		return ButiEngine::make_value<Node_value>(arg_str);

	if (arg_op == OP_STRING) {
		std::uint64_t pos = arg_str.rfind('\"');
		if (pos != std::string::npos)
			return ButiEngine::make_value<Node>(arg_op, arg_str.substr(0, pos));
	}
	return ButiEngine::make_value<Node>(arg_op, arg_str);
}
// 単項演算子のノードを生成
Node_t Node::make_node(const std::int32_t arg_op, Node_t arg_left, const Compiler* arg_compiler)
{
	if (arg_op == OP_METHOD) {
		return nullptr;
	}
	switch (arg_op) {
	case OP_NEG:
		if (arg_left->op == OP_INT) {			// 定数演算を計算する
			arg_left->num_int = -arg_left->num_int;
			return arg_left;
		}
		if (arg_left->op == OP_FLOAT) {			// 定数演算を計算する
			arg_left->num_float = -arg_left->num_float;
			return arg_left;
		}
		break;
	case OP_NOT:
		if (arg_left->op == OP_INT) {			// 定数演算を計算する
			arg_left->num_int = !arg_left->num_int;
			return arg_left;
		}
		if (arg_left->op == OP_FLOAT) {			// 定数演算を計算する
			arg_left->num_float = !arg_left->num_float;
			return arg_left;
		}
		break;
	}
	return ButiEngine::make_value<Node>(arg_op, arg_left);
}


Node_t Node::make_node(const std::int32_t arg_op, Node_t arg_left, const std::string arg_memberName,const Compiler* arg_compiler)
{
	if (GetEnumType(arg_compiler,*arg_left)) {
		return  ButiEngine::make_value<Node_enum>( arg_left, arg_memberName);
	}

	if (arg_op == OP_MEMBER) {
		return ButiEngine::make_value<Node_Member>(arg_op, arg_left, arg_memberName);
	}
	else if (arg_op == OP_METHOD) {
		return nullptr;
	}
	return nullptr;
}




// 二項演算子のノードを生成
Node_t Node::make_node(const std::int32_t arg_op, Node_t arg_left, Node_t arg_right)
{
	// 配列ノードは、leftノードのleft_メンバに加える
	if (arg_op == OP_ARRAY) {
		arg_left->leftNode = arg_right;
		return arg_left;
	}
	auto leftOp = arg_left->op;
	auto rightOp = arg_right->op;

	// 定数演算を計算する
	if (leftOp == OP_INT && rightOp == OP_INT) {
		switch (arg_op) {
		case OP_LOGAND:
			arg_left->num_int = (arg_left->num_int && arg_right->num_int) ? 1 : 0;
			break;

		case OP_LOGOR:
			arg_left->num_int = (arg_left->num_int || arg_right->num_int) ? 1 : 0;
			break;

		case OP_EQ:
			arg_left->num_int = (arg_left->num_int == arg_right->num_int) ? 1 : 0;
			break;

		case OP_NE:
			arg_left->num_int = (arg_left->num_int != arg_right->num_int) ? 1 : 0;
			break;

		case OP_GT:
			arg_left->num_int = (arg_left->num_int > arg_right->num_int) ? 1 : 0;
			break;

		case OP_GE:
			arg_left->num_int = (arg_left->num_int >= arg_right->num_int) ? 1 : 0;
			break;

		case OP_LT:
			arg_left->num_int = (arg_left->num_int < arg_right->num_int) ? 1 : 0;
			break;

		case OP_LE:
			arg_left->num_int = (arg_left->num_int <= arg_right->num_int) ? 1 : 0;
			break;

		case OP_AND:
			arg_left->num_int &= arg_right->num_int;
			break;

		case OP_OR:
			arg_left->num_int |= arg_right->num_int;
			break;

		case OP_LSHIFT:
			arg_left->num_int <<= arg_right->num_int;
			break;

		case OP_RSHIFT:
			arg_left->num_int >>= arg_right->num_int;
			break;

		case OP_SUB:
			arg_left->num_int -= arg_right->num_int;
			break;

		case OP_ADD:
			arg_left->num_int += arg_right->num_int;
			break;

		case OP_MUL:
			arg_left->num_int *= arg_right->num_int;
			break;

		case OP_DIV:
			if (arg_right->num_int == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				arg_left->num_int /= arg_right->num_int;
			}
			break;

		case OP_MOD:
			if (arg_right->num_int == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				arg_left->num_int %= arg_right->num_int;
			}
			break;

		default:
			return ButiEngine::make_value<Node>(arg_op, arg_left, arg_right);
		}
		return arg_left;
	}

	// 定数浮動小数演算を計算する

	if (leftOp == OP_FLOAT && rightOp == OP_FLOAT) {
		switch (arg_op) {
		case OP_LOGAND:
			arg_left->num_float =(float)( (arg_left->num_float && arg_right->num_float) ? 1 : 0);
			break;

		case OP_LOGOR:
			arg_left->num_float = (float)((arg_left->num_float || arg_right->num_float) ? 1 : 0);
			break;

		case OP_EQ:
			arg_left->num_float = (float)((arg_left->num_float == arg_right->num_float) ? 1 : 0);
			break;

		case OP_NE:
			arg_left->num_float = (float)((arg_left->num_float != arg_right->num_float) ? 1 : 0);
			break;

		case OP_GT:
			arg_left->num_float = (float)((arg_left->num_float > arg_right->num_float) ? 1 : 0);
			break;

		case OP_GE:
			arg_left->num_float = (float)((arg_left->num_float >= arg_right->num_float) ? 1 : 0);
			break;

		case OP_LT:
			arg_left->num_float = (float)((arg_left->num_float < arg_right->num_float) ? 1 : 0);
			break;

		case OP_LE:
			arg_left->num_float = (float)((arg_left->num_float <= arg_right->num_float) ? 1 : 0);
			break;

		case OP_AND:
			arg_left->num_float =(float)( (std::int32_t)arg_left->num_float & (std::int32_t)arg_right->num_float);
			break;

		case OP_OR:
			arg_left->num_float = (float)((std::int32_t)arg_left->num_float | (std::int32_t)arg_right->num_float);
			break;

		case OP_LSHIFT:
			arg_left->num_float = (float)((std::int32_t)arg_left->num_float << (std::int32_t)arg_right->num_float);
			break;

		case OP_RSHIFT:
			arg_left->num_float = (float)((std::int32_t)arg_left->num_float >> (std::int32_t)arg_right->num_float);
			break;

		case OP_SUB:
			arg_left->num_float -= arg_right->num_float;
			break;

		case OP_ADD:
			arg_left->num_float += arg_right->num_float;
			break;

		case OP_MUL:
			arg_left->num_float *= arg_right->num_float;
			break;

		case OP_DIV:
			if (arg_right->num_float == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				arg_left->num_float /= arg_right->num_float;
			}
			break;

		case OP_MOD:
			if (arg_right->num_float == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				arg_left->num_float = (float)((std::int32_t)arg_left->num_float % (std::int32_t)arg_right->num_float);
			}
			break;

		default:
			return ButiEngine::make_value<Node>(arg_op, arg_left, arg_right);
		}
		return arg_left;
	}


	if (leftOp == OP_INT&& rightOp == OP_FLOAT) {
		switch (arg_op) {
		case OP_LOGAND:
			arg_left->num_int = (arg_left->num_int && (std::int32_t)arg_right->num_float) ? 1 : 0;
			break;

		case OP_LOGOR:
			arg_left->num_int = (arg_left->num_int || (std::int32_t)arg_right->num_float) ? 1 : 0;
			break;

		case OP_EQ:
			arg_left->num_int = (arg_left->num_int == (std::int32_t)arg_right->num_float) ? 1 : 0;
			break;

		case OP_NE:
			arg_left->num_int = (arg_left->num_int != (std::int32_t)arg_right->num_float) ? 1 : 0;
			break;

		case OP_GT:
			arg_left->num_int = (arg_left->num_int > (std::int32_t)arg_right->num_float) ? 1 : 0;
			break;

		case OP_GE:
			arg_left->num_int = (arg_left->num_int >= (std::int32_t)arg_right->num_float) ? 1 : 0;
			break;

		case OP_LT:
			arg_left->num_int = (arg_left->num_int < (std::int32_t)arg_right->num_float) ? 1 : 0;
			break;

		case OP_LE:
			arg_left->num_int = (arg_left->num_int <= (std::int32_t)arg_right->num_float) ? 1 : 0;
			break;

		case OP_AND:
			arg_left->num_int = (std::int32_t)arg_left->num_int & (std::int32_t)(std::int32_t)arg_right->num_float;
			break;

		case OP_OR:
			arg_left->num_int = (std::int32_t)arg_left->num_int | (std::int32_t)(std::int32_t)arg_right->num_float;
			break;

		case OP_LSHIFT:
			arg_left->num_int = (std::int32_t)arg_left->num_int << (std::int32_t)(std::int32_t)arg_right->num_float;
			break;

		case OP_RSHIFT:
			arg_left->num_int = (std::int32_t)arg_left->num_int >> (std::int32_t)(std::int32_t)arg_right->num_float;
			break;

		case OP_SUB:
			arg_left->num_int -= (std::int32_t)arg_right->num_float;
			break;

		case OP_ADD:
			arg_left->num_int += (std::int32_t)arg_right->num_float;
			break;

		case OP_MUL:
			arg_left->num_int *= (std::int32_t)arg_right->num_float;
			break;

		case OP_DIV:
			if ((std::int32_t)arg_right->num_float == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				arg_left->num_int /= (std::int32_t)arg_right->num_float;
			}
			break;

		case OP_MOD:
			if ((std::int32_t)arg_right->num_float == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				arg_left->num_int = (std::int32_t)arg_left->num_int % (std::int32_t)arg_right->num_float;
			}
			break;

		default:
			return ButiEngine::make_value<Node>(arg_op, arg_left, arg_right);
		}
		return arg_left;
	}

	if (leftOp == OP_FLOAT && rightOp == OP_INT) {
		switch (arg_op) {
		case OP_LOGAND:
			arg_left->num_float =(float)( (arg_left->num_float && arg_right->num_int) ? 1 : 0);
			break;

		case OP_LOGOR:
			arg_left->num_float = (float)((arg_left->num_float || arg_right->num_int) ? 1 : 0);
			break;

		case OP_EQ:
			arg_left->num_float = (float)((arg_left->num_float == arg_right->num_int) ? 1 : 0);
			break;

		case OP_NE:
			arg_left->num_float = (float)((arg_left->num_float != arg_right->num_int) ? 1 : 0);
			break;

		case OP_GT:
			arg_left->num_float = (float)((arg_left->num_float > arg_right->num_int) ? 1 : 0);
			break;

		case OP_GE:
			arg_left->num_float = (float)((arg_left->num_float >= arg_right->num_int) ? 1 : 0);
			break;

		case OP_LT:
			arg_left->num_float = (float)((arg_left->num_float < arg_right->num_int) ? 1 : 0);
			break;

		case OP_LE:
			arg_left->num_float = (float)((arg_left->num_float <= arg_right->num_int) ? 1 : 0);
			break;

		case OP_AND:
			arg_left->num_float = (float)((std::int32_t)arg_left->num_float & (std::int32_t)arg_right->num_int);
			break;

		case OP_OR:
			arg_left->num_float = (float)((std::int32_t)arg_left->num_float | (std::int32_t)arg_right->num_int);
			break;

		case OP_LSHIFT:
			arg_left->num_float = (float)((std::int32_t)arg_left->num_float << (std::int32_t)arg_right->num_int);
			break;

		case OP_RSHIFT:
			arg_left->num_float = (float)((std::int32_t)arg_left->num_float >> (std::int32_t)arg_right->num_int);
			break;

		case OP_SUB:
			arg_left->num_float -= arg_right->num_int;
			break;

		case OP_ADD:
			arg_left->num_float += arg_right->num_int;
			break;

		case OP_MUL:
			arg_left->num_float *= arg_right->num_int;
			break;

		case OP_DIV:
			if (arg_right->num_int == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				arg_left->num_float /= arg_right->num_int;
			}
			break;

		case OP_MOD:
			if (arg_right->num_int == 0) {
				std::cerr << "定数計算を0で除算しました。" << std::endl;
			}
			else {
				arg_left->num_float = (float)((std::int32_t)arg_left->num_float % (std::int32_t)arg_right->num_int);
			}
			break;

		default:
			return ButiEngine::make_value<Node>(arg_op, arg_left, arg_right);
		}
		return arg_left;
	}

	// 文字列同士の定数計算
	if (leftOp == OP_STRING && rightOp == OP_STRING) {
		if (arg_op == OP_ADD) {
			arg_left->strData += arg_right->strData;
			return arg_left;
		}

		std::int32_t Value = 0;
		switch (arg_op) {
		case OP_EQ:
			if (arg_left->strData == arg_right->strData)
				Value = 1;
			break;

		case OP_NE:
			if (arg_left->strData != arg_right->strData)
				Value = 1;
			break;

		case OP_GT:
			if (arg_left->strData > arg_right->strData)
				Value = 1;
			break;

		case OP_GE:
			if (arg_left->strData >= arg_right->strData)
				Value = 1;
			break;

		case OP_LT:
			if (arg_left->strData < arg_right->strData)
				Value = 1;
			break;

		case OP_LE:
			if (arg_left->strData <= arg_right->strData)
				Value = 1;
			break;

		default:
			std::cerr << "文字列同士ではできない計算です。" << std::endl;
			break;
		}
		return ButiEngine::make_value<Node>(OP_INT, Value);
	}
	return ButiEngine::make_value<Node>(arg_op, arg_left, arg_right);
}

Node_t Node::make_node(const std::int32_t arg_op, Node_t arg_left, Node_t arg_right, const Compiler* arg_compiler)
{
	assert(arg_op == OP_METHOD);
	return	arg_right->CreateMethod(arg_left);
}

// 引数有り関数ノードの生成
Node_t Node::make_node(const std::int32_t arg_op, Node_t arg_left, NodeList_t arg_list)
{
	auto left = ButiEngine::dynamic_value_ptr_cast<Node_function>(arg_left);
	if (left) {
		left->SetArgmentList(arg_list);
		return left;
	}
	return ButiEngine::make_value<Node_function>(arg_op, arg_left, arg_list);
}
//テンプレート引数有り関数ノードの生成
Node_t Node::make_node(const std::int32_t arg_op, Node_t arg_left, const ButiEngine::List<std::int32_t>& arg_templateTypes)
{
	return ButiEngine::make_value<Node_function>(arg_op, arg_left, arg_templateTypes);
}



template <typename LeftType,typename RightType,typename ReturnType>
bool SetDefaultOperator(const std::int32_t arg_op, Compiler* arg_compiler) {
	switch (arg_op) {
	case OP_ADD:
		arg_compiler->OpAdd<LeftType,RightType,ReturnType>();
		break;

	case OP_SUB:
		arg_compiler->OpSub<LeftType, RightType, ReturnType>();
		break;

	case OP_MUL:
		arg_compiler->OpMul<LeftType, RightType, ReturnType>();
		break;

	case OP_DIV:
		arg_compiler->OpDiv<LeftType, RightType, ReturnType>();
		break;


	default:
		return false;
		break;
	}
	return true;
}

template <typename LeftType, typename RightType>
bool SetModOperator(const std::int32_t arg_op, Compiler* arg_compiler) {
	if (arg_op == OP_MOD) {
		arg_compiler->OpMod<LeftType, RightType>();
		return true;
	}
	return false;
}
template <typename LeftType, typename RightType, typename ReturnType>
bool SetMulOperator(const std::int32_t arg_op, Compiler* arg_compiler) {
	if (arg_op == OP_MUL) {
		arg_compiler->OpMul<LeftType, RightType, ReturnType>();
		return true;
	}
	return false;
}
template <typename LeftType, typename RightType, typename ReturnType>
bool SetDivOperator(const std::int32_t arg_op, Compiler* arg_compiler) {
	if (arg_op == OP_DIV) {
		arg_compiler->OpDiv<LeftType, RightType, ReturnType>();
		return true;
	}
	return false;
}
template<typename LeftType,typename RightType>
bool SetLogicalOperator(const std::int32_t arg_op, Compiler* arg_compiler) {
	switch (arg_op) {
	case OP_LOGAND:
		arg_compiler->OpLogAnd();
		break;

	case OP_LOGOR:
		arg_compiler->OpLogOr();
		break;

	case OP_EQ:
		arg_compiler->OpEq<LeftType,RightType>();
		break;

	case OP_NE:
		arg_compiler->OpNe<LeftType, RightType>();
		break;

	case OP_GT:
		arg_compiler->OpGt<LeftType, RightType>();
		break;

	case OP_GE:
		arg_compiler->OpGe<LeftType, RightType>();
		break;

	case OP_LT:
		arg_compiler->OpLt<LeftType, RightType>();
		break;

	case OP_LE:
		arg_compiler->OpLe<LeftType, RightType>();
		break;

	case OP_AND:
		arg_compiler->OpAnd();
		break;

	case OP_OR:
		arg_compiler->OpOr();
		break;

	case OP_LSHIFT:
		arg_compiler->OpLeftShift();
		break;

	case OP_RSHIFT:
		arg_compiler->OpRightShift();
		break;

	default:
		return false;
		break;
	}
	return true;
}


template <typename T>
bool SetModAssignOperator(const std::int32_t arg_op, Compiler* arg_compiler) {
	if (arg_op == OP_MOD_ASSIGN) {
		arg_compiler->OpMod<T>();
		return true;
	}
	return false;
}

std::int32_t SystemTypeOperatorCheck(const std::int32_t arg_op,const std::int32_t arg_leftType, const std::int32_t arg_rightType, Compiler* arg_p_compiler) {

	// float計算ノード
	if ((arg_leftType == TYPE_FLOAT && arg_rightType == TYPE_FLOAT)) {
		if (!SetDefaultOperator<float, float, float>(arg_op, arg_p_compiler) && (!SetModOperator<float, float>(arg_op, arg_p_compiler)) && (!SetLogicalOperator<float, float>(arg_op, arg_p_compiler))) {
			
		}
		return TYPE_FLOAT;
	}

	if ((arg_leftType == TYPE_FLOAT && arg_rightType == TYPE_INTEGER)) {
		if (!SetDefaultOperator<float, std::int32_t, float>(arg_op, arg_p_compiler) && (!SetModOperator<float, std::int32_t>(arg_op, arg_p_compiler)) && (!SetLogicalOperator<float, std::int32_t>(arg_op, arg_p_compiler))) {
			
		}
		return TYPE_FLOAT;
	}

	if ((arg_leftType == TYPE_INTEGER && arg_rightType == TYPE_FLOAT)) {
		if (!SetDefaultOperator<std::int32_t, float, float>(arg_op, arg_p_compiler) && (!SetModOperator<std::int32_t, float>(arg_op, arg_p_compiler)) && (!SetLogicalOperator<std::int32_t, float>(arg_op, arg_p_compiler))) {
			
		}
		return TYPE_FLOAT;
	}


	// 整数計算ノードの処理
	if (arg_leftType == TYPE_INTEGER && arg_rightType == TYPE_INTEGER) {
		if (!SetDefaultOperator<std::int32_t, std::int32_t, std::int32_t>(arg_op, arg_p_compiler) && (!SetModOperator<std::int32_t, std::int32_t>(arg_op, arg_p_compiler)) && (!SetLogicalOperator<std::int32_t, std::int32_t>(arg_op, arg_p_compiler))) {
			
		}
		return TYPE_INTEGER;
	}

	//Vector2計算ノード
	if (arg_leftType == TYPE_VECTOR2 && arg_rightType == TYPE_VECTOR2) {
		if (!SetDefaultOperator<ButiEngine::Vector2, ButiEngine::Vector2, ButiEngine::Vector2>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR2;
	}
	if (arg_leftType == TYPE_VECTOR2 && arg_rightType == TYPE_FLOAT) {
		if (!SetMulOperator<ButiEngine::Vector2, float, ButiEngine::Vector2>(arg_op, arg_p_compiler) && !!SetDivOperator<ButiEngine::Vector2, float, ButiEngine::Vector2>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR2;
	}
	if (arg_leftType == TYPE_VECTOR2 && arg_rightType == TYPE_INTEGER) {
		if (!SetMulOperator<ButiEngine::Vector2, std::int32_t, ButiEngine::Vector2>(arg_op, arg_p_compiler) && !!SetDivOperator<ButiEngine::Vector2, std::int32_t, ButiEngine::Vector2>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR2;
	}
	if (arg_leftType == TYPE_FLOAT && arg_rightType == TYPE_VECTOR2) {
		if (!SetMulOperator<float, ButiEngine::Vector2, ButiEngine::Vector2>(arg_op, arg_p_compiler) && !SetDivOperator<float, ButiEngine::Vector2, ButiEngine::Vector2>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR2;
	}
	if (arg_leftType == TYPE_INTEGER && arg_rightType == TYPE_VECTOR2) {
		if (!SetMulOperator<std::int32_t, ButiEngine::Vector2, ButiEngine::Vector2>(arg_op, arg_p_compiler) && !SetDivOperator<std::int32_t, ButiEngine::Vector2, ButiEngine::Vector2>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR2;
	}
	//Vector3計算ノード
	if (arg_leftType == TYPE_VECTOR3 && arg_rightType == TYPE_VECTOR3) {
		if (!SetDefaultOperator<ButiEngine::Vector3, ButiEngine::Vector3, ButiEngine::Vector3>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR3;
	}
	if (arg_leftType == TYPE_VECTOR3 && arg_rightType == TYPE_FLOAT) {
		if (!SetMulOperator<ButiEngine::Vector3, float, ButiEngine::Vector3>(arg_op, arg_p_compiler) && !!SetDivOperator<ButiEngine::Vector3, float, ButiEngine::Vector3>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR3;
	}
	if (arg_leftType == TYPE_VECTOR3 && arg_rightType == TYPE_INTEGER) {
		if (!SetMulOperator<ButiEngine::Vector3, std::int32_t, ButiEngine::Vector3>(arg_op, arg_p_compiler) && !!SetDivOperator<ButiEngine::Vector3, std::int32_t, ButiEngine::Vector3>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR3;
	}
	if (arg_leftType == TYPE_FLOAT && arg_rightType == TYPE_VECTOR3) {
		if (!SetMulOperator<float, ButiEngine::Vector3, ButiEngine::Vector3>(arg_op, arg_p_compiler) && !SetDivOperator<float, ButiEngine::Vector3, ButiEngine::Vector3>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR3;
	}
	if (arg_leftType == TYPE_INTEGER && arg_rightType == TYPE_VECTOR3) {
		if (!SetMulOperator<std::int32_t, ButiEngine::Vector3, ButiEngine::Vector3>(arg_op, arg_p_compiler) && !SetDivOperator<std::int32_t, ButiEngine::Vector3, ButiEngine::Vector3>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR3;
	}
	//Vector4計算ノード
	if (arg_leftType == TYPE_VECTOR4 && arg_rightType == TYPE_VECTOR4) {
		if (!SetDefaultOperator<ButiEngine::Vector4, ButiEngine::Vector4, ButiEngine::Vector4>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR4;
	}
	if (arg_leftType == TYPE_VECTOR4 && arg_rightType == TYPE_FLOAT) {
		if (!SetMulOperator<ButiEngine::Vector4, float, ButiEngine::Vector4>(arg_op, arg_p_compiler) && !!SetDivOperator<ButiEngine::Vector4, float, ButiEngine::Vector4>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR4;
	}
	if (arg_leftType == TYPE_VECTOR4 && arg_rightType == TYPE_INTEGER) {
		if (!SetMulOperator<ButiEngine::Vector4, std::int32_t, ButiEngine::Vector4>(arg_op, arg_p_compiler) && !!SetDivOperator<ButiEngine::Vector4, std::int32_t, ButiEngine::Vector4>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR4;
	}
	if (arg_leftType == TYPE_FLOAT && arg_rightType == TYPE_VECTOR4) {
		if (!SetMulOperator<float, ButiEngine::Vector4, ButiEngine::Vector4>(arg_op, arg_p_compiler) && !SetDivOperator<float, ButiEngine::Vector4, ButiEngine::Vector4>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR4;
	}
	if (arg_leftType == TYPE_INTEGER && arg_rightType == TYPE_VECTOR4) {
		if (!SetMulOperator<std::int32_t, ButiEngine::Vector4, ButiEngine::Vector4>(arg_op, arg_p_compiler) && !SetDivOperator<std::int32_t, ButiEngine::Vector4, ButiEngine::Vector4>(arg_op, arg_p_compiler)) {
			
		}
		return TYPE_VECTOR4;
	}
	return -1;
}
// ノードのpush処理
std::int32_t Node::Push(Compiler* arg_compiler) const{
	if (op >= OP_ASSIGN && op <= OP_RSHIFT_ASSIGN) {
		return Assign(arg_compiler);
	}

	switch (op) {
	case OP_NEG:
	{
		auto typeIndex_raw = leftNode->Push(arg_compiler);
		auto typeIndex = typeIndex_raw & ~TYPE_REF;
		if (typeIndex == TYPE_STRING) {
			arg_compiler->error("文字列には到底許されない計算です。");
		}
		else if (typeIndex == TYPE_INTEGER) {
			arg_compiler->OpNeg<std::int32_t>();
		}
		else if (typeIndex == TYPE_FLOAT) {
			arg_compiler->OpNeg<float>();
		}
		else if (typeIndex == TYPE_VECTOR2) {
			arg_compiler->OpNeg<ButiEngine::Vector2>();
		}
		else if (typeIndex == TYPE_VECTOR3) {
			arg_compiler->OpNeg<ButiEngine::Vector3>();
		}
		else if (typeIndex == TYPE_VECTOR4) {
			arg_compiler->OpNeg<ButiEngine::Vector4>();
		}
		else if (typeIndex == TYPE_MATRIX4X4) {
			arg_compiler->OpNeg<ButiEngine::Matrix4x4>();
		}
		return typeIndex_raw;
	}
	case OP_NOT:
	{
		auto typeIndex_raw = leftNode->Push(arg_compiler);
		auto typeIndex = typeIndex_raw & ~TYPE_REF;
		if (typeIndex == TYPE_STRING) {
			arg_compiler->error("文字列には到底許されない計算です。");
		}
		else if (typeIndex == TYPE_INTEGER) {
			arg_compiler->OpNot<std::int32_t>();
		}
		else if (typeIndex== TYPE_FLOAT) {
			arg_compiler->OpNot<float>();
		}		
		return typeIndex_raw;
	}
	case OP_INCREMENT: 
	{
		auto typeIndex_raw = leftNode->Push(arg_compiler);
		auto typeIndex = typeIndex_raw & ~TYPE_REF;
		if (typeIndex == TYPE_STRING) {
			arg_compiler->error("文字列には到底許されない計算です。");
		}
		else if (typeIndex == TYPE_INTEGER) {
			arg_compiler->OpIncrement<std::int32_t>();
		}
		else if (typeIndex == TYPE_FLOAT) {
			arg_compiler->OpIncrement<float>();
		}
		return typeIndex_raw;
	}
	case OP_DECREMENT:
	{
		auto typeIndex_raw = leftNode->Push(arg_compiler);
		auto typeIndex = typeIndex_raw & ~TYPE_REF;
		if (typeIndex == TYPE_STRING) {
			arg_compiler->error("文字列には到底許されない計算です。");
		}
		else if (typeIndex == TYPE_INTEGER) {
			arg_compiler->OpDecrement<std::int32_t>();
		}
		else if (typeIndex == TYPE_FLOAT) {
			arg_compiler->OpDecrement<float>();
		}
		return typeIndex_raw;
	}
	case OP_INT:
		arg_compiler->PushConstInt(num_int);
		return TYPE_INTEGER;
	case OP_FLOAT:
		arg_compiler->PushConstFloat(num_float);
		return TYPE_FLOAT;

	case OP_STRING:
		arg_compiler->PushString(strData);
		return TYPE_STRING;

	case OP_FUNCTION:
		return Call(arg_compiler, strData, nullptr, {});
	}

	std::int32_t left_type = leftNode->Push(arg_compiler);
	std::int32_t right_type = rightNode->Push(arg_compiler);

	//右辺若しくは左辺が未定義関数
	if (left_type <= -1 || right_type <= -1) {
		return -1;
	}
	std::int32_t systemTypeOpRet = SystemTypeOperatorCheck(op, left_type, right_type, arg_compiler);
	if (systemTypeOpRet != -1) {
		return systemTypeOpRet;
	}

	if (arg_compiler->GetType(left_type)->p_enumTag && right_type == TYPE_INTEGER) {
		if (op == OP_EQ)
		{
			arg_compiler->OpEq<std::int32_t, std::int32_t>();
		}
		else if (op == OP_NE) {
			arg_compiler->OpNe<std::int32_t, std::int32_t>();
		}
		return left_type;
	}

	if (left_type == right_type && op == OP_EQ) {
		arg_compiler->OpEq<Type_ScriptClass,Type_ScriptClass>();
		return TYPE_INTEGER;
	}

	if (left_type == TYPE_VOID) {
		if (op == OP_EQ && (arg_compiler->GetType(right_type)->isShared || rightNode->GetType(arg_compiler)&TYPE_REF)) {
			arg_compiler->OpEq<Type_ScriptClass, Type_ScriptClass>();
		}
		else if (op == OP_NE) {
			arg_compiler->OpNe<Type_ScriptClass, Type_ScriptClass>();
		}
		return TYPE_INTEGER;
	}
	if (right_type == TYPE_VOID) {
		if (op == OP_EQ && (arg_compiler->GetType(left_type)->isShared || leftNode->GetType(arg_compiler) & TYPE_REF)) {
			arg_compiler->OpEq<Type_ScriptClass, Type_ScriptClass>();
		}
		else if (op == OP_NE) {
			arg_compiler->OpNe<Type_ScriptClass, Type_ScriptClass>();
		}
		return TYPE_INTEGER;
	}

	// 文字列計算ノードの処理
	switch (op) {
	case OP_EQ:
		arg_compiler->OpStrEq();
		return TYPE_INTEGER;

	case OP_NE:
		arg_compiler->OpStrNe();
		return TYPE_INTEGER;

	case OP_GT:
		arg_compiler->OpStrGt();
		return TYPE_INTEGER;

	case OP_GE:
		arg_compiler->OpStrGe();
		return TYPE_INTEGER;

	case OP_LT:
		arg_compiler->OpStrLt();
		return TYPE_INTEGER;

	case OP_LE:
		arg_compiler->OpStrLe();
		return TYPE_INTEGER;

	case OP_ADD:
		arg_compiler->OpStrAdd();
		break;

	default:
		arg_compiler->error("文字列では計算できない式です。");
		break;
	}
	return TYPE_STRING;


	arg_compiler->error("内部エラー：処理できない計算ノードがありました。");
	return -1;
}

// ノードのpop
// 計算ノードはpopできない

std::int32_t Node::Pop(Compiler* arg_compiler) const{
	arg_compiler->error("内部エラー：計算ノードをpopしています。");
	return TYPE_INTEGER;
}


//ノードの型チェック
std::int32_t Node::GetType(Compiler* arg_compiler)const {
	if (op >= OP_ASSIGN && op <= OP_RSHIFT_ASSIGN) {
		return -1;
	}

	switch (op) {

	case OP_INCREMENT:
	case OP_DECREMENT:
	case OP_NEG:
	case OP_NOT:
	{

		auto type = leftNode->GetType(arg_compiler);
		if (type == TYPE_STRING)
			arg_compiler->error("文字列には許されない計算です。");
		return type;
	}
	case OP_INT:
		return TYPE_INTEGER;
	case OP_FLOAT:
		return TYPE_FLOAT;

	case OP_STRING:
		return TYPE_STRING;

	case OP_FUNCTION:
		return GetCallType(arg_compiler, strData, nullptr, {});
	}
	if (op == OP_IDENTIFIER) {
		const ValueTag* valueTag = GetValueTag(arg_compiler);
		if (valueTag)  {
			return valueTag->valueType;
		}
		auto funcTag = GetFunctionType(arg_compiler, *this);

		if (!funcTag) {
			arg_compiler->error(GetString() + "は未定義です");
			return 0;
		}

		return arg_compiler->GetfunctionTypeIndex(funcTag->list_args, funcTag->valueType);

	}
	std::int32_t left_type = leftNode->GetType(arg_compiler);
	std::int32_t right_type = rightNode->GetType(arg_compiler);

	//右辺若しくは左辺が未定義関数
	if (left_type <= -1 || right_type <= -1) {
		return -1;
	}

	//Vector2計算ノードの処理
	if (left_type == TYPE_VECTOR2 && right_type == TYPE_VECTOR2) {
		return TYPE_VECTOR2;
	}
	if ((left_type == TYPE_VECTOR2 && right_type == TYPE_FLOAT)|| (left_type == TYPE_FLOAT && right_type == TYPE_VECTOR2)) {
		return TYPE_VECTOR2;
	}

	//Vector3計算ノードの処理
	if (left_type == TYPE_VECTOR3 && right_type == TYPE_VECTOR3) {
		return TYPE_VECTOR3;
	}
	if ((left_type == TYPE_VECTOR3 && right_type == TYPE_FLOAT) || (left_type == TYPE_FLOAT && right_type == TYPE_VECTOR3)) {
		return TYPE_VECTOR3;
	}
	//Vector4計算ノードの処理
	if (left_type == TYPE_VECTOR4 && right_type == TYPE_VECTOR4) {
		return TYPE_VECTOR4;
	}
	if ((left_type == TYPE_VECTOR4 && right_type == TYPE_FLOAT) || (left_type == TYPE_FLOAT && right_type == TYPE_VECTOR4)) {
		return TYPE_VECTOR4;
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
	switch (op) {
	case OP_EQ:case OP_NE:case OP_GT:case OP_GE:case OP_LT:case OP_LE:
		return TYPE_INTEGER;

	case OP_ADD:
		break;

	default:
		arg_compiler->error("文字列では計算できない式です。");
		break;
	}
	return TYPE_STRING;
}
const ValueTag* Node::GetValueTag(Compiler* arg_compiler) const
{
		arg_compiler->error("変数ノード以外から変数を受け取ろうとしています");
		return nullptr;
}
const ValueTag* Node::GetValueTag(const std::string& arg_name, Compiler* arg_compiler) const
{
	std::string  valueName;
	NameSpace_t currentsearchNameSpace = arg_compiler->GetCurrentNameSpace();
	const ValueTag* valueTag = nullptr;

	while (!valueTag)
	{
		if (currentsearchNameSpace) {
			valueName = currentsearchNameSpace->GetGlobalNameString() + arg_name;
		}
		else {
			valueName = arg_name;
		}

		valueTag = arg_compiler->GetValueTag(valueName);
		if (currentsearchNameSpace) {
			currentsearchNameSpace = currentsearchNameSpace->GetParent();
		}
		else {
			break;
		}

	}
	if (!valueTag) {
		//arg_compiler->error("変数 " + valueName + " は定義されていません。");
	}
	return valueTag;
}
std::int32_t  Node_function::GetType(Compiler* arg_compiler)const {
	return GetCallType(arg_compiler, leftNode->GetString(), &nodeList->list_args,list_templateTypes);
}
void Node_function::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	leftNode->LambdaCapture(arg_captureList, arg_compiler);
	nodeList->LambdaCapture(arg_captureList, arg_compiler);
}
Node_t Node_function::CreateMethod(Node_t arg_node)
{
	return ButiEngine::make_value<Node_Method>(OP_METHOD,leftNode,arg_node, nodeList, list_templateTypes);
}
//ノードの関数呼び出し型チェック
std::int32_t Node::GetCallType(Compiler* arg_compiler, const std::string& arg_name, const ButiEngine::List<Node_t>* arg_list_argNode, const ButiEngine::List<std::int32_t>& arg_list_temps)const {

	ButiEngine::List<std::int32_t> argTypes;
	if (arg_list_argNode) {
		auto end = arg_list_argNode->end();
		for (auto itr = arg_list_argNode->begin(); itr != end; itr++) {
			argTypes.Add((*itr)->GetType(arg_compiler));
		}
	}
	NameSpace_t currentsearchNameSpace = arg_compiler->GetCurrentNameSpace();
	const FunctionTag* tag= arg_compiler->GetFunctionTag(arg_name, argTypes, arg_list_temps, true);
	if (!tag) {
		tag= arg_compiler->GetFunctionTag(arg_name, argTypes,arg_list_temps, false);
	}

	if (tag == nullptr) {
		return -1;
	}
	return tag->valueType;
}

// 代入文
std::int32_t Node::Assign(Compiler* arg_compiler) const{
	std::int32_t left_type_raw;
	//代入のみのパターンではないので左辺をpush
	if (op != OP_ASSIGN) {
		left_type_raw = leftNode->Push(arg_compiler);
	}
	else {
		left_type_raw = leftNode->GetType(arg_compiler) ;
	}
	std::int32_t right_type_raw = rightNode->Push(arg_compiler) ,right_type=right_type_raw&~TYPE_REF,
		left_type = left_type_raw & ~TYPE_REF;
	if (op != OP_ASSIGN) {
		right_type = SystemTypeOperatorCheck(op - OP_ASSIGN + OP_DECREMENT, left_type, right_type, arg_compiler);
		if (right_type == -1 ) {
			arg_compiler->error("内部エラー：処理できない計算ノードがありました。");
			return -1;
		}
	}


	if (left_type == TYPE_STRING&& right_type == TYPE_STRING) {
		switch (op) {
		case OP_ADD_ASSIGN:
			arg_compiler->OpStrAdd();
			break;

		case OP_ASSIGN:
			break;

		default:
			arg_compiler->error("文字列では許されない計算です。");
			break;
		}
		leftNode->Pop(arg_compiler);

		return 0;
	}

	if (left_type == right_type) {
		//同じ型同士なので代入可能
		leftNode->Pop(arg_compiler);
		return 0;
	}
	else if (rightNode->EnumType(arg_compiler) == left_type) {
		leftNode->Pop(arg_compiler);
		return 0;
	}
	else if (right_type == TYPE_VOID && (arg_compiler->GetType(left_type)->isShared || left_type_raw & TYPE_REF)) {
		leftNode->Pop(arg_compiler);
		return 0;
	}
	else if ((arg_compiler->GetType(right_type)->p_enumTag && (left_type == TYPE_INTEGER || arg_compiler->GetType(left_type)->p_enumTag))
		|| (arg_compiler->GetType(left_type)->p_enumTag && (right_type== TYPE_INTEGER || arg_compiler->GetType(right_type)->p_enumTag))	) {
		leftNode->Pop(arg_compiler);
		return 0;
	}
	else if (right_type == TYPE_FLOAT && left_type == TYPE_INTEGER)
	{
		arg_compiler->OpCast<float, std::int32_t>();
		leftNode->Pop(arg_compiler);
		return 0;
	}
	else if (left_type== TYPE_FLOAT && right_type== TYPE_INTEGER)
	{
		arg_compiler->OpCast<std::int32_t, float>();
		leftNode->Pop(arg_compiler);
		return 0;
	}
	else
	{
		arg_compiler->error("代入出来ない変数の組み合わせです");
	}

	return -1;
}

std::int32_t Node_Null::Push(Compiler* arg_compiler) const
{
	arg_compiler->PushNull();
	return TYPE_VOID;
}
const ValueTag* Node_value::GetValueTag(Compiler* arg_compiler) const{

	if (op != OP_IDENTIFIER) {
		arg_compiler->error("内部エラー：変数ノードに変数以外が登録されています。");
		return nullptr;
	}

	else {

		std::string  valueName;
		NameSpace_t currentsearchNameSpace = arg_compiler->GetCurrentNameSpace();
		const ValueTag* valueTag = nullptr;

		while (!valueTag)
		{
			if (currentsearchNameSpace) {
				valueName = currentsearchNameSpace->GetGlobalNameString() + strData;
			}
			else {
				valueName = strData;
			}

			valueTag = arg_compiler->GetValueTag(valueName);
			if (currentsearchNameSpace) {
				currentsearchNameSpace = currentsearchNameSpace->GetParent();
			}
			else {
				break;
			}

		}
		if (!valueTag) {
			//arg_compiler->error("変数 " + valueName + " は定義されていません。");
		}
		return valueTag;
	}
}

// 変数ノードのpush
std::int32_t Node_value::Push(Compiler* arg_compiler) const{
	if (op != OP_IDENTIFIER) {
		arg_compiler->error("内部エラー：変数ノードに変数以外が登録されています。");
	}
	else {
		const ValueTag* valueTag = GetValueTag(arg_compiler);
		if(valueTag)
		{
			if (valueTag->isGlobal) {		// グローバル変数
				if (leftNode) {		// 配列
					leftNode->Push(arg_compiler);
					arg_compiler->PushGlobalArrayRef(valueTag->GetAddress());
				}
				else {
					arg_compiler->PushGlobalValueRef(valueTag->GetAddress());
				}
			}
			else {					// ローカル変数
				if (leftNode) {		// 配列
					leftNode->Push(arg_compiler);
					arg_compiler->PushLocalArrayRef(valueTag->GetAddress());
				}
				else {
					arg_compiler->PushLocalRef(valueTag->GetAddress());
				}
			}
			return valueTag->valueType & ~TYPE_REF;
		}
		const auto funcTag = GetFunctionType(arg_compiler, *this);

		if (!funcTag) {
			arg_compiler->error(GetString() + "は未定義です");
			return -1;
		}
		else {
			if (funcTag->isLambda) {
				arg_compiler->PushConstInt(funcTag->valueType);
				
				for (auto itr = funcTag->list_captureList.begin(),end=funcTag->list_captureList.end();itr!=end; itr++) {
					arg_compiler->PushConstInt(*itr);
				}

				arg_compiler->PushConstInt(funcTag->list_captureList.GetSize());

				arg_compiler->OpPushLambda(funcTag->GetIndex());
			}
			else {
				arg_compiler->OpPushFunctionAddress(funcTag->GetIndex());
			}
			return arg_compiler->GetfunctionTypeIndex(funcTag->list_args, funcTag->valueType);
		}
	}
	return TYPE_INTEGER;
}

std::int32_t Node_value::PushClone(Compiler* arg_compiler) const{
	if (op != OP_IDENTIFIER) {
		arg_compiler->error("内部エラー：変数ノードに変数以外が登録されています。");
	}
	else {
		const ValueTag* valueTag = GetValueTag(arg_compiler);
		if(valueTag)
		{
			if (valueTag->isGlobal) {		// グローバル変数
				if (leftNode) {		// 配列
					leftNode->Push(arg_compiler);
					arg_compiler->PushGlobalArray(valueTag->GetAddress());
				}
				else {
					arg_compiler->PushGlobalValue(valueTag->GetAddress());
				}
			}
			else {					// ローカル変数
				if (leftNode) {		// 配列
					leftNode->Push(arg_compiler);
					arg_compiler->PushLocalArray(valueTag->GetAddress());
				}
				else {
					arg_compiler->PushLocal(valueTag->GetAddress());
				}
			}
			return valueTag->valueType & ~TYPE_REF;
		}

		const auto funcTag = GetFunctionType(arg_compiler, *this);

		if (!funcTag) {
			arg_compiler->error(GetString() + "は未定義です");
			return -1;
		}
		else {
			arg_compiler->OpPushFunctionAddress (funcTag->GetIndex());
			return arg_compiler->GetfunctionTypeIndex(funcTag->list_args, funcTag->valueType);
		}
	}
	return TYPE_INTEGER;
}

// 変数ノードのpop
std::int32_t Node_value::Pop(Compiler* arg_compiler) const{
	if (op != OP_IDENTIFIER) {
		arg_compiler->error("内部エラー：変数ノードに変数以外が登録されています。");
	}
	else {
		const ValueTag* valueTag = GetValueTag(arg_compiler);
		if(valueTag)
		{
			if (valueTag->isGlobal) {		// グローバル変数
				if (leftNode) {		// 配列
					leftNode->Push(arg_compiler);
					arg_compiler->PopArray(valueTag->GetAddress());
				}
				else {
					arg_compiler->PopValue(valueTag->GetAddress());
				}
			}
			else {					// ローカル変数
				if (leftNode) {		// 配列
					leftNode->Push(arg_compiler);
					arg_compiler->PopLocalArray(valueTag->GetAddress());
				}
				else {
					arg_compiler->PopLocal(valueTag->GetAddress());
				}
			}
			return valueTag->valueType & ~TYPE_REF;
		}
		const auto funcTag = GetFunctionType(arg_compiler, *this);

		if (!funcTag) {
			arg_compiler->error(GetString() + "は未定義です");
			return -1;
		}
		else {
			arg_compiler->error(GetString() + "は定数です");
			return -1;
		}
	}
	return TYPE_INTEGER;
}

void Node_value::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	const ValueTag* valueTag = GetValueTag(arg_compiler);
	if (valueTag && !valueTag->isGlobal) {
		arg_captureList.emplace(GetString(), valueTag);
	}
}

// 関数呼び出し
struct set_arg {
	Compiler* p_compiler;
	const ButiEngine::List<std::int32_t>* argTypes_;
	mutable std::int32_t index_;
	set_arg(Compiler* arg_p_compiler, const FunctionTag* arg_function) : p_compiler(arg_p_compiler), argTypes_(&arg_function->list_args), index_(0) {}
	set_arg(Compiler* arg_p_compiler, const ButiEngine::List<std::int32_t>* arg_argTypes) : p_compiler(arg_p_compiler), argTypes_(arg_argTypes), index_(0){}

	void operator()(Node_t arg_node) const
	{
		std::int32_t type = (*argTypes_)[index_++];
		if ((type & TYPE_REF) != 0) {		// 参照
			if (arg_node->Op() != OP_IDENTIFIER) {
				p_compiler->error("参照型引数に、変数以外は指定できません。");
			}
			else {
				std::string  valueName;
				NameSpace_t currentsearchNameSpace = p_compiler->GetCurrentNameSpace();
				const ValueTag* tag = arg_node->GetValueTag(p_compiler);
				if (tag == nullptr) {
					p_compiler->error("変数 " + arg_node->GetString() + " は定義されていません。");
				}
				else {
					if (!TypeCheck(tag->valueType ,type,&p_compiler->GetTypeTable()) ){
						p_compiler->error("引数の型が合いません。");
					}

					if (tag->isGlobal) {
						if (arg_node->GetLeft()) {
							arg_node->GetLeft()->Push(p_compiler);
							p_compiler->PushGlobalArrayRef(tag->GetAddress());
						}
						else {
							p_compiler->PushGlobalValueRef(tag->GetAddress());
						}
					}
					else {
						if (arg_node->GetLeft()) {
							arg_node->GetLeft()->Push(p_compiler);
							p_compiler->PushLocalArrayRef(tag->GetAddress());
						}
						else {
							p_compiler->PushLocalRef(tag->GetAddress());
						}
					}
				}
			}
		}
		else {
			if (!TypeCheck( arg_node->PushClone(p_compiler), type,& p_compiler->GetTypeTable())) {
				p_compiler->error("引数の型が合いません。");
			}
		}
	}
};

// 関数呼び出し
std::int32_t Node::Call(Compiler* arg_compiler, const std::string& arg_name, const ButiEngine::List<Node_t>* arg_list_argNodes,  const ButiEngine::List<std::int32_t>& arg_list_temps) const
{

	ButiEngine::List<std::int32_t> argTypes;
	if (arg_list_argNodes) {
		auto end = arg_list_argNodes->end();
		for (auto itr = arg_list_argNodes->begin(); itr != end; itr++) {
			argTypes.Add((*itr)->GetType(arg_compiler));
		}
	}

	std::int32_t argSize = argTypes.GetSize();
	const FunctionTag* functionTag = arg_compiler->GetFunctionTag(arg_name, argTypes, arg_list_temps, true);
	if (!functionTag) {
		functionTag = arg_compiler->GetFunctionTag(arg_name, argTypes, arg_list_temps, false);
	}


	if (functionTag ) {
		// 引数をpush
		if (arg_list_argNodes && functionTag->ArgSize() == argSize) {
			std::for_each(arg_list_argNodes->begin(), arg_list_argNodes->end(), set_arg(arg_compiler, functionTag));
		}

		// 引数の数をpush
		arg_compiler->PushConstInt(argSize);

		if (functionTag->IsSystem()) {
			arg_compiler->OpSysCall(functionTag->GetIndex());		// 組み込み関数
		}
		else {
			arg_compiler->OpCall(functionTag->GetIndex());			// スクリプト上の関数
		}

		return functionTag->valueType;
	}
	//関数型変数からの呼び出し
	auto valueTag = GetValueTag(arg_name, arg_compiler);
	if (valueTag) {
		auto valueType=arg_compiler->GetType(valueTag->valueType);
		if (valueType->IsFunctionObjectType()) {
			// 引数をpush
			if (arg_list_argNodes && valueType->GetFunctionObjectArgSize() == argSize) {
				auto valueArgTypes = valueType->GetFunctionObjectArgment();
				std::for_each(arg_list_argNodes->begin(), arg_list_argNodes->end(), set_arg(arg_compiler,&valueArgTypes ));
			}

			// 引数の数をpush
			arg_compiler->PushConstInt(argSize);
			if (valueTag->isGlobal) {		// グローバル変数

				arg_compiler->PushGlobalValueRef(valueTag->GetAddress());
			}
			else {		

				arg_compiler->PushLocalRef(valueTag->GetAddress());
			}
			arg_compiler->OpCallByVariable();
			return valueType->GetFunctionObjectReturnType();
		}
	}


	std::string message = "";
	if (argSize) {
		for (std::int32_t i = 0; i < argSize; i++) {
			message += arg_compiler->GetTypeName(argTypes[i]) + " ";
		}
		message += "を引数にとる";
	}
	message += "関数" + arg_name + "は未宣言です";
	arg_compiler->error(message);
	return -1;
}

Node_t Node::CreateMethod(Node_t arg_node)
{
	static ButiEngine::List<std::int32_t> empty;
	return ButiEngine::make_value<Node_Method>(OP_METHOD, value_from_this(),arg_node ,nullptr, empty);
}

void Node::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler) const
{
	if (leftNode) {
		leftNode->LambdaCapture(arg_captureList,arg_compiler);
	}
	if (rightNode) {
		rightNode->LambdaCapture(arg_captureList,arg_compiler);
	}
	switch (op) {
	case OP_NEG:
	case OP_NOT:
	case OP_INT:
	case OP_FLOAT:
	case OP_STRING:
		return ;

	case OP_FUNCTION:

		return;
	}
	if (op == OP_IDENTIFIER) {
		const ValueTag* valueTag = GetValueTag(arg_compiler);
		if (valueTag&&!valueTag->isGlobal) {
			arg_captureList.emplace(GetString(), valueTag);
		}
	}

}

std::int32_t Node_function::Push(Compiler* arg_compiler) const
{
	return Call(arg_compiler, leftNode->GetString(), &nodeList->list_args,list_templateTypes);
}

// 関数にpopはできないのでエラーメッセージを出す
std::int32_t Node_function::Pop(Compiler* arg_compiler) const
{
	arg_compiler->error("内部エラー：関数ノードをpopした");
	return TYPE_INTEGER;
}

// 文ノード生成
Statement_t Statement::make_statement(const std::int32_t arg_list_state)
{
	switch (arg_list_state) {
	case NOP_STATE:
		return ButiEngine::make_value< Statement_nop>();

	case DEFAULT_STATE:
		return ButiEngine::make_value<Statement_default>();

	case BREAK_STATE:
		return ButiEngine::make_value< Statement_break>();

	case RETURN_STATE: {
		auto ret = ButiEngine::make_value< Statement_return>();

		return ret;
	}


	case IF_STATE:
		return ButiEngine::make_value<Statement_if>();

	case FOR_STATE:
		return ButiEngine::make_value<Statement_for>();

	case WHILE_STATE:
		return ButiEngine::make_value<Statement_while>();
	}

	std::cerr << "内部エラー：ステートメントノードミス" << std::endl;
	return ButiEngine::make_value<Statement_nop>();
}

Statement_t Statement::make_statement(const std::int32_t arg_list_state, const std::int32_t)
{
	return make_statement(arg_list_state);
}

Statement_t Statement::make_statement(const std::int32_t arg_list_state, Node_t arg_node)
{
	switch (arg_list_state) {

	case UNARY_STATE:
		return ButiEngine::make_value<Statement_unary>(arg_node);
	case ASSIGN_STATE:
		return ButiEngine::make_value<Statement_assign>(arg_node);

	case CASE_STATE:
		return ButiEngine::make_value<Statement_case>(arg_node);

	case SWITCH_STATE:
		return ButiEngine::make_value<Statement_switch>(arg_node);

	case CALL_STATE:
		return ButiEngine::make_value<ccall_statement>(arg_node);
	}

	std::cerr << "内部エラー：文ノードミス" << std::endl;
	return ButiEngine::make_value<Statement_nop>();
}

Statement_t Statement::make_statement(const std::int32_t arg_list_state, Block_t arg_block)
{
	switch (arg_list_state) {
	case BLOCK_STATE:
		return ButiEngine::make_value <Statement_block>(arg_block);
	}

	std::cerr << "内部エラー：文ノードミス" << std::endl;
	return ButiEngine::make_value<Statement_nop>();
}

// nop文
std::int32_t Statement_nop::Analyze(Compiler* arg_compiler) 
{
	return 0;
}

// 代入文
std::int32_t Statement_assign::Analyze(Compiler* arg_compiler) 
{
	return node->Assign(arg_compiler);
	
}


void Statement_assign::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// 関数呼び出し文
std::int32_t ccall_statement::Analyze(Compiler* arg_compiler) 
{
	std::int32_t type = node->Push(arg_compiler);

	if (type == -1) {

		return -1;
	}

	if (type != TYPE_VOID)
		arg_compiler->OpPop();			// 戻り値を捨てるためのpop

	return 0;
}


void ccall_statement::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// case文
std::int32_t Statement_case::Analyze(Compiler* arg_compiler) 
{
	arg_compiler->SetLabel(label_);
	return 0;
}

// case文の前処理
std::int32_t Statement_case::case_Analyze(Compiler* arg_compiler, std::int32_t* arg_default_label)
{
	label_ = arg_compiler->MakeLabel();
	if (node->Op() != OP_INT)
		arg_compiler->error("case 文には定数のみ指定できます。");
	node->Push(arg_compiler);
	arg_compiler->OpTest(label_);
	return 0;
}

void Statement_case::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// default文
std::int32_t Statement_default::Analyze(Compiler* arg_compiler) 
{
	arg_compiler->SetLabel(label_);
	return 0;
}

// default文の前処理
std::int32_t Statement_default::case_Analyze(Compiler* arg_compiler, std::int32_t* arg_default_label)
{
	label_ = arg_compiler->MakeLabel();
	*arg_default_label = label_;

	return 0;
}

// break文
std::int32_t Statement_break::Analyze(Compiler* arg_compiler) 
{
	if (!arg_compiler->JmpBreakLabel()) {
		arg_compiler->error("breakがswitch/for/while外に有ります");
	}
	return 0;
}


// return文
std::int32_t Statement_return::Analyze(Compiler* arg_compiler) 
{

	if (arg_compiler->GetCurrentFunctionType() == TYPE_VOID) {	// 戻り値無し
		if (node) {
			arg_compiler->error("void関数に戻り値が設定されています");
		}
		arg_compiler->OpReturn();
	}
	else {
		if (!node) {
			arg_compiler->error("関数の戻り値がありません");
		}
		else {
			std::int32_t nodeType = node->Push(arg_compiler);		// 戻り値をpush
			std::int32_t returnType = arg_compiler->GetCurrentFunctionType();
			if (!CanTypeCast( nodeType ,returnType)) {
				arg_compiler->error("戻り値の型が合いません");
			}
			if (nodeType == TYPE_INTEGER && returnType == TYPE_FLOAT) {
				arg_compiler->OpCast<std::int32_t, float>();
			}
			if (nodeType == TYPE_FLOAT&& returnType == TYPE_INTEGER) {
				arg_compiler->OpCast<float,std::int32_t>();
			}
		}
		arg_compiler->OpReturnV();
	}

	return 0;
}

void Statement_return::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// if文
std::int32_t Statement_if::Analyze(Compiler* arg_compiler) 
{
	node->Push(arg_compiler);
	std::int32_t label1 = arg_compiler->MakeLabel();
	arg_compiler->OpJmpNC(label1);
	list_statement[0]->Analyze(arg_compiler);

	if (list_statement[1]) {
		std::int32_t label2 = arg_compiler->MakeLabel();
		arg_compiler->OpJmp(label2);
		arg_compiler->SetLabel(label1);
		list_statement[1]->Analyze(arg_compiler);
		arg_compiler->SetLabel(label2);
	}
	else {
		arg_compiler->SetLabel(label1);
	}
	return 0;
}

void Statement_if::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	list_statement[0]->LambdaCapture(arg_captureList, arg_compiler);
	if (list_statement[1]) {
		list_statement[1]->LambdaCapture(arg_captureList, arg_compiler);
	}
}

// for文
std::int32_t Statement_for::Analyze(Compiler* arg_compiler) 
{
	std::int32_t label1 = arg_compiler->MakeLabel();
	std::int32_t label2 = arg_compiler->MakeLabel();

	std::int32_t break_label = arg_compiler->SetBreakLabel(label2);
	if(node[0])
		node[0]->Push(arg_compiler);
	arg_compiler->SetLabel(label1);
	node[1]->Push(arg_compiler);
	arg_compiler->OpJmpNC(label2);
	list_statement->Analyze(arg_compiler);
	if (node[2])
		node[2]->Push(arg_compiler);
	arg_compiler->OpJmp(label1);
	arg_compiler->SetLabel(label2);

	arg_compiler->SetBreakLabel(break_label);
	return 0;
}

void Statement_for::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	for (std::int32_t i = 0; i < sizeof(node) / sizeof(node[0]); i++) {
		node[i]->LambdaCapture(arg_captureList, arg_compiler);
	}
}

// while文
std::int32_t Statement_while::Analyze(Compiler* arg_compiler) 
{
	std::int32_t label1 = arg_compiler->MakeLabel();
	std::int32_t label2 = arg_compiler->MakeLabel();

	std::int32_t break_label = arg_compiler->SetBreakLabel(label2);

	arg_compiler->SetLabel(label1);
	node->Push(arg_compiler);
	arg_compiler->OpJmpNC(label2);
	list_statement->Analyze(arg_compiler);
	arg_compiler->OpJmp(label1);
	arg_compiler->SetLabel(label2);

	arg_compiler->SetBreakLabel(break_label);
	return 0;
}

void Statement_while::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// switch文
std::int32_t Statement_switch::Analyze(Compiler* arg_compiler) 
{
	if (!list_statement.IsEmpty()) {
		node->Push(arg_compiler);

		std::int32_t label = arg_compiler->MakeLabel();		// L0ラベル作成
		std::int32_t break_label = arg_compiler->SetBreakLabel(label);
		std::int32_t default_label = label;

		std::for_each(list_statement.begin(), list_statement.end(),
			boost::bind(&Statement::Case_Analyze, _1, arg_compiler, &default_label));

		arg_compiler->OpPop();
		arg_compiler->OpJmp(default_label);

		std::for_each(list_statement.begin(), list_statement.end(), boost::bind(&Statement::Analyze, _1, arg_compiler));
		arg_compiler->SetLabel(label);

		arg_compiler->SetBreakLabel(break_label);
	}
	return 0;
}

void Statement_switch::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// block文
std::int32_t Statement_block::Analyze(Compiler* arg_compiler) 
{
	arg_compiler->BlockIn();
	block_->Analyze(arg_compiler);
	arg_compiler->BlockOut();
	return 0;
}

void Statement_block::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	block_->LambdaCapture(arg_captureList, arg_compiler);
}


std::int32_t Statement_unary::Analyze(Compiler* arg_compiler)
{
	switch (node->Op())
	{
	case OP_INCREMENT:
	case OP_DECREMENT:

		node->Push(arg_compiler);
		arg_compiler->OpPop();
		return 0;
	default:
		node->Push(arg_compiler);
		return 0;
	}
}

// 文ブロック
std::int32_t Block::Analyze(Compiler* arg_compiler, ButiEngine::List<Function_t>& arg_captureCheck)
{
	auto ret = 0;
	{


		for (auto itr = list_decl.begin(), endItr = list_decl.end(); itr != endItr; itr++) {
			(*itr)->Define(arg_compiler);
		}
	}

	for (auto itr = arg_captureCheck.begin(), end = arg_captureCheck.end(); itr != end;itr++) {
		(*itr)->LambdaCapture(arg_compiler);
	}

	if (!list_decl.IsEmpty())
		arg_compiler->AllocStack();	// スタックフレーム確保

	{
		for (auto itr = list_state.begin(), endItr = list_state.end(); itr != endItr; itr++) {
			if ((ret = (*itr)->Analyze(arg_compiler)) != 0) {
				return ret;
			}
		}
	}

	return ret;
}


void Block::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	for (auto itr = list_state.begin(), endItr = list_state.end(); itr != endItr; itr++) {
		(*itr)->LambdaCapture(arg_captureList, arg_compiler);
	}
}

std::int32_t Declaration::PushCompiler(Compiler* arg_compiler)
{
	return Analyze(arg_compiler);
}

// 宣言の解析
std::int32_t Declaration::Analyze(Compiler* arg_compiler) 
{
	if (isFunction) {		// 関数
		arg_compiler->FunctionDefine(valueType, name, list_argType);
	}
	else {
		arg_compiler->ValueDefine(valueType, list_node,accessType);

		auto type = arg_compiler->GetType(valueType&~TYPE_REF);
		std::int32_t size = list_node.GetSize();
		if (valueType & TYPE_REF) {
			if (type->isSystem) {
				for (std::int32_t i = 0; i < size; i++) {
					arg_compiler->OpAllocStack_Ref(valueType);
				}
			}
			else if (type->p_enumTag) {
				for (std::int32_t i = 0; i < size; i++) {
					arg_compiler->OpAllocStack_Ref_EnumType(valueType);
				}
			}
			else if (type->IsFunctionObjectType()) {
				for (std::int32_t i = 0; i < size; i++) {
					arg_compiler->OpAllocStack_Ref_FunctionType(valueType);
				}
			}
			else {
				for (std::int32_t i = 0; i < size; i++) {
					arg_compiler->OpAllocStack_Ref_ScriptType((valueType & ~TYPE_REF) - arg_compiler->GetSystemTypeSize());
				}
			}
		}
		else {
			if (type->p_enumTag) {
				for (std::int32_t i = 0; i < size; i++) {
					arg_compiler->OpAllocStackEnumType(valueType);
				}
			}else
			if (type->isSystem) {
				for (std::int32_t i = 0; i < size; i++) {
					arg_compiler->OpAllocStack(valueType);
				}
			}
			else if (type->IsFunctionObjectType()) {
				for (std::int32_t i = 0; i < size; i++) {
					arg_compiler->OpAllocStackFunctionType(valueType);
				}
			}
			else {
				for (std::int32_t i = 0; i < size; i++) {
					arg_compiler->OpAllocStack_ScriptType(valueType - arg_compiler->GetSystemTypeSize());
				}
			}
		}
	}
	return 0;
}


void Declaration::Define(Compiler* arg_compiler)
{
	if (isFunction) {		// 関数
		arg_compiler->FunctionDefine(valueType, name, list_argType);
	}
	else {
		arg_compiler->ValueDefine(valueType, list_node,accessType);
	}
}
std::int32_t Node_Member::Push(Compiler* arg_compiler) const
{
	if (op != OP_MEMBER) {
		arg_compiler->error("内部エラー：メンバ変数ノードにメンバ変数以外が登録されています。");
	}
	else {
		leftNode->Push(arg_compiler);
		auto typeTag = arg_compiler->GetType(leftNode->GetType(arg_compiler) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(strData).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName+"の" + strData + "にアクセス出来ません");
		}

		arg_compiler->PushMemberRef(typeTag->map_memberValue.at(strData).index);
		return   typeTag->map_memberValue.at(strData).type & ~TYPE_REF;
	}
	return -1;
}
std::int32_t Node_Member::PushClone(Compiler* arg_compiler) const
{
	if (op != OP_MEMBER) {
		arg_compiler->error("内部エラー：メンバ変数ノードにメンバ変数以外が登録されています。");
	}
	else {
		leftNode->Push(arg_compiler);
		auto typeTag = arg_compiler->GetType(leftNode->GetType(arg_compiler) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(strData).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName + "の" + strData + "にアクセス出来ません");
		}
		arg_compiler->PushMember(typeTag->map_memberValue.at(strData).index);
		return   typeTag->map_memberValue.at(strData).type & ~TYPE_REF;

	}
	return -1;
}
std::int32_t Node_Member::Pop(Compiler* arg_compiler) const
{
	if (op != OP_MEMBER) {
		arg_compiler->error("内部エラー：メンバ変数ノードにメンバ変数以外が登録されています。");
	}
	else {

		//型
		auto typeTag = arg_compiler->GetType(leftNode->GetType(arg_compiler)&~TYPE_REF);
		if (typeTag->map_memberValue.at(strData).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName + "の" + strData + "にアクセス出来ません");
		}
		
		leftNode->Push(arg_compiler);
		auto type = typeTag->map_memberValue.at(strData).type;
		if (type & TYPE_REF) {
			arg_compiler->PopMemberRef(typeTag->map_memberValue.at(strData).index);
		}
		else if(type==TYPE_INTEGER||type==TYPE_FLOAT){
			arg_compiler->PopMemberValueType(typeTag->map_memberValue.at(strData).index);
		}
		else {
			arg_compiler->PopMember(typeTag->map_memberValue.at(strData).index);
		}

		return   typeTag->map_memberValue.at(strData).type & ~TYPE_REF;
		
	}
	return TYPE_INTEGER;
}
std::int32_t Node_Member::GetType(Compiler* arg_compiler) const
{
	if (op != OP_MEMBER) {
		arg_compiler->error("内部エラー：メンバ変数ノードにメンバ変数以外が登録されています。");
	}
	else {
		//変数のメンバ変数
		if (leftNode->Op() == OP_IDENTIFIER|| leftNode->Op() == OP_MEMBER||leftNode->Op()==OP_METHOD ||leftNode->Op()==OP_FUNCTION ) {
			{

				//型
				auto typeTag = arg_compiler->GetType(leftNode->GetType(arg_compiler)&~TYPE_REF);
				if (!typeTag->map_memberValue.count(strData)) {
					arg_compiler->error("構文エラー："+typeTag->typeName+"にメンバ変数"+strData+"は存在しません");
					return -1;
				}
				return typeTag->map_memberValue.at(strData).type;
			}
		}
	}
}
void Node_Member::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	leftNode->LambdaCapture(arg_captureList, arg_compiler);
}
std::int32_t Node_Method::Push(Compiler* arg_compiler) const
{
	if (op != OP_METHOD) {
		arg_compiler->error("内部エラー：メンバ関数ノードにメンバ関数以外が登録されています。");
	}

	ButiEngine::List<std::int32_t> argTypes;
	if (nodeList) {
		auto end = nodeList->list_args.end();
		for (auto itr = nodeList->list_args.begin(); itr != end; itr++) {
			argTypes.Add((*itr)->GetType(arg_compiler));
		}
	}

	std::int32_t argSize = argTypes.GetSize();
	const TypeTag* typeTag = nullptr;
	const FunctionTag* methodTag = nullptr;


	typeTag = arg_compiler->GetType(rightNode-> GetType(arg_compiler) & ~TYPE_REF);
	methodTag = typeTag->methods.Find(leftNode->GetString()+GetTemplateName(list_templateTypes, &arg_compiler->GetTypeTable()), argTypes,&arg_compiler->GetTypeTable());
	if (methodTag->accessType != AccessModifier::Public&&arg_compiler->GetCurrentThisType()!=typeTag) {

		arg_compiler->error(typeTag->typeName + "　の" + methodTag->name + "()はアクセス出来ません");
	}

	if (methodTag == nullptr) {
		std::string message = "";
		if (argSize) {
			for (std::int32_t i = 0; i < argSize; i++) {
				message += arg_compiler->GetTypeName(argTypes[i]) + " ";
			}
			message += "を引数にとる";
		}
		message += "関数" + strData + "は未宣言です";
		arg_compiler->error(message);
		return -1;
	}

	// 引数をpush
	if (nodeList && methodTag->ArgSize() == argSize) {
		std::for_each(nodeList->list_args.begin(), nodeList->list_args.end(), set_arg(arg_compiler, methodTag));
	}


	rightNode->Push(arg_compiler);


	if (methodTag->IsSystem()) {
		// 引数の数をpush
		arg_compiler->PushConstInt(argSize);
		arg_compiler->OpSysMethodCall(methodTag->GetIndex());		// 組み込みメソッド
	}
	else {
		// 引数の数+thisをpush
		arg_compiler->PushConstInt(argSize+1);
		arg_compiler->OpCall(methodTag->GetIndex());			// スクリプト上のメソッド
	}


	return methodTag->valueType;
}
std::int32_t Node_Method::Pop(Compiler* arg_compiler) const
{
	arg_compiler->error("内部エラー：メンバ関数ノードをpop");
	return TYPE_INTEGER;
}
std::int32_t Node_Method::GetType(Compiler* arg_compiler) const
{
	if (op != OP_METHOD) {
		arg_compiler->error("内部エラー：メンバ関数ノードにメンバ関数以外が登録されています。");
	}

	ButiEngine::List<std::int32_t> argTypes;

	if (nodeList) {
		for (auto itr = nodeList->list_args.begin(), end = nodeList->list_args.end(); itr != end; itr++) {
			argTypes.Add((*itr)->GetType(arg_compiler));
		}
	}


	const TypeTag* typeTag = arg_compiler->GetType(rightNode->GetType(arg_compiler) & ~TYPE_REF);
	const FunctionTag* tag = typeTag->methods.Find(leftNode->GetString() +GetTemplateName(list_templateTypes, &arg_compiler->GetTypeTable()), argTypes, &arg_compiler->GetTypeTable());

	return tag->valueType;
}
void Node_Method::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	if (nodeList) {
		nodeList->LambdaCapture(arg_captureList, arg_compiler);
	}


	leftNode->LambdaCapture(arg_captureList, arg_compiler);
}

void NodeList::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	for (auto itr = list_args.begin(), end = list_args.end(); itr != end; itr++) {
		(*itr)->LambdaCapture(arg_captureList, arg_compiler);
	}
}
std::int32_t Node_enum::Push(Compiler* arg_compiler) const
{
	auto enumType = GetEnumType(arg_compiler,*leftNode);
	if (enumType == nullptr) {

		arg_compiler->error("列挙型　" + leftNode->GetString() + "は未定義です");
		return -1;
	}
	if (!enumType->ExistIdentifers(strData)) {

		arg_compiler->error("列挙型　" + leftNode->GetString()+"."+strData + "は未定義です");
		return -1;
	}
	arg_compiler->PushConstInt( enumType->GetValue(strData));

	return TYPE_INTEGER;
}
std::int32_t Node_enum::Pop(Compiler* arg_compiler) const
{
	arg_compiler->error("内部エラー：列挙型ノードをpop");
	return -1;
}
std::int32_t Node_enum::GetType(Compiler* arg_compiler) const
{
	auto enumType = GetEnumType(arg_compiler, *leftNode);
	return enumType->typeIndex;
}
std::int32_t Node_enum::EnumType(Compiler* arg_compiler) const
{
	auto type=arg_compiler->GetType(leftNode->GetString());
	if (!type) {
		arg_compiler->error("列挙型　" + leftNode->GetString() + "." + strData + "は未定義です");
		return 0;
	}
	return type->typeIndex;
}

std::int32_t Node_FunctionObject::Push(Compiler* arg_compiler) const
{
	auto funcTag = GetFunctionType(arg_compiler, *this);

	if (!funcTag) {
		arg_compiler->error(GetString() + "は未定義です");
		return -1;
	}
	arg_compiler->PushConstInt(funcTag->GetIndex());
	return arg_compiler->GetfunctionTypeIndex(funcTag->list_args, funcTag->valueType);
}
std::int32_t Node_FunctionObject::Pop(Compiler* arg_compiler) const
{
	arg_compiler->error("内部エラー：関数型ノードをpop");
	return -1;
}
std::int32_t Node_FunctionObject::GetType(Compiler* arg_compiler) const
{
	auto funcTag = GetFunctionType(arg_compiler, *this);
	
	if (!funcTag) {
		arg_compiler->error(GetString() + "は未定義です");
		return 0;
	}

	return arg_compiler->GetfunctionTypeIndex(funcTag->list_args, funcTag->valueType);
}
void Enum::SetIdentifer(const std::string& arg_name)
{
	map_identifer.emplace(arg_name, map_identifer.size());
}
void Enum::SetIdentifer(const std::string& arg_name, const std::int32_t arg_value)
{
	map_identifer.emplace(arg_name, arg_value);
}
std::int32_t Enum::Analyze(Compiler* arg_compiler)
{
	arg_compiler->RegistEnumType(typeName);
	auto tag = arg_compiler->GetEnumTag(arg_compiler->GetCurrentNameSpace()->GetGlobalNameString()+ typeName);
	auto end = map_identifer.end();
	for (auto itr = map_identifer.begin(); itr != end; itr++) {
		tag->SetValue(itr->first, itr->second);
	}
	return 0;
}
std::int32_t Class::Analyze(Compiler* arg_compiler)
{
	arg_compiler->AnalyzeScriptType(name, map_values);
	auto typeTag = arg_compiler->GetType(name);
	auto methodTable = &typeTag->methods;

	arg_compiler->PushCurrentThisType(typeTag);
	for (auto itr = list_methods.begin(), end = list_methods.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler, methodTable);
	}
	arg_compiler->PopCurrentThisType();
	list_methods.Clear();
	return 0;
}

std::int32_t Class::PushCompiler(Compiler* arg_compiler)
{
	arg_compiler->PushAnalyzeClass(value_from_this());
	return 0;
}
std::int32_t Class::Regist(Compiler* arg_compiler)
{
	arg_compiler->RegistScriptType(name);
	return 0;
}
void Class::RegistMethod(Function_t arg_method, Compiler* arg_compiler)
{
	auto typeTag = arg_compiler->GetType(name);
	auto methodTable = &typeTag->methods;
	arg_method->PushCompiler(arg_compiler,methodTable);
	list_methods.Add(arg_method);
	
}
void Class::SetValue(const std::string& arg_name, const std::int32_t arg_type, const AccessModifier arg_accessType)
{
	std::pair<std::int32_t, AccessModifier> v = { arg_type,arg_accessType };
	map_values.emplace(arg_name,v);
}

std::int32_t Function::PushCompiler(Compiler* arg_compiler, FunctionTable* arg_p_funcTable )
{
	ownNameSpace = arg_compiler->GetCurrentNameSpace();
	arg_compiler->PopAnalyzeFunction();
	arg_compiler->RegistFunction(returnType, name, args, block, accessType,arg_p_funcTable);
	if (!arg_p_funcTable) {
		arg_compiler->GetCurrentNameSpace()->PushFunction(value_from_this());
	}
	searchName =arg_p_funcTable?name:  arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + name;
	
	return 0;
}

std::int32_t Function::PushCompiler_sub(Compiler* arg_compiler)
{
	ownNameSpace = arg_compiler->GetCurrentNameSpace();
	arg_compiler->PopAnalyzeFunction();
	arg_compiler->RegistFunction(returnType, name, args, block, accessType);
	searchName = arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + name;
	arg_compiler->PushSubFunction(value_from_this());
	return 0;
}


// 引数の変数名を登録
struct add_value {
	ButiScript::Compiler* p_compiler;
	ButiScript::ValueTable& values;
	std::int32_t addr;
	add_value(ButiScript::Compiler* arg_p_comp, ButiScript::ValueTable& arg_values, const std::int32_t arg_addres = argmentAddressStart) : p_compiler(arg_p_comp), values(arg_values), addr(arg_addres)
	{
	}

	void operator()(const ButiScript::ArgDefine& arg_argDefine) const
	{
		if (!values.add_arg(arg_argDefine.GetType(), arg_argDefine.GetName(), addr)) {
			p_compiler->error("引数 " + arg_argDefine.GetName() + " は既に登録されています。");
		}
	}
};
// キャプチャ変数を登録
struct add_capture {
	ButiScript::Compiler* p_compiler;
	ButiScript::ValueTable& values;
	std::int32_t addr;
	add_capture(ButiScript::Compiler* arg_p_comp, ButiScript::ValueTable& arg_values, const std::int32_t arg_addres = argmentAddressStart) : p_compiler(arg_p_comp), values(arg_values), addr(arg_addres)
	{
	}

	void operator()(const ButiScript::ArgDefine& arg_argDefine) const
	{
		if (!values.add_capture(arg_argDefine.GetType(), arg_argDefine.GetName(), addr)) {
			p_compiler->error("キャプチャ変数 " + arg_argDefine.GetName() + " は既に登録されています。");
		}
	}
};

// 関数の解析
std::int32_t Function::Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable)
{
	auto currentNameSpace = arg_compiler->GetCurrentNameSpace();
	ownNameSpace =ownNameSpace?ownNameSpace: currentNameSpace;
	arg_compiler->SetCurrentNameSpace(ownNameSpace);
	FunctionTable* p_functable = arg_p_funcTable ? arg_p_funcTable : &arg_compiler->GetFunctions();


	FunctionTag* tag = p_functable->Find_strict(searchName, args,&arg_compiler->GetTypeTable());
	if (tag) {
		if (tag->IsDefinition()) {
			arg_compiler-> error("関数 " + searchName+ " は既に定義されています");
			return 0;
		}
		if (tag->IsDeclaration() && !tag->CheckArgList_strict(args)) {
			arg_compiler-> error("関数 " + searchName + " に異なる型の引数が指定されています");
			return 0;
		}
		tag->SetDefinition();	// 定義済みに設定
	}
	else {
		FunctionTag func(returnType, searchName);
		func.SetArgs(args);				// 引数を設定
		func.SetDefinition();			// 定義済み
		func.SetIndex(arg_compiler-> MakeLabel());		// ラベル登録
		tag = p_functable->Add(searchName, func,&arg_compiler->GetTypeTable());
		if (tag == nullptr)
			arg_compiler->error("内部エラー：関数テーブルに登録できません");
	}

	arg_compiler->PushCurrentFunctionName(searchName);		// 処理中の関数名を登録
	arg_compiler->PushCurrentFunctionType(  returnType);		// 処理中の関数型を登録

	// 関数のエントリーポイントにラベルを置く

	arg_compiler->SetLabel(tag->GetIndex());

	arg_compiler-> BlockIn(false,true);		// 変数スタックを増やす

	// 引数リストを登録
	std::int32_t address = argmentAddressStart;
	//メンバ関数の場合thisを引数に追加
	if (arg_p_funcTable) {
		ArgDefine argDef(arg_compiler-> GetCurrentThisType()->typeIndex, thisPtrName);
		add_value(arg_compiler, arg_compiler->GetValueTable() .GetLast(), address)(argDef);
		address--;
	}
	
	for (auto itr = args.rbegin(), endItr = args.rend(); itr != endItr; itr++) {
		add_value(arg_compiler, arg_compiler->GetValueTable().GetLast(), address)(*itr);
		address--;
	}
	arg_compiler->ValueAddressSubtract(address);

	// 文があれば、文を登録
	if (block) {
		std::int32_t ret = block->Analyze(arg_compiler, list_subFunctions);
	}

	const VMCode& code = arg_compiler->GetStatement() .GetLast();
	if (returnType == TYPE_VOID) {
		if (code.op != VM_RETURN)		// returnが無ければreturnを追加
			arg_compiler-> OpReturn();
	}
	else {
		if (code.op != VM_RETURNV) {
			arg_compiler->error("関数 " + searchName + " の最後にreturn文が有りません。");
		}
	}


	for (auto itr = list_subFunctions.begin(), end = list_subFunctions.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler, nullptr);
	}
	list_subFunctions.Clear();
	arg_compiler->ValueAddressAddition(-address);
	arg_compiler->BlockOut();		// 変数スタックを減らす

	arg_compiler->PopCurrentFunctionName();		// 処理中の関数名を消去
	arg_compiler->PopCurrentFunctionType();		
	arg_compiler->SetCurrentNameSpace(currentNameSpace);
	return 0;
}

void Function::AddSubFunction(Function_t arg_function)
{
	list_subFunctions.Add(arg_function);
}


Lambda::Lambda(const std::int32_t arg_type,const ButiEngine::List<ArgDefine>& arg_list_argDefine, Compiler* arg_compiler)
{
	returnType = arg_type;
	args = arg_list_argDefine;

	lambdaIndex = arg_compiler->GetLambdaCount();
	arg_compiler->IncreaseLambdaCount();
	name = "@lambda:" + std::to_string(lambdaIndex);
}
std::int32_t Lambda::PushCompiler(Compiler* arg_compiler)
{
	auto typeTag = arg_compiler->GetType(returnType);

	arg_compiler->PopAnalyzeFunction();
	arg_compiler->RegistLambda(typeTag->GetFunctionObjectReturnType(),name, args, nullptr);
	searchName = arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + name;

	arg_compiler->PushSubFunction(value_from_this());

	return lambdaIndex;
}
std::int32_t Lambda::Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable)
{
	auto typeTag = arg_compiler->GetType(returnType);
	returnType = typeTag->GetFunctionObjectReturnType();
	auto currentNameSpace = arg_compiler->GetCurrentNameSpace();
	ownNameSpace = ownNameSpace ? ownNameSpace : currentNameSpace;
	arg_compiler->SetCurrentNameSpace(ownNameSpace);
	FunctionTable* p_functable = arg_p_funcTable ? arg_p_funcTable : &arg_compiler->GetFunctions();


	FunctionTag* tag = p_functable->Find_strict(searchName, args, & arg_compiler->GetTypeTable());
	if (tag) {
		if (tag->IsDefinition()) {
			arg_compiler->error("関数 " + searchName + " は既に定義されています");
			return 0;
		}
		if (tag->IsDeclaration() && !tag->CheckArgList_strict(args)) {
			arg_compiler->error("関数 " + searchName + " に異なる型の引数が指定されています");
			return 0;
		}
		tag->SetDefinition();	// 定義済みに設定
	}
	else {
		FunctionTag func(returnType, searchName);
		func.SetArgs(args);				// 引数を設定
		func.SetDefinition();			// 定義済み
		func.SetIndex(arg_compiler->MakeLabel());		// ラベル登録
		tag = p_functable->Add(searchName, func,& arg_compiler->GetTypeTable());
		if (tag == nullptr)
			arg_compiler->error("内部エラー：関数テーブルに登録できません");
	}

	arg_compiler->PushCurrentFunctionName(searchName);		// 処理中の関数名を登録
	arg_compiler->PushCurrentFunctionType(returnType);		// 処理中の関数型を登録

	// 関数のエントリーポイントにラベルを置く

	arg_compiler->SetLabel(tag->GetIndex());

	arg_compiler->BlockIn(false, true);		// 変数スタックを増やす

	// 引数リストを登録
	std::int32_t address = argmentAddressStart;
	//メンバ関数の場合thisを引数に追加
	if (arg_p_funcTable) {
		ArgDefine argDef(arg_compiler->GetCurrentThisType()->typeIndex, thisPtrName);
		add_value(arg_compiler, arg_compiler->GetValueTable().GetLast(), address)(argDef);
		address--;
	}

	for (auto itr = args.rbegin(), endItr = args.rend(); itr != endItr; itr++) {
		add_value(arg_compiler, arg_compiler->GetValueTable().GetLast(), address)(*itr);
		address--;
	}
	arg_compiler->ValueAddressSubtract(address);

	// 文があれば、文を登録
	if (block) {

		///キャプチャする変数を確保
		std::int32_t i = 0;
		for (auto itr = map_lambdaCapture.begin(), end = map_lambdaCapture.end(); itr!= end;i++, itr++) {
			add_capture(arg_compiler, arg_compiler->GetValueTable().GetLast(), i)(ArgDefine(itr->second->valueType,arg_compiler->GetCurrentNameSpace()->GetGlobalNameString()+ itr->first));
			//tag->list_captureList.push_back(itr->second->GetAddress());
		}

		arg_compiler->BlockIn();
		///
		std::int32_t ret = block->Analyze(arg_compiler,list_subFunctions);
		arg_compiler->BlockOut();
	}

	const VMCode& code = arg_compiler->GetStatement().GetLast();
	if (returnType == TYPE_VOID) {
		if (code.op != VM_RETURN)		// returnが無ければreturnを追加
			arg_compiler->OpReturn();
	}
	else {
		if (code.op != VM_RETURNV) {
			arg_compiler->error("関数 " + searchName + " の最後にreturn文が有りません。");
		}
	}


	for (auto itr = list_subFunctions.begin(), end = list_subFunctions.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler, nullptr);
	}
	list_subFunctions.Clear();
	arg_compiler->ValueAddressAddition(-address);
	arg_compiler->BlockOut();		// 変数スタックを減らす

	arg_compiler->PopCurrentFunctionName();		// 処理中の関数名を消去
	arg_compiler->PopCurrentFunctionType();
	arg_compiler->SetCurrentNameSpace(currentNameSpace);

	return lambdaIndex;
}
void Lambda::LambdaCapture(Compiler* arg_compiler)
{
	if (block) {

		FunctionTable* p_functable = &arg_compiler->GetFunctions();


		FunctionTag* tag = p_functable->Find_strict(searchName, args, &arg_compiler->GetTypeTable());
		block->LambdaCapture(map_lambdaCapture, arg_compiler);

		///キャプチャする変数を確保
		std::int32_t i = 0;
		for (auto itr = map_lambdaCapture.begin(), end = map_lambdaCapture.end(); itr != end; i++, itr++) {
			tag->list_captureList.Add(itr->second->GetAddress());
		}

	}
}
}