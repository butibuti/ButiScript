#pragma once

#define	VMCODE0(code_, name_)	case code_: name_(); break;
#define	VMCODE1(code_, name_)	case code_: name_(Value()); break;
#define	VMCODE2(code_, name_)	case code_: name_(Value_Float()); break;

#include"VM_code.h"