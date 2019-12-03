#ifndef _DBJ_REF_WRAP_INC_
#define _DBJ_REF_WRAP_INC_

/**
 * dbj reference wrapper with default constructor added
 * 
 * (c) 2019 by Dusan B. Jovanovic, https://dusanjovanovic.org
 * 
 * Licence CC BY SA 4.0
 * 
 * default constructed ref wrap contains a nullptr 
 * also has boolean implicit casting, to check if value is there
 * also has '*' unary operator to get the value stored
 * 
 * so what's the purpose?
 * 
 * dbj::ref_wrap<T> fun (unsigned idx_ ) {
 *      if (idx_ < data.size())
 *             return data[idx_];
 *      return dbj::ref_wrap<T>{} ; // nullptr 
 * }
 * usage:
 * auto slot42 = fun(42);
 * if ( ! slot42 ){
 *     perror("slot 42 does not exist!");
 * } else {
 *      auto value = *slot42 ;
 * }
 * 
 * other than above the behavior is exactly the same as C++20 std::reference_wrapper
 * 
 * upon trying to dereference the nullptr of the default constructed ref_wrap
 * application will exit.
*/

namespace dbj
{

#if !defined(_DBJ_HAS_CXX17) && !defined(_DBJ_HAS_CXX20)
#if defined(_MSVC_LANG)
#define _DBJ__STL_LANG _MSVC_LANG
#else
#define _DBJ__STL_LANG __cplusplus
#endif

#if _DBJ__STL_LANG > 201703L
#define _DBJ_HAS_CXX17 1
#define _DBJ_HAS_CXX20 1
#elif _DBJ__STL_LANG > 201402L
#define _DBJ_HAS_CXX17 1
#define _DBJ_HAS_CXX20 0
#else // _DBJ__STL_LANG <= 201402L
#define _DBJ_HAS_CXX17 0
#define _DBJ_HAS_CXX20 0
#endif // Use the value of _DBJ__STL_LANG to define _DBJ_HAS_CXX17 and _DBJ_HAS_CXX20

#undef _DBJ__STL_LANG
#endif // !defined(_DBJ_HAS_CXX17) && !defined(_DBJ_HAS_CXX20)

namespace detail
{

// STRUCT TEMPLATE remove_cv
template <class T_>
struct remove_cv
{ // remove top-level const and volatile qualifiers
    using type = T_;
};

template <class T_>
struct remove_cv<const T_>
{
    using type = T_;
};

template <class T_>
struct remove_cv<volatile T_>
{
    using type = T_;
};

template <class T_>
struct remove_cv<const volatile T_>
{
    using type = T_;
};

template <class T_>
using remove_cv_t = typename remove_cv<T_>::type;

// STRUCT TEMPLATE remove_reference
template <class T_>
struct remove_reference
{
    using type = T_;
};

template <class T_>
struct remove_reference<T_ &>
{
    using type = T_;
};

template <class T_>
struct remove_reference<T_ &&>
{
    using type = T_;
};

template <class T_>
using remove_reference_t = typename remove_reference<T_>::type;

template <class T_>
using _remove_cvref_t = remove_cv_t<remove_reference_t<T_>>;

#if !_DBH_HAS_CXX20
template <class T_>
using remove_cvref_t = _remove_cvref_t<T_>;

template <class T_>
struct remove_cvref
{
    using type = remove_cvref_t<T_>;
};
#endif // _DBH_HAS_CXX20

template <class T>
T &FUN(T &t) noexcept
{
    return t;
}
template <class T>
void FUN(T &&) = delete;

} // namespace detail

template <class T>
class ref_wrap
{
public:
    // types
    typedef T type;

    // construct/copy/destroy
    template <class U, class = decltype(
                           detail::FUN<T>(std::declval<U>()),
                           std::enable_if_t<!std::is_same_v<ref_wrap, detail::remove_cvref_t<U>>>())>
    constexpr ref_wrap(U &&u) noexcept(noexcept(detail::FUN<T>(std::forward<U>(u))))
        : ptr_(std::addressof(detail::FUN<T>(std::forward<U>(u)))) {}

    // DBJ ADDED
    constexpr ref_wrap() : ptr_(nullptr) {}
    // DBJ ADDED
    constexpr operator bool() const noexcept { return (ptr_ != nullptr); }

    ref_wrap(const ref_wrap &) noexcept = default;

    // assignment
    ref_wrap &operator=(const ref_wrap &x) noexcept = default;

    // access
    // DBJ added
    constexpr T &operator*() const noexcept
    {
        if (ptr_ == nullptr)
        {
            assert(ptr_);
            exit(1);
        }
        return *ptr_;
    }

    constexpr operator T &() const noexcept
    {
        if (ptr_ == nullptr)
        {
            assert(ptr_);
            exit(1);
        }
        return *ptr_;
    }
    constexpr T &get() const noexcept
    {
        if (ptr_ == nullptr)
        {
            assert(ptr_);
            exit(1);
        }
        return *ptr_;
    }

    template <class... ArgTypes>
    constexpr std::invoke_result_t<T &, ArgTypes...>
    operator()(ArgTypes &&... args) const
    {
        return std::invoke(get(), std::forward<ArgTypes>(args)...);
    }

private:
    T *ptr_;
};

// deduction guides
template <class T>
ref_wrap(T &)->ref_wrap<T>;

} // namespace dbj

#endif // _DBJ_REF_WRAP_INC_