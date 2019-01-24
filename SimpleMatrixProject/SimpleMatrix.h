#pragma once
#ifndef SIMPLE_MATRIX_HPP
#define SIMPLE_MATRIX_HPP

#include<ostream>
#include<stdexcept>
#include<memory>

namespace my
{
	using Index = int;

	template<class T, class A = std::allocator<T>>
	class SimpleMatrix // a matrix that doesn't change its size
	{
	public:
		using allocator_type = typename std::allocator_traits<A>::template rebind_alloc<T>;
		using value_type = T;
		using size_type = Index;

		class SimpleMatrixIterator;
		class ConstSimpleMatrixIterator;

		using iterator = SimpleMatrixIterator;
		using const_iterator = ConstSimpleMatrixIterator;

		explicit SimpleMatrix(Index row, Index col) : sz{row, col}
		{
			if (row <= 0 || col <= 0)
			{
				throw std::out_of_range{ "count of rows and columns in matrix must be greater than zero" };
			}

			elem = reinterpret_cast<T**>(alloc.allocate(sizeof(T*) * sz.row + sizeof(T) * sz.row * sz.col));
			T* ptr = reinterpret_cast<T*>(elem + row);
			for (Index i = 0; i < row; ++i)
			{
				elem[i] = ptr + i * col;
				for (Index j = 0; j < col; ++j)
				{
					alloc.construct(&(elem[i][j]));
				}
			}
		}
		explicit SimpleMatrix(Index row, Index col, const T& val) : sz{ row, col }
		{
			if (row <= 0 || col <= 0)
			{
				throw std::out_of_range{ "count of rows and columns in matrix must be greater than zero" };
			}

			elem = reinterpret_cast<T**>(alloc.allocate(sizeof(T*) * sz.row + sizeof(T) * sz.row * sz.col));
			T* ptr = reinterpret_cast<T*>(elem + row);
			for (Index i = 0; i < row; ++i)
			{
				elem[i] = ptr + i * col;
				for (Index j = 0; j < col; ++j)
				{
					alloc.construct(&(elem[i][j]), val);
				}
			}
		}

		SimpleMatrix(const SimpleMatrix& other) : sz{ other.sz }
		{
			if (other.sz.row <= 0 || other.sz.col <= 0)
			{
				throw std::exception{ "other matrix is invalid" };
			}

			elem = reinterpret_cast<T**>(alloc.allocate(sizeof(T*) * sz.row + sizeof(T) * sz.row * sz.col));
			T* ptr = reinterpret_cast<T*>(elem + sz.row);
			this->assign(other);
		}
		SimpleMatrix& operator=(const SimpleMatrix& other)
		{
			if (this == &other)
			{
				return *this;
			}

			if (other.sz.row <= 0 || other.sz.col <= 0)
			{
				throw std::exception{ "other matrix is invalid" };
			}

			this->destroy_all();
			if (other.sz.row != this->sz.row || other.sz.col != this->sz.col)
			{
				this->alloc.deallocate(reinterpret_cast<char*>(elem), (sizeof(T*) * sz.row + sizeof(T) * sz.row * sz.col));

				this->sz = other.sz;
				elem = reinterpret_cast<T**>(alloc.allocate(sizeof(T*) * sz.row + sizeof(T) * sz.row * sz.col));
			}
			this->assign(other);

			return *this;
		}

		SimpleMatrix(SimpleMatrix&& other)
		{
			std::swap(this->sz, other.sz);
			std::swap(this->elem, other.elem);
			std::swap(this->alloc, other.alloc);
		}
		SimpleMatrix& operator=(SimpleMatrix&& other)
		{
			if (this == &other)
			{
				return *this;
			}

			std::swap(this->sz, other.sz);
			std::swap(this->elem, other.elem);
			std::swap(this->alloc, other.alloc);

			return *this;
		}

		~SimpleMatrix() 
		{ 
			destroy_all();
			alloc.deallocate(reinterpret_cast<char*>(elem), sizeof(T*) * sz.row + sizeof(T) * sz.row * sz.col);
		}

		inline T** data() noexcept { return elem; }
		inline const T* const* data() const noexcept { return elem; }

		inline Index count_rows() noexcept { return sz.row; }
		inline Index count_cols() noexcept { return sz.col; }

		inline T* operator[](Index row) noexcept { return elem[row]; }
		inline const T* operator[](Index row) const noexcept { return elem[row]; }

		inline T& at(Index row, Index col)
		{
			check_range(row, col);
			return elem[row][col];
		}
		inline const T& at(Index row, Index col) const
		{
			check_range(row, col);
			return elem[row][col];
		}

		void fill(const T& val)
		{
			for (Index i = 0; i < sz.row; ++i)
			{
				for (Index j = 0; j < sz.col; ++j)
				{
					elem[i][j] = val;
				}
			}
		}

