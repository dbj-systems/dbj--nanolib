#pragma once
#ifndef DBJ_UTF_UTILS_INC
#define DBJ_UTF_UTILS_INC

#include <string.h>
/*
this is C++ code
*/
#include "dbj_utf_conversions.h"

namespace dbj::utf {

#ifdef __cpp_char8_t
	// C++20 or modern compiler C++17
#else // legacy 

#ifdef __cpp_lib_char8_t
	// GCC
#else
	typedef unsigned char char8_t;
#endif

#endif // not __cpp_char8_t defined

    inline conversion_result copy_string_8_to_32(char32_t* dst, size_t dstSize,
        size_t& dstCount, const char* src) {
        const UTF8* sourceStart = reinterpret_cast<const UTF8*>(src);
        const UTF8* sourceEnd = sourceStart + strlen(src);
        UTF32* targetStart = reinterpret_cast<UTF32*>(dst);
        UTF32* targetEnd = targetStart + dstSize;

        conversion_result res = convert_utf8_to_utf32(
            &sourceStart, sourceEnd, &targetStart, targetEnd, lenientConversion);

        if (res == conversionOK) {
            dstCount = targetStart - reinterpret_cast<UTF32*>(dst);

            if (dstCount < dstSize) {
                *targetStart = 0;
            }
        }

        return res;
    }

    inline conversion_result copy_string_8_to_32(char32_t* dst, size_t dstSize,
        size_t& dstCount, const char8_t* src) {
        return copy_string_8_to_32(dst, dstSize, dstCount,
            reinterpret_cast<const char*>(src));
    }

    inline size_t strlen_32(const char32_t* str) {
        const char32_t* ptr = str;

        while (*ptr) {
            ++ptr;
        }

        return ptr - str;
    }

    inline size_t strlen8(const char8_t* str) {
        return strlen(reinterpret_cast<const char*>(str));
    }

    inline char8_t* strdup8(const char* src) {
        return reinterpret_cast<char8_t*>(_strdup(src));
    }

    inline void copy_string_32_to_16
    (char16_t* dst, size_t dstSize, size_t* dstCount, const char32_t* src, size_t srcSize) 
    {
        const UTF32* sourceStart = reinterpret_cast<const UTF32*>(src);
        const UTF32* sourceEnd = sourceStart + srcSize;
        char16_t* targetStart = reinterpret_cast<char16_t*>(dst);
        char16_t* targetEnd = targetStart + dstSize;

        conversion_result res = convert_utf32_to_utf16(
            &sourceStart, sourceEnd, &targetStart, targetEnd, lenientConversion);

        if (res == conversionOK) {
            *dstCount = targetStart - reinterpret_cast<char16_t*>(dst);

            if (*dstCount < dstSize) {
                *targetStart = 0;
            }
        }
    }

    inline void copy_string_32_to_16(char16_t * dst, size_t dstLen, const char32_t* src) {
        size_t dstCount = 0;
        copy_string_32_to_16(dst, dstLen, &dstCount, src, strlen_32(src));
    }

    /*------------------------------------------------------------------------*/
    inline void copy_string_32_to_8(char* dst, size_t dstSize, size_t* dstCount,
        const char32_t* src, size_t srcSize) {
        const UTF32* sourceStart = reinterpret_cast<const UTF32*>(src);
        const UTF32* sourceEnd = sourceStart + srcSize;
        UTF8* targetStart = reinterpret_cast<UTF8*>(dst);
        UTF8* targetEnd = targetStart + dstSize;

        conversion_result res = convert_utf32_to_utf8(
            &sourceStart, sourceEnd, &targetStart, targetEnd, lenientConversion);

        if (res == conversionOK) {
            *dstCount = targetStart - reinterpret_cast<UTF8*>(dst);

            if (*dstCount < dstSize) {
                *targetStart = 0;
            }
        }
    }

    inline void copy_string_32_to_8(char* dst, size_t dstLen, const char32_t* src) {
        size_t dstCount = 0;
        copy_string_32_to_8(dst, dstLen, &dstCount, src, strlen_32(src));
    }

    inline void copy_string_32(char32_t* dst, const char32_t* src, size_t len) {
        while (0 < len && *src) {
            *dst++ = *src++;
            --len;
        }

        *dst = 0;
    }

    inline int strncmp32(const char32_t* left, const char32_t* right, size_t len) {
        while (0 < len && *left) {
            if (*left != *right) {
                return *left - *right;
            }

            ++left;
            ++right;
            --len;
        }

        return 0;
    }
} // dbj::utf

#endif // !DBJ_UTF_UTILS_INC

