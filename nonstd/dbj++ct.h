
#ifndef DBJ_COMPILE_TIME_H_
#define DBJ_COMPILE_TIME_H_

/*
It is easy to get lost and start believeing anything can be compile time in C++

It can not

Also we rely on the optimizations caqpabilities of modern compilers to make the simple functions
bellow almost as fast or as fast as inbuilt equvalents.

But we the added benefit, we can actually use them at C++ compile time. Example

static_assert( false ==  dbj::nanolib::ct::strcompare("A","B") ) ;

The other added benfit is you can follow through here in debugger
The other added benfit is you can place in here your conditional checks and asserts

NOTE: you can not have function named as some CRT function even if it is inside your namespace
NOTE: arguments to ct functions have to be ct themselves, that means: literals

*/

#ifndef size_t
#include <stddef.h> /* size_t */
#endif

namespace dbj::nanolib::ct {

    // memset, memcpy, memcmp, memchr, memmove

    constexpr inline void* memset(void* s, int c, size_t n)
    {
        unsigned char* p = s;
        while (n--)
            *p++ = (unsigned char)c;
        return s;
    }

    constexpr inline int memcmp(const void* s1, const void* s2, size_t n)
    {
        const unsigned char* p1 = s1, * p2 = s2;
        while (n--)
            if (*p1 != *p2)
                return *p1 - *p2;
            else
                p1++, p2++;
        return 0;
    }

    constexpr inline void* memcpy(void* dest, const void* src, size_t n)
    {
        char* dp = dest;
        const char* sp = src;
        while (n--)
            *dp++ = *sp++;
        return dest;
    }

    constexpr inline void* memchr(const void* s, int c, size_t n)
    {
        unsigned char* p = (unsigned char*)s;
        while (n--)
            if (*p != (unsigned char)c)
                p++;
            else
                return p;
        return 0;
    }

    // portable memmove is tricky in C
    // in C++ we have the luxury of template value arguments which are compile time
    // instead of 
    // memmove( dest, src, 32 );
    // you will do
    // memmove<32>( dest, src) ;

    template<size_t n>
    constexpr inline void* memmove(void* dest, const void* src )
    {
        // without n as template argument this will 
        // require C99 VLA feature
        unsigned char tmp[n]{0};
        dbj::nanolib::ct::memcpy(tmp, src, n);
        dbj::nanolib::ct::memcpy(dest, tmp, n);
        return dest;
    }

// -----------------------------------------------------------------------------------------
// compile time native string handling -- C++11 or better
//
// for testing use https://godbolt.org/z/gGL95T
//
// NOTE: C++20 char8_t is a path peppered with a shards of glass, just don't
// go there
//
// NOTE: char16_t and char32_t are ok. if need them add them bellow
//
// NOTE: WIN32 is UTF-16 aka wchar_t universe, WIN32 char API's are all
// translated to wchar_t
//
    constexpr inline bool str_equal(char const* lhs, char const* rhs) noexcept {
        while (*lhs || *rhs)
            if (*lhs++ != *rhs++)
                return false;
        return true;
    }

    constexpr inline bool str_equal(wchar_t const* lhs,
        wchar_t const* rhs) noexcept {
        while (*lhs || *rhs)
            if (*lhs++ != *rhs++)
                return false;
        return true;
    }

    constexpr inline size_t str_len(const char* ch) noexcept {
        size_t len = 0;
        while (ch[len])
            ++len;
        return len;
    }

    constexpr inline size_t str_len(const wchar_t* ch) noexcept {
        size_t len = 0;
        while (ch[len])
            ++len;
        return len;
    }

} // namespace dbj::nanolib::ct 


#endif // !DBJ_COMPILE_TIME_H_
