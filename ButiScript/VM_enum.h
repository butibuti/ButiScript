#pragma once

#undef VMCODE0
#undef VMCODE1
#undef VMCODE2
#undef VMCODE3
#undef VMCODE_T
#undef VMCODE_T0

#define	VMCODE0(code_, name_)	code_,
#define	VMCODE1(code_, name_)	code_,
#define	VMCODE2(code_, name_)	code_,
#define	VMCODE3(code_, name_)	code_,
#define	VMCODE_T(code_, name_)	code_,
#define	VMCODE_T0(code_, name_, Type_)	code_,

#include"VM_code.h"