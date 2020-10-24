#pragma once
#ifndef DBJ_MEMALIGNED_INC
#define DBJ_MEMALIGNED_INC

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#ifdef _MSC_VER
#include <iso646.h>
#endif

namespace dbj {

// https://tinyurl.com/y5zhw5x3
// in english: 8
// malloc aligns on this boundary
constexpr auto default_alignment = sizeof(double);

#define mistery _HEAP_MAXREQ
	/// ----------------------------------------------------------------------

	// https://tinyurl.com/y5zhw5x3
	// in english: 8
	// malloc aligns on this boundary
	constexpr auto default_alignment = sizeof(double);
	/**
	* Machine word size. Depending on the architecture,
	* can be 4 or 8 bytes.
	*/
	using word_t = intptr_t;
	/// ----------------------------------------------------------------------
	/**
	 * Aligns the size by the machine word.
	 http://dmitrysoshnikov.com/compilers/writing-a-memory-allocator/#memory-alignment
	 */
	constexpr inline size_t align(size_t n) noexcept {
		return (n + sizeof(word_t) - 1) & ~(sizeof(word_t) - 1);
	}

	/// ----------------------------------------------------------------------
	inline bool is_aligned(void *ptr, size_t alignment) noexcept {
		if (((unsigned long long)ptr % alignment) == 0)
			return true;
		return false;
	}

	/// ----------------------------------------------------------------------
	[[deprecated]]
	inline void* aligned_malloc(size_t alignment, size_t size) noexcept
	{
		void* p;
#ifdef _MSC_VER
		p = _aligned_malloc(size, alignment);
#elif defined(__MINGW32__) || defined(__MINGW64__)
		p = __mingw_aligned_malloc(size, alignment);
#else
		// somehow, if this is used before including "x86intrin.h", it creates an
		// implicit defined warning.
		if (posix_memalign(&p, alignment, size) != 0) {
			return nullptr;
		}
#endif
		return p;
	}

	/// ----------------------------------------------------------------------
	[[deprecated]]
	inline void aligned_free(void* mem_block) noexcept {
		if (mem_block == nullptr) {
			return;
		}
#ifdef _MSC_VER
		_aligned_free(mem_block);
#elif defined(__MINGW32__) || defined(__MINGW64__)
		__mingw_aligned_free(mem_block);
#else
		free(mem_block);
#endif
	}


// NOTE! there is not MT resilience in here!
namespace detail {

	//  int posix_memalign(void **memptr, size_t alignment, size_t size);
	// we use Strong types concept
	typedef struct Alignment { size_t val; } Alignment;
	typedef struct Size { size_t val; } Size;

	// corecrt_malloc.h contains _aligned_malloc
	// since we use string types as arguments nobody can not make 
	// a very common mistake of swapping alignment and size
	inline int dbj_alloc_aligned(void** p, Alignment alignment_, Size size_) noexcept
	{
		int erc = 0;
		_set_errno(0);

#ifdef _WIN32
		* p = _aligned_malloc(size_.val, alignment_.val);
		erc = errno;
#else  // ! _WIN32
		erc = posix_memalign(p, size_.val, alignment_.val);
#endif // ! _WIN32
		return erc;
	}

	inline int dbj_free(void* p) noexcept
	{
		int erc = 0;
		_set_errno(0);
#ifdef _WIN32
		_aligned_free(p);
		erc = errno;
#else  // ! _WIN32
		free(p);
		erc = errno;
#endif // ! _WIN32
		p = NULL;
		return erc;
	}
} // detail ns

#ifdef TEST_MALLOC_ALIGNED

inline int test_malloc_aligned() noexcept
{
    dbj::local_lock_unlock autolock_;

    printf("\n" VT100_LIGHT_BLUE __FUNCSIG__  VT100_RESET );

    char* mem = NULL;
    // CL does not do "compund literals"
    // so we do string types in a pedestrian way
    // that is we can not do
    // int erc = dbj_alloc_aligned((void**)&mem, (Alignment) { alignment }, (Size) { n });

	using namespace detail;

    Alignment alignment = { 64 };
    Size n = { 100 } ;

    int erc = dbj_alloc_aligned((void**)&mem, alignment , n );

    const size_t wrong_alignment = 13;

    printf("\nis %3zd byte aligned = %s", wrong_alignment, (((size_t)mem) % wrong_alignment) ? "no" : "yes");
    printf("\nis %3zd byte aligned = %s", alignment.val, (((size_t)mem) % alignment.val ) ? "no" : "yes");

    dbj_free(mem);

    return EXIT_SUCCESS ;
}

#endif // TEST_MALLOC_ALIGNED

} // dbj 


#undef mistery

#endif // DBJ_MEMALIGNED_INC
