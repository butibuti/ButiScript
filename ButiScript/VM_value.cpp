#include"stdafx.h"
#include "VM_value.h"

bool ButiVM::VirtualString::EqByString(VirtualString* arg_other)
{
	return str_==arg_other->str_;
}

bool ButiVM::VirtualString::GtByString(VirtualString* arg_other)
{
	return  arg_other->str_>str_ ;
}

bool ButiVM::VirtualString::GeByString(VirtualString* arg_other)
{
	return arg_other->str_>=str_ ;
}

void ButiVM::VirtualString::SetByInt(VirtualInteger* arg_v)
{
	str_ = std::to_string(*arg_v->GetIntPtr());
}

void ButiVM::VirtualString::SetByFloat(VirtualFloat* arg_v)
{
	str_ = std::to_string(*arg_v->GetFloatPtr());
}

void ButiVM::VirtualString::SetByString(VirtualString* arg_v)
{
	str_ = arg_v->str_;
}

bool ButiVM::VirtualInteger::EqByInt(VirtualInteger* arg_other)
{
	return arg_other->v==v;
}

bool ButiVM::VirtualInteger::GtByInt(VirtualInteger* arg_other)
{
	return arg_other->v > v;
}

bool ButiVM::VirtualInteger::GeByInt(VirtualInteger* arg_other)
{
	return arg_other->v >= v;
}

bool ButiVM::VirtualInteger::EqByFloat(VirtualFloat* arg_other)
{
	return arg_other->v ==(float) v;
}

bool ButiVM::VirtualInteger::GtByFloat(VirtualFloat* arg_other)
{
	return arg_other->v > (float)v;
}

bool ButiVM::VirtualInteger::GeByFloat(VirtualFloat* arg_other)
{
	return arg_other->v >= (float)v;
}

void ButiVM::VirtualInteger::SetByInt(VirtualInteger* arg_v)
{
	v = (*arg_v->GetIntPtr());
}

void ButiVM::VirtualInteger::SetByFloat(VirtualFloat* arg_v)
{
	v = (int)(*arg_v->GetFloatPtr());
}

bool ButiVM::VirtualFloat::EqByInt(VirtualInteger* arg_other)
{
	return (float) arg_other->v == v;
}

bool ButiVM::VirtualFloat::GtByInt(VirtualInteger* arg_other)
{
	return (float)arg_other->v > v;
}

bool ButiVM::VirtualFloat::GeByInt(VirtualInteger* arg_other)
{
	return (float)arg_other->v >= v;
}

bool ButiVM::VirtualFloat::EqByFloat(VirtualFloat* arg_other)
{
	return arg_other->v == v;
}

bool ButiVM::VirtualFloat::GtByFloat(VirtualFloat* arg_other)
{
	return arg_other->v > v;
}

bool ButiVM::VirtualFloat::GeByFloat(VirtualFloat* arg_other)
{
	return arg_other->v >= v;
}

void ButiVM::VirtualFloat::SetByInt(VirtualInteger* arg_v)
{
	v = (float)(*arg_v->GetIntPtr());
}

void ButiVM::VirtualFloat::SetByFloat(VirtualFloat* arg_v)
{
	v = (*arg_v->GetFloatPtr());
}
