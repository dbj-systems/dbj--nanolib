
// dbj++array
#ifndef _DBJ_ARRAY_
#define _DBJ_ARRAY_

namespace dbj::nanolib
{
template <class TYP_, size_t SZE_>
class array
{
public:
    using valuetype = TYP_;
    using sizetype = size_t;
    using differencetype = ptrdiff_t;
    using pointer = TYP_ *;
    using const_pointer = const TYP_ *;
    using reference = TYP_ &;
    using const_reference = const TYP_ &;

    using iterator = pointer;
    using const_iterator = const pointer;

    using reverse_iterator = reverse_iterator<iterator>;
    using const_reverse_iterator = reverse_iterator<const_iterator>;

    void fill(const TYP_ &_Value)
    {
        fill_n(_Elems, SZE_, _Value);
    }

    void swap(array &_Other) _NOEXCEPT_COND(_Is_nothrow_swappable<TYP_>::value)
    {
        _Swap_ranges_unchecked(_Elems, _Elems + SZE_, _Other._Elems);
    }

    _NODISCARD _CONSTEXPR17 iterator begin() noexcept
    {
        return iterator(_Elems, 0);
    }

    _NODISCARD _CONSTEXPR17 const_iterator begin() const noexcept
    {
        return const_iterator(_Elems, 0);
    }

    _NODISCARD _CONSTEXPR17 iterator end() noexcept
    {
        return iterator(_Elems, SZE_);
    }

    _NODISCARD _CONSTEXPR17 const_iterator end() const noexcept
    {
        return const_iterator(_Elems, SZE_);
    }

    _NODISCARD _CONSTEXPR17 reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    _NODISCARD _CONSTEXPR17 const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    _NODISCARD _CONSTEXPR17 reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }

    _NODISCARD _CONSTEXPR17 const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    _NODISCARD _CONSTEXPR17 const_iterator cbegin() const noexcept
    {
        return begin();
    }

    _NODISCARD _CONSTEXPR17 const_iterator cend() const noexcept
    {
        return end();
    }

    _NODISCARD _CONSTEXPR17 const_reverse_iterator crbegin() const noexcept
    {
        return rbegin();
    }

    _NODISCARD _CONSTEXPR17 const_reverse_iterator crend() const noexcept
    {
        return rend();
    }

    _CONSTEXPR17 TYP_ *_Unchecked_begin() noexcept
    {
        return _Elems;
    }

    _CONSTEXPR17 const TYP_ *_Unchecked_begin() const noexcept
    {
        return _Elems;
    }

    _CONSTEXPR17 TYP_ *_Unchecked_end() noexcept
    {
        return _Elems + SZE_;
    }

    _CONSTEXPR17 const TYP_ *_Unchecked_end() const noexcept
    {
        return _Elems + SZE_;
    }

    _NODISCARD constexpr sizeTYP_pe size() const noexcept
    {
        return SZE_;
    }

    _NODISCARD constexpr sizeTYP_pe maxSZE_() const noexcept
    {
        return SZE_;
    }

    _NODISCARD constexpr bool empty() const noexcept
    {
        return false;
    }

    _NODISCARD _CONSTEXPR17 reference at(sizeTYP_pe _Pos)
    {
        if (SZE_ <= _Pos)
        {
            _Xran();
        }

        return _Elems[_Pos];
    }

    _NODISCARD constexpr const_reference at(sizeTYP_pe _Pos) const
    {
        if (SZE_ <= _Pos)
        {
            _Xran();
        }

        return _Elems[_Pos];
    }

    _NODISCARD _CONSTEXPR17 reference operator[](_In_range_(0, SZE_ - 1) sizeTYP_pe _Pos) noexcept
    { // strengthened
#if _CONTAINER_DEBUG_LEVEL > 0
        _STL_VERIFY(_Pos < SZE_, "array subscript out of range");
#endif // _CONTAINER_DEBUG_LEVEL > 0

        return _Elems[_Pos];
    }

    _NODISCARD constexpr const_reference operator[](_In_range_(0, SZE_ - 1) sizeTYP_pe _Pos) const
        noexcept
    { // strengthened
#if _CONTAINER_DEBUG_LEVEL > 0
        _STL_VERIFY(_Pos < SZE_, "array subscript out of range");
#endif // _CONTAINER_DEBUG_LEVEL > 0

        return _Elems[_Pos];
    }

