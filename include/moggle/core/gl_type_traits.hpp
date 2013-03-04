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

#include "gl.hpp"

namespace moggle {

template<typename>
struct gl_type_traits;

#define X(T, C) \
	template<> \
	struct gl_type_traits<T> { \
		static constexpr GLenum gl_constant = C; \
	}

X(GLfloat , GL_FLOAT         );
X(GLdouble, GL_DOUBLE        );
X(GLuint  , GL_UNSIGNED_INT  );
X(GLint   , GL_INT           );
X(GLbyte  , GL_BYTE          );
X(GLubyte , GL_UNSIGNED_BYTE );
X(GLshort , GL_SHORT         );
X(GLushort, GL_UNSIGNED_SHORT);

#undef X

}
