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
const int argmentAddressStart=-4;
bool CanTypeCast(const int arg_left, const int arg_right) {
	if (arg_left == TYPE_STRING || arg_right == TYPE_STRING) {
		if (arg_left != arg_right) {
			return false;
		}
	}
	return true;
}


const EnumTag* GetEnumType(const Compiler* arg_compiler, Node& arg_leftNode) {

	auto shp_namespace = arg_compiler->GetCurrentNameSpace();
	std::string serchName;
	const  EnumTag* enumType = nullptr;
	while (!enumType)
	{
		serchName = shp_namespace->GetGlobalNameString() + arg_leftNode.GetString();

		enumType = arg_compiler->GetEnumTag(serchName);

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
const FunctionTag* GetFunctionType(const Compiler* arg_compiler, const std::string& arg_str) {

	auto shp_namespace = arg_compiler->GetCurrentNameSpace();
	std::string serchName;
	const  FunctionTag* tag = nullptr;
	while (!tag)
	{
		serchName = shp_namespace->GetGlobalNameString() + arg_str;

		tag = arg_compiler->GetFunctionTag(serchName);

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

const FunctionTag* GetFunctionType(const Compiler* arg_compiler, const Node& leftNode) {
	return GetFunctionType(arg_compiler, leftNode.GetString());
}
// �ϐ��m�[�h�𐶐�
Node_t Node::make_node(const int arg_op, const std::string& arg_str, const Compiler* arg_compiler)
{
	if (arg_op == OP_IDENTIFIER)
		return Node_t(new Node_value(arg_str));

	if (arg_op == OP_STRING) {
		size_t pos = arg_str.rfind('\"');
		if (pos != std::string::npos)
			return Node_t(new Node(arg_op, arg_str.substr(0, pos)));
	}
	return Node_t(new Node(arg_op, arg_str));
}

// �P�����Z�q�̃m�[�h�𐶐�
Node_t Node::make_node(const int arg_op, Node_t arg_left, const Compiler* arg_compiler)
{
	if (arg_op == OP_METHOD) {
		return Node_t(new Node_Method(arg_op, arg_left->GetLeft(), arg_left->GetString()));
	}
	switch (arg_op) {
	case OP_NEG:
		if (arg_left->op == OP_INT) {			// �萔���Z���v�Z����
			arg_left->num_int = -arg_left->num_int;
			return arg_left;
		}
		break;
	}
	return Node_t(new Node(arg_op, arg_left));
}


Node_t Node::make_node(const int arg_op, Node_t arg_left, const std::string arg_memberName,const Compiler* arg_compiler)
{
	if (GetEnumType(arg_compiler,*arg_left)) {
		return  Node_t(new Node_enum( arg_left, arg_memberName));
	}

	if (arg_op == OP_MEMBER) {
		return Node_t(new Node_Member(arg_op, arg_left, arg_memberName));
	}
	else if (arg_op == OP_METHOD) {
		return Node_t(new Node_Method(arg_op, arg_left, arg_memberName));
	}
	return Node_t();
}




// �񍀉��Z�q�̃m�[�h�𐶐�
Node_t Node::make_node(const int arg_op, Node_t arg_left, Node_t arg_right)
{
	// �z��m�[�h�́Aleft�m�[�h��left_�����o�ɉ�����
	if (arg_op == OP_ARRAY) {
		arg_left->leftNode = arg_right;
		return arg_left;
	}
	auto leftOp = arg_left->op;
	auto rightOp = arg_right->op;

	// �萔���Z���v�Z����
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
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				arg_left->num_int /= arg_right->num_int;
			}
			break;

		case OP_MOD:
			if (arg_right->num_int == 0) {
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				arg_left->num_int %= arg_right->num_int;
			}
			break;

		default:
			return Node_t(new Node(arg_op, arg_left, arg_right));
		}
		return arg_left;
	}

	// �萔�����������Z���v�Z����

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
			arg_left->num_float =(float)( (int)arg_left->num_float & (int)arg_right->num_float);
			break;

		case OP_OR:
			arg_left->num_float = (float)((int)arg_left->num_float | (int)arg_right->num_float);
			break;

		case OP_LSHIFT:
			arg_left->num_float = (float)((int)arg_left->num_float << (int)arg_right->num_float);
			break;

		case OP_RSHIFT:
			arg_left->num_float = (float)((int)arg_left->num_float >> (int)arg_right->num_float);
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
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				arg_left->num_float /= arg_right->num_float;
			}
			break;

		case OP_MOD:
			if (arg_right->num_float == 0) {
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				arg_left->num_float = (float)((int)arg_left->num_float % (int)arg_right->num_float);
			}
			break;

		default:
			return Node_t(new Node(arg_op, arg_left, arg_right));
		}
		return arg_left;
	}


	if (leftOp == OP_INT&& rightOp == OP_FLOAT) {
		switch (arg_op) {
		case OP_LOGAND:
			arg_left->num_int = (arg_left->num_int && (int)arg_right->num_float) ? 1 : 0;
			break;

		case OP_LOGOR:
			arg_left->num_int = (arg_left->num_int || (int)arg_right->num_float) ? 1 : 0;
			break;

		case OP_EQ:
			arg_left->num_int = (arg_left->num_int == (int)arg_right->num_float) ? 1 : 0;
			break;

		case OP_NE:
			arg_left->num_int = (arg_left->num_int != (int)arg_right->num_float) ? 1 : 0;
			break;

		case OP_GT:
			arg_left->num_int = (arg_left->num_int > (int)arg_right->num_float) ? 1 : 0;
			break;

		case OP_GE:
			arg_left->num_int = (arg_left->num_int >= (int)arg_right->num_float) ? 1 : 0;
			break;

		case OP_LT:
			arg_left->num_int = (arg_left->num_int < (int)arg_right->num_float) ? 1 : 0;
			break;

		case OP_LE:
			arg_left->num_int = (arg_left->num_int <= (int)arg_right->num_float) ? 1 : 0;
			break;

		case OP_AND:
			arg_left->num_int = (int)arg_left->num_int & (int)(int)arg_right->num_float;
			break;

		case OP_OR:
			arg_left->num_int = (int)arg_left->num_int | (int)(int)arg_right->num_float;
			break;

		case OP_LSHIFT:
			arg_left->num_int = (int)arg_left->num_int << (int)(int)arg_right->num_float;
			break;

		case OP_RSHIFT:
			arg_left->num_int = (int)arg_left->num_int >> (int)(int)arg_right->num_float;
			break;

		case OP_SUB:
			arg_left->num_int -= (int)arg_right->num_float;
			break;

		case OP_ADD:
			arg_left->num_int += (int)arg_right->num_float;
			break;

		case OP_MUL:
			arg_left->num_int *= (int)arg_right->num_float;
			break;

		case OP_DIV:
			if ((int)arg_right->num_float == 0) {
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				arg_left->num_int /= (int)arg_right->num_float;
			}
			break;

		case OP_MOD:
			if ((int)arg_right->num_float == 0) {
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				arg_left->num_int = (int)arg_left->num_int % (int)arg_right->num_float;
			}
			break;

		default:
			return Node_t(new Node(arg_op, arg_left, arg_right));
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
			arg_left->num_float = (float)((int)arg_left->num_float & (int)arg_right->num_int);
			break;

		case OP_OR:
			arg_left->num_float = (float)((int)arg_left->num_float | (int)arg_right->num_int);
			break;

		case OP_LSHIFT:
			arg_left->num_float = (float)((int)arg_left->num_float << (int)arg_right->num_int);
			break;

		case OP_RSHIFT:
			arg_left->num_float = (float)((int)arg_left->num_float >> (int)arg_right->num_int);
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
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				arg_left->num_float /= arg_right->num_int;
			}
			break;

		case OP_MOD:
			if (arg_right->num_int == 0) {
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				arg_left->num_float = (float)((int)arg_left->num_float % (int)arg_right->num_int);
			}
			break;

		default:
			return Node_t(new Node(arg_op, arg_left, arg_right));
		}
		return arg_left;
	}

	// �����񓯎m�̒萔�v�Z
	if (leftOp == OP_STRING && rightOp == OP_STRING) {
		if (arg_op == OP_ADD) {
			arg_left->strData += arg_right->strData;
			return arg_left;
		}

		int Value = 0;
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
			std::cerr << "�����񓯎m�ł͂ł��Ȃ��v�Z�ł��B" << std::endl;
			break;
		}
		return Node_t(new Node(OP_INT, Value));
	}
	return Node_t(new Node(arg_op, arg_left, arg_right));
}

