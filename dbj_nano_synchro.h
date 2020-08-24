#ifndef _DBJ_NANO_CYNCHRO_INC_
#define _DBJ_NANO_CYNCHRO_INC_

#ifndef DBJ_NANOLIB_INCLUDED
#error please include dbj++nanolib.h before dbj_nano_synchro.h
#endif

//#include <process.h> /* _beginthread, _endthread */

#ifndef WIN32_LEAN_AND_MEAN
#error please include windows before dbj_nano_synchro.h
#endif // WIN32_LEAN_AND_MEAN

/*
ONE SINGLE PER PROCESS dbj nano critical section
Thus using it in one pleace locks eveything else using it in every other place!

used internaly. if one wants to be in sync with dbj nanolib the one can use this

DBJ NANO LIB is single threaded by default in case user need the opposite please 
#define DBJ_NANO_LIB_MT

Note: this is obviously WIN32 only
*/

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#undef DBJ_NANO_KERNEL_BUILD

    // /kernel CL switch macro
#ifdef _KERNEL_MODE
#define DBJ_NANO_KERNEL_BUILD
#else
#undef DBJ_NANO_KERNEL_BUILD
#endif


#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#define DBJ_NANO_THREADLOCAL __thread
#elif defined(_MSC_VER)
#define DBJ_NANO_THREADLOCAL __declspec(thread)
#else
#error can not create DBJ_NANO_THREADLOCAL ?
#endif

/*
NOTE! __declspec(thread) is not supported with /kernel
*/
#ifdef DBJ_NANO_KERNEL_BUILD
#undef DBJ_NANO_THREADLOCAL
#define DBJ_NANO_THREADLOCAL
#endif 

/// --------------------------------------------------------------------------------------------
    extern "C" {
        int __cdecl atexit(void(__cdecl*)(void));
    }

    /// --------------------------------------------------------------------------------------------
    /// we need to make common function work in presence of multiple threads
    typedef struct
    {
        bool initalized;
        CRITICAL_SECTION crit_sect;
    } dbj_nano_synchro_type;

    dbj_nano_synchro_type* dbj_nano_crit_sect_initor();

    inline void exit_common(void)
    {
        dbj_nano_synchro_type crit_ = *dbj_nano_crit_sect_initor();

        if (crit_.initalized)
        {
            DeleteCriticalSection(&crit_.crit_sect);
            crit_.initalized = false;
        }
    }

    inline dbj_nano_synchro_type* dbj_nano_crit_sect_initor()
    {
        // this means: one per process
        static dbj_nano_synchro_type synchro_ = { false };
        if (!synchro_.initalized)
        {
            InitializeCriticalSection(&synchro_.crit_sect);
            synchro_.initalized = true;
            atexit(exit_common);
        }

        return &synchro_;
    }

    inline void synchro_enter() { EnterCriticalSection(&dbj_nano_crit_sect_initor()->crit_sect); }
    inline void synchro_leave() { LeaveCriticalSection(&dbj_nano_crit_sect_initor()->crit_sect); }

#ifdef DBJ_NANO_LIB_MT
#define DBJ_NANO_LIB_SYNC_ENTER synchro_enter()
#define DBJ_NANO_LIB_SYNC_LEAVE synchro_leave()
#else
#define DBJ_NANO_LIB_SYNC_ENTER 
#define DBJ_NANO_LIB_SYNC_LEAVE 
#endif

#ifdef __cplusplus
} // extern "C" 
#endif // __cplusplus

///	-----------------------------------------------------------------------------------------
#ifdef __cplusplus
#pragma region cpp oo synchronisation
/*
This uses process wide single ciritical section! Be warned.

usage:	void thread_safe_fun() 
{	
lock_unlock autolock_ ;  	

. . . 
wahtever happens here is not entered before it is finished 
. . .

}
*/
struct lock_unlock final
{
    lock_unlock() noexcept {
        synchro_enter();
    }
    ~lock_unlock() { 
        synchro_leave();
    }
};

#pragma endregion
#endif // __cplusplus

#endif // !_DBJ_NANO_CYNCHRO_INC_
