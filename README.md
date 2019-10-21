# dbj++nanolib&trade; 
&copy; 2019 by dbj.org

Very opinionated and very small ISO C++ library.I do hope you might find it useful too. 

dbj nano lib, is not portable. Yet. I develop primarily on Windows 10 PRO using Visual Studio, the latest.

## The Purpose & Motivation
However small they might be, C++ programs should always promote good design, good implementation and reusability.

Also, good solutions should not "get in the way". They should be small and transparent and comfortable to use, beside being very useful too.

## Modern C++

I am actively developing in C++ since 1992. C++ is a powerful beast. One is never sure it is tamed properly. Never turn your back to C++. The weeds of over engineering will very quickly stop your project and drag it to the bottom.

Part of the required self discipline is to develop and use mature core concepts one has proven to work.  But why not just using `std::` lib? 

I do. But my key concepts are not prominent in there. Part of my responsibilities is to make sure organizations is adhering to rigour and method. Not just in Architecture (my primary role), but in developing what is architected too.

But in doing so I know people are good and creative, thus I do not want to impose a lot of discipline, rigour and method. Thus my coding solution tend to be more of an agreement than development. 

Modern, fancy keyword is "contract". dbj++ Nanolib is primarily a tiny universal contract, I might advise you to impose on your projects.

### Top Level Library Organization

The top level dependencies with some descriptions.

