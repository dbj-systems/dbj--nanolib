#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	
#undef  WIN32_ERROR_MESSAGE_SIZE
#define WIN32_ERROR_MESSAGE_SIZE 0xFF

	typedef struct win32_error_msg_box_arg
	{
		bool display_error_message;
		bool process_exit;
		/*DWORD*/ int last_error /* from GetLastError()*/;
		const char* error_source;
	}
	win32_error_msg_box_arg ;

	typedef struct win32_error_msg { 
		char data[WIN32_ERROR_MESSAGE_SIZE]; 
	} win32_error_msg ;

	win32_error_msg
		win32_error_msg_box_exit(win32_error_msg_box_arg);

#ifdef __cplusplus
} // "C"
#endif // __cplusplus
