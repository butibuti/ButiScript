#pragma once
#ifndef	__PARSER_H__
#define	__PARSER_H__

#include <string>
namespace ButiScript {

	class Compiler;
	// \•¶‰ğÍ
	bool ScriptParse(const std::string& path, Compiler* driver);
}


#endif
