#pragma once
/*
   (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/
*/

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <typeinfo>
#include <iostream>
#include <memory>

#ifndef _WIN32
#include <cxxabi.h>
#endif

#include "dbj_buffer.h"

/*
OS agnostic typename
works with GCC and CLANG on Linux etc ...
*/
#define DBJ_TYPE_NAME(T) dbj::nanolib::name<T>().data()
#define DBJ_TYPENAME(T) dbj::nanolib::name<decltype(T)>().data()

namespace dbj::nanolib {

	using typename_buffer = typename v_buffer::buffer_type;

	template < typename T >
	const typename_buffer name() noexcept
	{
#ifdef _WIN32
		return v_buffer::make( typeid(T).name() ) ;
#else // __linux__
		// delete malloc'd memory
		struct free_ {
			void operator()(void* p) const { std::free(p); }
		};
		// custom smart pointer for c-style strings allocated with std::malloc
		using ptr_type = std::unique_ptr<char, free_>;

		// special function to de-mangle names
		int error{};
		ptr_type name{ abi::__cxa_demangle(typeid(T).name(), 0, 0, &error) };

		if (!error)        return v_buffer::make( name.get() );
		if (error == -1)   return v_buffer::make( "memory allocation failed" );
		if (error == -2)   return v_buffer::make( "not a valid mangled name" );
		// else if(error == -3)
		return v_buffer::make( "bad argument" );
#endif // __linux__
	} // name()

} // dbj::nanolib

#ifdef DBJ_TYPENAME
/*
nano debugging

show expression, it's type and its value
*/
#define DBJ_SX(x) std::cout << "\neXpression:\t\t" << #x << "\ntype\t\t\t" << DBJ_TYPENAME(x) << "\nvalue\t\t\t" << (x) << "\n\n"
/*
show expression and it's type
*/
#define DBJ_SXT(x) std::cout << "\neXpression:\t\t" << #x << "\ntype\t\t\t" << DBJ_TYPENAME(x) << "\n\n"

#endif // DBJ_TYPENAME