#pragma once
#ifndef	__PARSER_H__
#define	__PARSER_H__

#include <string>
namespace ButiScript {

	class Compiler;
	/// <summary>
	/// 構文解析
	/// </summary>
	/// <param name="arg_filePath">コンパイルするファイルパス</param>
	/// <param name="arg_compiler">使用するコンパイラ</param>
	/// <returns>成功</returns>
	bool ScriptParse(const std::string& arg_filePath, Compiler* arg_compiler);
}


#endif
