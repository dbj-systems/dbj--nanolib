# dbj++nanolib&trade; 
### CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ 
&copy; 2019 by dbj.org

Very opinionated and very small ISO C++ library.I do hope you might find it usefull too. 

It is not portable, yet. I develop primarily on Windows 10 PRO using Visual Studio, the latest.

## The Purpose & Motivation
However small they might be, your C++ programs should always promote good design, good implementation and reusability.

Also, you reusable solutions should not "get in the way". They should be small and transparent and comfortable to use, beside being very usefull too.

#### Use subset of UML
Whatever you do, alwyas plan and think about it. Use diagrams before deciding your design is complete and you can proceed onto the implementation.

![Current dbj++nanolib](https://yuml.me/ab2e4249.jpg)

Desings and [diagrams](http://yuml.me/edit/ab2e4249) are invaluable. First for explaining to yourself your solution, and second to make others understand your solution, before you lose the patience.

## Implementation concepts
- inheritance in C++ is used for
  - [Sub-Typing](https://en.wikipedia.org/wiki/Subtyping)
  - Sub-Classing
    - Sub-Classing is evil
- Polymorphism does not require inheritance
  - Inheritance inside the c++ std lib is very rare
- `std::string` is not made to be used as char buffer. 
  - it makes for one very slow char buffer
- exceptions create slow applications
  - if you need to throw an exception from a constructor, yuor design is probably wrong.
- `std::system_error` is old, over-complicated design and somewhat poor implementation
  - it is not a replacement but consider using `std::errc`
- C++ streams is old, complicated design too. 
   - using iostreams creates slow applications
- use ISO C++, which is C++17 as of 2019 Q4.
  - use ISO C++ std lib as much as possible
  - Do not lose your C experience. 
    - C is simple and forces you to deliver things, instead of re-inventing things before delivering things. 
    - take same time to learn about modern ISO C

**This is "nano" lib. Before adding anything into it, think.**



