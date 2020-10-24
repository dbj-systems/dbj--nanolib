#pragma once
/*
nanolib private type traits so we do not depend on std lib
*/

namespace dbj::typetraits {
	///	-----------------------------------------------------------------------------------------
#pragma region very core type traits
/*
Check at compile time if value (of 'any' type) is inside given boundaries (inclusive)
example usage:

constexpr unsigned sixty_four = inside_inclusive_v<unsigned, 64, 0, 127> ;

template<int K>
using ascii_index_t = ::inside_inclusive_t<unsigned, K, 0, 127>;

constexpr auto ascii_index = ascii_index_t<127>() ;

template<int N>
char i2c () {    return char(ascii_index_t<N>()) ; }

int main () {    char C = i2c<32>(); }
*/
	template <typename T, T X, T L, T H>
	using inside_inclusive_t =
		::std::enable_if_t<(X <= H) && (X >= L),
		::std::integral_constant<T, X>>;

	template <typename T, T X, T L, T H>
	inline constexpr bool inside_inclusive_v = inside_inclusive_t<T, X, L, H>();

#ifdef DBJ_NANOLIB_QUICK_COMPILE_TIME_TESTING

	/*
		this is inclusive inside
		this works if arguments are compile time values
		*/
	template <typename T, T L, T X, T H>
	constexpr bool is_between()
	{
		return (X <= H) && (X >= L);
	}

	static_assert(is_between<unsigned, 0, 64, 127>());

	template <typename T, T L, T X, T H>
	constexpr T between()
	{
		static_assert(std::is_move_constructible_v<T>);
		static_assert(std::is_move_assignable_v<T>);
		static_assert((X <= H) && (X >= L));
		return X;
	}

	static_assert(between<unsigned, 0, 64, 127>());

	static_assert(inside_inclusive_v<unsigned, 64, 0, 127>);

	template <int K>
	using ascii_index_t = inside_inclusive_t<unsigned, K, 0, 127>;

	static_assert(ascii_index_t<64>());
#endif
	/*
		Example usage of bellow:
		ok: static_assert(  all_same_type_v<float, float, float> ) ;
		fails:	static_assert(  dbj::is_any_same_as_first_v<bool,  float, float>  );
		*/
	template <class _Ty,
		class... _Types>
		inline constexpr bool all_same_type_v = ::std::disjunction_v<::std::is_same<_Ty, _Types>...>;
#pragma endregion
} // namespace dbj::typetraits 
