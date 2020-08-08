#pragma once
#ifndef DBJ_LOG_INC_
#define DBJ_LOG_INC_

// will exit if formating strings have escape chars
// never used in release builds
#ifndef NDEBUG
#define DBJ_NANO_LOG_NO_ESCAPE_CODES_FATAL
#endif // !NDEBUG
//
#ifndef DBJ_ERR_PROMPT
#define DBJ_FILE_LINE_TSTAMP __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")[" __TIMESTAMP__ "] "
/* will not compile if MSG_ is not string literal */
#define DBJ_ERR_PROMPT(MSG_) DBJ_FILE_LINE_TSTAMP MSG_
#endif // DBJ_ERR_PROMPT
//
#define _CRT_SECURE_NO_WARNINGS 1
//
// here is a lot of virtual tables
// not fast
// TODO: change to std::format usage when time comes
#include <sstream>
//
#include <array>
#include <string_view>
//
#define __STDC_LIB_EXT1__ 1
#ifndef __STDC_LIB_EXT1__
#error __STDC_LIB_EXT1__ needs to be defined (as 1) before using gmtime_s
#endif
#include <ctime>

///------------------------------------------------------------------
#include "nonstd/nano_printf.h"

///------------------------------------------------------------------
#ifndef DBJ_VT100WIN10_INCLUDED
// VT100 escape codes
// and macros
// for coloured output
#include "vt100win10.h"
#endif // DBJ_VT100WIN10_INCLUDED

#pragma region ostream operators for types
#include <sstream>

///------------------------------------------------------------------
/// basically the rule is: do not incluide any output anywhere 
/// but use the api presented in here
///
/// the most basic output is achieved by redirecting stderr to a file
/// so if you use stdout you will know (if you have a console that is)
#define DBJ_DEFAULT_LOG_STD_TARGET stderr
///------------------------------------------------------------------

#pragma region tuple print
namespace dbj::nanolib::logging
{

// for ADL to work (https://en.cppreference.com/w/cpp/language/adl)
// this operator has to be in the same namespace as log() and logf()

#include <tuple>

// https://stackoverflow.com/a/54383242/10870835
// currently (2020 Q1) we base output processing
// on ostringstream
// when C++20 compilers stabilize we will switch to std::format

namespace detail
{
template <class TupType, size_t... I>
inline std::ostream &tuple_print(std::ostream &os,
                                 const TupType &_tup, std::index_sequence<I...>)
{
    os << "(";
    (..., (os << (I == 0 ? "" : ", ") << std::get<I>(_tup)));
    os << ")";
    return os;
}
} // namespace detail

template <class... T>
inline std::ostream &operator<<(std::ostream &os, const std::tuple<T...> &_tup)
{
    return detail::tuple_print(os, _tup, std::make_index_sequence<sizeof...(T)>());
}

#pragma endregion tuple print
} // namespace dbj::nanolib::logging

#pragma endregion

