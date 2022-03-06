#pragma once

#undef VMCODE0
#undef VMCODE1
#undef VMCODE2
#undef VMCODE_T
#undef VMCODE_T0
#undef VMCODE_TU
#undef VMCODE_TU0
#undef VMCODE_TUV
#undef VMCODE_TUV0

#define	VMCODE0(code_, name)	void name()			{ statement.push_back(VMCode(code_)); }
#define	VMCODE1(code_, name)	void name(const std::int32_t arg1)	{ statement.push_back(VMCode(code_, arg1)); }
#define	VMCODE2(code_, name)	void name(const float arg1)	{ statement.push_back(VMCode(code_, arg1)); }

#define	VMCODE_T(code_, name) template<typename T> void name (){   statement.push_back(VMCode(code_)); }
#define	VMCODE_T0(code_, name, Type_)	template< > void name < Type_ > (){   statement.push_back(VMCode(code_)); }
#define	VMCODE_TU(code_, name) template<typename T,typename U> void name (){   statement.push_back(VMCode(code_)); }
#define	VMCODE_TU0(code_, name, Type_T, Type_U) template<  > void name < Type_T,Type_U > (){   statement.push_back(VMCode(code_)); }
#define	VMCODE_TUV(code_, name) template<typename T,typename U,typename V> void name (){   statement.push_back(VMCode(code_)); }
#define	VMCODE_TUV0(code_, name, Type_T, Type_U ,Type_V) template<  > void name < Type_T,Type_U ,Type_V> (){   statement.push_back(VMCode(code_)); }
#include"VM_code.h"