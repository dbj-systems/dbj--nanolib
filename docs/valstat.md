# VALSTAT
## Value & Status
(c) 2019 by dbj.org -- [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)

## Return handling paradigm shift

Three primary objectives

1. Can not wait C++23 and [P0709 realization](https://herbsutter.com/2018/07/02/trip-report-summer-iso-c-standards-meeting-rapperswil/) 
2. Produce a simple, universally applicable solution, now.
3. Achieve maximum with minimum 

They are always coming as trios. After spending much more time than expected, in experimenting and testing, this is my architecture of the solution. With reasoning behind.

And first part of the reasoning behind is to draw logical conclusions from the current state of affairs, around this issue.

#### What do we know so far

C++ is going ([now officially](https://herbsutter.com/2018/07/02/trip-report-summer-iso-c-standards-meeting-rapperswil/)) into the following direction
, without the [outcome](https://ned14.github.io/outcome/) and certainly without the completely unknown `valstat` :

- No to `try`/`catch`/`throw` as we know them today
     - Removal of the `try {}` block -- perhaps
- No to `std::outcome`
- Yes to contracts 
- Yes to `std::error` and `throws` adornments
- No to exceptions popping out of `std::` space

#### What seems to be a common wisdom by now

- return consuming situation is **not** binary, it is not error or no error
   - there are possible outcomes in between these two
   - There are signs collective is moving in that direction. Please if you could, find some time to read [this paper](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1677r1.pdf) aptly titled "Cancellation is not an Error" 
 - Every error **if** consumed as a such, should be treated as one 
    - in that context fast exit is not a bad idea
    - in the same context return might not be an error at all
  - `if`/`else` code replacing `try`/`catch` blocks makes code smaller (Nial has informed that Herb has checked with a Xbox team, I have no source yet)
     -  Why is that so? Because compilers are able to compile if/else cascades to zero bytes, surprisingly well.


## The Works


#### "Error Handling" becomes Light and Useful

Error (handling) was a wrong name. Not every return denotes an error. In this architecture, both value and status are potentially returned. The **"and"** is the key word. Not the **"or"**.

Here is the simple and standard C++ code, actually explaining it all.

```cpp
using namespace std ;

/* the core data structure */
template <typename T1_, typename T2_>
using pair_of_options = pair<optional<T1_>, optional<T2_>>;
```
Here is the solution domain for the above: 
 Instant type to return optional value and optional status.
```cpp
template<typename T>
using valstat = pair_of_options<T, string > ;
```
Not much is developed here, it is all just about using the std:: lib. Not much can go wrong. The simple usage with the "structured binding" core usage idioms.
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
    fprintf ( stdout, "\nValue: %d ", *val ) ; 
        }
    };
    test(+ 42);
    test(- 42);
}
```
And that is it in essence. This is the core proposal. An distillation of many weeks of work. Very simple and logical. No macros. Just standard C++. Although this is just the concept, not the implementation of it.

It might be that valstat and [outcome](https://github.com/ned14/outcome) are not to be compared. Or it might be people will use status code, [well defined in there](https://github.com/ned14/status-code) with valstat, time will tell.


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

1. EMPTY  
    1. Both value AND status empty, are acceptable in some scenarios
2. INFO 
    1. if both value AND status are not empty that is an info state.
3. OK 
   1. Just value is returned 
4. ERROR 
   1. Just status is returned, there is no value.

Does this mean we have to check always, for all four when using this type returned? I think not.

EMPTY, INFO, OK or ERROR consuming  depends on the consumers logic, on the context.

There is a nice term we might use here "algorithms that consume returns".

As an example, consider consuming HTTP codes.
```cpp
// declaration
valstat<http_code> http_get ( uri );
// consuming site
auto [ val, stat ] = http_get("...") ;
   // only in debug builds check for the EMPTY state
   // both can't be empty in the same time
   // in the context of the HTTP returns
   assert( val || stat ) ;
   // no value means "hard" error
    if ( ! val ) return ;
   // or if that is required we can easily pass the valstat
   // to the caller in an error state
   // return {{},{stat}};
```
All the HTTP valstat results are made to be in the INFO state, both value and status are present as described by HTTP protocol 

At debug time we might check if the implementor of http_get() has done that
```cpp
assert( val && stat );


if ( val == http_code(200)) { LOG(stat); /* the request was fulfilled */  }

if ( val == http_code(203)) { LOG(stat); /* partial information */  }

if ( val == http_code(400)) { LOG(stat); /* bad request */ }

// ... and so on ...
```
I any of the cases above, status returned is expected and used. `LOG` is probably some macro using the `syslog()` behind.
## Conclusion

All the similar solutions up till now are based on the "value OR error" concept, 
most often implemented using the union type. Sometime using the [discriminated union](https://pdfs.semanticscholar.org/0a8c/2e0f3a194b15970472dca07c37c2172b69fb.pdf) type, a.k.a variant. 

I might be so bold to claim they are mostly over-engineered. In this instance, I do not implement things. I simply use the types from the std:: lib. This is "Almost a Zero Effort" library. It is more of an agreement to use it's core data structure ubiquitously.

I know about expected and outcome, etc. I  might suggest if and when approaching them please do read first the ["History" page](https://ned14.github.io/outcome/history/). Nial is really great guy (never met him). I think his experiences written here are the most precious. 

Outcome of the outcome V1, peer review, is the most telling part for me. VALSTAT fully conforms to the point 1: **Lightweight**. 

At its core there is almost no implementation. Just usage of the types available from the std:: lib of C++17.

Please do remember the 3 valid and urgent requirements from the top. I am going ahead and using this.

#### Application

I have developed a valstat core inside my `dbj++nanolib`, then I expanded and used it in my minimal SQLITE3, C++ wrap up.

The code is certainly working, but it is perhaps not as simple as in this text, but the concept is exactly the same. I am hoping in future releases, I can make it much simpler.

I do hope the VALSTAT solution for handling c++ returns, is recognized as simple enough to be used and resilient enough to be trusted, by the "significant others".

#### The Future

**Distributed Systems**

Domain where valstat might be used with no much effort. Even in different languages, the same core principles can be preserved.

Think of micro services. Cluster of micro services are orchestrated to make applications. 

On the cluster level request sending and status feedback is reported usually through some messaging. 

valstat status if made to be "just" a string can me JSON formated string. Universally acceptable and usable. Even between components developed in different languages.

I have actually implemented valstat to have this shape in my `dbj++nanolib`.
```cpp
	using return_type = pair<optional<value_type>, optional<status_type>>;
```
Where `status type` is a special char buffer, aka "string". Not an `std::string` .

**C Language**

I am not sure yet. How about

```cpp
typedef valstat struct 
{
  void * value ;
  void * status;
} valstat ;
```

With a "Wall of Macros" around it. C community might have no problems with that. It is all in agreement being reached.

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

It is simpler to type `{}` vs `optional`. Also.

**Please use empty `std::optional<T>` judiciously.**

You might have noticed we do not use std::nullopt, but rather '{}'. Is there a "penalty" if one does (not) uses `std::nullopt`?  It depends on the implementation, but we have not suffered so far.

Now `the core of the trick`: one can try and, for example, output (aka print) the empty optional, but one should never do it. It is very easy and advisable to test the optional value for existence, before using it.

Using the value of empty optional is clearly an "Undefined Behavior" (UB). Please do avoid it.

Just get in the habit, as you do with native pointers.
```cpp
optional<int> opti ;
// some code here
// then
if ( ! opti ) { /* it is empty */ }
else {  auto value = *opti; }

```

## Appendix B

#### Non movable and non copyable types

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

To solve that situation we shall use the `std::reference_wrapper<T>`. Why would we do this? Example

```cpp
// there is always a single instance of a
// single database
class sql::database ;

// since that makes sql::database objects of non movable not copyable type we have to use the reference_wrapper<> like so
// otherwise the code will not compile
using db_valstat = valstat< reference_wrapper<sql::database>> ;

```
Thus we have solved the `optional<T>` handling of the non copyable / non moveable object.

`std::reference_wrapper` has this transformation operator to take out what is in, without using the ugly `get()` method
```cpp
int forty_two = 42 ;
std::reference_wrapper<int> ir = forty_two ;
// the ref type value contained
int & val = ir; 
```
Obviously the `auto` will produce the wrong value
```cpp
int forty_two = 42 ;
std::reference_wrapper<int> ir;
// just the copy of the std::reference_wrapper<int> instance
auto val = ir; 
```
As ever the best course of action is to use C++ , by the book
```cpp
using ref_int = std::reference_wrapper<int> ;
int forty_two = 42 ;
ref_int ir = forty_two ;
// value of the type contained is nested in the ref_int type
ref_int::type val = ir; 
```