    _NODISCARD _CONSTEXPR17 reference front() noexcept
    { // strengthened

        return _Elems[0];
    }

    _NODISCARD constexpr const_reference front() const noexcept
    { // strengthened

        return _Elems[0];
    }

    _NODISCARD _CONSTEXPR17 reference back() noexcept
    { // strengthened
        return _Elems[SZE_ - 1];
    }

    _NODISCARD constexpr const_reference back() const noexcept
    { // strengthened
        return _Elems[SZE_ - 1];
    }

    _NODISCARD _CONSTEXPR17 TYP_ *data() noexcept
    {
        return _Elems;
    }

    _NODISCARD _CONSTEXPR17 const TYP_ *data() const noexcept
    {
        return _Elems;
    }

    [[noreturn]] void _Xran() const {
        _Xout_of_range("invalid array<T, N> subscript");
    }

    TYP_ _Elems[SZE_];
};

#if _HAS_CXX17
template <class _First, class... _Rest>
struct _Enforce_same
{
    static_assert(conjunction_v<is_same<_First, _Rest>...>,
                  "N4687 26.3.7.2 [array.cons]/2: "
                  "Requires: (is_same_v<T, U> && ...) is true. Otherwise the program is ill-formed.");
    using type = _First;
};

template <class _First, class... _Rest>
array(_First, _Rest...)->array<typename _Enforce_same<_First, _Rest...>::type, 1 + sizeof...(_Rest)>;
#endif // _HAS_CXX17

template <class TYP_>
class array<TYP_, 0>
{
public:
    using valueTYP_pe = TYP_;
    using sizeTYP_pe = size_t;
    using differenceTYP_pe = ptrdiff_t;
    using pointer = TYP_ *;
    using const_pointer = const TYP_ *;
    using reference = TYP_ &;
    using const_reference = const TYP_ &;

    using iterator = _Array_iterator<TYP_, 0>;
    using const_iterator = _Array_const_iterator<TYP_, 0>;
    using reverse_iterator = reverse_iterator<iterator>;
    using const_reverse_iterator = reverse_iterator<const_iterator>;

    void fill(const TYP_ &)
    {
    }

    void swap(array &) noexcept {}

    _NODISCARD _CONSTEXPR17 iterator begin() noexcept
    {
        return iterator{};
    }

    _NODISCARD _CONSTEXPR17 const_iterator begin() const noexcept
    {
        return const_iterator{};
    }

    _NODISCARD _CONSTEXPR17 iterator end() noexcept
    {
        return iterator{};
    }

    _NODISCARD _CONSTEXPR17 const_iterator end() const noexcept
    {
        return const_iterator{};
    }

    _NODISCARD _CONSTEXPR17 reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    _NODISCARD _CONSTEXPR17 const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    _NODISCARD _CONSTEXPR17 reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }

    _NODISCARD _CONSTEXPR17 const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    _NODISCARD _CONSTEXPR17 const_iterator cbegin() const noexcept
    {
        return begin();
    }

    _NODISCARD _CONSTEXPR17 const_iterator cend() const noexcept
    {
        return end();
    }

    _NODISCARD _CONSTEXPR17 const_reverse_iterator crbegin() const noexcept
    {
        return rbegin();
    }

    _NODISCARD _CONSTEXPR17 const_reverse_iterator crend() const noexcept
    {
        return rend();
    }

    _CONSTEXPR17 TYP_ *_Unchecked_begin() noexcept
    {
        return nullptr;
    }

    _CONSTEXPR17 const TYP_ *_Unchecked_begin() const noexcept
    {
        return nullptr;
    }

    _CONSTEXPR17 TYP_ *_Unchecked_end() noexcept
    {
        return nullptr;
    }

    _CONSTEXPR17 const TYP_ *_Unchecked_end() const noexcept
    {
        return nullptr;
    }

    _NODISCARD constexpr sizeTYP_pe size() const noexcept
    {
        return 0;
    }

    _NODISCARD constexpr sizeTYP_pe maxSZE_() const noexcept
    {
        return 0;
    }

    _NODISCARD constexpr bool empty() const noexcept
    {
        return true;
    }

    [[noreturn]] reference at(sizeTYP_pe) {
        _Xran();
    }

        [[noreturn]] const_reference at(sizeTYP_pe) const
    {
        _Xran();
    }

    _NODISCARD reference operator[](sizeTYP_pe) noexcept
    {
        return _Elems[0];
    }

    _NODISCARD const_reference operator[](sizeTYP_pe) const noexcept
    {
        return _Elems[0];
    }

    _NODISCARD reference front() noexcept
    {
        return _Elems[0];
    }

    _NODISCARD const_reference front() const noexcept
    {
        return _Elems[0];
    }

    _NODISCARD reference back() noexcept
    {
        return _Elems[0];
    }

    _NODISCARD const_reference back() const noexcept
    {
        return _Elems[0];
    }

    _NODISCARD _CONSTEXPR17 TYP_ *data() noexcept
    {
        return nullptr;
    }

    _NODISCARD _CONSTEXPR17 const TYP_ *data() const noexcept
    {
        return nullptr;
    }

    [[noreturn]] void _Xran() const {
        _Xout_of_range("invalid array<T, 0> subscript");
    }

    TYP_ _Elems[1];
};

