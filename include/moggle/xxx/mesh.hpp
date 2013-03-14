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

#include <memory>

#include "buffer.hpp"
#include "vertices.hpp"
#include "shader_pipeline.hpp"

namespace moggle {

// TODO: Put this somewhere logical and use it in moggle::vertices as well.
template<typename T>
class implicit_shared {
private:
	std::shared_ptr<T> p;
public:
	implicit_shared(std::nullptr_t) : p(nullptr) {}
	implicit_shared(std::shared_ptr<T> const & p) : p(p) {}
	implicit_shared(T && v) : p(std::make_shared<T>(std::move(v))) {}
	operator std::shared_ptr<T> const & () const { return p; }
};

class mesh {

private:
	std::shared_ptr<class vertices> vertices_;
	std::shared_ptr<buffer<GLushort>> indices_;

public:
	explicit mesh(
		implicit_shared<class vertices> v,
		implicit_shared<buffer<GLushort>> i = nullptr
	) : vertices_(v), indices_(i) {}

	std::shared_ptr<class vertices> vertices() { return vertices_; }

	void draw() const {
		GLuint i = 0;
		for (auto const & a : pipeline::active_pipeline()->vertex_attributes()) {
			vertices_->attribute(a.name).use(i++); // TODO: check type
		}
		vertices_->bind();
		if (indices_) {
			indices_->sync();
			indices_->bind(GL_ELEMENT_ARRAY_BUFFER);
			gl::draw_elements(GL_TRIANGLES, indices_->size(), GL_UNSIGNED_SHORT, nullptr);
		} else {
			// TODO: gl::draw_arrays(GL_TRIANGLES, 0, vertices_->size());
			// There is no vertices::size().
		}
	}

};

}
