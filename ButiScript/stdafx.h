#ifndef STDAFX_H
#define STDAFX_H

#include <vector>
#include <string>
#include"ButiMath/ButiMath.h"
#include"ButiMemorySystem/ButiMemorySystem/ButiPtr.h"
#include"ButiMemorySystem/ButiMemorySystem/ButiList.h"
#ifndef BOOST_INCLUDE_H
#define BOOST_INCLUDE_H
#include <boost/bind.hpp>
#include <boost/spirit.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix1_functions.hpp>
#include <boost/spirit/include/phoenix1_new.hpp>
#include <boost/mem_fn.hpp>
#endif
#include <memory>
#include <iostream>
#include <fstream>
#include <map>
#include <cassert>
#include <algorithm>
#include <functional>
#ifndef _BUTIENGINEBUILD
template<typename T>
class MemoryReleaser {
public:
	MemoryReleaser(T** arg_p_memoryAddress) :p_memoryAddress(arg_p_memoryAddress) {}
	~MemoryReleaser()
	{
		if (*p_memoryAddress) {
			delete (*p_memoryAddress);
		}
	}
private:
	T** p_memoryAddress;
};
#endif
#endif