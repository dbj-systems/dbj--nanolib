#pragma once
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */
#ifndef DBJ_TU_INCLUDED
#define DBJ_TU_INCLUDED

#include "dbj++nanolib.h"
#include "nonstd/dbj++array.h"

// please see this if wondering why do we use dbj::tu::fp_storage_limited
// #error https://stackoverflow.com/q/58569773/10870835
#define  DBJ_USES_STATIC_STORAGE_FOR_TU 1

#ifdef __clang__
#ifdef NDEBUG
#pragma clang system_header
#endif
#endif

/*
testing nano-lib

Usage is with this single macro bellow.
Testing unit is a lambda with no arguments.

TU_REGISTER(
	[]{ / * testing code goes here * / }
);

void main() {
	// execute all registered tests
	dbj::tu::catalog.execute();
}

*/
#define TU_REGISTER inline auto	\
_DBJ_CONCATENATE(dbj_unused_tu_function_pointer_, __LINE__ ) \
 = ::dbj::tu::testing_system::append_test_function

#define DBJ_CHECKING_IS_TU_FP_UNIQUE 0

namespace dbj::tu
{
	using namespace std;

	struct timer final
	{
		using buffer = array<char, 24>;
		using timepoint = typename std::chrono::system_clock::time_point;
		const timepoint start_ = std::chrono::system_clock::now();

		double nano() const
		{
			timepoint end_ = std::chrono::system_clock::now();
			return static_cast<double>((end_ - start_).count());
		}

		double micro() const { return nano() / 1000.0; }
		double seconds() const { return micro() / 1000.0; }
		double decimal3(double arg) { return (std::round(arg * 1000)) / 1000; }

		enum class kind { nano, micro, second };

		friend buffer as_buffer(timer const& timer_, kind which_ = kind::micro)
		{
			buffer retval{ char{0} };
			double arg{};
			char const* unit_{};
			switch (which_)
			{
			case kind::nano:
				arg = timer_.nano();
				unit_ = " nano seconds ";
				break;
			case kind::micro:
				arg = timer_.micro();
				unit_ = " micro seconds ";
				break;
			default: //seconds
				arg = timer_.seconds();
				unit_ = " seconds ";
			}
			std::snprintf(retval.data(), retval.size(), "%.3f%s", arg, unit_);
			return retval;
		}
	}; // timer

	using tu_function = void (*)();
#if (DBJ_USES_STATIC_STORAGE_FOR_TU == 1)

		/*
		4095 test units is a lot of tet units for any kind of project
		more than 4095 test units means something is wrong with 
		a project logic
		*/
		constexpr size_t fp_storage_size{ 0xFFF };


	using units_ = DBJ_ARRAY_STORAGE< tu_function, fp_storage_size >;
#else
// CLANG 8.0.1, 9.x 10.x can not work on this design
// https://stackoverflow.com/q/58569773/10870835
// when CLANG 11 comes, leve this in to test
// after tests are passed ok, then remove it
// non vector design
	using units_ = DBJ_VECTOR<tu_function>;
#endif // DBJ_USES_STATIC_STORAGE_FOR_TU

	inline void line() noexcept {
		DBJ_PRINT("\n----------------------------------------------------------------------");
	}

	struct testing_system final
	{
		inline static units_ units{};

		// method for adding test functions
		// NOTE: __clang__ and __GNUC__  are following the standard
		// so passing lambda/fp by value does the "degrade"
		// to the "function pointer"
		// side effect of that is same adresses for different lambdas
		// meaning: loosing them
		// so be sure to pass lambda/fp by const ref
		static auto append_test_function( tu_function const & fun_) noexcept
		{
#if DBJ_CHECKING_IS_TU_FP_UNIQUE
			{
				bool test_found_before_registration{ false };
				for (auto& elem : units)
				{
					if (elem == fun_) { test_found_before_registration = true; break; }
				}
				if (test_found_before_registration) {
					using dbj::nanolib::v_buffer;
						v_buffer::buffer_type report = v_buffer::format("Test Unit %s [%p], found before registration", typeid(fun_).name(), fun_);
					wstring final(report.begin(), report.end());
					_ASSERT_EXPR(false == test_found_before_registration, final.data());
				}
			}
#endif
			auto rezult = units.push_back(fun_);
			DBJ_ASSERT(rezult != nullptr);
			return fun_;
		}

		void start( int = 0, char ** = nullptr ) const
		{
			DBJ_PRINT(DBJ_FG_CYAN);
			line();
#ifdef __clang__
			//__clang__             // set to 1 if compiler is clang
			//	__clang_major__       // integer: major marketing version number of clang
			//	__clang_minor__       // integer: minor marketing version number of clang
			//	__clang_patchlevel__  // integer: marketing patch level of clang
			//	__clang_version__     // string: full version number
			DBJ_PRINT(DBJ_FG_CYAN  "\nCLANG: %s" DBJ_RESET, __clang_version__);
#else
			DBJ_PRINT("\n _MSVC_LANG: %lu", _MSVC_LANG);
#endif
#if DBJ_TERMINATE_ON_BAD_ALLOC
			DBJ_PRINT(DBJ_FG_RED_BOLD "\nProgram is configured to terminate on heap memory exhaustion" DBJ_RESET);
#else
			DBJ_PRINT(DBJ_FG_RED_BOLD "\nProgram is configured to throw std::bad_alloc on heap memory exhaustion" DBJ_RESET);
#endif
			DBJ_PRINT("\nCatalogue has %zd test units", units.size());
			line();
			DBJ_PRINT(DBJ_RESET);
		}

