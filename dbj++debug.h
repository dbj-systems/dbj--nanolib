#pragma once
/* (c) 2019, 2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#include <typeinfo>
#include <iostream>
#include <memory>

#ifndef _WIN32
#include <cxxabi.h>
#endif

/*
OS agnostic typename
works with GCC and CLANG on Linux etc ...
*/
#define DBJ_TYPE_NAME(T) dbj::name<T>().c_str()
#define DBJ_TYPENAME(T) dbj::name<decltype(T)>().c_str()

/*
nano debugging 

show expression, it's type and its value
*/
#define DBJ_SX(x) std::cout << "\neXpression:\t\t" << #x << "\ntype\t\t\t" << DBJ_TYPENAME(x) << "\nvalue\t\t\t" << (x) << "\n\n"
/*
show expression and it's type
*/
#define DBJ_SXT(x) std::cout << "\neXpression:\t\t" << #x << "\ntype\t\t\t" << DBJ_TYPENAME(x) << "\n\n"


namespace dbj {


	template<typename T, size_t N>
	constexpr inline auto make_arr_buffer(const T(&string)[N])
	{
	   static_assert( N > 1);

	   DBJ_ARRAY<T, N> buffy_{};
		size_t k{};
		for (auto CH : string)
			buffy_[k++] = CH;
		return buffy_;
	}

	template < typename T >
	const std::string name() noexcept
	{
#ifdef _WIN32
		return { typeid(T).name() };
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

		if (!error)        return { name.get() };
		if (error == -1)   return { "memory allocation failed" };
		if (error == -2)   return { "not a valid mangled name" };
		// else if(error == -3)
		return { "bad argument" };
#endif // __linux__
	} // name()
} // dbj
