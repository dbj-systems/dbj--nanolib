#ifndef _DBJ_INC_DBJ_VECTOR_
#define _DBJ_INC_DBJ_VECTOR_

#ifdef __clang__
#pragma clang system_header
#endif

/*
TODO -- specialize erase for scalars
TODO -- specialize for wchar_t
*/

/*
vector minus std::allocator
based on `lni::vector` by Jasmine "lnishan" Chen, licensed under a [Creative Commons Attribution 4.0 International License](http://creativecommons.org/licenses/by/4.0/).
*/

#include <cstddef>
#include <cstring>
#include <utility>
#include <iterator>

#define DBJ_VECTOR_HAS_EXCEPTIONS 0

#if _HAS_EXCEPTIONS || DBJ_VECTOR_HAS_EXCEPTIONS
#include <stdexcept>
#endif

// seting both to 1 will make for a "proper" std::vector interface
// set to 1 to have reverse iterators  and associated methods
#define _DBJ_VEC_REVERSING 0
// set to 1 to have insertion methods
#define _DBJ_VEC_INSERTING 0

namespace dbj::nanolib
{

inline constexpr auto _DBJ_INC_DBJ_VECTOR__MAX_SZ = MAXINT;

/*
	renamed from vector to light_vector
	so that both dbj: and std: namespaces can be used with both
	std::vector and dbj::light_vector
	*/
template <typename T>
class vector;

template <typename T>
class vector
{
public:
	// types:
	typedef T value_type;
	typedef T &reference;
	typedef const T &const_reference;
	typedef T *pointer;
	typedef const T *const_pointer;
	typedef T *iterator;
	typedef const T *const_iterator;
#if _DBJ_VEC_REVERSING
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
#endif // _DBJ_VEC_REVERSING
	typedef std::ptrdiff_t difference_type;
	typedef std::size_t size_type;

	vector() noexcept
	{
		arr = new T[rsrv_sz];
	}

	vector(typename size_type n)
	{
		_ASSERTE(n < _DBJ_INC_DBJ_VECTOR__MAX_SZ);
		size_type i;
		rsrv_sz = n << 2;
		arr = new T[rsrv_sz];
		for (i = 0; i < n; ++i)
			arr[i] = T();
		vec_sz = n;
	}

	vector(typename size_type n, const T &value)
	{
		_ASSERTE(n < _DBJ_INC_DBJ_VECTOR__MAX_SZ);
		size_type i;
		rsrv_sz = n << 2;
		arr = new T[rsrv_sz];
		for (i = 0; i < n; ++i)
			arr[i] = value;
		vec_sz = n;
	}

	//vector(typename iterator first, typename iterator last) {
	//	size_type i, count = last - first;
	//  _ASSERTE( count < _DBJ_INC_DBJ_VECTOR__MAX_SZ );
	//	rsrv_sz = count << 2;
	//	arr = new T[rsrv_sz];
	//	for (i = 0; i < count; ++i, ++first)
	//		arr[i] = *first;
	//	vec_sz = count;
	//}

	vector(typename const_iterator first, typename const_iterator last)
	{
		size_type i, count = last - first;
		_ASSERTE(count < _DBJ_INC_DBJ_VECTOR__MAX_SZ);
		rsrv_sz = count << 2;
		arr = new T[rsrv_sz];
		for (i = 0; i < count; ++i, ++first)
			arr[i] = *first;
		vec_sz = count;
	}

	vector(std::initializer_list<T> lst)
	{
		rsrv_sz = lst.size() << 2;
		arr = new T[rsrv_sz];
		for (auto &item : lst)
			arr[vec_sz++] = item;
	}

	vector(const vector<T> &other)
	{
		size_type i;
		rsrv_sz = other.rsrv_sz;
		arr = new T[rsrv_sz];
		for (i = 0; i < other.vec_sz; ++i)
			arr[i] = other.arr[i];
		vec_sz = other.vec_sz;
	}

	vector(vector<T> &&other) noexcept
	{
		size_type i;
		rsrv_sz = other.rsrv_sz;
		arr = new T[rsrv_sz];
		for (i = 0; i < other.vec_sz; ++i)
			arr[i] = std::move(other.arr[i]);
		vec_sz = other.vec_sz;
	}

	~vector()
	{
		delete[] arr;
	}

