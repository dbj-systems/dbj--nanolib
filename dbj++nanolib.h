#pragma once
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */
#ifndef DBJ_NANOLIB_INCLUDED
#define DBJ_NANOLIB_INCLUDED

#ifdef _MSVC_LANG
#define DBJ_NANO_WIN32
#endif

#include <stdint.h>
#include <stdio.h>
#include <array>
#include <vector>
#include <chrono>
#include <cmath>
#include <string_view>
#include <optional>
#include <mutex>

#ifdef DBJ_NANO_WIN32
#include <io.h>
#include <fcntl.h>
#define NOMINMAX
#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <system_error>
#include "vt100win10.h"
#endif

#ifdef _MSVC_LANG
#if _MSVC_LANG < 201402L
#error "C++17 required ..."
#endif
#endif

#ifdef DBJ_ASSERT
#error remove previous DBJ_ASSERT definition
#endif

#ifdef ASSERTE_
#define DBJ_ASSERT ASSERTE_
#else
#include <cassert>
#define DBJ_ASSERT assert
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
constexpr auto RELEASE = "2.0.0";

using namespace std;

#ifdef DBJ_NANO_WIN32
void enable_vt_100();
#endif

/* do this only once and do this as soon as possible */
inline const bool dbj_nanolib_initialized = ([]() -> bool {
#ifdef DBJ_NANO_WIN32
/*
			WIN32 console is one notorious 30+ years old forever teenager
			WIN32 UNICODE situation does not help at all
            UCRT team started well then dispersed
			https://www.goland.org/unicode_c_windows/

			To stay sane and healthy, the rules are:

			0. stick to UTF8 as much as you can -- article above is good but sadly wrong about UTF16, see www.utf8.com
			1. NEVER mix printf and wprintf
			1.1 you can mix printf and std::cout but very carefully
			1.2 UCRT and printf and _setmode() are not friends see the commenct bellow, just here
			2. NEVER mix std::cout  and std::wcout
			3. be (very_ aware that you need particular font to see *all* of your funky unicode glyphs is windows console
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
#endif
			enable_vt_100(); // enable VT100 ESC code for WIN10 console

#endif // DBJ_NANO_WIN32
			/* this might(!) slow down the ostreams but is much safer */
			ios_base::sync_with_stdio(true);
			/*-----------------------------------------------------------------------------------------*/
			return true; }());
/*
	terror == terminating error
	NOTE: std::exit *is* different to C API exit()
	*/
[[noreturn]] inline void dbj_terror(const char *msg_, const char *file_, const int line_)
{
    DBJ_ASSERT(msg_ && file_ && line_);
    std::fprintf(stderr, "\n\ndbj++ Terminating error:%s\n%s (%d)", msg_, file_, line_);
    std::exit(EXIT_FAILURE);
}

// CAUTION! DBJ_VERIFY works in release builds too
#ifndef DBJ_VERIFY
#define DBJ_VERIFY_(x, file, line) \
    if (false == x)                \
    ::dbj::nanolib::dbj_terror("Expression: " #x ", failed ", file, line)

#define DBJ_VERIFY(x) DBJ_VERIFY_(x, __FILE__, __LINE__)
#endif

#pragma region synchronisation
/* 
usage:	void thread_safe_fun() {		lock_unlock autolock_ ;  	} 
*/
struct lock_unlock final
{
    mutable std::mutex mux_;
    lock_unlock() noexcept { mux_.lock(); }
    ~lock_unlock() { mux_.unlock(); }
};

#pragma endregion

#pragma region buffer type and helper

/*
in case you need more change this
by default it is 64KB aka 65535 bytes, which is quite a lot perhaps?
*/
constexpr inline std::size_t DBJ_MAX_BUFER_SIZE = UINT16_MAX;
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
        DBJ_ASSERT(count_ > 0);
        DBJ_ASSERT(DBJ_MAX_BUFER_SIZE >= count_);
        buffer_type retval_(count_ + 1, char(0));
        return retval_;
    }

    static buffer_type make(std::basic_string_view<char> sview_)
    {
        DBJ_ASSERT(sview_.size() > 0);
        DBJ_ASSERT(DBJ_MAX_BUFER_SIZE >= sview_.size());
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
        DBJ_ASSERT(format_);
        // 1: what is the size required
        size_t size = 1 + std::snprintf(nullptr, 0, format_, args...);
        DBJ_ASSERT(size > 0);
        // 2: use it at runtime
        buffer_type buf = make(size);
        //
        size = std::snprintf(buf.data(), size, format_, args...);
        DBJ_ASSERT(size > 0);

        return buf;
    }

#ifdef DBJ_NANO_WIN32
    /*
		CP_ACP == ANSI
		CP_UTF8
		*/
    template <auto CODE_PAGE_T_P_ = CP_UTF8>
    static std::vector<wchar_t> n2w(string_view s)
    {
        const int slength = (int)s.size() + 1;
        int len = MultiByteToWideChar(CODE_PAGE_T_P_, 0, s.data(), slength, 0, 0);
        std::vector<wchar_t> rez(len, L'\0');
        MultiByteToWideChar(CODE_PAGE_T_P_, 0, s.data(), slength, rez.data(), len);
        return rez;
    }

    template <auto CODE_PAGE_T_P_ = CP_UTF8>
    static buffer_type w2n(wstring_view s)
    {
        const int slength = (int)s.size() + 1;
        int len = WideCharToMultiByte(CODE_PAGE_T_P_, 0, s.data(), slength, 0, 0, 0, 0);
        buffer_type rez(len, '\0');
        WideCharToMultiByte(CODE_PAGE_T_P_, 0, s.data(), slength, rez.data(), len, 0, 0);
        return rez;
    }
