#ifndef DBJ_ALIGNED_ALLOCATOR_INC_
#define DBJ_ALIGNED_ALLOCATOR_INC_

#ifdef _WIN32
#include <malloc.h>
#endif
#include <cstdint>
#include <vector>

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

// redefine this to return instead of exit() if required
#define DBJ_ALLIGNED_ALLOCATOR_FAIL_POLICY( MSG_) \
perror( " (" __FILE__ ") " MSG_ ); \
exit(EXIT_FAILURE);

namespace dbj::nanolib::alloc
{

	/**
	 * Allocator for aligned data.
	*  https://gist.github.com/donny-dont/1471329
	 */
	template <typename T, std::size_t Alignment>
	struct aligned_allocator
		final
		// DBJ -- G++ will not work if not inheriting from 
		: std::allocator<T>
	{
	public:

#if 0
		// The following will be the same for virtually all allocators.
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		typedef std::size_t size_type;
		typedef ptrdiff_t difference_type;
#endif // 0
		// this is not part of standard allocator requirements
		static std::size_t max_size()
		{
			// The following has been carefully written to be independent of
			// the definition of size_t and to avoid signed/unsigned warnings.
			return (static_cast<std::size_t>(0) - static_cast<std::size_t>(1)) / sizeof(T);
		}

		// DBJ NOTE: as of 2019 DEC 26, VStudio 2019 fully updated
		// err's wit compilation message: vector end of file not found
		// if rebind is not defined as bellow
		template <typename U>
		struct rebind
		{
			typedef aligned_allocator<U, Alignment> other;
		};

		// DBJ NOTE: as of 2019 DEC 26, VStudio 2019 fully updated
		// err's with compilation message: default ctor not found
		// if it is not defined as bellow
		aligned_allocator() { }

		// DBJ NOTE: as of 2019 DEC 26, VStudio 2019 fully updated
		// err's with complex compilation message
		// if the following rebinding ctor is not defined as bellow
		template <typename U> aligned_allocator(const aligned_allocator<U, Alignment>&) { }

		// The following will be different for each allocator.
		T* allocate(const std::size_t n) const
		{
			// The return value of allocate(0) is unspecified.
			// We return NULL in order to avoid depending
			// on malloc(0)'s implementation-defined behavior
			// (the implementation can define malloc(0) to return NULL,
			// in which case the bad_alloc check below would fire).
			// All allocators can return NULL in this case.
			if (n == 0) {
				return NULL;
			}

			// All allocators should contain an integer overflow check.
			// The Standardization Committee recommends that std::length_error
			// be thrown in the case of integer overflow.
			// Alas we do not throw; we will just calmly exit the app
			if (n > max_size())
			{
				DBJ_ALLIGNED_ALLOCATOR_FAIL_POLICY("aligned_allocator<T>::allocate() - Integer overflow.");
			}

			// should be defined universaly, across C and C++ compilers/libs
#ifndef _mm_malloc
#error  _mm_malloc undefined?
#endif
			void* const pv = _mm_malloc(n * sizeof(T), Alignment);

			// Allocators should throw std::bad_alloc in the case of memory allocation failure.
			// alas DBJDBJ does not throw, so he will just calmly exit the app
			if (pv == NULL)
			{
				DBJ_ALLIGNED_ALLOCATOR_FAIL_POLICY("aligned_allocator<T>::allocate() - memory allocation failure");
			}

			return static_cast<T*>(pv);
		}

		void deallocate(T* const p, const std::size_t n) const
		{
			// should be defined universaly, across C and C++ compilers/libs
#ifndef _mm_free
#error  _mm_free undefined?
#endif
			_mm_free(p);
		}


	private:
		// Allocators are not required to be assignable, so
		// all allocators should have a private unimplemented
		// assignment operator. Note that this will trigger the
		// off-by-default (enabled under /Wall) warning C4626
		// "assignment operator could not be generated because a
		// base class assignment operator is inaccessible" within
		// the STL headers, but that warning is useless.
		aligned_allocator& operator=(const aligned_allocator&) = delete ;

	}; // aligned_allocator

} // namespace dbj::nanolib::alloc 

#undef DBJ_ALLIGNED_ALLOCATOR_FAIL_POLICY

#endif // DBJ_ALIGNED_ALLOCATOR_INC_