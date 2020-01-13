// dbj++array
#ifndef DBJ_ARRAY_INCLUDED_
#define DBJ_ARRAY_INCLUDED_

// not yet
// #define DBJ_ARRAY_FULL_IMPLEMENTATION 

// avoid names clashes with std::array
// use these simple macros
#define DBJ_ARRAY ::dbj::nanolib::containers::array
#define DBJ_ARRAY_STORAGE ::dbj::nanolib::containers::array_storage

namespace dbj::nanolib::containers
{
	constexpr size_t MAX_ARRAY_SIZE = 0xFFFF; // 64K == plenty!

template <class TYP_, size_t SZE_ >
struct array
{
	static_assert(SZE_ > 0 ); 
	static_assert(SZE_ < dbj::nanolib::containers::MAX_ARRAY_SIZE );

	static_assert(std::is_nothrow_swappable_v<TYP_> );

public:
	using type = array;
    using value_type = TYP_;
    using size_type = size_t;
    using differencetype = ptrdiff_t;
    using pointer = TYP_ *;
    using const_pointer = const TYP_ *;
    using reference = TYP_ &;
    using const_reference = const TYP_ &;

    using iterator = pointer;
    using const_iterator = const_pointer;

#ifdef DBJ_ARRAY_FULL_IMPLEMENTATION 
    using reverse_iterator = reverse_iterator<iterator>;
    using const_reverse_iterator = reverse_iterator<const_iterator>;
#endif // DBJ_ARRAY_FULL_IMPLEMENTATION 

    void fill(const TYP_ &value_)
    {
        std::fill_n(elements_, SZE_, value_);
    }

	/*
	swap in place

	template <typename T> void swap(T& t1, T& t2) {
	T temp = std::move(t1);
	t1 = std::move(t2);
	t2 = std::move(temp);
}
	*/
    void swap(type & other_arr_) 
    {
		auto swap_two = [] (auto & left_, auto & right_) 
		{
			auto temp_ = std::move( right_ );
			left_ = std::move(right_);
			right_ = std::move(temp_);
		};

		for (size_t idx_ = 0; idx_ < SZE_; idx_++)
			swap_two( (*this).at(idx_), other_arr_.at(idx_)  );
    }

    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return elements_ ;
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return elements_ ;
    }

    [[nodiscard]] constexpr iterator end() noexcept
    {
        return elements_ + SZE_ ;
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return elements_ + SZE_ ;
    }

#ifdef DBJ_ARRAY_FULL_IMPLEMENTATION 

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept
    {
        return (end());
    }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
    {
        return (end());
    }

    [[nodiscard]] constexpr reverse_iterator rend() noexcept
    {
        return (begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
    {
        return (begin());
    }

#endif // DBJ_ARRAY_FULL_IMPLEMENTATION 


    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return end();
    }

#ifdef DBJ_ARRAY_FULL_IMPLEMENTATION 
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
        return rbegin();
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
    {
        return rend();
    }

    constexpr TYP_ *unchecked_begin() noexcept
    {
        return elements_[0];
    }

    constexpr const TYP_ *unchecked_begin() const noexcept
    {
        return elements_[0];
    }

    constexpr TYP_ *unchecked_end() noexcept
    {
        return elements_[SZE_];
    }

    constexpr const TYP_ *unchecked_end() const noexcept
    {
        return elements_[SZE_];
    }
#endif // DBJ_ARRAY_FULL_IMPLEMENTATION 

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return SZE_;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        return SZE_;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return false;
    }

    [[nodiscard]] constexpr reference at(size_type idx_)
    {
		DBJ_ASSERT(idx_ < SZE_);
		return elements_[idx_];
    }

    [[nodiscard]] constexpr const_reference at(size_type idx_) const
    {
		DBJ_ASSERT(idx_ < SZE_);
        return elements_[idx_];
    }

    [[nodiscard]] constexpr reference operator[](size_type idx_) noexcept
    { 
		DBJ_ASSERT(idx_ < SZE_ );
        return elements_[idx_];
    }

    [[nodiscard]] constexpr const_reference operator[](size_type idx_) const noexcept
    {
		DBJ_ASSERT(idx_ < SZE_);
        return elements_[idx_];
    }

    [[nodiscard]] constexpr reference front() noexcept
    { 
        return elements_[0];
    }

    [[nodiscard]] constexpr const_reference front() const noexcept
    { 
        return elements_[0];
    }

    [[nodiscard]] constexpr reference back() noexcept
    { 
        return elements_ + SZE_ - 1;
    }

    [[nodiscard]] constexpr const_reference back() const noexcept
    { // strengthened
        return elements_ + SZE_ - 1;
    }

    [[nodiscard]] constexpr TYP_ *data() noexcept
    {
        return elements_;
    }

    [[nodiscard]] constexpr const TYP_ *data() const noexcept
    {
        return elements_;
    }
	// for aggregate init the data must be public too
    TYP_ elements_[SZE_];
}; // array

template <class First_, class... Rest_>
struct ensure_param_pack_same_type
{
    static_assert(conjunction_v<is_same<First_, Rest_>...>,
      "dbj::nanolib::array, user defined template guide, requires all the types to be the same!");
    using type = First_;
};

template <class First_, class... Rest_>
array(First_, Rest_...)->array<typename ensure_param_pack_same_type<First_, Rest_...>::type, 1 + sizeof...(Rest_)>;

// vs the std::array I can make more constexpr functions here

template <class TYP_, size_t SZE_ >
 DBJ_ARRAY<TYP_, SZE_> swap
