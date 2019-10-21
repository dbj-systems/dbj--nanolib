# VALSTAT
## Value & Status
(c) 2019 by dbj.org -- [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)

*"... An API is a defined by the code that calls it, not by the code that implements it ... " -- Tony Van Eerd*

# Implementation Issues

#### Using `std::nullopt`

Core structure is pair of `std::optional`'s.

By the standard, `T` in `optional<T>` cannot be `nullopt_t`. 

To denote 'empty', I always use optional default constructor instead of `std::nullopt` :

```cpp
// 'empty ' preferred style 
pair_of_options<bool,bool> valstat 
= {
    { true }, 
    {} /* empty */
    /* using nullopt also works */
  } ;
```
To consistently use the preferred style is important for code resilience. It also makes cleaner code. Here is the same but with `std::nullopt` to signal 'empty':

The standard doesn't say anything about the implementation of those two constructors, the default one and the one taking `nullopt_t`. According to [optional.ctor]:
```cpp
constexpr optional() noexcept;
constexpr optional(nullopt_t) noexcept;
```
*...Ensures, after any of those constructions the optional doesn't contain any value*... No other guarantees are given. 

**Please use empty `std::optional<T>` judiciously.**

Is there a "penalty" if one does (not) uses `std::nullopt`?  It actually depends on the implementation but ,I have not suffered so far.

Further. Using the value of empty optional is clearly an "Undefined Behavior" (UB). Please do avoid it.

Just get in the habit, as you do with native pointers.
```cpp
optional<int> opti ;
// some code here
// then
if ( ! opti ) { /* it is empty */ }
else {  auto value = *opti; }

```

## Appendix B

#### `optiref` -- Non movable and non copyable types as values returned

As `std::optional` is used as holder of instance of actual type we are bounded by the requirements of that std:: type.

To save you of a lot of prose let me just paste from the `std::optional` implementation

```cpp
    static_assert(!is_reference_v<_Ty>, "T in optional<T> cannot be a reference type (N4659 23.6.2 [optional.syn]/1).");
    static_assert(!is_same_v<remove_cv_t<_Ty>, nullopt_t>,
        "T in optional<T> cannot be nullopt_t (N4659 23.6.2 [optional.syn]/1).");
    static_assert(!is_same_v<remove_cv_t<_Ty>, in_place_t>,
        "T in optional<T> cannot be in_place_t (N4659 23.6.2 [optional.syn]/1).");
    static_assert(is_reference_v<_Ty> || is_object_v<_Ty>,
        "T in optional<T> must be an object type (N4659 23.6.3 [optional.optional]/3).");
    static_assert(is_destructible_v<_Ty> && !is_array_v<_Ty>,
        "T in optional<T> must satisfy the requirements of Destructible (N4659 23.6.3 [optional.optional]/3).");
```
Of the above, the somewhat controversial one is the requirement **not** to handle references.

To solve that situation we shall invent and use the `optiref`.

#### optiref 

Synopsis.

```cpp
	using namespace std;
	/*
	OPTIREF
	----------
	optional references
	*/
	template< typename T>
	using optiref = optional< reference_wrapper< T >>;

	// template guides are not allowed on templated aliases
	// ditto ...
	template<typename T>
	inline auto make_optiref( T & tref_) -> optiref<T>
	{
		return { tref_ };
	}

	// disalow temporaries
	template<typename T> auto make_optiref(T&& tref_)->optiref<T> = delete;
```

Helpers with simple and catchy names, help users to remember and actually use them.

Example usage:

```cpp
// there is always a single instance of a
// single database
sql::database db_;

// since that makes sql::database objects of non movable not copyable type we have to use the reference_wrapper<> like so
using db_valstat = valstat< reference_wrapper<sql::database>> ;
// Ok return
return db_valstat< ref( db_ )  > ;
```

<<NOTE TO EDITORS and myself: above is confused. optiref has no role. Simplify.>>

Side note:

`std::reference_wrapper` has this transformation operator to take out what is in, without using the ugly `get()` method
```cpp
int forty_two = 42 ;
std::reference_wrapper<int> ir = forty_two ;
// the ref type value contained
int & val = ir; 
```
Obviously using the `auto val` would produce the wrong value.
As ever the best course of action is to use C++ , by the book
```cpp
using ref_int = std::reference_wrapper<int> ;
int forty_two = 42 ;
ref_int ir = forty_two ;
// value of the type contained is nested in the ref_int type
ref_int::type val = ir; 
```

# Acknowledgments

My thanks to the numerous people who have contributed feedback. Thanks are due to each and every one of you.

# References

- Lawrence Crowl, Chris Mysen

A Class for Status and Optional Value

http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0262r1.html

- Herb Sutter,

Zero-overhead deterministic exceptions

https://wg21.link/P0709

- Douglas, Niall

SG14 status_code and standard error object for P0709 Zero-overhead deterministic exceptions

https://wg21.link/P1028

- Douglas, Niall

Zero overhead deterministic failure – A unified mechanism for C and C++

https://wg21.link/P1095

- Gustedt, Jens

Out-of-band bit for exceptional return and errno replacement

http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2361.pdf

- Douglas, Niall / Gustedt, Jens

Function failure annotation

http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2429.pdf