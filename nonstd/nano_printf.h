#pragma once
#ifndef __NANO_PRINTF__INC__
#define __NANO_PRINTF__INC__

#include <stdarg.h>
#include <stddef.h>

extern "C" {

#define PRINTF_LONG_SUPPORT

    typedef void (*putcf) (void*, char);

    // GCC thread_local macro
    static DBJ_NANO_THREADLOCAL putcf stdout_putf;
    static DBJ_NANO_THREADLOCAL void* stdout_putp;

	void nano_init_printf(void* putp, void (*putf) (void*, char));
    void nano_printf(const char* fmt, ...);
	void nano_sprintf(char* s, const char* fmt, ...);
    void nano_format(void* putp, putcf putf, const char* fmt, va_list va);

#ifdef NANO_PRINTF_IN_USE
	#define printf nano_printf
	#define sprintf nano_sprintf
#endif // NANO_PRINTF_IN_USE

	/*
	*******************************************************************************
	*/

#ifdef PRINTF_LONG_SUPPORT

    inline void uli2a(unsigned long int num, unsigned int base, int uc, char* bf)
    {
        int n = 0;
        unsigned int d = 1;
        while (num / d >= base)
            d *= base;
        while (d != 0) {
            int dgt = num / d;
            num %= d;
            d /= base;
            if (n || dgt > 0 || d == 0) {
                *bf++ = (char)(dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10));
                ++n;
            }
        }
        *bf = 0;
    }

    inline void li2a(long num, char* bf)
    {
        if (num < 0) {
            num = -num;
            *bf++ = '-';
        }
        uli2a(num, 10, 0, bf);
    }

#endif

    inline void ui2a(unsigned int num, unsigned int base, int uc, char* bf)
    {
        int n = 0;
        unsigned int d = 1;
        while (num / d >= base)
            d *= base;
        while (d != 0) {
            int dgt = num / d;
            num %= d;
            d /= base;
            if (n || dgt > 0 || d == 0) {
                *bf++ = (char)(dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10));
                ++n;
            }
        }
        *bf = 0;
    }

    inline void i2a(int num, char* bf)
    {
        if (num < 0) {
            num = -num;
            *bf++ = '-';
        }
        ui2a(num, 10, 0, bf);
    }

    inline int a2d(char ch)
    {
        if (ch >= '0' && ch <= '9')
            return ch - '0';
        else if (ch >= 'a' && ch <= 'f')
            return ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F')
            return ch - 'A' + 10;
        else return -1;
    }

    inline char a2i(char ch, const char** src, int base, int* nump)
    {
        const char* p = *src;
        int num = 0;
        int digit;
        while ((digit = a2d(ch)) >= 0) {
            if (digit > base) break;
            num = num * base + digit;
            ch = *p++;
        }
        *src = p;
        *nump = num;
        return ch;
    }

    inline void putchw(void* putp, putcf putf, int n, char z, char* bf)
    {
        char fc = z ? '0' : ' ';
        char ch;
        char* p = bf;
        while (*p++ && n > 0)
            n--;
        while (n-- > 0)
            putf(putp, fc);
        while ((ch = *bf++))
            putf(putp, ch);
    }

    inline void nano_format(void* putp, putcf putf, const char* fmt, va_list va)
    {
        char bf[24];

        char ch;


        while ((ch = *(fmt++))) {
            if (ch != '%')
                putf(putp, ch);
            else {
                char lz = 0;
#ifdef  PRINTF_LONG_SUPPORT
                char lng = 0;
#endif
                int w = 0;
                ch = *(fmt++);
                if (ch == '0') {
                    ch = *(fmt++);
                    lz = 1;
                }
                if (ch >= '0' && ch <= '9') {
                    ch = a2i(ch, &fmt, 10, &w);
                }
#ifdef  PRINTF_LONG_SUPPORT
                if (ch == 'l') {
                    ch = *(fmt++);
                    lng = 1;
                }
#endif
                switch (ch) {
                case 0:
                    goto abort;
                case 'u': {
#ifdef  PRINTF_LONG_SUPPORT
                    if (lng)
                        uli2a(va_arg(va, unsigned long int), 10, 0, bf);
                    else
#endif
                        ui2a(va_arg(va, unsigned int), 10, 0, bf);
                    putchw(putp, putf, w, lz, bf);
                    break;
                }
                case 'd': {
#ifdef  PRINTF_LONG_SUPPORT
                    if (lng)
                        li2a(va_arg(va, unsigned long int), bf);
                    else
#endif
                        i2a(va_arg(va, int), bf);
                    putchw(putp, putf, w, lz, bf);
                    break;
                }
                case 'x': case 'X':
#ifdef  PRINTF_LONG_SUPPORT
                    if (lng)
                        uli2a(va_arg(va, unsigned long int), 16, (ch == 'X'), bf);
                    else
#endif
                        ui2a(va_arg(va, unsigned int), 16, (ch == 'X'), bf);
                    putchw(putp, putf, w, lz, bf);
                    break;
                case 'p':
                { /* Print one or two lots of %x depending on sizeof(size_t) */
                    size_t pointer = (size_t)va_arg(va, void*);
                    lz = 1;
                    w = 8;
                    if (sizeof(size_t) > 4)
                    {
                        ui2a((unsigned int)((pointer >> 32) & 0xffffffff), 16, 0, bf);
                        putchw(putp, putf, w, lz, bf);
                    }
                    ui2a((unsigned int)(pointer & 0xffffffff), 16, 0, bf);
                    putchw(putp, putf, w, lz, bf);
                    break;
                }
                case 'c':
                    putf(putp, (char)(va_arg(va, int)));
                    break;
                case 's':
                    putchw(putp, putf, w, 0, va_arg(va, char*));
                    break;
                case '%':
                    putf(putp, ch);
                default:
                    break;
                }
            }
        }
    abort:;
    }


    inline void nano_init_printf(void* putp, void (*putf) (void*, char))
    {
        stdout_putf = putf;
        stdout_putp = putp;
    }

    inline void nano_printf(const char* fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        nano_format(stdout_putp, stdout_putf, fmt, va);
        va_end(va);
    }

    inline void putcp(void* p, char c)
    {
        *(*((char**)p))++ = c;
    }

    inline void nano_sprintf(char* s, const char* fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        nano_format(&s, putcp, fmt, va);
        putcp(&s, 0);
        va_end(va);
    }

} // "C"