namespace dbj::nanolib::logging
{

using namespace std;

#ifndef DBJ_LOG_MAX_LINE_LEN
#define DBJ_LOG_MAX_LINE_LEN 1024U
#endif // DBJ_LOG_MAX_LINE_LEN

using sink_function_p = void (*)(std::string_view);

namespace detail
{
inline void default_sink_function(std::string_view log_line_) { fprintf(DBJ_DEFAULT_LOG_STD_TARGET, "\n%s", log_line_.data()); }
inline sink_function_p current_sink_function = default_sink_function;
} // namespace detail

// simplicity == resilience
using timestamp_buffer_type = std::array<char, 0xFF>;
// note! depeneds on the current app wide locale

/*
  1. we do not put Y M D into this time stamp
  2. we add offset from UTC, (UTC+0100) for example
*/
template <bool add_nano_seconds = false>
inline timestamp_buffer_type high_precision_timestamp()
{
#ifndef __STDC_LIB_EXT1__
#error __STDC_LIB_EXT1__ needs to be defined before using gmtime_s
#endif
    // available from C++17 onwards
    timespec ts{};
    struct tm time_buf {};
    timespec_get(&ts, TIME_UTC);
    char hours_mins_secs[100]{};

    errno_t chk = gmtime_s(&time_buf, &ts.tv_sec );

    if (chk) {
        perror("gmtime_s() has failed");
        exit(1);
    }

    // CL err's on using std::gmtime but offers no explanation about __STDC_LIB_EXT1__ requirement
#ifdef DBJ_NANOLIB_TSTAMP_INCLUDES_UTC
    std::strftime(hours_mins_secs, sizeof hours_mins_secs, "%T (UTC%z)", &time_buf);
#else
    std::strftime(hours_mins_secs, sizeof hours_mins_secs, "%T", &time_buf);
#endif // DBJ_NANOLIB_TSTAMP_INCLUDES_UTC

    timestamp_buffer_type time_stamp_{{}}; // value initalization of native array inside std::array aggregate

    if constexpr (add_nano_seconds)
    {
        (void)std::snprintf(&time_stamp_[0], time_stamp_.size(), "[%s (%09ld)]", hours_mins_secs, ts.tv_nsec );
    }
    else
    {
        (void)std::snprintf(&time_stamp_[0], time_stamp_.size(), "[%s]", hours_mins_secs);
    }

    return time_stamp_;
}

// for when we want no time stamp
inline timestamp_buffer_type null_timestamp()
{
    timestamp_buffer_type time_stamp_{{}}; // value initalization of native array inside std::array aggregate
    return time_stamp_;
}

// ----------------------------------------------------------------------------------------------------
namespace config
{
using namespace std;

/*
-----------------------------------------------------------------------------------------------------
*/
inline std::array 
   timestamp_functions
    {
    high_precision_timestamp<false>, high_precision_timestamp<true>, null_timestamp
    };

enum class timestamp_type : int
{
    normal = 0,
    nanoseconds = 1,
    nots = 2
};

inline timestamp_type current_timestamp_idx{timestamp_type::normal};

// default is no timestamp!
inline void default_timestamp() { current_timestamp_idx = timestamp_type::nots; }
inline void nanosecond_timestamp() { current_timestamp_idx = timestamp_type::nanoseconds; }

// singularity in a design
// we know if no time stamp then do not color the empty space for the no time stamp 
void nocolor_timestamp_output();
inline void no_timestamp() {
    current_timestamp_idx = timestamp_type::nots; 
    nocolor_timestamp_output();
}

// ----------------------------------------------------------------------------------------------------
enum class timestamp_output_type : int
{
    color = 0,
    nocolor = 1
};

inline timestamp_output_type current_timestamp_output_type{ timestamp_output_type::color };

template< timestamp_output_type  output_type  >
void timestamp_output_function(  std::ostream & os_  )
{
    auto out = [&](auto const& obj_) {
        os_ << obj_;
    };

    if constexpr (output_type == timestamp_output_type::color) {
        out(DBJ_FG_GREEN);
        out(config::timestamp_functions[int(config::current_timestamp_idx)]().data());
        out(DBJ_RESET);
    }
    else
    {
        out(config::timestamp_functions[int(config::current_timestamp_idx)]().data());
    }
}
inline std::array timestamp_output_functions{
    timestamp_output_function<timestamp_output_type::color>,   /* 0  == timestamp_output_type::color  */
    timestamp_output_function<timestamp_output_type::nocolor>  /* 1 == timestamp_output_type::nocolor */
};

inline auto current_timestamp_output_fp(timestamp_output_type which_ = current_timestamp_output_type) {
    return timestamp_output_functions[ int(which_) ];
}

inline void default_timestamp_output () { current_timestamp_output_type = timestamp_output_type::color; }
inline void color_timestamp_output () { current_timestamp_output_type = timestamp_output_type::color; }
inline void nocolor_timestamp_output() { current_timestamp_output_type = timestamp_output_type::nocolor; }

// ----------------------------------------------------------------------------------------------------
inline void set_sink_function(sink_function_p new_sfp)
{
    detail::current_sink_function = new_sfp;
}

inline void default_sink_function()
{
    detail::current_sink_function = detail::default_sink_function;
}

}; // namespace config
// ----------------------------------------------------------------------------------------------------
// general logging function
template <
    typename T1, typename... T2>
inline void log(const T1 &first_param, const T2 &... params)
{
    using namespace std;
    using namespace config;

    char buff_[DBJ_LOG_MAX_LINE_LEN]{/*zero it the buff_*/};
    ostringstream os_(buff_);

    auto out = [&](auto const& obj_) {
        os_ << obj_;
    };

    current_timestamp_output_fp()( os_ );

    out(first_param);
    (..., (out(params))); // the rest

    os_.flush();
    detail::current_sink_function(os_.str());
}

///------------------------------------------------------------
/// formatted log
/// name logf no can use, dito logfmt it is
template <typename... Args>
inline void logfmt(const char *format_, Args... args) noexcept
{
    DBJ_NANO_LIB_SYNC_ENTER;

#ifdef DBJ_NANO_LOG_NO_ESCAPE_CODES_FATAL
    if (
        strchr(format_, '\n') ||
        strchr(format_, '\r') ||
        strchr(format_, '\t')
        )
    {
#ifdef NDEBUG
        perror("\n\nFATAL ERROR\n\n" DBJ_ERR_PROMPT( "\n\nDo not use escape codes in dbj nano logging formatting\n"  ));
        exit(1);
#else
        printf("\n\nFATAL ERROR IN RELEASE BUILDS!\n\n" DBJ_ERR_PROMPT("\n\nDo not use escape codes in dbj nano logging formatting\n"));
#endif // DEBUG
    }
#endif // NDEBUG

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
#endif // __clang__

    auto buffy = v_buffer::format(format_, args...);

    //size_t sz = std::snprintf(nullptr, 0, format_, args...);
    //std::vector<char> buffer_(sz + 1); // +1 for null terminator
    //std::snprintf(&buffer_[0], buffer_.size(), format_, args...);

    detail::current_sink_function(buffy.data());
#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__
    DBJ_NANO_LIB_SYNC_LEAVE ;
}

#ifdef _WIN32_WINNT_WIN10
/*
current machine may or may not  be on WIN10 where VT100 ESC codes are on by default
they are or have been off by default

Reuired WIN10 build number is 10586 or greater

to dance with exact win version please proceed here:
https://docs.microsoft.com/en-us/windows/win32/sysinfo/verifying-the-system-version
*/

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


#endif // _WIN32_WINNT_WIN10

#pragma region test macros

// all of a sudden here we use  ::dbj::nanolib::logging::log?
// should these not be in that header?

#undef TU_CHECK
#define TU_CHECK(x) do {   \
if (false == (x))      \
{                      \
::dbj::nanolib::logging::log(DBJ_FG_YELLOW, #x, DBJ_RESET, DBJ_FG_RED_BOLD, " -- Failed! ", DBJ_RESET); \
}                      \
} while (0)

/*
    TX stands for Test eXpression
    I know my core principle is not to use iostringstreams, but I am not a zealot
    I am an pragmatist. For simple and usefull test displays one can use iostreams,
    like in this macro bellow.

    Usage:

    DBJ_TX( 4 + 2 );
    */
#undef DBJ_TX 

#define DBJ_TX(x) do {\
dbj::nanolib::logging::log("\n\nExpression: '", DBJ_FG_YELLOW, #x, \
DBJ_RESET, "'\nResult: ", DBJ_FG_YELLOW_BOLD, (x), \
DBJ_RESET, " -- Type: ", typeid(x).name()); \
} while (0)

#pragma endregion

} // namespace dbj::nanolib::logging

#endif // !DBJ_LOG_INC_
