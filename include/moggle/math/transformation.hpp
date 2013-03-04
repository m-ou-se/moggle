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

#include "matrix.hpp"

namespace moggle {
namespace transformation_matrices {

	inline matrix<float, 4> translate(vector<float, 3> v) {
		return {
			   1,    0,    0, v[0],
			   0,    1,    0, v[1],
			   0,    0,    1, v[2],
			   0,    0,    0,    1,
		};
	}

	inline matrix<float, 4> scale(homogeneous_vector<float, 4> v) {
		return {
			v[0],    0,    0,    0,
			   0, v[1],    0,    0,
			   0,    0, v[2],    0,
			   0,    0,    0,  v[3]
		};
	}

	inline matrix<float, 4> scale(float s) {
		return scale({s, s, s});
	}

	inline matrix<float, 4> rotate(vector<float, 3> axis, float angle) {
		normalize(axis);
		auto & u = axis[0];
		auto & v = axis[1];
		auto & w = axis[2];
		float sin = std::sin(angle);
		float cos = std::cos(angle);
		return {
			  u*u+(1-u*u)*cos, u*v*(1-cos)-w*sin, u*w*(1-cos)+v*sin, 0,
			v*u*(1-cos)+w*sin,   v*v+(1-v*v)*cos, v*w*(1-cos)-u*sin, 0,
			w*u*(1-cos)-v*sin, w*v*(1-cos)+u*sin,   w*w+(1-w*w)*cos, 0,
			                0,                 0,                 0, 1
		};
	}

}
}
