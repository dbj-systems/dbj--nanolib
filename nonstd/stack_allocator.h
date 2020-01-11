#ifndef DBJ_STACK_ALLOCATOR_INC_
#define DBJ_STACK_ALLOCATOR_INC_

/*
 NOTE:

 stack allocator is very fast and very dangerous and has very limited
 number of uses cases where it can be used.

 Basically it is fast because it does not use heap at all. It will exit() if
 some container uysing it asks beyond avaialbe internal array.

 It does not throw exceptions.

 It does not free what was taken.

 On stack exhaustion your app will very likely just vanish. Especially in a release mode.

 Ok, what's the point than?

 The point is you want a std lib container API + you know how large it will be
 + you know it will not grow over certain limit.  For example vectorchar> is
 one very fast char buffer and has familiar and convenient API

 using namespace dbj::nanolib::alloc ;
 std::vector<char, stack_allocator< char, 2048> > my_buffer( 1024, '+') ;

 Above is very fast anyway, but with stack allocator it is instant.
 Ok, says you, but why not just using:

 char my_buffer[2048] { 0 };

 Because you might use the libraries which do require std::vector, or std::string, etc..

 Also do not forget there are project which are forbidden to use heap!

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
	constexpr static const size_t MAX_STACK_ARENA{ 0xFFFF };

	namespace detail {
		/// <summary>
		/// stack memory arena
		/// adapted by dbjdbj from Howard Hinant
		/// </summary>
		template <std::size_t SIZE_>
		class stack_arena final
		{
		public:

			// constexpr std::size_t alignment = alignof(std::max_align_t);
			// same as
			using buf_element_type = unsigned char;
			constexpr static std::size_t alignment = alignof(buf_element_type);
			using type = stack_arena;

			static_assert(SIZE_ < MAX_STACK_ARENA, "max stack_arena size is 64kb");
		private:

			alignas(stack_arena::alignment) buf_element_type buf_[SIZE_]{ 0 };
			const buf_element_type* one_beyond_last_{ buf_ + SIZE_ };

			buf_element_type* ptr_{ buf_ };

			std::size_t align_up(std::size_t n) const noexcept
			{
				return (n + (alignment - 1)) & ~(alignment - 1);
			}

			// probably redundant?
			bool pointer_in_buffer(buf_element_type* p) const noexcept
			{
				if (buf_ <= p)
					if (p < one_beyond_last_)
						return true;
				return false;
			}

		public:

			static std::size_t size() noexcept { return SIZE_; }
			std::size_t used() const noexcept { return static_cast<std::size_t>(ptr_ - buf_); }
			void reset() { ptr_ = buf_; }

			buf_element_type* allocate(std::size_t n)
			{
				n = align_up(n);
				if (size_t(one_beyond_last_ - ptr_) >= n)
				{
					// buf_element_type* r = ptr_;
					ptr_ += n;
					return ptr_;
				}

				// dbj::nanolib::dbj_terror("stack_allocator arrena has no enough memory", __FILE__, __LINE__);
				perror("stack_allocator arrena has no enough memory: ");
				exit(EXIT_FAILURE);
				return nullptr;
			}

			void deallocate(buf_element_type*, std::size_t) noexcept
			{
				return;
			}
		}; // stack_arena

	} // detail ns

// WARNING! G++ REQUIRES allocators to inherit from std::allocator
#define DBJ_ALLOCATOR_INHERITS_FROM_STD_ALLOCATOR

	template <class T, size_t size_template_arg >
	struct stack_allocator
#ifndef DBJ_ALLOCATOR_INHERITS_FROM_STD_ALLOCATOR
		final
#else
		// DBJ -- G++ will not work id not inheriting from 
		: std::allocator<T>
#endif // DBJ_ALLOCATOR_INHERITS_FROM_STD_ALLOCATOR
	{

		using type = stack_allocator;
		using arena_type = detail::stack_arena<size_template_arg>;

		constexpr static size_t capacity_{ size_template_arg };

#ifndef DBJ_ALLOCATOR_INHERITS_FROM_STD_ALLOCATOR
		// The following is the same for all allocators.
		// most STL implementations will require this 
		// full complement
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		typedef std::size_t size_type;
		typedef ptrdiff_t difference_type;
#endif // DBJ_ALLOCATOR_INHERITS_FROM_STD_ALLOCATOR

		// DBJ NOTE: as of 2019 DEC 26, VStudio 2019 fully updated
		// err's wit compilation message: vector end of file not found
		// if rebind is not defined as bellow
		template <typename U>
		struct rebind
		{
			typedef stack_allocator<U, size_template_arg> other;
		};

		std::size_t max_size() const
		{
			// The following has been carefully written to be independent of
			// the definition of size_t and to avoid signed/unsigned warnings.
			return (static_cast<std::size_t>(0) - static_cast<std::size_t>(1)) / sizeof(T);
		}

		// DBJ NOTE: as of 2019 DEC 26, VStudio 2019 fully updated
		// err's with compilation message: default ctor not found
		// if it is not defined as bellow
		stack_allocator() { }

		// DBJ NOTE: as of 2019 DEC 26, VStudio 2019 fully updated
		// err's with complex compilation message
		// if the following rebinding ctor is not defined as bellow
		template <typename U> stack_allocator(const stack_allocator<U, size_template_arg>&) { }

		// without these two
		// allocator traits will not work
		// get_allocator() too
#ifdef DBJ_ALLOCATOR_INHERITS_FROM_STD_ALLOCATOR
		using parent = std::allocator<T>;
		using parent::parent;
#endif

		T* allocate(std::size_t n) noexcept
		{
			if (n == 0) {
				return nullptr;
			}
			return reinterpret_cast<T*>(arena_.allocate(n * sizeof(T)));
		}
		void deallocate(T* p, std::size_t n) noexcept
		{
			arena_.deallocate(reinterpret_cast<typename arena_type::buf_element_type*>(p), n * sizeof(T));
		}

	private:
		arena_type arena_;
	}; // stack_allocator


	// very fast and very dangerous
	template< template< class T_, class A_> typename container_type, typename value_type, size_t stack_alloc_size = 0xFFF,
		typename stack_alloc_type = stack_allocator<value_type, stack_alloc_size> >
		using small_container = container_type< value_type, stack_alloc_type >;

} // namespace dbj::nanolib::alloc  

#endif // DBJ_STACK_ALLOCATOR_INC_