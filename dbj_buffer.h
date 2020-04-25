#ifndef DBJ_BUFFER_INC
#define DBJ_BUFFER_INC

/*
-----------------------------------------------------------------------------------------
 2020-04-25 DBJ NOTE: this is pretty clumsy. has to be "normalized"
-----------------------------------------------------------------------------------------
*/

#include <cstdint>
#include <string.h>

#ifndef DBJ_NANOLIB_INCLUDED
#error For now dbj_buffer.h can not be included on its own
#endif // DBJ_NANOLIB_INCLUDED

#pragma region buffer type and helper

namespace dbj::nanolib {

/*
		in case you need more change this
		by default it is 64KB aka 65535 bytes, which is quite a lot perhaps?
		*/
	constexpr inline std::size_t DBJ_MAX_BUFER_SIZE = UINT16_MAX;
	/*
    for runtime buffering the most comfortable and in the same time fast
    solution is vector<char_type>
    only unique_ptr<char[]> is faster than vector of  chars, by a margin
    UNICODE does not mean 'char' is forbiden. We deliver 'char' based buffering
    only.
    Bellow is a helper, with function most frequently used to make buffer aka vector<char>
    */
	struct v_buffer final
	{

		using buffer_type = DBJ_VECTOR<char>;

		static buffer_type make(size_t count_)
		{
			DBJ_ASSERT(count_ < DBJ_MAX_BUFER_SIZE);
			buffer_type retval_(count_ /*+ 1*/, char(0));
			return retval_;
		}

		static buffer_type make(std::basic_string_view<char> sview_)
		{
			DBJ_ASSERT(sview_.size() > 0);
			DBJ_ASSERT(DBJ_MAX_BUFER_SIZE >= sview_.size());
			buffer_type retval_(sview_.data(), sview_.data() + sview_.size());
			// zero terminate?
			retval_.push_back(char(0));
			return retval_;
		}

		template <typename... Args, size_t max_arguments = 255>
		static buffer_type
			format(char const* format_, Args... args) noexcept
		{
			static_assert(sizeof...(args) < max_arguments, "\n\nmax 255 arguments allowed\n");
			DBJ_ASSERT(format_);
			// 1: what is the size required
			size_t size = 1 + size_t(
				std::snprintf(nullptr, 0, format_, args...));
			DBJ_ASSERT(size > 0);
			// 2: use it at runtime
			buffer_type buf = make(size);
			//
			size = std::snprintf(buf.data(), size, format_, args...);
			DBJ_ASSERT(size > 0);

			return buf;
		}

		// replace char with another char
		static buffer_type replace(buffer_type buff_, char find, char replace)
		{
			char* str = buff_.data();
			while (true)
			{
				if (char* current_pos = strchr(str, find); current_pos)
				{
					*current_pos = replace;
					// shorten next search
					str = current_pos;
				}
				else
				{
					break;
				}
			}
			return buff_;
		}

		/*
		CP_ACP == ANSI
		CP_UTF8
		*/
		template <auto CODE_PAGE_T_P_ = CP_UTF8>
		static DBJ_VECTOR<wchar_t> n2w(std::string_view s)
		{
			const int slength = (int)s.size() + 1;
			int len = MultiByteToWideChar(CODE_PAGE_T_P_, 0, s.data(), slength, 0, 0);
			DBJ_VECTOR<wchar_t> rez(len, L'\0');
			MultiByteToWideChar(CODE_PAGE_T_P_, 0, s.data(), slength, rez.data(), len);
			return rez;
		}

		template <auto CODE_PAGE_T_P_ = CP_UTF8>
		static buffer_type w2n(std::wstring_view s)
		{
			const int slength = (int)s.size() + 1;
			int len = WideCharToMultiByte(CODE_PAGE_T_P_, 0, s.data(), slength, 0, 0, 0, 0);
			buffer_type rez(len, '\0');
			WideCharToMultiByte(CODE_PAGE_T_P_, 0, s.data(), slength, rez.data(), len, 0, 0);
			return rez;
		}
	};	 // v_buffer

} // namespace dbj::nanolib 

#pragma endregion

#endif // DBJ_BUFFER_INC