#include "stdafx.h"
#include "Compiler.h"

#include"VirtualMachine.h"
#include <iomanip>
#include "Parser.h"

// �R���X�g���N�^
Compiler::Compiler()
	: break_index(-1), error_count(0)
{
}

Compiler::~Compiler()
{
}

// �R���p�C��
bool Compiler::Compile(const std::string& file, ButiVM::Data& Data)
{
	// �g�ݍ��݊֐��̐ݒ�
	DefineSystemFunction(ButiVM::SYS_PRINT, TYPE_VOID, "print", "s");
	DefineSystemFunction(ButiVM::SYS_PAUSE, TYPE_VOID, "pause", nullptr);
	DefineSystemFunction(ButiVM::SYS_TOSTR, TYPE_STRING, "ToString", "i");
	DefineSystemFunction(ButiVM::SYS_TOSTRF, TYPE_STRING, "ToStringF", "f");

	// �O���[�o���ϐ��p�A�ϐ��e�[�u�����Z�b�g
	variables.push_back(ValueTable());
	variables[0].set_global();

	// �擪��HALT���߂ɂ��Ă���
	OpHalt();

	bool result = ScriptParser(file, this);	// �\�����

	if (!result)
		return false;// �p�[�T�[�G���[

	int code_size = LabelSetting();				// ���x���ɃA�h���X��ݒ�
	CraeteData(Data, code_size);				// �o�C�i������
	return error_count == 0;
}

// �G���[���b�Z�[�W���o��
void Compiler::error(const std::string& m)
{
	std::cerr << m << std::endl;
	error_count++;
}

// �����֐��̒�`
bool Compiler::DefineSystemFunction(int index, int type, const char* name, const char* args)
{
	FunctionTag func(type);
	if (!func.SetArgs(args))		// ������ݒ�
		return false;

	func.SetDeclaration();			// �錾�ς�
	func.SetSystem();				// System�t���O�Z�b�g
	func.SetIndex(index);			// �g�ݍ��݊֐��ԍ���ݒ�
	if (functions.Add(name, func) == 0) {
		return false;
	}
	return true;
}

// �O���ϐ��̒�`
struct Define_value {
	Compiler* comp_;
	int type_;
	Define_value(Compiler* comp, int type) : comp_(comp), type_(type)
	{
	}

	void operator()(Node_t node) const
	{
		comp_->AddValue(type_, node->GetString(), node->GetLeft());
	}
};

void Compiler::ValueDefine(int type, const std::vector<Node_t>& node)
{
	std::for_each(node.begin(), node.end(), Define_value(this, type));
}

// �֐��錾
void Compiler::FunctionDefine(int type, const std::string& name, const std::vector<int>& args)
{
	const FunctionTag* tag = functions.find(name);
	if (tag) {			// ���ɐ錾�ς�
		if (!tag->ChkArgList(args)) {
			error("�֐� " + name + " �ɈقȂ�^�̈������w�肳��Ă��܂�");
			return;
		}
	}
	else {
		FunctionTag func(type);
		func.SetArgs(args);				// ������ݒ�
		func.SetDeclaration();			// �錾�ς�
		func.SetIndex(MakeLabel());		// ���x���o�^
		if (functions.Add(name, func) == 0) {
			error("�����G���[�F�֐��e�[�u���ɓo�^�ł��܂���");
		}
	}
}

// �֐���`
//
//	�֐����Ă΂ꂽ���_�̃X�^�b�N
//
//	+--------------+
//	|     arg2     | -5
//	+--------------+
//	|     arg1     | -4
//	+--------------+
//	|  arg count   | -3
//	+--------------+
//	| base_pointer | -2
//	+--------------+
//	| return addr  | -1
//	+--------------+
//
//	���������āA�����̊J�n�A�h���X��-4�ƂȂ�A�f�N�������g���Ă����B

// �����̕ϐ�����o�^
struct add_value {
	Compiler* comp_;
	ValueTable& values_;
	mutable int addr_;
	add_value(Compiler* comp, ValueTable& values) : comp_(comp), values_(values), addr_(-4)
	{
	}

	void operator()(const ArgDefine& arg) const
	{
		if (!values_.add_arg(arg.type(), arg.name(), addr_)) {
			comp_->error("���� " + arg.name() + " �͊��ɓo�^����Ă��܂��B");
		}
		addr_--;
	}
};