		template<class It>
		void assign(It first, It last)
		{
			if (std::distance(first, last) != (sz.row*sz.col))
			{
				throw std::out_of_range{ "size of assignable range is not equal to matrix" };
			}

			for (Index i = 0; i < sz.row; ++i)
			{
				for (Index j = 0; j < sz.col; ++j)
				{
					elem[i][j] = *first;
					++first;
				}
			}
		}

		friend std::ostream& operator<<(std::ostream& os, const SimpleMatrix& mtx)
		{
			for (Index i = 0; i < mtx.sz.row; ++i)
			{
				os << "| ";
				for (Index j = 0; j < mtx.sz.col; ++j)
					os << mtx[i][j] << " ";

				os << "|" << std::endl;
			}
			return os;
		}

		iterator begin() { return iterator(elem, sz.col); }
		iterator end() { return iterator(elem + sz.row, sz.col); }

		const_iterator begin() const { return const_iterator(elem, sz.col); }
		const_iterator end() const { return const_iterator(elem + sz.row, sz.col); }

	private:
		inline void check_range(Index r, Index c)
		{
			if (r < 0 || r >= sz.row || 
				c < 0 || c >= sz.col)
			{
				throw std::out_of_range{ "index of matrix is out of range" };
			}
		}
		void destroy_all()
		{
			for (Index i = 0; i < sz.row; ++i)
				for (Index j = 0; j < sz.col; ++j)
					alloc.destroy(&(elem[i][j]));
		}
		void assign(const SimpleMatrix& other)
		{
			T* ptr = reinterpret_cast<T*>(elem + sz.row);
			for (Index i = 0; i < sz.row; ++i)
			{
				elem[i] = ptr + i * sz.col;
				for (Index j = 0; j < sz.col; ++j)
				{
					alloc.construct(&(elem[i][j]), other[i][j]);
				}
			}
		}

		struct matrix_size
		{
			Index row{};
			Index col{};
		} sz;
		T** elem{};

		using real_alloc_t = typename std::allocator_traits<A>::template rebind_alloc<char>;
		real_alloc_t alloc;
	};

	template<class T, class A>
	class SimpleMatrix<T, A>::SimpleMatrixIterator : public std::iterator<std::input_iterator_tag, T>
	{
		friend class SimpleMatrix<T, A>;
	private:
		SimpleMatrixIterator(T** p, Index s) noexcept : p(p), step(s - 1), left(0) {}
	public:
		SimpleMatrixIterator(const SimpleMatrixIterator &it) noexcept : p(it.p), step(it.step), left(it.left) {}
		SimpleMatrixIterator& operator =(const SimpleMatrixIterator&) = default;

		inline bool operator!=(SimpleMatrixIterator const& other) const noexcept { return p != other.p || left != other.left || step != other.step; }
		inline bool operator==(SimpleMatrixIterator const& other) const noexcept { return p == other.p && left == other.left && step == other.step; }
		inline T& operator*() const noexcept { return *(*p + left); }
		inline T* operator->() noexcept { return (*p + left); }
		inline SimpleMatrixIterator& operator++() noexcept
		{
			if (left >= step)
			{
				left = 0;
				++p;
				return *this;
			}
			left++;
			return *this;
		}
		inline SimpleMatrixIterator& operator++(int) noexcept
		{
			auto _tmp = *this;
			this->operator++();
			return _tmp;
		}
	private:
		T * * p;
		Index step{};
		Index left{};
	};

	template<class T, class A>
	class ConstSimpleMatrixIterator : public std::iterator<std::input_iterator_tag, T>
	{
		friend class SimpleMatrix<T, A>;
	private:
		ConstSimpleMatrixIterator(T** p, Index s) noexcept : p(p), step(s - 1), left(0) {}
	public:
		ConstSimpleMatrixIterator(const ConstSimpleMatrixIterator &it) noexcept : p(it.p), step(it.step), left(it.left) {}
		ConstSimpleMatrixIterator& operator =(const ConstSimpleMatrixIterator&) = default;

		inline bool operator!=(ConstSimpleMatrixIterator const& other) const noexcept { return p != other.p || left != other.left || step != other.step; }
		inline bool operator==(ConstSimpleMatrixIterator const& other) const noexcept { return p == other.p && left == other.left && step == other.step; }
		inline T const& operator*() const noexcept { return *(*p + left); }
		inline const T* operator->() noexcept { return (*p + left); }
		inline ConstSimpleMatrixIterator& operator++() noexcept
		{
			if (left >= step)
			{
				left = 0;
				++p;
				return *this;
			}
			left++;
			return *this;
		}
		inline ConstSimpleMatrixIterator& operator++(int) noexcept
		{
			auto _tmp = *this;
			this->operator++();
			return _tmp;
		}
	private:
		T** p;
		Index step{};
		Index left{};
	};

}

#endif // !SIMPLE_MATRIX_HPP
