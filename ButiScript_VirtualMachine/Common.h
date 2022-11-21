#pragma once
#ifndef _BUTISCRIPT_VM_COMMON_H
#define _BUTISCRIPT_VM_COMMON_H
#include<string>
#include<ButiMath/ButiMath.h>
#ifdef BUTISCRIPTVIRTUALMACHINE_EXPORTS
#define BUTISCRIPT_VM_API __declspec(dllexport)
#else
#define BUTISCRIPT_VM_API __declspec(dllimport)
#endif
namespace ButiScript {
BUTISCRIPT_VM_API void SetPrintFunction(void (*arg_printFunction)(const std::string&));
BUTISCRIPT_VM_API void SetGUITextFunction(void (*arg_GUITextFunction)(const std::string&));
BUTISCRIPT_VM_API void SetColorPrintFunction(void (*arg_colorPrintFunction)(const std::string&, const ButiEngine::Vector4&));
BUTISCRIPT_VM_API void SetTreeNodePushFunction(bool (*arg_treeNodePushFunction)(const std::string&));
BUTISCRIPT_VM_API void SetTreeNodePopFunction(void (*arg_treeNodePopFunction)());

BUTISCRIPT_VM_API void (*GetPrintFunction())(const std::string&);
BUTISCRIPT_VM_API void (*GetPrintFunction_Integer())(const std::int32_t);
BUTISCRIPT_VM_API void (*GetGUITextFunction())(const std::string&);
BUTISCRIPT_VM_API void (*GetColorPrintFunction())(const std::string&, const ButiEngine::Vector4&);
BUTISCRIPT_VM_API bool (*GetTreeNodePushFunction())(const std::string&);
BUTISCRIPT_VM_API void (*GetTreeNodePopFunction())();

}
#endif // !_BUTISCRIPT_VM_COMMON_H