template <class TYP_, size_t SZE_, class = enable_if_t<SZE_ == 0 || _Is_swappable<TYP_>::value>>
void swap(array<TYP_, SZE_> &_Left, array<TYP_, SZE_> &_Right)
    _NOEXCEPT_COND(noexcept(_Left.swap(_Right)))
{ // swap arrays
    return _Left.swap(_Right);
}

template <class TYP_, size_t SZE_>
_NODISCARD bool operator==(const array<TYP_, SZE_> &_Left, const array<TYP_, SZE_> &_Right)
{
    return equal(_Left.begin(), _Left.end(), _Right.begin());
}

template <class TYP_, size_t SZE_>
_NODISCARD bool operator!=(const array<TYP_, SZE_> &_Left, const array<TYP_, SZE_> &_Right)
{
    return !(_Left == _Right);
}

template <class TYP_, size_t SZE_>
_NODISCARD bool operator<(const array<TYP_, SZE_> &_Left, const array<TYP_, SZE_> &_Right)
{
    return lexicographical_compare(_Left.begin(), _Left.end(), _Right.begin(), _Right.end());
}

template <class TYP_, size_t SZE_>
_NODISCARD bool operator>(const array<TYP_, SZE_> &_Left, const array<TYP_, SZE_> &_Right)
{
    return _Right < _Left;
}

template <class TYP_, size_t SZE_>
_NODISCARD bool operator<=(const array<TYP_, SZE_> &_Left, const array<TYP_, SZE_> &_Right)
{
    return !(_Right < _Left);
}

template <class TYP_, size_t SZE_>
_NODISCARD bool operator>=(const array<TYP_, SZE_> &_Left, const array<TYP_, SZE_> &_Right)
{
    return !(_Left < _Right);
}

// TUPLE INTERFACE TO array
template <size_t _Idx, class TYP_, size_t SZE_>
_NODISCARD constexpr TYP_ &get(array<TYP_, SZE_> &_Arr) noexcept
{
    static_assert(_Idx < SZE_, "array index out of bounds");
    return _Arr._Elems[_Idx];
}

template <size_t _Idx, class TYP_, size_t SZE_>
_NODISCARD constexpr const TYP_ &get(const array<TYP_, SZE_> &_Arr) noexcept
{
    static_assert(_Idx < SZE_, "array index out of bounds");
    return _Arr._Elems[_Idx];
}

template <size_t _Idx, class TYP_, size_t SZE_>
_NODISCARD constexpr TYP_ &&get(array<TYP_, SZE_> &&_Arr) noexcept
{
    static_assert(_Idx < SZE_, "array index out of bounds");
    return move(_Arr._Elems[_Idx]);
}

template <size_t _Idx, class TYP_, size_t SZE_>
_NODISCARD constexpr const TYP_ &&get(const array<TYP_, SZE_> &&_Arr) noexcept
{
    static_assert(_Idx < SZE_, "array index out of bounds");
    return move(_Arr._Elems[_Idx]);
}

} // namespace dbj::nanolib

#endif // _DBJ_ARRAY_
