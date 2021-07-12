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


const EnumTag* GetEnumType(const Compiler* c,Node& left_) {

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

// �ϐ��m�[�h�𐶐�
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

// �P�����Z�q�̃m�[�h�𐶐�
Node_t Node::make_node(const int Op, Node_t left)
{
	switch (Op) {
	case OP_NEG:
		if (left->op_ == OP_INT) {			// �萔���Z���v�Z����
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




// �񍀉��Z�q�̃m�[�h�𐶐�
Node_t Node::make_node(const int Op, Node_t left, Node_t right)
{
	// �z��m�[�h�́Aleft�m�[�h��left_�����o�ɉ�����
	if (Op == OP_ARRAY) {
		left->left_ = right;
		return left;
	}
	auto leftOp = left->op_;
	auto rightOp = right->op_;

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



// �m�[�h��push����
int Node::Push(Compiler* c) const{
	if (op_ >= OP_ASSIGN && op_ <= OP_RSHIFT_ASSIGN) {
		return Assign(c);
	}

	switch (op_) {
	case OP_NEG:
		if (left_->Push(c) == TYPE_STRING)
			c->error("������ɂ͓��ꋖ����Ȃ��v�Z�ł��B");
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

	//�E�ӎႵ���͍��ӂ�����`�֐�
	if (left_type <= -1 || right_type <= -1) {
		return -1;
	}

	// float�v�Z�m�[�h�̏���
	if ((left_type == TYPE_FLOAT && right_type == TYPE_FLOAT ) || (left_type == TYPE_INTEGER && right_type == TYPE_FLOAT) || (left_type == TYPE_FLOAT && right_type == TYPE_INTEGER)) {
		if (!SetDefaultOperator<float>(op_, c) && (!SetModOperator<float>(op_, c)) && (!SetLogicalOperator(op_, c))) {
				c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}

		return TYPE_FLOAT;
	}

	// �����v�Z�m�[�h�̏���
	if (left_type ==  TYPE_INTEGER && right_type==TYPE_INTEGER) {
		if (!SetDefaultOperator<int>(op_, c)&&  (!SetModOperator<int>(op_, c))&& (!SetLogicalOperator(op_, c))) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");	
		}
		return TYPE_INTEGER;
	}

	//Vector2�v�Z�m�[�h
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_VOID + 1) {
		if (!SetDefaultOperator<ButiEngine::Vector2>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulOperator<ButiEngine::Vector2, float>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector2, float>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulOperator<ButiEngine::Vector2, int>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector2, int>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_FLOAT && right_type == TYPE_VOID + 1) {
		if (!SetDeferentTypeMulOperator<float, ButiEngine::Vector2>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 1;
	}
	if (left_type == TYPE_INTEGER && right_type == TYPE_VOID + 1) {

		if (!SetDeferentTypeMulOperator<int, ButiEngine::Vector2>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 1;
	}
	//Vector3�v�Z�m�[�h
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_VOID + 2) {
		if (!SetDefaultOperator<ButiEngine::Vector3>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulOperator<ButiEngine::Vector3, float>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector3, float>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulOperator<ButiEngine::Vector3, int>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector3, int>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_FLOAT && right_type == TYPE_VOID + 2) {
		if (!SetDeferentTypeMulOperator<float, ButiEngine::Vector3>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 2;
	}
	if (left_type == TYPE_INTEGER && right_type == TYPE_VOID + 2) {

		if (!SetDeferentTypeMulOperator<int, ButiEngine::Vector3>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 2;
	}
	//Vector4�v�Z�m�[�h
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_VOID + 3) {
		if (!SetDefaultOperator<ButiEngine::Vector4>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulOperator<ButiEngine::Vector4, float>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector4, float>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_VOID +3 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulOperator<ButiEngine::Vector4, int>(op_, c) && !SetDeferentTypeDivOperator<ButiEngine::Vector4, int>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_FLOAT && right_type == TYPE_VOID + 3) {
		if (!SetDeferentTypeMulOperator<float, ButiEngine::Vector4>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 3;
	}
	if (left_type == TYPE_INTEGER && right_type == TYPE_VOID + 3) {

		if (!SetDeferentTypeMulOperator<int, ButiEngine::Vector4>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		return TYPE_VOID + 3;
	}

	// ������v�Z�m�[�h�̏���
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
		c->error("������ł͌v�Z�ł��Ȃ����ł��B");
		break;
	}
	return TYPE_STRING;
}

// �m�[�h��pop
// �v�Z�m�[�h��pop�ł��Ȃ�

int Node::Pop(Compiler* c) const{
	c->error("�����G���[�F�v�Z�m�[�h��pop���Ă��܂��B");
	return TYPE_INTEGER;
}

//�m�[�h�̌^�`�F�b�N
int Node::GetType(Compiler* c)const {
	if (op_ >= OP_ASSIGN && op_ <= OP_RSHIFT_ASSIGN) {
		return -1;
	}

	switch (op_) {
	case OP_NEG:
		if (left_->GetType(c) == TYPE_STRING)
			c->error("������ɂ͋�����Ȃ��v�Z�ł��B");
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
		const ValueTag* tag = GetValueTag(c);
		if (tag == nullptr) {
			c->error("�ϐ� " + string_ + " �͒�`����Ă��܂���B");
		}
		else {
			return tag->valueType;
		}
	}
	int left_type = left_->GetType(c);
	int right_type = right_->GetType(c);

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
	switch (op_) {
	case OP_EQ:case OP_NE:case OP_GT:case OP_GE:case OP_LT:case OP_LE:
		return TYPE_INTEGER;

	case OP_ADD:
		break;

	default:
		c->error("������ł͌v�Z�ł��Ȃ����ł��B");
		break;
	}
	return TYPE_STRING;
}
const ValueTag* Node::GetValueTag(Compiler* c) const
{
		c->error("�ϐ��m�[�h�ȊO����ϐ����󂯎�낤�Ƃ��Ă��܂�");
		return nullptr;
}
int  Node_function::GetType(Compiler* c)const {
	return GetCallType(c, left_->GetString(), &node_list_->args_);
}
//�m�[�h�̊֐��Ăяo���^�`�F�b�N
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

// �����
int Node::Assign(Compiler* c) const{
	int left_type = -1;
	//����݂̂̃p�^�[���ł͂Ȃ��̂ō��ӂ�push
	if (op_ != OP_ASSIGN) {
		left_type = left_->Push(c)&~TYPE_REF;
	}
	else {
		left_type = left_->GetType(c) & ~TYPE_REF;
	}
	int right_type = right_->Push(c) & ~TYPE_REF;



	// float�v�Z�m�[�h�̏���
	if ((left_type == TYPE_FLOAT && right_type == TYPE_FLOAT) || (left_type == TYPE_INTEGER && right_type == TYPE_FLOAT) || (left_type == TYPE_FLOAT && right_type == TYPE_INTEGER)) {
		if (!SetDefaultAssignOperator<float>(op_, c) && (!SetModAssignOperator<float>(op_, c)) && (!SetLogicalAssignOperator(op_, c))) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(c);
		return 0;
	}

	// �����v�Z�m�[�h�̏���
	if (left_type == TYPE_INTEGER && right_type == TYPE_INTEGER) {
		if (!SetDefaultAssignOperator<int>(op_, c) && (!SetModAssignOperator<int>(op_, c)) && (!SetLogicalAssignOperator(op_, c))) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(c);
		return 0;
	}


	//Vector2�v�Z�m�[�h
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_VOID + 1) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector2>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(c);
		return 0;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector2, float>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector2, float>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(c); 
		return 0;
	}
	if (left_type == TYPE_VOID + 1 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector2, int>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector2, int>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(c);
		return 0;
	}
	//Vector3�v�Z�m�[�h
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_VOID + 2) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector3>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(c);
		return 0;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector3, float>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector3, float>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(c);
		return 0;
	}
	if (left_type == TYPE_VOID + 2 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector3, int>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector3, int>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(c);
		return 0;
	}
	//Vector4�v�Z�m�[�h
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_VOID + 3) {
		if (!SetDefaultAssignOperator<ButiEngine::Vector4>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(c);
		return 0;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_FLOAT) {
		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector4, float>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector4, float>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
		}
		left_->Pop(c);
		return 0;
	}
	if (left_type == TYPE_VOID + 3 && right_type == TYPE_INTEGER) {

		if (!SetDeferentTypeMulAssignOperator<ButiEngine::Vector4, int>(op_, c) && !SetDeferentTypeDivAssignOperator<ButiEngine::Vector4, int>(op_, c)) {
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
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
			c->error("������ł͋�����Ȃ��v�Z�ł��B");
			break;
		}
		left_->Pop(c);

		return 0;
	}

	if (left_type == right_type) {
		//�����^���m�Ȃ̂ő���\
		left_->Pop(c);
		return 0;
	}else
	{
		c->error("����o���Ȃ��ϐ��̑g�ݍ��킹�ł�");
	}

	return -1;
}

const ValueTag* Node_value::GetValueTag(Compiler* c) const{

	if (op_ != OP_IDENTIFIER) {
		c->error("�����G���[�F�ϐ��m�[�h�ɕϐ��ȊO���o�^����Ă��܂��B");
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
			c->error("�ϐ� " + valueName + " �͒�`����Ă��܂���B");
		}
		return valueTag;
	}
}

// �ϐ��m�[�h��push
int Node_value::Push(Compiler* c) const{
	if (op_ != OP_IDENTIFIER) {
		c->error("�����G���[�F�ϐ��m�[�h�ɕϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		const ValueTag* tag = GetValueTag(c);
		{
			if (tag->global_) {		// �O���[�o���ϐ�
				if (left_) {		// �z��
					left_->Push(c);
					c->PushGlobalArrayRef(tag->address);
				}
				else {
					c->PushGlobalValueRef(tag->address);
				}
			}
			else {					// ���[�J���ϐ�
				if (left_) {		// �z��
					left_->Push(c);
					c->PushLocalArrayRef(tag->address);
				}
				else {
					c->PushLocalRef(tag->address);
				}
			}
			return tag->valueType & ~TYPE_REF;
		}
	}
	return TYPE_INTEGER;
}

int Node_value::PushClone(Compiler* c) const{
	if (op_ != OP_IDENTIFIER) {
		c->error("�����G���[�F�ϐ��m�[�h�ɕϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		const ValueTag* tag = GetValueTag(c);
		{
			if (tag->global_) {		// �O���[�o���ϐ�
				if (left_) {		// �z��
					left_->Push(c);
					c->PushGlobalArray(tag->address);
				}
				else {
					c->PushGlobalValue(tag->address);
				}
			}
			else {					// ���[�J���ϐ�
				if (left_) {		// �z��
					left_->Push(c);
					c->PushLocalArray(tag->address);
				}
				else {
					c->PushLocal(tag->address);
				}
			}
			return tag->valueType & ~TYPE_REF;
		}
	}
	return TYPE_INTEGER;
}

// �ϐ��m�[�h��pop
int Node_value::Pop(Compiler* c) const{
	if (op_ != OP_IDENTIFIER) {
		c->error("�����G���[�F�ϐ��m�[�h�ɕϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		const ValueTag* tag = GetValueTag(c);
		{
			if (tag->global_) {		// �O���[�o���ϐ�
				if (left_) {		// �z��
					left_->Push(c);
					c->PopArray(tag->address);
				}
				else {
					c->PopValue(tag->address);
				}
			}
			else {					// ���[�J���ϐ�
				if (left_) {		// �z��
					left_->Push(c);
					c->PopLocalArray(tag->address);
				}
				else {
					c->PopLocal(tag->address);
				}
			}
			return tag->valueType & ~TYPE_REF;
		}
	}
	return TYPE_INTEGER;
}

// �֐��Ăяo��
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
		if ((type & TYPE_REF) != 0) {		// �Q��
			if (node->Op() != OP_IDENTIFIER) {
				comp_->error("�Q�ƌ^�����ɁA�ϐ��ȊO�͎w��ł��܂���B");
			}
			else {
				std::string  valueName;
				NameSpace_t currentSerchNameSpace = comp_->GetCurrentNameSpace();
				const ValueTag* tag = node->GetValueTag(comp_);
				if (tag == nullptr) {
					comp_->error("�ϐ� " + node->GetString() + " �͒�`����Ă��܂���B");
				}
				else {
					if (!TypeCheck(tag->valueType ,type) ){
						comp_->error("�����̌^�������܂���B");
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
				comp_->error("�����̌^�������܂���B");
			}
		}
	}
};

// �֐��Ăяo��
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
		std::string message = "";
		if (argSize) {
			for (int i = 0; i < argSize; i++) {
				message += c->GetTypeName(argTypes[i]) + " ";
			}
			message += "�������ɂƂ�";
		}
		message += "�֐�" + functionName + "�͖��錾�ł�";
		c->error(message);
		return -1;
	}

	// ������push
	if (args && tag->ArgSize() == argSize) {
		std::for_each(args->begin(), args->end(), set_arg(c, tag));
	}

	// �����̐���push
	c->PushConstInt(argSize);

	if (tag->IsSystem()) {
		c->OpSysCall(tag->GetIndex());		// �g�ݍ��݊֐�
	}
	else {
		c->OpCall(tag->GetIndex());			// �X�N���v�g��̊֐�
	}

	return tag->valueType;
}

int Node_function::Push(Compiler* c) const
{
	return Call(c, left_->GetString(), &node_list_->args_);
}

// �֐���pop�͂ł��Ȃ��̂ŃG���[���b�Z�[�W���o��
int Node_function::Pop(Compiler* c) const
{
	c->error("�����G���[�F�֐��m�[�h��pop����");
	return TYPE_INTEGER;
}

// ���m�[�h����
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

	std::cerr << "�����G���[�F���m�[�h�~�X" << std::endl;
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

	std::cerr << "�����G���[�F���m�[�h�~�X" << std::endl;
	return Statement_t(new Statement_nop());
}

Statement_t Statement::make_statement(const int state, Block_t block)
{
	switch (state) {
	case BLOCK_STATE:
		return Statement_t(new Statement_block(block));
	}

	std::cerr << "�����G���[�F���m�[�h�~�X" << std::endl;
	return Statement_t(new Statement_nop());
}

// nop��
int Statement_nop::Analyze(Compiler* c) 
{
	return 0;
}

// �����
int Statement_assign::Analyze(Compiler* c) 
{
	return vec_node->Assign(c);
	
}

int Statement_assign::ReAnalyze(Compiler* c)
{
	return vec_node->Assign(c);
	
}

// �֐��Ăяo����
int ccall_statement::Analyze(Compiler* c) 
{
	int type = vec_node->Push(c);

	if (type == -1) {

		return -1;
	}

	if (type != TYPE_VOID)
		c->OpPop();			// �߂�l���̂Ă邽�߂�pop

	return 0;
}

void ccall_statement::ReAnalyze(Compiler* c) 
{
	int type = vec_node->Push(c);

	if (type == -1) {

		c->error("��`����Ă��Ȃ��֐����Q�Ƃ��Ă��܂�");
	}

	if (type != TYPE_VOID)
		c->OpPop();
}

// case��
int Statement_case::Analyze(Compiler* c) 
{
	c->SetLabel(label_);
	return 0;
}

// case���̑O����
int Statement_case::case_Analyze(Compiler* c, int* default_label)
{
	label_ = c->MakeLabel();
	if (vec_node->Op() != OP_INT)
		c->error("case ���ɂ͒萔�̂ݎw��ł��܂��B");
	vec_node->Push(c);
	c->OpTest(label_);
	return 0;
}

// default��
int Statement_default::Analyze(Compiler* c) 
{
	c->SetLabel(label_);
	return 0;
}

// default���̑O����
int Statement_default::case_Analyze(Compiler* c, int* default_label)
{
	label_ = c->MakeLabel();
	*default_label = label_;

	return 0;
}

// break��
int Statement_break::Analyze(Compiler* c) 
{
	if (!c->JmpBreakLabel()) {
		c->error("break��switch/for/while�O�ɗL��܂�");
	}
	return 0;
}


// return��
int Statement_return::Analyze(Compiler* c) 
{

	if (c->GetFunctionType() == TYPE_VOID) {	// �߂�l����
		if (vec_node != 0) {
			c->error("void�֐��ɖ߂�l���ݒ肳��Ă��܂�");
		}
		c->OpReturn();
	}
	else {
		if (vec_node == 0) {
			c->error("�֐��̖߂�l������܂���");
		}
		else {
			int node_type = vec_node->Push(c);		// �߂�l��push

			if (!CanTypeCast( node_type ,c->GetFunctionType())) {
				c->error("�߂�l�̌^�������܂���");
			}
		}
		c->OpReturnV();
	}

	return 0;
}

// if��
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

// for��
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

// while��
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

// switch��
int Statement_switch::Analyze(Compiler* c) 
{
	if (!statement_.empty()) {
		vec_node->Push(c);

		int label = c->MakeLabel();		// L0���x���쐬
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

// block��
int Statement_block::Analyze(Compiler* c) 
{
	c->BlockIn();
	block_->Analyze(c);
	c->BlockOut();
	return 0;
}

// ���u���b�N
int Block::Analyze(Compiler* c) 
{
	auto ret = 0;
	{

		auto endItr = decl_.end();

		for (auto itr = decl_.begin(); itr != endItr; itr++) {
			(*itr)->Define(c);
		}
	}
	if (!decl_.empty())
		c->AllocStack();	// �X�^�b�N�t���[���m��

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

// �錾�̉��
int Declaration::Analyze(Compiler* c) 
{
	if (isFunction) {		// �֐�
		c->FunctionDefine(valueType, name_, args);
	}
	else {
		c->ValueDefine(valueType, vec_node);
		c->OpAllocStack(valueType);
	}
	return 0;
}

// �֐��̉��
int Function::Analyze(Compiler* c) 
{

	c->AddFunction(valueType, name_, args_, block_);

	return 0;
}

int Function::Regist(Compiler* c)
{
	c->RegistFunction(valueType, name_, args_, block_);
	return 0;
}


void Declaration::Define(Compiler* c)
{
	if (isFunction) {		// �֐�
		c->FunctionDefine(valueType, name_, args);
	}
	else {
		c->ValueDefine(valueType, vec_node);
	}
}
int Node_Member::Push(Compiler* c) const
{
	if (op_ != OP_MEMBER) {
		c->error("�����G���[�F�����o�ϐ��m�[�h�Ƀ����o�ϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		//�ϐ��̃����o�ϐ�
		if (left_->Op() == OP_IDENTIFIER|| left_->Op() == OP_MEMBER) {
			const ValueTag* valueTag = left_->GetValueTag(c);
			{

				//�^
				auto typeTag = c->GetType(left_->GetType(c)&~TYPE_REF);


				if (valueTag->global_) {		// �O���ϐ�
					if (left_->GetLeft()) {		// �z��
						c->PushConstInt(valueTag->address);
						left_->GetLeft()->Push(c);
						c->PushGlobalArrayMemberRef(typeTag->map_memberIndex.at(string_));
					}
					else {
						c->PushConstInt(valueTag->address);
						c->PushGlobalMemberRef(typeTag->map_memberIndex.at(string_));
					}
				}
				else {					// ���[�J���ϐ�
					if (left_->GetLeft()) {		// �z��
						c->PushConstInt(valueTag->address);
						left_->GetLeft()->Push(c);
						c->PushLocalArrayMemberRef(typeTag->map_memberIndex.at(string_));
					}
					else {
						c->PushConstInt(valueTag->address);
						c->PushLocalMemberRef(typeTag->map_memberIndex.at(string_));
					}
				}
				return   typeTag->map_memberType.at(string_) & ~TYPE_REF;
			}
		}

		
	}
}
int Node_Member::PushClone(Compiler* c) const
{
	if (op_ != OP_MEMBER) {
		c->error("�����G���[�F�����o�ϐ��m�[�h�Ƀ����o�ϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		//�ϐ��̃����o�ϐ�
		if (left_->Op() == OP_IDENTIFIER || left_->Op() == OP_MEMBER) {
			const ValueTag* valueTag = left_->GetValueTag(c);
			{

				//�^
				auto typeTag = c->GetType(left_->GetType(c));


				if (valueTag->global_) {		// �O���ϐ�
					if (left_->GetLeft()) {		// �z��
						left_->GetLeft()->PushClone(c);
						c->PushGlobalArray(valueTag->address);
					}
					else {
						c->PushGlobalValue(valueTag->address);
					}
				}
				else {					// ���[�J���ϐ�
					if (left_->GetLeft()) {		// �z��
						left_->GetLeft()->PushClone(c);
						c->PushLocalArray(valueTag->address);
					}
					else {
						c->PushConstInt(valueTag->address);
						c->PushLocalMember(typeTag->map_memberIndex.at(string_));
					}
				}
				return   typeTag->map_memberType.at(string_) & ~TYPE_REF;
			}
		}


	}
}
int Node_Member::Pop(Compiler* c) const
{
	if (op_ != OP_MEMBER) {
		c->error("�����G���[�F�����o�ϐ��m�[�h�Ƀ����o�ϐ��ȊO���o�^����Ă��܂��B");
	}
	else {

		const ValueTag* valueTag = left_->GetValueTag(c);
		{

			//�^
			auto typeTag = c->GetType(left_->GetType(c));
			
			if (valueTag->global_) {		// �O���ϐ�
				if (left_->GetLeft()) {		// �z��
					c->PushConstInt(valueTag->address);
					left_->GetLeft()->Push(c);
					c->PopGlobalArrayMember(valueTag->address);
				}
				else {
					c->PushConstInt(valueTag->address);
					c->PopGlobalMember(valueTag->address);
				}
			}
			else {					// ���[�J���ϐ�
				if (left_->GetLeft()) {		// �z��
					c->PushConstInt(valueTag->address);
					left_->GetLeft()->Push(c);
					c->PopLocalArrayMember(typeTag->map_memberIndex.at(string_));
				}
				else {
					c->PushConstInt(valueTag->address);
					c->PopLocalMember(typeTag->map_memberIndex.at(string_));
				}
			}
			return   typeTag->map_memberType.at(string_) & ~TYPE_REF;;
		}
	}
	return TYPE_INTEGER;
}
int Node_Member::GetType(Compiler* c) const
{
	if (op_ != OP_MEMBER) {
		c->error("�����G���[�F�����o�ϐ��m�[�h�Ƀ����o�ϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		//�ϐ��̃����o�ϐ�
		if (left_->Op() == OP_IDENTIFIER || left_->Op() == OP_MEMBER) {
			std::string  valueName;
			NameSpace_t currentSerchNameSpace = c->GetCurrentNameSpace();
			const ValueTag* valueTag=  left_->GetValueTag(c);
			{

				//�^
				auto typeTag = c->GetType(left_->GetType(c));

				return typeTag->map_memberType.at(string_);
			}
		}
	}
}
int Node_Method::Push(Compiler* c) const
{
	if (op_ != OP_METHOD) {
		c->error("�����G���[�F�����o�֐��m�[�h�Ƀ����o�֐��ȊO���o�^����Ă��܂��B");
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


	if (methodTag == nullptr) {
		std::string message = "";
		if (argSize) {
			for (int i = 0; i < argSize; i++) {
				message += c->GetTypeName(argTypes[i]) + " ";
			}
			message += "�������ɂƂ�";
		}
		message += "�֐�" + string_ + "�͖��錾�ł�";
		c->error(message);
		return -1;
	}

	// ������push
	if (node_list_&& methodTag->ArgSize() == argSize) {
		std::for_each(node_list_->args_.begin(), node_list_->args_.end(), set_arg(c, methodTag));
	}

	left_->Push(c);

	// �����̐���push
	c->PushConstInt(argSize);


	if (methodTag->IsSystem()) {
		c->OpSysMethodCall(methodTag->GetIndex());		// �g�ݍ��݃��\�b�h
	}
	else {
		c->OpCall(methodTag->GetIndex());			// �X�N���v�g��̃��\�b�h
	}


	return methodTag->valueType;
}
int Node_Method::Pop(Compiler* c) const
{
	c->error("�����G���[�F�����o�֐��m�[�h��pop");
	return TYPE_INTEGER;
}
int Node_Method::GetType(Compiler* c) const
{
	if (op_ != OP_METHOD) {
		c->error("�����G���[�F�����o�֐��m�[�h�Ƀ����o�֐��ȊO���o�^����Ă��܂��B");
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

		c->error("�񋓌^�@" + left_->GetString() + "�͖���`�ł�");
		return -1;
	}
	if (!enumType->ExistenceIdentifers(string_)) {

		c->error("�񋓌^�@" + left_->GetString()+"."+string_ + "�͖���`�ł�");
		return -1;
	}
	c->PushConstInt( enumType->GetValue(string_));

	return TYPE_INTEGER;
}
int Node_enum::Pop(Compiler* c) const
{
	c->error("�����G���[�F�񋓌^�m�[�h��pop");
	return -1;
}
int Node_enum::GetType(Compiler* c) const
{
	return TYPE_INTEGER;
}
void Enum::SetIdentifer(const std::string& arg_name)
{
	map_identifer.emplace(arg_name, map_identifer.size());
}
void Enum::SetIdentifer(const std::string& arg_name, const int value)
{
	map_identifer.emplace(arg_name, value);
}
void Enum::Analyze(Compiler* c)
{
	c->RegistEnumType(typeName);
	auto tag = c->GetEnumTag(c->GetCurrentNameSpace()->GetGlobalNameString()+ typeName);
	auto end = map_identifer.end();
	for (auto itr = map_identifer.begin(); itr != end; itr++) {
		tag->SetValue(itr->first, itr->second);
	}
}
}