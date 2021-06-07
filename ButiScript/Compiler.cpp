#include "stdafx.h"
#include "Compiler.h"

#include"VirtualMachine.h"
#include <iomanip>
#include "Parser.h"

// �R���X�g���N�^
ButiScript:: Compiler::Compiler()
	: break_index(-1), error_count(0)
{
}

ButiScript::Compiler::~Compiler()
{
}

// �R���p�C��
bool ButiScript::Compiler::Compile(const std::string& file, ButiScript::Data& Data)
{
	//�O���[�o�����O��Ԃ̐ݒ�
	currentNameSpace = std::make_shared<NameSpace>("");
	//�g�ݍ��݊֐��̐ݒ�

	DefineSystemFunction(&ButiScript::VirtualCPU::sys_print, TYPE_VOID, "print", "s");
	DefineSystemFunction(&ButiScript::VirtualCPU::Sys_pause, TYPE_VOID, "pause", nullptr);
	DefineSystemFunction(&ButiScript::VirtualCPU::sys_tostr, TYPE_STRING, "ToString", "i");
	DefineSystemFunction(&ButiScript::VirtualCPU::sys_tostrf, TYPE_STRING, "ToString", "f");

	//�ϐ��e�[�u�����Z�b�g
	variables.push_back(ValueTable());
	variables[0].set_global();
	OpHalt();

	bool result = ScriptParser(file, this);	// �\�����

	if (!result)
		return false;// �p�[�T�[�G���[

	int code_size = LabelSetting();				// ���x���ɃA�h���X��ݒ�
	CraeteData(Data, code_size);				// �o�C�i������
	return error_count == 0;
}

// �G���[���b�Z�[�W���o��
void ButiScript::Compiler::error(const std::string& m)
{
	std::cerr << m << std::endl;
	error_count++;
}

void ButiScript::Compiler::ClearStatement()
{
	statement.clear();
}

std::string ButiScript::Compiler::GetTypeName(const int arg_type) const
{
	int type = arg_type & ~TYPE_REF;
	std::string output = "";
	switch (type)
	{
	case TYPE_INTEGER:
		output = "int";
		break;
	case TYPE_FLOAT:
		output = "float";
		break;
	case TYPE_STRING:
		output = "string";
		break;
	}

	if (arg_type&TYPE_REF) {
		output += "&";
	}

	return output;
}


// �����֐��̒�`
bool ButiScript::Compiler::DefineSystemFunction(SysFunction arg_op,const int type, const char* name, const char* args)
{
	FunctionTag func(type);
	if (!func.SetArgs(args))		// ������ݒ�
		return false;

	func.SetDeclaration();			// �錾�ς�
	func.SetSystem();				// System�t���O�Z�b�g
	func.SetIndex(vec_sysCalls.size());			// �g�ݍ��݊֐��ԍ���ݒ�
	vec_sysCalls.push_back(arg_op);
	if (functions.Add(name, func) == 0) {
		return false;
	}
	return true;
}

// �O���ϐ��̒�`
struct Define_value {
	ButiScript::Compiler* comp_;
	int type_;
	Define_value(ButiScript::Compiler* comp, int type) : comp_(comp), type_(type)
	{
	}

	void operator()(ButiScript::Node_t node) const
	{
		comp_->AddValue(type_, node->GetString(), node->GetLeft());
	}
};

void ButiScript::Compiler::ValueDefine(int type, const std::vector<Node_t>& node)
{
	std::for_each(node.begin(), node.end(), Define_value(this, type));
}