// �֐��m�[�h�̐���
Node_t Node::make_node(const int arg_op, Node_t arg_left, NodeList_t arg_right)
{
	if (arg_op == OP_METHOD) {
		return  Node_t(new Node_Method(arg_op, arg_left, arg_right));
	}
	return Node_t(new Node_function(arg_op, arg_left, arg_right));
}



template <typename T>
bool SetDefaultOperator(const int arg_op, Compiler* arg_compiler) {
	switch (arg_op) {
	case OP_ADD:
		arg_compiler->OpAdd<T>();
		break;

	case OP_SUB:
		arg_compiler->OpSub<T>();
		break;

	case OP_MUL:
		arg_compiler->OpMul<T>();
		break;

	case OP_DIV:
		arg_compiler->OpDiv<T>();
		break;


	default:
		return false;
		break;
	}
	return true;
}
template <typename T, typename U>
bool SetDeferentTypeMulOperator(const int arg_op, Compiler* arg_compiler) {
	if (arg_op == OP_MUL) {

		arg_compiler->OpMul<T, U>();
		return true;
	}
	return false;
}
template <typename T, typename U>
bool SetDeferentTypeDivOperator(const int arg_op, Compiler* arg_compiler) {
	if (arg_op == OP_DIV) {

		arg_compiler->OpDiv<T, U>();
		return true;
	}
	return false;
}

