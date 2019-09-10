#pragma once

#ifndef DBJ_NANOLIB_INCLUDED
#define DBJ_NANOLIB_INCLUDED
/*
(c) 2019 by dbj.org

Appache 2.0 licence. Please review the licence file in this project
*/

#include <stdint.h>
#include <stdio.h>
#include <array>
#include <vector>
#include <chrono>
#include <cmath>
#include <string_view>
#include <future>
#include <mutex>

#ifdef _MSVC_LANG
#if _MSVC_LANG < 201402L
#error "C++17 required ..."
#endif
#endif

/*
from vcruntime.h
*/
#define _DBJ_STRINGIZE_(x) #x
#define _DBJ_STRINGIZE(x) _DBJ_STRINGIZE_(x)

#define _DBJ_WIDE_(s) L##s
#define _DBJ_WIDE(s) _DBJ_WIDE_(s)

#define _DBJ_CONCATENATE_(a, b) a##b
#define _DBJ_CONCATENATE(a, b) _DBJ_CONCATENATE_(a, b)

#define _DBJ_EXPAND_(s) s
#define _DBJ_EXPAND(s) _DBJ_EXPAND_(s)

#ifdef _MSC_VER
// https://developercommunity.visualstudio.com/content/problem/195665/-line-cannot-be-used-as-an-argument-for-constexpr.html
#define CONSTEXPR_LINE long(_DBJ_CONCATENATE(__LINE__, U))
#else
#define CONSTEXPR_LINE __LINE__
#endif

/*

this macro is actually superior solution to the repeat template function
_dbj_repeat_counter is local for each macro expansion

DBJ_REPEAT(50){ std::printf("\n%d", _dbj_repeat_counter ); }

*/
#define DBJ_REPEAT(N) for (size_t _dbj_repeat_counter = 0; _dbj_repeat_counter < static_cast<size_t>(N); _dbj_repeat_counter++)

#ifdef _unused
#error _unused is already defined somewhere ...
#else
#define _unused(...) static_assert(noexcept(__VA_ARGS__, true))
#endif

