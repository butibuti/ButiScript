#pragma once


#ifdef BUTISCRIPTCOMPILER_EXPORTS
#define BUTISCRIPT_CMP_API __declspec(dllexport)
#else
#define BUTISCRIPT_CMP_API __declspec(dllimport)
#endif