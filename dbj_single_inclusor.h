#pragma once
#ifndef DBJ_SINGLE_INCLUSOR_INC
#define DBJ_SINGLE_INCLUSOR_INC
/*   (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/  */

/*
 A single point of inclusion of either CRT or STD lib headers
 Reason: to be able to manage what is included and what is not

 Roadmap: do not use std lib

 MOTE: very rarely you will need to include this header. Because many headers already include it.

 NOTE: windows.h is not included through here. Use dbj_windows_include.h
*/

#ifndef DBJ_USES_CRT
#define DBJ_USES_CRT
#endif

#ifdef __clang__
// #pragma clang diagnostic push
#pragma clang system_header
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__

#ifdef _KERNEL_MODE
#define DBJ_KERNEL_BUILD
#endif // _KERNEL_MODE

//#if defined( __cplusplus )  && (! defined (DBJ_USES_STD_LIB) || ! defined (DBJ_KERNEL_BUILD))
//#error "dbj_single_inclusor.h": This is not C code. DBJ_USES_STD_LIB or DBJ_KERNEL_BUILD have to be defined.
//#endif


#if defined( DBJ_KERNEL_BUILD ) || ! defined( __cplusplus ) 
#ifdef DBJ_USES_STD_LIB
#undef DBJ_USES_STD_LIB
#pragma message("This is kernel build or C build. Thus DBJ_USES_STD_LIB was undefined")
#endif // DBJ_USES_STD_LIB
#endif

// this is MSVC incantation we use in any case
// although nobody seems to know what it is 
// exactly for?
#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

//////////////////////////////////////////////////////////////////////////////////////////
#ifdef DBJ_USES_CRT
///CRT + STD lib is allowed 
#include <assert.h> // do not use assert()
#include <crtdbg.h> // use _ASSERTE()
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>
#endif // DBJ_USES_CRT

//////////////////////////////////////////////////////////////////////////////////////////
#ifdef DBJ_USES_STD_LIB
#pragma message("remove the dependancy on the std lib")
#include <chrono>
#include <string_view>
#include <optional>
#include <utility>
#include <mutex>
#include <cassert>
#include <future>
#include <iomanip>
#include <random>
#include <thread>
#include <list>
#include <deque>
#include <iterator>
#include <any>
#include <forward_list>

#include <vector>
#define DBJ_VECTOR std::vector

#include <array>
#define DBJ_ARRAY std::array

#include <string>
#define DBJ_STRING std::string

#endif // DBJ_USES_STD_LIB

//////////////////////////////////////////////////////////////////////////////////////////
// kernel + std lib is not allowed
#ifdef DBJ_KERNEL_BUILD
#endif // DBJ_KERNEL_BUILD

//////////////////////////////////////////////////////////////////////////////////////////
// DBJ TYPES
// are macros helping us to transparently switch away from std lib types
// right now we are trying to use EASTL2010 instrad of std lib core types
// NOTE: do use dbj_eastl_inclusor.h , it contains user functions EASTL2010 needs

#undef DBJ_VECTOR
#include <EASTL/vector.h>
#define DBJ_VECTOR eastl::vector

#undef DBJ_ARRAY
#include "nonstd/dbj++array.h"
#define DBJ_ARRAY ::dbj::nanolib::containers::array

#undef DBJ_ARRAY_WITH_PUSH
#define DBJ_ARRAY_WITH_PUSH ::dbj::nanolib::containers::array_with_push

#undef  DBJ_STRING
#include <EASTL/string.h>
#define DBJ_STRING eastl::string


//------------------------------------------------------------------



#endif // !DBJ_SINGLE_INCLUSOR_INC

