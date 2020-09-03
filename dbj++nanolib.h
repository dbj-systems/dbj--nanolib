#ifndef DBJ_NANOLIB_INCLUDED
#define DBJ_NANOLIB_INCLUDED
/*
   (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/
*/

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

/// -------------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>
/// -------------------------------------------------------------------------------
#include <array>
#include <chrono>
#include <string_view>
#include <optional>
#include <utility>
#include <mutex>

#undef  DBJ_PERROR 
#ifdef _DEBUG
#define DBJ_PERROR (perror(__FILE__ " # " _CRT_STRINGIZE(__LINE__))) 
#else
#define DBJ_PERROR
#endif // _DEBUG

#undef DBJ_FERROR
#ifdef _DEBUG
#define DBJ_FERROR( FP_) \
do { \
if (ferror(FP_) != 0) {\
	DBJ_PERROR ;\
	clearerr_s(FP_);\
} \
} while(0)
#else
#define DBJ_FERROR( FP_ )
#endif // _DEBUG

#ifndef __cplusplus
#error DBJ NANOLIB requires C++ compiler
#endif

#if defined(__clang__) 
#define DBJ_PURE_FUNCTION __attribute__((const))
#else
#define DBJ_PURE_FUNCTION 
#endif

/// -------------------------------------------------------------------------------
/// NDEBUG *is* standard macro and it is used
/// https://stackoverflow.com/a/29253284/10870835
#ifndef NDEBUG
#if defined (DEBUG) || defined(_DEBUG)
/* do nothing */
#else
#define NDEBUG
#endif
#endif // NDEBUG

/// -------------------------------------------------------------------------------
/// stolen from vcruntime.h 
#define _DBJ_STRINGIZE_(x) #x
#define _DBJ_STRINGIZE(x) _DBJ_STRINGIZE_(x)

#define _DBJ_WIDE_(s) L##s
#define _DBJ_WIDE(s) _DBJ_WIDE_(s)

#define _DBJ_CONCATENATE_(a, b) a##b
#define _DBJ_CONCATENATE(a, b) _DBJ_CONCATENATE_(a, b)

#define _DBJ_EXPAND_(s) s
#define _DBJ_EXPAND(s) _DBJ_EXPAND_(s)

#ifdef _MSVC_LANG
// https://developercommunity.visualstudio.com/content/problem/195665/-line-cannot-be-used-as-an-argument-for-constexpr.html
#define CONSTEXPR_LINE long(_DBJ_CONCATENATE(__LINE__, U))
#else
#define CONSTEXPR_LINE __LINE__
#endif


///-----------------------------------------------------------------------------------------
/// this is stolen from the MSVC STD LIB code
/// it actually does not depend on C++20 __cplusplus
/// which is yet undefined as of 2020 Q1

#if !defined(DBJ_HAS_CXX17) && !defined(DBJ_HAS_CXX20)

#undef DBJ_STL_LANG

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

#endif // !defined(DBJ_HAS_CXX17) && !defined(DBJ_HAS_CXX20)

/// usage is without ifndef/ifdef
#if  ! DBJ_HAS_CXX17
#error DBJ NANOLIB requires the standard C++17 (or better) compiler
#endif
#if DBJ_HAS_CXX20
#pragma message( "WARNING -- DBJ NANOLIBis not fully ready yet for the standard C++20 (or higher) -- " __TIMESTAMP__ )
#endif

///-----------------------------------------------------------------------------------------
#ifdef _KERNEL_MODE
#define NONPAGESECTION __declspec(code_seg("$dbj_nanolib_kerneltext$"))
#else
#define NONPAGESECTION
#endif // _KERNEL_MODE
/*
class NONPAGESECTION MyNonPagedClass
{
	...
};
*/

//-----------------------------------------------------------------------------------------

#include "dbj_debug.h" // DBJ_PRINT and friends

//-----------------------------------------------------------------------------------------
// new failure will provoke fast exit -- ALWAYS!
// this is APP WIDE for all users of dbj nanolib
// Policy is to serve "noexcept people" with standard C++ core language
// that means: no std lib
// that is a immediate milestone on the roadmap as of 2020 Q3
#define DBJ_TERMINATE_ON_BAD_ALLOC 1

#if DBJ_TERMINATE_ON_BAD_ALLOC
// do not throw bad_alloc
// call default termination on heap memory exhausted
// NOTE: this is not declaration but immediate execution 
// of anonymous lambda
inline auto setting_new_handler_to_terminate_ = []() {
	std::set_new_handler(
		[] { perror(__FILE__ " Terminating because of heap exhaustion");   std::terminate(); }
	);
	return true;
}();
// #pragma message( "WARNING -- DBJ NANOLIB has set std::new_handler() to immediately terminate. No std::bad_alloc!" )
#else
// #pragma message( "WARNING -- DBJ NANOLIB has *not* set std::new_handler()" )
#endif

