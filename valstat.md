# VALSTAT
## Value & Status
(c) 2019 by dbj.org -- [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)

## New return paradigm

Three primary objectives

1. Can not wait C++23 and [P0709 realization](https://herbsutter.com/2018/07/02/trip-report-summer-iso-c-standards-meeting-rapperswil/) 
2. Make a simple, universally applicable solution, now.
3. Achieve maximum with minimum 

They are always coming as trios. After much more than expected,  experimenting and testing, this is my architecture of the solution. With reasoning behind.

#### "Error Handling" becomes Light and Useful

Error handling was a wrong name. Not every return denotes an error. In this solution, an value and status are returned. And, the **"and"** is the key word. Not "or".

Here is the simple standard C++ code, actually explaining it all.

```cpp
using namespace std ;

/* abstract generic structure, with no apparent solution domain */
template <typename T1_, typename T2_>
using pair_of_options = std::pair<optional<T1_>, optional<T2_>>;
/* 
here is the solution domain for the above 
return optional value and optional status

Not much is developed, it is all just using the std:: lib. Not much can go wrong.
*/
template<typename T>
using valstat = pair_of_options<T, string > ;

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
        /* structured binding is the preffered way to consume value and status */
        auto [ val, stat ] = my_fun(arg_);
    
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
An destillation of many weeks of work. Very simple and logical. No macros. Just standard C++. 

## Key concept is: Value AND Status
Thinking about and solving the architecture of return types, I have came to 
the conscious and key conceptual conclusion: value AND status, not value OR error. Error is just one of the states (a condition) of the return event. 

Error is a wrong name. Status is the right name for what might be returned, with optional value. Both absence and presence, of both value and state, gives the logic, the meaning for the consumers aka callers.

 In the core structure, both Value and Status are optional. They might be or might not be present in the structure returned. This renders four (4) posible states at the consuming site.

1. FATAL  
    1. If both value AND status are empty that is an fatal error
2. INFO 
    1. if value AND status are not empty that is info state.
3. OK 
   1. Just value is returned 
4. ERROR 
   1. Just status is returned

Does this mean we have to check always, for all four in using this paradigm? I think not.

FATAL state we might take care of checking in debug builds only. INFO, OK or ERROR consuimg  depends on the consumers logic, on the conext.

As an example, consider consuming HTTP codes.
```cpp
// declaration
valstat<http_code> http_get ( uri );
// consuming site
auto [ val, stat ] = http_get("...") ;
   // debug builds check for the FATAL state
   // bith can't be empty in the same time
   assert( val || stat ) ;
    if ( ! val ) return ;
/* all the results are in the INFO state , both value and staus are present as described by HTTP protocol */
if ( val = http_code(200)) { /* the request was fulfilled */  }
if ( val = http_code(203)) { /* partial information */  }
if ( val = http_code(400)) { /* bad request */  }
```
I any of the cases above, status returned will be used to pass information we need to decide on the logic of next htpp call.
## Conclusion

Probably, all the solutions up till now are based on the "value OR error" concept, 
most often implemented using the union type. Sometime using the [discriminated union](https://pdfs.semanticscholar.org/0a8c/2e0f3a194b15970472dca07c37c2172b69fb.pdf) type, a.k.a variant. 

I might be so bold to thin they are mostly over-engineered. I do not implement things (at least in this instance). I simply use the types from the std:: lib.