void Compiler::AddFunction(int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block, bool isReRegist)
{
	FunctionTag* tag = functions.find(name);
	if (tag) {
		if (tag->IsDefinition()&&!isReRegist) {
			error("�֐� " + name + " �͊��ɒ�`����Ă��܂�");
			return;
		}
		if (tag->IsDeclaration() && !tag->ChkArgList(args)) {
			error("�֐� " + name + " �ɈقȂ�^�̈������w�肳��Ă��܂�");
			return;
		}
		tag->SetDefinition();	// ��`�ς݂ɐݒ�
	}
	else {
		FunctionTag func(type);
		func.SetArgs(args);				// ������ݒ�
		func.SetDefinition();			// ��`�ς�
		func.SetIndex(MakeLabel());		// ���x���o�^
		tag = functions.Add(name, func);
		if (tag == nullptr)
			error("�����G���[�F�֐��e�[�u���ɓo�^�ł��܂���");
	}

	current_function_name = name;		// �������̊֐�����o�^���Ă���
	current_function_type = type;		// �������̊֐��^��o�^���Ă���
										// �֐����֐��i����q�\���j�͖����̂ŁA
										// �O���[�o���ϐ��P�ł悢

	// �֐��̃G���g���[�|�C���g�Ƀ��x����u��

	SetLabel(tag->GetIndex());

	BlockIn();		// �ϐ��X�^�b�N�𑝂₷

	// �������X�g��o�^
	auto endItr = args.rend();
	for (auto itr = args.rbegin(); itr != endItr; itr++) {
		add_value(this, variables.back())(*itr);
	}

	// ��������΁A����o�^
	if (block) {
		int ret=block->Analyze(this);

		if (ret != 0) {
			if (isReRegist) {
				error("��`����Ă��Ȃ��֐����Q�Ƃ��Ă��܂�");
				return;
			}


			BlockOut();		// �ϐ��X�^�b�N�����炷
			statement.pop_back();
			current_function_name.clear();		// �������̊֐���������
			AddCallingNonDeclaredFunction(type, name, args, block);
			return;
		}
	}

	const VMCode& code = statement.back();
	if (type == TYPE_VOID) {			// �߂�l����
		if (code.op_ != VM_RETURN)		// return�������Ȃ��
			OpReturn();					// return��ǉ�
	}
	else {
		if (code.op_ != VM_RETURNV) {	// return�������Ȃ��
			error("�֐� " + name + " �̍Ō��return�����L��܂���B");
		}
	}

	BlockOut();		// �ϐ��X�^�b�N�����炷

	current_function_name.clear();		// �������̊֐���������
}

// �ϐ��̓o�^
void Compiler::AddValue(int type, const std::string& name, Node_t node)
{
	int size = 1;
	if (node) {
		if (node->Op() != OP_INT) {
			error("�z��̃T�C�Y�͒萔�Ŏw�肵�Ă��������B");
		}
		else if (node->GetNumber() <= 0) {
			error("�z��̃T�C�Y�͂P�ȏ�̒萔���K�v�ł��B");
		}
		size = node->GetNumber();
	}

	ValueTable& values = variables.back();
	if (!values.Add(type, name, size)) {
		error("�ϐ� " + name + " �͊��ɓo�^����Ă��܂��B");
	}
}

// ���x������
int Compiler::MakeLabel()
{
	int index = (int)labels.size();
	labels.push_back(Label(index));
	return index;
}

// ���x���̃_�~�[�R�}���h���X�e�[�g�����g���X�g�ɓo�^����
void Compiler::SetLabel(int label)
{
	statement.push_back(VMCode(VM_MAXCOMMAND, label));
}

// ������萔��push
void Compiler::PushString(const std::string& str)
{
	PushString(((int)text_table.size()));
	text_table.insert(text_table.end(), str.begin(), str.end());
	text_table.push_back('\0');
}

// break���ɑΉ�����Jmp�R�}���h����

bool Compiler::JmpBreakLabel()
{
	if (break_index < 0)
		return false;
	OpJmp(break_index);
	return true;
}

// �u���b�N���ł́A�V�����ϐ��Z�b�g�ɕϐ���o�^����

void Compiler::BlockIn()
{
	int start_addr = 0;					// �ϐ��A�h���X�̊J�n�ʒu
	if (variables.size() > 1) {			// �u���b�N�̓���q�́A�J�n�A�h���X�𑱂�����ɂ���B
		start_addr = variables.back().size();
	}
	variables.push_back(ValueTable(start_addr));
}

// �u���b�N�̏I���ŁA�ϐ��X�R�[�v��������i�ϐ��Z�b�g���폜����j

