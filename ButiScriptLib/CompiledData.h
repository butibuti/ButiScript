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

	std::uint8_t* commandTable;	// コマンドテーブル
	char* textBuffer;			// テキストデータ
	std::int32_t commandSize;			// コマンドサイズ
	std::int32_t textSize;				// テキストサイズ
	std::int32_t valueSize;			// グローバル変数サイズ

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

