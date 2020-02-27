#ifndef DBJ_STACK_ALLOCATOR_INC_
#define DBJ_STACK_ALLOCATOR_INC_

/*
 NOTE:

 stack allocator is very fast and very dangerous and has very limited
 number of uses cases where it can be used.

 Basically it is fast because it does not use heap at all. It will exit() if
 some container using it asks beyond avaialbe stack space.

 It does not throw exceptions.
 It does not free what was taken.
 On stack exhaustion your app will very likely just vanish. Especially in a release mode.

 Ok, what's the point then?

 The point is you want a std lib container API + you know how large it will be
 + you know it will not grow over certain limit.  For example vector<char> is
 one very fast char buffer and has familiar and convenient API

 using namespace dbj::nanolib::alloc ;
 std::vector<char, stack_allocator< char > > my_buffer( 1024, '+') ;

 Above is very fast anyway, but with stack allocator it is instant.
 Ok, says you, but why not just using:

 char my_buffer[1024] { 0 };

 Because you might use the libraries which do require std::vector, or std::string, etc..
 Also do not forget there are project which are forbidden to use heap!
 If you place such a vector in a global space it will stay as long as the app stays.

*/

#ifndef DBJ_CPLUSPLUS
#if defined(_MSVC_LANG) && !defined(__clang__)
#define DBJ_CPLUSPLUS (_MSC_VER == 1900 ? 201103L : _MSVC_LANG)
#else
#define DBJ_CPLUSPLUS __cplusplus
#endif
#endif

#if ! (DBJ_CPLUSPLUS > 201402L )
#error C++17 or greater is required ...
#endif

// MSVC STD LIB aka STL
// _ITERATOR_DEBUG_LEVEL 2 is the DEBUG build default
// and that will produce segv with dbj stack alocator 
// that was not tested with stack allocator I suppose?
// https://docs.microsoft.com/en-us/cpp/standard-library/iterator-debug-level?view=vs-2019

#if (_ITERATOR_DEBUG_LEVEL != 0)
#undef _ITERATOR_DEBUG_LEVEL
#define _ITERATOR_DEBUG_LEVEL  0
#endif

// https://codereview.stackexchange.com/a/31575

#include <cstddef>
#include <cassert>
#include <vector>
#include <new>

namespace dbj::nanolib::alloc
{

/// G++ REQUIRES allocators to inherit from std::allocator

	template <class T>
	struct stack_allocator
		final
		: std::allocator<T>
	{

		using type = stack_allocator;

		// DBJ NOTE: as of 2019 DEC 26, VStudio 2019 fully updated
		// err's with compilation message: vector end of file not found
		// if rebind is not defined as bellow
		template <typename U>
		struct rebind
		{
			typedef stack_allocator<U> other;
		};

		// The following has been carefully written to be independent of
		// the definition of size_t and to avoid signed/unsigned warnings.
		std::size_t max_size() const
		{
			return (static_cast<std::size_t>(0) - static_cast<std::size_t>(1)) / sizeof(T);
		}

		// DBJ NOTE: as of 2019 DEC 26, VStudio 2019 fully updated
		// err's with compilation message: default ctor not found
		// if it is not defined as bellow
		stack_allocator() { }

		// DBJ NOTE: as of 2019 DEC 26, VStudio 2019 fully updated
		// err's with complex compilation message
		// if the following rebinding ctor is not defined as bellow
		template <typename U> stack_allocator(const stack_allocator<U> &) { }

		// without these two
		// allocator traits will not work
		// get_allocator() too
		using parent = std::allocator<T>;
		using parent::parent;

		/// https://www.gnu.org/software/libc/manual/html_node/Advantages-of-Alloca.html
		/// https://www.gnu.org/software/libc/manual/html_node/Disadvantages-of-Alloca.html#Disadvantages-of-Alloca

		T* allocate(std::size_t n) noexcept
		{
			if (n == 0) {
				return nullptr;
			}
			return static_cast<T*>(alloca(n * sizeof(T)));
		}
		void deallocate(T* /*p*/, std::size_t /*n*/) noexcept
		{
			/* do not free alloca results! */
		}

	}; // stack_allocator

} // namespace dbj::nanolib::alloc  

#endif // DBJ_STACK_ALLOCATOR_INC_