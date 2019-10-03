# VALSTAT
## Value & Status
(c) 2019 by dbj.org -- [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)

## Return handling paradigm shift

Three primary objectives

1. Can not wait C++23 and [P0709 realization](https://herbsutter.com/2018/07/02/trip-report-summer-iso-c-standards-meeting-rapperswil/) 
2. Make a simple, universally applicable solution, now.
3. Achieve maximum with minimum 

They are always coming as trios. After spending much more time than expected, in experimenting and testing, this is my architecture of the solution. With reasoning behind.

#### "Error Handling" becomes Light and Useful

Error (handling) was a wrong name. Not every return denotes an error. In this architecture, both value and status are potentially returned. The **"and"** is the key word. Not **"or"**.

Here is the simple and standard C++ code, actually explaining it all.

```cpp
using namespace std ;

/* the core data structure */
template <typename T1_, typename T2_>
using pair_of_options = std::pair<optional<T1_>, optional<T2_>>;
```
Here is the solution domain for the above: 
 Instant type to return optional value and optional status.
```cpp
/* 
Not much is developed, it is all just about using the std:: lib. Not much can go wrong.
*/
template<typename T>
using valstat = pair_of_options<T, string > ;
```
The simple usage with the "structured binding" core usage idioms.
```cpp
valstat<int> my_fun ( int arg ) {
      if ( arg < 0 ) 
/* on error return status only */
        return {{}, "Argument must be > 0" };
      /* ok, return value only */    
      return {{ arg + arg }, {} };
}

int main( int , char * [] )
{
    auto test = []( int arg_) 
    {
/* Structured binding is the preferred way to consume value and status */
        auto [ val, stat ] = my_fun(arg_);
/* No macros here */    
    if ( ! val  ) 
    { 
    fprintf ( stderr, "\nError: %s ", stat->c_str() ) ; 
    } else {
    fprintf ( stdout, "\nValue: %d ", *(val) ) ; 
        }
    };
    test(+ 42);
    test(- 42);
}
```
An distillation of many weeks of work. Very simple and logical. No macros. Just standard C++. 

## Key concept is the AND word
#### Value AND Status

Recap. `pair_of_options` is the core data structure. 
```cpp
template <typename T1_, typename T2_>
using pair_of_options = std::pair<std::optional<T1_>, std::optional<T2_>>;
```
Four (4) possible states of "occupancy" of this structure are (a and b are instances of types T1 and T2):

 | id  | occupancy pattern   | name   |
 | --- | ------------------- | ------ |
 | 1   | `{ { a } , { b } }` | full   |
 | 2   | `{ { a } , {   } }` | first  |
 | 3   | `{ {   } , { b } }` | second |
 | 4   | `{ {   } , {   } }` | empty  |

Above are the four (4) possible states in which instance of the core structure can exist.

Thinking about and solving the architecture of return types, I have came to 
the conscious and key conceptual conclusion: value AND status, not value OR error. 

Error is just one of the states (a condition) of the return event, at the consuming site. 

Error is a misleading name here. **Status** is the right name for what might be returned, with optional value. 

Both absence and presence, of both value and state, gives the logic, to be used by the consumers aka callers.

 In the core structure, both Value and Status are optional. They might be or might not be present in the structure returned. For the consuming code, this renders four (4) possible states at the consuming site.

1. FATAL  
    1. If both value AND status are empty that is an fatal error
2. INFO 
    1. if both value AND status are not empty that is an info state.
3. OK 
   1. Just value is returned 
4. ERROR 
   1. Just status is returned, there is no value.

Does this mean we have to check always, for all four when using this type returned? I think not.

FATAL state we might take care of checking in debug builds only. INFO, OK or ERROR consuming  depends on the consumers logic, on the context.

As an example, consider consuming HTTP codes.
```cpp
// declaration
valstat<http_code> http_get ( uri );
// consuming site
auto [ val, stat ] = http_get("...") ;
   // only in debug builds check for the FATAL state
   // both can't be empty in the same time
   assert( val || stat ) ;
   // no value means error
    if ( ! val ) return ;
   // if that is required we can easily pass to the caller
   // the valstat<http_code>  in an error state
   // return {{},{stat}};

/* 
all the HTTP valstat results are made to be in the INFO state, both value and status are present as described by HTTP protocol 

at debug time we might check if the implementor of http_get() has done that
*/
assert( val && stat );

/* the request was fulfilled */ 
if ( val == http_code(200)) { LOG(stat); }
/* partial information */ 
if ( val == http_code(203)) { LOG(stat); }
/* bad request */
if ( val == http_code(400)) { LOG(stat);  }

...
```
I any of the cases above, status returned is expected and used. `LOG` is probably some macro using the `syslog()` behind.
## Conclusion

Probably, all the similar solutions up till now are based on the "value OR error" concept, 
most often implemented using the union type. Sometime using the [discriminated union](https://pdfs.semanticscholar.org/0a8c/2e0f3a194b15970472dca07c37c2172b69fb.pdf) type, a.k.a variant. 

I might be so bold to claim they are mostly over-engineered. I do not implement things (at least not in this instance). I simply use the types from the std:: lib.

I do hope this solution for handling c++ returns, is recognized as simple enough to be used and resilient enough to be trusted.

## Appendix A

#### Using `std::nullopt`

Core structure is based on `std::optional` To denote 'empty' we always use optional default constructor instead of `std::nullopt` :

```cpp
// 'empty ' preferred style 
pair_of_options<bool,bool> valstat 
= {
    { true }, 
    {} /* empty */
  } ;
```
To consistently use the preferred style is important for code resilience. It also makes cleaner code. Here is the same but with `std::nullopt` to signal 'empty':

```cpp
// 'empty' as std::nullopt
pair_of_options<bool,bool> valstat 
= {
    { true }, 
    std::nullopt /* additional symbol */
  } ;
```
The standard doesn't say anything about the implementation of those two constructors. According to [optional.ctor]:
```cpp
constexpr optional() noexcept;
constexpr optional(nullopt_t) noexcept;
```
It just specifies the signature of those two constructors, and "Ensures": *...after any of those constructions the optional doesn't contain any value*... No other guarantees are given.

It is simpler to type `{}` vs `optional` and most importantly code exhibits different behavior. 

**Please use `std::optional' judiciously.**

Is there a resilience penalty if one does (not) uses `std::nullopt`?  It depends on the implementation of `std::optional`.

Now `the core of the trick`: one can try and output (aka print) the empty optional, but one should never do it. It is very easy and advisable to test the optional value for existence, before using it.

```cpp
  pair_of_options<bool, string > flags = {{ true },{}};

  if ( ! flags.second )
   // second optional has no value
   // it is in a *empty state*
   // this is how optional is used
```

Further to that, different compilers and std:: implementations do implement `std::optional` differently. Little, but non-trivial test:

```cpp
   pair_of_options<bool, string > flags = {{ true },{}};
        printf("\nFlags A: %d , %s", *flags.first , flags.second->c_str() );
```
Using CLANG, output is garbage for the empty  `optional<string>`. Using G++ output is nicely formated, string: "(null)".
```cpp        
    pair_of_options<bool, string > flags2 = {{ true }, nullopt };
        printf("\nFlags B: %d , %s", *flags2.first , flags2.second->c_str() );
```
Using CLANG, output is empty string, when using `std::optional'. Using G++ output is (a lot of) garbage.

## Appendix B

#### Non movable ans non copyable types

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
Of the above the somewhat controversial one is the requirement **not** to handle references.

To solve that situation we shall use the `std::reference_wrapper<T>`. Example

```cpp
// there is always a single instance of a
// single database
class sql::database ;

// since that makes objects of non movable not copyable type we have to use the reference_wrapper<> like so
using db_valstat = valstat< reference_wrapper<sql::database>> ;

```