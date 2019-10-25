#ifndef _DBJ_STATUS_INC_
#define _DBJ_STATUS_INC_
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#ifdef __clang__
#pragma clang system_header
#endif

#include "dbj++nanolib.h"
/* from dbj::nanolib we use mainly std::vector<char>  as a buffer */
#define _VALSTAT_BUFFER_TYPE_ v_buffer::buffer_type

namespace dbj::nanolib
{
/*
abstract generic structure, with no apparent solution domain
Four (4) possible states of "occupancy" of this structure are

		1. { { a } , { b } }
		2. { { a } , {   } }
		3. { {   } , { b } }
		4. { {   } , {   } }

*/
template <typename T1_, typename T2_>
using pair_of_options = std::pair<optional<T1_>, optional<T2_>>;

/*
Thinking about and solving the architecture of my return type I have came to 
the conscious and key conceptual decision: value AND status, not value OR status. 
AFAIK, all the solutions up till now are based on the value *OR* error concept, 
most often implemented with a union structure.

DBJ return type concept is: Value AND Status

1. FATAL 	if value AND status are empty that is an fatal error (std::exit() )
2. INFO 	if value AND status are not empty that is info state, e.g in dealing with TCP retval’s
3. OK		Just value –> OK state
4. ERROR	Just status –> ERR state

Bellow are two little top level traits templates for std::errc and for WIN32 ::GetLastError , valstat's
Looks rather complex but just because of namespaces and long type names … And yes there are macros.

		In here we use pair_of_options in the domain of return types
		synopsis:

		auto [value,status] = generic_function() ;
		if ( ! value ) ... then error ..
		else ... use the value ...

		Looks a lot like GO concepts. https://blog.golang.org/error-handling-and-go
		
		First we decide the status type will be a message
		we will use dbj nanolib buffer type
		status type is not a template 
		it is used in any returned type value pair as this same type
		*/
using status_type = typename v_buffer::buffer_type;

/*
	here is the secret sauce DBJ: status is a json encoded string

			which in 99.99% of use cases and in the context of programing is enough.
			in case it is not, we need to decode this string message to find out what was
			the code for example.

			for that purpose we encode the messages as JSON, so if need be we can decode them
			by using home made or one of the numerous other JSON lib's!

			in essence we erase the type by using json formated string messages.
		*/
constexpr auto json_code_message_template =
	//"{ \"code\" : %%d , \"message\" : \"%%s\", \"category\" : \"%%s\", \"location\" : { \"file\" : \"%%s\", \"line\" : %%d } }";
	"{ \"code\" : %d , \"message\" : \"%s\", \"category\" : \"%s\", \"location\" : { \"file\" : \"%s\", \"line\" : %d } }";
/*
general info status is typeless, just a category name is associating it with a solution domain
just make info status, example json generated:

{ "code" : 0 , "message" : "All is fine here", "category" : "win32", location : { file: "main.cpp", line: 42 } }";
*/
inline status_type
make_info_status(char const *information, char const *category_name, char const *file, long line)
{
	auto buff = v_buffer::format(
		json_code_message_template,
		0,
		information,
		category_name,
		file,
		line);
	return buff;
}

// for	the time being we are strict and modern
// thus we pass arguments by value only
// this is about the value part of the valstat
#ifndef DBJ_VALSTAT_VALUE_ARG_PASSING
#define DBJ_VALSTAT_VALUE_ARG_PASSING const &
#endif
// users can combat that situation by using reference_wrapper

/*
-----------------------------------------------------------------------
val stat trait for a particular domain
requires a type, presumably from that domain
and all the other info describing that domain sufficiently
*/
template <
	typename value_type_,
	typename code_type_param, /* has to be castable to int */
	v_buffer::buffer_type (*code_to_message)(code_type_param),
	int (*code_to_int)(code_type_param),
	char const *(*category_name)()>
struct valstat_trait final
{
	using type = valstat_trait;
	using value_type = value_type_;
	using return_type = pair<optional<value_type>, optional<status_type>>;
	using code_type = code_type_param;
	constexpr static inline char const *category = category_name();