template <typename T>
bool SetModOperator(const int arg_op, Compiler* arg_compiler) {
	if (arg_op == OP_MOD) {
		arg_compiler->OpMod<T>();
		return true;
	}
	return false;
}
bool SetLogicalOperator(const int arg_op, Compiler* arg_compiler) {
	switch (arg_op) {
	case OP_LOGAND:
		arg_compiler->OpLogAnd();
		break;

	case OP_LOGOR:
		arg_compiler->OpLogOr();
		break;

	case OP_EQ:
		arg_compiler->OpEq();
		break;

	case OP_NE:
		arg_compiler->OpNe();
		break;

	case OP_GT:
		arg_compiler->OpGt();
		break;

	case OP_GE:
		arg_compiler->OpGe();
		break;

	case OP_LT:
		arg_compiler->OpLt();
		break;

	case OP_LE:
		arg_compiler->OpLe();
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
bool SetDefaultAssignOperator(const int arg_op, Compiler* arg_compiler) {
	switch (arg_op) {
	case OP_ADD_ASSIGN:
		arg_compiler->OpAdd<T>();
		break;

	case OP_SUB_ASSIGN:
		arg_compiler->OpSub<T>();
		break;

	case OP_MUL_ASSIGN:
		arg_compiler->OpMul<T>();
		break;

	case OP_DIV_ASSIGN:
		arg_compiler->OpDiv<T>();
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
bool SetDeferentTypeMulAssignOperator(const int arg_op, Compiler* arg_compiler) {
	if (arg_op == OP_MUL_ASSIGN) {

		arg_compiler->OpMul<T, U>();
		return true;
	}
	return false;
}
template <typename T, typename U>
bool SetDeferentTypeDivAssignOperator(const int arg_op, Compiler* arg_compiler) {
	if (arg_op == OP_DIV_ASSIGN) {

		arg_compiler->OpDiv<T, U>();
		return true;
	}
	return false;
}

template <typename T>
bool SetModAssignOperator(const int arg_op, Compiler* arg_compiler) {
	if (arg_op == OP_MOD_ASSIGN) {
		arg_compiler->OpMod<T>();
		return true;
	}
	return false;
}
bool SetLogicalAssignOperator(const int arg_op, Compiler* arg_compiler) {
	switch (arg_op) {
	case OP_AND_ASSIGN:
		arg_compiler->OpAnd();
		break;

	case OP_OR_ASSIGN:
		arg_compiler->OpOr();
		break;

	case OP_LSHIFT_ASSIGN:
		arg_compiler->OpLeftShift();
		break;

	case OP_RSHIFT_ASSIGN:
		arg_compiler->OpRightShift();
		break;

	default:
		return false;
		break;
	}
	return true;
}



// �m�[�h��push����
int Node::Push(Compiler* arg_compiler) const{
	if (op >= OP_ASSIGN && op <= OP_RSHIFT_ASSIGN) {
		return Assign(arg_compiler);
	}

	switch (op) {
	case OP_NEG:
		if (leftNode->Push(arg_compiler) == TYPE_STRING)
			arg_compiler->error("������ɂ͓��ꋖ����Ȃ��v�Z�ł��B");
		arg_compiler->OpNeg();
		return TYPE_INTEGER;

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
		return Call(arg_compiler, strData, nullptr);
	}

	int left_type = leftNode->Push(arg_compiler);
	int right_type = rightNode->Push(arg_compiler);

	//�E�ӎႵ���͍��ӂ�����`�֐�
	if (left_type <= -1 || right_type <= -1) {
		return -1;
	}

	// float�v�Z�m�[�h�̏���
	if ((left_type == TYPE_FLOAT && right_type == TYPE_FLOAT ) || (left_type == TYPE_INTEGER && right_type == TYPE_FLOAT) || (left_type == TYPE_FLOAT && right_type == TYPE_INTEGER)) {
		if (!SetDefaultOperator<float>(op, arg_compiler) && (!SetModOperator<float>(op, arg_compiler)) && (!SetLogicalOperator(op, arg_compiler))) {
				arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}

		return TYPE_FLOAT;
	}

	// �����v�Z�m�[�h�̏���
	if (left_type ==  TYPE_INTEGER && right_type==TYPE_INTEGER) {
		if (!SetDefaultOperator<int>(op, arg_compiler)&&  (!SetModOperator<int>(op, arg_compiler))&& (!SetLogicalOperator(op, arg_compiler))) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");	
		}
		return TYPE_INTEGER;
	}

	//Vector2�v�Z�m�[�h
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_VOID + 1) {
		if (!SetDefaultOperator<ButiEngine::Vector2>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulOperator<ButiEngine::Vector2, float>(op, arg_compiler) && !SetDeferentTypeDivOperator<ButiEngine::Vector2, float>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulOperator<ButiEngine::Vector2, int>(op, arg_compiler) && !SetDeferentTypeDivOperator<ButiEngine::Vector2, int>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_FLOAT && right_type == TYPE_VOID + 1) {
		if (!SetDeferentTypeMulOperator<float, ButiEngine::Vector2>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_INTEGER && right_type == TYPE_VOID + 1) {

		if (!SetDeferentTypeMulOperator<int, ButiEngine::Vector2>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 1;
	}
	//Vector3�v�Z�m�[�h
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_VOID + 2) {
		if (!SetDefaultOperator<ButiEngine::Vector3>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulOperator<ButiEngine::Vector3, float>(op, arg_compiler) && !SetDeferentTypeDivOperator<ButiEngine::Vector3, float>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulOperator<ButiEngine::Vector3, int>(op, arg_compiler) && !SetDeferentTypeDivOperator<ButiEngine::Vector3, int>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_FLOAT && right_type == TYPE_VOID + 2) {
		if (!SetDeferentTypeMulOperator<float, ButiEngine::Vector3>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_INTEGER && right_type == TYPE_VOID + 2) {

		if (!SetDeferentTypeMulOperator<int, ButiEngine::Vector3>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 2;
	}
	//Vector4�v�Z�m�[�h
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_VOID + 3) {
		if (!SetDefaultOperator<ButiEngine::Vector4>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulOperator<ButiEngine::Vector4, float>(op, arg_compiler) && !SetDeferentTypeDivOperator<ButiEngine::Vector4, float>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_VOID +3 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulOperator<ButiEngine::Vector4, int>(op, arg_compiler) && !SetDeferentTypeDivOperator<ButiEngine::Vector4, int>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_FLOAT && right_type == TYPE_VOID + 3) {
		if (!SetDeferentTypeMulOperator<float, ButiEngine::Vector4>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_INTEGER && right_type == TYPE_VOID + 3) {

		if (!SetDeferentTypeMulOperator<int, ButiEngine::Vector4>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 3;
	}

	if (arg_compiler->GetType(left_type)->p_enumTag && right_type == TYPE_INTEGER) {
		if (op == OP_EQ)
		{
			arg_compiler->OpEq();
		}
		return left_type;
	}

	// ������v�Z�m�[�h�̏���
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
		arg_compiler->error("������ł͌v�Z�ł��Ȃ����ł��B");
		break;
	}
	return TYPE_STRING;
}

// �m�[�h��pop
// �v�Z�m�[�h��pop�ł��Ȃ�

int Node::Pop(Compiler* arg_compiler) const{
	arg_compiler->error("�����G���[�F�v�Z�m�[�h��pop���Ă��܂��B");
	return TYPE_INTEGER;
}

//�m�[�h�̌^�`�F�b�N
int Node::GetType(Compiler* arg_compiler)const {
	if (op >= OP_ASSIGN && op <= OP_RSHIFT_ASSIGN) {
		return -1;
	}

	switch (op) {
	case OP_NEG:
		if (leftNode->GetType(arg_compiler) == TYPE_STRING)
			arg_compiler->error("������ɂ͋�����Ȃ��v�Z�ł��B");
		return TYPE_INTEGER;

	case OP_INT:
		return TYPE_INTEGER;
	case OP_FLOAT:
		return TYPE_FLOAT;

	case OP_STRING:
		return TYPE_STRING;

	case OP_FUNCTION:
		return GetCallType(arg_compiler, strData, nullptr);
	}
	if (op == OP_IDENTIFIER) {
		const ValueTag* valueTag = GetValueTag(arg_compiler);
		if (valueTag)  {
			return valueTag->valueType;
		}
		auto funcTag = GetFunctionType(arg_compiler, *this);

		if (!funcTag) {
			arg_compiler->error(GetString() + "�͖���`�ł�");
			return 0;
		}

		return arg_compiler->GetfunctionTypeIndex(funcTag->vec_args, funcTag->valueType);

	}
	int left_type = leftNode->GetType(arg_compiler);
	int right_type = rightNode->GetType(arg_compiler);

	//�E�ӎႵ���͍��ӂ�����`�֐�
	if (left_type <= -1 || right_type <= -1) {
		return -1;
	}

	// float�v�Z�m�[�h�̏���
	if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
		return TYPE_FLOAT;
	}

	// �����v�Z�m�[�h�̏���
	if (left_type == TYPE_INTEGER && right_type == TYPE_INTEGER) {
		return TYPE_INTEGER;
	}


	// ������v�Z�m�[�h�̏���
	switch (op) {
	case OP_EQ:case OP_NE:case OP_GT:case OP_GE:case OP_LT:case OP_LE:
		return TYPE_INTEGER;

	case OP_ADD:
		break;

	default:
		arg_compiler->error("������ł͌v�Z�ł��Ȃ����ł��B");
		break;
	}
	return TYPE_STRING;
}
const ValueTag* Node::GetValueTag(Compiler* arg_compiler) const
{
		arg_compiler->error("�ϐ��m�[�h�ȊO����ϐ����󂯎�낤�Ƃ��Ă��܂�");
		return nullptr;
}
const ValueTag* Node::GetValueTag(const std::string& arg_name, Compiler* arg_compiler) const
{
	std::string  valueName;
	NameSpace_t currentSerchNameSpace = arg_compiler->GetCurrentNameSpace();
	const ValueTag* valueTag = nullptr;

	while (!valueTag)
	{
		if (currentSerchNameSpace) {
			valueName = currentSerchNameSpace->GetGlobalNameString() + arg_name;
		}
		else {
			valueName = arg_name;
		}

		valueTag = arg_compiler->GetValueTag(valueName);
		if (currentSerchNameSpace) {
			currentSerchNameSpace = currentSerchNameSpace->GetParent();
		}
		else {
			break;
		}

	}
	if (!valueTag) {
		//arg_compiler->error("�ϐ� " + valueName + " �͒�`����Ă��܂���B");
	}
	return valueTag;
}
int  Node_function::GetType(Compiler* arg_compiler)const {
	return GetCallType(arg_compiler, leftNode->GetString(), &node_list_->vec_args);
}
void Node_function::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	leftNode->LambdaCapture(arg_captureList, arg_compiler);
	node_list_->LambdaCapture(arg_captureList, arg_compiler);
}
//�m�[�h�̊֐��Ăяo���^�`�F�b�N
int Node::GetCallType(Compiler* arg_compiler, const std::string& arg_name, const std::vector<Node_t>* arg_vec_argNode)const {

	std::vector<int> argTypes;
	if (arg_vec_argNode) {
		auto end = arg_vec_argNode->end();
		for (auto itr = arg_vec_argNode->begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(arg_compiler));
		}
	}
	std::string  functionName;
	NameSpace_t currentSerchNameSpace = arg_compiler->GetCurrentNameSpace();
	const FunctionTag* tag=nullptr;

	while (!tag)
	{
		if (currentSerchNameSpace) {
			functionName = currentSerchNameSpace->GetGlobalNameString() + arg_name;
		}
		else {
			functionName = arg_name;
		}

		tag = arg_compiler->GetFunctionTag(functionName, argTypes, true);
		if (!tag) {
			tag = arg_compiler->GetFunctionTag(functionName, argTypes, false);
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

// �����
int Node::Assign(Compiler* arg_compiler) const{
	int left_type = -1;
	//����݂̂̃p�^�[���ł͂Ȃ��̂ō��ӂ�push
	if (op != OP_ASSIGN) {
		left_type = leftNode->Push(arg_compiler)&~TYPE_REF;
	}
	else {
		left_type = leftNode->GetType(arg_compiler) & ~TYPE_REF;
	}
	int right_type = rightNode->Push(arg_compiler) & ~TYPE_REF;



	// float�v�Z�m�[�h�̏���
	if ((left_type == TYPE_FLOAT && right_type == TYPE_FLOAT) || (left_type == TYPE_INTEGER && right_type == TYPE_FLOAT) || (left_type == TYPE_FLOAT && right_type == TYPE_INTEGER)) {
		if (!SetDefaultAssignOperator<float>(op, arg_compiler) && (!SetModAssignOperator<float>(op, arg_compiler)) && (!SetLogicalAssignOperator(op, arg_compiler))) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler);
		return 0;
	}

	// �����v�Z�m�[�h�̏���
	if (left_type == TYPE_INTEGER && right_type == TYPE_INTEGER) {
		if (!SetDefaultAssignOperator<int>(op, arg_compiler) && (!SetModAssignOperator<int>(op, arg_compiler)) && (!SetLogicalAssignOperator(op, arg_compiler))) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler);
		return 0;
	}


	//Vector2�v�Z�m�[�h
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_VOID + 1) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector2>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler);
		return 0;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector2, float>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector2, float>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler); 
		return 0;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector2, int>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector2, int>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler);
		return 0;
	}
	//Vector3�v�Z�m�[�h
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_VOID + 2) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector3>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler);
		return 0;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector3, float>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector3, float>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler);
		return 0;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector3, int>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector3, int>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler);
		return 0;
	}
	//Vector4�v�Z�m�[�h
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_VOID + 3) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector4>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler);
		return 0;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector4, float>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector4, float>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler);
		return 0;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector4, int>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector4, int>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		leftNode->Pop(arg_compiler);
		return 0;
	}

	if (left_type == TYPE_STRING&& right_type == TYPE_STRING) {
		switch (op) {
		case OP_ADD_ASSIGN:
			arg_compiler->OpStrAdd();
			break;

		case OP_ASSIGN:
			break;

		default:
			arg_compiler->error("������ł͋�����Ȃ��v�Z�ł��B");
			break;
		}
		leftNode->Pop(arg_compiler);

		return 0;
	}

	if (left_type == right_type) {
		//�����^���m�Ȃ̂ő���\
		leftNode->Pop(arg_compiler);
		return 0;
	}
	else if (rightNode->EnumType(arg_compiler) == left_type) {
		leftNode->Pop(arg_compiler);
		return 0;
	}
	else
	{
		arg_compiler->error("����o���Ȃ��ϐ��̑g�ݍ��킹�ł�");
	}

	return -1;
}

