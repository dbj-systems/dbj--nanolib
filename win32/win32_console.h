#pragma once

#include <Windows.h>

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