![Current dbj++nanolib](https://yuml.me/0b59568c.jpg)

Please keep in mind however "nano" (aka tiny) we wanted it to be, in future sequence of releases dbj++ Nano lib will grow. Hopefully not match.

### How to use

`dbj++nanolib` is C++17, Windows only library. It uses always the latest Visual Studio reincarnation. This is header only library.

As per diagram above, one can include `dbj++nanolib.h` for the core functionality.

Or just `dbj++tu.h` for the testing framework, or just `dbj++valstat` to start using `valstat` and two valstat traits from there.

**Slight detour: Use subset of UML**

Whatever you do, alwayas plan and think about it. Use simple diagrams before deciding your design is complete and you can proceed onto the implementation.

Designs and diagrams are invaluable. First for explaining to yourself your solution, and second to make others understand your solution, before you loose the patience `¯\_(ツ)_/¯`

## What's inside

I see no point of generating documentation from code, if code is available. I tend to write a lot of comments,if there is a need.

Please just dive in and enjoy. At last this is made for you, not for someone needing a book to understand how to use the modern C++ library.

## what is 'dbj++nano play'

That is a twin project. We use it to test the quality of implementation and usability of the nanolib.

Newcomers to `dbj++nanolib` should definitely start from the as the entry point. It will contain more and more test to show how are the things from here to be used.

Next I will just list and quickly describe, in random order, important things in the Nanolib. As per your demand this documentation will be more detailed as we progress.

Do not forget code here should be simple and headers manageable.

## dbj++nanolib.h

The library core header. Namespace `dbj::nanolib`.

#### configuration

`#define DBJ_TERMINATE_ON_BAD_ALLOC` in order not to throw bad_alloc, on heap memory exhausted. Instead, default termination will be called.
```cpp
  (std::set_new_handler([]{terminate();}););
```
For safe computing experiene, `#define DBJ_SYNC_WITH_STDIO` in case of mixing `stdio.h` and `iostreams`.
```cpp
			ios_base::sync_with_stdio(true);
```

#### Macros

- usual macros
   - macros are named starting with `DBJ...` or `_DBJ...`
 - `DBJ_REPEAT(N)` is interesting, there is no "do something N times" C++ keyword
 - the namespace is `dbj::nanolib`
 - there is `ios_base::sync_with_stdio(true);` line in there. Find it and replace true with false if you think you need to. For example if not building console programs.
 - most peculiar little thingy in there is `v_buffer` because we do not want to use `std::string` in core libraries (see Appendix A)
   - we use exclusively `std::vector<char>` as the char buffer in dbj++nanolib
   - `v_buffer` is actually a trait made to provide the required functionality for handling the `v_buffer::buffer_type`
  - Another peculiarity worth mentioning is we never use `printf`/`fprintf` naked but through  two variadic macros
    - DBJ_FPRINTF(...)
    - DBJ_PRINT(...)
    - Primarily because debug build versions do contain checking the returns of `std::fprintf`. If you have  been (and I was) ever beaten by not checking `stdio.h` functions, return values you will understand.  Invaluable even if it works only once for you.

## dbj++tu.h

Fully functional Testing framework. namespace `dbj::tu`

It works as one might expect C++ Testing Frameworks to work. You register the "Test Unit". It gets collected at compile time and executed at runtime.

Advisable to start is to look at the bottom of the `dbj++valstat` header where one test resides. This is where you want to put your Visual Studio break point and hit F5, while in the [nano play project](https://github.com/dbj-systems/dbj--nanoplay).
```cpp
TU_REGISTER(
	[] {
    // the rest of the code
  });
```

Macro `DBJ_TX` is indeed using `iostreams`. That is ok in the context of testing apps. Makes for simpler and shorter testing macro. 

Lastly, to run the test units collected, on has to call, somewhere from main():

`dbj::tu::catalog.execute()`

As expected for every decent testing framework, the coloured console output pops-up, and every TU is timed too.

![tu in action](docs/dbj_tu_in_action.jpg)

# Appendix A
## Key concepts

This is the contracts I am advising you to respect.

### No sub-classing
- inheritance in C++ is used for
  - [Sub-Typing](https://en.wikipedia.org/wiki/Subtyping)
  - Sub-Classing
    - Sub-Classing is evil
- [Polymorphism does not require inheritance](https://en.wikipedia.org/wiki/Composition_over_inheritance)
  - Sub-classing inside the c++ std lib is very rare

### Minimise the string usage

- `std::string` is very versatile, but it is **not** made to be used as char buffer. 
  - it makes for one too complex type and  large memory structure, to act as a simple char buffer
  - `std::string` operator '+' is notoriously slow, thus people try and use the `append()` method, complicating the code.
  - it is probably more elegant and faster to do `sprintf` into the buffer, instead of using `std::string` append
  ```cpp
  // using std::string to compose a new string
  // need to use append() to avoid '+'
  std::string new_val(  reader->kv_map_[key].data()  );
  new_val.append("\n").append(value);

  // instead dbj nano lib offers this an elegant solution
  	buffer_type new_val = buffer::format( "%s%c%s",
		reader->kv_map_[key].data() , '\n', value
	);
  ```
  Unless you are developing large and complex text processing, be very modest with `std::string`.
### No throwing and catching

First, if your whatever, is intended to operate with continuous uptime. It should never fail and never exhibit undefined behavior. Thus, the error handling is of utmost importance. It has to be very explicit and unforgiving.

- throwing and catching do create slow applications
- the raising of the error and handling it should be tightly coupled and at the same place in the source code. Make it easy to understand what happens if error happens.
  - if you need to throw an exception from a constructor, your design is probably wrong.
  - if your constructor is complex and might be source of operational error's consider static factory functions or friends, whatever you fancy most.
  - however silly that looks to a C++ developer consider removing tricky destructors into static or friend functions to be called by some external deterministic mechanism, similar to the lock unlock pattern.

### Avoid system error
- `std::system_error` is old, over-engineered design and somewhat poor implementation
- `std::system_error` was not designed to be universal error mechanism for C++ `std::` lib. Neither it was ever adopted as such.
- it is not a replacement but consider using `std::errc` perhaps with `std::generic_category()`
- win32 errors are well implemented by MSVC `std::system_category()`

### Avoid c++ streams
- C++ streams is old, complicated design too. 
- using iostreams creates large and slow applications
- if you need to do a lot of console output consider using the [fmt library](https://github.com/fmtlib/fmt).
- Although I personally am puzzled how is it better than `cstdio` functions

### Standards
- use ISO C++, which is C++17 as of 2019 Q4.
- use ISO C++ std lib as much as possible
- Do not lose your C experience. 
  - C is simple and makes you deliver things, instead of re-inventing things before delivering things. 
   - take same time to learn about [modern ISO C](https://gforge.inria.fr/frs/?group_id=6881). It has advanced. A lot.

**This is "nano" lib. Before adding anything into it, think.**

## Contact

In case you hotly dislike or hotly like my work, or anything in between, for what you might find in here, please do write me:

[dbj@dbj.org](mailto:dbj@dbj.org)

## License

### CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ 

---
[![dbjdbj github avatar](https://github.com/dbjdbj.png)
 &copy; 2019](https://dbj.netlify.com)


