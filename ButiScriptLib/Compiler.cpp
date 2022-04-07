#include "stdafx.h"
#include "Compiler.h"
#include"BuiltInTypeRegister.h"
#include"VirtualMachine.h"
#include <iomanip>
#include "Parser.h"
#include<direct.h>
auto baseFunc = &ButiScript::VirtualMachine::Initialize;
const char* thisPtrName = "this";
constexpr std::int32_t functionStackSize = 4;
namespace AccessModifierStr {
const char* publicStr = "public";
const char* protectedStr = "protected";
const char* privateStr = "private";
}
ButiScript::AccessModifier ButiScript::StringToAccessModifier(const std::string& arg_modifierStr)
{
	if (arg_modifierStr == AccessModifierStr::publicStr) {
		return AccessModifier::Public;
	}
	if (arg_modifierStr == AccessModifierStr::privateStr) {
		return AccessModifier::Private;
	}
	if (arg_modifierStr == AccessModifierStr::protectedStr) {
		return AccessModifier::Protected;
	}
	return AccessModifier::Public;
}

// �R���X�g���N�^
ButiScript:: Compiler::Compiler()
	: break_index(-1), error_count(0)
{
}

ButiScript::Compiler::~Compiler()
{
	types.Release();
}

ButiScript::Compiler* p_instance;

void ButiScript::Compiler::CreateBaseInstance()
{
	p_instance = new Compiler();
	p_instance->RegistDefaultSystems();
}

ButiScript::Compiler* ButiScript::Compiler::GetBaseInstance()
{
	if (!p_instance) {
		CreateBaseInstance();
	}
	return p_instance;
}

void ButiScript::Compiler::RegistDefaultSystems()
{
	types = SystemTypeRegister::GetInstance()->types;
	enums = SystemTypeRegister::GetInstance()->enums;
	map_valueAllocCallsIndex = SystemTypeRegister::GetInstance()->map_valueAllocCallsIndex;
	map_refValueAllocCallsIndex = SystemTypeRegister::GetInstance()->map_refValueAllocCallsIndex;
	list_valueAllocCall = SystemTypeRegister::GetInstance()->list_valueAllocCall;
	list_refValueAllocCall = SystemTypeRegister::GetInstance()->list_refValueAllocCall;
	functions = SystemFuntionRegister::GetInstance()->functions;
	map_sysCallsIndex = SystemFuntionRegister::GetInstance()->map_sysCallsIndex;
	map_sysMethodCallsIndex = SystemFuntionRegister::GetInstance()->map_sysMethodCallsIndex;
	list_sysCalls = SystemFuntionRegister::GetInstance()->list_sysCalls;
	list_sysMethodCalls =SystemFuntionRegister::GetInstance()->list_sysMethodCalls;


	//�O���[�o�����O��Ԃ̐ݒ�
	globalNameSpace = ButiEngine::make_value<NameSpace>("");
	PushNameSpace(globalNameSpace);

}

// �R���p�C��
bool ButiScript::Compiler::Compile(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data)
{

	//�ϐ��e�[�u�����Z�b�g
	variables.Add(ValueTable());
	variables[0].set_global();
	OpHalt();
	bool result = ScriptParse(arg_filePath, this);	// �\�����

	if (!result) {

		labels.Clear();
		statement.Clear();
		text_table.Clear();
		variables.Clear();
		functions.Clear_notSystem();
		types.Clear_notSystem();
		enums.Clear_notSystem();
		error_count = 0;
		return true;// �p�[�T�[�G���[
	}

	std::int32_t code_size = LabelSetting();				// ���x���ɃA�h���X��ݒ�
	CreateData(arg_ref_data, code_size);				// �o�C�i������

	arg_ref_data.sourceFilePath = arg_filePath;

	labels.Clear();
	statement.Clear();
	text_table.Clear();
	variables.Clear();
	functions.Clear_notSystem();
	types.Clear_notSystem();
	enums.Clear_notSystem();

	return false;
}


// �G���[���b�Z�[�W���o��
void ButiScript::Compiler::error(const std::string& arg_message)
{
	std::cerr << arg_message << std::endl;
	error_count++;
}

void ButiScript::Compiler::ClearStatement()
{
	statement.Clear();
}

std::string ButiScript::Compiler::GetTypeName(const std::int32_t arg_type) const
{
	std::int32_t type = arg_type & ~TYPE_REF;
	std::string output = "";
	
	output = types.GetType(type)->typeName;

	if (arg_type&TYPE_REF) {
		output += "&";
	}

	return output;
}

void ButiScript::Compiler::LambdaCountReset()
{
	lambdaCount = 0;
}

void ButiScript::Compiler::PushAnalyzeFunction(Function_t arg_function)
{
	PushNameSpace(ButiEngine::make_value<NameSpace>(arg_function->GetName()));
	if (list_parentFunction.GetSize()) {
		arg_function->SetParent(list_parentFunction.GetLast());
	}
	list_parentFunction.Add(arg_function);

}

void ButiScript::Compiler::PopAnalyzeFunction()
{
	if (list_parentFunction.GetSize()) {
		list_parentFunction.erase(list_parentFunction.end() - 1);
		PopNameSpace();
	}
}

void ButiScript::Compiler::PushSubFunction(Function_t arg_function)
{
	list_parentFunction.GetLast()->AddSubFunction(arg_function);
}

void ButiScript::Compiler::PushAnalyzeClass(Class_t arg_class)
{
	currentNameSpace->PushClass(arg_class);
}

void ButiScript::Compiler::ClearNameSpace()
{
	list_namespaces.Clear();
	list_namespaces.Add(globalNameSpace);
}

void ButiScript::Compiler::Analyze()
{
	auto nameSpaceBuffer = currentNameSpace;
	for (auto namespaceItr = list_namespaces.begin(), namespaceEnd = list_namespaces.end(); namespaceItr != namespaceEnd; namespaceItr++) {
		currentNameSpace = (*namespaceItr);
		(*namespaceItr)->AnalyzeClasses(this);
		(*namespaceItr)->AnalyzeFunctions(this);
	}
	currentNameSpace = nameSpaceBuffer;
}

void ButiScript::Compiler::IncreaseLambdaCount()
{
	lambdaCount++;
}

void ButiScript::Compiler::PopCurrentFunctionType()
{
	if (list_function_type.GetSize()) {
		list_function_type.erase((list_function_type.end()-1));
	}
}

