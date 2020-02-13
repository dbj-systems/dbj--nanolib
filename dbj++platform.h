#pragma once
#ifndef DBJ_PLATFORM_INC	
#define DBJ_PLATFORM_INC	
/*
(c) 2020 by dbj@dbj.org, licence CC BY SA 4.0
--------------------------------------------------------------------------------------------------
 Platform  = OS + Compiler + Version of C++

 we use constexpr globals as much as possible, with names in ALL_CAPITALS

 In here we try to sniff them out and to do that with the least ammount of macros as possible.

 dbj++ codes base is C++17 or better. thus C++17 features and mechanisms
 are used for comfortable API experience
 */
 /*
 ------------------------------------------------------------------------------------------------
 CHECK FOR C++ VERSION
 ------------------------------------------------------------------------------------------------
 Note: these should be intrinsic macros
 */

 // this is MSVC STD LIB code
 // it actually does not depend on C++20 __cplusplus
 // which is yet undefined as of 2020 Q1

#if !defined(DBJ_HAS_CXX17) && !defined(DBJ_HAS_CXX20)

#if defined(_MSVC_LANG)
#define DBJ_STL_LANG _MSVC_LANG
#else
#define DBJ_STL_LANG __cplusplus
#endif

#if DBJ_STL_LANG > 201703L
#define DBJ_HAS_CXX17 1
#define DBJ_HAS_CXX20 1
#elif DBJ_STL_LANG > 201402L
#define DBJ_HAS_CXX17 1
#define DBJ_HAS_CXX20 0
#else // DBJ_STL_LANG <= 201402L
#define DBJ_HAS_CXX17 0
#define DBJ_HAS_CXX20 0
#endif // Use the value of DBJ_STL_LANG to define DBJ_HAS_CXX17 and \
       // DBJ_HAS_CXX20

// #undef DBJ_STL_LANG
#endif // !defined(DBJ_HAS_CXX17) && !defined(DBJ_HAS_CXX20)

 /*
 We include climits so that non intrinsic macros are avaiable too.
 For C++20 that should be the role of the <version> header
 http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0754r2.pdf

*/

#include <climits>

#if defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
#include <sys/param.h>
#endif

#if defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
#include <TargetConditionals.h>
#endif

namespace dbj::nanolib {
	/*
	--------------------------------------------------------------------------------------------------
	THE PLATFORM

	what C++ compiler or C compiler and what platform are we on.
	--------------------------------------------------------------------------------------------------
	https://stackoverflow.com/a/42040445
	Obviously users are free to add a platform if missing. Example:

		namespace pm = dbj::nanolib::platform ;
        
		if constexpr ( pm::CODE == pm::WINDOWS_32 ) {
					...
		} else {
					...
		}

	*/
	namespace platform {

		/*
		might be a double edged sword. non class enum  but adorned with namespace
		for "cleverness"
		*/
		enum platform_code {
			WINDOWS_32 = 0, WINDOWS_64, CYGWIN, ANDROID, LINUX, BSD, HP_UX, AIX, IOS, OSX, SOLARIS, UNKNOWN
		};

#if defined(_WIN32)
		constexpr inline auto NAME = "windows"; // Windows
		constexpr inline auto CODE = WINDOWS_32;
#elif defined(_WIN64)
		constexpr inline auto NAME = "windows"; // Windows
		constexpr inline auto CODE = WINDOWS_64;
#elif defined(__CYGWIN__) && !defined(_WIN32)
		constexpr inline auto NAME = "windows"; // Windows (Cygwin POSIX under Microsoft Window)
		constexpr inline auto CODE = CYGWIN;
#elif defined(__ANDROID__)
		constexpr inline auto NAME = "android"; // Android (implies Linux, so it must come first)
		constexpr inline auto CODE = ANDROID;
#elif defined(__linux__)
		constexpr inline auto NAME = "linux"; // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
		constexpr inline auto CODE = LINUX;
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
#if defined(BSD)
		constexpr inline auto NAME = "bsd"; // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
		constexpr inline auto CODE = BSD;
#endif
#elif defined(__hpux)
		constexpr inline auto NAME = "hp-ux"; // HP-UX
		constexpr inline auto CODE = HP_UX;
#elif defined(_AIX)
		constexpr inline auto NAME = "aix"; // IBM AIX
		constexpr inline auto CODE = AIX;
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
#if TARGET_IPHONE_SIMULATOR == 1
		constexpr inline auto NAME = "ios"; // Apple iOS
		constexpr inline auto CODE = IOS;
#elif TARGET_OS_IPHONE == 1
		constexpr inline auto NAME = "ios"; // Apple iOS
		constexpr inline auto CODE = IOS;
#elif TARGET_OS_MAC == 1
		constexpr inline auto NAME = "osx"; // Apple OSX
		constexpr inline auto CODE = OSX;
#endif
#elif defined(__sun) && defined(__SVR4)
		constexpr inline auto NAME = "solaris"; // Oracle Solaris, Open Indiana
		constexpr inline auto CODE = SOLARIS;
#else
		constexpr inline auto NAME = "unknown"; // 
		constexpr inline auto CODE = UNKNOWN;
#endif

