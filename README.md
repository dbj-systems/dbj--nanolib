# dbj++nanolib&trade; 
### CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ 
&copy; 2019 by dbj.org

Very opinionated and very small ISO C++ library.I do hope you might find it useful too. 

dbj nano lib, is not portable. Yet. I develop primarily on Windows 10 PRO using Visual Studio, the latest.

## The Purpose & Motivation
However small they might be, C++ programs should always promote good design, good implementation and reusability.

Also, reusable solutions should not "get in the way". They should be small and transparent and comfortable to use, beside being very useful too.
#### Slight detour: Use subset of UML
Whatever you do, alwyas plan and think about it. Use diagrams before deciding your design is complete and you can proceed onto the implementation.

![Current dbj++nanolib](https://yuml.me/ab2e4249.jpg)

Desings and [diagrams](http://yuml.me/edit/ab2e4249) are invaluable. First for explaining to yourself your solution, and second to make others understand your solution, before you loose the patience `¯\_(ツ)_/¯`
## Implementation concepts
### No sub-classing
- inheritance in C++ is used for
  - [Sub-Typing](https://en.wikipedia.org/wiki/Subtyping)
  - Sub-Classing
    - Sub-Classing is evil
- [Polymorphism does not require inheritance](https://en.wikipedia.org/wiki/Composition_over_inheritance)
  - Sub-classing inside the c++ std lib is very rare

### Minimise the string usage
- `std::string` is not made to be used as char buffer. 
  - it makes for one complex type and  large memory structure to act as a simple char buffer
  - `std::string` operator '+' is notoriously slow, thus people use the append method
  - it is more elegant and faster to do `sprintf` into the buffer, instead of using `std::string` append
  ```cpp
  // using std::string to compose a new string
  // need to use append() to avoid '+'
  std::string new_val(  reader->kv_map_[key].data()  );
  new_val.append("\n").append(value);

  // instead dbj nano lib makes this an frequent and elegant idiom
  	buffer_type new_val = buffer::format( "%s%c%s",
		reader->kv_map_[key].data() , '\n', value
	);
  ```
### No throwing and catching

- throwing and catching do create slow applications
  - if you need to throw an exception from a constructor, yuor design is probably wrong.

### Avoid system error
- `std::system_error` is old, over-engineered design and somewhat poor implementation
- it is not a replacement but consider using `std::errc`

### Avoid c++ streams
- C++ streams is old, complicated design too. 
- using iostreams creates slow applications

### Standards
- use ISO C++, which is C++17 as of 2019 Q4.
- use ISO C++ std lib as much as possible
- Do not lose your C experience. 
  - C is simple and forces you to deliver things, instead of re-inventing things before delivering things. 
   - take same time to learn about modern ISO C

**This is "nano" lib. Before adding anything into it, think.**



