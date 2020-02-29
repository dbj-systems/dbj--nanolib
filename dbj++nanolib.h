/* (c) 2019, 2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */
#ifndef DBJ_NANOLIB_INCLUDED
#define DBJ_NANOLIB_INCLUDED

#ifndef __cplusplus
#error DBJ NANOLIB requires C++ compiler
#endif

/// -------------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <array>
#include <chrono>
#include <cmath>
#include <string_view>
#include <optional>
#include <utility>
#include <mutex>

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

/// usage is without ifndef/ifdef
#if  ! DBJ_HAS_CXX17
#error DBJ NANOLIB requires the standard C++17 compiler
#endif
#if DBJ_HAS_CXX20
#pragma message( "WARNING -- DBJ NANOLIBis not ready yet for the standard C++20 (or higher) -- " __TIMESTAMP__ )
#endif

///-----------------------------------------------------------------------------------------
#ifdef _MSVC_LANG
#define DBJ_NANO_WIN32
#endif

///-----------------------------------------------------------------------------------------
/// https://stackoverflow.com/a/29253284/10870835
#ifndef NDEBUG
#if defined (DEBUG) || defined(_DEBUG)
#define NDEBUG
#else
/* do nothing */
#endif
#endif // NDEBUG

///-----------------------------------------------------------------------------------------
/// no. this is not "nano" --> #include "dbj++platform.h"
#include "dbj++log.h"

///-----------------------------------------------------------------------------------------
// new failure will provoke fast exit is set to 1
#define DBJ_TERMINATE_ON_BAD_ALLOC 1

#if DBJ_TERMINATE_ON_BAD_ALLOC
// do not throw bad_alloc
// call default termination on heap memory exhausted
inline auto setting_new_handler_to_terminate_ = []() {
	std::set_new_handler(
		[] { perror(__FILE__ " Terminating because of heap exhaustion");   std::terminate(); }
	);
	return true;
}();
#else
/* do nothing */
#endif

///-----------------------------------------------------------------------------------------
#ifdef __clang__
#ifdef NDEBUG
#pragma clang system_header
#endif
#endif

#ifdef DBJ_ASSERT
#error remove previous DBJ_ASSERT definition
#endif

#ifdef _ASSERTE
#define DBJ_ASSERT _ASSERTE
#else
/// NOTE! MSVC assert is not constexpr -- G++/CLANG is
#include <cassert>
#define DBJ_ASSERT assert
#endif

/*
--------------------------------------------------------
works for C too
https://www.drdobbs.com/compile-time-assertions/184401873?pgno=1

be carefull to use compile time values for this to work
*/
#define assert_static(e) \
   do { \
      enum { assert_static__ = 1/(e) }; \
      } while (0)

/// --------------------------------------------------------
/// decide which vector you will use
/// call it DBJ_VECTOR in any case
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
#ifdef DBJ_NANO_WIN32

#include <io.h>
#include <fcntl.h>
#define NOMINMAX
#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <system_error>
#include "vt100win10.h"

#endif // DBJ_NANO_WIN32

/// -------------------------------------------------------------------------------
/// there is no `repeat` in C++
/*
this macro is actually superior solution to the repeat template function
_dbj_repeat_counter is local for each macro expansion
usage:
	  DBJ_REPEAT(50){ std::printf("\n%d", _dbj_repeat_counter ); }
*/
#define DBJ_REPEAT(N) for (size_t _dbj_repeat_counter = 0; _dbj_repeat_counter != static_cast<size_t>(N); ++_dbj_repeat_counter)

/// -------------------------------------------------------------------------------
#ifdef _unused
#error _unused is already defined somewhere ...
#else
#define _unused(...) static_assert(noexcept(__VA_ARGS__, true))
#endif

/// -------------------------------------------------------------------------------
namespace dbj::nanolib
{
	enum class SEMVER
	{
		major = 2,
		minor = 9,
		patch = 0
	};
	// SEMVER + TIMESTAMP
	constexpr auto VERSION = "2.9.0 " __TIME__ " " __DATE__;

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
#ifdef DBJ_NANO_WIN32
	void enable_vt_100_and_unicode();
#endif // DBJ_NANO_WIN32

