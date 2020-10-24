#pragma once

/* Windows doesn't have gettimeofday. While Cygwin and some
 * versions of MinGW supply one, it is very coarse. This substitute
 * gives much more accurate elapsed times under Windows. */
#if (( defined _WIN32 )) 
#include <windows.h>
static void win_gettimeofday(struct timeval* p, void* tz /* IGNORED */)
{
    LARGE_INTEGER q;
    static long long freq;
    static long long cyg_timer;
    QueryPerformanceFrequency(&q);
    freq = q.QuadPart;
    QueryPerformanceCounter(&q);
    cyg_timer = q.QuadPart;
    p->tv_sec = (long)(cyg_timer / freq);
    p->tv_usec = (long)(((cyg_timer % freq) * 1000000) / freq);
}
#define gettimeofday win_gettimeofday
#endif