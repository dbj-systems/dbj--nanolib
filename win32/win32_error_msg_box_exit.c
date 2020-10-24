
#include "win32_error_msg_box_exit.h"

#include <Windows.h>
#include <strsafe.h>
#include <crtdbg.h>

/*
core nicked from
https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
*/
win32_error_msg
win32_error_msg_box_exit(
    win32_error_msg_box_arg  arg_ret_val 
)
{   // def value is empty string
    win32_error_msg result_err_msg = { (char)'\0' } ;

    // if last error not given ask for it
    if (arg_ret_val.last_error < 1) arg_ret_val.last_error =  GetLastError() ;

    arg_ret_val.error_source = 
        (arg_ret_val.error_source == NULL ? "null" : arg_ret_val.error_source);

    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf = { 0 };
    LPVOID lpDisplayBuf = {0};
    // DWORD dw = GetLastError();

#ifndef  NDEBUG
    int format_rezult = 
#endif // ! NDEBUG
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        arg_ret_val.last_error ,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf,
        0, NULL);

#ifndef  NDEBUG
    _ASSERTE(format_rezult > 0);
    if (format_rezult < 1) {
        LocalFree(lpMsgBuf);
        return result_err_msg; // empty msg
    }
#endif // ! NDEBUG

    // Display the error message and exit the process
    if (arg_ret_val.display_error_message) {
        
        lpDisplayBuf = 
            (LPVOID)LocalAlloc(LMEM_ZEROINIT,
            (strlen((LPCSTR)lpMsgBuf) + strlen((LPCSTR)arg_ret_val.error_source) + 40) * sizeof(char)
        );

        StringCchPrintfA(
            (LPSTR)lpDisplayBuf,
            LocalSize(lpDisplayBuf) / sizeof(char),
            "%s failed\n\nerror code %d\nmessage: %s" ,
            arg_ret_val.error_source, arg_ret_val.last_error, (LPSTR)lpMsgBuf);

        MessageBoxA(NULL, (LPCSTR)lpDisplayBuf, "Error", MB_OK);
    }

    if (arg_ret_val.process_exit) {
        ExitProcess(arg_ret_val.last_error);
    }

    strncpy_s(result_err_msg.data, WIN32_ERROR_MESSAGE_SIZE, (LPCSTR)lpMsgBuf, WIN32_ERROR_MESSAGE_SIZE);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    SetLastError(0); // yes this helps sometimes

	return result_err_msg ;
}