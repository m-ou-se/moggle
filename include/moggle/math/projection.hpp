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
namespace projection_matrices {

	inline matrix<float, 4> frustrum(
		float l, //left
		float r, //right
		float b, //bottom
		float t, //top
		float n, //near (positive value, near plane will be at -n)
		float f  //far  (positive value, far plane will be at -f)
	) {
		return {
			2 * n / (r-l), 0            , (r+l) / (r-l), 0                ,
			0            , 2 * n / (t-b), (t+b) / (t-b), 0                ,
			0            , 0            , (n+f) / (n-f), 2 * n * f / (n-f),
			0            , 0            , -1           , 0
		};
	}

	inline matrix<float, 4> perspective(
		float field_of_view,
		float aspect_ratio,
		float near,
		float far
	) {
		float h = std::tan(field_of_view / 360.0f * std::acos(-1.0f)) * near;
		float w = h * aspect_ratio;
		return frustrum(-w, w, -h, h, near, far);
	}

	inline matrix<float, 4> orthographic(
		float l, //left
		float r, //right
		float b, //bottom
		float t, //top
		float n, //near (positive value, near plane will be at -n)
		float f  //far  (positive value, far plane will be at -f)
	) {
		return {
			2 / (r-l), 0        , 0        , (l+r) / (l-r),
			0        , 2 / (t-b), 0        , (b+t) / (b-t),
			0        , 0        , 2 / (n-f), (n+f) / (n-f),
			0        , 0        , 0        , 1
		};
	}

}
}
