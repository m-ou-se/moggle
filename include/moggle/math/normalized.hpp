// Copyright 2013 Maurice Bos
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

#include <limits>
#include <type_traits>
#include <cstdint>

namespace moggle {

template<typename T, typename F = float>
class normalized_type {

	static_assert(std::is_integral<T>(), "This only works on integral types.");

private:
	T value_ = 0;

	constexpr static F scale_ = std::numeric_limits<T>::max();

	constexpr static F cap_(F v) {
		return v > 1 ? 1 : v < 0 && std::is_unsigned<T>() ? 0 : v < -1 ? -1 : v;
	}

	struct raw_tag_ {};

	constexpr normalized_type(raw_tag_, T v) : value_(v) {}

public:
	constexpr normalized_type() {}
	constexpr normalized_type(F v) : value_(cap_(v) * scale_) {}
	constexpr operator F () const { return value_ / scale_; }

	constexpr static normalized_type raw(T v) { return { raw_tag_(), v }; }

	constexpr T raw() const { return value_; }
	T & raw() { return value_; }

	normalized_type & operator += (F v) { return *this = *this + v; }
	normalized_type & operator -= (F v) { return *this = *this - v; }
	normalized_type & operator *= (F v) { return *this = *this * v; }
	normalized_type & operator /= (F v) { return *this = *this / v; }

};

template<typename T>
struct normalized_type_traits {
	constexpr static bool is_normalized_type = false;
	using raw_type = T;
};

template<typename T>
struct normalized_type_traits<normalized_type<T>> {
	constexpr static bool is_normalized_type = true;
	using raw_type = T;
};

using normalized_int8_t = normalized_type<int8_t>;
using normalized_int16_t = normalized_type<int16_t>;
using normalized_int32_t = normalized_type<int32_t>;
using normalized_int64_t = normalized_type<int64_t>;
using normalized_uint8_t = normalized_type<uint8_t>;
using normalized_uint16_t = normalized_type<uint16_t>;
using normalized_uint32_t = normalized_type<uint32_t>;
using normalized_uint64_t = normalized_type<uint64_t>;

}