		namespace detail {
			// this is compile time literal
			struct name_code final {
				constexpr static auto name{ NAME };
				constexpr static auto code{ CODE };
			};
		}

		constexpr auto platform_name_code() {
			return detail::name_code{};
		}

	} // namespace platform 


	namespace compiler {
		// add more to your liking
		enum compiler_code { MSVC, CLANG, GNUC, GNUG, UNSUPPORTED };

#if defined(__clang__)
		constexpr auto MAJOR = __clang__;
		constexpr auto CODE = CLANG;
		constexpr auto NAME = "clang";
#elif defined(__GNUC__)
		constexpr auto MAJOR = __GNUC__;
		constexpr auto CODE = GNUC;
		constexpr auto NAME = "gnuc";
		/* The GNU C++ compiler defines __GNUG__. Testing it is equivalent to testing (__GNUC__ && __cplusplus). */
#elif  defined(__GNUG__)
		constexpr auto MAJOR = __GNUC__;
		constexpr auto CODE = GNUG;
		constexpr auto NAME = "g++";
#elif defined(_MSC_VER)
		constexpr auto MAJOR = _MSC_VER;
		constexpr auto CODE = MSVC;
		constexpr auto NAME = "msvc";
#else
		constexpr auto MAJOR = 0;
		constexpr auto CODE = UNSUPPORTED;
		constexpr auto NAME = "unsupported";
#endif

	} // namespace compiler 

	namespace language {

		enum language_code { CPP17, CPP20, C90, C99, C11, C17, C18, C2x, UNKNOWN };

#ifdef DBJ_STL_LANG

#ifdef DBJ_HAS_CXX17
		constexpr auto CODE = CPP17;
		constexpr auto NAME = "c++17";
#elif   DBJ_HAS_CXX20
		constexpr auto CODE = CPP20;
		constexpr auto NAME = "c++20";
#else
		constexpr auto CODE = UNKNOWN; // irrelevant since not 17 or 20, 23 soon
		constexpr auto NAME = "unknown";
#endif

#else // not C++

#ifdef __STDC_VERSION__

#if  __STDC_VERSION__ < 199901L
		constexpr auto CODE = C90; /* or bellow */
		constexpr auto NAME = "c90";
#elif __STDC_VERSION__ ==  199901L
		constexpr auto CODE = C99; 
		constexpr auto NAME = "c99";
#elif __STDC_VERSION__ ==  201112L:
		constexpr auto CODE = C11; 
		constexpr auto NAME = "c11";
#elif __STDC_VERSION__ ==  201710L :
		constexpr auto CODE = C17;  // same code as C18, GNUC only
		constexpr auto NAME = "c17";
#elif __STDC_VERSION__ ==  202000L :
		constexpr auto CODE = C2x;  // GNUC only
		constexpr auto NAME = "c2x";
#else // not __STDC_VERSION__ 
		/* MSFT refuses to declare which C version is CL conforming to */
		constexpr auto CODE = UNKNOWN; /* or bellow */
		constexpr auto NAME = "unknown";
#endif // not __STDC_VERSION__

#endif // __STDC_VERSION__

#endif // __cplusplus

	} // namespace language

} // namespace dbj::nanolib 

#endif // DBJ_PLATFORM_INC	
