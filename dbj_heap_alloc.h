#ifndef DBJ_HEAP_ALLOC_INCLUDE
#define DBJ_HEAP_ALLOC_INCLUDE
/*
(c) 2020 by dbj@dbj.org CC BY SA 4.0

 Now here is the secret sauce key ingredient
 on windows machine these are the fastest
 proven and measured

 Also here are the forward declaration to avoid 
 including windows.h
*/
#ifdef _WIN32


#define DBJ_NANO_CALLOC(T_,S_) (T_*)HeapAlloc(GetProcessHeap(), 0, S_ * sizeof(T_))

#define DBJ_NANO_MALLOC_2(T_,S_)(T_*)HeapAlloc(GetProcessHeap(), 0, S_)

#define DBJ_NANO_FREE(P_) HeapFree(GetProcessHeap(), 0, (void*)P_)

#ifndef _INC_WINDOWS

#ifdef __cplusplus
extern "C" {
#endif __cplusplus

__declspec(dllimport) void* __stdcall  GetProcessHeap(void);
__declspec(allocator) void* __stdcall HeapAlloc(void* /*hHeap*/, int /* flags */, size_t /*dwBytes*/);
int __stdcall HeapFree(void* /*hHeap*/, int  /*dwFlags*/, void* /*lpMem*/);

#ifdef __cplusplus
} // extern "C"
#endif __cplusplus

#endif // _INC_WINDOWS

#else // not WIN32

/// no WIN32 -- standard allocation

#define DBJ_NANO_CALLOC(T_,S_) (T_*)calloc( S_ , sizeof(T_))

#define DBJ_NANO_MALLOC_2(T_,S_)(T_*)malloc( S_ )

#define DBJ_NANO_FREE(P_) free ((void*)P_)

#endif // not WIN32

#endif // DBJ_HEAP_ALLOC_INCLUDE
