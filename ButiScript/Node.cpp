#include "stdafx.h"
#include "Node.h"
#include "compiler.h"

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
	return Node_t(new Node_function(Op, left, right));
}

// �m�[�h��push����
int Node::Push(Compiler* c) const
{
	switch (op_) {
	case OP_NEG:
		if (left_->Push(c) == TYPE_STRING)
			c->error("������ɂ͋�����Ȃ��v�Z�ł��B");
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

	//�E�ӎႵ���͍��ӂ�����`�֐�
	if (left_type <= -1 || right_type <= -1) {
		return -1;
	}

	// float�v�Z�m�[�h�̏���
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
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
			break;
		}
		return TYPE_FLOAT;
	}

	// �����v�Z�m�[�h�̏���
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
			c->error("�����G���[�F�����ł��Ȃ��v�Z�m�[�h������܂����B");
			break;
		}
		return TYPE_INTEGER;
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

int Node::Pop(Compiler* c) const
{
	c->error("�����G���[�F�v�Z�m�[�h��pop���Ă��܂��B");
	return TYPE_INTEGER;
}

// �����
int Node::Assign(Compiler* c) const
{
	if (op_ != OP_ASSIGN)
		left_->Push(c);
	auto rightType = right_->Push(c);

	//right���܂���`����ĂȂ��֐��Ȃ̂ŃX�L�b�v
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
			c->error("������^�ɐ����������Ă��܂��B");
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
			c->error("������^�ɕ��������������Ă��܂��B");
		return 0;
	}

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
	return 0;
	if (left_->Pop(c) != TYPE_STRING)
		c->error("�����^�ɕ�����������Ă��܂��B");

	return -1;
}

// �ϐ��m�[�h��push
int Node_value::Push(Compiler* c) const
{
	if (op_ != OP_IDENTIFIER) {
		c->error("�����G���[�F�ϐ��m�[�h�ɕϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		const ValueTag* tag = c->GetValueTag(string_);
		if (tag == 0) {
			c->error("�ϐ� " + string_ + " �͒�`����Ă��܂���B");
		}
		else {
			// �Q�ƌ^�ϐ��́A�����ɂ������݂��Ȃ�
			if ((tag->type_ & TYPE_REF) != 0) {
				if (left_) {		// �z��
					left_->Push(c);
					c->PushLocalArrayRef(tag->addr_);
				}
				else {
					c->PushLocalRef(tag->addr_);
				}
				return tag->type_ & ~TYPE_REF;
			}
			if (tag->global_) {		// �O���ϐ�
				if (left_) {		// �z��
					left_->Push(c);
					c->PushArray(tag->addr_);
				}
				else {
					c->PushValue(tag->addr_);
				}
			}
			else {					// ���[�J���ϐ�
				if (left_) {		// �z��
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

// �ϐ��m�[�h��pop
int Node_value::Pop(Compiler* c) const
{
	if (op_ != OP_IDENTIFIER) {
		c->error("�����G���[�F�ϐ��m�[�h�ɕϐ��ȊO���o�^����Ă��܂��B");
	}
	else {
		const ValueTag* tag = c->GetValueTag(string_);
		if (tag == 0) {
			c->error("�ϐ� " + string_ + " �͒�`����Ă��܂���B");
		}
		else {
			// �Q�ƌ^�ϐ��́A�����ɂ������݂��Ȃ�
			if ((tag->type_ & TYPE_REF) != 0) {
				if (left_) {		// �z��
					left_->Push(c);
					c->PopLocalArrayRef(tag->addr_);
				}
				else {
					c->PopLocalRef(tag->addr_);
				}
				return tag->type_ & ~TYPE_REF;
			}
			if (tag->global_) {		// �O���ϐ�
				if (left_) {		// �z��
					left_->Push(c);
					c->PopArray(tag->addr_);
				}
				else {
					c->PopValue(tag->addr_);
				}
			}
			else {					// ���[�J���ϐ�
				if (left_) {		// �z��
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
				const ValueTag* tag = comp_->GetValueTag(node->GetString());
				if (tag == 0) {
					comp_->error("�ϐ� " + node->GetString() + " �͒�`����Ă��܂���B");
				}
				else if (tag->type_ >= TYPE_INTEGER_REF) {		// �Q��
					// �Q�ƌ^�ϐ��́A���[�J�������Ȃ�
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
						comp_->error("�����̌^�������܂���B");
					}
					int addr = tag->addr_;
					if (tag->global_)			// �O���ϐ�
						addr |= ButiVM::VirtualCPU::global_flag;
					// �A�h���X��push
					if (node->GetLeft()) {			// �z��
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
				comp_->error("�����̌^�������܂���B");
			}
		}
	}
};

// �֐��Ăяo��
int Node::Call(Compiler* c, const std::string& name, const std::vector<Node_t>* args) const
{
	const FunctionTag* tag = c->GetFunctionTag(name);
	if (tag == NULL) {
		return -1;
	}

	int arg_size = (args) ? (int)args->size() : 0;
	if (tag->ArgSize() != arg_size) {
		c->error("�����̐��������܂���B");
	}

	// ������push
	if (args && tag->ArgSize() == arg_size) {
		std::for_each(args->begin(), args->end(), set_arg(c, tag));
	}

	// �����̐���push
	c->PushConst(arg_size);

	if (tag->IsSystem()) {
		c->OpSysCall(tag->GetIndex());		// �g�ݍ��݊֐�
	}
	else {
		c->OpCall(tag->GetIndex());			// �X�N���v�g��̊֐�
	}

	return tag->type_;
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

	case RETURN_STATE:
		return Statement_t(new Statement_return());

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
	return node_->Assign(c);
	
}

int Statement_assign::ReAnalyze(Compiler* c)
{
	return node_->Assign(c);
	
}

// �֐��Ăяo����
int ccall_statement::Analyze(Compiler* c) 
{
	int type = node_->Push(c);

	if (type == -1) {

		return -1;
	}

	if (type != TYPE_VOID)
		c->OpPop();			// �߂�l���̂Ă邽�߂�pop

	return 0;
}

void ccall_statement::ReAnalyze(Compiler* c) 
{
	int type = node_->Push(c);

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
	if (node_->Op() != OP_INT)
		c->error("case ���ɂ͒萔�̂ݎw��ł��܂��B");
	node_->Push(c);
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
		if (node_ != 0) {
			c->error("void�֐��ɖ߂�l���ݒ肳��Ă��܂�");
		}
		c->OpReturn();
	}
	else {
		if (node_ == 0) {
			c->error("�֐��̖߂�l������܂���");
		}
		else {
			int node_type = node_->Push(c);		// �߂�l��push

			if (node_type == -1) {//�錾����Ă��Ȃ��֐��Q�Ƃ��Ă���X�L�b�v
				return -1;
			}

			if (node_type != c->GetFunctionType()) {
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

// for��
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

// while��
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

// switch��
int Statement_switch::Analyze(Compiler* c) 
{
	if (!statement_.empty()) {
		node_->Push(c);

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
			if ((ret=(*itr)->Analyze(c))!=0) {
				return ret;
			}
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
	if (is_func_) {		// �֐�
		c->FunctionDefine(type_, name_, arg_);
	}
	else {
		c->ValueDefine(type_, node_);
	}
	return 0;
}

// �֐��̉��
int Function::Analyze(Compiler* c) 
{
	c->AddFunction(type_, name_, args_, block_);

	return 0;
}
