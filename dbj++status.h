#ifndef _DBJ_STATUS_INC_
#define _DBJ_STATUS_INC_

#include "dbj++nanolib.h"

namespace dbj::nanolib
{
	/*
	generic_return_type generic_function () ;

	auto [value,status] = generic_function() ;

	if ( ! value ) ... then error ..

	In case you have missed it: status is error when there is no value returned
	*/
	template <typename TYPE_, typename STATUS_ >
	using generic_return_type = std::pair<std::optional<TYPE_>, std::optional<STATUS_> >;
	/*
	return_type dbj_function () ;

	auto [value,status] = dbj_function() ;

	if ( ! value )
		 printf ("\nError: %s", status.data() ) ;
	*/
	using status_type = v_buffer::buffer_type;

	template <typename T >
	using return_type = generic_return_type<T, status_type >;

	/*
	here is the secret sauce DBJ: status is a json encoded string

	which in 99.99% of use cases and in the context of programing is enough
	in case it is not, we need to decode this string message to find out what was
	the code for example.

	for that puropse we encode the messages as JSON, so if need be we can decode them
	by using home made or one of the numerous other JSON lib's!

	in essence we erase the type by using json formated string messages.
	*/
	constexpr auto json_code_message_template =
	"{ \"code\" : %%d , \"message\" : \"%%s\", \"category\" : \"%%s\", \"location\" : { \"file\" : \"%%s\", \"line\" : %%d } }";


	template<
		typename code_type_param, /* has to be castable to int */
		v_buffer::buffer_type (*code_to_message)(code_type_param),
		int  (*code_to_int)(code_type_param),
		char const * (* category_name)() >
		struct return_type_service final
	{
		using code_type = code_type_param;

		static status_type make_status(code_type code, char const* file, long line)
		{
			auto buff = v_buffer::format(json_code_message_template,
				code_to_int(code),
				code_to_message(code).data(),
				category_name(),
				file, 
				line
			);
				return buff;
		}

/* just make info status , for example:
		
{ "code" : 0 , "message" : "All is fine here", "category" : "win32", location : { file: "main.cpp", line: 42 } }";
*/
		static status_type make_status(char const * information, char const* file, long line)
		{
			auto buff = v_buffer::format(json_code_message_template,
				0,
				information,
				category_name,
				file,
				line
			);
			return buff;
		}
		/*
		error return has no value, just a status second part
		*/
		template<typename T>
		static return_type<T> make_retval (
			optional<T> value ,
			code_type code, char const* file, long line
		)
		{
			/*	error return has no value, just a status second part	*/
			return { value ,  {  make_status( code, file, line) } };
		}
	}; // return_type_service

	/*
	----------------------------------------------------------------------------------------------
	factory methods is all we need to reach to, and use a particular service
	*/
	template<typename SVC_>
	inline status_type make_status(
		typename SVC_::code_type code,
		char const* file, long line
	) {
		return SVC_::make_status(code, file, line);
	}

	template<typename SVC_, typename T>
	inline return_type<T> make_retval(
		std::optional<T> val ,
		typename SVC_::code_type code,
		char const* file, long line
	) {
		return SVC_::make_retval( val, code, file, line);
	}
	/*
And now the shamefull macros ;)
*/
#define DBJ_STATUS( SVC_, CODE_) dbj::nanolib::make_status<SVC_>( CODE_, __FILE__, __LINE__ )

#define DBJ_RETVAL_ERR( SVC_, T, CODE_) dbj::nanolib::make_retval<SVC_, T >( std::nullopt, CODE_, __FILE__, __LINE__ )

#define DBJ_RETVAL_OK(VAL_) typename dbj::nanolib::return_type< decltype<VAL_> >{ { VAL_ }, {} }

/*
some use cases do require both value and simple status message
users should, if required, handle that separately
here is just a simple macro to do this for example:

return DBJ_RETVAL_INFO( false, "but all was OK" ) ;
*/
#define DBJ_RETVAL_INFO(SVC_, VAL_, MSG_) \
	dbj::nanolib::make_error<SVC_>{ { VAL_ }, { dbj::nanolib::make_status<SVC_>( MSG_ , __FILE__, __LINE__ )  } }
	/*----------------------------------------------------------------------------------------------
	create servises
	*/

	namespace posix {
		inline auto code_to_message (std::errc posix_err_code) -> v_buffer::buffer_type
		{
			::std::error_code ec = std::make_error_code(posix_err_code);
			return v_buffer::format("%s", ec.message().c_str());
		};

		inline constexpr auto code_to_int (std::errc posix_err_code) -> int {
			return static_cast<int>(posix_err_code);
		};

		constexpr auto category_name () { return "posix";  }

	} // posix

	/*
	And now the two concrete services: posix and win32
	*/
	using posix_retval_service = return_type_service <
		//  code type
		std::errc,
		// code to message
		posix::code_to_message,
		// code to int
		posix::code_to_int ,
		// category name
		posix::category_name
	> ;

#ifdef DBJ_NANO_WIN32

	namespace win32{

	/* win32 error code as a (strong) type */
	struct error_code { int v{ 0 }; win32::error_code() : v(::GetLastError()) { ::SetLastError(0); } };

	
	inline auto code_to_message (win32::error_code code) -> v_buffer::buffer_type
	{
		if (code.v)	return last_win32_error_message(code.v);
		return v_buffer::format("%s", "No error");
	};
	inline  auto code_to_int (win32::error_code code) -> int {
		return code.v;
	};

	constexpr auto category_name() { return "win32"; }
	} // win32
	
	using win32_retval_service = return_type_service <
		//  code type
		win32::error_code,
		win32::code_to_message,
		win32::code_to_int,
		// category name
		win32::category_name
	> ;

#endif // DBJ_NANO_WIN32


} // dbj::nanolib

#endif // _DBJ_STATUS_INC_