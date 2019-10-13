#ifndef _DBJ_STATUS_INC_
#define _DBJ_STATUS_INC_
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

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
inline status_type make_status(char const *information, char const *category_name, char const *file, long line)
{
	auto buff = v_buffer::format(json_code_message_template,
								 0,
								 information,
								 category_name,
								 file,
								 line);
	return buff;
}
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

	static status_type make_status(code_type code, char const *file, long line)
	{
		auto buff = v_buffer::format(json_code_message_template,
									 code_to_int(code),
									 code_to_message(code).data(),
									 category_name(),
									 file,
									 line);
		return buff;
	}
	/* make info status from inside the trait */
	static status_type make_status(char const *information, char const *file, long line)
	{
		return make_status(json_code_message_template,
						   0,
						   information,
						   category_name(), /* trait 'knows' about its category */
						   file,
						   line);
	}

	// just status present  means error
	// --> { { } , { status } }
	static return_type make_error(status_type status_)
	{
		// return value_and_status::make(status_);
		return return_type{{}, {status_}};
	}

	// just value no status is normal return
	// status part is redundant --> { { value } , { } }
	static return_type make_ok(value_type /*const&*/ value_)
	{
		return {{value_}, {}};
	}

	// both status and value we cann "info return"
	// --> { { value } , { status } }
	static return_type make_full(value_type /*const&*/ value_, status_type status_)
	{
		return {{value_}, {status_}};
	}
}; // valstat_trait
   /*----------------------------------------------------------------------------------------------
		And now the shamefull macros ;)
	*/
   /*
	if static method inside the class depends on the nested type
	of valstat_trait above
	the said class will not be defined until the last };
	and that declaration will not compile
	ditto we can simply re-declare that return as local non nested type
	and use it insted
	*/
#define DBJ_DECLARE_VALSTAT_TYPE(VT_) pair<optional<VT_>, optional<dbj::nanolib::v_buffer::buffer_type>>

#define DBJ_STATUS(SVC_, CODE_) SVC_::make_status(CODE_, __FILE__, __LINE__)

// value part is redundant --> { {} , { status } }
#define DBJ_VALSTAT_ERR(SVC_, CODE_) SVC_::make_error(DBJ_STATUS(SVC_, CODE_))

// status part is redundant --> { { value } , { } }
#define DBJ_VALSTAT_OK(SVC_, VAL_) SVC_::make_ok(VAL_)

/*
some use cases do require both value and simple status message
users should, if required, handle that separately
here is just a simple macro to do this for example:
*/
#define DBJ_VALSTAT_FULL(SVC_, VAL_, CODE_) SVC_::make_full(VAL_, DBJ_STATUS(SVC_, CODE_))
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
// particular service will use
// since template declarations are not permited inside functions
// we declare the type here
// error C2951 : template declarations are only permitted at global, namespace, or class scope
template <typename value_type>
using win32 = typename dbj::nanolib::win32_valstat_trait<value_type>;

TU_REGISTER(
	[] {
		// here we pre-define the service we will use
		// here we are saying our status will be posix codes related
		// and the value's return will be of the bool type
		using posix = typename dbj::nanolib::posix_valstat_trait<bool>;

		DBJ_TX(DBJ_STATUS(posix, std::errc::already_connected));
		DBJ_TX(DBJ_STATUS(posix, "Wowza!"));
		DBJ_TX(DBJ_RETVAL_ERR(posix, std::errc::already_connected));
		DBJ_TX(DBJ_RETVAL_OK(posix, true));
		DBJ_TX(DBJ_RETVAL_FULL(posix, true, "OK"));

		// we respect the error_code type for a win32  returns service
		// if you look into it you will see it is a struct
		// it also uses win32 GetLastError to obtain the last win32 error
		// automaticaly, users can not send it in
		using win32_ec = typename dbj::nanolib::win32::error_code;

		DBJ_TX(DBJ_STATUS(win32<bool>, win32_ec{}));
		DBJ_TX(DBJ_STATUS(win32<float>, "Wowza!"));
		DBJ_TX(DBJ_RETVAL_ERR(win32<std::string>, win32_ec{}));
		DBJ_TX(DBJ_RETVAL_OK(win32<std::string>, "this content is made and returned"));
		DBJ_TX(DBJ_RETVAL_FULL(win32<char>, '0', 0));
	});
} // namespace tempo_test
#endif // TESTING_DBJ_RETVALS

#endif // _DBJ_STATUS_INC_