/*
2020 APR 11	DBJ	Transformed into single header C lib
                This version does not use heap allocations
                See the long commnet just bellow

This is an implementation of an embedded printf which makes
no use of malloc. Taken from
http://www.sparetimelabs.com/tinyprintf/index.html.

    modifications are:
 1. PRINTF_LONG_SUPPORT is permanently enabled.
 2. Support has been added for %p (32 and 64 bits).
 3. The code has been updated to C99 standards.

 ****************************************************************************

 File: printf.h

Copyright (C) 2004  Kustaa Nyholm

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

This library is realy just two files: 'printf.h' and 'printf.c'.

They provide a simple and small (+200 loc) printf functionality to
be used in embedded systems.

I've found them so usefull in debugging that I do not bother with a
debugger at all.

They are distributed in source form, so to use them, just compile them
into your project.

Two printf variants are provided: printf and sprintf.

The formats supported by this implementation are: 'd' 'u' 'c' 's' 'x' 'X'.

Zero padding and field width are also supported.

If the library is compiled with 'PRINTF_SUPPORT_LONG' defined then the
long specifier is also
supported. Note that this will pull in some long math routines (pun intended!)
and thus make your executable noticably longer.

The memory foot print of course depends on the target cpu, compiler and
compiler options, but a rough guestimate (based on a H8S target) is about
1.4 kB for code and some twenty 'int's and 'char's, say 60 bytes of stack space.
Not too bad. Your milage may vary. By hacking the source code you can
get rid of some hunred bytes, I'm sure, but personally I feel the balance of
functionality and flexibility versus  code size is close to optimal for
many embedded systems.

To use the printf you need to supply your own character output function,
something like :

    void putc ( void* p, char c)
        {
        while (!SERIAL_PORT_EMPTY) ;
        SERIAL_PORT_TX_REGISTER = c;
        }

Before you can call printf you need to initialize it to use your
character output function with something like:

    nano_init_printf(NULL,putc);

Notice the 'NULL' in 'nano_init_printf' and the parameter 'void* p' in 'putc',
the NULL (or any pointer) you pass into the 'nano_init_printf' will eventually be
passed to your 'putc' routine. This allows you to pass some storage space (or
anything realy) to the character output function, if necessary.
This is not often needed but it was implemented like that because it made
implementing the sprintf function so neat (look at the source code).

The code is re-entrant, except for the 'nano_init_printf' function, so it
is safe to call it from interupts too, although this may result in mixed output.
If you rely on re-entrancy, take care that your 'putc' function is re-entrant!

The printf and sprintf functions are actually macros that translate to
'nano_printf' and 'nano_sprintf'. This makes it possible
to use them along with 'stdio.h' printf's in a single source file.
You just need to undef the names before you include the 'stdio.h'.
Note that these are not function like macros, so if you have variables
or struct members with these names, things will explode in your face.
Without variadic macros this is the best we can do to wrap these
function. If it is a problem just give up the macros and use the
functions directly or rename them.

For further details see source code.

regs Kusti, 23.10.2004
*/

#endif // __NANO_PRINTF__INC__