		void end() const
		{
			DBJ_PRINT(DBJ_FG_CYAN "\n\n All tests done.\n\n" DBJ_RESET);
		}

		void execute(bool listing_ = false) const
		{
			unsigned counter_{};
			start();

			for (tu_function tu_ : units)
			{
				DBJ_ASSERT ( tu_ );
				DBJ_PRINT(DBJ_FG_CYAN "\n\nTest Unit:  " DBJ_FG_RED_BOLD  "%d [%p]\n" DBJ_RESET, counter_++, tu_);
				if (listing_) continue;

				timer timer_{};

				tu_();
				// line();
				DBJ_PRINT(DBJ_FG_CYAN);
				DBJ_PRINT("\nDone in: %s", as_buffer(timer_).data());
				line();
				DBJ_PRINT(DBJ_RESET);
			}
			if (!listing_) end();
		}
	}; // testing system

	constexpr /*inline*/ testing_system catalog;

#pragma region test macros

#define TU_CHECK(x)                                                            \
    do                                                                         \
    {                                                                          \
        if ( false == (x) ){                                                   \
           std::cout << std::boolalpha ;                                       \
           std::cout << DBJ_FG_YELLOW << #x << DBJ_RESET ;                     \
	       std::cout << DBJ_FG_RED_BOLD << "'\nFailed! " << DBJ_RESET ;        \
		}                                                                      \
    } while (0)

	/*
	TX stands for Test eXpression
	I know my core principle is not to use iostreams, but I am not a zealot
	I am an pragmatist. For simple and usefull test displays one can use iostreams,
	like in this macro bellow.

	Usage:

	DBJ_TX( 4 + 2 );
	*/
#define DBJ_TX(x)                                                              \
    do                                                                         \
    {                                                                          \
        std::cout << std::boolalpha << "\n\nExpression: '"                     \
                  << DBJ_FG_YELLOW << #x << DBJ_RESET                          \
                  << "'\nResult: " << DBJ_FG_YELLOW_BOLD << (x) << DBJ_RESET   \
                  << "\nType: " << typeid(x).name() << "\n";                   \
    } while (0)

#pragma endregion

} // namespace dbj::tu

#ifdef DBJ_TX
#include <iostream>

// https://stackoverflow.com/a/54383242/10870835

#pragma region tuple print

#include <tuple>

namespace detail {
	template<class TupType, size_t... I>
	inline
		std::ostream& tuple_print(std::ostream& os,
			const TupType& _tup, std::index_sequence<I...>)
	{
		os << "(";
		(..., (os << (I == 0 ? "" : ", ") << std::get<I>(_tup)));
		os << ")";
		return os;
	}
}

template<class... T>
inline
std::ostream& operator<< (std::ostream& os, const std::tuple<T...>& _tup)
{
	return detail::tuple_print(os, _tup, std::make_index_sequence<sizeof...(T)>());
}

#pragma endregion tuple print

/*
primary runtime buffer is vector of a char_type
primary compile time buffer is array of a char_type
thus I will put required operators in here
to print them as strings
*/

inline std::ostream& operator<<(std::ostream& os_, DBJ_VECTOR<char> buff_)
{
	if (os_.good())
	{
		os_ << buff_.data();
	}
	return os_;
}

inline std::ostream& operator<<(std::ostream& os_, DBJ_VECTOR<wchar_t> buff_)
{
	if (os_.good())
	{
		os_ << buff_.data();
	}
	return os_;
}

template <size_t N>
inline std::ostream& operator<<(std::ostream& os_, std::array<char, N> buff_)
{
	if (os_.good())
	{
		os_ << buff_.data();
	}
	return os_;
}

template <size_t N>
inline std::ostream& operator<<(std::ostream& os_, std::array<wchar_t, N> buff_)
{
	if (os_.good())
	{
		os_ << buff_.data();
	}
	return os_;
}

inline std::ostream& operator<<(std::ostream& os_, std::nullopt_t const&)
{
	if (os_.good())
	{
		os_ << "nullopt";
	}
	return os_;
}

/*
no this does not help
https://www.boost.org/doc/libs/1_34_0/boost/optional/optional_io.hpp
*/
template <typename T1>
inline std::ostream& operator<<(std::ostream& os_, std::optional<T1> const& opt_)
{
	if (os_.good())
	{
		if (opt_)
			return os_ << std::boolalpha << "{ " << *opt_ << " }";

		return os_ << "{ empty }";
	}
	return os_;
}

/*
std::pair pair **was** the core of valstat_1
*/
template <typename T1, typename T2>
inline std::ostream& operator<<(std::ostream& os_, std::pair<T1, T2> pair_)
{
	if (os_.good())
	{
		os_ << "{ " << pair_.first << " , " << pair_.second << " }";
	}
	return os_;
}
#endif

#endif // DBJ_TU_INCLUDED