void ButiScript::Compiler::PopCurrentFunctionName()
{
	if (list_function_name.GetSize()) {
		list_function_name.erase((list_function_name.end() - 1));
	}
}

void ButiScript::Compiler::ClearGlobalNameSpace()
{
	globalNameSpace->Clear();
}



// �O���ϐ��̒�`
void DefineValue(ButiScript::Compiler* arg_p_comp, const std::int32_t arg_type, ButiScript::AccessModifier arg_access, ButiScript::Node_t arg_node) {

	arg_p_comp->AddValue(arg_type, arg_node->GetString(), arg_node->GetLeft(), arg_access);
}

void ButiScript::Compiler::AnalyzeScriptType(const std::string& arg_typeName, const std::map < std::string, std::pair< std::int32_t, AccessModifier>>& arg_memberInfo)
{
	auto typeTag = GetType(arg_typeName);
	if (arg_memberInfo.size()) {
		auto memberInfoEnd = arg_memberInfo.end();
		std::int32_t memberIndex = 0;
		for (auto itr = arg_memberInfo.begin(); itr != memberInfoEnd; memberIndex++,itr++) {
			if (typeTag->typeIndex == itr->second.first) {
				error("�N���X "+itr->first + "�����g�Ɠ����^�������o�ϐ��Ƃ��ĕێ����Ă��܂��B");
			}
			MemberValueInfo info = { memberIndex ,itr->second.first,itr->second.second };
			typeTag->map_memberValue.emplace(itr->first, info);

		}
	}
}

void ButiScript::Compiler::RegistScriptType(const std::string& arg_typeName)
{

	TypeTag type;
	std::int32_t typeIndex = types.GetSize();
	type.isSystem = false;


	type.typeName = arg_typeName;
	type.typeIndex = typeIndex;
	type.argName = arg_typeName;


	types.RegistType(type);
}

void ButiScript::Compiler::ValueDefine(std::int32_t arg_type, const ButiEngine::List<Node_t>& arg_node, const AccessModifier arg_access)
{
	for (auto& valueNode : arg_node) {
		DefineValue(this, arg_type, arg_access,valueNode);
	}
}

// �֐��錾
void ButiScript::Compiler::FunctionDefine(std::int32_t arg_type, const std::string& arg_name, const ButiEngine::List<std::int32_t>& arg_list_argIndex)
{
	const FunctionTag* tag = functions.Find_strict(arg_name,arg_list_argIndex,&GetTypeTable());
	if (tag) {			// ���ɐ錾�ς�
		if (!tag->CheckArgList_strict(arg_list_argIndex)) {
			error("�֐� " + arg_name + " �ɈقȂ�^�̈������w�肳��Ă��܂�");
			return;
		}
	}
	else {
		FunctionTag func(arg_type, arg_name);
		func.SetArgs(arg_list_argIndex);				// ������ݒ�
		func.SetDeclaration();			// �錾�ς�
		func.SetIndex(MakeLabel());		// ���x���o�^
		if (functions.Add(arg_name, func, &GetTypeTable()) == 0) {
			error("�����G���[�F�֐��e�[�u���ɓo�^�ł��܂���");
		}
	}
}




ButiScript::FunctionTag* ButiScript::Compiler::RegistFunction(const std::int32_t arg_type, const std::string& arg_name, const ButiEngine::List<ArgDefine>& arg_list_argDefines, Block_t arg_block, const AccessModifier arg_access,FunctionTable* arg_funcTable )
{
	std::string functionName =arg_funcTable? arg_name: currentNameSpace->GetGlobalNameString() + arg_name;
	arg_funcTable = arg_funcTable ? arg_funcTable : &functions;

	FunctionTag* tag = arg_funcTable->Find_strict(functionName,arg_list_argDefines,&GetTypeTable());
	if (tag) {			// ���ɐ錾�ς�
		if (!tag->CheckArgList_strict(arg_list_argDefines)) {
			error("�֐� " + functionName + " �ɈقȂ�^�̈������w�肳��Ă��܂�");
			return tag;
		}
	}
	else {
		FunctionTag func(arg_type, functionName);
		func.SetArgs(arg_list_argDefines);				// ������ݒ�
		func.SetDeclaration();			// �錾�ς�
		func.SetIndex(MakeLabel());		// ���x���o�^
		func.SetAccessType(arg_access);
		if (arg_funcTable->Add(functionName, func, &GetTypeTable()) == 0) {
			error("�����G���[�F�֐��e�[�u���ɓo�^�ł��܂���");
		}
	}
	return  arg_funcTable->Find_strict(functionName, arg_list_argDefines,&GetTypeTable());
}

void ButiScript::Compiler::RegistLambda(const std::int32_t arg_type, const std::string& arg_name, const ButiEngine::List<ArgDefine>& arg_list_argDefines, FunctionTable* arg_functionTable)
{
	auto tag= RegistFunction(arg_type,arg_name , arg_list_argDefines, nullptr, AccessModifier::Public, arg_functionTable);
	tag->isLambda = true;
}

void ButiScript::Compiler::RegistEnum(const std::string& arg_typeName, const std::string& arg_identiferName, const std::int32_t arg_value)
{
	auto enumType = GetEnumTag(arg_typeName);
	if (!enumType) {
		RegistEnumType(arg_typeName);
		enumType = enums.FindType(arg_typeName);
	}

	enumType->SetValue(arg_identiferName, arg_value);
}

void ButiScript::Compiler::RegistEnumType(const std::string& arg_typeName)
{
	auto typeName = currentNameSpace->GetGlobalNameString()+arg_typeName;
	EnumTag tag(typeName);
	enums.SetEnum(tag);
	types.RegistType(*enums.FindType(arg_typeName));
}

std::int32_t ButiScript::Compiler::GetfunctionTypeIndex(const ButiEngine::List<ArgDefine>& arg_list_argmentTypes, const std::int32_t arg_retType)
{
	ButiEngine::List<std::int32_t> list_argTypes;
	for (auto& argmentType : arg_list_argmentTypes) {
		list_argTypes.Add(argmentType.GetType());
	}

	
	return GetfunctionTypeIndex(list_argTypes,arg_retType);
}
std::int32_t ButiScript::Compiler::GetfunctionTypeIndex(const ButiEngine::List<std::int32_t>& arg_list_argmentTypes, const std::int32_t arg_retType)
{
	auto type = types.GetFunctionType(arg_list_argmentTypes, arg_retType);
	if (!type) {
		type = types.CreateFunctionType(arg_list_argmentTypes, arg_retType);
	}
	return type->typeIndex;
}


