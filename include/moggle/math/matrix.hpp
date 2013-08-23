// Copyright 2013 Mara Bos
//
// This file is part of Moggle.
//
// Moggle is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Moggle is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Moggle. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <iostream>
#include <algorithm>
#include <array>
#include <initializer_list>
#include <cmath>

namespace moggle {

// {{{ matrix
template<typename T, size_t N, size_t M = N>
class matrix : public std::array<T, N * M> {

	using array = std::array<T, N * M>;

public:

	matrix() : array{{}} {}

	template<typename A, typename B, typename... C>
	matrix(A a, B b, C... c)
	: array{{ static_cast<T>(a), static_cast<T>(b), static_cast<T>(c)... }} {
		static_assert(sizeof...(c) + 2 <= N * M, "Too many elements.");
	}

	matrix(T const & v) {
		for (size_t i = 0; i < M; ++i)
		for (size_t j = 0; j < N; ++j) {
			(*this)(i, j) = i == j ? v : 0;
		}
	}

	template<typename T2, size_t N2, size_t M2>
	matrix(matrix<T2, N2, M2> const & m) : matrix() {
		*this = m;
	}

	template<typename... T2, size_t... N2>
	static matrix from_columns(matrix<T2, N2, 1>... columns) {
		static_assert(sizeof...(columns) <= M, "Too many columns.");
		matrix result;
		matrix<T, N, 1> c[M] = { columns... };
		for (size_t i = 0; i < M; ++i)
		for (size_t j = 0; j < N; ++j) {
			result(j, i) = c[i][j];
		}
		return result;
	}

	static matrix from_columns() { return {}; }

	template<typename... T2, size_t... M2>
	static matrix from_rows(matrix<T2, 1, M2>... rows) {
		static_assert(sizeof...(rows) <= N, "Too many rows.");
		matrix result;
		matrix<T, 1, M> r[N] = { rows... };
		for (size_t i = 0; i < N; ++i)
		for (size_t j = 0; j < M; ++j) {
			result(i, j) = r[i][j];
		}
		return result;
	}

	static matrix from_rows() { return {}; }

	static matrix identity() {
		matrix m;
		for (size_t i = 0; i < N && i < M; ++i) m(i, i) = 1;
		return m;
	}

	T       & operator () (size_t i)       { return operator () (i % N, i / N); }
	T const & operator () (size_t i) const { return operator () (i % N, i / N); }

	T       & operator () (size_t i, size_t j)       { return (*this)[i*M + j]; }
	T const & operator () (size_t i, size_t j) const { return (*this)[i*M + j]; }

	constexpr size_t  width() const { return M; };
	constexpr size_t height() const { return N; };

	template<typename T2, size_t N2, size_t M2>
	matrix & operator = (matrix<T2, N2, M2> const & m) {
		for (size_t i = 0; i < N && i < N2; ++i)
		for (size_t j = 0; j < M && j < M2; ++j) {
			(*this)(i, j) = m(i, j);
		}
		return *this;
	}

	template<size_t br, size_t er, size_t bc = 0, size_t ec = M>
	matrix<T, er - br, ec - br> slice() const {
		matrix<T, er - br, ec - bc> m;
		for (size_t i = br; i < er; ++i)
		for (size_t j = bc; j < ec; ++j) {
			m(i - br, j - bc) = (*this)(i, j);
		}
		return m;
	}

	matrix<T, N, 1> column(size_t c) const {
		matrix<T, N, 1> m;
		for (size_t i = 0; i < N; ++i) m[i] = (*this)(i, c);
		return m;
	}

	matrix<T, 1, M> row(size_t r) const {
		matrix<T, 1, M> m;
		for (size_t i = 0; i < M; ++i) m[i] = (*this)(r, i);
		return m;
	}

	matrix<T, N, M-1> without_column(size_t c) const {
		matrix<T, N, M-1> m;
		for (size_t i = 0; i < N; ++i)
		for (size_t j = 0; j < M-1; ++j) {
			m(i, j) = (*this)(i, j >= c ? j + 1 : j);
		}
		return m;
	}