#endif // DBJ_NANO_WIN32
};     // v_buffer
#pragma endregion

/*
DO NOT USE "naked" printf() family !
UCRT is still in a mess about CONSOLE output

We use (x)fprintf through a macro to increase the resilience + the change-ability
of dbj nano lib

first arg has to be stdout, stderr, etc ...
*/
#ifdef NDEBUG
#define DBJ_FPRINTF(...) std::fprintf(__VA_ARGS__)
#else
#define DBJ_FPRINTF(...)                                                                                   \
    do                                                                                                     \
    {                                                                                                      \
        if (errno_t result_ = std::fprintf(__VA_ARGS__); result_ < 0)                                      \
            ::dbj::nanolib::dbj_terror(::dbj::nanolib::safe_strerror(result_).data(), __FILE__, __LINE__); \
    } while (false)

#endif

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

#ifdef DBJ_NANO_WIN32

namespace dbj::nanolib
{
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

/* Last WIN32 error, message */
inline v_buffer::buffer_type last_win32_error_message(int code = 0)
{
    std::error_code ec(
        (code ? code : ::GetLastError()),
        std::system_category());
    ::SetLastError(0); //yes this helps
    return v_buffer::format("%s", ec.message().c_str());
}

/* like perror but for WIN32 */
inline void last_perror(char const *prompt = nullptr)
{
    std::error_code ec(::GetLastError(), std::system_category());
    DBJ_FPRINTF(stderr, "\n\n%s\nLast WIN32 Error message: %s\n\n", (prompt ? prompt : ""), ec.message().c_str());
    ::SetLastError(0);
}

inline bool set_console_font(wstring_view font_name, SHORT font_height_ = SHORT(0))
{
    CONSOLE_FONT_INFOEX font_info{};
    font_info.cbSize = sizeof(CONSOLE_FONT_INFOEX);

    HANDLE con_out_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (con_out_handle == INVALID_HANDLE_VALUE)
        return false;

    BOOL rez_ = GetCurrentConsoleFontEx(
        con_out_handle,
        TRUE,
        &font_info);

    if (rez_ == 0)
    {
        dbj::nanolib::last_perror("GetCurrentConsoleFontEx() failed with message: ");
        return false;
    }

    // set the new font name
    (void)memset(font_info.FaceName, 0, LF_FACESIZE);
    std::copy(font_name.begin(), font_name.end(), std::begin(font_info.FaceName));

    // if reuested set the new font size
    if (font_height_ > 0)
    {
        // quietly discard the silly sizes
        if ((font_height_ > 7) && (font_height_ < 145))
        {
            font_info.dwFontSize.Y = font_height_;
        }
    }

    rez_ = SetCurrentConsoleFontEx(
        con_out_handle,
        TRUE, /* for the max window size */
        &font_info);

    if (rez_ == 0)
    {
        dbj::nanolib::last_perror("SetCurrentConsoleFontEx() failed with message: ");
        return false;
    }
    return true;
}

/*
	current machine may or may not  be on WIN10 where VT100 ESC codes are on by default
	they are or have been off by default

	Reuired WIN10 build number is 10586 or greater

	to dance with exact win version please proceed here:
	https://docs.microsoft.com/en-us/windows/win32/sysinfo/verifying-the-system-version
	*/

#ifdef _WIN32_WINNT_WIN10

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#error ENABLE_VIRTUAL_TERMINAL_PROCESSING not found? Try retargeting to the latest SDK.
#endif

/*
will not exit the app *only* if app is started in WIN32 CONSOLE
Example: if running from git bash on win this will exit the app
if app output is redirected to file, this will also fail.
*/
inline void enable_vt_100()
{
    static bool visited{false};
    if (visited)
        return;

    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        last_perror();
        fprintf(stderr, "\nFile: %s\nLine: %ul\nWhy: %s\n", __FILE__, __LINE__, "GetStdHandle() failed");
        return;
    }

    DWORD dwMode{};
    if (!GetConsoleMode(hOut, &dwMode))
    {
        last_perror();
        fprintf(stderr, "\nFile: %s\nLine: %ul\nWhy: %s\n", __FILE__, __LINE__, "GetConsoleMode() failed");
        fprintf(stderr, "\nPlease rerurn in either WIN console or powershell console\n");
        return;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        last_perror();
        fprintf(stderr, "\nFile: %s\nLine: %ul\nWhy: %s\n", __FILE__, __LINE__, "SetConsoleMode() failed");
        return;
    }
    visited = true;
}

#endif _WIN32_WINNT_WIN10

} // namespace dbj::nanolib
#endif DBJ_NANO_WIN32

#endif // DBJ_NANOLIB_INCLUDED
