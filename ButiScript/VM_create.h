#pragma once

#define	VMCODE0(code_, name_)	void name_()			{ statement.push_back(VMCode(code_)); }
#define	VMCODE1(code_, name_)	void name_(int arg1)	{ statement.push_back(VMCode(code_, arg1)); }
#define	VMCODE2(code_, name_)	void name_(float arg1)	{ statement.push_back(VMCode(code_, arg1)); }


#include"VM_code.h"