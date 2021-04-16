#pragma once
#ifndef	__PARSER_H__
#define	__PARSER_H__

#include <string>

class Compiler;
bool ScriptParser(const std::string& path, Compiler* driver);

#endif
