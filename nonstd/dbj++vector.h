#ifndef _DBJ_INC_DBJ_VECTOR_
#define _DBJ_INC_DBJ_VECTOR_

#ifdef __clang__
#ifdef NDEBUG
#pragma clang system_header
#endif
#endif

/*
TODO -- specialize erase for scalars
TODO -- specialize for wchar_t
*/

/*
vector minus std::allocator
based on `lni::vector` by Jasmine "lnishan" Chen, licensed under a
[Creative Commons Attribution 4.0 International License](http://creativecommons.org/licenses/by/4.0/).
*/

#include <cstddef>
#include <cstring>
#include <utility>
#include <iterator>

// use this simple macro ... do not open the whole namespace with the using statement
#define DBJ_VECTOR ::dbj::nanolib::containers::vector

#define DBJ_VECTOR_HAS_EXCEPTIONS 0

#if _HAS_EXCEPTIONS || DBJ_VECTOR_HAS_EXCEPTIONS
#include <stdexcept>
#endif

// seting both to 1 will make for a "proper" std::vector interface
// set to 1 to have reverse iterators  and associated methods
#define _DBJ_VEC_REVERSING 0
// set to 1 to have insertion methods
#define _DBJ_VEC_INSERTING 0

namespace dbj::nanolib::containers
{

	inline constexpr auto _DBJ_INC_DBJ_VECTOR_MAX_SZ = 0xFFFF;

	template <class T>
	class vector;

	template <class T>
	class vector
	{
		// same as size * 4 but '<< 2' is faster
		// convenient place to do assertions too
		constexpr static size_t rezerved(size_t size_) noexcept
		{
			_ASSERTE(size_ < _DBJ_INC_DBJ_VECTOR_MAX_SZ);
			_ASSERTE((size_ << 2) < _DBJ_INC_DBJ_VECTOR_MAX_SZ);
			return (size_ << 2);
		}

	public:
		// DBJ -- added
		using type = vector;
		// types:
		typedef T value_type;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T* iterator;
		typedef const T* const_iterator;
#if _DBJ_VEC_REVERSING
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
#endif // _DBJ_VEC_REVERSING
		typedef std::ptrdiff_t difference_type;
		typedef std::size_t size_type;

		vector() noexcept
			: data_(new T[rezerved_size_])
		{
		}

		vector(size_type n)
			: rezerved_size_( rezerved(n)),
			data_size_( n ) ,
			data_ ( new T[rezerved_size_] )
		{
			_ASSERTE( data_ );
			for (size_type i = 0; i < n; ++i)
				data_[i] = T();
		}

		vector(size_type n, const T& value)
			: rezerved_size_(rezerved(n)),
			data_size_(n),
			data_(new T[rezerved_size_])
		{
			_ASSERTE(data_);
			for (size_t i = 0; i < n; ++i)
				data_[i] = value;
		}

		vector(iterator first, iterator last)
			: rezerved_size_(rezerved(last - first)),
			data_size_(last - first),
			data_(new T[rezerved_size_])
		{
			for (size_t i = 0; i < data_size_; ++i, ++first)
				data_[i] = *first;
		}

		vector(const_iterator first, const_iterator last)
			: rezerved_size_(rezerved(last - first)),
			data_size_(last - first),
			data_(new T[rezerved_size_])
		{
			for (size_t i = 0; i < data_size_; ++i, ++first)
				data_[i] = *first;
		}

		vector(std::initializer_list<T> lst)
			: rezerved_size_(rezerved(lst.size())),
			data_size_(lst.size()),
			data_(new T[rezerved_size_])
		{
			size_t j{};
			for (auto& item : lst)
				data_[j++] = item;
		}

		vector(const vector<T>& other)
		{
			size_type i;
			rezerved_size_ = other.rezerved_size_;
			data_ = new T[rezerved_size_];
			for (i = 0; i < other.data_size_; ++i)
				data_[i] = other.data_[i];
			data_size_ = other.data_size_;
		}

