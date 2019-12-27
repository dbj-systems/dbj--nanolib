#ifndef DBJ_STACK_ALLOCATOR_INC_
#define DBJ_STACK_ALLOCATOR_INC_

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
				//if (!pointer_in_buffer(ptr_)) {
				//    perror("stack_allocator has outlived stack_arena");
				//        exit(EXIT_FAILURE);
				//}
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

			void deallocate(buf_element_type* p, std::size_t n) noexcept
			{
				// return; // oops ;)

				if (pointer_in_buffer(p))
				{
					n = align_up(n);
					if (p + n == ptr_)
						ptr_ = p;
				}
				else {
					// dbj::nanolib::dbj_terror("stack_allocator arrena deallocate has been given pointer it does not own.", __FILE__, __LINE__);
					perror("stack_allocator arrena deallocate has been given a pointer it does not own.");
					exit(EXIT_FAILURE);
				}
			}
		}; // stack_arena

	} // detail ns

	template <class T, size_t size_template_arg >
	struct stack_allocator final : std::allocator<T>
	{
		using type = stack_allocator;
		using arena_type = detail::stack_arena<size_template_arg>;

		constexpr static size_t size_{ size_template_arg };

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
