#pragma once

namespace dbj::nanolib {
	/// -------------------------------------------------------------------------------
	/* this can speed up things considerably. but test comprehensively first! */
	inline void assume(bool cond)
	{
#if defined(__clang__) // Must go first -- clang also defines __GNUC__
		__builtin_assume(cond);
#elif defined(__GNUC__)
		if (!cond)
		{
			__builtin_unreachable();
		}
#elif defined(_MSC_VER)
		__assume(cond);
#else
		// Do nothing.
#endif
	}


	/// -------------------------------------------------------------------------------
	/* happens once and as soon as possible */
	inline const bool dbj_nanolib_initialized = ([]() -> bool {

		/*
				WIN32 console is one notorious 30+ years old brat
				WIN32 UNICODE situation does not help at all
				MSFT UCRT team started well then dispersed

				https://www.goland.org/unicode_c_windows/

				To stay sane and healthy, the rules are:

				0. If you need unicode glyphs stick to UTF8 as much as you can
				   -- article above is good but sadly wrong about UTF16, see www.utf8.com
				1. NEVER mix printf and wprintf
				   1.1 you can mix printf and std::cout but very carefully
				   1.2 UCRT and printf and _setmode() are not friends see the commenct bellow, just here
				2. NEVER mix std::cout  and std::wcout
				3. be (very) aware that you need particular font to see *all* of your funky unicode glyphs is windows console
				4. never (ever) use C++20 char8_t and anything using it
				   4.1 if you need to use <cuchar> for utf translations

				*/
#if 0
				/*
				Steve Wishnousky (MSFT) publicly has advised me personaly, against
				using _setmode(), at all
				https://developercommunity.visualstudio.com/solutions/411680/view.html
				*/
				//#define _O_TEXT        0x4000  // file mode is text (translated)
				//#define _O_BINARY      0x8000  // file mode is binary (untranslated)
				//#define _O_WTEXT       0x10000 // file mode is UTF16 (translated)
				//#define _O_U16TEXT     0x20000 // file mode is UTF16 no BOM (translated)
				//#define _O_U8TEXT      0x40000 // file mode is UTF8  no BOM (translated)

		if (-1 == _setmode(_fileno(stdin), _O_U8TEXT)) perror("Can not set mode");
		if (-1 == _setmode(_fileno(stdout), _O_U8TEXT)) perror("Can not set mode");
		if (-1 == _setmode(_fileno(stderr), _O_U8TEXT)) perror("Can not set mode");

		// with _O_TEXT simply no output
		// works with _O_WTEXT, _O_U16TEXT and _O_U8TEXT
		wprintf(L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");

		// "kicks the bucket" with _O_WTEXT, _O_U16TEXT and _O_U8TEXT
		// works with _O_TEXT and u8

		// THIS IS THE ONLY WAY TO USE CHAR AND UTF8 AND HAVE THE UCRT CONSOLE UNICODE OUTPUT
		printf(u8"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");

		// also see the /utf-8 compiler command line option
		// https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=vs-2019&viewFallbackFrom=vs-2017)

		// error C2022:  '1082': too big for character and so on  for every character
		// printf(  "\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");
#endif // 0

// currently (2019Q4) WIN10 CONSOLE "appears" to need manual enabling the ability to
// interpret VT100 ESC codes
// nanolib loggin is deprecated
// logging::enable_vt_100_and_unicode(); // enable VT100 ESC code for WIN10 console

#ifdef DBJ_SYNC_WITH_STDIO
		/*
		We use iostream but only and strictly for dbj++tu testing fwork
		this might(!) slow down the ostreams
		but renders much safer interop with stdio.h
		*/
		ios_base::sync_with_stdio(true);
#endif
		/*-----------------------------------------------------------------------------------------
		immediately call the nano-lib initialization function, and ... do it only once
		*/
		return true; }());

	///	-----------------------------------------------------------------------------------------
	using void_void_function_ptr = void (*)(void);
	// yes I am aware of: https://ricab.github.io/scope_guard/
	// but I do not see the point of that complexity ;)
	template <typename Function_PTR = dbj::nanolib::void_void_function_ptr>
	struct on_scope_exit final
	{
		static Function_PTR null_call() {}
		// much faster + cleaner vs giving nullptr
		// no if in destructor required
		const Function_PTR callable_{ null_call };

		explicit on_scope_exit(Function_PTR fun_) noexcept : callable_(fun_) {}

		~on_scope_exit()
		{
			// no if in destructor required
			callable_();
		}
	}; // eof on_scope_exit

} // namespace dbj::nanolib 