// �֐��錾
void ButiScript::Compiler::FunctionDefine(int type, const std::string& name, const std::vector<int>& args)
{
	const FunctionTag* tag = functions.Find_strict(name,args);
	if (tag) {			// ���ɐ錾�ς�
		if (!tag->CheckArgList_strict(args)) {
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
//	

// �����̕ϐ�����o�^
struct add_value {
	ButiScript::Compiler* comp_;
	ButiScript::ValueTable& values_;
	mutable int addr_;
	add_value(ButiScript::Compiler* comp, ButiScript::ValueTable& values) : comp_(comp), values_(values), addr_(-4)
	{
	}

	void operator()(const ButiScript::ArgDefine& arg) const
	{
		if (!values_.add_arg(arg.type(), arg.name(), addr_)) {
			comp_->error("���� " + arg.name() + " �͊��ɓo�^����Ă��܂��B");
		}
		addr_--;
	}
};

void ButiScript::Compiler::AddFunction(int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block, bool isReRegist)
{
	std::string functionName = currentNameSpace->GetGlobalNameString()+ name;
	FunctionTag* tag = functions.Find_strict(functionName,args);
	if (tag) {
		if (tag->IsDefinition()&&!isReRegist) {
			error("�֐� " + functionName + " �͊��ɒ�`����Ă��܂�");
			return;
		}
		if (tag->IsDeclaration() && !tag->CheckArgList_strict(args)) {
			error("�֐� " + functionName + " �ɈقȂ�^�̈������w�肳��Ă��܂�");
			return;
		}
		tag->SetDefinition();	// ��`�ς݂ɐݒ�
	}
	else {
		FunctionTag func(type);
		func.SetArgs(args);				// ������ݒ�
		func.SetDefinition();			// ��`�ς�
		func.SetIndex(MakeLabel());		// ���x���o�^
		tag = functions.Add(functionName, func);
		if (tag == nullptr)
			error("�����G���[�F�֐��e�[�u���ɓo�^�ł��܂���");
	}

	current_function_name = functionName;		// �������̊֐�����o�^
	current_function_type = type;		// �������̊֐��^��o�^

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
	}

	const VMCode& code = statement.back();
	if (type == TYPE_VOID) {			// �߂�l����
		if (code.op_ != VM_RETURN)		// return�������Ȃ��
			OpReturn();					// return��ǉ�
	}
	else {
		if (code.op_ != VM_RETURNV) {	// return�������Ȃ��
			error("�֐� " + functionName + " �̍Ō��return�����L��܂���B");
		}
	}

	BlockOut();		// �ϐ��X�^�b�N�����炷

	current_function_name.clear();		// �������̊֐���������
}

void ButiScript::Compiler::RegistFunction(const int type, const std::string& name, const std::vector<ArgDefine>& args, Block_t block, const bool isReRegist)
{
	std::string functionName = currentNameSpace->GetGlobalNameString() + name;
	const FunctionTag* tag = functions.Find_strict(functionName,args);
	if (tag) {			// ���ɐ錾�ς�
		if (!tag->CheckArgList_strict(args)) {
			error("�֐� " + functionName + " �ɈقȂ�^�̈������w�肳��Ă��܂�");
			return;
		}
	}
	else {
		FunctionTag func(type);
		func.SetArgs(args);				// ������ݒ�
		func.SetDeclaration();			// �錾�ς�
		func.SetIndex(MakeLabel());		// ���x���o�^
		if (functions.Add(functionName, func) == 0) {
			error("�����G���[�F�֐��e�[�u���ɓo�^�ł��܂���");
		}
	}
}

// �ϐ��̓o�^
void ButiScript::Compiler::AddValue(int type, const std::string& name, Node_t node)
{
	std::string valueName = GetCurrentNameSpace()->GetGlobalNameString() + name;
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
	if (!values.Add(type, valueName, size)) {
		error("�ϐ� " + valueName + " �͊��ɓo�^����Ă��܂��B");
	}
}

// ���x������
int ButiScript::Compiler::MakeLabel()
{
	int index = (int)labels.size();
	labels.push_back(Label(index));
	return index;
}

// ���x���̃_�~�[�R�}���h���X�e�[�g�����g���X�g�ɓo�^����
void ButiScript::Compiler::SetLabel(int label)
{
	statement.push_back(VMCode(VM_MAXCOMMAND, label));
}

// ������萔��push
void ButiScript::Compiler::PushString(const std::string& str)
{
	PushString(((int)text_table.size()));
	text_table.insert(text_table.end(), str.begin(), str.end());
	text_table.push_back('\0');
}

// break���ɑΉ�����Jmp�R�}���h����

bool ButiScript::Compiler::JmpBreakLabel()
{
	if (break_index < 0)
		return false;
	OpJmp(break_index);
	return true;
}

// �u���b�N���ł́A�V�����ϐ��Z�b�g�ɕϐ���o�^����

void ButiScript::Compiler::BlockIn()
{
	int start_addr = 0;					// �ϐ��A�h���X�̊J�n�ʒu
	if (variables.size() >= 1) {			// �u���b�N�̓���q�́A�J�n�A�h���X�𑱂�����ɂ���B
		start_addr = variables.back().size();
	}
	variables.push_back(ValueTable(start_addr));
}

// �u���b�N�̏I���ŁA�ϐ��X�R�[�v��������i�ϐ��Z�b�g���폜����j

void ButiScript::Compiler::BlockOut()
{
	variables.pop_back();
}

// ���[�J���ϐ��p�ɃX�^�b�N���m��

void ButiScript::Compiler::AllocStack()
{
	variables.back().Alloc(this);
}

// ���x������
//
// �P�D�A�h���X�𐶐�����
// �Q�D�_�~�[�̃��x���R�}���h���L�����A�h���X���A���x���e�[�u���ɓo�^����
// �R�DJmp�R�}���h�̔�ѐ�����x���e�[�u���ɓo�^���ꂽ�A�h���X�ɂ���

// �A�h���X�v�Z
struct calc_addr {
	std::vector<ButiScript::Label>& labels_;
	int& pos_;
	calc_addr(std::vector<ButiScript::Label>& labels, int& pos) : labels_(labels), pos_(pos)
	{
	}
	void operator()(const ButiScript::VMCode& code)
	{
		if (code.op_ == ButiScript::VM_MAXCOMMAND) {			// ���x���̃_�~�[�R�}���h
			labels_[code.GetConstValue<int>()].pos_ = pos_;
		}
		else {
			pos_ += code.size_;
		}
	}
};

// �W�����v�A�h���X�ݒ�
struct set_addr {
	std::vector<ButiScript::Label>& labels_;
	set_addr(std::vector<ButiScript::Label>& labels) : labels_(labels)
	{
	}
	void operator()(ButiScript::VMCode& code)
	{
		switch (code.op_) {
		case ButiScript:: VM_JMP:
		case ButiScript::VM_JMPC:
		case ButiScript::VM_JMPNC:
		case ButiScript::VM_TEST:
		case ButiScript::VM_CALL:
			code.SetConstValue( labels_[code.GetConstValue<int>()].pos_);
			break;
		}
	}
};

int ButiScript::Compiler::LabelSetting()
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
	void operator()(const ButiScript::VMCode& code)
	{
		p = code.Get(p);
	}
};

bool ButiScript::Compiler::CraeteData(ButiScript::Data& Data, int code_size)
{
	const FunctionTag* tag = GetFunctionTag("main",std::vector<int>(),false);	// �J�n�ʒu
	if (tag == 0) {
		error("�֐� \"main\" ��������܂���B");
		return false;
	}

	Data.commandTable = new unsigned char[code_size];
	Data.textBuffer = new char[text_table.size()];
	Data.commandSize = code_size;
	Data.textSize = (int)text_table.size();
	Data.valueSize = (int)variables[0].size();
	Data.entryPoint = labels[tag->index_].pos_;
	Data.vec_sysCalls = vec_sysCalls;

	if (Data.textSize != 0)
		memcpy(Data.textBuffer, &text_table[0], Data.textSize);

	std::for_each(statement.begin(), statement.end(), copy_code(Data.commandTable));

	return true;
}

void ButiScript::Compiler::PushNameSpace(NameSpace_t arg_namespace)
{
	if (currentNameSpace) {
		arg_namespace->SetParent(currentNameSpace);
	}
	currentNameSpace = arg_namespace;
}

void ButiScript::Compiler::PopNameSpace()
{
	if (currentNameSpace) {
		currentNameSpace = currentNameSpace->GetParent();
	}
}

// �f�o�b�O�_���v
#ifdef	_DEBUG
void ButiScript::Compiler::debug_dump()
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
			std::cout << ", " << statement[i].GetConstValue<int>();
		}
		std::cout << std::endl;

		if (statement[i].op_ != VM_MAXCOMMAND) {
			pos += statement[i].size_;
		}
	}
}
#endif

const std::string& ButiScript::NameSpace::GetNameString() const
{
	return name;
}

std::string ButiScript::NameSpace::GetGlobalNameString() const
{
	if (shp_parentNamespace) {
		return shp_parentNamespace->GetGlobalNameString() + name+ "::";
	}

	if (name.size()==0) {
		return "";
	}

	return name+"::";
}

void ButiScript::NameSpace::Regist(Compiler* arg_compiler)
{
	arg_compiler->PushNameSpace(std::make_shared<NameSpace>(name));
}

void ButiScript::NameSpace::SetParent(std::shared_ptr<NameSpace> arg_parent)
{
	shp_parentNamespace = arg_parent;
}

std::shared_ptr<ButiScript::NameSpace> ButiScript::NameSpace::GetParent() const
{
	return shp_parentNamespace;
}

void ButiScript::ValueTable::Alloc(Compiler* arg_comp) const
{
	auto end = variables_.rend();
	for (auto itr = variables_.rbegin(); itr != end; itr++)
	{
		arg_comp->OpAllocStack(itr->second.type_);
	}
}
