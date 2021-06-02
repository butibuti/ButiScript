#include"stdafx.h"
#include "VM_value.h"

bool ButiScript::VirtualString::EqByString(VirtualString* arg_other)
{
	return str_==arg_other->str_;
}

bool ButiScript::VirtualString::GtByString(VirtualString* arg_other)
{
	return  arg_other->str_>str_ ;
}

bool ButiScript::VirtualString::GeByString(VirtualString* arg_other)
{
	return arg_other->str_>=str_ ;
}

void ButiScript::VirtualString::SetByInt(VirtualInteger* arg_v)
{
	str_ = std::to_string(*arg_v->GetIntPtr());
}

void ButiScript::VirtualString::SetByFloat(VirtualFloat* arg_v)
{
	str_ = std::to_string(*arg_v->GetFloatPtr());
}

void ButiScript::VirtualString::SetByString(VirtualString* arg_v)
{
	str_ = arg_v->str_;
}

bool ButiScript::VirtualInteger::EqByInt(VirtualInteger* arg_other)
{
	return arg_other->v==v;
}

bool ButiScript::VirtualInteger::GtByInt(VirtualInteger* arg_other)
{
	return arg_other->v > v;
}

bool ButiScript::VirtualInteger::GeByInt(VirtualInteger* arg_other)
{
	return arg_other->v >= v;
}

bool ButiScript::VirtualInteger::EqByFloat(VirtualFloat* arg_other)
{
	return arg_other->v ==(float) v;
}

bool ButiScript::VirtualInteger::GtByFloat(VirtualFloat* arg_other)
{
	return arg_other->v > (float)v;
}

bool ButiScript::VirtualInteger::GeByFloat(VirtualFloat* arg_other)
{
	return arg_other->v >= (float)v;
}

void ButiScript::VirtualInteger::SetByInt(VirtualInteger* arg_v)
{
	v = (*arg_v->GetIntPtr());
}

void ButiScript::VirtualInteger::SetByFloat(VirtualFloat* arg_v)
{
	v = (int)(*arg_v->GetFloatPtr());
}

bool ButiScript::VirtualFloat::EqByInt(VirtualInteger* arg_other)
{
	return (float) arg_other->v == v;
}

bool ButiScript::VirtualFloat::GtByInt(VirtualInteger* arg_other)
{
	return (float)arg_other->v > v;
}

bool ButiScript::VirtualFloat::GeByInt(VirtualInteger* arg_other)
{
	return (float)arg_other->v >= v;
}

bool ButiScript::VirtualFloat::EqByFloat(VirtualFloat* arg_other)
{
	return arg_other->v == v;
}

bool ButiScript::VirtualFloat::GtByFloat(VirtualFloat* arg_other)
{
	return arg_other->v > v;
}

bool ButiScript::VirtualFloat::GeByFloat(VirtualFloat* arg_other)
{
	return arg_other->v >= v;
}

void ButiScript::VirtualFloat::SetByInt(VirtualInteger* arg_v)
{
	v = (float)(*arg_v->GetIntPtr());
}

void ButiScript::VirtualFloat::SetByFloat(VirtualFloat* arg_v)
{
	v = (*arg_v->GetFloatPtr());
}
