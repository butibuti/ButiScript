#pragma once

#undef VMCODE0
#undef VMCODE1
#undef VMCODE2
#undef VMCODE3
#define	VMCODE0(code_, name_)	code_,
#define	VMCODE1(code_, name_)	code_,
#define	VMCODE2(code_, name_)	code_,
#define	VMCODE3(code_, name_)	code_,

#include"VM_code.h"