#ifndef _DBJ_STATUS_INC_
#define _DBJ_STATUS_INC_

#include "dbj__nanolib.h"

template <typename T, typename STATUS_ =>
using generic_return_type = std::pair<std::optional<T>, std::optional<std::string>>;

template <typename T>
using return_type = std::pair<std::optional<T>, std::optional<std::string>>;

return_type return_error(std::errc posix_err_code, char const *file, long line)
{
    ::std::error_code ec = std::make_error_code(posix_err_code);
    ::std::string msg = ec.message();
}

#endif // _DBJ_STATUS_INC_