		vector(vector<T>&& other) noexcept
		{
			size_type i;
			rezerved_size_ = other.rezerved_size_;
			data_ = new T[rezerved_size_];
			for (i = 0; i < other.data_size_; ++i)
				data_[i] = std::move(other.data_[i]);
			data_size_ = other.data_size_;
		}

		~vector()
		{
			if (data_) {
				delete[]  data_;
			}
		}

		type& operator=(const vector<T>& other)
		{
			size_type i;
			if (rezerved_size_ < other.data_size_)
			{
				rezerved_size_ = other.data_size_ << 2;
				reallocate();
			}
			for (i = 0; i < other.data_size_; ++i)
				data_[i] = other.data_[i];
			data_size_ = other.data_size_;
			return *this;
		}

		type& operator=(vector<T>&& other)
		{
			size_type i;
			if (rezerved_size_ < other.data_size_)
			{
				rezerved_size_ = other.data_size_ << 2;
				reallocate();
			}
			for (i = 0; i < other.data_size_; ++i)
				data_[i] = std::move(other.data_[i]);
			data_size_ = other.data_size_;

			return *this;
		}

		type& operator=(std::initializer_list<T> lst)
		{
			if (rezerved_size_ < lst.size())
			{
				rezerved_size_ = lst.size() << 2;
				reallocate();
			}
			data_size_ = 0;
			for (auto& item : lst)
				data_[data_size_++] = item;
			return *this;
		}

		void assign(size_type count, const T& value)
		{
			size_type i;
			if (count > rezerved_size_)
			{
				rezerved_size_ = count << 2;
				reallocate();
			}
			for (i = 0; i < count; ++i)
				data_[i] = value;
			data_size_ = count;
		}

		void assign(iterator first, iterator last)
		{
			size_type i, count = last - first;
			if (count > rezerved_size_)
			{
				rezerved_size_ = count << 2;
				reallocate();
			}
			for (i = 0; i < count; ++i, ++first)
				data_[i] = *first;
			data_size_ = count;
		}

		void assign(std::initializer_list<T> lst)
		{
			size_type i, count = lst.size();
			if (count > rezerved_size_)
			{
				rezerved_size_ = count << 2;
				reallocate();
			}
			i = 0;
			for (auto& item : lst)
				data_[i++] = item;
		}

		iterator begin() noexcept
		{
			return  data_;
		}

		const_iterator cbegin() const noexcept
		{
			return  data_;
		}

		iterator end() noexcept
		{
			return  data_ + data_size_;
		}

		const_iterator cend() const noexcept
		{
			return  data_ + data_size_;
		}

#if _DBJ_VEC_REVERSING

		reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(data_ + data_size_);
		}

		const_reverse_iterator crbegin() const noexcept
		{
			return reverse_iterator(data_ + data_size_);
		}

		reverse_iterator rend() noexcept
		{
			return reverse_iterator(data_);
		}

		const_reverse_iterator crend() const noexcept
		{
			return reverse_iterator(data_);
		}

#endif // _DBJ_VEC_REVERSING

		void reallocate()
		{
			T* tarr = new T[rezerved_size_];
			memcpy(tarr, data_, data_size_ * sizeof(T));
			delete[]  data_;
			data_ = tarr;
		}

		bool empty() const noexcept
		{
			return data_size_ == 0;
		}

		size_type size() const noexcept
		{
			return data_size_;
		}

		size_type max_size() const noexcept
		{
			return static_cast<size_type>(_DBJ_INC_DBJ_VECTOR_MAX_SZ);
		}

		size_type capacity() const noexcept
		{
			return rezerved_size_;
		}

		void resize(size_type sz)
		{
			if (sz > data_size_)
			{
				if (sz > rezerved_size_)
				{
					rezerved_size_ = sz;
					reallocate();
				}
			}
			else
			{
				size_type i;
				for (i = data_size_; i < sz; ++i)
					data_[i].~T();
			}
			data_size_ = sz;
		}