void Compiler::BlockOut()
{
	variables.pop_back();
}

// ���[�J���ϐ��p�ɃX�^�b�N���m��

void Compiler::AllocStack()
{
	OpAllocStack(variables.back().size());
}

// ���x������
//
// �P�D�A�h���X�𐶐�����
// �Q�D�_�~�[�̃��x���R�}���h���L�����A�h���X���A���x���e�[�u���ɓo�^����
// �R�DJmp�R�}���h�̔�ѐ�����x���e�[�u���ɓo�^���ꂽ�A�h���X�ɂ���

// �A�h���X�v�Z
struct calc_addr {
	std::vector<Label>& labels_;
	int& pos_;
	calc_addr(std::vector<Label>& labels, int& pos) : labels_(labels), pos_(pos)
	{
	}
	void operator()(const VMCode& code)
	{
		if (code.op_ == VM_MAXCOMMAND) {			// ���x���̃_�~�[�R�}���h
			labels_[code.codeValue].pos_ = pos_;
		}
		else {
			pos_ += code.size_;
		}
	}
};

// �W�����v�A�h���X�ݒ�
struct set_addr {
	std::vector<Label>& labels_;
	set_addr(std::vector<Label>& labels) : labels_(labels)
	{
	}
	void operator()(VMCode& code)
	{
		switch (code.op_) {
		case VM_JMP:
		case VM_JMPC:
		case VM_JMPNC:
		case VM_TEST:
		case VM_CALL:
			code.codeValue = labels_[code.codeValue].pos_;
			break;
		}
	}
};

int Compiler::LabelSetting()
{
	// �A�h���X�v�Z
	int pos = 0;
	std::for_each(statement.begin(), statement.end(), calc_addr(labels, pos));
	// �W�����v�A�h���X�ݒ�
	std::for_each(statement.begin(), statement.end(), set_addr(labels));

	return pos;
}

// �o�C�i���f�[�^����

struct copy_code {
	unsigned char* p;
	copy_code(unsigned char* code) : p(code)
	{
	}
	void operator()(const VMCode& code)
	{
		p = code.Get(p);
	}
};

bool Compiler::CraeteData(ButiVM::Data& Data, int code_size)
{
	const FunctionTag* tag = GetFunctionTag("main");	// �J�n�ʒu
	if (tag == 0) {
		error("�֐� \"main\" ��������܂���B");
		return false;
	}

	Data.command_ = new unsigned char[code_size];
	Data.text_buffer_ = new char[text_table.size()];
	Data.command_size_ = code_size;
	Data.text_size_ = (int)text_table.size();
	Data.value_size_ = (int)variables[0].size();
	Data.entry_point_ = labels[tag->index_].pos_;

	if (Data.text_size_ != 0)
		memcpy(Data.text_buffer_, &text_table[0], Data.text_size_);

	std::for_each(statement.begin(), statement.end(), copy_code(Data.command_));

	return true;
}

void Compiler::AddCallingNonDeclaredFunction(int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block)
{
	AddFunctionInfo info;
	info.type = type;
	info.name = name,
	info.args = args;
	info.block = block;
	vec_callingNonDeclaredFunctions.push_back(info);
}

void Compiler::ReRegistFunctions()
{
	auto end = vec_callingNonDeclaredFunctions.end();
	for (auto itr = vec_callingNonDeclaredFunctions.begin(); itr != end; itr++) {
		AddFunction(itr->type, itr->name, itr->args, itr->block,true);
	}
}

// �f�o�b�O�_���v
#ifdef	_DEBUG
void Compiler::debug_dump()
{
	std::cout << "---variables---" << std::endl;
	size_t vsize = variables.size();
	std::cout << "value stack = " << vsize << std::endl;
	for (size_t i = 0; i < vsize; i++) {
		variables[i].dump();
	}
	std::cout << "---code---" << std::endl;

	static const char* op_name[] = {
#define	VM_NAMETABLE
#include "VM_nametable.h"
#undef	VM_NAMETABLE
		"LABEL",
	};

	int	pos = 0;
	size_t size = statement.size();
	for (size_t i = 0; i < size; i++) {
		std::cout << std::setw(6) << pos << ": " << op_name[statement[i].op_];
		if (statement[i].size_ > 1) {
			std::cout << ", " << statement[i].codeValue;
		}
		std::cout << std::endl;

		if (statement[i].op_ != VM_MAXCOMMAND) {
			pos += statement[i].size_;
		}
	}
}
#endif
