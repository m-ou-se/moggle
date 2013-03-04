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

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include "../core/gl.hpp"
#include "../core/gl_type_traits.hpp"
#include "../core/vao.hpp"
#include "../core/vbo.hpp"
#include "buffer.hpp"

namespace moggle {

struct attribute_error : std::runtime_error {
	attribute_error(std::string const & s) : std::runtime_error(s) {}
};

class vertices {

public:
	struct attribute_data {
		std::shared_ptr<generic_buffer> buffer;
		std::uint8_t width;
		std::uint8_t height;
		GLenum type; // One of GL_FLOAT, GL_INT, etc.
		bool normalized;
		std::size_t size_of_type;
	};

private:
	vao vao_;
	std::map<std::string, attribute_data> attributes_;

public:
	class attribute_ref {
	private:
		vertices & v;
		attribute_data const * a;
		attribute_ref(vertices & v, attribute_data const * a) : v(v), a(a) {}
		friend class vertices;
	public:
		bool exists() const { return a; }
		explicit operator bool () const { return exists(); }
		void use(GLuint attribute_id) {
			if (!exists()) throw attribute_error("No such attribute.");
			a->buffer->sync(); // TODO: check if this line belongs here or somewhere else.
			v.vao_.attribute(
				attribute_id,
				*(a->buffer),
				a->width * a->height,
				a->type,
				a->normalized,
				a->size_of_type,
				nullptr
			);
		}
	};

	friend class attribute_ref;

public:
	std::map<std::string, attribute_data> const & attributes() const { return attributes_; }

	template<typename T>
	void attribute(std::string const & name, std::shared_ptr<buffer<T>> buf) {
		using mt = matrix_traits<T>;
		using nt = normalized_type_traits<typename mt::element_type>;
		attributes_[name] = {
			std::move(buf),
			mt::width,
			mt::height,
			gl_type_traits<typename nt::raw_type>::gl_constant,
			nt::is_normalized_type,
			sizeof(T)
		};
	}

	template<typename T>
	void attribute(std::string const & name, buffer<T> && buf) {
		attribute(name, buf.make_shared());
	}

	attribute_ref attribute(std::string const & name) {
		return attribute(attributes_.find(name));
	}

	attribute_ref attribute(decltype(attributes_)::const_iterator i) {
		return { *this, i == attributes_.end() ? nullptr : &i->second };
	}

	void bind() const {
		vao_.bind();
	}

};

}