		void resize(size_type sz, const T& c)
		{
			if (sz > data_size_)
			{
				if (sz > rezerved_size_)
				{
					rezerved_size_ = sz;
					reallocate();
				}
				size_type i;
				for (i = data_size_; i < sz; ++i)
					data_[i] = c;
			}
			else
			{
				size_type i;
				for (i = data_size_; i < sz; ++i)
					data_[i].~T();
			}
			data_size_ = sz;
		}

		void reserve(size_type _sz)
		{
			if (_sz > rezerved_size_)
			{
				rezerved_size_ = _sz;
				reallocate();
			}
		}

		void shrink_to_fit()
		{
			rezerved_size_ = data_size_;
			reallocate();
		}

		reference operator[](size_type idx)
		{
			_ASSERTE(idx < data_size_); // dbj added
			return  data_[idx];
		}

		const_reference operator[](size_type idx) const
		{
			_ASSERTE(idx < data_size_); // dbj added
			return  data_[idx];
		}

		reference at(size_type pos)
		{
#if _HAS_EXCEPTIONS
			if (pos < data_size_)
				return  data_[pos];
			else
				throw std::out_of_range("accessed position is out of range");
#else
			_ASSERTE(pos < data_size_)
				return  data_[pos];
#endif
		}

		const_reference at(size_type pos) const
		{
#if _HAS_EXCEPTIONS
			if (pos < data_size_)
				return  data_[pos];
			else
				throw std::out_of_range("accessed position is out of range");
#else
			_ASSERTE(pos < data_size_)
				return  data_[pos];
#endif
		}

		reference front()
		{
			return  data_[0];
		}

		const_reference front() const
		{
			return  data_[0];
		}

		reference back()
		{
			return  data_[data_size_ - 1];
		}

		const_reference back() const
		{
			return  data_[data_size_ - 1];
		}

		T* data() noexcept
		{
			return  data_;
		}

		const T* data() const noexcept
		{
			return  data_;
		}

		template <class... Args>
		void emplace_back(Args&&... args)
		{
			if (data_size_ == rezerved_size_)
			{
				rezerved_size_ <<= 2;
				reallocate();
			}
			data_[data_size_] = std::move(T(std::forward<Args>(args)...));
			++data_size_;
		}

		void push_back(const T& val)
		{
			if (data_size_ == rezerved_size_)
			{
				rezerved_size_ <<= 2;
				reallocate();
			}
			data_[data_size_] = val;
			++data_size_;
		}

		void push_back(T&& val)
		{
			if (data_size_ == rezerved_size_)
			{
				rezerved_size_ <<= 2;
				reallocate();
			}
			data_[data_size_] = std::move(val);
			++data_size_;
		}

		void pop_back()
		{
			--data_size_;
			data_[data_size_].~T();
		}

		template <class... Args>
		iterator emplace(const_iterator it, Args&&... args)
		{
			iterator iit = &data_[it - data_];
			if (data_size_ == rezerved_size_)
			{
				rezerved_size_ <<= 2;
				reallocate();
			}
			memmove(iit + 1, iit, (data_size_ - (it - data_)) * sizeof(T));
			(*iit) = std::move(T(std::forward<Args>(args)...));
			++data_size_;
			return iit;
		}

#if _DBJ_VEC_INSERTING
		iterator insert(const_iterator it, const T& val)
		{
			iterator iit = &data_[it - data_];
			if (data_size_ == rezerved_size_)
			{
				rezerved_size_ <<= 2;
				reallocate();
			}
			memmove(iit + 1, iit, (data_size_ - (it - data_)) * sizeof(T));
			(*iit) = val;
			++data_size_;
			return iit;
		}

		iterator insert(const_iterator it, T&& val)
		{
			iterator iit = &data_[it - data_];
			if (data_size_ == rezerved_size_)
			{
				rezerved_size_ <<= 2;
				reallocate();
			}
			memmove(iit + 1, iit, (data_size_ - (it - data_)) * sizeof(T));
			(*iit) = std::move(val);
			++data_size_;
			return iit;
		}

