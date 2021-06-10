#pragma once
#ifndef VALUETYPE_H
#define VALUETYPE_H
enum VALUE_TYPE {
	TYPE_INTEGER,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_VOID,
	TYPE_REF = 0x80,
	TYPE_INTEGER_REF = TYPE_INTEGER | TYPE_REF,
	TYPE_FLOAT_REF = TYPE_FLOAT | TYPE_REF,
	TYPE_STRING_REF = TYPE_STRING | TYPE_REF,
};
#endif