	/// -------------------------------------------------------------------------------
	/* happens once and as soon as possible */
	inline const bool dbj_nanolib_initialized = ([]() -> bool {

		/*
				WIN32 console is one notorious 30+ years old brat
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

				Steve Wishnousky (MSFT) publicly has advised me personaly, against
				using _setmode(), at all
				https://developercommunity.visualstudio.com/solutions/411680/view.html
				*/
#if 0
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
#ifdef DBJ_NANO_WIN32
 // currently (2019Q4) WIN10 CONSOLE "appears" to need manual enabling the ability to
 // interpret VT100 ESC codes
		enable_vt_100_and_unicode(); // enable VT100 ESC code for WIN10 console
#endif // DBJ_NANO_WIN32

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
	// but I do not see the point ;)
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
	   /*
	 terror == terminating error
	 NOTE: std::exit *is* different to C API exit()
	 NOTE: all the bets are of so no point of using some logging
	*/
	[[noreturn]] inline void dbj_terror(const char* /*msg_*/, const char* /*file_*/, const int /*line_*/)
	{
		/// DBJ_ASSERT(msg_ && file_ && line_);
		/// all the bets are of so no point of using some logging
		perror("dbj nanolib Terminating error!");
		std::exit(EXIT_FAILURE);
	}

	///	-----------------------------------------------------------------------------------------
	// CAUTION! DBJ_VERIFY works in release builds too
#ifndef DBJ_VERIFY
#define DBJ_VERIFY_(x, file, line) \
	if (false == x)                \
	::dbj::nanolib::dbj_terror("Expression: " #x ", failed ", file, line)

#define DBJ_VERIFY(x) DBJ_VERIFY_(x, __FILE__, __LINE__)
#endif

///	-----------------------------------------------------------------------------------------
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

	///	-----------------------------------------------------------------------------------------
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

		using buffer_type = DBJ_VECTOR<char>;

		static buffer_type make(size_t count_)
		{
			DBJ_ASSERT(count_ < DBJ_MAX_BUFER_SIZE);
			buffer_type retval_(count_ /*+ 1*/, char(0));
			return retval_;
		}

		static buffer_type make(std::basic_string_view<char> sview_)
		{
			DBJ_ASSERT(sview_.size() > 0);
			DBJ_ASSERT(DBJ_MAX_BUFER_SIZE >= sview_.size());
			buffer_type retval_(sview_.data(), sview_.data() + sview_.size());
			// zero terminate?
			retval_.push_back(char(0));
			return retval_;
		}

		template <typename... Args, size_t max_arguments = 255>
		static buffer_type
			format(char const* format_, Args... args) noexcept
		{
			static_assert(sizeof...(args) < max_arguments, "\n\nmax 255 arguments allowed\n");
			DBJ_ASSERT(format_);
			// 1: what is the size required
			size_t size = 1 + size_t(
				std::snprintf(nullptr, 0, format_, args...));
			DBJ_ASSERT(size > 0);
			// 2: use it at runtime
			buffer_type buf = make(size);
			//
			size = std::snprintf(buf.data(), size, format_, args...);
			DBJ_ASSERT(size > 0);

			return buf;
		}

		// replace char with another char
		static buffer_type replace(buffer_type buff_, char find, char replace)
		{
			char* str = buff_.data();
			while (true)
			{
				if (char* current_pos = strchr(str, find); current_pos)
				{
					*current_pos = replace;
					// shorten next search
					str = current_pos;
				}
				else
				{
					break;
				}
			}
			return buff_;
		}

#ifdef DBJ_NANO_WIN32
		/*
		CP_ACP == ANSI
		CP_UTF8
		*/
		template <auto CODE_PAGE_T_P_ = CP_UTF8>
		static DBJ_VECTOR<wchar_t> n2w(std::string_view s)
		{
			const int slength = (int)s.size() + 1;
			int len = MultiByteToWideChar(CODE_PAGE_T_P_, 0, s.data(), slength, 0, 0);
			DBJ_VECTOR<wchar_t> rez(len, L'\0');
			MultiByteToWideChar(CODE_PAGE_T_P_, 0, s.data(), slength, rez.data(), len);
			return rez;
		}

		template <auto CODE_PAGE_T_P_ = CP_UTF8>
		static buffer_type w2n(std::wstring_view s)
		{
			const int slength = (int)s.size() + 1;
			int len = WideCharToMultiByte(CODE_PAGE_T_P_, 0, s.data(), slength, 0, 0, 0, 0);
			buffer_type rez(len, '\0');
			WideCharToMultiByte(CODE_PAGE_T_P_, 0, s.data(), slength, rez.data(), len, 0, 0);
			return rez;
		}
#endif // DBJ_NANO_WIN32
	};	 // v_buffer
#pragma endregion

///	-----------------------------------------------------------------------------------------
/// 
#define DBJ_PRINT(FMT_, ...) (void)::dbj::nanolib::logging::logfmt(FMT_, __VA_ARGS__)

///	-----------------------------------------------------------------------------------------
/// 
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

#define DBJ_CHK(x)    \
	if (false == (x)) \
	DBJ_PRINT("Evaluated to false! ", DBJ_FLT_PROMPT(x))
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
#pragma region very core type traits
/*
Check at compile time if value (of 'any' type) is inside given boundaries (inclusive)
example usage:

constexpr unsigned sixty_four = inside_inclusive_v<unsigned, 64, 0, 127> ;

template<int K>
using ascii_index_t = ::inside_inclusive_t<unsigned, K, 0, 127>;

constexpr auto ascii_index = ascii_index_t<127>() ;

template<int N>
char i2c () {    return char(ascii_index_t<N>()) ; }

int main () {    char C = i2c<32>(); }
*/
	template <typename T, T X, T L, T H>
	using inside_inclusive_t =
		::std::enable_if_t<(X <= H) && (X >= L),
		::std::integral_constant<T, X>>;

	template <typename T, T X, T L, T H>
	inline constexpr bool inside_inclusive_v = inside_inclusive_t<T, X, L, H>();

#ifdef DBJ_NANOLIB_QUICK_COMPILE_TIME_TESTING

	/*
		this is inclusive inside
		this works if arguments are compile time values
		*/
	template <typename T, T L, T X, T H>
	constexpr bool is_between()
	{
		return (X <= H) && (X >= L);
	}

	static_assert(is_between<unsigned, 0, 64, 127>());

	template <typename T, T L, T X, T H>
	constexpr T between()
	{
		static_assert(std::is_move_constructible_v<T>);
		static_assert(std::is_move_assignable_v<T>);
		static_assert((X <= H) && (X >= L));
		return X;
	}

	static_assert(between<unsigned, 0, 64, 127>());

	static_assert(inside_inclusive_v<unsigned, 64, 0, 127>);

	template <int K>
	using ascii_index_t = inside_inclusive_t<unsigned, K, 0, 127>;

	static_assert(ascii_index_t<64>());
#endif
	/*
		Example usage of bellow:
		ok: static_assert(  all_same_type_v<float, float, float> ) ;
		fails:	static_assert(  dbj::is_any_same_as_first_v<bool,  float, float>  );
		*/
	template <class _Ty,
		class... _Types>
		inline constexpr bool all_same_type_v = ::std::disjunction_v<::std::is_same<_Ty, _Types>...>;
#pragma endregion

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
			dbj_terror("strerror_s failed", __FILE__, __LINE__);
		return buffy_;
#elif __GNUC__
		// TODO: untested!
		v_buffer::buffer_type buffy_ = v_buffer::make(BUFSIZ);
		(void)strerror_r(errno_, buffy_.data(), buffy_.size()))
		dbj_terror("strerror_r failed", __FILE__, __LINE__);
		return buffy_;
#else
		// TODO: untested!
		v_buffer::buffer_type buffy_ = v_buffer::make(strerror(errno_));
		return buffy_;
#endif
	}

#ifdef  DBJ_NANO_WIN32
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
	inline void last_perror(char const* prompt = nullptr)
	{
		std::error_code ec(::GetLastError(), std::system_category());
		DBJ_PRINT("\n\n%s\nLast WIN32 Error message: %s\n\n", (prompt ? prompt : ""), ec.message().c_str());
		::SetLastError(0);
	}