// �ϐ��̓o�^
void ButiScript::Compiler::AddValue(const std::int32_t arg_typeIndex, const std::string& arg_name, Node_t arg_node ,const AccessModifier arg_access)
{
	std::string valueName = GetCurrentNameSpace()->GetGlobalNameString() + arg_name;
	std::int32_t size = 1;
	if (arg_node) {
		if (arg_node->Op() != OP_INT) {
			error("�z��̃T�C�Y�͒萔�Ŏw�肵�Ă��������B");
		}
		else if (arg_node->GetNumber() <= 0) {
			error("�z��̃T�C�Y�͂P�ȏ�̒萔���K�v�ł��B");
		}
		size = arg_node->GetNumber();
	}

	ValueTable& values = variables.GetLast();
	if (!values.Add(arg_typeIndex, valueName, arg_access,size)) {
		error("�ϐ� " + valueName + " �͊��ɓo�^����Ă��܂��B");
	}
}

// ���x������
std::int32_t ButiScript::Compiler::MakeLabel()
{
	std::int32_t index = (std::int32_t)labels.GetSize();
	labels.Add(Label(index));
	return index;
}

// ���x���̃_�~�[�R�}���h���X�e�[�g�����g���X�g�ɓo�^����
void ButiScript::Compiler::SetLabel(const std::int32_t arg_label)
{
	statement.Add(VMCode(VM_MAXCOMMAND, arg_label));
}

// ������萔��push
void ButiScript::Compiler::PushString(const std::string& arg_str)
{
	PushString(((std::int32_t)text_table.GetSize()));
	text_table.Insert(text_table.end(), arg_str.begin(), arg_str.end());
	text_table.Add('\0');
}

// break���ɑΉ�����Jmp�R�}���h����

bool ButiScript::Compiler::JmpBreakLabel()
{
	if (break_index < 0) {
		return false;
	}
	OpJmp(break_index);
	return true;
}

// �u���b�N���ł͐V�����ϐ��Z�b�g�ɕϐ���o�^����
void ButiScript::Compiler::BlockIn(const bool arg_isFunctionBlock, const bool arg_isSubFunctionBlock)
{
	std::int32_t start_addr = 0;					
	if (variables.GetSize() > 1&&!arg_isSubFunctionBlock) {			
		start_addr = variables.GetLast().size();
	}
	variables.Add(ValueTable(start_addr,arg_isFunctionBlock));
}

void ButiScript::Compiler::BlockOut()
{
	variables.RemoveLast();
}

void ButiScript::Compiler::ValueAddressAddition(const std::int32_t arg_difference)
{
	auto difference = arg_difference;
	std::int32_t endIndex = 1;
	for (std::int32_t index = variables.GetSize() - 2; index >= endIndex; index--) {
		difference=variables[index].AddressAdd(difference);
		if (variables[index - 1].IsFunctionBlock()) {
			difference += functionStackSize;
		}
	}
}

void ButiScript::Compiler::ValueAddressSubtract(const std::int32_t arg_difference)
{
	auto difference = arg_difference;
	std::int32_t endIndex = 1;
	for (std::int32_t index = variables.GetSize() - 2; index >= endIndex; index--) {
		difference = variables[index].AddressSub(difference);
		if (variables[index - 1].IsFunctionBlock()) {
			difference -= functionStackSize;
		}
	}
}

// ���[�J���ϐ��p�ɃX�^�b�N���m��
void ButiScript::Compiler::AllocStack()
{
	variables.GetLast().Alloc(this);
}

// �A�h���X�v�Z
void CalcAddress(ButiEngine::List<ButiScript::Label>& arg_list_labels, std::int32_t& arg_pos, const ButiScript::VMCode& arg_code) {

	if (arg_code.op == ButiScript::VM_MAXCOMMAND) {			// ���x���̃_�~�[�R�}���h
		arg_list_labels[arg_code.GetConstValue<std::int32_t>()].pos = arg_pos;
	}
	else {
		arg_pos += arg_code.size;
	}
}
void SetAddress(ButiEngine::List<ButiScript::Label>& arg_list_labels,   ButiScript::VMCode& arg_code) {
	switch (arg_code.op) {
	case ButiScript::VM_JMP:
	case ButiScript::VM_JMPC:
	case ButiScript::VM_JMPNC:
	case ButiScript::VM_TEST:
	case ButiScript::VM_CALL:
	case ButiScript::VM_PUSHFUNCTIONOBJECT:
	case ButiScript::VM_PUSHRAMDA:
		arg_code.SetConstValue(arg_list_labels[arg_code.GetConstValue<std::int32_t>()].pos);
		break;
	}
}

std::int32_t ButiScript::Compiler::LabelSetting()
{
	// �A�h���X�v�Z
	std::int32_t pos = 0;
	for (auto& statementItr : statement)
	{
		CalcAddress(labels, pos, statementItr);
	}
	for (auto& statementItr : statement)
	{
		SetAddress(labels,  statementItr);
	}

	return pos;
}

// �o�C�i���f�[�^����
std::uint8_t* CodeCopy(std::uint8_t* arg_p_code, const ButiScript::VMCode& arg_code) {

	return arg_code.Get(arg_p_code);
}