const ValueTag* Node_value::GetValueTag(Compiler* arg_compiler) const{

	if (op != OP_IDENTIFIER) {
		arg_compiler->error("�����G���[�F�ϐ��m�[�h�ɕϐ��ȊO���o�^����Ă��܂��B");
		return nullptr;
	}

	else {

		std::string  valueName;
		NameSpace_t currentSerchNameSpace = arg_compiler->GetCurrentNameSpace();
		const ValueTag* valueTag = nullptr;

		while (!valueTag)
		{
			if (currentSerchNameSpace) {
				valueName = currentSerchNameSpace->GetGlobalNameString() + strData;
			}
			else {
				valueName = strData;
			}

			valueTag = arg_compiler->GetValueTag(valueName);
			if (currentSerchNameSpace) {
				currentSerchNameSpace = currentSerchNameSpace->GetParent();
			}
			else {
				break;
			}

		}
		if (!valueTag) {
			//arg_compiler->error("�ϐ� " + valueName + " �͒�`����Ă��܂���B");
		}
		return valueTag;
	}
}

// �ϐ��m�[�h��push
int Node_value::Push(Compiler* arg_compiler) const{
	if (op != OP_IDENTIFIER) {
		arg_compiler->error("�����G���[�F�ϐ��m�[�h�ɕϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		const ValueTag* valueTag = GetValueTag(arg_compiler);
		if(valueTag)
		{
			if (valueTag->isGlobal) {		// �O���[�o���ϐ�
				if (leftNode) {		// �z��
					leftNode->Push(arg_compiler);
					arg_compiler->PushGlobalArrayRef(valueTag->GetAddress());
				}
				else {
					arg_compiler->PushGlobalValueRef(valueTag->GetAddress());
				}
			}
			else {					// ���[�J���ϐ�
				if (leftNode) {		// �z��
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
			arg_compiler->error(GetString() + "�͖���`�ł�");
			return -1;
		}
		else {
			if (funcTag->isLambda) {
				arg_compiler->PushConstInt(funcTag->valueType);
				
				for (auto itr = funcTag->vec_captureList.begin(),end=funcTag->vec_captureList.end();itr!=end; itr++) {
					arg_compiler->PushConstInt(*itr);
				}

				arg_compiler->PushConstInt(funcTag->vec_captureList.size());

				arg_compiler->OpPushLambda(funcTag->GetIndex());
			}
			else {
				arg_compiler->OpPushFunctionAddress(funcTag->GetIndex());
			}
			return arg_compiler->GetfunctionTypeIndex(funcTag->vec_args, funcTag->valueType);
		}
	}
	return TYPE_INTEGER;
}

int Node_value::PushClone(Compiler* arg_compiler) const{
	if (op != OP_IDENTIFIER) {
		arg_compiler->error("�����G���[�F�ϐ��m�[�h�ɕϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		const ValueTag* valueTag = GetValueTag(arg_compiler);
		if(valueTag)
		{
			if (valueTag->isGlobal) {		// �O���[�o���ϐ�
				if (leftNode) {		// �z��
					leftNode->Push(arg_compiler);
					arg_compiler->PushGlobalArray(valueTag->GetAddress());
				}
				else {
					arg_compiler->PushGlobalValue(valueTag->GetAddress());
				}
			}
			else {					// ���[�J���ϐ�
				if (leftNode) {		// �z��
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
			arg_compiler->error(GetString() + "�͖���`�ł�");
			return -1;
		}
		else {
			arg_compiler->OpPushFunctionAddress (funcTag->GetIndex());
			return arg_compiler->GetfunctionTypeIndex(funcTag->vec_args, funcTag->valueType);
		}
	}
	return TYPE_INTEGER;
}

// �ϐ��m�[�h��pop
int Node_value::Pop(Compiler* arg_compiler) const{
	if (op != OP_IDENTIFIER) {
		arg_compiler->error("�����G���[�F�ϐ��m�[�h�ɕϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		const ValueTag* valueTag = GetValueTag(arg_compiler);
		if(valueTag)
		{
			if (valueTag->isGlobal) {		// �O���[�o���ϐ�
				if (leftNode) {		// �z��
					leftNode->Push(arg_compiler);
					arg_compiler->PopArray(valueTag->GetAddress());
				}
				else {
					arg_compiler->PopValue(valueTag->GetAddress());
				}
			}
			else {					// ���[�J���ϐ�
				if (leftNode) {		// �z��
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
			arg_compiler->error(GetString() + "�͖���`�ł�");
			return -1;
		}
		else {
			arg_compiler->error(GetString() + "�͒萔�ł�");
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

// �֐��Ăяo��
struct set_arg {
	Compiler* p_compiler;
	const std::vector<int>* argTypes_;
	mutable int index_;
	set_arg(Compiler* arg_p_compiler, const FunctionTag* arg_function) : p_compiler(arg_p_compiler), argTypes_(&arg_function->vec_args), index_(0) {}
	set_arg(Compiler* arg_p_compiler, const std::vector<int>* arg_argTypes) : p_compiler(arg_p_compiler), argTypes_(arg_argTypes), index_(0){}

	void operator()(Node_t arg_node) const
	{
		int type = (*argTypes_)[index_++];
		if ((type & TYPE_REF) != 0) {		// �Q��
			if (arg_node->Op() != OP_IDENTIFIER) {
				p_compiler->error("�Q�ƌ^�����ɁA�ϐ��ȊO�͎w��ł��܂���B");
			}
			else {
				std::string  valueName;
				NameSpace_t currentSerchNameSpace = p_compiler->GetCurrentNameSpace();
				const ValueTag* tag = arg_node->GetValueTag(p_compiler);
				if (tag == nullptr) {
					p_compiler->error("�ϐ� " + arg_node->GetString() + " �͒�`����Ă��܂���B");
				}
				else {
					if (!TypeCheck(tag->valueType ,type) ){
						p_compiler->error("�����̌^�������܂���B");
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
			if (!TypeCheck( arg_node->PushClone(p_compiler), type)) {
				p_compiler->error("�����̌^�������܂���B");
			}
		}
	}
};

// �֐��Ăяo��
int Node::Call(Compiler* arg_compiler, const std::string& arg_name, const std::vector<Node_t>* arg_vec_argNodes) const
{
	std::string  functionName;
	NameSpace_t currentSerchNameSpace = arg_compiler->GetCurrentNameSpace();

	std::vector<int> argTypes;
	if (arg_vec_argNodes) {
		auto end = arg_vec_argNodes->end();
		for (auto itr = arg_vec_argNodes->begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(arg_compiler));
		}
	}

	int argSize = argTypes.size();
	const FunctionTag* functionTag=nullptr;
	
	while (!functionTag)
	{
		if (currentSerchNameSpace) {
			functionName = currentSerchNameSpace->GetGlobalNameString() + arg_name;
		}
		else {
			functionName = arg_name;
		}
		
		functionTag = arg_compiler->GetFunctionTag(functionName, argTypes, true);
		if (!functionTag) {
			functionTag = arg_compiler->GetFunctionTag(functionName, argTypes, false);
		}
		if (currentSerchNameSpace) {
			currentSerchNameSpace = currentSerchNameSpace->GetParent();
		}
		else {
			break;
		}

	}


	if (functionTag ) {
		// ������push
		if (arg_vec_argNodes && functionTag->ArgSize() == argSize) {
			std::for_each(arg_vec_argNodes->begin(), arg_vec_argNodes->end(), set_arg(arg_compiler, functionTag));
		}

		// �����̐���push
		arg_compiler->PushConstInt(argSize);

		if (functionTag->IsSystem()) {
			arg_compiler->OpSysCall(functionTag->GetIndex());		// �g�ݍ��݊֐�
		}
		else {
			arg_compiler->OpCall(functionTag->GetIndex());			// �X�N���v�g��̊֐�
		}

		return functionTag->valueType;
	}
	//�֐��^�ϐ�����̌Ăяo��
	auto valueTag = GetValueTag(arg_name, arg_compiler);
	if (valueTag) {
		auto valueType=arg_compiler->GetType(valueTag->valueType);
		if (valueType->IsFunctionObjectType()) {
			// ������push
			if (arg_vec_argNodes && valueType->GetFunctionObjectArgSize() == argSize) {
				auto valueArgTypes = valueType->GetFunctionObjectArgment();
				std::for_each(arg_vec_argNodes->begin(), arg_vec_argNodes->end(), set_arg(arg_compiler,&valueArgTypes ));
			}

			// �����̐���push
			arg_compiler->PushConstInt(argSize);
			if (valueTag->isGlobal) {		// �O���[�o���ϐ�

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
		for (int i = 0; i < argSize; i++) {
			message += arg_compiler->GetTypeName(argTypes[i]) + " ";
		}
		message += "�������ɂƂ�";
	}
	message += "�֐�" + functionName + "�͖��錾�ł�";
	arg_compiler->error(message);
	return -1;
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

int Node_function::Push(Compiler* arg_compiler) const
{
	return Call(arg_compiler, leftNode->GetString(), &node_list_->vec_args);
}

// �֐���pop�͂ł��Ȃ��̂ŃG���[���b�Z�[�W���o��
int Node_function::Pop(Compiler* arg_compiler) const
{
	arg_compiler->error("�����G���[�F�֐��m�[�h��pop����");
	return TYPE_INTEGER;
}

// ���m�[�h����
Statement_t Statement::make_statement(const int arg_vec_state)
{
	switch (arg_vec_state) {
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

	std::cerr << "�����G���[�F���m�[�h�~�X" << std::endl;
	return Statement_t(new Statement_nop());
}

Statement_t Statement::make_statement(const int arg_vec_state, const int)
{
	return make_statement(arg_vec_state);
}

Statement_t Statement::make_statement(const int arg_vec_state, Node_t arg_node)
{
	switch (arg_vec_state) {
	case ASSIGN_STATE:
		return Statement_t(new Statement_assign(arg_node));

	case CASE_STATE:
		return Statement_t(new Statement_case(arg_node));

	case SWITCH_STATE:
		return Statement_t(new Statement_switch(arg_node));

	case CALL_STATE:
		return Statement_t(new ccall_statement(arg_node));
	}

	std::cerr << "�����G���[�F���m�[�h�~�X" << std::endl;
	return Statement_t(new Statement_nop());
}

Statement_t Statement::make_statement(const int arg_vec_state, Block_t arg_block)
{
	switch (arg_vec_state) {
	case BLOCK_STATE:
		return Statement_t(new Statement_block(arg_block));
	}

	std::cerr << "�����G���[�F���m�[�h�~�X" << std::endl;
	return Statement_t(new Statement_nop());
}

// nop��
int Statement_nop::Analyze(Compiler* arg_compiler) 
{
	return 0;
}

// �����
int Statement_assign::Analyze(Compiler* arg_compiler) 
{
	return node->Assign(arg_compiler);
	
}


void Statement_assign::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// �֐��Ăяo����
int ccall_statement::Analyze(Compiler* arg_compiler) 
{
	int type = node->Push(arg_compiler);

	if (type == -1) {

		return -1;
	}

	if (type != TYPE_VOID)
		arg_compiler->OpPop();			// �߂�l���̂Ă邽�߂�pop

	return 0;
}


void ccall_statement::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// case��
int Statement_case::Analyze(Compiler* arg_compiler) 
{
	arg_compiler->SetLabel(label_);
	return 0;
}

// case���̑O����
int Statement_case::case_Analyze(Compiler* arg_compiler, int* arg_default_label)
{
	label_ = arg_compiler->MakeLabel();
	if (node->Op() != OP_INT)
		arg_compiler->error("case ���ɂ͒萔�̂ݎw��ł��܂��B");
	node->Push(arg_compiler);
	arg_compiler->OpTest(label_);
	return 0;
}

void Statement_case::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// default��
int Statement_default::Analyze(Compiler* arg_compiler) 
{
	arg_compiler->SetLabel(label_);
	return 0;
}

// default���̑O����
int Statement_default::case_Analyze(Compiler* arg_compiler, int* arg_default_label)
{
	label_ = arg_compiler->MakeLabel();
	*arg_default_label = label_;

	return 0;
}

// break��
int Statement_break::Analyze(Compiler* arg_compiler) 
{
	if (!arg_compiler->JmpBreakLabel()) {
		arg_compiler->error("break��switch/for/while�O�ɗL��܂�");
	}
	return 0;
}


// return��
int Statement_return::Analyze(Compiler* arg_compiler) 
{

	if (arg_compiler->GetCurrentFunctionType() == TYPE_VOID) {	// �߂�l����
		if (node != 0) {
			arg_compiler->error("void�֐��ɖ߂�l���ݒ肳��Ă��܂�");
		}
		arg_compiler->OpReturn();
	}
	else {
		if (node == 0) {
			arg_compiler->error("�֐��̖߂�l������܂���");
		}
		else {
			int node_type = node->Push(arg_compiler);		// �߂�l��push

			if (!CanTypeCast( node_type ,arg_compiler->GetCurrentFunctionType())) {
				arg_compiler->error("�߂�l�̌^�������܂���");
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

// if��
int Statement_if::Analyze(Compiler* arg_compiler) 
{
	node->Push(arg_compiler);
	int label1 = arg_compiler->MakeLabel();
	arg_compiler->OpJmpNC(label1);
	vec_statement[0]->Analyze(arg_compiler);

	if (vec_statement[1]) {
		int label2 = arg_compiler->MakeLabel();
		arg_compiler->OpJmp(label2);
		arg_compiler->SetLabel(label1);
		vec_statement[1]->Analyze(arg_compiler);
		arg_compiler->SetLabel(label2);
	}
	else {
		arg_compiler->SetLabel(label1);
	}
	return 0;
}

void Statement_if::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	vec_statement[0]->LambdaCapture(arg_captureList, arg_compiler);
	if (vec_statement[1]) {
		vec_statement[1]->LambdaCapture(arg_captureList, arg_compiler);
	}
}

// for��
int Statement_for::Analyze(Compiler* arg_compiler) 
{
	int label1 = arg_compiler->MakeLabel();
	int label2 = arg_compiler->MakeLabel();

	int break_label = arg_compiler->SetBreakLabel(label2);
	if(node[0])
		node[0]->Push(arg_compiler);
	arg_compiler->SetLabel(label1);
	node[1]->Push(arg_compiler);
	arg_compiler->OpJmpNC(label2);
	vec_statement->Analyze(arg_compiler);
	if (node[2])
		node[2]->Push(arg_compiler);
	arg_compiler->OpJmp(label1);
	arg_compiler->SetLabel(label2);

	arg_compiler->SetBreakLabel(break_label);
	return 0;
}

void Statement_for::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	for (int i = 0; i < sizeof(node) / sizeof(node[0]); i++) {
		node[i]->LambdaCapture(arg_captureList, arg_compiler);
	}
}

// while��
int Statement_while::Analyze(Compiler* arg_compiler) 
{
	int label1 = arg_compiler->MakeLabel();
	int label2 = arg_compiler->MakeLabel();

	int break_label = arg_compiler->SetBreakLabel(label2);

	arg_compiler->SetLabel(label1);
	node->Push(arg_compiler);
	arg_compiler->OpJmpNC(label2);
	vec_statement->Analyze(arg_compiler);
	arg_compiler->OpJmp(label1);
	arg_compiler->SetLabel(label2);

	arg_compiler->SetBreakLabel(break_label);
	return 0;
}

void Statement_while::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// switch��
int Statement_switch::Analyze(Compiler* arg_compiler) 
{
	if (!vec_statement.empty()) {
		node->Push(arg_compiler);

		int label = arg_compiler->MakeLabel();		// L0���x���쐬
		int break_label = arg_compiler->SetBreakLabel(label);
		int default_label = label;

		std::for_each(vec_statement.begin(), vec_statement.end(),
			boost::bind(&Statement::Case_Analyze, _1, arg_compiler, &default_label));

		arg_compiler->OpPop();
		arg_compiler->OpJmp(default_label);

		std::for_each(vec_statement.begin(), vec_statement.end(), boost::bind(&Statement::Analyze, _1, arg_compiler));
		arg_compiler->SetLabel(label);

		arg_compiler->SetBreakLabel(break_label);
	}
	return 0;
}

void Statement_switch::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	node->LambdaCapture(arg_captureList, arg_compiler);
}

// block��
int Statement_block::Analyze(Compiler* arg_compiler) 
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

// ���u���b�N
int Block::Analyze(Compiler* arg_compiler, std::vector<Function_t>& arg_captureCheck)
{
	auto ret = 0;
	{


		for (auto itr = vec_decl.begin(), endItr = vec_decl.end(); itr != endItr; itr++) {
			(*itr)->Define(arg_compiler);
		}
	}

	for (auto itr = arg_captureCheck.begin(), end = arg_captureCheck.end(); itr != end;itr++) {
		(*itr)->LambdaCapture(arg_compiler);
	}

	if (!vec_decl.empty())
		arg_compiler->AllocStack();	// �X�^�b�N�t���[���m��

	{
		for (auto itr = vec_state.begin(), endItr = vec_state.end(); itr != endItr; itr++) {
			if ((ret = (*itr)->Analyze(arg_compiler)) != 0) {
				return ret;
			}
		}
	}

	return ret;
}


void Block::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList,Compiler* arg_compiler)
{
	for (auto itr = vec_state.begin(), endItr = vec_state.end(); itr != endItr; itr++) {
		(*itr)->LambdaCapture(arg_captureList, arg_compiler);
	}
}

int Declaration::PushCompiler(Compiler* arg_compiler)
{
	return Analyze(arg_compiler);
}

// �錾�̉��
int Declaration::Analyze(Compiler* arg_compiler) 
{
	if (isFunction) {		// �֐�
		arg_compiler->FunctionDefine(valueType, name, vec_argType);
	}
	else {
		arg_compiler->ValueDefine(valueType, vec_node,accessType);

		auto type = arg_compiler->GetType(valueType&~TYPE_REF);
		int size = vec_node.size();
		if (valueType & TYPE_REF) {
			if (type->isSystem) {
				for (int i = 0; i < size; i++) {
					arg_compiler->OpAllocStack_Ref(valueType);
				}
			}
			else if (type->p_enumTag) {
				for (int i = 0; i < size; i++) {
					arg_compiler->OpAllocStack_Ref_EnumType(valueType);
				}
			}
			else if (type->IsFunctionObjectType()) {
				for (int i = 0; i < size; i++) {
					arg_compiler->OpAllocStack_Ref_FunctionType(valueType);
				}
			}
			else {
				for (int i = 0; i < size; i++) {
					arg_compiler->OpAllocStack_Ref_ScriptType((valueType & ~TYPE_REF) - arg_compiler->GetSystemTypeSize());
				}
			}
		}
		else {
			if (type->isSystem) {
				for (int i = 0; i < size; i++) {
					arg_compiler->OpAllocStack(valueType);
				}
			}
			else if (type->p_enumTag) {
				for (int i = 0; i < size; i++) {
					arg_compiler->OpAllocStackEnumType(valueType);
				}
			}
			else if (type->IsFunctionObjectType()) {
				for (int i = 0; i < size; i++) {
					arg_compiler->OpAllocStackFunctionType(valueType);
				}
			}
			else {
				for (int i = 0; i < size; i++) {
					arg_compiler->OpAllocStack_ScriptType(valueType - arg_compiler->GetSystemTypeSize());
				}
			}
		}
	}
	return 0;
}


void Declaration::Define(Compiler* arg_compiler)
{
	if (isFunction) {		// �֐�
		arg_compiler->FunctionDefine(valueType, name, vec_argType);
	}
	else {
		arg_compiler->ValueDefine(valueType, vec_node,accessType);
	}
}
int Node_Member::Push(Compiler* arg_compiler) const
{
	if (op != OP_MEMBER) {
		arg_compiler->error("�����G���[�F�����o�ϐ��m�[�h�Ƀ����o�ϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		leftNode->Push(arg_compiler);
		auto typeTag = arg_compiler->GetType(leftNode->GetType(arg_compiler) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(strData).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName+"��" + strData + "�ɃA�N�Z�X�o���܂���");
		}

		arg_compiler->PushMemberRef(typeTag->map_memberValue.at(strData).index);
		return   typeTag->map_memberValue.at(strData).type & ~TYPE_REF;
	}
	return -1;
}
int Node_Member::PushClone(Compiler* arg_compiler) const
{
	if (op != OP_MEMBER) {
		arg_compiler->error("�����G���[�F�����o�ϐ��m�[�h�Ƀ����o�ϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		leftNode->Push(arg_compiler);
		auto typeTag = arg_compiler->GetType(leftNode->GetType(arg_compiler) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(strData).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName + "��" + strData + "�ɃA�N�Z�X�o���܂���");
		}
		arg_compiler->PushMember(typeTag->map_memberValue.at(strData).index);
		return   typeTag->map_memberValue.at(strData).type & ~TYPE_REF;

	}
	return -1;
}
int Node_Member::Pop(Compiler* arg_compiler) const
{
	if (op != OP_MEMBER) {
		arg_compiler->error("�����G���[�F�����o�ϐ��m�[�h�Ƀ����o�ϐ��ȊO���o�^����Ă��܂��B");
	}
	else {

		//�^
		auto typeTag = arg_compiler->GetType(leftNode->GetType(arg_compiler));
		if (typeTag->map_memberValue.at(strData).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName + "��" + strData + "�ɃA�N�Z�X�o���܂���");
		}
		
		leftNode->Push(arg_compiler);
		auto type = typeTag->map_memberValue.at(strData).type;
		if (type & TYPE_REF) {
			arg_compiler->PopMemberRef(typeTag->map_memberValue.at(strData).index);
		}
		else {
			arg_compiler->PopMember(typeTag->map_memberValue.at(strData).index);
		}

		return   typeTag->map_memberValue.at(strData).type & ~TYPE_REF;
		
	}
	return TYPE_INTEGER;
}
int Node_Member::GetType(Compiler* arg_compiler) const
{
	if (op != OP_MEMBER) {
		arg_compiler->error("�����G���[�F�����o�ϐ��m�[�h�Ƀ����o�ϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		//�ϐ��̃����o�ϐ�
		if (leftNode->Op() == OP_IDENTIFIER|| leftNode->Op() == OP_MEMBER) {
			{

				//�^
				auto typeTag = arg_compiler->GetType(leftNode->GetType(arg_compiler));
				if (!typeTag->map_memberValue.count(strData)) {
					arg_compiler->error("�\���G���[�F"+typeTag->typeName+"�Ƀ����o�ϐ�"+strData+"�͑��݂��܂���");
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
int Node_Method::Push(Compiler* arg_compiler) const
{
	if (op != OP_METHOD) {
		arg_compiler->error("�����G���[�F�����o�֐��m�[�h�Ƀ����o�֐��ȊO���o�^����Ă��܂��B");
	}

	std::vector<int> argTypes;
	if (node_list_) {
		auto end = node_list_->vec_args.end();
		for (auto itr = node_list_->vec_args.begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(arg_compiler));
		}
	}

	int argSize = argTypes.size();
	const TypeTag* typeTag = nullptr;
	const FunctionTag* methodTag = nullptr;


	typeTag = arg_compiler->GetType(leftNode->GetType(arg_compiler) & ~TYPE_REF);
	methodTag = typeTag->methods.Find(strData, argTypes);
	if (methodTag->accessType != AccessModifier::Public&&arg_compiler->GetCurrentThisType()!=typeTag) {

		arg_compiler->error(typeTag->typeName + "�@��" + methodTag->name + "()�̓A�N�Z�X�o���܂���");
	}

	if (methodTag == nullptr) {
		std::string message = "";
		if (argSize) {
			for (int i = 0; i < argSize; i++) {
				message += arg_compiler->GetTypeName(argTypes[i]) + " ";
			}
			message += "�������ɂƂ�";
		}
		message += "�֐�" + strData + "�͖��錾�ł�";
		arg_compiler->error(message);
		return -1;
	}

	// ������push
	if (node_list_&& methodTag->ArgSize() == argSize) {
		std::for_each(node_list_->vec_args.begin(), node_list_->vec_args.end(), set_arg(arg_compiler, methodTag));
	}


	leftNode->Push(arg_compiler);


	if (methodTag->IsSystem()) {
		// �����̐���push
		arg_compiler->PushConstInt(argSize);
		arg_compiler->OpSysMethodCall(methodTag->GetIndex());		// �g�ݍ��݃��\�b�h
	}
	else {
		// �����̐�+this��push
		arg_compiler->PushConstInt(argSize+1);
		arg_compiler->OpCall(methodTag->GetIndex());			// �X�N���v�g��̃��\�b�h
	}


	return methodTag->valueType;
}
int Node_Method::Pop(Compiler* arg_compiler) const
{
	arg_compiler->error("�����G���[�F�����o�֐��m�[�h��pop");
	return TYPE_INTEGER;
}
int Node_Method::GetType(Compiler* arg_compiler) const
{
	if (op != OP_METHOD) {
		arg_compiler->error("�����G���[�F�����o�֐��m�[�h�Ƀ����o�֐��ȊO���o�^����Ă��܂��B");
	}

	std::vector<int> argTypes;

	if (node_list_) {
		auto end = node_list_->vec_args.end();
		for (auto itr = node_list_->vec_args.begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(arg_compiler));
		}
	}


	const TypeTag* typeTag = arg_compiler->GetType(leftNode->GetType(arg_compiler) & ~TYPE_REF);
	const FunctionTag* tag = typeTag->methods.Find(strData, argTypes);

	return tag->valueType;
}
void Node_Method::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	if (node_list_ ) {
		node_list_->LambdaCapture(arg_captureList, arg_compiler);
	}


	leftNode->LambdaCapture(arg_captureList, arg_compiler);
}

void NodeList::LambdaCapture(std::map<std::string, const ValueTag*>& arg_captureList, Compiler* arg_compiler) const
{
	for (auto itr = vec_args.begin(), end = vec_args.end(); itr != end; itr++) {
		(*itr)->LambdaCapture(arg_captureList, arg_compiler);
	}
}
int Node_enum::Push(Compiler* arg_compiler) const
{

	auto enumType = GetEnumType(arg_compiler,*leftNode);
	if (enumType == nullptr) {

		arg_compiler->error("�񋓌^�@" + leftNode->GetString() + "�͖���`�ł�");
		return -1;
	}
	if (!enumType->ExistenceIdentifers(strData)) {

		arg_compiler->error("�񋓌^�@" + leftNode->GetString()+"."+strData + "�͖���`�ł�");
		return -1;
	}
	arg_compiler->PushConstInt( enumType->GetValue(strData));

	return TYPE_INTEGER;
}
int Node_enum::Pop(Compiler* arg_compiler) const
{
	arg_compiler->error("�����G���[�F�񋓌^�m�[�h��pop");
	return -1;
}
int Node_enum::GetType(Compiler* arg_compiler) const
{
	return TYPE_INTEGER;
}
int Node_enum::EnumType(Compiler* arg_compiler) const
{
	auto type=arg_compiler->GetType(leftNode->GetString());
	if (!type) {
		arg_compiler->error("�񋓌^�@" + leftNode->GetString() + "." + strData + "�͖���`�ł�");
		return 0;
	}
	return type->typeIndex;
}

int Node_FunctionObject::Push(Compiler* arg_compiler) const
{
	auto funcTag = GetFunctionType(arg_compiler, *this);

	if (!funcTag) {
		arg_compiler->error(GetString() + "�͖���`�ł�");
		return -1;
	}
	arg_compiler->PushConstInt(funcTag->GetIndex());
	return arg_compiler->GetfunctionTypeIndex(funcTag->vec_args, funcTag->valueType);
}
int Node_FunctionObject::Pop(Compiler* arg_compiler) const
{
	arg_compiler->error("�����G���[�F�֐��^�m�[�h��pop");
	return -1;
}
int Node_FunctionObject::GetType(Compiler* arg_compiler) const
{
	auto funcTag = GetFunctionType(arg_compiler, *this);
	
	if (!funcTag) {
		arg_compiler->error(GetString() + "�͖���`�ł�");
		return 0;
	}

	return arg_compiler->GetfunctionTypeIndex(funcTag->vec_args, funcTag->valueType);
}
void Enum::SetIdentifer(const std::string& arg_name)
{
	map_identifer.emplace(arg_name, map_identifer.size());
}
void Enum::SetIdentifer(const std::string& arg_name, const int arg_value)
{
	map_identifer.emplace(arg_name, arg_value);
}
int Enum::Analyze(Compiler* arg_compiler)
{
	arg_compiler->RegistEnumType(typeName);
	auto tag = arg_compiler->GetEnumTag(arg_compiler->GetCurrentNameSpace()->GetGlobalNameString()+ typeName);
	auto end = map_identifer.end();
	for (auto itr = map_identifer.begin(); itr != end; itr++) {
		tag->SetValue(itr->first, itr->second);
	}
	return 0;
}
int Class::Analyze(Compiler* arg_compiler)
{
	arg_compiler->AnalyzeScriptType(name, map_values);
	auto typeTag = arg_compiler->GetType(name);
	auto methodTable = &typeTag->methods;

	arg_compiler->PushCurrentThisType(typeTag);
	for (auto itr = vec_methods.begin(), end = vec_methods.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler, methodTable);
	}
	arg_compiler->PopCurrentThisType();
	vec_methods.clear();
	return 0;
}

int Class::PushCompiler(Compiler* arg_compiler)
{
	arg_compiler->PushAnalyzeClass(shared_from_this());
	return 0;
}
int Class::Regist(Compiler* arg_compiler)
{
	arg_compiler->RegistScriptType(name);
	return 0;
}
void Class::RegistMethod(Function_t arg_method, Compiler* arg_compiler)
{
	auto typeTag = arg_compiler->GetType(name);
	auto methodTable = &typeTag->methods;
	arg_method->PushCompiler(arg_compiler,methodTable);
	vec_methods.push_back(arg_method);
	
}
void Class::SetValue(const std::string& arg_name, const int arg_type, const AccessModifier arg_accessType)
{
	std::pair<int, AccessModifier> v = { arg_type,arg_accessType };
	map_values.emplace(arg_name,v);
}

int Function::PushCompiler(Compiler* arg_compiler, FunctionTable* arg_p_funcTable )
{
	ownNameSpace = arg_compiler->GetCurrentNameSpace();
	arg_compiler->PopAnalyzeFunction();
	arg_compiler->RegistFunction(valueType, name, args, block, accessType,arg_p_funcTable);
	if (!arg_p_funcTable) {
		arg_compiler->GetCurrentNameSpace()->PushFunction(shared_from_this());
	}
	serchName =arg_p_funcTable?name:  arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + name;
	
	return 0;
}

int Function::PushCompiler_sub(Compiler* arg_compiler)
{
	ownNameSpace = arg_compiler->GetCurrentNameSpace();
	arg_compiler->PopAnalyzeFunction();
	arg_compiler->RegistFunction(valueType, name, args, block, accessType);
	serchName = arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + name;
	arg_compiler->PushSubFunction(shared_from_this());
	return 0;
}


// �����̕ϐ�����o�^
struct add_value {
	ButiScript::Compiler* p_compiler;
	ButiScript::ValueTable& values;
	int addr;
	add_value(ButiScript::Compiler* arg_p_comp, ButiScript::ValueTable& arg_values, const int arg_addres = argmentAddressStart) : p_compiler(arg_p_comp), values(arg_values), addr(arg_addres)
	{
	}

	void operator()(const ButiScript::ArgDefine& arg_argDefine) const
	{
		if (!values.add_arg(arg_argDefine.GetType(), arg_argDefine.GetName(), addr)) {
			p_compiler->error("���� " + arg_argDefine.GetName() + " �͊��ɓo�^����Ă��܂��B");
		}
	}
};
// �L���v�`���ϐ���o�^
struct add_capture {
	ButiScript::Compiler* p_compiler;
	ButiScript::ValueTable& values;
	int addr;
	add_capture(ButiScript::Compiler* arg_p_comp, ButiScript::ValueTable& arg_values, const int arg_addres = argmentAddressStart) : p_compiler(arg_p_comp), values(arg_values), addr(arg_addres)
	{
	}

	void operator()(const ButiScript::ArgDefine& arg_argDefine) const
	{
		if (!values.add_capture(arg_argDefine.GetType(), arg_argDefine.GetName(), addr)) {
			p_compiler->error("�L���v�`���ϐ� " + arg_argDefine.GetName() + " �͊��ɓo�^����Ă��܂��B");
		}
	}
};

// �֐��̉��
int Function::Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable)
{
	auto currentNameSpace = arg_compiler->GetCurrentNameSpace();
	ownNameSpace =ownNameSpace?ownNameSpace: currentNameSpace;
	arg_compiler->SetCurrentNameSpace(ownNameSpace);
	FunctionTable* p_functable = arg_p_funcTable ? arg_p_funcTable : &arg_compiler->GetFunctions();


	FunctionTag* tag = p_functable->Find_strict(serchName, args);
	if (tag) {
		if (tag->IsDefinition()) {
			arg_compiler-> error("�֐� " + serchName+ " �͊��ɒ�`����Ă��܂�");
			return 0;
		}
		if (tag->IsDeclaration() && !tag->CheckArgList_strict(args)) {
			arg_compiler-> error("�֐� " + serchName + " �ɈقȂ�^�̈������w�肳��Ă��܂�");
			return 0;
		}
		tag->SetDefinition();	// ��`�ς݂ɐݒ�
	}
	else {
		FunctionTag func(valueType, serchName);
		func.SetArgs(args);				// ������ݒ�
		func.SetDefinition();			// ��`�ς�
		func.SetIndex(arg_compiler-> MakeLabel());		// ���x���o�^
		tag = p_functable->Add(serchName, func);
		if (tag == nullptr)
			arg_compiler->error("�����G���[�F�֐��e�[�u���ɓo�^�ł��܂���");
	}

	arg_compiler->PushCurrentFunctionName(serchName);		// �������̊֐�����o�^
	arg_compiler->PushCurrentFunctionType(  valueType);		// �������̊֐��^��o�^

	// �֐��̃G���g���[�|�C���g�Ƀ��x����u��

	arg_compiler->SetLabel(tag->GetIndex());

	arg_compiler-> BlockIn(false,true);		// �ϐ��X�^�b�N�𑝂₷

	// �������X�g��o�^
	int address = argmentAddressStart;
	//�����o�֐��̏ꍇthis�������ɒǉ�
	if (arg_p_funcTable) {
		ArgDefine argDef(arg_compiler-> GetCurrentThisType()->typeIndex, thisPtrName);
		add_value(arg_compiler, arg_compiler->GetValueTable() .back(), address)(argDef);
		address--;
	}
	
	for (auto itr = args.rbegin(), endItr = args.rend(); itr != endItr; itr++) {
		add_value(arg_compiler, arg_compiler->GetValueTable().back(), address)(*itr);
		address--;
	}
	arg_compiler->ValueAddressSubtract(address);

	// ��������΁A����o�^
	if (block) {
		int ret = block->Analyze(arg_compiler, vec_subFunctions);
	}

	const VMCode& code = arg_compiler->GetStatement() .back();
	if (valueType == TYPE_VOID) {
		if (code.op != VM_RETURN)		// return���������return��ǉ�
			arg_compiler-> OpReturn();
	}
	else {
		if (code.op != VM_RETURNV) {
			arg_compiler->error("�֐� " + serchName + " �̍Ō��return�����L��܂���B");
		}
	}


	for (auto itr = vec_subFunctions.begin(), end = vec_subFunctions.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler, nullptr);
	}
	vec_subFunctions.clear();
	arg_compiler->ValueAddressAddition(-address);
	arg_compiler->BlockOut();		// �ϐ��X�^�b�N�����炷

	arg_compiler->PopCurrentFunctionName();		// �������̊֐���������
	arg_compiler->PopCurrentFunctionType();		
	arg_compiler->SetCurrentNameSpace(currentNameSpace);
	return 0;
}

void Function::AddSubFunction(Function_t arg_function)
{
	vec_subFunctions.push_back(arg_function);
}


Lambda::Lambda(const int arg_type,const std::vector<ArgDefine>& arg_vec_argDefine, Compiler* arg_compiler)
{
	valueType = arg_type;
	args = arg_vec_argDefine;

	lambdaIndex = arg_compiler->GetLambdaCount();
	arg_compiler->IncreaseLambdaCount();
	name = "@lambda:" + std::to_string(lambdaIndex);
}
int Lambda::PushCompiler(Compiler* arg_compiler)
{
	auto typeTag = arg_compiler->GetType(valueType);

	arg_compiler->PopAnalyzeFunction();
	arg_compiler->RegistLambda(typeTag->GetFunctionObjectReturnType(),name, args, nullptr);
	serchName = arg_compiler->GetCurrentNameSpace()->GetGlobalNameString() + name;

	arg_compiler->PushSubFunction(shared_from_this());

	return lambdaIndex;
}
int Lambda::Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable)
{
	auto typeTag = arg_compiler->GetType(valueType);
	valueType = typeTag->GetFunctionObjectReturnType();
	auto currentNameSpace = arg_compiler->GetCurrentNameSpace();
	ownNameSpace = ownNameSpace ? ownNameSpace : currentNameSpace;
	arg_compiler->SetCurrentNameSpace(ownNameSpace);
	FunctionTable* p_functable = arg_p_funcTable ? arg_p_funcTable : &arg_compiler->GetFunctions();


	FunctionTag* tag = p_functable->Find_strict(serchName, args);
	if (tag) {
		if (tag->IsDefinition()) {
			arg_compiler->error("�֐� " + serchName + " �͊��ɒ�`����Ă��܂�");
			return 0;
		}
		if (tag->IsDeclaration() && !tag->CheckArgList_strict(args)) {
			arg_compiler->error("�֐� " + serchName + " �ɈقȂ�^�̈������w�肳��Ă��܂�");
			return 0;
		}
		tag->SetDefinition();	// ��`�ς݂ɐݒ�
	}
	else {
		FunctionTag func(valueType, serchName);
		func.SetArgs(args);				// ������ݒ�
		func.SetDefinition();			// ��`�ς�
		func.SetIndex(arg_compiler->MakeLabel());		// ���x���o�^
		tag = p_functable->Add(serchName, func);
		if (tag == nullptr)
			arg_compiler->error("�����G���[�F�֐��e�[�u���ɓo�^�ł��܂���");
	}

	arg_compiler->PushCurrentFunctionName(serchName);		// �������̊֐�����o�^
	arg_compiler->PushCurrentFunctionType(valueType);		// �������̊֐��^��o�^

	// �֐��̃G���g���[�|�C���g�Ƀ��x����u��

	arg_compiler->SetLabel(tag->GetIndex());

	arg_compiler->BlockIn(false, true);		// �ϐ��X�^�b�N�𑝂₷

	// �������X�g��o�^
	int address = argmentAddressStart;
	//�����o�֐��̏ꍇthis�������ɒǉ�
	if (arg_p_funcTable) {
		ArgDefine argDef(arg_compiler->GetCurrentThisType()->typeIndex, thisPtrName);
		add_value(arg_compiler, arg_compiler->GetValueTable().back(), address)(argDef);
		address--;
	}

	for (auto itr = args.rbegin(), endItr = args.rend(); itr != endItr; itr++) {
		add_value(arg_compiler, arg_compiler->GetValueTable().back(), address)(*itr);
		address--;
	}
	arg_compiler->ValueAddressSubtract(address);

	// ��������΁A����o�^
	if (block) {

		///�L���v�`������ϐ����m��
		int i = 0;
		for (auto itr = map_lambdaCapture.begin(), end = map_lambdaCapture.end(); itr!= end;i++, itr++) {
			add_capture(arg_compiler, arg_compiler->GetValueTable().back(), i)(ArgDefine(itr->second->valueType,arg_compiler->GetCurrentNameSpace()->GetGlobalNameString()+ itr->first));
			//tag->vec_captureList.push_back(itr->second->GetAddress());
		}

		arg_compiler->BlockIn();
		///
		int ret = block->Analyze(arg_compiler,vec_subFunctions);
		arg_compiler->BlockOut();
	}

	const VMCode& code = arg_compiler->GetStatement().back();
	if (valueType == TYPE_VOID) {
		if (code.op != VM_RETURN)		// return���������return��ǉ�
			arg_compiler->OpReturn();
	}
	else {
		if (code.op != VM_RETURNV) {
			arg_compiler->error("�֐� " + serchName + " �̍Ō��return�����L��܂���B");
		}
	}


	for (auto itr = vec_subFunctions.begin(), end = vec_subFunctions.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler, nullptr);
	}
	vec_subFunctions.clear();
	arg_compiler->ValueAddressAddition(-address);
	arg_compiler->BlockOut();		// �ϐ��X�^�b�N�����炷

	arg_compiler->PopCurrentFunctionName();		// �������̊֐���������
	arg_compiler->PopCurrentFunctionType();
	arg_compiler->SetCurrentNameSpace(currentNameSpace);

	return lambdaIndex;
}
void Lambda::LambdaCapture(Compiler* arg_compiler)
{
	if (block) {

		FunctionTable* p_functable = &arg_compiler->GetFunctions();


		FunctionTag* tag = p_functable->Find_strict(serchName, args);
		block->LambdaCapture(map_lambdaCapture, arg_compiler);

		///�L���v�`������ϐ����m��
		int i = 0;
		for (auto itr = map_lambdaCapture.begin(), end = map_lambdaCapture.end(); itr != end; i++, itr++) {
			tag->vec_captureList.push_back(itr->second->GetAddress());
		}

	}
}
}