	vector<T> &operator=(const vector<T> &other)
	{
		size_type i;
		if (rsrv_sz < other.vec_sz)
		{
			rsrv_sz = other.vec_sz << 2;
			reallocate();
		}
		for (i = 0; i < other.vec_sz; ++i)
			arr[i] = other.arr[i];
		vec_sz = other.vec_sz;
	}

	vector<T> &operator=(vector<T> &&other)
	{
		size_type i;
		if (rsrv_sz < other.vec_sz)
		{
			rsrv_sz = other.vec_sz << 2;
			reallocate();
		}
		for (i = 0; i < other.vec_sz; ++i)
			arr[i] = std::move(other.arr[i]);
		vec_sz = other.vec_sz;
	}

	vector<T> &operator=(std::initializer_list<T> lst)
	{
		if (rsrv_sz < lst.size())
		{
			rsrv_sz = lst.size() << 2;
			reallocate();
		}
		vec_sz = 0;
		for (auto &item : lst)
			arr[vec_sz++] = item;
	}

	void assign(typename size_type count, const T &value)
	{
		size_type i;
		if (count > rsrv_sz)
		{
			rsrv_sz = count << 2;
			reallocate();
		}
		for (i = 0; i < count; ++i)
			arr[i] = value;
		vec_sz = count;
	}

	void assign(typename iterator first, typename iterator last)
	{
		size_type i, count = last - first;
		if (count > rsrv_sz)
		{
			rsrv_sz = count << 2;
			reallocate();
		}
		for (i = 0; i < count; ++i, ++first)
			arr[i] = *first;
		vec_sz = count;
	}

	void assign(std::initializer_list<T> lst)
	{
		size_type i, count = lst.size();
		if (count > rsrv_sz)
		{
			rsrv_sz = count << 2;
			reallocate();
		}
		i = 0;
		for (auto &item : lst)
			arr[i++] = item;
	}

	typename iterator begin() noexcept
	{
		return arr;
	}

	typename const_iterator cbegin() const noexcept
	{
		return arr;
	}

	typename iterator end() noexcept
	{
		return arr + vec_sz;
	}

	typename const_iterator cend() const noexcept
	{
		return arr + vec_sz;
	}

#if _DBJ_VEC_REVERSING

	typename reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(arr + vec_sz);
	}

	typename const_reverse_iterator crbegin() const noexcept
	{
		return reverse_iterator(arr + vec_sz);
	}

	typename reverse_iterator rend() noexcept
	{
		return reverse_iterator(arr);
	}

	typename const_reverse_iterator crend() const noexcept
	{
		return reverse_iterator(arr);
	}

#endif // _DBJ_VEC_REVERSING

	void reallocate()
	{
		T *tarr = new T[rsrv_sz];
		memcpy(tarr, arr, vec_sz * sizeof(T));
		delete[] arr;
		arr = tarr;
	}

	bool empty() const noexcept
	{
		return vec_sz == 0;
	}

	typename size_type size() const noexcept
	{
		return vec_sz;
	}

	typename size_type max_size() const noexcept
	{
		return static_cast<size_type>(_DBJ_INC_DBJ_VECTOR__MAX_SZ);
	}

	typename size_type capacity() const noexcept
	{
		return rsrv_sz;
	}

	void resize(typename size_type sz)
	{
		if (sz > vec_sz)
		{
			if (sz > rsrv_sz)
			{
				rsrv_sz = sz;
				reallocate();
			}
		}
		else
		{
			size_type i;
			for (i = vec_sz; i < sz; ++i)
				arr[i].~T();
		}
		vec_sz = sz;
	}

	void resize(typename size_type sz, const T &c)
	{
		if (sz > vec_sz)
		{
			if (sz > rsrv_sz)
			{
				rsrv_sz = sz;
				reallocate();
			}
			size_type i;
			for (i = vec_sz; i < sz; ++i)
				arr[i] = c;
		}
		else
		{
			size_type i;
			for (i = vec_sz; i < sz; ++i)
				arr[i].~T();
		}
		vec_sz = sz;
	}

	void reserve(typename size_type _sz)
	{
		if (_sz > rsrv_sz)
		{
			rsrv_sz = _sz;
			reallocate();
		}
	}

	void shrink_to_fit()
	{
		rsrv_sz = vec_sz;
		reallocate();
	}