	matrix<T, N-1, M> without_row(size_t r) const {
		matrix<T, N-1, M> m;
		for (size_t i = 0; i < N-1; ++i)
		for (size_t j = 0; j < M; ++j) {
			m(i, j) = (*this)(i >= r ? i + 1 : i, j);
		}
		return m;
	}

	matrix<T, N-1, M-1> without_row_column(size_t r, size_t c) const {
		matrix<T, N-1, M-1> m;
		for (size_t i = 0; i < N-1; ++i)
		for (size_t j = 0; j < M-1; ++j) {
			m(i, j) = (*this)(i >= r ? i + 1 : i, j >= c ? j + 1 : j);
		}
		return m;
	}

};
// }}}

// {{{ Typedefs: vector vectorN matrixN matrixNxM
template<typename T, size_t N> using vector = matrix<T, N, 1>;

template<typename T> using vector2 = vector<T, 2>;
template<typename T> using vector3 = vector<T, 3>;
template<typename T> using vector4 = vector<T, 4>;

template<typename T> using matrix2 = matrix<T, 2>;
template<typename T> using matrix3 = matrix<T, 3>;
template<typename T> using matrix4 = matrix<T, 4>;

template<typename T> using matrix2x2 = matrix<T, 2, 2>;
template<typename T> using matrix2x3 = matrix<T, 2, 3>;
template<typename T> using matrix2x4 = matrix<T, 2, 4>;
template<typename T> using matrix3x2 = matrix<T, 3, 2>;
template<typename T> using matrix3x3 = matrix<T, 3, 3>;
template<typename T> using matrix3x4 = matrix<T, 3, 4>;
template<typename T> using matrix4x2 = matrix<T, 4, 2>;
template<typename T> using matrix4x3 = matrix<T, 4, 3>;
template<typename T> using matrix4x4 = matrix<T, 4, 4>;
// }}}

// {{{ Homogeneous vector
template<typename T, size_t N>
class homogeneous_vector : public vector<T, N> {

private:
	static constexpr size_t Nm1 = N - 1; // Workaround for a g++ bug from september 2012.

public:
	homogeneous_vector() {
		(*this)[N-1] = 1;
	}

	template<typename A, typename B, typename... C>
	homogeneous_vector(A a, B b, C... c)
	: vector<T, N>{ a, b, c... } {
		static_assert(sizeof...(c) + 2 <= N, "Too many elements.");
		if (sizeof...(c) + 2 < N) (*this)[N-1] = 1;
	}

	template<typename T2>
	homogeneous_vector(vector<T2, N> const & v) {
		std::copy(v.begin(), v.end(), vector<T, N>::begin());
	}

	template<typename T2>
	homogeneous_vector(vector<T2, Nm1> const & v) {
		std::copy(v.begin(), v.end(), vector<T, N>::begin());
		(*this)[N-1] = 1;
	}

