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

#define	VMCODE0(code_, name)	#name,
#define	VMCODE1(code_, name)	#name,
#define	VMCODE2(code_, name)	#name,
#define	VMCODE3(code_, name)	#name,
#define	VMCODE_T(code_, name)	#name,
#define	VMCODE_T0(code_, name, Type_)	#name,
#define	VMCODE_TU(code_, name)					#name,
#define	VMCODE_TU0(code_, name, Type_T, Type_U) #name,
#define	VMCODE_TUV(code_, name)					#name,
#define	VMCODE_TUV0(code_, name, Type_T, Type_U,Type_V) #name,
#include"ButiScriptLib/VM_code.h"