#include "stdafx.h"
#include "Node.h"
#include "Declaration.h"
#include "compiler.h"
#include"ButiScript_VirtualMachine/VM_value.h"


namespace ButiScript {
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
	if (arg_op == OP_STRING) {
		std::uint64_t pos = arg_str.rfind('\"');
		if (pos != std::string::npos) {
			auto output = ButiEngine::make_value<Node>(arg_op, arg_str.substr(0, pos));
			return output;
		}
	}
	auto output = ButiEngine::make_value<Node>(arg_op, arg_str);
	return output;
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

Node_t Node::make_node(const std::int32_t arg_op, Node_t arg_refference,const std::string &arg_str)
{
	auto output = arg_refference ? ButiEngine::make_value<Node_value>(arg_str, arg_refference):ButiEngine::make_value<Node_value>(arg_str);

	return output;
}

// 引数有り関数ノードの生成
Node_t Node::make_node(const std::int32_t arg_op, Node_t arg_left, NodeList_t arg_list)
{
	auto left = arg_left ? arg_left : ButiEngine::make_value<Node_value>("");
	left->SetArgmentList(arg_list);
	return left;
}
//テンプレート引数有りノードの生成
Node_t Node::make_node(const std::int32_t arg_op, Node_t arg_left, const ButiEngine::List<std::string>& arg_templateTypes)
{
	auto left = arg_left ? arg_left : ButiEngine::make_value<Node_value>("");
	left->SetTemplateList(arg_templateTypes);
	return left;
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
		else {
			arg_compiler->OpNullCheck();
			arg_compiler->OpNot<std::int32_t>();
			return TYPE_INTEGER;
		}
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
		assert("プログラムエラー：不正なノード");
		return -1;
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


	arg_compiler->error("プログラムエラー：処理できない計算ノードがありました。");
	return -1;
}

// ノードのpop
// 計算ノードはpopできない

std::int32_t Node::Pop(Compiler* arg_compiler) const{
	arg_compiler->error("プログラムエラー：計算ノードをpopしています。");
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
	if ((left_type == TYPE_VECTOR2 && right_type == TYPE_FLOAT) || (left_type == TYPE_FLOAT && right_type == TYPE_VECTOR2)
		|| (left_type == TYPE_VECTOR2 && right_type == TYPE_INTEGER) || (left_type == TYPE_INTEGER&& right_type == TYPE_VECTOR2)) {
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
	return valueTag;
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
	if (right_type_raw == -1 || left_type_raw == -1) {
		arg_compiler->error("無効な代入があります");
		return -1;
	}

	if (op != OP_ASSIGN) {
		right_type = SystemTypeOperatorCheck(op - OP_ASSIGN + OP_DECREMENT, left_type, right_type, arg_compiler);
		if (right_type == -1 ) {
			arg_compiler->error("プログラムエラー：処理できない計算ノードがありました。");
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
	else if (arg_compiler->GetType(right_type)->p_enumTag&& left_type==TYPE_INTEGER) {
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
const ValueTag* Node_value::GetValueTag(const std::string& arg_searchStr, Compiler* arg_compiler) const{

	std::string  valueName;
	NameSpace_t currentsearchNameSpace = arg_compiler->GetCurrentNameSpace();
	const ValueTag* valueTag = nullptr;

	while (!valueTag)
	{
		if (currentsearchNameSpace) {
			valueName = currentsearchNameSpace->GetGlobalNameString() + arg_searchStr;
		}
		else {
			valueName = arg_searchStr;
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

// 値ノードのpush
std::int32_t Node_value::Push(Compiler* arg_compiler) const{
	if (m_vlp_refference && !m_vlp_refference->IsNameSpaceNode(arg_compiler)) {
		if (auto enumType = arg_compiler->GetEnumTag(m_vlp_refference->GetString())) {
			if (enumType->ExistIdentifers(strData)) {
				arg_compiler->PushConstInt(enumType->GetValue(strData));
				return TYPE_INTEGER;
			}
			arg_compiler->error(strData + "は" + enumType->GetTypeName() + "に存在しません");
			return -1;
		}
		m_vlp_refference->Push(arg_compiler);
		auto typeTag = arg_compiler->GetType(m_vlp_refference->GetType(arg_compiler) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(strData).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName + "の" + strData + "にアクセス出来ません");
		}
		if (ButiEngine::dynamic_value_ptr_cast<Node_functionCall>(m_vlp_refference)) {
			arg_compiler->PushMember(typeTag->map_memberValue.at(strData).index);
		}
		else {
			arg_compiler->PushMemberRef(typeTag->map_memberValue.at(strData).index);
		}
		return   typeTag->map_memberValue.at(strData).type & ~TYPE_REF;
	}
	


	if (auto thisType = arg_compiler->GetCurrentThisType()) {
		if (thisType->map_memberValue.count(strData)) {
			arg_compiler->PushLocalRef(arg_compiler->GetCurrentThisLocation());
			arg_compiler->PushMemberRef(thisType->map_memberValue.at(strData).index);
			return thisType->map_memberValue.at(strData).type;
		}
	}

	const ValueTag* valueTag = GetValueTag(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData:strData, arg_compiler);
	if (valueTag)
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

			for (auto itr = funcTag->list_captureList.begin(), end = funcTag->list_captureList.end(); itr != end; itr++) {
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
	return TYPE_INTEGER;
}

std::int32_t Node_value::PushClone(Compiler* arg_compiler) const{
	if (m_vlp_refference && !m_vlp_refference->IsNameSpaceNode(arg_compiler)) {
		if (auto enumType = arg_compiler->GetEnumTag(m_vlp_refference->GetString())) {
			if (enumType->ExistIdentifers(strData)) {
				arg_compiler->PushConstInt(enumType->GetValue(strData));
				return TYPE_INTEGER;
			}
			arg_compiler->error(strData + "は" + enumType->GetTypeName() + "に存在しません");
			return -1;
		}
		m_vlp_refference->Push(arg_compiler);
		auto typeTag = arg_compiler->GetType(m_vlp_refference->GetType(arg_compiler) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(strData).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName + "の" + strData + "にアクセス出来ません");
		}
		arg_compiler->PushMember(typeTag->map_memberValue.at(strData).index);
		return   typeTag->map_memberValue.at(strData).type & ~TYPE_REF;
	}

	if (auto thisType = arg_compiler->GetCurrentThisType()) {
		if (thisType->map_memberValue.count(strData)) {
			arg_compiler->PushLocalRef(arg_compiler->GetCurrentThisLocation());
			arg_compiler->PushMember(thisType->map_memberValue.at(strData).index);
			return thisType->map_memberValue.at(strData).type;
		}
	}
	const ValueTag* valueTag = GetValueTag(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData : strData, arg_compiler);
	if (valueTag)
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
		arg_compiler->OpPushFunctionAddress(funcTag->GetIndex());
		return arg_compiler->GetfunctionTypeIndex(funcTag->list_args, funcTag->valueType);
	}
}

// 変数ノードのpop
std::int32_t Node_value::Pop(Compiler* arg_compiler) const{
	if (m_vlp_refference && !m_vlp_refference->IsNameSpaceNode(arg_compiler)) {
		if (auto enumType = arg_compiler->GetEnumTag(m_vlp_refference->GetString())) {
			arg_compiler->error("enumへのpopは不可能です");
			return -1;
		}

		//型
		auto typeTag = arg_compiler->GetType(m_vlp_refference->GetType(arg_compiler) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(strData).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName + "の" + strData + "にアクセス出来ません");
		}

		m_vlp_refference->Push(arg_compiler);
		auto type = typeTag->map_memberValue.at(strData).type;
		if (type & TYPE_REF) {
			arg_compiler->PopMemberRef(typeTag->map_memberValue.at(strData).index);
		}
		else if (type == TYPE_INTEGER || type == TYPE_FLOAT) {
			arg_compiler->PopMemberValueType(typeTag->map_memberValue.at(strData).index);
		}
		else {
			arg_compiler->PopMember(typeTag->map_memberValue.at(strData).index);
		}
		return   typeTag->map_memberValue.at(strData).type & ~TYPE_REF;
	}

	if (auto thisType = arg_compiler->GetCurrentThisType()) {
		if (thisType->map_memberValue.count(strData)) {
			arg_compiler->PushLocalRef(arg_compiler->GetCurrentThisLocation());
			auto type = thisType->map_memberValue.at(strData).type, index = thisType->map_memberValue.at(strData).index;
			if (type & TYPE_REF) {
				arg_compiler->PopMemberRef(index);
			}
			else if (type == TYPE_INTEGER || type == TYPE_FLOAT) {
				arg_compiler->PopMemberValueType(index);
			}
			else {
				arg_compiler->PopMember(index);
			}
			return type & ~TYPE_REF;
		}
	}


	const ValueTag* valueTag = GetValueTag(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData : strData, arg_compiler);
	if (valueTag)
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
	//const auto funcTag = GetFunctionType(arg_compiler, *this);

	arg_compiler->error(GetString() + "は定数です");
	return -1;
}

std::int32_t Node_value::GetType(Compiler* arg_compiler) const
{//変数のメンバ変数
	if (m_vlp_refference&&!m_vlp_refference->IsNameSpaceNode(arg_compiler)) {
		if (auto enumType = arg_compiler->GetEnumTag(m_vlp_refference->GetString())) {
			if (enumType->ExistIdentifers(strData)) {
				return enumType->typeIndex;
			}
			arg_compiler->error(strData + "は" + enumType->GetTypeName() + "に存在しません");
			return -1;
		}
		auto typeTag = arg_compiler->GetType(m_vlp_refference->GetType(arg_compiler) & ~TYPE_REF);
		if (!typeTag->map_memberValue.count(strData)) {
			arg_compiler->error("構文エラー：" + typeTag->typeName + "にメンバ変数" + strData + "は存在しません");
			return -1;
		}
		return typeTag->map_memberValue.at(strData).type;
	}
	if (auto thisType= arg_compiler->GetCurrentThisType()) {
		if (thisType->map_memberValue.count(strData)) {
			return thisType->map_memberValue.at(strData).type;
		}
	}

	const ValueTag* valueTag = GetValueTag(m_vlp_refference?m_vlp_refference->ToNameSpaceString()+strData : strData, arg_compiler);
	if (valueTag) {
		return valueTag->valueType;
	}
	auto funcTag = GetFunctionType(arg_compiler, *this);

	if (!funcTag) {
		arg_compiler->error(GetString() + "は未定義です");
		return 0;
	}

	return arg_compiler->GetfunctionTypeIndex(funcTag->list_args, funcTag->valueType);
}

void Node_value::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	if (m_vlp_refference) {
		m_vlp_refference->LambdaCapture(arg_captureList,arg_compiler);
	}
	const ValueTag* valueTag = GetValueTag(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData : strData, arg_compiler);
	if (valueTag && !valueTag->isGlobal) {
		arg_captureList.emplace(GetString(), valueTag);
	}
}

Node_t Node_value::ToFunctionCall() const
{
	return ButiEngine::make_value<Node_functionCall>(strData, m_vlp_refference,m_list_templateTypeNames);
}

bool Node_value::IsNameSpaceNode(Compiler* arg_compiler) const
{
	if (m_vlp_refference&&!m_vlp_refference->IsNameSpaceNode(arg_compiler)) {
		return false;
	}
	bool output = false;
	if (auto thisType = arg_compiler->GetCurrentThisType()) {
		output |= thisType->map_memberValue.count(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData : strData);
	}
	return  !(GetValueTag(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData : strData, arg_compiler) || arg_compiler->GetEnumTag(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData : strData) || output);
}

// 関数呼び出し
struct set_arg {
	Compiler* p_compiler;
	const ButiEngine::List<std::int32_t>* argTypes_;
	mutable std::int32_t index_;
	bool isSystem = false;
	set_arg(Compiler* arg_p_compiler, const FunctionTag* arg_function,const bool arg_isSystem) : p_compiler(arg_p_compiler), argTypes_(&arg_function->list_args), index_(0),isSystem(arg_isSystem) {}
	set_arg(Compiler* arg_p_compiler, const ButiEngine::List<std::int32_t>* arg_argTypes) : p_compiler(arg_p_compiler), argTypes_(arg_argTypes), index_(0) {}

	void operator()(Node_t arg_node) const
	{
		std::int32_t type = (*argTypes_)[isSystem?(index_): (argTypes_->GetSize()-1- (index_))];
		index_++;
		if ((type & TYPE_REF)) {		// 参照
			if (arg_node->Op() != OP_IDENTIFIER) {
				p_compiler->error("参照型引数に、変数以外は指定できません。");
			}
			else {
				std::string  valueName;
				NameSpace_t currentsearchNameSpace = p_compiler->GetCurrentNameSpace();
				const ValueTag* tag = arg_node->GetValueTag(arg_node->GetString(), p_compiler);
				if (!tag) {
					p_compiler->error("変数 " + arg_node->GetString() + " は定義されていません。");
				}
				else {
					if (!TypeCheck(tag->valueType, type, &p_compiler->GetTypeTable())) {
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
			if (!TypeCheck(arg_node->PushClone(p_compiler), type, &p_compiler->GetTypeTable())) {
				p_compiler->error("引数の型が合いません。");
			}
		}
	}
};
std::int32_t Node_functionCall::Push(Compiler* arg_compiler) const
{
	ButiEngine::List<std::int32_t> temp;
	arg_compiler->TemplateTypeStrToTypeIndex(m_list_templateTypeNames, temp);

	ButiEngine::List<std::int32_t> argTypes;
	ButiEngine::List<Node_t> reversedArgNode;
	if (m_nodeList) {
		reversedArgNode = m_nodeList->list_args;
		std::reverse(reversedArgNode.begin(), reversedArgNode.end());
		for (auto itr : reversedArgNode) {
			argTypes.Add(itr->GetType(arg_compiler));
		}
	}
	std::int32_t argSize = argTypes.GetSize();
	if (m_vlp_refference&& !m_vlp_refference->IsNameSpaceNode(arg_compiler)) {
		const TypeTag* typeTag = nullptr;
		const FunctionTag* methodTag = nullptr;
		typeTag = arg_compiler->GetType(m_vlp_refference->GetType(arg_compiler) & ~TYPE_REF);
		methodTag = typeTag->methods.Find(strData + GetTemplateName(temp, &arg_compiler->GetTypeTable()), argTypes, &arg_compiler->GetTypeTable());
		
		if (!methodTag) {
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
		if (methodTag->accessType != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName + "　の" + methodTag->name + "()はアクセス出来ません");
		}
		// 引数をpush
		if (m_nodeList && methodTag->ArgSize() == argSize) {
			if (methodTag->IsSystem()) {
				for (auto arg : reversedArgNode) {
					set_arg(arg_compiler, methodTag, methodTag->IsSystem())(arg);
				}

			}
			else {
				for (auto arg : m_nodeList->list_args) {
					set_arg(arg_compiler, methodTag, methodTag->IsSystem())(arg);
				}
			}
		}

		m_vlp_refference->Push(arg_compiler);


		if (methodTag->IsSystem()) {
			// 引数の数をpush
			arg_compiler->PushConstInt(argSize);
			arg_compiler->OpSysMethodCall(methodTag->GetIndex());		// 組み込みメソッド
		}
		else {
			// 引数の数+thisをpush
			arg_compiler->PushConstInt(argSize + 1);
			arg_compiler->OpCall(methodTag->GetIndex());			// スクリプト上のメソッド
		}
		return methodTag->valueType;
	}
	//メソッドからメソッドを呼び出す
	if (auto thisType = arg_compiler->GetCurrentThisType()) {
		if (auto methodTag=thisType->methods.Find(strData + GetTemplateName(temp, &arg_compiler->GetTypeTable()), argTypes, &arg_compiler->GetTypeTable())) {
			
			// 引数をpush
			if (m_nodeList) {
				if (methodTag->IsSystem()) {
					for (auto arg : reversedArgNode) {
						set_arg(arg_compiler, methodTag, methodTag->IsSystem())(arg);
					}

				}
				else {
					for (auto arg : m_nodeList->list_args) {
						set_arg(arg_compiler, methodTag, methodTag->IsSystem())(arg);
					}
				}
			}

			arg_compiler->PushLocalRef(arg_compiler->GetCurrentThisLocation());


			if (methodTag->IsSystem()) {
				// 引数の数をpush
				arg_compiler->PushConstInt(argSize);
				arg_compiler->OpSysMethodCall(methodTag->GetIndex());		// 組み込みメソッド
			}
			else {
				// 引数の数+thisをpush
				arg_compiler->PushConstInt(argSize + 1);
				arg_compiler->OpCall(methodTag->GetIndex());			// スクリプト上のメソッド
			}
			return methodTag->valueType;
		}
	}


	const FunctionTag* functionTag = arg_compiler->GetFunctionTag(m_vlp_refference? m_vlp_refference->ToNameSpaceString()+strData:strData, argTypes, temp, true);
	if (!functionTag) {
		functionTag = arg_compiler->GetFunctionTag(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData
			: strData, argTypes, temp, false);
	}


	if (functionTag) {
		// 引数をpush
		if (m_nodeList && functionTag->ArgSize() == argSize) {
			if (functionTag->IsSystem()) {
				for (auto arg : reversedArgNode) {
					set_arg(arg_compiler, functionTag, functionTag->IsSystem())(arg);
				}
				
			}
			else {
				for (auto arg : m_nodeList->list_args) {
					set_arg(arg_compiler, functionTag, functionTag->IsSystem())(arg);
				}
			}
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
	auto valueTag = GetValueTag(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData: strData, arg_compiler);
	if (valueTag) {
		auto valueType = arg_compiler->GetType(valueTag->valueType);
		if (valueType->IsFunctionObjectType()) {
			// 引数をpush
			if (m_nodeList && valueType->GetFunctionObjectArgSize() == argSize) {
				auto valueArgTypes = valueType->GetFunctionObjectArgment();
				for (auto arg : m_nodeList->list_args) {
					set_arg(arg_compiler, &valueArgTypes)(arg);
				}
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
			if (argTypes[i] >= 0 && i < argTypes.GetSize()) {
				message += arg_compiler->GetTypeName(argTypes[i]) + " ";
			}
			else {
				message += "不明な型 ";
			}
		}
		message += "を引数にとる";
	}
	message += "関数" +( m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData: strData) + "は未宣言です";
	arg_compiler->error(message);
	return -1;
}
std::int32_t Node_functionCall::PushClone(Compiler* arg_compiler) const
{
	return Push(arg_compiler);
}
std::int32_t Node_functionCall::Pop(Compiler* arg_compiler) const
{
	arg_compiler->error("プログラムエラー：関数ノードをpopした");
	return TYPE_INTEGER;
}
std::int32_t Node_functionCall::GetType(Compiler* arg_compiler) const
{
	ButiEngine::List<std::int32_t> temp;
	arg_compiler->TemplateTypeStrToTypeIndex(m_list_templateTypeNames, temp);

	ButiEngine::List<std::int32_t> argTypes;
	ButiEngine::List<Node_t> reversedArgNode;
	if (m_nodeList) {
		reversedArgNode = m_nodeList->list_args;
		std::reverse(reversedArgNode.begin(), reversedArgNode.end());
		for (auto itr : reversedArgNode) {
			argTypes.Add(itr->GetType(arg_compiler));
		}
	}
	std::int32_t argSize = argTypes.GetSize();
	if (m_vlp_refference && !m_vlp_refference->IsNameSpaceNode(arg_compiler)) {
		const TypeTag* typeTag = arg_compiler->GetType(m_vlp_refference->GetType(arg_compiler) & ~TYPE_REF);
		const FunctionTag* tag = typeTag->methods.Find(strData + GetTemplateName(temp, &arg_compiler->GetTypeTable()), argTypes, &arg_compiler->GetTypeTable());

		return tag->valueType;
	}
	//メソッドからメソッドを呼び出す
	if (auto thisType = arg_compiler->GetCurrentThisType()) {
		if (auto methodTag = thisType->methods.Find(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData : strData + GetTemplateName(temp, &arg_compiler->GetTypeTable()), argTypes, &arg_compiler->GetTypeTable())) {
			return methodTag->valueType;
		}
	}

	NameSpace_t currentsearchNameSpace = arg_compiler->GetCurrentNameSpace();
	const FunctionTag* tag = arg_compiler->GetFunctionTag(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData : strData, argTypes, temp, true);
	if (!tag) {
		tag = arg_compiler->GetFunctionTag(m_vlp_refference ? m_vlp_refference->ToNameSpaceString() + strData : strData, argTypes, temp, false);
	}

	if (tag == nullptr) {
		return -1;
	}
	return tag->valueType;
}

void Node_functionCall::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	if (m_vlp_refference) {
		if (m_nodeList) {
			m_nodeList->LambdaCapture(arg_captureList, arg_compiler);
		}
		m_vlp_refference->LambdaCapture(arg_captureList, arg_compiler);
	}
	else {
		leftNode->LambdaCapture(arg_captureList, arg_compiler);
		m_nodeList->LambdaCapture(arg_captureList, arg_compiler);
	}
}

void Node_functionCall::SetArgmentList(NodeList_t arg_List)
{
	m_nodeList = arg_List;
	m_nodeList->Reverse();
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

	std::cerr << "プログラムエラー：ステートメントノードミス" << std::endl;
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

	std::cerr << "プログラムエラー：文ノードミス" << std::endl;
	return ButiEngine::make_value<Statement_nop>();
}

Statement_t Statement::make_statement(const std::int32_t arg_list_state, Block_t arg_block)
{
	switch (arg_list_state) {
	case BLOCK_STATE:
		return ButiEngine::make_value <Statement_block>(arg_block);
	}

	std::cerr << "プログラムエラー：文ノードミス" << std::endl;
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
	auto checkExceptionType =node->Push(arg_compiler);
	if (checkExceptionType != TYPE_INTEGER && checkExceptionType != TYPE_FLOAT) {
		arg_compiler->OpNullCheck();
	}
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
		for (auto statement : list_statement) {
			statement->Case_Analyze(arg_compiler, &default_label);
		}

		arg_compiler->OpPop();
		arg_compiler->OpJmp(default_label);

		for (auto statement : list_statement) {
			statement->Analyze(arg_compiler);
		}
		
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
	for (auto itr : list_decl) {
		itr->Define(arg_compiler);
	}

	for (auto itr : arg_captureCheck) {
		itr->LambdaCapture(arg_compiler);
	}

	if (!list_decl.IsEmpty()) {
		arg_compiler->AllocStack();
	}

	for (auto itr : list_state) {
		if ((ret = itr->Analyze(arg_compiler)) != 0) {
			return ret;
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
	auto valueType = arg_compiler->GetTypeIndex(valueTypeName);
	if (isFunction) {// 関数
		arg_compiler->FunctionDefine(valueType, list_names[0], list_argType);
	}
	else {
		arg_compiler->ValueDefine(valueType, list_names,accessType);

		auto type = arg_compiler->GetType(valueType&~TYPE_REF);
		std::int32_t size = list_names.GetSize();
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
		arg_compiler->FunctionDefine(arg_compiler->GetTypeIndex(valueTypeName), list_names[0], list_argType);
	}
	else {
		arg_compiler->ValueDefine(arg_compiler->GetTypeIndex(valueTypeName), list_names,accessType);
	}
}

void NodeList::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	for (auto itr = list_args.begin(), end = list_args.end(); itr != end; itr++) {
		(*itr)->LambdaCapture(arg_captureList, arg_compiler);
	}
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
	arg_compiler->error("プログラムエラー：関数型ノードをpop");
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
}