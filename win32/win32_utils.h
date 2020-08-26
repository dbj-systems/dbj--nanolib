#pragma once

#include "../dbj_windows_include.h"
#include "../dbj_buffer.h"
#include "../dbj_debug.h"

// DBJ TODO: get rid of <system_error>
#include <system_error>


namespace dbj::nanolib {
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

    #ifdef _WIN32_WINNT_WIN10
// dbj::nanolib::system_call("@chcp 65001")
	inline bool system_call(const char* cmd_)
	{
		_ASSERTE(cmd_);
		volatile auto whatever_ = cmd_;

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
