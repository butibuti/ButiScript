#ifndef _BUTISCRIPT_COMMON_H
#define _BUTISCRIPT_COMMON_H
#include<string>
namespace ButiEngine {
class Vector4;
}
namespace ButiScript {


void SetPrintFunction(void (*arg_printFunction)(const std::string&) );
void SetGUITextFunction( void (*arg_GUITextFunction)(const std::string&));
void SetColorPrintFunction( void (*arg_colorPrintFunction)(const std::string&, const ButiEngine::Vector4&));
void SetTreeNodePushFunction( bool (*arg_treeNodePushFunction)(const std::string&) );
void SetTreeNodePopFunction( void (*arg_treeNodePopFunction)());


void (*GetPrintFunction())(const std::string&) ;
void (*GetGUITextFunction())(const std::string&);
void (*GetColorPrintFunction())(const std::string&, const ButiEngine::Vector4&);
bool (*GetTreeNodePushFunction())(const std::string&) ;
void (*GetTreeNodePopFunction())() ;

}
#endif // !_BUTISCRIPT_COMMON_H
