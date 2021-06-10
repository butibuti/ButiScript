#pragma once

#undef VMCODE0
#undef VMCODE1
#undef VMCODE2
#undef VMCODE3
#define	VMCODE0(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;
#define	VMCODE1(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;
#define	VMCODE2(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;
#define	VMCODE3(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;

#include"VM_code.h"