namespace dbj::nanolib
{

using namespace std;

[[noreturn]] inline void dbj_terror(const char *msg_, const char *file_, const int line_)
{
    _ASSERTE(msg_ && file_ && line_);
    std::fprintf(stderr, "\n\ndbj++ Terminating error:%s\n%s (%d)", msg_, file_, line_);
    std::exit(EXIT_FAILURE);
}

#ifndef DBJ_VERIFY
#define DBJ_VERIFY_(x, file, line) \
    if (false == x)                \
    ::dbj::nanolib::dbj_terror(#x ", failed", file, line)
#define DBJ_VERIFY(x) DBJ_VERIFY_(x, __FILE__, __LINE__)
#endif

#pragma region synchronisation

/*
	usage:

	void thread_safe_fun() {
		lock_unlock autolock_ ;
	}

	*/
struct lock_unlock final
{

    mutable std::mutex mux_;

    lock_unlock() noexcept { mux_.lock(); }
    ~lock_unlock() { mux_.unlock(); }
};

#pragma endregion

#pragma region buffer type and helper

constexpr inline std::size_t DBJ_64KB = UINT16_MAX;
/*
for runtime buffering the most comfortable and in the same time fast
solution is vector<char_type>
only unique_ptr<char[]> is faster than vector of  chars, by a margin
UNICODE does not mean 'char' is forbiden. We deliver 'char' based buffering 
only.
Bellow is a helper, with function most frequently used to make buffer aka vector<char>
*/
struct v_buffer final
{

    using buffer_type = std::vector<char>;

    static buffer_type make(size_t count_)
    {
        _ASSERTE(count_ > 0);
        _ASSERTE(DBJ_64KB >= count_);
        buffer_type retval_(count_ + 1, char(0));
        return retval_;
    }

    static buffer_type make(std::basic_string_view<char> sview_)
    {
        _ASSERTE(sview_.size() > 0);
        _ASSERTE(DBJ_64KB >= sview_.size());
        buffer_type retval_(sview_.begin(), sview_.end());
        // zero terminate?
        retval_.push_back(char(0));
        return retval_;
    }

    template <typename... Args, size_t max_arguments = 255>
    static buffer_type
    format(char const *format_, Args... args) noexcept
    {
        static_assert(sizeof...(args) < max_arguments, "\n\nmax 255 arguments allowed\n");
        _ASSERTE(format_);
        // 1: what is the size required
        size_t size = 1 + std::snprintf(nullptr, 0, format_, args...);
        _ASSERTE(size > 0);
        // 2: use it at runtime
        buffer_type buf = make(size);
        //
        size = std::snprintf(buf.get(), size, format_, args...);
        _ASSERTE(size > 0);

        return buf;
    }

}; // v_buffer
#pragma endregion

/* 
strerror_s() is very stronlgy recommended instead of strerror()
this is using it with dbj nanolib buffer type
*/
inline v_buffer::buffer_type safe_strerror(int errno_)
{
    v_buffer::buffer_type buffy_ = v_buffer::make(BUFSIZ);
    if (0 != strerror_s(buffy_.data(), buffy_.size(), errno_))
        dbj_terror("strerror_s failed", __FILE__, __LINE__);
    return buffy_;
}

/*
we use fprintf throgh a macro to increase the resilience + the change-ability
of dbj nano lib

first arg has to be stdout, stderr, etc ...
*/
#define DBJ_FPRINTF(...)                                                                                   \
    do                                                                                                     \
    {                                                                                                      \
        if (errno_t result_ = ::fprintf(__VA_ARGS__); result_ < 0)                                         \
            ::dbj::nanolib::dbj_terror(::dbj::nanolib::safe_strerror(result_).data(), __FILE__, __LINE__); \
    } while (false)

#define DBJ_PRINT(...) DBJ_FPRINTF(stdout, __VA_ARGS__)
/*
we use the macro bellow to create ever needed location info always 
associated with the offending expression
*/
#define DBJ_ERR_PROMPT(x) (__FILE__ "(" _CRT_STRINGIZE(__LINE__) ") " _CRT_STRINGIZE(x))

#define DBJ_CHK(x)    \
    if (false == (x)) \
    DBJ_FPRINTF(stderr, "Evaluated to false! ", DBJ_ERR_PROMPT(x))

/*
	this is for variables only
	example
	long DBJ_MAYBE(var) {42L} ;
	after expansion:
	long var [[maybe_unused]] {42L} ;
	*/
#define DBJ_MAYBE(x) x[[maybe_unused]]

#pragma region very core type traits

/* 
Check at compile time if value (of 'any' type) is inside given boundaries (inclusive)

example usage:

template<unsigned K>
using ascii_ordinal_compile_time = ::dbj::inside_t<unsigned, K, 0, 127>;

constexpr auto compile_time_ascii_index = ascii_ordinal_compile_time<164>() ;

164 above is outide of [0..127), compiler fails:

'std::enable_if_t<false,std::integral_constant<unsigned int,164>>' : Failed to specialize alias template
 constexpr auto compile_time__not_ascii_index = ascii_ordinal_compile_time<164>() ;
*/
template <typename T, T X, T L, T H>
using inside_inclusive_t =
    ::std::enable_if_t<(X <= H) && (X >= L),
                       ::std::integral_constant<T, X>>;

template <typename T, T X, T L, T H>
inline constexpr bool inside_inclusive_v = inside_inclusive_t<T, X, L, H>();

/*
Example usage of bellow:
	
    static_assert(  dbj::is_any_same_as_first_v<float, float, float> ) ;

	fails, none is same as bool:
		static_assert(  dbj::is_any_same_as_first_v<bool,  float, float>  );
	*/
template <class _Ty,
          class... _Types>
inline constexpr bool is_any_same_as_first_v = ::std::disjunction_v<::std::is_same<_Ty, _Types>...>;
#pragma endregion

#pragma region numerics
// compile time extremely precise PI approximation
//
//  https://en.wikipedia.org/wiki/Proof_that_22/7_exceeds_π
// https://www.wired.com/story/a-major-proof-shows-how-to-approximate-numbers-like-pi/
constexpr inline auto DBJ_PI = 104348 / 33215;
#pragma endregion

} // namespace dbj::nanolib

#endif // DBJ_NANOLIB_INCLUDED