///-----------------------------------------------------------------------------------------
#ifdef __clang__
#pragma clang system_header
#endif

extern "C" {
// https://godbolt.org/z/eP7Txf
#undef  dbj_assert_static
#define dbj_assert_static(e) (void)(1/(e))
} // "C"
/// --------------------------------------------------------
/// decide which vector you will use
/// keep it behind macro DBJ_VECTOR in any case
#define _DBJ_USING_STD_VECTOR 0

#if _DBJ_USING_STD_VECTOR
#include <vector>
#define DBJ_VECTOR std::vector
#else

#include "nonstd/dbj++vector.h"

#if !DBJ_TERMINATE_ON_BAD_ALLOC
#pragma message("\n\nWARNING!\n\nUsing non standard vector with bad_alloc throwing enabled.\n\n")
#endif

#endif

#include "dbj_buffer.h"

/// --------------------------------------------------------
// there are various compile time tests sprinkled arround this library
// they will be all executed in debug builds
// but why?
// because we like to keep the tests together with things declared/defined and
// then immediately tested in the same file
//
#ifndef NDEBUG
#define DBJ_NANOLIB_QUICK_COMPILE_TIME_TESTING
#endif

/// -------------------------------------------------------------------------------
/// there is no `repeat` in C++
/*
this macro is actually superior solution to the repeat template function
dbj_repeat_counter_ is local for each macro expansion
usage:
	  DBJ_REPEAT(50){ std::printf("\n%d", dbj_repeat_counter_ ); }
*/
#define DBJ_REPEAT(N) for (size_t dbj_repeat_counter_ = 0; dbj_repeat_counter_ < static_cast<size_t>(N); ++dbj_repeat_counter_)

/// -------------------------------------------------------------------------------
#ifdef _unused
#error dbj nanolib _unused is already defined somewhere ...?
#else
#define _unused(...) static_assert(noexcept(__VA_ARGS__, true))
#endif

/// -------------------------------------------------------------------------------
namespace dbj::nanolib
{
	enum class SEMVER
	{
		major = 3,
		minor = 7,
		patch = 0
	};
	// SEMVER + TIMESTAMP
	constexpr auto VERSION = "3.7.0 " __TIME__ " " __DATE__;

	/// -------------------------------------------------------------------------------
	/* this can speed up things considerably. but test comprehensively first! */
	inline void assume(bool cond)
	{
#if defined(__clang__) // Must go first -- clang also defines __GNUC__
		__builtin_assume(cond);
#elif defined(__GNUC__)
		if (!cond)
		{
			__builtin_unreachable();
		}
#elif defined(_MSC_VER)
		__assume(cond);
#else
		// Do nothing.
#endif
	}

	/// -------------------------------------------------------------------------------
	// nanolib loggin is deprecated

	//namespace logging {
	//	void enable_vt_100_and_unicode();
	//}