		iterator insert(const_iterator it, size_type cnt, const T& val)
		{
			iterator f = &data_[it - data_];
			if (!cnt)
				return f;
			if (data_size_ + cnt > rezerved_size_)
			{
				rezerved_size_ = (data_size_ + cnt) << 2;
				reallocate();
			}
			memmove(f + cnt, f, (data_size_ - (it - data_)) * sizeof(T));
			data_size_ += cnt;
			for (iterator it = f; cnt--; ++it)
				(*it) = val;
			return f;
		}

		template <class InputIt>
		iterator insert(const_iterator it, InputIt first, InputIt last)
		{
			iterator f = &data_[it - data_];
			size_type cnt = last - first;
			if (!cnt)
				return f;
			if (data_size_ + cnt > rezerved_size_)
			{
				rezerved_size_ = (data_size_ + cnt) << 2;
				reallocate();
			}
			memmove(f + cnt, f, (data_size_ - (it - data_)) * sizeof(T));
			for (iterator it = f; first != last; ++it, ++first)
				(*it) = *first;
			data_size_ += cnt;
			return f;
		}

		iterator insert(const_iterator it, std::initializer_list<T> lst)
		{
			size_type cnt = lst.size();
			iterator f = &data_[it - data_];
			if (!cnt)
				return f;
			if (data_size_ + cnt > rezerved_size_)
			{
				rezerved_size_ = (data_size_ + cnt) << 2;
				reallocate();
			}
			memmove(f + cnt, f, (data_size_ - (it - data_)) * sizeof(T));
			iterator iit = f;
			for (auto& item : lst)
			{
				(*iit) = item;
				++iit;
			}
			data_size_ += cnt;
			return f;
		}
#endif // _DBJ_VEC_INSERTING

