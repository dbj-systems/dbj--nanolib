
#pragma once
/*
this is bottom line everything uses it; 'thing'
thus no dependancies beside crt
*/

#ifdef __clang__
#ifdef NDEBUG
#pragma clang system_header
#endif
#endif

#include <crtdbg.h>
#include <stdio.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------
#undef DBJ_ASSERT
#define DBJ_ASSERT _ASSERTE

/*
we use the macros bellow to create ever needed location info always
associated with the offending expression
timestamp included
*/
#define DBJ_FILE_LINE __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")"
#define DBJ_FILE_LINE_TSTAMP __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")[" __TIMESTAMP__ "] "
#define DBJ_FLT_PROMPT(x) DBJ_FILE_LINE_TSTAMP _CRT_STRINGIZE(x)

/* will not compile if MSG_ is not string literal */
#define DBJ_ERR_PROMPT(MSG_) DBJ_FILE_LINE_TSTAMP MSG_

// -----------------------------------------------------------------------------
#undef  DBJ_PRINT

namespace dbj {

	/*
terror == terminating error
NOTE: std::exit *is* different to C API exit()
NOTE: all the bets are of so no point of using some logging
*/
	[[noreturn]] inline void terror(const char* msg_, const char* file_, const int line_ )
	{
		/// DBJ_ASSERT(msg_ && file_ && line_);
		/// all the bets are of so no point of using some logging
		perror("\n\n" DBJ_ERR_PROMPT("\n\ndbj nanolib Terminating error!") );
		::exit(EXIT_FAILURE);
	}

	namespace  debug
	{
#ifndef _DEBUG
		constexpr bool release_mode_build = (1 == 1);
#else
		constexpr bool release_mode_build = (1 == 0);
#endif

#if 0
		template <bool = false>
		struct release_mode : std::false_type {};

		template <>
		struct release_mode< true > : std::true_type {};

		constexpr inline bool release_mode_v = release_mode<release_mode_build>::value;
#endif // 0
		template < typename ... A >
		inline void print(const char* format_string, A ... args_) noexcept
		{
			if constexpr (!release_mode_build)
			{
				::fprintf(stderr, format_string, args_ ...);
			}
		}
	} // debug ns
} // dbj ns

#ifndef _DEBUG
#define DBJ_PRINT(...) dbj::debug::print(__VA_ARGS__)
#else
#define DBJ_PRINT(...) 
#endif

#define DBJ_CHK(x)    \
	if (false == (x)) \
	DBJ_PRINT("Evaluated to false! ", DBJ_FLT_PROMPT(x))

	///	-----------------------------------------------------------------------------------------
	// CAUTION! DBJ_VERIFY works in release builds too
#undef DBJ_VERIFY
#undef DBJ_VERIFY_

#define DBJ_VERIFY_(x, file, line) \
	if (false == x)                \
	::dbj::terror("Expression: " #x ", failed ", file, line)

#define DBJ_VERIFY(x) DBJ_VERIFY_(x, __FILE__, __LINE__)

