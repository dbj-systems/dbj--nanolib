#pragma once
#ifndef DBJ_UTF_CPP_INC
#define DBJ_UTF_CPP_INC

#include "dbj_utf_utils.h"
/* 
C++ dbj utf types using dbj utf functions
*/

namespace dbj::utf {
    /*
    utf32 string is the lowest common denominator
    */
    class utf32_string final {
    public:
        utf32_string() : _length(0), _data(nullptr) {
            // note: parens intentional, _data must be properly initialized
            _data = new char32_t[1]();
        }

        explicit utf32_string(const char* src) : _length(0), _data(nullptr) {
            size_t len = strlen(src);
            // note: parens intentional, _data must be properly initialized
            _data = new char32_t[len + 1]();
            copy_string_8_to_32(_data, len + 1, _length, src);
        }

        explicit utf32_string(const char8_t* src) 
            :    utf32_string(reinterpret_cast<const char*>(src))
        {
        }

        explicit utf32_string(const char32_t* src) : _length(0), _data(nullptr) {
            for (_length = 0; src[_length] != 0; ++_length) {
            }

            // note: parens intentional, _data must be properly initialized
            _data = new char32_t[_length + 1]();
            memcpy(_data, src, _length * sizeof(char32_t));
        }

        explicit utf32_string(const char32_t* src, int len) : _length(len), _data(nullptr) {
            // note: parens intentional, _data must be properly initialized
            _data = new char32_t[len + 1]();
            memcpy(_data, src, len * sizeof(char32_t));
        }

        explicit utf32_string(int len) : _length(0), _data(nullptr) {
            // note: parens intentional, _data must be properly initialized
            _data = new char32_t[len]();
        }

        explicit utf32_string(const utf32_string& that) : _length(that._length), _data(nullptr) {
            // note: parens intentional, _data must be properly initialized
            _data = new char32_t[_length + 1]();
            memcpy(_data, that._data, sizeof(char32_t) * _length);
        }

        utf32_string& operator=(const utf32_string& that) {
            if (this != &that) {
                delete[] _data;
                _data = new char32_t[that._length]();
                _length = that._length;
                memcpy(_data, that._data, sizeof(char32_t) * _length);
            }

            return *this;
        }

        ~utf32_string() { delete[] _data; }

    public:
        char32_t* get() const { return _data; }

        size_t length() const { return _length; }

        const char32_t& operator[](size_t pos) const { return _data[pos]; }

        char32_t& operator[](size_t pos) { return _data[pos]; }

    private:

        void initFromBuffer() {
            for (_length = 0; _data[_length] != 0; ++_length) {
            }
        }

        // size_t chars() const { return _length; }

        size_t _length;
        char32_t* _data;
    };

    class utf8_string final {
        utf8_string(const utf8_string&) = delete;
        utf8_string& operator=(const utf8_string&) = delete;

    public:
        utf8_string() = delete;

        explicit utf8_string(const utf32_string& src) 
            : len_(src.length() * 4 + 1 ),
            data_(new char[len_])

        {
            assert(len_ > 1);
            assert(data_);
            copy_string_32_to_8(data_, len_, src.get());
            assert(data_);
        }

        ~utf8_string() { delete[] data_; }

    public:
        char* get() const noexcept { return data_; }
        size_t size() const noexcept { return len_;  }

    private:
        size_t len_{};
        char* data_{};
    };

    class utf16_string final {

        // no copy
        utf16_string(const utf16_string&) = delete;
        utf16_string& operator=(const utf16_string&) = delete;

    public:
        utf16_string() = delete;

        explicit utf16_string(const utf32_string& src) 
            : len_(src.length() * 2 + 1 ),
            data_(new char16_t[len_])

        {
            assert(len_ > 1);
            assert(data_);
            copy_string_32_to_16(data_, len_, src.get());
            assert(data_);
        }

        ~utf16_string() { delete[] data_; }

    public:
#ifdef WIN32
        wchar_t * get() const noexcept { return  (wchar_t*)data_ ; }
#else
        char16_t* get() const noexcept { return data_; }
#endif

        size_t size() const noexcept { return len_;  }

    private:
        size_t len_{};
        char16_t * data_{};
    };

} // namespace dbj::utf
#endif // !DBJ_UTF_CPP_INC

