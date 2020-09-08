#ifndef DBJ_NANOLIB_INCLUDED
#define DBJ_NANOLIB_INCLUDED
/*
   (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/
*/

#ifndef __cplusplus
#error DBJ NANOLIB requires C++ compiler
#endif

// including the inclusor :)
#define DBJ_USES_STD_LIB
#include "dbj_single_inclusor.h"

#if defined(__clang__) 
#define DBJ_PURE_FUNCTION __attribute__((const))
#else
#define DBJ_PURE_FUNCTION 
#endif

/// -------------------------------------------------------------------------------
/// https://stackoverflow.com/a/29253284/10870835

#if (! defined (_DEBUG)) &&  (! defined (NDEBUG))
#error  NDEBUG *is* standard macro and has to exist.
#endif

#undef DBJ_RELEASE_BUILD
#ifdef NDEBUG 
#define DBJ_RELEASE_BUILD
#endif

/// -------------------------------------------------------------------------------
#undef  DBJ_PERROR 
#ifndef NDEBUG
#define DBJ_PERROR (perror(__FILE__ " # " _CRT_STRINGIZE(__LINE__))) 
#else
#define DBJ_PERROR
#endif // NDEBUG

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
#define DBJ_CONSTEXPR_LINE long(_DBJ_CONCATENATE(__LINE__, U))
#else
#define DBJ_CONSTEXPR_LINE __LINE__
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
#define DBJ_NONPAGESECTION __declspec(code_seg("$dbj_nanolib_kerneltext$"))
#else
#define DBJ_NONPAGESECTION
#endif // _KERNEL_MODE
/*
usage:

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

extern "C" {
// https://godbolt.org/z/eP7Txf
#undef  dbj_assert_static
#define dbj_assert_static(e) (void)(1/(e))
} // "C"

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
		minor = 9,
		patch = 0
	};
	// SEMVER + TIMESTAMP
	constexpr auto VERSION = "3.9.0 " __TIME__ " " __DATE__;

	_unused(VERSION);

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

#endif // DBJ_NANOLIB_INCLUDED
