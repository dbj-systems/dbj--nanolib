# VALSTAT
## Value & Status
(c) 2019 by dbj.org -- [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)

*"There are two ways of constructing a software design: One way is to make it so simple that there are obviously no deficiencies, and the other way is to make it so complicated that there are no obvious deficiencies..." -- [C. A. R. Hoare](https://en.wikiquote.org/wiki/C._A._R._Hoare)*

## Return handling paradigm shift

Currently (2019 Q4) there is no usable, simple and resilient standard return type in standard C++. And there is no consensus around such thing. No thing no consensus.

This concept and implementation have risen as the direct consequence of my [Reddit post](https://www.reddit.com/r/cpp/comments/ae60nb/decades_have_passed_standard_c_has_no_agreed_and/). Here, I am proposing this as a concept and core implementation around which consensus might be made. By which we mean ISO C++ committee consensus.

As evident from the that post and comments of many "fellow sufferers" there are three primary objectives:

1. Modernize C++ error handling and move toward a mostly-noexcept world. We can not wait C++23 and [P0709 realization](https://herbsutter.com/2018/07/02/trip-report-summer-iso-c-standards-meeting-rapperswil/) 
2. Produce a simple, universally applicable solution, now.
3. Achieve maximum with minimum 
   1. An concept & implementation for a common function result type with some tradeoffs. 

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


## The Proposal

In one sentence. I recommend each and every return consuming, site to follow this usage pattern:

```cpp
using namespace std;

auto [value, status] = function() ;

if (value)
   cout << endl << "Value: " << *value;

if (status)
   cout << endl << "Status: " << *status;
```


#### Light and Useful "Return Handling" 

Error (handling) is a wrong name. Not every return denotes an error. In this architecture, both value and status are (potentially) returned. The **"and"** is the key word. Not the **"or"**.

Here is the simple and standard C++ code, actually discovering it all.

```cpp
using namespace std ;

/* 
the core, generic,  data structure 
*/
template <typename T1_, typename T2_>
using pair_of_options = pair<optional<T1_>, optional<T2_>>;
```
Here is the solution domain for the above: 
 "Instant" type to return optional value and optional status.
```cpp
template<typename T>
using valstat = pair_of_options<T, string > ;
```
Added secret sauce is: ***status is a string.***

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
/* Structured binding is the way to consume value and status */
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
And that is it in essence. That is the fully functional code solving a lot (An distillation of many weeks of work). Very simple and logical. No macros. Just standard C++. Although this is just the concept, not the full implementation of it, but fully usable nevertheless.

### Simplicity == ubiquity

It might be that valstat and [outcome](https://github.com/ned14/outcome) are not to be compared. Or it might be people will use status code, [well defined in there](https://github.com/ned14/status-code) with valstat, time will tell.


#### Transparent Architecture

`pair_of_options` is the core data structure. It is generic but it imposes behavior through states of its instances.
```cpp
using namespace std;
template <typename T1_, typename T2_>
using pair_of_options = pair<optional<T1_>, optional<T2_>>;
```
Four (4) possible states of "occupancy" of the instance of this type are (a and b are instances of types T1 and T2):

 | id  | occupancy pattern   | name   |
 | --- | ------------------- | ------ |
 | 1   | `{ { a } , { b } }` | full   |
 | 2   | `{ { a } , {   } }` | first  |
 | 3   | `{ {   } , { b } }` | second |
 | 4   | `{ {   } , {   } }` | empty  |

Above are the four (4) possible states in which instance of the core structure can exist.

#### Error is a state of return

Thinking about and solving the architecture of various complex return types, I have came to 
the conscious and key conceptual conclusion: value AND status, not value OR error. 

Error is just one of the states (a condition) of the returned value and status duet. 

Thus error is a misleading name here. **Status** is the right name for what might be returned, always in pair with value. Both optional.

Both absence and presence, of both value and state, delivers the logic, to be used by the consumers aka callers.

For the consuming code, this renders four (4) possible states at the returning and consuming sites.

#### Terminology

1. EMPTY  
    1. Both value AND status empty
2. INFO 
    1. if both value AND status are not empty.
3. OK 
   1. Just value is returned 
4. ERROR 
   1. Just status is returned, there is no value.

#### Is this complex returns consuming?

Does this mean we have to check always, for all four when using this type returned? I think not.

EMPTY, INFO, OK or ERROR consuming  depends on the consumers logic, and most importantly on the **context**.

There is a nice term we might use here: *"algorithms that consume returns" -- Kirk Shoop*.

As an example, consider the context of proverbial HTTP codes.
```cpp
// declaration
valstat<http_code> http_get ( uri );
// consuming site
auto [ val, stat ] = http_get("localhost:42") ;
   // only in debug builds check for the EMPTY state
   // both can't be empty in the same time
   // in this context
   assert( val || stat ) ;
   // no value means "hard" error
    if ( ! val ) return ;
   // or if that is required we can easily pass the valstat
   // to the caller in an error state
   // return {{},{stat}};
```
All the HTTP valstat results are made to be in the INFO state, both value and status are present as described by HTTP protocol. 

At debug time we might check if the implementor of http_get() has done that right.
```cpp
// out-of-the-box ability
// of the valstat elements
assert( val && stat );
```
Now we proceed to use the value **and** state at consuming algorithm site.
```cpp
if ( val == http_code(200)) { LOG(stat); /* the request was fulfilled */  }

if ( val == http_code(203)) { LOG(stat); /* partial information */  }

if ( val == http_code(400)) { LOG(stat); /* bad request */ }

// ... and so on ...
```
I any of the cases above, both value and status returned are expected and used. (`LOG` is probably some macro using the `syslog()` behind)
## Concept Conclusion

ISO C++ community, collective knowledge on the thorny subject of consistent error handling has recently provoked significant developments.

As far as I know, all the similar solutions up till now are based on the "value OR error" concept, 
most often implemented using the union type. Sometimes using the [discriminated union](https://pdfs.semanticscholar.org/0a8c/2e0f3a194b15970472dca07c37c2172b69fb.pdf) type, a.k.a variant. 

I might be so bold to claim they are mostly over-engineered. Requiring somewhat bolder faith in implementations qualities. Thus raising the bar to adoption, even in the new code base.

I might suggest please do 30 sec detour to the ["History" page](https://ned14.github.io/outcome/history/), on the "outcome" site. I think these experiences are very good reminder on what it takes to adopt new concept and its implementation. However critical and useful it might be.

In that short text, outcome of the `outcome` V1, peer review, is the most telling part for me. The point 1 is: **Lightweight**. 

In the plausible summary of the valstat concept implementation, I do hardly implement anything. I mostly use the types from the std:: lib. This is "Almost a Zero Effort" concept and implementation. It is more of an agreement to use it ubiquitously.

In here it is easy to spot a fine balance between tradeoffs and comfort.

I do hope the `valstat` is branded as a "solution" for consistently, resiliently and simply, managing c++ function returns. I do hope it is recognized as simple enough to be used and resilient enough to be trusted.

# Applicability
## C++23

One idea is to extend the C++20 contracts for declaring the return types as part of the C++20 contracts vocabulary.

```cpp
// contract attribute
// "returns"
// example:
using namespace std;
// contract declared val 
// and stat types 
using status = char const *;
double sqrt(double x) 
[[returns: int, status]]
;
```
Above contract requires the valstat calling pattern
```cpp
// as per contract
auto [val,stat] = sqrt(42);

// cover all the four states
// of return:
// INFO, OK, ERR, EMPTY
if ( val )
   printf("\nValue: %d", *val);
else 
   printf("\nError!" );

if (stat)
   printf("\nStatus: %s", *stat);
else
   printf"\nOK return");   

```

## C 2.x

Today C functions can return structures. Those structures can be made in C code and consumed in C++ by structured binding, too.

```cpp
/* valstat_sqrt.h */
extern "C" {
/* C 1.x */
typedef struct sqrt_valstat {
   int * value;
   const char * status ;
} sqrt_valstat ;

/* declaration */
sqrt_valstat sqrt (int);
}
```
Usage is exactly the same as the previous `sqrt` C++ example.

Author might be so (incredibly) bold to think, current C/C++ unified error handling proposals ( one example: 
W21 N2429 ["function editions"](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2429.pdf) ) above code covers most, if not all, of the uses cases for both C++ 2.x and C 2.x.

## Distributed Systems

This architectural pattern is one domain where valstat might be useful and used with no much effort. Even in different languages, the same core principles can be preserved.

Think of micro services. Cluster of micro services are orchestrated to make applications. 

On the cluster level request sending and status feedback is reported usually (perhaps: always) through some messaging. 

valstat status if made to be "just" a JSON formated string. De facto, lingua franca of interoperability. Universally acceptable and usable. Even between components developed in different languages.

[Dusan B. Jovanovic](https://dusanjovanovic.org) 

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