bool ButiScript::Compiler::CreateData(ButiScript::CompiledData& arg_ref_data, std::int32_t arg_codeSize)
{

	for (std::int32_t index = 0; index < functions.Size(); index++) {
		auto func = functions[index];
		if (func->IsSystem()) {
			continue;
		}
		auto name = functions[index]->GetName();
		if (!arg_ref_data.map_entryPoints.count(name)) {
			arg_ref_data.map_entryPoints.emplace(name, labels[functions[index]->index].pos);
		}
		else {
			arg_ref_data.map_entryPoints.emplace(functions[index]->GetNameWithArgment(types), labels[functions[index]->index].pos);
		}
	}
	
	for (auto itr = arg_ref_data.map_entryPoints.begin(), end = arg_ref_data.map_entryPoints.end(); itr != end;itr++) {

		const std::string* p_str = &itr->first;
		arg_ref_data.map_functionJumpPointsTable.emplace(itr->second, p_str);
	}

	arg_ref_data.functions = functions;

	arg_ref_data.commandTable = new std::uint8_t[arg_codeSize];
	arg_ref_data.textBuffer = new char[text_table.GetSize()];
	arg_ref_data.commandSize = arg_codeSize;
	arg_ref_data.textSize = (std::int32_t)text_table.GetSize();
	arg_ref_data.valueSize = (std::int32_t)variables[0].size();

	arg_ref_data.list_sysCalls = list_sysCalls;
	arg_ref_data.list_sysCallMethods = list_sysMethodCalls;
	types.CreateTypeVec(arg_ref_data.list_types);
	arg_ref_data.list_scriptClassInfo = types.GetScriptClassInfo();
	arg_ref_data.functionTypeCount = types.GetFunctionTypeSize();
	for (std::int32_t index = 0; index < arg_ref_data.valueSize; index++) {
		auto p_value = &variables[0][index];
		if (p_value->access == AccessModifier::Public) {
			arg_ref_data.map_globalValueAddress.emplace(variables[0].GetVariableName(index), p_value->GetAddress());
		}
	}
	for (std::int32_t index = 0; index < enums.Size();index++) {
		arg_ref_data.map_enumTag.emplace(enums[index]->typeIndex, *enums[index]);
	}
	if (arg_ref_data.textSize != 0) {
		memcpy(arg_ref_data.textBuffer, &text_table[0], arg_ref_data.textSize);
	}
	auto commandStartPtr = arg_ref_data.commandTable;
	for (auto& statementItr : statement) {
		arg_ref_data.commandTable=CodeCopy(arg_ref_data.commandTable, statementItr);
	}
	arg_ref_data.commandTable = commandStartPtr;
	for (auto& statementItr : statement) {
		statementItr.Release();
	}
	return true;
}

void ButiScript::Compiler::PushNameSpace(NameSpace_t arg_namespace)
{
	if (!arg_namespace) {
		arg_namespace = ButiEngine::make_value<NameSpace>("");
	}
	if (currentNameSpace) {
		arg_namespace->SetParent(currentNameSpace);
	}
	currentNameSpace = arg_namespace;
	list_namespaces.Add(arg_namespace);
}

void ButiScript::Compiler::PopNameSpace()
{
	if (currentNameSpace) {
		currentNameSpace = currentNameSpace->GetParent();
	}
}

// �f�o�b�O�_���v
#ifdef	_DEBUG

void ButiScript::Compiler::DebugDump()

#ifdef BUTIGUI_H

// �f�o�b�O�_���v
{
	std::string message = "---variables---\n";
	std::uint64_t vsize = variables.GetSize();
	message += "value stack = " + std::to_string(vsize) + '\n';
	for (std::uint64_t index = 0; index < vsize; index++) {
		variables[index].Dump();
	}
	message += "---code---" + '\n';

	static const char* op_name[] = {
#define	VM_NAMETABLE
#include "VM_nametable.h"
#undef	VM_NAMETABLE
		"LABEL",
	};

	std::int32_t	pos = 0;
	std::uint64_t size = statement.GetSize();
	for (std::uint64_t index = 0; index < size; index++) {
		message += std::to_string(pos) + ": " + op_name[statement[index].op];
		if (statement[index].size > 1) {
			message += ", " + std::to_string(statement[index].GetConstValue<std::int32_t>());
		}
		message += '\n';

		if (statement[index].op != VM_MAXCOMMAND) {
			pos += statement[index].size;
		}
	}
	ButiEngine::GUI::Console(message);
}
#else

{
	std::cout << "---variables---" << std::endl;
	std::uint64_t vsize = variables.GetSize();
	std::cout << "value stack = " << vsize << std::endl;
	for (std::uint64_t i = 0; i < vsize; i++) {
		variables[i].Dump();
	}
	std::cout << "---code---" << std::endl;

	static const char* op_name[] = {
#define	VM_NAMETABLE
#include "VM_nametable.h"
#include "Tags.h"
#undef	VM_NAMETABLE
		"LABEL",
	};

	std::int32_t	pos = 0;
	std::uint64_t size = statement.GetSize();
	for (std::uint64_t i = 0; i < size; i++) {
		std::cout << std::setw(6) << pos << ": " << op_name[statement[i].op];
		if (statement[i].size > 1) {
			std::cout << ", " << statement[i].GetConstValue<std::int32_t>();
		}
		std::cout << std::endl;

		if (statement[i].op != VM_MAXCOMMAND) {
			pos += statement[i].size;
		}
	}
}
#endif // BUTIGUI_H
#endif // _DEBUG


