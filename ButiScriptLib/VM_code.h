VMCODE1(VM_PUSHCONST, PushConstInt)
VMCODE2(VM_PUSHCONSTFLOAT, PushConstFloat)
VMCODE1(VM_PUSHSTRING, PushString)
VMCODE1(VM_PUSHVALUEREF, PushGlobalValueRef)
VMCODE1(VM_PUSHLOCALREF, PushLocalRef)
VMCODE1(VM_PUSHMEMBERREF, PushMemberRef)
VMCODE1(VM_PUSHARRAYREF, PushGlobalArrayRef)
VMCODE1(VM_PUSHLOCALARRAYREF, PushLocalArrayRef)
VMCODE1(VM_PUSHVALUE, PushGlobalValue)
VMCODE1(VM_PUSHLOCAL, PushLocal)
VMCODE1(VM_PUSHMEMBER, PushMember)
VMCODE1(VM_PUSHARRAY, PushGlobalArray)
VMCODE1(VM_PUSHLOCALARRAY, PushLocalArray)
VMCODE1(VM_PUSHADDR, PushAddr)
VMCODE0(VM_PUSHNULL, PushNull)
VMCODE1(VM_PUSHARRAYADDR, PushArrayAddr)
VMCODE1(VM_POPVALUE, PopValue)
VMCODE1(VM_POPLOCAL, PopLocal)
VMCODE1(VM_POPMEMBER, PopMember)
VMCODE1(VM_POPMEMBERREF, PopMemberRef)
VMCODE1(VM_POPARRAY, PopArray)
VMCODE1(VM_POPLOCALARRAY, PopLocalArray)
VMCODE1(VM_POPLOCALREF, PopLocalRef)
VMCODE1(VM_POPLOCALARRAYREF, PopLocalArrayRef)
VMCODE1(VM_ALLOCSTACK, OpAllocStack)
VMCODE1(VM_ENUMALLOCSTACK, OpAllocStackEnumType)
VMCODE1(VM_ALLOCSTACKSCRIPTTYPE, OpAllocStack_ScriptType)
VMCODE1(VM_ALLOCSTACKFUNCTIONTYPE, OpAllocStackFunctionType)
VMCODE1(VM_ALLOCSTACKREF, OpAllocStack_Ref)
VMCODE1(VM_ALLOCSTACKREFENUMTYPE, OpAllocStack_Ref_EnumType)
VMCODE1(VM_ALLOCSTACKREFSCRIPTTYPE, OpAllocStack_Ref_ScriptType)
VMCODE1(VM_ALLOCSTACKREFFUNCTIONTYPE, OpAllocStack_Ref_FunctionType)
VMCODE0(VM_POP, OpPop)
VMCODE0(VM_NEG, OpNeg)
VMCODE0(VM_NOT, OpNot)
VMCODE0(VM_INCREMENT, OpIncrement)
VMCODE0(VM_DECREMENT, OpDecrement)
VMCODE0(VM_EQ, OpEq)
VMCODE0(VM_NE, OpNe)
VMCODE0(VM_GT, OpGt)
VMCODE0(VM_GE, OpGe)
VMCODE0(VM_LT, OpLt)
VMCODE0(VM_LE, OpLe)
VMCODE0(VM_LOGAND, OpLogAnd)
VMCODE0(VM_LOGOR, OpLogOr)
VMCODE0(VM_AND, OpAnd)
VMCODE0(VM_OR, OpOr)
VMCODE0(VM_LSHIFT, OpLeftShift)
VMCODE0(VM_RSHIFT, OpRightShift)
VMCODE_T(VM_ADD, OpAdd)
VMCODE_T(VM_SUB, OpSub)
VMCODE_T(VM_MULTI, OpMul)
VMCODE_T(VM_DIV, OpDiv)
VMCODE_T(VM_MOD, OpMod)
VMCODE_TU(VM_MULTID, OpMul)
VMCODE_TU(VM_DIVD, OpDiv)
VMCODE_T0(VM_ADDINT, OpAdd, int)
VMCODE_T0(VM_SUBINT, OpSub,int)
VMCODE_T0(VM_MULINT, OpMul,int)
VMCODE_T0(VM_DIVINT, OpDiv,int)
VMCODE_T0(VM_MODINT, OpMod,int)
VMCODE_T0(VM_ADDFLOAT, OpAdd, float)
VMCODE_T0(VM_SUBFLOAT, OpSub, float)
VMCODE_T0(VM_MULFLOAT, OpMul, float)
VMCODE_T0(VM_DIVFLOAT, OpDiv, float)
VMCODE_T0(VM_MODFLOAT, OpMod, float)
VMCODE_T0(VM_ADDVEC2, OpAdd, ButiEngine::Vector2)
VMCODE_T0(VM_SUBVEC2, OpSub, ButiEngine::Vector2)
VMCODE_T0(VM_MULVEC2, OpMul, ButiEngine::Vector2)
VMCODE_T0(VM_DIVVEC2, OpDiv, ButiEngine::Vector2)
VMCODE_TU0(VM_MULVEC2INT, OpMul, ButiEngine::Vector2, int)
VMCODE_TU0(VM_DIVVEC2INT, OpDiv, ButiEngine::Vector2, int)
VMCODE_TU0(VM_MULVEC2FLOAT, OpMul, ButiEngine::Vector2, float)
VMCODE_TU0(VM_DIVVEC2FLOAT, OpDiv, ButiEngine::Vector2, float)
VMCODE_TU0(VM_MULINTVEC2, OpMul, int, ButiEngine::Vector2)
VMCODE_TU0(VM_MULFLOATVEC2, OpMul, float, ButiEngine::Vector2)
VMCODE_T0(VM_ADDVEC3, OpAdd, ButiEngine::Vector3)
VMCODE_T0(VM_SUBVEC3, OpSub, ButiEngine::Vector3)
VMCODE_T0(VM_MULVEC3, OpMul, ButiEngine::Vector3)
VMCODE_T0(VM_DIVVEC3, OpDiv, ButiEngine::Vector3)
VMCODE_TU0(VM_MULVEC3INT, OpMul, ButiEngine::Vector3, int)
VMCODE_TU0(VM_DIVVEC3INT, OpDiv, ButiEngine::Vector3, int)
VMCODE_TU0(VM_MULVEC3FLOAT, OpMul, ButiEngine::Vector3, float)
VMCODE_TU0(VM_DIVVEC3FLOAT, OpDiv, ButiEngine::Vector3, float)
VMCODE_TU0(VM_MULINTVEC3, OpMul, int, ButiEngine::Vector3)
VMCODE_TU0(VM_MULFLOATVEC3, OpMul, float, ButiEngine::Vector3)
VMCODE_T0(VM_ADDVEC4, OpAdd, ButiEngine::Vector4)
VMCODE_T0(VM_SUBVEC4, OpSub, ButiEngine::Vector4)
VMCODE_T0(VM_MULVEC4, OpMul, ButiEngine::Vector4)
VMCODE_T0(VM_DIVVEC4, OpDiv, ButiEngine::Vector4)
VMCODE_TU0(VM_MULVEC4INT, OpMul, ButiEngine::Vector4, int)
VMCODE_TU0(VM_DIVVEC4INT, OpDiv, ButiEngine::Vector4, int)
VMCODE_TU0(VM_MULVEC4FLOAT, OpMul, ButiEngine::Vector4, float)
VMCODE_TU0(VM_DIVVEC4FLOAT, OpDiv, ButiEngine::Vector4, float)
VMCODE_TU0(VM_MULINTVEC4, OpMul, int, ButiEngine::Vector4)
VMCODE_TU0(VM_MULFLOATVEC4, OpMul, float, ButiEngine::Vector4)
VMCODE_T0(VM_ADDMAT4, OpAdd, ButiEngine::Matrix4x4)
VMCODE_T0(VM_SUBMAT4, OpSub, ButiEngine::Matrix4x4)
VMCODE_T0(VM_MULMAT4, OpMul, ButiEngine::Matrix4x4)
VMCODE_TU0(VM_MULMAT4INT, OpMul, ButiEngine::Matrix4x4, int)
VMCODE_TU0(VM_MULMAT4FLOAT, OpMul, ButiEngine::Matrix4x4, float)
VMCODE_TU0(VM_MULINTMAT4, OpMul, int, ButiEngine::Matrix4x4)
VMCODE_TU0(VM_MULFLOATMAT4, OpMul, float, ButiEngine::Matrix4x4)
VMCODE0(VM_STREQ, OpStrEq)
VMCODE0(VM_STRNE, OpStrNe)
VMCODE0(VM_STRGT, OpStrGt)
VMCODE0(VM_STRGE, OpStrGe)
VMCODE0(VM_STRLT, OpStrLt)
VMCODE0(VM_STRLE, OpStrLe)
VMCODE0(VM_STRADD, OpStrAdd)
VMCODE1(VM_JMP, OpJmp)
VMCODE1(VM_JMPC, OpJmpC)
VMCODE1(VM_JMPNC, OpJmpNC)
VMCODE1(VM_TEST, OpTest)
VMCODE1(VM_CALL, OpCall)
VMCODE0(VM_CALLBYVARIABLE, OpCallByVariable)
VMCODE1(VM_PUSHFUNCTIONOBJECT, OpPushFunctionAddress)
VMCODE1(VM_PUSHRAMDA, OpPushLambda)
VMCODE1(VM_SYSCALL, OpSysCall)
VMCODE1(VM_SYSCALLMETHOD, OpSysMethodCall)
VMCODE0(VM_RETURN, OpReturn)
VMCODE0(VM_RETURNV, OpReturnV)
VMCODE0(VM_HALT, OpHalt)