	typename reference operator[](typename size_type idx)
	{
		_ASSERTE(idx < vec_sz); // dbj added
		return arr[idx];
	}

	typename const_reference operator[](typename size_type idx) const
	{
		_ASSERTE(idx < vec_sz); // dbj added
		return arr[idx];
	}

	typename reference at(size_type pos)
	{
#if _HAS_EXCEPTIONS
		if (pos < vec_sz)
			return arr[pos];
		else
			throw std::out_of_range("accessed position is out of range");
#else
		_ASSERTE(pos < vec_sz)
		return arr[pos];
#endif
	}

	typename const_reference at(size_type pos) const
	{
#if _HAS_EXCEPTIONS
		if (pos < vec_sz)
			return arr[pos];
		else
			throw std::out_of_range("accessed position is out of range");
#else
		_ASSERTE(pos < vec_sz)
		return arr[pos];
#endif
	}

	typename reference front()
	{
		return arr[0];
	}

	typename const_reference front() const
	{
		return arr[0];
	}

	typename reference back()
	{
		return arr[vec_sz - 1];
	}

	typename const_reference back() const
	{
		return arr[vec_sz - 1];
	}

	T *data() noexcept
	{
		return arr;
	}

	const T *data() const noexcept
	{
		return arr;
	}

	template <class... Args>
	void emplace_back(Args &&... args)
	{
		if (vec_sz == rsrv_sz)
		{
			rsrv_sz <<= 2;
			reallocate();
		}
		arr[vec_sz] = std::move(T(std::forward<Args>(args)...));
		++vec_sz;
	}

	void push_back(const T &val)
	{
		if (vec_sz == rsrv_sz)
		{
			rsrv_sz <<= 2;
			reallocate();
		}
		arr[vec_sz] = val;
		++vec_sz;
	}

	void push_back(T &&val)
	{
		if (vec_sz == rsrv_sz)
		{
			rsrv_sz <<= 2;
			reallocate();
		}
		arr[vec_sz] = std::move(val);
		++vec_sz;
	}

	void pop_back()
	{
		--vec_sz;
		arr[vec_sz].~T();
	}

	template <class... Args>
	typename iterator emplace(typename const_iterator it, Args &&... args)
	{
		iterator iit = &arr[it - arr];
		if (vec_sz == rsrv_sz)
		{
			rsrv_sz <<= 2;
			reallocate();
		}
		memmove(iit + 1, iit, (vec_sz - (it - arr)) * sizeof(T));
		(*iit) = std::move(T(std::forward<Args>(args)...));
		++vec_sz;
		return iit;
	}

#if _DBJ_VEC_INSERTING
	typename iterator insert(typename const_iterator it, const T &val)
	{
		iterator iit = &arr[it - arr];
		if (vec_sz == rsrv_sz)
		{
			rsrv_sz <<= 2;
			reallocate();
		}
		memmove(iit + 1, iit, (vec_sz - (it - arr)) * sizeof(T));
		(*iit) = val;
		++vec_sz;
		return iit;
	}

	typename iterator insert(typename const_iterator it, T &&val)
	{
		iterator iit = &arr[it - arr];
		if (vec_sz == rsrv_sz)
		{
			rsrv_sz <<= 2;
			reallocate();
		}
		memmove(iit + 1, iit, (vec_sz - (it - arr)) * sizeof(T));
		(*iit) = std::move(val);
		++vec_sz;
		return iit;
	}

	typename iterator insert(typename const_iterator it, typename size_type cnt, const T &val)
	{
		iterator f = &arr[it - arr];
		if (!cnt)
			return f;
		if (vec_sz + cnt > rsrv_sz)
		{
			rsrv_sz = (vec_sz + cnt) << 2;
			reallocate();
		}
		memmove(f + cnt, f, (vec_sz - (it - arr)) * sizeof(T));
		vec_sz += cnt;
		for (iterator it = f; cnt--; ++it)
			(*it) = val;
		return f;
	}

	template <class InputIt>
	typename iterator insert(typename const_iterator it, InputIt first, InputIt last)
	{
		iterator f = &arr[it - arr];
		size_type cnt = last - first;
		if (!cnt)
			return f;
		if (vec_sz + cnt > rsrv_sz)
		{
			rsrv_sz = (vec_sz + cnt) << 2;
			reallocate();
		}
		memmove(f + cnt, f, (vec_sz - (it - arr)) * sizeof(T));
		for (iterator it = f; first != last; ++it, ++first)
			(*it) = *first;
		vec_sz += cnt;
		return f;
	}

