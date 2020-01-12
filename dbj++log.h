#ifndef DBJ_LOG_INC_
#define DBJ_LOG_INC_
// 
#define _CRT_SECURE_NO_WARNINGS 1
//
#include <iostream>
#include <array>
#include <string_view>
#include <type_traits>
#include <ctime>

namespace dbj::nanolib::logging
{

using namespace std;

// simplicity == resilience
using timestamp_buffer_type = std::array<char, 0xFF>;
// note! depeneds on the current app wide locale
inline timestamp_buffer_type high_precision_timestamp( )
{
    // available from C++17 onwards
    std::timespec ts;
    std::timespec_get(&ts, TIME_UTC);
    char hours_mins_secs[100];

    std::strftime(hours_mins_secs, sizeof hours_mins_secs, "%F %T %z", std::gmtime(&ts.tv_sec));
    
    timestamp_buffer_type time_stamp_{ {} }; // value initalization of native array inside std::array aggregate

    // nano seconds --> (void)std::snprintf( &time_stamp_[0], time_stamp_.size() , "\n[%s.%09ld]", hours_mins_secs, ts.tv_nsec);
    (void)std::snprintf( &time_stamp_[0], time_stamp_.size() , "\n[%s]", hours_mins_secs);

    return time_stamp_;
}

namespace detail {
    // here we decouple from the fact we 
    // are using std::cout as default log sink
    template <typename T>
    inline void out(T const& obj_) {
        std::cout << obj_;
    }
}

// general logging function
template<
    bool timestamp_prefix = true,
    typename T1, typename ... T2  >
    inline void log(const T1& first_param, const T2& ... params)
{
    if constexpr ( timestamp_prefix ) {
        detail::out( high_precision_timestamp().data() );
        detail::out(first_param);
        (..., detail::out(params)); // the rest
    }
    else {
        detail::out(first_param);
        (..., detail::out(params)); // the rest
    }
}

// print is log with no prefix
template<
    typename T1, typename ... T2  >
    inline void print(const T1& first_param, const T2& ... params)
{
    log<false>(first_param, params ... );
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

// prinf is logf with no prefix
template <typename T, typename ... Args>
inline void prinf(T const* format_, Args ... args) noexcept
{
    size_t sz = std::snprintf(nullptr, 0, format_, args ...);
    std::vector<char> buffer_(sz + 1); // +1 for null terminator
    std::snprintf(&buffer_[0], buffer_.size(), format_, args ...);

    log<false>(buffer_.data());
}

} //namepsace dbj::nanolib::log

#endif // !DBJ_LOG_INC_

