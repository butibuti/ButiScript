#pragma once
#ifndef	__PARSER_H__
#define	__PARSER_H__

#include <string>
namespace ButiScript {

	class Compiler;
	/// <summary>
	/// �\�����
	/// </summary>
	/// <param name="arg_filePath">�R���p�C������t�@�C���p�X</param>
	/// <param name="arg_compiler">�g�p����R���p�C��</param>
	/// <returns>����</returns>
	bool ScriptParse(const std::string& arg_filePath, Compiler* arg_compiler);
}


#endif
