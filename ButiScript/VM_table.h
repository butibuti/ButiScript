#pragma once

#define	VMCODE0(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;
#define	VMCODE1(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;
#define	VMCODE2(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;

#include"VM_code.h"