	typename iterator insert(typename const_iterator it, std::initializer_list<T> lst)
	{
		size_type cnt = lst.size();
		iterator f = &arr[it - arr];
		if (!cnt)
			return f;
		if (vec_sz + cnt > rsrv_sz)
		{
			rsrv_sz = (vec_sz + cnt) << 2;
			reallocate();
		}
		memmove(f + cnt, f, (vec_sz - (it - arr)) * sizeof(T));
		iterator iit = f;
		for (auto &item : lst)
		{
			(*iit) = item;
			++iit;
		}
		vec_sz += cnt;
		return f;
	}
#endif // _DBJ_VEC_INSERTING

	typename iterator erase(typename const_iterator it)
	{
		iterator iit = &arr[it - arr];
		(*iit).~T();
		memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(T));
		--vec_sz;
		return iit;
	}

	typename iterator erase(typename const_iterator first, typename const_iterator last)
	{
		iterator f = &arr[first - arr];
		if (first == last)
			return f;
		for (; first != last; ++first)
			(*first).~T();
		memmove(f, last, (vec_sz - (last - arr)) * sizeof(T));
		vec_sz -= last - first;
		return f;
	}

	void swap(vector<T> &rhs)
	{
		size_t tvec_sz = vec_sz,
			   trsrv_sz = rsrv_sz;
		T *tarr = arr;

		vec_sz = rhs.vec_sz;
		rsrv_sz = rhs.rsrv_sz;
		arr = rhs.arr;

		rhs.vec_sz = tvec_sz;
		rhs.rsrv_sz = trsrv_sz;
		rhs.arr = tarr;
	}

	void clear() noexcept
	{
		size_type i;
		for (i = 0; i < vec_sz; ++i)
			arr[i].~T();
		vec_sz = 0;
	}

	bool operator==(const vector<T> &rhs) const
	{
		if (vec_sz != rhs.vec_sz)
			return false;
		size_type i;
		for (i = 0; i < vec_sz; ++i)
			if (arr[i] != rhs.arr[i])
				return false;
		return true;
	}

	bool operator!=(const vector<T> &rhs) const
	{
		if (vec_sz != rhs.vec_sz)
			return true;
		size_type i;
		for (i = 0; i < vec_sz; ++i)
			if (arr[i] != rhs.arr[i])
				return true;
		return false;
	}

	bool operator<(const vector<T> &rhs) const
	{
		size_type i, ub = vec_sz < rhs.vec_sz ? vec_sz : rhs.vec_sz;
		for (i = 0; i < ub; ++i)
			if (arr[i] != rhs.arr[i])
				return arr[i] < rhs.arr[i];
		return vec_sz < rhs.vec_sz;
	}

	bool operator<=(const vector<T> &rhs) const
	{
		size_type i, ub = vec_sz < rhs.vec_sz ? vec_sz : rhs.vec_sz;
		for (i = 0; i < ub; ++i)
			if (arr[i] != rhs.arr[i])
				return arr[i] < rhs.arr[i];
		return vec_sz <= rhs.vec_sz;
	}

	bool operator>(const vector<T> &rhs) const
	{
		size_type i, ub = vec_sz < rhs.vec_sz ? vec_sz : rhs.vec_sz;
		for (i = 0; i < ub; ++i)
			if (arr[i] != rhs.arr[i])
				return arr[i] > rhs.arr[i];
		return vec_sz > rhs.vec_sz;
	}

	bool operator>=(const vector<T> &rhs) const
	{
		size_type i, ub = vec_sz < rhs.vec_sz ? vec_sz : rhs.vec_sz;
		for (i = 0; i < ub; ++i)
			if (arr[i] != rhs.arr[i])
				return arr[i] > rhs.arr[i];
		return vec_sz >= rhs.vec_sz;
	}

private:
	size_type rsrv_sz = 4;
	size_type vec_sz = 0;
	T *arr;
}; // eof class dbj::vector

/*
	optimization -- specialized methods for arythmetic types
	*/

