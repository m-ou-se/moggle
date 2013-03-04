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

// TODO: This file should include gl.h, or whatever that's called on the
// OS used. It also provides Moggle::gl_init() that does any initialisation
// required to use OpenGL. (Such as glewInit())
//
// Currently, it only works for glew, and assumes <GL/glew.h> exists.
// However, it should detect what to use on the current OS and include
// the right file(s) and provide the right initialisation function.

#include <cstdlib>

#include <GL/glew.h>

namespace moggle {
	inline void gl_init() {
		if (glewInit() != GLEW_OK) std::abort();
	}
}