	/* default status is made from code */
	static status_type status(code_type code, char const *file, long line)
	{
		auto buff = v_buffer::format(
			json_code_message_template,
			code_to_int(code),
			code_to_message(code).data(),
			category_name(),
			file,
			line);
		return buff;
	}
	/* make info status from inside the trait */
	static status_type status(char const *information, char const *file, long line)
	{
		return make_info_status(
			information,
			category_name(), /* trait 'knows' about its category */
			file,
			line);
	}

	// just status present  means error
	// --> { { } , { status } }
	static return_type error(status_type status_)
	{
		// return value_and_status::make(status_);
		return return_type{{}, {status_}};
	}

	static return_type error(code_type code, char const *file, long line)
	{
		return type::error(type::status(code, file, line));
	}

	static return_type error(char const *information, char const *file, long line)
	{
		return type::error(type::status(information, file, line));
	}

	// just value no status is normal return
	// status part is redundant --> { { value } , { } }
	static return_type ok(value_type DBJ_VALSTAT_VALUE_ARG_PASSING value_)
	{
		return {{value_}, {}};
	}

	// both status and value aka "info return"
	// --> { { value } , { status } }
	static return_type info(value_type DBJ_VALSTAT_VALUE_ARG_PASSING value_, status_type status_)
	{
		return {{value_}, {status_}};
	}

	static return_type info(value_type DBJ_VALSTAT_VALUE_ARG_PASSING value_, code_type code, char const *file, long line)
	{
		return type::info(value_, type::status(code, file, line));
	}

	static return_type info(value_type DBJ_VALSTAT_VALUE_ARG_PASSING value_, const char *information, char const *file, long line)
	{
		return type::info(value_, type::status(information, file, line));
	}

	// no status and no value aka "empty return"
	// --> { {  } , {  } }
	static return_type empty()
	{
		return {{}, {}};
	}

	// the quesion is do we need interrogators at all
	// since structured return binding is the "party way" ?
	// is_error
	// is_ok
	// is_empty
	// is_info

}; // valstat_trait
/*----------------------------------------------------------------------------------------------
		And now the shamefull macros ;)

	if static method inside the class depends on the nested type
	of valstat_trait above
	the said class will not be defined until the last };
	and that declaration will not compile
	ditto we can simply re-declare that return as local non nested type
	and use it insted
	*/
#define DBJ_DECLARE_VALSTAT_TYPE(VT_) \
	pair<optional<VT_>, optional<dbj::nanolib::v_buffer::buffer_type>>

// make_stauts inside a trait is overloaded
// this is not good, CODE_ type can be char * or "something else"
// using macros we do not have types
#define DBJ_STATUS(TRT_, CODE_) TRT_::status(CODE_, __FILE__, __LINE__)

// value part is redundant --> { {} , { status } }
#define DBJ_VALSTAT_ERR(TRT_, CODE_) TRT_::error(DBJ_STATUS(TRT_, CODE_))

// status part is redundant --> { { value } , { } }
#define DBJ_VALSTAT_OK(TRT_, VAL_) TRT_::ok(VAL_)

/*
some use cases do require both value and simple status message
here is just a simple macro to do this for example:
*/
#define DBJ_VALSTAT_FULL(TRT_, VAL_, CODE_) TRT_::info(VAL_, DBJ_STATUS(TRT_, CODE_))
/*
do not go overboard with macros
*/
#define DBJ_PRINT_STATUS(S_) DBJ_FPRINTF(stderr, "\nStatus" DBJ_FG_YELLOW_BOLD " \n%s\n\n" DBJ_RESET, S_->data())

/*----------------------------------------------------------------------------------------------
	And now the two concrete services: posix and win32
*/
namespace posix
{
inline auto code_to_message(std::errc posix_err_code) -> v_buffer::buffer_type
{
	::std::error_code ec = std::make_error_code(posix_err_code);
	return v_buffer::format("%s", ec.message().c_str());
};

inline constexpr auto code_to_int(std::errc posix_err_code) -> int
{
	return static_cast<int>(posix_err_code);
};

constexpr auto category_name() { return "posix"; }

} // namespace posix

template <typename T>
using posix_valstat_trait = valstat_trait<
	// value type
	T,
	//  code type
	std::errc,
	// code to message
	posix::code_to_message,
	// code to int
	posix::code_to_int,
	// category name
	posix::category_name>;

#ifdef DBJ_NANO_WIN32

namespace win32
{
/* win32 error code as a (strong) type */
struct error_code;
struct error_code
{
	int v{0};
	error_code() : v(::GetLastError()) { ::SetLastError(0); }
};

constexpr inline bool is_ok(error_code const &ec_) { return ec_.v == 0; };

inline auto code_to_message(win32::error_code code) -> v_buffer::buffer_type
{
	if (code.v)
		return last_win32_error_message(code.v);
	return v_buffer::format("%s", "No error");
};
inline auto code_to_int(win32::error_code code) -> int
{
	return code.v;
};

constexpr auto category_name() { return "win32"; }
} // namespace win32

template <typename T>
using win32_valstat_trait = valstat_trait<
	// value type
	T,
	//  code type
	win32::error_code,
	win32::code_to_message,
	win32::code_to_int,
	// category name
	win32::category_name>;

#endif // DBJ_NANO_WIN32

} // namespace dbj::nanolib

