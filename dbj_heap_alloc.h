#ifndef DBJ_HEAP_ALLOC_INCLUDE
#define DBJ_HEAP_ALLOC_INCLUDE
/*
(c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/
*/

#ifdef _WIN32
#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#define NOMINMAX

#undef  min
#define min(x, y) ((x) < (y) ? (x) : (y))

#undef  max
#define max(x, y) ((x) > (y) ? (x) : (y))

#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
/*
 Now here is the secret sauce secret ingredient:
 On windows machine these are faster vs crt malloc/free. Proven and measured.
 PPL.H -- Disclaimer: yes I know about parrallel maloc and free.
*/

#define DBJ_CALLOC(S_,T_) HeapAlloc(GetProcessHeap(), 0, S_ * sizeof(T_))

#define DBJ_MALLOC( S_) HeapAlloc(GetProcessHeap(), 0, S_)

#define DBJ_FREE(P_) HeapFree(GetProcessHeap(), 0, (void*)P_)

#else // ! WIN32

/// standard allocation
/// be advised clang can sometimes do some serious magic 
/// while optimizing these calls

#define DBJ_CALLOC(S_,T_) calloc( S_ , sizeof(T_))

#define DBJ_MALLOC(S_)malloc( S_ )

#define DBJ_FREE(P_) free(P_)

#endif // ! WIN32

#endif // DBJ_HEAP_ALLOC_INCLUDE