	/// -------------------------------------------------------------------------------
	/* happens once and as soon as possible */
	inline const bool dbj_nanolib_initialized = ([]() -> bool {

		/*
				WIN32 console is one notorious 30+ years old brat
				WIN32 UNICODE situation does not help at all
				MSFT UCRT team started well then dispersed

				https://www.goland.org/unicode_c_windows/

				To stay sane and healthy, the rules are:

				0. If you need unicode glyphs stick to UTF8 as much as you can 
				   -- article above is good but sadly wrong about UTF16, see www.utf8.com
				1. NEVER mix printf and wprintf
				   1.1 you can mix printf and std::cout but very carefully
				   1.2 UCRT and printf and _setmode() are not friends see the commenct bellow, just here
				2. NEVER mix std::cout  and std::wcout
				3. be (very) aware that you need particular font to see *all* of your funky unicode glyphs is windows console
				4. never (ever) use C++20 char8_t and anything using it
				   4.1 if you need to use <cuchar> for utf translations

				*/
#if 0
		/*
		Steve Wishnousky (MSFT) publicly has advised me personaly, against
		using _setmode(), at all
		https://developercommunity.visualstudio.com/solutions/411680/view.html
		*/
				//#define _O_TEXT        0x4000  // file mode is text (translated)
				//#define _O_BINARY      0x8000  // file mode is binary (untranslated)
				//#define _O_WTEXT       0x10000 // file mode is UTF16 (translated)
				//#define _O_U16TEXT     0x20000 // file mode is UTF16 no BOM (translated)
				//#define _O_U8TEXT      0x40000 // file mode is UTF8  no BOM (translated)

		if (-1 == _setmode(_fileno(stdin), _O_U8TEXT)) perror("Can not set mode");
		if (-1 == _setmode(_fileno(stdout), _O_U8TEXT)) perror("Can not set mode");
		if (-1 == _setmode(_fileno(stderr), _O_U8TEXT)) perror("Can not set mode");

		// with _O_TEXT simply no output
		// works with _O_WTEXT, _O_U16TEXT and _O_U8TEXT
		wprintf(L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");

		// "kicks the bucket" with _O_WTEXT, _O_U16TEXT and _O_U8TEXT
		// works with _O_TEXT and u8

		// THIS IS THE ONLY WAY TO USE CHAR AND UTF8 AND HAVE THE UCRT CONSOLE UNICODE OUTPUT
		printf(u8"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");

		// also see the /utf-8 compiler command line option
		// https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=vs-2019&viewFallbackFrom=vs-2017)

		// error C2022:  '1082': too big for character and so on  for every character
		// printf(  "\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");
#endif // 0

// currently (2019Q4) WIN10 CONSOLE "appears" to need manual enabling the ability to
// interpret VT100 ESC codes
// nanolib loggin is deprecated
// logging::enable_vt_100_and_unicode(); // enable VT100 ESC code for WIN10 console

#ifdef DBJ_SYNC_WITH_STDIO
		/*
		We use iostream but only and strictly for dbj++tu testing fwork
		this might(!) slow down the ostreams
		but renders much safer interop with stdio.h
		*/
		ios_base::sync_with_stdio(true);
#endif
		/*-----------------------------------------------------------------------------------------
		immediately call the nano-lib initialization function, and ... do it only once
		*/
		return true; }());

	///	-----------------------------------------------------------------------------------------
	using void_void_function_ptr = void (*)(void);
	// yes I am aware of: https://ricab.github.io/scope_guard/
	// but I do not see the point of that complexity ;)
	template <typename Function_PTR = dbj::nanolib::void_void_function_ptr>
	struct on_scope_exit final
	{
		static Function_PTR null_call() {}
		// much faster + cleaner vs giving nullptr
		// no if in destructor required
		const Function_PTR callable_{ null_call };

		explicit on_scope_exit(Function_PTR fun_) noexcept : callable_(fun_) {}

		~on_scope_exit()
		{
			// no if in destructor required
			callable_();
		}
	}; // eof on_scope_exit
	///	-----------------------------------------------------------------------------------------



///	-----------------------------------------------------------------------------------------
/*
this is for variables only
example
long DBJ_MAYBE(var) {42L} ;
after expansion:
long var [[maybe_unused]] {42L} ;
*/
#define DBJ_MAYBE(x) x[[maybe_unused]]

///	-----------------------------------------------------------------------------------------
#pragma region numerics
// compile time extremely precise PI approximation
//
//  https://en.wikipedia.org/wiki/Proof_that_22/7_exceeds_π
// https://www.wired.com/story/a-major-proof-shows-how-to-approximate-numbers-like-pi/
	constexpr inline auto DBJ_PI = 104348 / 33215;
#pragma endregion

	///	-----------------------------------------------------------------------------------------
	/*
	strerror_s() is very strongly recommended instead of strerror()
	this is using it with dbj nanolib buffer type

	Also note: https://linux.die.net/man/3/strerror_r
	__GNUC__ version is de-facto standard for non-windows situations
	*/
	inline v_buffer::buffer_type safe_strerror(int errno_)
	{
#ifdef _MSC_VER
		v_buffer::buffer_type buffy_ = v_buffer::make(BUFSIZ);
		if (0 != strerror_s(buffy_.data(), buffy_.size(), errno_))
			dbj::terror("strerror_s failed", __FILE__, __LINE__);
		return buffy_;
#elif __GNUC__
		// TODO: untested!
		v_buffer::buffer_type buffy_ = v_buffer::make(BUFSIZ);
		(void)strerror_r(errno_, buffy_.data(), buffy_.size()))
		dbj::terror("strerror_r failed", __FILE__, __LINE__);
		return buffy_;
#else
		// TODO: untested!
		v_buffer::buffer_type buffy_ = v_buffer::make(strerror(errno_));
		return buffy_;
#endif
	}

} // namespace dbj::nanolib

// can be used on its own
#include "dbj_heap_alloc.h"
// #include "vt100win10.h"
// #include "dbj++debug.h"

/// internal (but not private) critical section
#include "dbj_nano_synchro.h"

// why do we need this here?
// #include "nonstd/nano_printf.h"

// deprecated --> #include "dbj++log.h"

#include "dbj_typename.h" // DBJ_SX and DBJ_SXT

#endif // DBJ_NANOLIB_INCLUDED
