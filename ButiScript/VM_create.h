#pragma once

#undef VMCODE0
#undef VMCODE1
#undef VMCODE2
#undef VMCODE3

#define	VMCODE0(code_, name_)	void name_()			{ statement.push_back(VMCode(code_)); }
#define	VMCODE1(code_, name_)	void name_(const int arg1)	{ statement.push_back(VMCode(code_, arg1)); }
#define	VMCODE2(code_, name_)	void name_(const float arg1)	{ statement.push_back(VMCode(code_, arg1)); }
#define	VMCODE3(code_, name_)	void name_(const int arg1,const int arg2)	{PushConstInt(arg1); statement.push_back(VMCode(code_, arg2)); }


#include"VM_code.h"