(
	DBJ_ARRAY<TYP_, SZE_>& left_arr_, 
	DBJ_ARRAY<TYP_, SZE_>& right_arr_
)
{ 
    left_arr_.swap(right_arr_);
	return left_arr_;
}

 // this is expensive operation
template <class TYP_, size_t SZE_ >
[[nodiscard]] constexpr bool operator==
(
	const DBJ_ARRAY<TYP_, SZE_> &left_arr_, const DBJ_ARRAY<TYP_, SZE_> &right_arr_
)
{
	for (size_t idx_ = 0; idx_ < SZE_; idx_++)
		if (left_arr_.at(idx_) != right_arr_.at(idx_)) return false;
	return true;
}

template <class TYP_, size_t SZE_ >
[[nodiscard]] constexpr  bool operator!=(const DBJ_ARRAY<TYP_, SZE_> &left_arr_, const DBJ_ARRAY<TYP_, SZE_> &right_arr_)
{
    return !(left_arr_ == right_arr_);
}

template <class TYP_, size_t SZE_ >
[[nodiscard]]  bool operator<(const DBJ_ARRAY<TYP_, SZE_> &left_arr_, const DBJ_ARRAY<TYP_, SZE_> &right_arr_)
{
    return std::lexicographical_compare(left_arr_.begin(), left_arr_.end(), right_arr_.begin(), right_arr_.end());
}

template <class TYP_, size_t SZE_ >
[[nodiscard]] bool operator>(const DBJ_ARRAY<TYP_, SZE_>&left_arr_, const DBJ_ARRAY<TYP_, SZE_> &right_arr_)
{
    return right_arr_ < left_arr_;
}

template <class TYP_, size_t SZE_ >
[[nodiscard]] bool operator<=(const DBJ_ARRAY<TYP_, SZE_> &left_arr_, const DBJ_ARRAY<TYP_, SZE_> &right_arr_)
{
    return !(right_arr_ < left_arr_);
}

template <class TYP_, size_t SZE_ >
[[nodiscard]] bool operator>=(const DBJ_ARRAY<TYP_, SZE_> &left_arr_, const DBJ_ARRAY<TYP_, SZE_> &right_arr_)
{
    return !(left_arr_ < right_arr_);
}

// TUPLE INTERFACE -------------------------------------------------------------

template <size_t idx_, class TYP_, size_t SZE_ >
[[nodiscard]] constexpr TYP_ &get( DBJ_ARRAY<TYP_, SZE_>& arr_) noexcept
{
    static_assert( idx_  < SZE_, "array index out of bounds");
    return  arr_.elements_[idx_ ];
}

template <size_t idx_ , class TYP_, size_t SZE_ >
[[nodiscard]] constexpr const TYP_ &get(const DBJ_ARRAY<TYP_, SZE_>& arr_) noexcept
{
    static_assert(idx_  < SZE_, "array index out of bounds");
    return  arr_.elements_[idx_ ];
}

template <size_t idx_ , class TYP_, size_t SZE_ >
[[nodiscard]] constexpr TYP_ &&get(DBJ_ARRAY<TYP_, SZE_>&& arr_) noexcept
{
    static_assert(idx_  < SZE_, "array index out of bounds");
    return std::move( arr_.elements_[idx_ ]);
}

template <size_t idx_ , class TYP_, size_t SZE_ >
[[nodiscard]] constexpr const TYP_ &&get(const DBJ_ARRAY<TYP_, SZE_> && arr_) noexcept
{
    static_assert(idx_  < SZE_, "array index out of bounds");
    return std::move( arr_.elements_[idx_ ]);
}

/*
-----------------------------------------------------------------------------------------------
What is the purpose of this type? kind of a array with the 'push_back' method
and end() methods which return, one after the latest stored element , not the end of the 
internal native array

Thus one can use this as very fast  storage with max capacity + all the array methods
for traversal and usage
-----------------------------------------------------------------------------------------------
*/

template<typename T_, size_t S_>
struct array_storage : DBJ_ARRAY< T_, S_ >
{
	using base = DBJ_ARRAY< T_, S_ >;

	constexpr static size_t storage_capacity{ S_ };

	size_t level_{ 0 };

	bool is_empty() const { return level_ == 0; }
	bool is_full() const { return level_ == storage_capacity; }

	T_ push_back(T_ next_fp) {
		if (is_full()) return nullptr;
		(*this)[level_] = next_fp;
		level_ += 1;
		return next_fp;
	}

	[[nodiscard]] constexpr typename base::const_iterator end() const noexcept {
		return typename base::const_iterator( this->elements_ + level_);
	}

	[[nodiscard]] constexpr typename  base::iterator end() noexcept {
		return typename base::iterator( this->elements_ + level_);
	}

	[[nodiscard]] constexpr typename base::size_type size() const noexcept {
		return level_;
	}

	[[nodiscard]] constexpr typename base::size_type max_size() const noexcept {
		return storage_capacity;
	}

}; // array_storage

namespace always_repeated_compile_time_tests {

	constexpr DBJ_ARRAY abc = { 'A', 'B', 'C' };
	constexpr DBJ_ARRAY def = { 'D', 'E', 'F' };

	constexpr decltype(abc)::value_type A = get<0>(abc);
	constexpr decltype(def)::value_type D = get<0>(def);

	static_assert( A != D ); 

	static_assert( abc != def );

} // always_repeated_compile_time_tests

} // namespace dbj::nanolib::containers

#endif // _DBJ_ARRAY_
