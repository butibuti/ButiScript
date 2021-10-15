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


const EnumTag* GetEnumType(const Compiler* arg_compiler, Node& left_) {

	auto shp_namespace = arg_compiler->GetCurrentNameSpace();
	std::string serchName;
	const  EnumTag* enumType = nullptr;
	while (!enumType)
	{
		serchName = shp_namespace->GetGlobalNameString() + left_.GetString();

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
const FunctionTag* GetFunctionType(const Compiler* arg_compiler, const Node& left_) {

	auto shp_namespace = arg_compiler->GetCurrentNameSpace();
	std::string serchName;
	const  FunctionTag* tag = nullptr;
	while (!tag)
	{
		serchName = shp_namespace->GetGlobalNameString() + left_.GetString();

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
const FunctionTag* GetFunctionType(const Compiler* arg_compiler, const std::string& str) {

	auto shp_namespace = arg_compiler->GetCurrentNameSpace();
	std::string serchName;
	const  FunctionTag* tag = nullptr;
	while (!tag)
	{
		serchName = shp_namespace->GetGlobalNameString() + str;

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

// �ϐ��m�[�h�𐶐�
Node_t Node::make_node(const int Op, const std::string& str, const Compiler* arg_compiler)
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

// �P�����Z�q�̃m�[�h�𐶐�
Node_t Node::make_node(const int Op, Node_t left, const Compiler* arg_compiler)
{
	if (Op == OP_METHOD) {
		return Node_t(new Node_Method(Op, left->GetLeft(), left->GetString()));
	}
	switch (Op) {
	case OP_NEG:
		if (left->op == OP_INT) {			// �萔���Z���v�Z����
			left->number_ = -left->number_;
			return left;
		}
		break;
	}
	return Node_t(new Node(Op, left));
}


Node_t Node::make_node(const int Op, Node_t left, const std::string arg_memberName,const Compiler* arg_compiler)
{
	if (GetEnumType(arg_compiler,*left)) {
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




// �񍀉��Z�q�̃m�[�h�𐶐�
Node_t Node::make_node(const int Op, Node_t left, Node_t right)
{
	// �z��m�[�h�́Aleft�m�[�h��left_�����o�ɉ�����
	if (Op == OP_ARRAY) {
		left->left_ = right;
		return left;
	}
	auto leftOp = left->op;
	auto rightOp = right->op;

	// �萔���Z���v�Z����
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
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				left->number_ /= right->number_;
			}
			break;

		case OP_MOD:
			if (right->number_ == 0) {
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
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

	// �萔�����������Z���v�Z����

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
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				left->num_float /= right->num_float;
			}
			break;

		case OP_MOD:
			if (right->num_float == 0) {
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
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
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				left->number_ /= (int)right->num_float;
			}
			break;

		case OP_MOD:
			if ((int)right->num_float == 0) {
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
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
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
			}
			else {
				left->num_float /= right->number_;
			}
			break;

		case OP_MOD:
			if (right->number_ == 0) {
				std::cerr << "�萔�v�Z��0�ŏ��Z���܂����B" << std::endl;
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

	// �����񓯎m�̒萔�v�Z
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
			std::cerr << "�����񓯎m�ł͂ł��Ȃ��v�Z�ł��B" << std::endl;
			break;
		}
		return Node_t(new Node(OP_INT, Value));
	}
	return Node_t(new Node(Op, left, right));
}

// �֐��m�[�h�̐���
Node_t Node::make_node(const int Op, Node_t left, NodeList_t right)
{
	if (Op == OP_METHOD) {
		return  Node_t(new Node_Method(Op, left, right));
	}
	return Node_t(new Node_function(Op, left, right));
}



template <typename T>
bool SetDefaultOperator(const int op, Compiler* arg_compiler) {
	switch (op) {
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
bool SetDeferentTypeMulOperator(const int op, Compiler* arg_compiler) {
	if (op == OP_MUL) {

		arg_compiler->OpMul<T, U>();
		return true;
	}
	return false;
}
template <typename T, typename U>
bool SetDeferentTypeDivOperator(const int op, Compiler* arg_compiler) {
	if (op == OP_DIV) {

		arg_compiler->OpDiv<T, U>();
		return true;
	}
	return false;
}

template <typename T>
bool SetModOperator(const int op, Compiler* arg_compiler) {
	if (op == OP_MOD) {
		arg_compiler->OpMod<T>();
		return true;
	}
	return false;
}
bool SetLogicalOperator(const int op, Compiler* arg_compiler) {
	switch (op) {
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
bool SetDefaultAssignOperator(const int op, Compiler* arg_compiler) {
	switch (op) {
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
bool SetDeferentTypeMulAssignOperator(const int op, Compiler* arg_compiler) {
	if (op == OP_MUL_ASSIGN) {

		arg_compiler->OpMul<T, U>();
		return true;
	}
	return false;
}
template <typename T, typename U>
bool SetDeferentTypeDivAssignOperator(const int op, Compiler* arg_compiler) {
	if (op == OP_DIV_ASSIGN) {

		arg_compiler->OpDiv<T, U>();
		return true;
	}
	return false;
}

template <typename T>
bool SetModAssignOperator(const int op, Compiler* arg_compiler) {
	if (op == OP_MOD_ASSIGN) {
		arg_compiler->OpMod<T>();
		return true;
	}
	return false;
}
bool SetLogicalAssignOperator(const int op, Compiler* arg_compiler) {
	switch (op) {
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
		if (left_->Push(arg_compiler) == TYPE_STRING)
			arg_compiler->error("������ɂ͓��ꋖ����Ȃ��v�Z�ł��B");
		arg_compiler->OpNeg();
		return TYPE_INTEGER;

	case OP_INT:
		arg_compiler->PushConstInt(number_);
		return TYPE_INTEGER;
	case OP_FLOAT:
		arg_compiler->PushConstFloat(num_float);
		return TYPE_FLOAT;

	case OP_STRING:
		arg_compiler->PushString(string_);
		return TYPE_STRING;

	case OP_FUNCTION:
		return Call(arg_compiler, string_, nullptr);
	}

	int left_type = left_->Push(arg_compiler);
	int right_type = right_->Push(arg_compiler);

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
		if (left_->GetType(arg_compiler) == TYPE_STRING)
			arg_compiler->error("������ɂ͋�����Ȃ��v�Z�ł��B");
		return TYPE_INTEGER;

	case OP_INT:
		return TYPE_INTEGER;
	case OP_FLOAT:
		return TYPE_FLOAT;

	case OP_STRING:
		return TYPE_STRING;

	case OP_FUNCTION:
		return GetCallType(arg_compiler, string_, nullptr);
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

		return arg_compiler->GetfunctionTypeIndex(funcTag->args_, funcTag->valueType);

	}
	int left_type = left_->GetType(arg_compiler);
	int right_type = right_->GetType(arg_compiler);

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
	return GetCallType(arg_compiler, left_->GetString(), &node_list_->args_);
}
//�m�[�h�̊֐��Ăяo���^�`�F�b�N
int Node::GetCallType(Compiler* arg_compiler, const std::string& name, const std::vector<Node_t>* args)const {

	std::vector<int> argTypes;
	if (args) {
		auto end = args->end();
		for (auto itr = args->begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(arg_compiler));
		}
	}
	std::string  functionName;
	NameSpace_t currentSerchNameSpace = arg_compiler->GetCurrentNameSpace();
	const FunctionTag* tag=nullptr;

	while (!tag)
	{
		if (currentSerchNameSpace) {
			functionName = currentSerchNameSpace->GetGlobalNameString() + name;
		}
		else {
			functionName = name;
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
		left_type = left_->Push(arg_compiler)&~TYPE_REF;
	}
	else {
		left_type = left_->GetType(arg_compiler) & ~TYPE_REF;
	}
	int right_type = right_->Push(arg_compiler) & ~TYPE_REF;



	// float�v�Z�m�[�h�̏���
	if ((left_type == TYPE_FLOAT && right_type == TYPE_FLOAT) || (left_type == TYPE_INTEGER && right_type == TYPE_FLOAT) || (left_type == TYPE_FLOAT && right_type == TYPE_INTEGER)) {
		if (!SetDefaultAssignOperator<float>(op, arg_compiler) && (!SetModAssignOperator<float>(op, arg_compiler)) && (!SetLogicalAssignOperator(op, arg_compiler))) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler);
		return 0;
	}

	// �����v�Z�m�[�h�̏���
	if (left_type == TYPE_INTEGER && right_type == TYPE_INTEGER) {
		if (!SetDefaultAssignOperator<int>(op, arg_compiler) && (!SetModAssignOperator<int>(op, arg_compiler)) && (!SetLogicalAssignOperator(op, arg_compiler))) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler);
		return 0;
	}


	//Vector2�v�Z�m�[�h
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_VOID + 1) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector2>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler);
		return 0;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector2, float>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector2, float>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler); 
		return 0;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector2, int>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector2, int>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler);
		return 0;
	}
	//Vector3�v�Z�m�[�h
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_VOID + 2) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector3>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler);
		return 0;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector3, float>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector3, float>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler);
		return 0;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector3, int>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector3, int>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler);
		return 0;
	}
	//Vector4�v�Z�m�[�h
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_VOID + 3) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector4>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler);
		return 0;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector4, float>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector4, float>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler);
		return 0;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector4, int>(op, arg_compiler) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector4, int>(op, arg_compiler)) {
			arg_compiler->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(arg_compiler);
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
		left_->Pop(arg_compiler);

		return 0;
	}

	if (left_type == right_type) {
		//�����^���m�Ȃ̂ő���\
		left_->Pop(arg_compiler);
		return 0;
	}
	else if (right_->EnumType(arg_compiler) == left_type) {
		left_->Pop(arg_compiler);
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
				valueName = currentSerchNameSpace->GetGlobalNameString() + string_;
			}
			else {
				valueName = string_;
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
			if (valueTag->global_) {		// �O���[�o���ϐ�
				if (left_) {		// �z��
					left_->Push(arg_compiler);
					arg_compiler->PushGlobalArrayRef(valueTag->address);
				}
				else {
					arg_compiler->PushGlobalValueRef(valueTag->address);
				}
			}
			else {					// ���[�J���ϐ�
				if (left_) {		// �z��
					left_->Push(arg_compiler);
					arg_compiler->PushLocalArrayRef(valueTag->address);
				}
				else {
					arg_compiler->PushLocalRef(valueTag->address);
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
			arg_compiler->OpPushFunctionAddress(funcTag->GetIndex());
			return arg_compiler->GetfunctionTypeIndex(funcTag->args_, funcTag->valueType);
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
			if (valueTag->global_) {		// �O���[�o���ϐ�
				if (left_) {		// �z��
					left_->Push(arg_compiler);
					arg_compiler->PushGlobalArray(valueTag->address);
				}
				else {
					arg_compiler->PushGlobalValue(valueTag->address);
				}
			}
			else {					// ���[�J���ϐ�
				if (left_) {		// �z��
					left_->Push(arg_compiler);
					arg_compiler->PushLocalArray(valueTag->address);
				}
				else {
					arg_compiler->PushLocal(valueTag->address);
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
			return arg_compiler->GetfunctionTypeIndex(funcTag->args_, funcTag->valueType);
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
			if (valueTag->global_) {		// �O���[�o���ϐ�
				if (left_) {		// �z��
					left_->Push(arg_compiler);
					arg_compiler->PopArray(valueTag->address);
				}
				else {
					arg_compiler->PopValue(valueTag->address);
				}
			}
			else {					// ���[�J���ϐ�
				if (left_) {		// �z��
					left_->Push(arg_compiler);
					arg_compiler->PopLocalArray(valueTag->address);
				}
				else {
					arg_compiler->PopLocal(valueTag->address);
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

// �֐��Ăяo��
struct set_arg {
	Compiler* p_compiler;
	const std::vector<int>* argTypes_;
	mutable int index_;
	set_arg(Compiler* comp, const FunctionTag* func) : p_compiler(comp), argTypes_(&func->args_), index_(0) {}
	set_arg(Compiler* comp, const std::vector<int>* arg_argTypes) : p_compiler(comp), argTypes_(arg_argTypes), index_(0){}

	void operator()(Node_t node) const
	{
		int type = (*argTypes_)[index_++];
		if ((type & TYPE_REF) != 0) {		// �Q��
			if (node->Op() != OP_IDENTIFIER) {
				p_compiler->error("�Q�ƌ^�����ɁA�ϐ��ȊO�͎w��ł��܂���B");
			}
			else {
				std::string  valueName;
				NameSpace_t currentSerchNameSpace = p_compiler->GetCurrentNameSpace();
				const ValueTag* tag = node->GetValueTag(p_compiler);
				if (tag == nullptr) {
					p_compiler->error("�ϐ� " + node->GetString() + " �͒�`����Ă��܂���B");
				}
				else {
					if (!TypeCheck(tag->valueType ,type) ){
						p_compiler->error("�����̌^�������܂���B");
					}

					if (tag->global_) {
						if (node->GetLeft()) {
							node->GetLeft()->Push(p_compiler);
							p_compiler->PushGlobalArrayRef(tag->address);
						}
						else {
							p_compiler->PushGlobalValueRef(tag->address);
						}
					}
					else {
						if (node->GetLeft()) {
							node->GetLeft()->Push(p_compiler);
							p_compiler->PushLocalArrayRef(tag->address);
						}
						else {
							p_compiler->PushLocalRef(tag->address);
						}
					}
				}
			}
		}
		else {
			if (!TypeCheck( node->PushClone(p_compiler), type)) {
				p_compiler->error("�����̌^�������܂���B");
			}
		}
	}
};

// �֐��Ăяo��
int Node::Call(Compiler* arg_compiler, const std::string& name, const std::vector<Node_t>* args) const
{
	std::string  functionName;
	NameSpace_t currentSerchNameSpace = arg_compiler->GetCurrentNameSpace();

	std::vector<int> argTypes;
	if (args) {
		auto end = args->end();
		for (auto itr = args->begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(arg_compiler));
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
		if (args && functionTag->ArgSize() == argSize) {
			std::for_each(args->begin(), args->end(), set_arg(arg_compiler, functionTag));
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
	auto valueTag = GetValueTag(name, arg_compiler);
	if (valueTag) {
		auto valueType=arg_compiler->GetType(valueTag->valueType);
		if (valueType->IsFunctionObjectType()) {
			// ������push
			if (args && valueType->GetFunctionObjectArgSize() == argSize) {
				auto valueArgTypes = valueType->GetFunctionObjectArgment();
				std::for_each(args->begin(), args->end(), set_arg(arg_compiler,&valueArgTypes ));
			}

			// �����̐���push
			arg_compiler->PushConstInt(argSize);
			if (valueTag->global_) {		// �O���[�o���ϐ�

				arg_compiler->PushGlobalValue(valueTag->address);
			}
			else {		

				arg_compiler->PushLocal(valueTag->address);
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

int Node_function::Push(Compiler* arg_compiler) const
{
	return Call(arg_compiler, left_->GetString(), &node_list_->args_);
}

// �֐���pop�͂ł��Ȃ��̂ŃG���[���b�Z�[�W���o��
int Node_function::Pop(Compiler* arg_compiler) const
{
	arg_compiler->error("�����G���[�F�֐��m�[�h��pop����");
	return TYPE_INTEGER;
}

// ���m�[�h����
Statement_t Statement::make_statement(const int vec_state)
{
	switch (vec_state) {
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

Statement_t Statement::make_statement(const int vec_state, const int)
{
	return make_statement(vec_state);
}

Statement_t Statement::make_statement(const int vec_state, Node_t node)
{
	switch (vec_state) {
	case ASSIGN_STATE:
		return Statement_t(new Statement_assign(node));

	case CASE_STATE:
		return Statement_t(new Statement_case(node));

	case SWITCH_STATE:
		return Statement_t(new Statement_switch(node));

	case CALL_STATE:
		return Statement_t(new ccall_statement(node));
	}

	std::cerr << "�����G���[�F���m�[�h�~�X" << std::endl;
	return Statement_t(new Statement_nop());
}

Statement_t Statement::make_statement(const int vec_state, Block_t block)
{
	switch (vec_state) {
	case BLOCK_STATE:
		return Statement_t(new Statement_block(block));
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
	return vec_node->Assign(arg_compiler);
	
}

int Statement_assign::ReAnalyze(Compiler* arg_compiler)
{
	return vec_node->Assign(arg_compiler);
	
}

// �֐��Ăяo����
int ccall_statement::Analyze(Compiler* arg_compiler) 
{
	int type = vec_node->Push(arg_compiler);

	if (type == -1) {

		return -1;
	}

	if (type != TYPE_VOID)
		arg_compiler->OpPop();			// �߂�l���̂Ă邽�߂�pop

	return 0;
}

void ccall_statement::ReAnalyze(Compiler* arg_compiler) 
{
	int type = vec_node->Push(arg_compiler);

	if (type == -1) {

		arg_compiler->error("��`����Ă��Ȃ��֐����Q�Ƃ��Ă��܂�");
	}

	if (type != TYPE_VOID)
		arg_compiler->OpPop();
}

// case��
int Statement_case::Analyze(Compiler* arg_compiler) 
{
	arg_compiler->SetLabel(label_);
	return 0;
}

// case���̑O����
int Statement_case::case_Analyze(Compiler* arg_compiler, int* default_label)
{
	label_ = arg_compiler->MakeLabel();
	if (vec_node->Op() != OP_INT)
		arg_compiler->error("case ���ɂ͒萔�̂ݎw��ł��܂��B");
	vec_node->Push(arg_compiler);
	arg_compiler->OpTest(label_);
	return 0;
}

// default��
int Statement_default::Analyze(Compiler* arg_compiler) 
{
	arg_compiler->SetLabel(label_);
	return 0;
}

// default���̑O����
int Statement_default::case_Analyze(Compiler* arg_compiler, int* default_label)
{
	label_ = arg_compiler->MakeLabel();
	*default_label = label_;

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

	if (arg_compiler->GetFunctionType() == TYPE_VOID) {	// �߂�l����
		if (vec_node != 0) {
			arg_compiler->error("void�֐��ɖ߂�l���ݒ肳��Ă��܂�");
		}
		arg_compiler->OpReturn();
	}
	else {
		if (vec_node == 0) {
			arg_compiler->error("�֐��̖߂�l������܂���");
		}
		else {
			int node_type = vec_node->Push(arg_compiler);		// �߂�l��push

			if (!CanTypeCast( node_type ,arg_compiler->GetFunctionType())) {
				arg_compiler->error("�߂�l�̌^�������܂���");
			}
		}
		arg_compiler->OpReturnV();
	}

	return 0;
}

// if��
int Statement_if::Analyze(Compiler* arg_compiler) 
{
	vec_node->Push(arg_compiler);
	int label1 = arg_compiler->MakeLabel();
	arg_compiler->OpJmpNC(label1);
	statement_[0]->Analyze(arg_compiler);

	if (statement_[1]) {
		int label2 = arg_compiler->MakeLabel();
		arg_compiler->OpJmp(label2);
		arg_compiler->SetLabel(label1);
		statement_[1]->Analyze(arg_compiler);
		arg_compiler->SetLabel(label2);
	}
	else {
		arg_compiler->SetLabel(label1);
	}
	return 0;
}

// for��
int Statement_for::Analyze(Compiler* arg_compiler) 
{
	int label1 = arg_compiler->MakeLabel();
	int label2 = arg_compiler->MakeLabel();

	int break_label = arg_compiler->SetBreakLabel(label2);
	if(vec_node[0])
		vec_node[0]->Push(arg_compiler);
	arg_compiler->SetLabel(label1);
	vec_node[1]->Push(arg_compiler);
	arg_compiler->OpJmpNC(label2);
	statement_->Analyze(arg_compiler);
	if (vec_node[2])
		vec_node[2]->Push(arg_compiler);
	arg_compiler->OpJmp(label1);
	arg_compiler->SetLabel(label2);

	arg_compiler->SetBreakLabel(break_label);
	return 0;
}

// while��
int Statement_while::Analyze(Compiler* arg_compiler) 
{
	int label1 = arg_compiler->MakeLabel();
	int label2 = arg_compiler->MakeLabel();

	int break_label = arg_compiler->SetBreakLabel(label2);

	arg_compiler->SetLabel(label1);
	vec_node->Push(arg_compiler);
	arg_compiler->OpJmpNC(label2);
	statement_->Analyze(arg_compiler);
	arg_compiler->OpJmp(label1);
	arg_compiler->SetLabel(label2);

	arg_compiler->SetBreakLabel(break_label);
	return 0;
}

// switch��
int Statement_switch::Analyze(Compiler* arg_compiler) 
{
	if (!statement_.empty()) {
		vec_node->Push(arg_compiler);

		int label = arg_compiler->MakeLabel();		// L0���x���쐬
		int break_label = arg_compiler->SetBreakLabel(label);
		int default_label = label;

		std::for_each(statement_.begin(), statement_.end(),
			boost::bind(&Statement::Case_Analyze, _1, arg_compiler, &default_label));

		arg_compiler->OpPop();
		arg_compiler->OpJmp(default_label);

		std::for_each(statement_.begin(), statement_.end(), boost::bind(&Statement::Analyze, _1, arg_compiler));
		arg_compiler->SetLabel(label);

		arg_compiler->SetBreakLabel(break_label);
	}
	return 0;
}

// block��
int Statement_block::Analyze(Compiler* arg_compiler) 
{
	arg_compiler->BlockIn();
	block_->Analyze(arg_compiler);
	arg_compiler->BlockOut();
	return 0;
}

// ���u���b�N
int Block::Analyze(Compiler* arg_compiler) 
{
	auto ret = 0;
	{


		for (auto itr = vec_decl.begin(), endItr = vec_decl.end(); itr != endItr; itr++) {
			(*itr)->Define(arg_compiler);
		}
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

int Declaration::PushCompiler(Compiler* arg_compiler)
{
	return Analyze(arg_compiler);
}

// �錾�̉��
int Declaration::Analyze(Compiler* arg_compiler) 
{
	if (isFunction) {		// �֐�
		arg_compiler->FunctionDefine(valueType, name_, args);
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
		arg_compiler->FunctionDefine(valueType, name_, args);
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
		left_->Push(arg_compiler);
		auto typeTag = arg_compiler->GetType(left_->GetType(arg_compiler) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(string_).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName+"��" + string_ + "�ɃA�N�Z�X�o���܂���");
		}

		arg_compiler->PushMemberRef(typeTag->map_memberValue.at(string_).index);
		return   typeTag->map_memberValue.at(string_).type & ~TYPE_REF;
	}
	return -1;
}
int Node_Member::PushClone(Compiler* arg_compiler) const
{
	if (op != OP_MEMBER) {
		arg_compiler->error("�����G���[�F�����o�ϐ��m�[�h�Ƀ����o�ϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		left_->Push(arg_compiler);
		auto typeTag = arg_compiler->GetType(left_->GetType(arg_compiler) & ~TYPE_REF);
		if (typeTag->map_memberValue.at(string_).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName + "��" + string_ + "�ɃA�N�Z�X�o���܂���");
		}
		arg_compiler->PushMember(typeTag->map_memberValue.at(string_).index);
		return   typeTag->map_memberValue.at(string_).type & ~TYPE_REF;

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
		auto typeTag = arg_compiler->GetType(left_->GetType(arg_compiler));
		if (typeTag->map_memberValue.at(string_).access != AccessModifier::Public && arg_compiler->GetCurrentThisType() != typeTag) {
			arg_compiler->error(typeTag->typeName + "��" + string_ + "�ɃA�N�Z�X�o���܂���");
		}
		
		left_->Push(arg_compiler);
		auto type = typeTag->map_memberValue.at(string_).type;
		if (type & TYPE_REF) {
			arg_compiler->PopMemberRef(typeTag->map_memberValue.at(string_).index);
		}
		else {
			arg_compiler->PopMember(typeTag->map_memberValue.at(string_).index);
		}

		return   typeTag->map_memberValue.at(string_).type & ~TYPE_REF;
		
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
		if (left_->Op() == OP_IDENTIFIER|| left_->Op() == OP_MEMBER) {
			{

				//�^
				auto typeTag = arg_compiler->GetType(left_->GetType(arg_compiler));
				if (!typeTag->map_memberValue.count(string_)) {
					arg_compiler->error("�\���G���[�F"+typeTag->typeName+"�Ƀ����o�ϐ�"+string_+"�͑��݂��܂���");
					return -1;
				}
				return typeTag->map_memberValue.at(string_).type;
			}
		}
	}
}
int Node_Method::Push(Compiler* arg_compiler) const
{
	if (op != OP_METHOD) {
		arg_compiler->error("�����G���[�F�����o�֐��m�[�h�Ƀ����o�֐��ȊO���o�^����Ă��܂��B");
	}

	std::vector<int> argTypes;
	if (node_list_) {
		auto end = node_list_->args_.end();
		for (auto itr = node_list_->args_.begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(arg_compiler));
		}
	}

	int argSize = argTypes.size();
	const TypeTag* typeTag = nullptr;
	const FunctionTag* methodTag = nullptr;


	typeTag = arg_compiler->GetType(left_->GetType(arg_compiler) & ~TYPE_REF);
	methodTag = typeTag->methods.Find(string_, argTypes);
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
		message += "�֐�" + string_ + "�͖��錾�ł�";
		arg_compiler->error(message);
		return -1;
	}

	// ������push
	if (node_list_&& methodTag->ArgSize() == argSize) {
		std::for_each(node_list_->args_.begin(), node_list_->args_.end(), set_arg(arg_compiler, methodTag));
	}


	left_->Push(arg_compiler);


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
		auto end = node_list_->args_.end();
		for (auto itr = node_list_->args_.begin(); itr != end; itr++) {
			argTypes.push_back((*itr)->GetType(arg_compiler));
		}
	}


	const TypeTag* typeTag = arg_compiler->GetType(left_->GetType(arg_compiler) & ~TYPE_REF);
	const FunctionTag* tag = typeTag->methods.Find(string_, argTypes);

	return tag->valueType;
}
int Node_enum::Push(Compiler* arg_compiler) const
{

	auto enumType = GetEnumType(arg_compiler,*left_);
	if (enumType == nullptr) {

		arg_compiler->error("�񋓌^�@" + left_->GetString() + "�͖���`�ł�");
		return -1;
	}
	if (!enumType->ExistenceIdentifers(string_)) {

		arg_compiler->error("�񋓌^�@" + left_->GetString()+"."+string_ + "�͖���`�ł�");
		return -1;
	}
	arg_compiler->PushConstInt( enumType->GetValue(string_));

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
	auto type=arg_compiler->GetType(left_->GetString());
	if (!type) {
		arg_compiler->error("�񋓌^�@" + left_->GetString() + "." + string_ + "�͖���`�ł�");
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
	return arg_compiler->GetfunctionTypeIndex(funcTag->args_, funcTag->valueType);
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

	return arg_compiler->GetfunctionTypeIndex(funcTag->args_, funcTag->valueType);
}
void Enum::SetIdentifer(const std::string& arg_name)
{
	map_identifer.emplace(arg_name, map_identifer.size());
}
void Enum::SetIdentifer(const std::string& arg_name, const int value)
{
	map_identifer.emplace(arg_name, value);
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
	arg_compiler->AnalyzeScriptType(name_, map_values);
	auto typeTag = arg_compiler->GetType(name_);
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
	arg_compiler->RegistScriptType(name_);
	return 0;
}
void Class::RegistMethod(Function_t method, Compiler* arg_compiler)
{
	vec_methods.push_back(method);
	
}
void Class::SetValue(const std::string& arg_name, const int arg_type, const AccessModifier accessType)
{
	std::pair<int, AccessModifier> v = { arg_type,accessType };
	map_values.emplace(arg_name,v);
}

int Function::PushCompiler(Compiler* arg_compiler)
{
	arg_compiler->PushAnalyzeFunction(shared_from_this());
	arg_compiler->RegistFunction(valueType, name_, args_, block_, accessType);
	return 0;
}

// �֐��̉��
int Function::Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable)
{

	arg_compiler->AddFunction(valueType, name_, args_, block_, accessType, arg_p_funcTable);

	return 0;
}


Ramda::Ramda(const int arg_type,const std::vector<ArgDefine>& arg_args)
{
	valueType = arg_type;
	args_ = arg_args;
}
int Ramda::PushCompiler(Compiler* arg_compiler)
{
	auto typeTag = arg_compiler->GetType(valueType);
	auto ramdaCount = arg_compiler->GetRamdaCount();
	name_= "@ramda:" + std::to_string(ramdaCount);
	arg_compiler->RegistRamda(typeTag->GetFunctionObjectReturnType(), args_,nullptr);
	arg_compiler->IncreaseRamdaCount();
	arg_compiler->PushAnalyzeFunction(shared_from_this());
	return ramdaCount;
}
int Ramda::Analyze(Compiler* arg_compiler, FunctionTable* arg_p_funcTable)
{
	auto typeTag = arg_compiler->GetType(valueType);
	auto ramdaCount = arg_compiler->GetRamdaCount();
	arg_compiler->AddRamda (typeTag->GetFunctionObjectReturnType(), args_, block_, arg_p_funcTable);
	return ramdaCount;
}
}