	template<typename T2>
	homogeneous_vector & operator = (vector<T2, Nm1> const & v) {
		std::copy(v.begin(), v.end(), vector<T, N>::begin());
		(*this)[N-1] = 1;
		return *this;
	}

};
// }}}

// {{{ Typedefs: hvector hvectorN
template<typename T, size_t N> using hvector = homogeneous_vector<T, N>;

template<typename T> using hvector2 = hvector<T, 2>;
template<typename T> using hvector3 = hvector<T, 3>;
template<typename T> using hvector4 = hvector<T, 4>;
// }}}

// {{{ matrix_traits
template<typename T>
struct matrix_traits {
	static constexpr bool is_matrix = false;
	static constexpr bool is_vector = false;
	static constexpr bool is_homogeneous = false;
	static constexpr size_t width = 1;
	static constexpr size_t height = 1;
	static constexpr size_t size = 1;
	using element_type = T;
};

template<typename T, size_t N, size_t M>
struct matrix_traits<matrix<T, N, M>> {
	static constexpr bool is_matrix = true;
	static constexpr bool is_vector = N == 1;
	static constexpr bool is_homogeneous = false;
	static constexpr size_t width = M;
	static constexpr size_t height = N;
	static constexpr size_t size = N * M;
	using element_type = T;
};

template<typename T, size_t N>
struct matrix_traits<homogeneous_vector<T, N>> {
	static constexpr bool is_matrix = true;
	static constexpr bool is_vector = true;
	static constexpr bool is_homogeneous = true;
	static constexpr size_t width = 1;
	static constexpr size_t height = N;
	static constexpr size_t size = N;
	using element_type = T;
};
// }}}

// {{{ Operators: +M -M M+=M M-=M M+M M-M

template<typename T, size_t N, size_t M>
matrix<T, N, M> const & operator + (matrix<T, N, M> const & m) {
	return m;
}

template<typename T, size_t N, size_t M>
matrix<T, N, M> operator - (matrix<T, N, M> const & m) {
	matrix<T, N, M> r;
	for (size_t i = 0; i < m.size(); ++i) r[i] = -m[i];
	return r;
}

template<typename T, size_t N, size_t M, typename T2, size_t N2, size_t M2>
matrix<T, N, M> & operator += (matrix<T, N, M> & a, matrix<T2, N2, M2> const & b) {
	static_assert(N2 <= N && M2 <= M, "Too big.");
	for (size_t i = 0; i < N2; ++i)
	for (size_t j = 0; j < M2; ++j) {
		a(i, j) += b(i, j);
	}
	return a;
}

template<typename T, size_t N, size_t M, typename T2, size_t N2, size_t M2>
matrix<T, N, M> & operator -= (matrix<T, N, M> & a, matrix<T2, N2, M2> const & b) {
	static_assert(N2 <= N && M2 <= M, "Too big.");
	for (size_t i = 0; i < N2; ++i)
	for (size_t j = 0; j < M2; ++j) {
		a(i, j) -= b(i, j);
	}
	return a;
}

template<typename T, size_t N, size_t M, typename T2, size_t N2, size_t M2>
matrix<decltype(T() + T2()), (N > N2 ? N : N2), (M > M2 ? M : M2)>
operator + (matrix<T, N, M> const & a, matrix<T2, N2, M2> const & b) {
	matrix<decltype(T() + T2()), (N > N2 ? N : N2), (M > M2 ? M : M2)> r = a;
	return r += b;
}

template<typename T, size_t N, size_t M, typename T2, size_t N2, size_t M2>
matrix<decltype(T() - T2()), (N > N2 ? N : N2), (M > M2 ? M : M2)>
operator - (matrix<T, N, M> const & a, matrix<T2, N2, M2> const & b) {
	matrix<decltype(T() - T2()), (N > N2 ? N : N2), (M > M2 ? M : M2)> r = a;
	return r -= b;
}
// }}}

// {{{ Operators: M*=S M/=S S*M M*S M/S
template<typename T, size_t N, size_t M, typename S>
typename std::enable_if<!matrix_traits<S>::is_matrix, matrix<T, N, M> &>::type
operator *= (matrix<T, N, M> & m, S const & s) {
	for (T & x : m) x *= s;
	return m;
}

template<typename T, size_t N, size_t M, typename S>
typename std::enable_if<!matrix_traits<S>::is_matrix, matrix<T, N, M> &>::type
operator /= (matrix<T, N, M> & m, S const & s) {
	for (T & x : m) x /= s;
	return m;
}

template<typename T, size_t N, size_t M, typename S>
typename std::enable_if<!matrix_traits<S>::is_matrix, matrix<decltype(T() * S()), N, M>>::type
operator * (matrix<T, N, M> const & m, S const & s) {
	matrix<decltype(T() * S()), N, M> r;
	for (size_t i = 0; i < m.size(); ++i) r[i] = m[i] * s;
	return r;
}

template<typename S, typename T, size_t N, size_t M>
typename std::enable_if<!matrix_traits<S>::is_matrix, matrix<decltype(S() * T()), N, M>>::type
operator * (S const & s, matrix<T, N, M> const & m) {
	matrix<decltype(S() * T()), N, M> r;
	for (size_t i = 0; i < m.size(); ++i) r[i] = s * m[i];
	return r;
}

template<typename T, size_t N, size_t M, typename S>
typename std::enable_if<!matrix_traits<S>::is_matrix, matrix<decltype(T() / S()), N, M>>::type
operator / (matrix<T, N, M> const & m, S const & s) {
	matrix<decltype(T() / S()), N, M> r;
	for (size_t i = 0; i < m.size(); ++i) r[i] = m[i] / s;
	return r;
}
// }}}

// {{{ Operators: M*M M*=M
template<typename T, size_t N, size_t M, typename T2, size_t M2>
matrix<decltype(T() * T2()), N, M2>
operator * (matrix<T, N, M> const & a, matrix<T2, M, M2> const & b) {
	matrix<decltype(T() * T2()), N, M2> result;
	for (size_t i = 0; i < N; ++i)
	for (size_t j = 0; j < M2; ++j) {
		T r = 0;
		for (size_t k = 0; k < M; ++k) r += a(i, k) * b(k, j);
		result(i, j) = r;
	}
	return result;
}

template<typename T, size_t N, size_t M, typename T2>
matrix<T, N, M> & operator *= (matrix<T, N, M> & a, matrix<T2, M, M> const & b) {
	return a = a * b;
}
// }}}

// {{{ Operators: V*=V V/=V V*V V/V
template<typename T, size_t N, typename T2, size_t N2>
vector<T, N> & operator *= (vector<T, N> & a, vector<T2, N2> const & b) {
	static_assert(N2 <= N, "Too big.");
	for (size_t i = 0; i < N2; ++i) a[i] *= b[i];
	return a;
}

template<typename T, size_t N, typename T2, size_t N2>
vector<T, N> & operator /= (vector<T, N> & a, vector<T2, N2> const & b) {
	static_assert(N2 <= N, "Too big.");
	for (size_t i = 0; i < N2; ++i) a[i] /= b[i];
	return a;
}

template<typename T, size_t N, typename T2, size_t N2>
vector<decltype(T() * T2()), (N > N2 ? N : N2)>
operator * (vector<T, N> const & a, vector<T2, N2> const & b) {
	vector<decltype(T() * T2()), (N > N2 ? N : N2)> r = a;
	return r *= b;
}

template<typename T, size_t N, typename T2, size_t N2>
vector<decltype(T() / T2()), (N > N2 ? N : N2)>
operator / (vector<T, N> const & a, vector<T2, N2> const & b) {
	vector<decltype(T() / T2()), (N > N2 ? N : N2)> r = a;
	return r /= b;
}
// }}}

// {{{ vector functions: dot length normalize normalized
template<typename T, size_t N>
T length(vector<T, N> const & v) {
	return std::sqrt(dot(v, v));
}

template<typename T, size_t N>
void normalize(vector<T, N> & v) {
	v /= length(v);
}

template<typename T, size_t N>
vector<T, N> normalized(vector<T, N> const & v) {
	return v / length(v);
}

template<typename T, size_t N, size_t M, typename T2, size_t N2, size_t M2>
decltype(T() * T2())
dot(matrix<T, N, M> const & a, matrix<T2, N2, M2> const & b) {
	decltype(T() * T2()) r = 0;
	for (size_t i = 0; i < N && i < N2; ++i)
	for (size_t j = 0; j < M && j < M2; ++j) {
		r += a(i, j) * b(i, j);
	}
	return r;
}
// }}}

// {{{ matrix functions: transpose transposed
template<typename T, size_t N>
void transpose(matrix<T, N, N> & m) {
	for (size_t i = 1; i < N; ++i)
	for (size_t j = 0; j < i; ++j) {
		swap(m(i, j), m(j, i));
	}
}

template<typename T, size_t N, size_t M>
matrix<T, M, N> transposed(matrix<T, N, M> const & m) {
	matrix<T, M, N> r;
	for (size_t i = 0; i < N; ++i)
	for (size_t j = 0; j < M; ++j) {
		r(j, i) = m(i, j);
	}
	return r;
}
// }}}

// {{{ matrix functions: determinant cofactor
template<typename T, size_t N>
static T determinant(matrix<T, N> const & m);

template<typename T, size_t N>
T cofactor(matrix<T, N> const & m, size_t row, size_t column) {
	auto d = determinant(m.without_row_column(row, column));
	return (row + column) % 2 ? -d : d;
}

template<typename T, size_t N>
struct determinant_ {
	static T determinant(matrix<T, N> const & m) {
		T result = 0;
		for (size_t i = 0; i < N; ++i) {
			result += m(0, i) * cofactor(m, 0, i);
		}
		return result;
	}
};

template<typename T>
struct determinant_<T, 1> {
	static T determinant(matrix<T, 1> const & m) {
		return m[0];
	}
};

template<typename T>
struct determinant_<T, 0> {
	static T determinant(matrix<T, 0> const &) {
		return 1;
	}
};

template<typename T, size_t N>
T determinant(matrix<T, N> const & m) {
	return determinant_<T, N>::determinant(m);
}
// }}}

// {{{ matrix functions: cofactor_matrix adjugate inverse invert
template<typename T, size_t N>
matrix<T, N> cofactor_matrix(matrix<T, N> const & m) {
	matrix<T, N> result;
	for (size_t i = 0; i < N; ++i)
	for (size_t j = 0; j < N; ++j) {
		result(i, j) = cofactor(m, i, j);
	}
	return result;
}

template<typename T, size_t N>
matrix<T, N> adjugate(matrix<T, N> const & m) {
	return transposed(cofactor_matrix(m));
}

template<typename T, size_t N>
matrix<T, N> inverse(matrix<T, N> const & m) {
	return (T(1) / determinant(m)) * adjugate(m);
}

template<typename T, size_t N>
void invert(matrix<T, N> & m) {
	m = inverse(m);
}
// }}}

// {{{ vector function: cross
namespace vector_cross_private {
	template<typename... T> struct result_type { using type = int; };
	template<typename T> struct result_type<T> { using type = T; };
	template<typename T, typename... Ts> struct result_type<T, Ts...> {
		using type = decltype(T() * typename result_type<Ts...>::type());
	};
}

/// Gives a N+1 dimensional vector perpendicular to the N given vectors.
/// \note Yes, it works for all dimensions.
template<typename... T, size_t... N>
vector<typename vector_cross_private::result_type<T...>::type, sizeof...(T) + 1>
cross(vector<T, N>... v) {
	constexpr size_t n = sizeof...(v);
	auto m = matrix<typename vector_cross_private::result_type<T...>::type, n, n+1>::from_rows(transposed(v)...);
	vector<float, n+1> result;
	for (size_t i = 0; i < n+1; ++i) {
		auto d = determinant(m.without_column(i));
		result[i] = (n + i) % 2 ? -d : d;
	}
	return result;
}
// }}}

// {{{ matrix/vector function: pointwise
template<
	typename... T,
	size_t N, size_t M,
	typename F,
	typename R =
		typename std::remove_cv<
			typename std::remove_reference<
				decltype(std::declval<F>()(std::declval<T>()...))
			>::type
		>::type
>
matrix<R, N, M>
pointwise(F f, matrix<T, N, M>... matrices) {
	matrix<R, N, M> result;
	for (size_t i = 0; i < N*M; ++i) result[i] = f(matrices[i]...);
	return result;
}
// }}}

// {{{ Output operator <<
template<typename T, size_t N, size_t M>
std::ostream & operator << (std::ostream & out, matrix<T, N, M> const & m) {
	out << '[';
	for (size_t i = 0; i < N; ++i) {
		if (i) out << ';';
		for (size_t j = 0; j < M; ++j) {
			if (i || j) out << ' ';
			out << m(i, j);
		}
	}
	return out << ']';
}
/// }}}

}