// #define TESTING_DBJ_RETVALS
#ifdef TESTING_DBJ_RETVALS
#include "dbj++tu.h"

namespace tempo_test
{

// we do not need to predefine the type
// particular valstat trait will use
// template declarations are not permited inside functions
// we declare the templated type alias here
// here we are doing this to avoid very long nested names
// suffix '_vt' denotes "valstat trait"
template <typename value_type>
using win32_vt = typename dbj::nanolib::win32_valstat_trait<value_type>;

inline void prompt(const char *m_ = "")
{
	DBJ_PRINT(DBJ_FG_YELLOW "\n%s" DBJ_RESET, m_);
}

TU_REGISTER(
	[] {
		// here we pre-define the trait we will use
		// here we are saying our status will be posix codes related
		// and the value's return will be of the bool type
		// suffix '_vt' denotes "valstat trait"
		using posix_vt = typename dbj::nanolib::posix_valstat_trait<bool>;

		prompt("posix valstat trait is prepared");
		DBJ_TX(typeid(posix_vt).name());
		// DBJ_TX is a Test Unit macro showing the expression, it's value and type
		// DBJ_STATUS and DBJ_VALSTAT* macros need a
		// valstat trait as a first argument
		prompt("showing how status is made and what is the result json formated string");
		DBJ_TX(DBJ_STATUS(posix_vt, std::errc::already_connected));

		prompt("making a status with user defined message");
		DBJ_TX(DBJ_STATUS(posix_vt, "Wowza!"));

		prompt("making a valstat with particular std::errc error code");
		DBJ_TX(DBJ_VALSTAT_ERR(posix_vt, std::errc::already_connected));

		prompt("this is classical OK return");
		prompt("notice it has to be of the type for which the trait is made");
		DBJ_TX(DBJ_VALSTAT_OK(posix_vt, true));

		prompt("classical FULL return");
		prompt("with user defined message");
		DBJ_TX(DBJ_VALSTAT_FULL(posix_vt, true, "OK"));

		prompt("");
		prompt("Going to test WIN32 valstat trait");

		using win32_ec = typename dbj::nanolib::win32::error_code;
		prompt("WIN32 valstat trait 'code' is a struct i.e it is non trivial");
		DBJ_TX(typeid(win32_ec).name());

		prompt("\nvs the previous example this time we crate required vastat traits ad-hoc");

		prompt("\ntrait for a bool type, status making for the default win32 error code");
		DBJ_TX(DBJ_STATUS(win32_vt<bool>, win32_ec{}));

		prompt("\ntrait for a float type, status making for the user defined message");
		DBJ_TX(DBJ_STATUS(win32_vt<float>, "Wowza!"));

		prompt("\nERR valstat, trait for a string type, status making for the default win32  code");
		DBJ_TX(DBJ_VALSTAT_ERR(win32_vt<std::string>, win32_ec{}));

		prompt("\nOK valstat, trait for a string type, value is made from user defined string literal");
		DBJ_TX(DBJ_VALSTAT_OK(win32_vt<std::string>, "this message is made and returned as std::string"));

		prompt("\nFULL valstat, trait for a char type, value is made from user defined char literal");
		DBJ_TX(DBJ_VALSTAT_FULL(win32_vt<char>, '0', 0));
	});
} // namespace tempo_test
#endif // TESTING_DBJ_RETVALS

#endif // _DBJ_STATUS_INC_