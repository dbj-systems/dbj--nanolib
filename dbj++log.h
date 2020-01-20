#pragma once
#ifndef DBJ_LOG_INC_
#define DBJ_LOG_INC_
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

#ifndef DBJ_VT100WIN10_INCLUDED
// VT100 escape codes
// and macros
// for coloured output
#include "vt100win10.h"
#endif // DBJ_VT100WIN10_INCLUDED

#pragma region ostream operators for types
#include <sstream>

#pragma region tuple print
namespace dbj::nanolib::logging
{
// for ADL to work (https://en.cppreference.com/w/cpp/language/adl)
// this operator has to be in the same namespace as log() and logf()

#include <tuple>

// https://stackoverflow.com/a/54383242/10870835
// currently (2020 Q1) we base output processing
// on ostringstream
// when C++20 compilkers stabilize we will switch to std::format

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
inline void default_sink_function(std::string_view log_line_) { fprintf(stdout, "\n%s", log_line_.data()); }
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
    std::strftime(hours_mins_secs, sizeof hours_mins_secs, "%T (UTC%z)", &time_buf);

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

inline array timestamp_functions{high_precision_timestamp<false>, high_precision_timestamp<true>, null_timestamp};

enum class timestamp_type : int
{
    normal = 0,
    nanoseconds = 1,
    nots = 2
};

inline timestamp_type current_timestamp_idx{timestamp_type::normal};

inline void default_timestamp() { current_timestamp_idx = timestamp_type::normal; }
inline void nanosecond_timestamp() { current_timestamp_idx = timestamp_type::nanoseconds; }
inline void no_timestamp() { current_timestamp_idx = timestamp_type::nots; }

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

    char buff_[DBJ_LOG_MAX_LINE_LEN]{/*zero it the buff_*/};
    ostringstream os_(buff_);

    auto out = [&](auto const &obj_) {
        os_ << obj_;
    };

    // out(DBJ_FG_GREEN);
    out(config::timestamp_functions[int(config::current_timestamp_idx)]().data());
    // out(DBJ_RESET);

    out(first_param);
    (..., (out(params))); // the rest

    os_.flush();
    detail::current_sink_function(os_.str());
}

// formatted log
template <typename... Args>
inline void logf(const char *format_, Args... args) noexcept
{
    size_t sz = std::snprintf(nullptr, 0, format_, args...);
    std::vector<char> buffer_(sz + 1); // +1 for null terminator
    std::snprintf(&buffer_[0], buffer_.size(), format_, args...);

    log(buffer_.data());
}

} // namespace dbj::nanolib::logging

#endif // !DBJ_LOG_INC_
