#pragma once

#undef VMCODE0
#undef VMCODE1
#undef VMCODE2
#undef VMCODE3
#undef VMCODE_T
#undef VMCODE_T0

#define	VMCODE0(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;
#define	VMCODE1(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;
#define	VMCODE2(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;
#define	VMCODE3(code_, name_)	p_op[code_] = &VirtualCPU:: name_ ;

#define	VMCODE_T(code_, name_)	p_op[code_] = &VirtualCPU:: name_ <int>;
#define VMCODE_T0(code_, name_ ,Type_) p_op[code_] = &VirtualCPU:: name_ <Type_>;
#define	VMCODE_TU(code_, name_)					p_op[code_] = &VirtualCPU:: name_ <int,int>;
#define	VMCODE_TU0(code_, name_, Type_T, Type_U) p_op[code_] = &VirtualCPU:: name_ <Type_T,Type_U>;

#include"VM_code.h"