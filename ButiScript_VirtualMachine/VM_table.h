

#undef VMCODE0
#undef VMCODE1
#undef VMCODE2
#undef VMCODE_T   
#undef VMCODE_T0  
#undef VMCODE_TU  
#undef VMCODE_TU0 
#undef VMCODE_TUV 
#undef VMCODE_TUV0

#define	VMCODE0(code_, name)	p_op[code_] = &VirtualMachine:: name ;
#define	VMCODE1(code_, name)	p_op[code_] = &VirtualMachine:: name ;
#define	VMCODE2(code_, name)	p_op[code_] = &VirtualMachine:: name ;
#define	VMCODE3(code_, name)	p_op[code_] = &VirtualMachine:: name ;

#define	VMCODE_T(code_, name)	p_op[code_] = &VirtualMachine:: name <std::int32_t>;
#define VMCODE_T0(code_, name ,Type_) p_op[code_] = &VirtualMachine:: name <Type_>;
#define	VMCODE_TU(code_, name)					p_op[code_] = &VirtualMachine:: name <std::int32_t,std::int32_t>;
#define	VMCODE_TU0(code_, name, Type_T, Type_U) p_op[code_] = &VirtualMachine:: name <Type_T,Type_U>;
#define	VMCODE_TUV(code_, name)					p_op[code_] = &VirtualMachine:: name <std::int32_t,std::int32_t,std::int32_t>;
#define	VMCODE_TUV0(code_, name, Type_T, Type_U,Type_V) p_op[code_] = &VirtualMachine:: name <Type_T,Type_U,Type_V>;

#include"ButiScriptLib/VM_code.h"