std::int32_t ButiScript::Compiler::InputCompiledData(const std::string& arg_filePath, ButiScript::CompiledData& arg_ref_data)
{
	std::ifstream fIn;
	fIn.open(arg_filePath,std::ios::binary);
	if (!fIn.is_open()) {
		fIn.close();
		return 1;
	}


	std::int32_t sourceFilePathStrSize = 0;
	fIn.read((char*)&sourceFilePathStrSize, sizeof(sourceFilePathStrSize));
	char* sourceFilePathStrBuff = (char*)malloc(sourceFilePathStrSize);
	fIn.read(sourceFilePathStrBuff, sourceFilePathStrSize);
	arg_ref_data.sourceFilePath = std::string(sourceFilePathStrBuff, sourceFilePathStrSize);
	free(sourceFilePathStrBuff);


	fIn.read((char*)&arg_ref_data.commandSize, sizeof(std::int32_t));
	arg_ref_data.commandTable = (std::uint8_t*)malloc(arg_ref_data.commandSize);
	fIn.read((char*)arg_ref_data.commandTable, arg_ref_data.commandSize);

	fIn.read((char*)&arg_ref_data.textSize, sizeof(std::int32_t));
	arg_ref_data.textBuffer = (char*)malloc(arg_ref_data.textSize);
	fIn.read((char*)arg_ref_data.textBuffer, arg_ref_data.textSize);


	fIn.read((char*)&arg_ref_data.valueSize, sizeof(std::int32_t));

	std::int32_t sysCallSize=0;
	fIn.read((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		std::int32_t index=0;
		fIn.read((char*)&index, sizeof(index));
		SysFunction sysFunc = list_sysCalls[index];
		arg_ref_data.list_sysCalls.Add(sysFunc);
	}



	fIn.read((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		std::int32_t index = 0;
		fIn.read((char*)&index, sizeof(index));
		SysFunction sysFunc = list_sysMethodCalls[index];
		arg_ref_data.list_sysCallMethods.Add(sysFunc);
	}


	fIn.read((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		TypeTag typeTag;

		std::int32_t size=0;
		fIn.read((char*)&size, sizeof(size));
		char* p_strBuff = (char*)malloc(size);
		fIn.read(p_strBuff, size);
		typeTag.argName =std::string( p_strBuff,size);
		free(p_strBuff);

		fIn.read((char*)&size, sizeof(size));
		p_strBuff = (char*)malloc(size);
		fIn.read(p_strBuff, size);
		typeTag.typeName = std::string(p_strBuff, size);
		free(p_strBuff);

		std::int32_t index = 0;
		fIn.read((char*)&index, sizeof(index));
		SysFunction sysFunc=nullptr;
		if (index < list_valueAllocCall.GetSize()&&index>=0) {
			sysFunc = list_valueAllocCall[index];
		}
		typeTag.typeFunc = sysFunc;

		index = 0;
		fIn.read((char*)&index, sizeof(index));
		if (index < list_refValueAllocCall.GetSize() && index >= 0) {
			sysFunc = list_refValueAllocCall[index];
		}
		typeTag.refTypeFunc = sysFunc;




		fIn.read((char*)&typeTag.typeIndex, sizeof(std::int32_t));

		std::int32_t typeMapSize = 0;
		fIn.read((char*)&typeMapSize, sizeof(typeMapSize));

		for (std::int32_t j=0; j < typeMapSize;j++) {
			std::int32_t size =0;
			std::string typeNameStr;
			MemberValueInfo memberInfo;
			fIn.read((char*)&size, sizeof(size));
			char* p_strBuff = (char*)malloc(size);
			free(p_strBuff);
			fIn.read(p_strBuff, size);
			typeNameStr = std::string(p_strBuff,size);
			fIn.read((char*)&memberInfo, sizeof(memberInfo));
			typeTag.map_memberValue.emplace(typeNameStr, memberInfo);
		}

		typeTag.methods.FileInput(fIn);
		typeTag.isSystem = true;
		arg_ref_data.list_types.Add(typeTag);
	}


	std::int32_t entryPointsSize = 0;
	fIn.read((char*)&entryPointsSize, sizeof(entryPointsSize));
	
	for (std::int32_t count = 0; count < entryPointsSize;count++) {
		

		std::int32_t size =0, entryPoint = 0;

		fIn.read((char*)&size, sizeof(size));
		char* buff = (char*)malloc(size);
		fIn.read(buff, size);
		std::string name = std::string(buff, size);
		free(buff); 
		fIn.read((char*)&entryPoint, sizeof(std::int32_t));
		std::int32_t current = fIn.tellg();
		arg_ref_data.map_entryPoints.emplace(name, entryPoint);

	}
	for (auto entryPointItr = arg_ref_data.map_entryPoints.begin(), endItr = arg_ref_data.map_entryPoints.end(); entryPointItr != endItr; entryPointItr++) {

		arg_ref_data.map_functionJumpPointsTable.emplace(entryPointItr->second, &entryPointItr->first);
	}

	std::int32_t publicGlobalValue = 0;
	fIn.read((char*)&publicGlobalValue, sizeof(publicGlobalValue));
	for (std::int32_t count = 0; count < publicGlobalValue; count++) {
		std::int32_t strSize = 0;
		fIn.read((char*)&strSize, sizeof(std::int32_t));
		char* strBuff = (char*)malloc(strSize);
		fIn.read(strBuff, strSize);
		std::int32_t address = 0;
		fIn.read((char*)&address, sizeof(std::int32_t));
		arg_ref_data.map_globalValueAddress.emplace(std::string(strBuff, strSize), address);
		arg_ref_data.map_addressToValueName.emplace(address, std::string(strBuff, strSize));
		free(strBuff);
	}

	std::int32_t definedTypeCount = 0;
	fIn.read((char*)&definedTypeCount, sizeof(definedTypeCount));
	arg_ref_data.list_scriptClassInfo.Resize(definedTypeCount);
	for (std::int32_t index = 0; index < definedTypeCount; index++) {
		arg_ref_data.list_scriptClassInfo.At(index).InputFile(fIn);
	}

	fIn.read((char*)&arg_ref_data.functionTypeCount, sizeof(arg_ref_data.functionTypeCount));

	arg_ref_data.systemTypeCount = arg_ref_data.list_types.GetSize() - definedTypeCount-arg_ref_data.functionTypeCount;
	for (std::int32_t index = 0; index < definedTypeCount; index++) {
		arg_ref_data.list_scriptClassInfo.At(index).SetSystemTypeCount(arg_ref_data.systemTypeCount);
		arg_ref_data.list_types.At(arg_ref_data.list_scriptClassInfo.At(index).GetTypeIndex()).isSystem = false;
	}
	std::int32_t enumCount =0;
	fIn.read((char*)&enumCount, sizeof(enumCount));
	for (std::int32_t index = 0; index < enumCount;index++) {
		EnumTag tag;
		std::int32_t typeIndex = 0;
		fIn.read((char*)&typeIndex, sizeof(std::int32_t));
		tag.InputFile(fIn);
		arg_ref_data.map_enumTag.emplace(typeIndex,tag);
		arg_ref_data.map_enumTag.at(typeIndex).CreateEnumMap();
		arg_ref_data.list_types.At(typeIndex).p_enumTag = &arg_ref_data.map_enumTag.at(typeIndex);
	}

	arg_ref_data.functions.FileInput(fIn);

	fIn.close();

	return 0;
}

std::int32_t ButiScript::Compiler::OutputCompiledData(const std::string& arg_filePath, const ButiScript::CompiledData& arg_ref_data)
{

	auto dirPath = StringHelper::GetDirectory(arg_filePath);
	if (dirPath != arg_filePath) {
		auto dirRes = _mkdir(dirPath.c_str());
	}
	std::ofstream fOut(arg_filePath,  std::ios::binary);
	std::int32_t sourcePathSize = arg_ref_data.sourceFilePath.size();
	fOut.write((char*)&sourcePathSize,sizeof(sourcePathSize));
	fOut.write(arg_ref_data.sourceFilePath.c_str(),sourcePathSize);

	fOut.write((char*)&arg_ref_data.commandSize, sizeof(std::int32_t));
	fOut.write((char*)arg_ref_data.commandTable, arg_ref_data.commandSize);

	fOut.write((char*)&arg_ref_data.textSize, sizeof(std::int32_t));
	fOut.write((char*)arg_ref_data.textBuffer,  arg_ref_data.textSize);


	fOut.write((char*)&arg_ref_data.valueSize, sizeof(std::int32_t));

	std::int32_t sysCallSize = arg_ref_data.list_sysCalls.GetSize();
	fOut.write((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		 auto p_sysCallFunc=arg_ref_data.list_sysCalls[count];
		 std::int64_t address = *(std::int64_t*) & p_sysCallFunc;
		 std::int32_t index = map_sysCallsIndex.at(address);

		 fOut.write((char*)&index, sizeof(index));
	}

	sysCallSize = arg_ref_data.list_sysCallMethods.GetSize();
	fOut.write((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		auto p_sysCallFunc = arg_ref_data.list_sysCallMethods[count];
		std::int64_t address = *(std::int64_t*) & p_sysCallFunc;
		std::int32_t index = map_sysMethodCallsIndex.at(address);

		fOut.write((char*)&index, sizeof(index));
	}


	sysCallSize = arg_ref_data.list_types.GetSize();
	fOut.write((char*)&sysCallSize, sizeof(std::int32_t));
	for (std::int32_t count = 0; count < sysCallSize; count++) {
		auto p_type =& arg_ref_data.list_types[count];

		std::int32_t size = p_type->argName.size();
		fOut.write((char*)&size, sizeof(size));
		fOut.write(p_type->argName.c_str(), size); 

		size = p_type->typeName.size();
		fOut.write((char*)&size, sizeof(size));
		fOut.write(p_type->typeName.c_str(),size);

		auto p_sysCallFunc = p_type->typeFunc;

		std::int32_t index = -1;
		if (p_type->isSystem&&!p_type->p_enumTag) {
			index = map_valueAllocCallsIndex.at(p_type->typeIndex);
		}
		fOut.write((char*)&index, sizeof(index));

		p_sysCallFunc = p_type->refTypeFunc;
		if (p_type->isSystem&&!p_type->p_enumTag) {
			index = map_refValueAllocCallsIndex.at(p_type->typeIndex);
		}
		fOut.write((char*)&index, sizeof(index));



		fOut.write((char*)&p_type->typeIndex, sizeof(p_type->typeIndex));

		std::int32_t typeMapSize = p_type->map_memberValue.size();
		fOut.write((char*)&typeMapSize, sizeof(typeMapSize));
		auto end = p_type->map_memberValue.end();
		for (auto itr = p_type->map_memberValue.begin(); itr != end; itr++) {
			std::int32_t size = itr->first.size();
			fOut.write((char*)&size, sizeof(size));
			fOut.write(itr->first.c_str(), size);
			fOut.write((char*)&itr->second, sizeof(itr->second));
		}

		p_type->methods.FileOutput(fOut);

	}

	std::int32_t entryPointsSize = arg_ref_data.map_entryPoints.size();
	fOut.write((char*)&entryPointsSize, sizeof(entryPointsSize));
	auto end = arg_ref_data.map_entryPoints.end();
	for (auto itr = arg_ref_data.map_entryPoints.begin(); itr != end; itr++) {
		std::int32_t size = itr->first.size();
		fOut.write((char*)&size,sizeof(size));
		fOut.write(itr->first.c_str(), size);
		fOut.write((char*)&itr->second, sizeof(itr->second));
	}

	std::int32_t publicGlobalValue = arg_ref_data.map_globalValueAddress.size();
	fOut.write((char*)&publicGlobalValue, sizeof(publicGlobalValue));
	auto itr = arg_ref_data.map_globalValueAddress.begin();
	for (std::int32_t count = 0; count < publicGlobalValue; count++,itr++) {
		std::int32_t strSize =itr->first.size();
		fOut.write((char*)&strSize, sizeof(std::int32_t));
		fOut.write(itr->first.c_str(), strSize);
		fOut.write((char*)&itr->second, sizeof(std::int32_t));
	}

	std::int32_t defineTypeCount = arg_ref_data.list_scriptClassInfo.GetSize();
	fOut.write((char*)&defineTypeCount, sizeof(defineTypeCount));
	for (std::int32_t index = 0; index < defineTypeCount; index++) {
		arg_ref_data.list_scriptClassInfo[index].OutputFile(fOut);
	}

	fOut.write((char*)&arg_ref_data.functionTypeCount, sizeof(arg_ref_data.functionTypeCount));


	std::int32_t enumCount = arg_ref_data.map_enumTag.size();
	fOut.write((char*)&enumCount, sizeof(enumCount));
	for (auto itr = arg_ref_data.map_enumTag.begin(), end = arg_ref_data.map_enumTag.end(); itr != end; itr++) {
		fOut.write((char*)&itr->first,sizeof(std::int32_t));
		itr->second.OutputFile(fOut);

	}
	arg_ref_data.functions.FileOutput(fOut);
	
	fOut.close();

	return 0;
}
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
	arg_compiler->PushNameSpace(ButiEngine::make_value<NameSpace>(name));
}

void ButiScript::NameSpace::SetParent(NameSpace_t arg_parent)
{
	shp_parentNamespace = arg_parent;
}

ButiScript::NameSpace_t ButiScript::NameSpace::GetParent() const
{
	return shp_parentNamespace;
}

void ButiScript::NameSpace::PushFunction(Function_t arg_func)
{
	list_analyzeFunctionBuffer.Add(arg_func);
}

void ButiScript::NameSpace::PushClass(Class_t arg_class)
{
	list_analyzeClassBuffer.Add(arg_class);
}



void ButiScript::NameSpace::AnalyzeFunctions(Compiler* arg_compiler)
{
	for (auto itr = list_analyzeFunctionBuffer.begin(), end = list_analyzeFunctionBuffer.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler, nullptr);
	}
}

void ButiScript::NameSpace::AnalyzeClasses(Compiler* arg_compiler)
{
	for (auto itr = list_analyzeClassBuffer.begin(), end = list_analyzeClassBuffer.end(); itr != end; itr++) {
		(*itr)->Analyze(arg_compiler);
	}
}

void ButiScript::NameSpace::Clear()
{
	list_analyzeClassBuffer.Clear();
	list_analyzeFunctionBuffer.Clear();
}


void ButiScript::ValueTable::Alloc(Compiler* arg_comp) const
{
	auto end = list_variableTypes.end();
	for (auto itr = list_variableTypes.begin(); itr != end; itr++)
	{
		if (*itr & TYPE_REF) {
			std::int32_t typeIndex = *itr &~TYPE_REF;
			auto type = arg_comp->GetType(typeIndex);
			if (type->isSystem) {
				arg_comp->OpAllocStack_Ref(*itr);
			}
			else  if (type->p_enumTag) {
				arg_comp->OpAllocStack_Ref_EnumType(*itr);
			}
			else  if (type->IsFunctionObjectType()) {
				arg_comp->OpAllocStack_Ref_FunctionType(*itr);
			}
			else {
				arg_comp->OpAllocStack_Ref_ScriptType(*itr - arg_comp->GetSystemTypeSize());
			}
		}
		else {
			auto type = arg_comp->GetType(*itr);
			if (type->p_enumTag) {
				arg_comp->OpAllocStackEnumType(*itr);
			}else if (type->isSystem) {
				arg_comp->OpAllocStack(*itr);
			}
			else  if (type->IsFunctionObjectType()) {
				arg_comp->OpAllocStackFunctionType(*itr);
			}
			else
			{
				arg_comp->OpAllocStack_ScriptType(*itr - arg_comp->GetSystemTypeSize());
			}
		}
		
	}
}

ButiScript::SystemTypeRegister* p_sysreginstance ;

ButiScript::SystemTypeRegister* ButiScript::SystemTypeRegister::GetInstance()
{
	if (!p_sysreginstance) {

		p_sysreginstance = new SystemTypeRegister();

		p_sysreginstance->SetDefaultSystemType();

	}
	return p_sysreginstance;
}
void ButiScript::SystemTypeRegister::SetDefaultSystemType()
{
	RegistSystemType<std::int32_t>("std::int32_t", "i");
	RegistSystemType<float>("float", "f");
	RegistSystemType<std::string>("string", "s");
	RegistSystemType<Type_Null>("void", "v");
	RegistSystemType<ButiEngine::Vector2,Type_hasMember<ButiEngine::Vector2>>("Vector2", "vec2", "x:f,y:f");
	RegistSystemType<ButiEngine::Vector3,Type_hasMember<ButiEngine::Vector3>>("Vector3", "vec3", "x:f,y:f,z:f");
	RegistSystemType<ButiEngine::Vector4,Type_hasMember<ButiEngine::Vector4>>("Vector4", "vec4", "x:f,y:f,z:f,w:f");
	RegistSystemType<ButiEngine::Matrix4x4,Type_hasMember<ButiEngine::Matrix4x4>>("Matrix4x4", "Matrix4x4", "_m11:f,_m12:f,_m13:f,_m14:f,_m21:f,_m22:f,_m23:f,_m24:f,_m31:f,_m32:f,_m33:f,_m34:f,_m41:f,_m42:f,_m43:f,_m44:f");
}
void ButiScript::SystemTypeRegister::RegistSystemEnumType(const std::string& arg_typeName)
{
	EnumTag tag(arg_typeName);
	tag.isSystem = true;
	enums.SetEnum(tag);
	types.RegistType(*enums.FindType(arg_typeName));
}
void ButiScript::SystemTypeRegister::RegistEnum(const std::string& arg_typeName, const std::string& arg_identiferName, const std::int32_t arg_value)
{
	auto enumType = enums.FindType(arg_typeName);
	if (!enumType) {
		RegistSystemEnumType(arg_typeName);
		enumType = enums.FindType(arg_typeName);
	}

	enumType->SetValue(arg_identiferName, arg_value);
}
std::int32_t ButiScript::SystemTypeRegister::GetIndex(const std::string& arg_typeName)
{
	auto tag = types.GetType(arg_typeName);
	if (tag) {
		return tag->typeIndex;
	}
	return -1;
}

ButiScript::SystemFuntionRegister* p_sysfuncregister;
ButiScript::SystemFuntionRegister* ButiScript::SystemFuntionRegister::GetInstance()
{
	if (!p_sysfuncregister) {
		p_sysfuncregister = new SystemFuntionRegister();
		p_sysfuncregister->SetDefaultFunctions();
	}

	return p_sysfuncregister;
}

void ButiScript::SystemFuntionRegister::SetDefaultFunctions()
{
	using namespace ButiEngine;
	DefineSystemFunction(&VirtualMachine::sys_print, TYPE_VOID, "print", "s");
	//DefineSystemFunction(&VirtualMachine::sys_debugPrint, TYPE_VOID, "print_debug", "s");
	DefineSystemFunction(&VirtualMachine::Sys_pause, TYPE_VOID, "pause", "");
	DefineSystemFunction(&VirtualMachine::sys_tostr, TYPE_STRING, "ToString", "i");
	DefineSystemFunction(&VirtualMachine::sys_tostr, TYPE_STRING, "ToString", "f");
	DefineSystemFunction(&VirtualMachine::sys_tostr, TYPE_STRING, "ToString", "vec2");
	DefineSystemFunction(&VirtualMachine::sys_tostr, TYPE_STRING, "ToString", "vec3");
	DefineSystemFunction(&VirtualMachine::sys_tostr, TYPE_STRING, "ToString", "vec4");
#ifdef _BUTIENGINEBUILD

	DefineSystemFunction(&VirtualMachine::sys_registEventListner, TYPE_STRING, "RegistEvent", "s,s,s");
	DefineSystemFunction(&VirtualMachine::sys_unregistEventListner, TYPE_VOID, "UnRegistEvent", "s,s");
	DefineSystemFunction(&VirtualMachine::sys_addEventMessanger, TYPE_VOID, "AddEventMessanger", "s");
	DefineSystemFunction(&VirtualMachine::sys_removeEventMessanger, TYPE_VOID, "RemoveEventMessanger", "s");
	DefineSystemFunction(&VirtualMachine::sys_executeEvent, TYPE_VOID, "EventExecute", "s");
	DefineSystemFunction(&VirtualMachine::sys_pushTask, TYPE_VOID, "PushTask", "s");
#endif // _BUTIENGINEBUILD

	DefineSystemMethod(&VirtualMachine::sys_method_cast< &std::string::size, std::int32_t, &VirtualMachine::GetTypePtr  >, TYPE_STRING, TYPE_INTEGER, "Size", "");

	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::Normalize, &VirtualMachine::GetTypePtr >, TYPE_VOID + 1, TYPE_VOID, "Normalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetNormalize, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetNormalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetLength, &VirtualMachine::GetTypePtr >, TYPE_VOID + 1, TYPE_FLOAT, "GetLength", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetLengthSqr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_FLOAT, "GetLengthSqr", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::Dot, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_FLOAT, "Dot", "vec2");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::Floor, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, (TYPE_VOID + 1), "Floor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::Round, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, (TYPE_VOID + 1), "Round", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::Ceil, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 1, (TYPE_VOID + 1), "Ceil", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetFloor, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetFloor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetRound, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetRound", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector2::GetCeil, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 1, TYPE_VOID + 1, "GetCeil", "i");

	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::Normalize, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID, "Normalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetNormalize, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 2, TYPE_VOID + 2, "GetNormalize", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetLength, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_FLOAT, "GetLength", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetLengthSqr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_FLOAT, "GetLengthSqr", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::Dot, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_FLOAT, "Dot", "vec3");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetCross, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetCross", "vec3");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::Floor, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 2, (TYPE_VOID + 2), "Floor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::Round, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 2, (TYPE_VOID + 2), "Round", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::Ceil, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, (TYPE_VOID + 2), "Ceil", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetFloor, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr  >, TYPE_VOID + 2, TYPE_VOID + 2, "GetFloor", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetRound, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetRound", "i");
	DefineSystemMethod(&VirtualMachine::sys_method< &Vector3::GetCeil, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 2, TYPE_VOID + 2, "GetCeil", "i");

	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::Transpose, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "Transpose", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetTranspose, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "GetTranspose", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::Inverse, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "Inverse", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetInverse, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "GetInverse", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::Identity, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "Identity", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetEulerOneValue, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetEulerOneValue", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetPosition, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetPosition", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetPosition_Transpose, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetPosition_Transpose", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::GetEulerOneValue_local, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 2, "GetEulerOneValue_local", "");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::CreateFromEuler, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "CreateFromEuler", "vec3");
	DefineSystemMethod(&VirtualMachine::sys_method< &Matrix4x4::CreateFromEuler_local, &VirtualMachine::GetTypePtr, &VirtualMachine::GetTypePtr >, TYPE_VOID + 4, TYPE_VOID + 4, "CreateFromEuler_local", "vec3");
	
}

bool ButiScript::SystemFuntionRegister::DefineSystemFunction(SysFunction arg_op, const std::int32_t arg_retType, const std::string& arg_name, const std::string& arg_list_argDefines, const ButiEngine::List<std::int32_t>& arg_list_templateTypes)
{
	FunctionTag func(arg_retType, arg_name);
	if (!func.SetArgs(arg_list_argDefines, SystemTypeRegister::GetInstance()->types .GetArgmentKeyMap()))
		return false;

	func.SetDeclaration();
	func.SetSystem();				// System�t���O�Z�b�g
	std::int32_t index = list_sysCalls.GetSize();
	func.SetIndex(index);			// �g�ݍ��݊֐��ԍ���ݒ�
	func.SetTemplateType(arg_list_templateTypes);
	std::int64_t address = *(std::int64_t*) & arg_op;
	map_sysCallsIndex.emplace(address, list_sysCalls.GetSize());
	list_sysCalls.Add(arg_op);
	if (functions.Add(arg_name, func, &SystemTypeRegister::GetInstance()->types) == 0) {
		return false;
	}
	return true;
}

bool ButiScript::SystemFuntionRegister::DefineSystemMethod(SysFunction arg_p_method, const std::int32_t arg_type, const std::int32_t arg_retType, const std::string& arg_name, const std::string& arg_args, const ButiEngine::List<std::int32_t>& arg_list_templateTypes)
{
	FunctionTag func(arg_retType, arg_name);
	if (!func.SetArgs(arg_args, SystemTypeRegister::GetInstance()->types.GetArgmentKeyMap()))
		return false;

	func.SetDeclaration();
	func.SetSystem();				// System�t���O�Z�b�g
	func.SetIndex(list_sysMethodCalls.GetSize());			// �g�ݍ��݊֐��ԍ���ݒ�
	func.SetTemplateType(arg_list_templateTypes);
	std::int64_t address = *(std::int64_t*) & arg_p_method;
	map_sysMethodCallsIndex.emplace(address, list_sysMethodCalls.GetSize());
	list_sysMethodCalls.Add(arg_p_method);
	auto typeTag = SystemTypeRegister::GetInstance()->types.GetType(arg_type);
	if (typeTag->AddMethod(arg_name, func,&SystemTypeRegister::GetInstance()->types) == 0) {
		return false;
	}
	return true;
}
std::int32_t ButiScript::TypeTag::GetFunctionObjectReturnType() const
{
	if (!p_functionObjectData) {
		return -1;
	}

	return p_functionObjectData->returnType;
}
std::int32_t ButiScript::TypeTag::GetFunctionObjectArgSize() const
{
	if (!p_functionObjectData) {
		return -1;
	}
	return p_functionObjectData->list_argTypes.GetSize();
}

const ButiEngine::List<std::int32_t>& ButiScript::TypeTag::GetFunctionObjectArgment() const
{

	return p_functionObjectData->list_argTypes;
}

void ButiScript::TypeTable::Release() {
	for (auto& type: map_types) {
		type.second.Release();
	}
	map_types.clear();
	map_argmentChars.clear();
	list_types.Clear();
}

#ifndef _BUTIENGINEBUILD
auto compilerRelease = MemoryReleaser<ButiScript::Compiler>(&p_instance);
auto sysTypeRelease = MemoryReleaser<ButiScript::SystemTypeRegister>(&p_sysreginstance);
auto sysFuncRelease = MemoryReleaser<ButiScript::SystemFuntionRegister>(&p_sysfuncregister);
#else

auto compilerRelease = ButiEngine::Util::MemoryReleaser(&p_instance);
auto sysTypeRelease = ButiEngine::Util::MemoryReleaser(&p_sysreginstance);
auto sysFuncRelease = ButiEngine::Util::MemoryReleaser(&p_sysfuncregister);
#endif
