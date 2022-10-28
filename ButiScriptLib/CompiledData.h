#ifndef BUTISCRIPT_COMPILED_DATA_H
#include<string>
#include<map>
#include<unordered_map>
#include"ButiMemorySystem/ButiMemorySystem/ButiList.h"
#include"Tags.h"
namespace ButiScript {

class CompiledData {
public:
	CompiledData() : commandTable(0), textBuffer(0)
	{
	}
	~CompiledData()
	{
		delete[] commandTable;
		delete[] textBuffer;
	}

	std::uint8_t* commandTable;	// �R�}���h�e�[�u��
	char* textBuffer;			// �e�L�X�g�f�[�^
	std::int32_t commandSize;			// �R�}���h�T�C�Y
	std::int32_t textSize;				// �e�L�X�g�T�C�Y
	std::int32_t valueSize;			// �O���[�o���ϐ��T�C�Y

	ButiEngine::List<OperationFunction> list_sysCalls;
	ButiEngine::List<OperationFunction> list_sysCallMethods;
	ButiEngine::List<TypeTag> list_types;
	std::unordered_map<std::string, std::int32_t> map_entryPoints;
	std::map< std::int32_t, const std::string*> map_functionJumpPointsTable;
	std::map<std::string, std::int32_t>map_globalValueAddress;
	std::map<std::int32_t, std::string>map_addressToValueName;
	std::map<std::int32_t, EnumTag> map_enumTag;
	FunctionTable functions;
	ButiEngine::List<ScriptClassInfo> list_scriptClassInfo;
	std::int32_t systemTypeCount, functionTypeCount;
	std::string sourceFilePath;
};

}
#pragma once
#endif // !BUTISCRIPT_COMPILED_DATA_H

