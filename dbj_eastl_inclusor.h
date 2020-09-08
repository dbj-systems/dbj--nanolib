#pragma once
#ifndef DBJ_EASTL_INCLUSOR_INC
#define DBJ_EASTL_INCLUSOR_INC

#include "dbj_heap_alloc.h"

// dbj tweaked EASTL2010 requires these functions to be user defined

namespace eastl {
	extern "C" {
		/////////////////////////////////////////////////////////////////////////////
		// EASTL expects us to define these, see allocator.h 
		inline void* user_defined_alloc(
			size_t size_,
			const char* pName,
			int flags,
			unsigned debugFlags,
			const char* file,
			int line
		) noexcept
		{
			// note: no checks whatsoever ...
			return DBJ_NANO_MALLOC(void, size_);
		}

		// alligned allocation
		inline void* user_defined_alloc_aligned(
			size_t size_,
			size_t alignment_,
			size_t alignmentOffset,
			const char* pName,
			int flags,
			unsigned debugFlags,
			const char* file,
			int line
		) noexcept
		{
			// this allocator doesn't support alignment
			EASTL_ASSERT(alignment_ <= 8);
			// note: no checks whatsoever ...
			return DBJ_NANO_MALLOC(void, size_);
		}

		// dbj eastl2010 change to allow for default allocator
		// to use matching de allocation / allocation
		// just like in this case
		inline void user_defined_deallocate(void* ptr_) noexcept
		{
			DBJ_NANO_FREE(ptr_);
		}
	} // "C"
} // eastl ns


// EASTL also wants us to define this (see string.h line 197)
extern "C" inline int Vsnprintf8(char* pDestination, size_t count_,
	const /*char8_t*/ char* pFormat, va_list arguments) {
#ifdef _MSC_VER
	// return _vsnprintf(pDestination, count_ , pFormat, arguments);
	return _vsnprintf_s(pDestination, strlen(pDestination), count_, pFormat, arguments);
#else
	return vsnprintf(pDestination, count_, pFormat, arguments);
#endif
}


#endif // !DBJ_EASTL_INCLUSOR_INC

