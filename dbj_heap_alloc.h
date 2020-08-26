#ifndef DBJ_HEAP_ALLOC_INCLUDE
#define DBJ_HEAP_ALLOC_INCLUDE
/*
(c) 2020 by dbj@dbj.org CC BY SA 4.0

 Now here is the secret sauce key ingredient:
 on windows machine these are the fastest,
 proven and measured

 Here are the forward declaration to avoid 
 including windows.h

 PPL.H -- Disclaimer: yes I know about parrallel maloc and free.  
          This is for when you do not want or cannot include ppl.h
*/

#ifdef _WIN32


#define DBJ_NANO_CALLOC(T_,S_) (T_*)HeapAlloc(GetProcessHeap(), 0, S_ * sizeof(T_))

#define DBJ_NANO_MALLOC(T_,S_)(T_*)HeapAlloc(GetProcessHeap(), 0, S_)

#define DBJ_NANO_FREE(P_) HeapFree(GetProcessHeap(), 0, (void*)P_)

#else // not WIN32

/// no WIN32 -- standard allocation
/// be advised clang can sometimes do some serious magic 
/// with opitmizing these calls

#define DBJ_NANO_CALLOC(T_,S_) (T_*)calloc( S_ , sizeof(T_))

#define DBJ_NANO_MALLOC(T_,S_)(T_*)malloc( S_ )

#define DBJ_NANO_FREE(P_) do { assert(P_ != NULL ); if(P_ != NULL) free(P_); P_ = NULL; } while(0)

#endif // not WIN32

#endif // DBJ_HEAP_ALLOC_INCLUDE