		iterator erase(const_iterator it)
		{
			iterator iit = &data_[it - data_];
			(*iit).~T();
			memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(T));
			--data_size_;
			return iit;
		}

		iterator erase(const_iterator first, const_iterator last)
		{
			iterator f = &data_[first - data_];
			if (first == last)
				return f;
			for (; first != last; ++first)
				(*first).~T();
			memmove(f, last, (data_size_ - (last - data_)) * sizeof(T));
			data_size_ -= last - first;
			return f;
		}

		void swap(vector<T>& rhs)
		{
			size_t tvec_sz = data_size_,
				trsrv_sz = rezerved_size_;
			T* tarr = data_;

			data_size_ = rhs.data_size_;
			rezerved_size_ = rhs.rezerved_size_;
			data_ = rhs.data_;

			rhs.data_size_ = tvec_sz;
			rhs.rezerved_size_ = trsrv_sz;
			rhs.data_ = tarr;
		}

		void clear() noexcept
		{
			size_type i;
			for (i = 0; i < data_size_; ++i)
				data_[i].~T();
			data_size_ = 0;
		}

		bool operator==(const vector<T>& rhs) const
		{
			if (data_size_ != rhs.data_size_)
				return false;
			size_type i;
			for (i = 0; i < data_size_; ++i)
				if (data_[i] != rhs.data_[i])
					return false;
			return true;
		}

		bool operator!=(const vector<T>& rhs) const
		{
			if (data_size_ != rhs.data_size_)
				return true;
			size_type i;
			for (i = 0; i < data_size_; ++i)
				if (data_[i] != rhs.data_[i])
					return true;
			return false;
		}

		bool operator<(const vector<T>& rhs) const
		{
			size_type i, ub = data_size_ < rhs.data_size_ ? data_size_ : rhs.data_size_;
			for (i = 0; i < ub; ++i)
				if (data_[i] != rhs.data_[i])
					return  data_[i] < rhs.data_[i];
			return data_size_ < rhs.data_size_;
		}

		bool operator<=(const vector<T>& rhs) const
		{
			size_type i, ub = data_size_ < rhs.data_size_ ? data_size_ : rhs.data_size_;
			for (i = 0; i < ub; ++i)
				if (data_[i] != rhs.data_[i])
					return  data_[i] < rhs.data_[i];
			return data_size_ <= rhs.data_size_;
		}

		bool operator>(const vector<T>& rhs) const
		{
			size_type i, ub = data_size_ < rhs.data_size_ ? data_size_ : rhs.data_size_;
			for (i = 0; i < ub; ++i)
				if (data_[i] != rhs.data_[i])
					return  data_[i] > rhs.data_[i];
			return data_size_ > rhs.data_size_;
		}

		bool operator>=(const vector<T>& rhs) const
		{
			size_type i, ub = data_size_ < rhs.data_size_ ? data_size_ : rhs.data_size_;
			for (i = 0; i < ub; ++i)
				if (data_[i] != rhs.data_[i])
					return  data_[i] > rhs.data_[i];
			return data_size_ >= rhs.data_size_;
		}

	private:
		size_type rezerved_size_ = 4;
		size_type data_size_ = 0;
		T* data_{};
	}; // eof class dbj::vector

	/*
		optimization -- specialized methods for arythmetic types
		*/

	template <> inline
		void vector<bool>::resize(vector<bool>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<signed char>::resize(vector<signed char>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<unsigned char>::resize(vector<unsigned char>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<char>::resize(vector<char>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<short int>::resize(vector<short int>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<unsigned short int>::resize(vector<unsigned short int>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<int>::resize(vector<int>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<unsigned int>::resize(vector<unsigned int>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<long int>::resize(vector<long int>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<unsigned long int>::resize(vector<unsigned long int>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<long long int>::resize(vector<long long int>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<unsigned long long int>::resize(vector<unsigned long long int>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<float>::resize(vector<float>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<double>::resize(vector<double>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<long double>::resize(vector<long double>::size_type sz)
	{
		if (sz > rezerved_size_)
		{
			rezerved_size_ = sz;
			reallocate();
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<bool>::resize(vector<bool>::size_type sz, const bool& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<signed char>::resize(vector<signed char>::size_type sz, const signed char& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<unsigned char>::resize(vector<unsigned char>::size_type sz, const unsigned char& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<char>::resize(vector<char>::size_type sz, const char& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<short int>::resize(vector<short int>::size_type sz, const short int& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<unsigned short int>::resize(vector<unsigned short int>::size_type sz, const unsigned short int& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<int>::resize(vector<int>::size_type sz, const int& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<unsigned int>::resize(vector<unsigned int>::size_type sz, const unsigned int& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<long int>::resize(vector<long int>::size_type sz, const long int& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<unsigned long int>::resize(vector<unsigned long int>::size_type sz, const unsigned long int& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<long long int>::resize(vector<long long int>::size_type sz, const long long int& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<unsigned long long int>::resize(vector<unsigned long long int>::size_type sz, const unsigned long long int& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<float>::resize(vector<float>::size_type sz, const float& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<double>::resize(vector<double>::size_type sz, const double& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<long double>::resize(vector<long double>::size_type sz, const long double& c)
	{
		if (sz > data_size_)
		{
			if (sz > rezerved_size_)
			{
				rezerved_size_ = sz;
				reallocate();
			}
			size_type i;
			for (i = data_size_; i < sz; ++i)
				data_[i] = c;
		}
		data_size_ = sz;
	}

	template <> inline
		void vector<bool>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<signed char>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<unsigned char>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<char>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<short int>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<unsigned short int>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<int>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<unsigned int>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<long int>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<unsigned long int>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<long long int>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<unsigned long long int>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<float>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<double>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		void vector<long double>::pop_back()
	{
		--data_size_;
	}

	template <> inline
		vector<bool>::iterator vector<bool>::erase(vector<bool>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(bool));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<signed char>::iterator vector<signed char>::erase(vector<signed char>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(signed char));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<unsigned char>::iterator vector<unsigned char>::erase(vector<unsigned char>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(unsigned char));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<char>::iterator vector<char>::erase(vector<char>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(char));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<wchar_t>::iterator vector<wchar_t>::erase(vector<wchar_t>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(wchar_t));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<short int>::iterator vector<short int>::erase(vector<short int>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(short int));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<unsigned short int>::iterator vector<unsigned short int>::erase(vector<unsigned short int>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(unsigned short int));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<int>::iterator vector<int>::erase(vector<int>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(int));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<unsigned int>::iterator vector<unsigned int>::erase(vector<unsigned int>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(unsigned int));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<long int>::iterator vector<long int>::erase(vector<long int>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(long int));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<unsigned long int>::iterator vector<unsigned long int>::erase(vector<unsigned long int>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(unsigned long int));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<long long int>::iterator vector<long long int>::erase(vector<long long int>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(long long int));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<unsigned long long int>::iterator vector<unsigned long long int>::erase(vector<unsigned long long int>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(unsigned long long int));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<float>::iterator vector<float>::erase(vector<float>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(float));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<double>::iterator vector<double>::erase(vector<double>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(double));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<long double>::iterator vector<long double>::erase(vector<long double>::const_iterator it)
	{
		iterator iit = &data_[it - data_];
		memmove(iit, iit + 1, (data_size_ - (it - data_) - 1) * sizeof(long double));
		--data_size_;
		return iit;
	}

	template <> inline
		vector<bool>::iterator vector<bool>::erase(vector<bool>::const_iterator first, vector<bool>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(bool));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<signed char>::iterator vector<signed char>::erase(vector<signed char>::const_iterator first, vector<signed char>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(signed char));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<unsigned char>::iterator vector<unsigned char>::erase(vector<unsigned char>::const_iterator first, vector<unsigned char>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(unsigned char));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<char>::iterator vector<char>::erase(vector<char>::const_iterator first, vector<char>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(char));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<wchar_t>::iterator vector<wchar_t>::erase(vector<wchar_t>::const_iterator first, vector<wchar_t>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(wchar_t));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<short int>::iterator vector<short int>::erase(vector<short int>::const_iterator first, vector<short int>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(short int));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<unsigned short int>::iterator vector<unsigned short int>::erase(vector<unsigned short int>::const_iterator first, vector<unsigned short int>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(unsigned short int));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<int>::iterator vector<int>::erase(vector<int>::const_iterator first, vector<int>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(int));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<unsigned int>::iterator vector<unsigned int>::erase(vector<unsigned int>::const_iterator first, vector<unsigned int>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(unsigned int));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<long long int>::iterator vector<long long int>::erase(vector<long long int>::const_iterator first, vector<long long int>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(long long int));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<unsigned long long int>::iterator vector<unsigned long long int>::erase(vector<unsigned long long int>::const_iterator first, vector<unsigned long long int>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(unsigned long long int));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<float>::iterator vector<float>::erase(vector<float>::const_iterator first, vector<float>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(float));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<double>::iterator vector<double>::erase(vector<double>::const_iterator first, vector<double>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(double));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		vector<long double>::iterator vector<long double>::erase(vector<long double>::const_iterator first, vector<long double>::const_iterator last)
	{
		iterator f = &data_[first - data_];
		if (first == last)
			return f;
		memmove(f, last, (data_size_ - (last - data_)) * sizeof(long double));
		data_size_ -= last - first;
		return f;
	}

	template <> inline
		void vector<bool>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<signed char>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<unsigned char>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline  void vector<char>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline  void vector<wchar_t>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<short int>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<unsigned short int>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<int>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<unsigned int>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<long int>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<unsigned long int>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<long long int>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<unsigned long long int>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<float>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<double>::clear() noexcept
	{
		data_size_ = 0;
	}

	template <> inline
		void vector<long double>::clear() noexcept
	{
		data_size_ = 0;
	}

} // namespace dbj::nanolib::containers

#endif // _DBJ_INC_DBJ_VECTOR_