	inline bool set_console_font(std::wstring_view font_name, SHORT font_height_ = SHORT(0))
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
#error ENABLE_VIRTUAL_TERMINAL_PROCESSING not found? Try re-targeting to the latest SDK.
#endif

				/*
							will not exit the app *only* if app is started in WIN32 CONSOLE
							Example: if running from git bash on win this will exit the app
							if app output is redirected to file, this will also fail.
							*/
	inline void enable_vt_100_and_unicode()
	{
		static bool visited{ false };
		if (visited)
			return;

		auto rez = ::SetConsoleOutputCP(CP_UTF8 /*65001*/);
		{
			if (rez == 0)
			{
				last_perror();
				DBJ_PRINT("\nFile: %s\nLine: %ul\nWhy: %s\n", __FILE__, __LINE__, ", SetConsoleOutputCP() failed");
				return;
			}
		}
		// Set output mode to handle virtual terminal sequences
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut == INVALID_HANDLE_VALUE)
		{
			last_perror();
			DBJ_PRINT("\nFile: %s\nLine: %ul\nWhy: %s\n", __FILE__, __LINE__, ", GetStdHandle() failed");
			return;
		}

		DWORD dwMode{};
		if (!GetConsoleMode(hOut, &dwMode))
		{
			last_perror();
			DBJ_PRINT("\nFile: %s\nLine: %ul\nWhy: %s\n", __FILE__, __LINE__, ", GetConsoleMode() failed");
			DBJ_PRINT("\nPlease re-run in either WIN console %s", " or powershell console\n");
			return;
		}

		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		if (!SetConsoleMode(hOut, dwMode))
		{
			last_perror();
			DBJ_PRINT("\nFile: %s\nLine: %ul\nWhy: %s\n", __FILE__, __LINE__, ", SetConsoleMode() failed");
			return;
		}
		visited = true;
	}

	// dbj::nanolib::system_call("@chcp 65001")
	inline bool system_call(const char* cmd_)
	{
		_ASSERTE(cmd_);
		if (0 != system(NULL))
		{
			if (-1 == system(cmd_)) // utf-8 codepage!
			{
				switch (errno)
				{
				case E2BIG:
					last_perror("The argument list(which is system - dependent) is too big");
					break;
				case ENOENT:
					last_perror("The command interpreter cannot be found.");
					break;
				case ENOEXEC:
					last_perror("The command - interpreter file cannot be executed because the format is not valid.");
					break;
				case ENOMEM:
					last_perror("Not enough memory is available to execute command; or available memory has been corrupted; or a non - valid block exists, which indicates that the process that's making the call was not allocated correctly.");
					break;
				}
				return false;
			}
			return true;
		}
		return false;
	}

#endif // _WIN32_WINNT_WIN10

} // namespace dbj::nanolib
#endif // DBJ_NANO_WIN32

#endif // DBJ_NANOLIB_INCLUDED
