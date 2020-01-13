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
// #include <type_traits>
#include <ctime>

#ifndef DBJ_VT100WIN10_INCLUDED
    // VT100 escape codes 
    // and macros
    // for coloured output
    #include "vt100win10.h"
#endif // DBJ_VT100WIN10_INCLUDED

namespace dbj::nanolib::logging
{

using namespace std;

#ifndef DBJ_LOG_MAX_LINE_LEN
#define DBJ_LOG_MAX_LINE_LEN 1024U
#endif // DBJ_LOG_MAX_LINE_LEN

using sink_function_p = void (*)(std::string_view);

namespace detail {
    inline void default_sink_function(std::string_view log_line_) { fprintf(stdout, "\n%s" , log_line_.data() ); }
    inline sink_function_p current_sink_function = default_sink_function;
}

// simplicity == resilience
using timestamp_buffer_type = std::array<char, 0xFF>;
// note! depeneds on the current app wide locale

template< bool add_nano_seconds = false >
inline timestamp_buffer_type high_precision_timestamp()
{
    // available from C++17 onwards
    std::timespec ts;
    std::timespec_get(&ts, TIME_UTC);
    char hours_mins_secs[100];

    // CL err's on using std::gmtime but offers no std::gmtime_s ?
    std::strftime(hours_mins_secs, sizeof hours_mins_secs, "%F %T %z", std::gmtime(&ts.tv_sec));

    timestamp_buffer_type time_stamp_{ {} }; // value initalization of native array inside std::array aggregate

    if constexpr ( add_nano_seconds )
    {
        (void)std::snprintf( &time_stamp_[0], time_stamp_.size() , "[%s (%09ld)]", hours_mins_secs, ts.tv_nsec);
    }
    else {
        (void)std::snprintf(&time_stamp_[0], time_stamp_.size(), "[%s]", hours_mins_secs);
    }

    return time_stamp_;
}

// for when we want no time stamp
inline timestamp_buffer_type null_timestamp()
{
    timestamp_buffer_type time_stamp_{ {} }; // value initalization of native array inside std::array aggregate
    return time_stamp_;
}

// ----------------------------------------------------------------------------------------------------
namespace config {
    
    using namespace std;

    inline array timestamp_functions { high_precision_timestamp<false>, high_precision_timestamp<true>, null_timestamp };

    enum class timestamp_type : int { normal = 0, nanoseconds = 1, nots = 2 };

    inline timestamp_type current_timestamp_idx{ timestamp_type::normal  };

    inline void default_timestamp() { current_timestamp_idx = timestamp_type::normal;  }
    inline void nanosecond_timestamp() { current_timestamp_idx = timestamp_type::nanoseconds; }
    inline void no_timestamp() { current_timestamp_idx = timestamp_type::nots; }

    inline void set_sink_function( sink_function_p new_sfp  ) {
        detail::current_sink_function = new_sfp ;
    }

    inline void default_sink_function ( ) {
        detail::current_sink_function = detail::default_sink_function ;
    }
    
};
// ----------------------------------------------------------------------------------------------------
// general logging function
template<
    typename T1, typename ... T2  >
    inline void log(const T1& first_param, const T2& ... params)
{
    using namespace std;

    char buff_[DBJ_LOG_MAX_LINE_LEN]{ /*zero it the buff_*/ };
    ostringstream os_(buff_);

    auto out = [&]( auto const & obj_) {
        os_ << obj_;
    };

        out( DBJ_FG_GREEN  );
        out(  config::timestamp_functions[ int(config::current_timestamp_idx) ]().data() );
        out( DBJ_RESET );
        out(first_param);
        (..., (out(params)) ); // the rest
    
    os_.flush();
    detail::current_sink_function( os_.str() ) ;
}

// formatted log
template <typename T, typename ... Args>
inline void logf(T const* format_, Args ... args) noexcept
{
    size_t sz = std::snprintf(nullptr, 0, format_, args ...);
    std::vector<char> buffer_(sz + 1); // +1 for null terminator
    std::snprintf(&buffer_[0], buffer_.size(), format_, args ...);

    log(buffer_.data());
}

} //namepsace dbj::nanolib::log

#endif // !DBJ_LOG_INC_