template <>
void vector<bool>::resize(typename vector<bool>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<signed char>::resize(typename vector<signed char>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<unsigned char>::resize(typename vector<unsigned char>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<char>::resize(typename vector<char>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<short int>::resize(typename vector<short int>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<unsigned short int>::resize(typename vector<unsigned short int>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<int>::resize(typename vector<int>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<unsigned int>::resize(typename vector<unsigned int>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<long int>::resize(typename vector<long int>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<unsigned long int>::resize(typename vector<unsigned long int>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<long long int>::resize(typename vector<long long int>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<unsigned long long int>::resize(typename vector<unsigned long long int>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<float>::resize(typename vector<float>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<double>::resize(typename vector<double>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<long double>::resize(typename vector<long double>::size_type sz)
{
	if (sz > rsrv_sz)
	{
		rsrv_sz = sz;
		reallocate();
	}
	vec_sz = sz;
}

template <>
void vector<bool>::resize(typename vector<bool>::size_type sz, const bool &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<signed char>::resize(typename vector<signed char>::size_type sz, const signed char &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<unsigned char>::resize(typename vector<unsigned char>::size_type sz, const unsigned char &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<char>::resize(typename vector<char>::size_type sz, const char &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<short int>::resize(typename vector<short int>::size_type sz, const short int &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<unsigned short int>::resize(typename vector<unsigned short int>::size_type sz, const unsigned short int &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<int>::resize(typename vector<int>::size_type sz, const int &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<unsigned int>::resize(typename vector<unsigned int>::size_type sz, const unsigned int &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<long int>::resize(typename vector<long int>::size_type sz, const long int &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<unsigned long int>::resize(typename vector<unsigned long int>::size_type sz, const unsigned long int &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<long long int>::resize(typename vector<long long int>::size_type sz, const long long int &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<unsigned long long int>::resize(typename vector<unsigned long long int>::size_type sz, const unsigned long long int &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<float>::resize(typename vector<float>::size_type sz, const float &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<double>::resize(typename vector<double>::size_type sz, const double &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<long double>::resize(typename vector<long double>::size_type sz, const long double &c)
{
	if (sz > vec_sz)
	{
		if (sz > rsrv_sz)
		{
			rsrv_sz = sz;
			reallocate();
		}
		size_type i;
		for (i = vec_sz; i < sz; ++i)
			arr[i] = c;
	}
	vec_sz = sz;
}

template <>
void vector<bool>::pop_back()
{
	--vec_sz;
}

template <>
void vector<signed char>::pop_back()
{
	--vec_sz;
}

template <>
void vector<unsigned char>::pop_back()
{
	--vec_sz;
}

template <>
void vector<char>::pop_back()
{
	--vec_sz;
}

template <>
void vector<short int>::pop_back()
{
	--vec_sz;
}

template <>
void vector<unsigned short int>::pop_back()
{
	--vec_sz;
}

template <>
void vector<int>::pop_back()
{
	--vec_sz;
}

template <>
void vector<unsigned int>::pop_back()
{
	--vec_sz;
}

template <>
void vector<long int>::pop_back()
{
	--vec_sz;
}

template <>
void vector<unsigned long int>::pop_back()
{
	--vec_sz;
}

template <>
void vector<long long int>::pop_back()
{
	--vec_sz;
}

template <>
void vector<unsigned long long int>::pop_back()
{
	--vec_sz;
}

template <>
void vector<float>::pop_back()
{
	--vec_sz;
}

template <>
void vector<double>::pop_back()
{
	--vec_sz;
}

template <>
void vector<long double>::pop_back()
{
	--vec_sz;
}

template <>
typename vector<bool>::iterator vector<bool>::erase(typename vector<bool>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(bool));
	--vec_sz;
	return iit;
}

template <>
typename vector<signed char>::iterator vector<signed char>::erase(typename vector<signed char>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(signed char));
	--vec_sz;
	return iit;
}

template <>
typename vector<unsigned char>::iterator vector<unsigned char>::erase(typename vector<unsigned char>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(unsigned char));
	--vec_sz;
	return iit;
}

template <>
typename vector<char>::iterator vector<char>::erase(typename vector<char>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(char));
	--vec_sz;
	return iit;
}

template <>
typename vector<short int>::iterator vector<short int>::erase(typename vector<short int>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(short int));
	--vec_sz;
	return iit;
}

template <>
typename vector<unsigned short int>::iterator vector<unsigned short int>::erase(typename vector<unsigned short int>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(unsigned short int));
	--vec_sz;
	return iit;
}

template <>
typename vector<int>::iterator vector<int>::erase(typename vector<int>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(int));
	--vec_sz;
	return iit;
}

template <>
typename vector<unsigned int>::iterator vector<unsigned int>::erase(typename vector<unsigned int>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(unsigned int));
	--vec_sz;
	return iit;
}

template <>
typename vector<long int>::iterator vector<long int>::erase(typename vector<long int>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(long int));
	--vec_sz;
	return iit;
}

template <>
typename vector<unsigned long int>::iterator vector<unsigned long int>::erase(typename vector<unsigned long int>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(unsigned long int));
	--vec_sz;
	return iit;
}

template <>
typename vector<long long int>::iterator vector<long long int>::erase(typename vector<long long int>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(long long int));
	--vec_sz;
	return iit;
}

template <>
typename vector<unsigned long long int>::iterator vector<unsigned long long int>::erase(typename vector<unsigned long long int>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(unsigned long long int));
	--vec_sz;
	return iit;
}

template <>
typename vector<float>::iterator vector<float>::erase(typename vector<float>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(float));
	--vec_sz;
	return iit;
}

template <>
typename vector<double>::iterator vector<double>::erase(typename vector<double>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(double));
	--vec_sz;
	return iit;
}

template <>
typename vector<long double>::iterator vector<long double>::erase(typename vector<long double>::const_iterator it)
{
	iterator iit = &arr[it - arr];
	memmove(iit, iit + 1, (vec_sz - (it - arr) - 1) * sizeof(long double));
	--vec_sz;
	return iit;
}

template <>
typename vector<bool>::iterator vector<bool>::erase(typename vector<bool>::const_iterator first, typename vector<bool>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(bool));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<signed char>::iterator vector<signed char>::erase(typename vector<signed char>::const_iterator first, typename vector<signed char>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(signed char));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<unsigned char>::iterator vector<unsigned char>::erase(typename vector<unsigned char>::const_iterator first, typename vector<unsigned char>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(unsigned char));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<char>::iterator vector<char>::erase(typename vector<char>::const_iterator first, typename vector<char>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(char));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<short int>::iterator vector<short int>::erase(typename vector<short int>::const_iterator first, typename vector<short int>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(short int));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<unsigned short int>::iterator vector<unsigned short int>::erase(typename vector<unsigned short int>::const_iterator first, typename vector<unsigned short int>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(unsigned short int));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<int>::iterator vector<int>::erase(typename vector<int>::const_iterator first, typename vector<int>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(int));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<unsigned int>::iterator vector<unsigned int>::erase(typename vector<unsigned int>::const_iterator first, typename vector<unsigned int>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(unsigned int));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<long long int>::iterator vector<long long int>::erase(typename vector<long long int>::const_iterator first, typename vector<long long int>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(long long int));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<unsigned long long int>::iterator vector<unsigned long long int>::erase(typename vector<unsigned long long int>::const_iterator first, typename vector<unsigned long long int>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(unsigned long long int));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<float>::iterator vector<float>::erase(typename vector<float>::const_iterator first, typename vector<float>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(float));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<double>::iterator vector<double>::erase(typename vector<double>::const_iterator first, typename vector<double>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(double));
	vec_sz -= last - first;
	return f;
}

template <>
typename vector<long double>::iterator vector<long double>::erase(typename vector<long double>::const_iterator first, typename vector<long double>::const_iterator last)
{
	iterator f = &arr[first - arr];
	if (first == last)
		return f;
	memmove(f, last, (vec_sz - (last - arr)) * sizeof(long double));
	vec_sz -= last - first;
	return f;
}

template <>
void vector<bool>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<signed char>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<unsigned char>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<char>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<short int>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<unsigned short int>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<int>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<unsigned int>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<long int>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<unsigned long int>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<long long int>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<unsigned long long int>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<float>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<double>::clear() noexcept
{
	vec_sz = 0;
}

template <>
void vector<long double>::clear() noexcept
{
	vec_sz = 0;
}

} // namespace dbj::nanolib

#endif // _DBJ_INC_DBJ_VECTOR_
