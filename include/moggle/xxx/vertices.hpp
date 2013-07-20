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
#include <stdexcept>
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
		std::string n;
		attribute_ref(vertices & v, attribute_data const * a, std::string n)
			: v(v), a(a), n(std::move(n)) {}
		friend class vertices;
		void must_exist() const {
			if (!exists()) {
				std::string error{"No such attribute: " + name() + " (available attributes are: "};
				for (auto const & x : v.attributes()) {
					if (&x != &*v.attributes().begin()) error += ", ";
					error += x.first;
				}
				error += ")";
				throw attribute_error{error};
			}
		}
	public:
		bool exists() const { return a; }
		explicit operator bool () const { return exists(); }
		std::string const & name() const { return n; }
		void use(GLuint attribute_id) {
			must_exist();
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
		std::shared_ptr<class generic_buffer> generic_buffer() {
			must_exist();
			return a->buffer;
		}
		template<typename T>
		std::shared_ptr<moggle::buffer<T>> buffer() {
			must_exist();
			return std::dynamic_pointer_cast<moggle::buffer<T>>(a->buffer);
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

	// TODO: const_attribute_ref

	attribute_ref attribute(std::string const & name) {
		auto i = attributes_.find(name);
		if (i != attributes_.end()) return attribute(i);
		return { *this, nullptr, name };
	}

	attribute_ref attribute(decltype(attributes_)::const_iterator i) {
		return { *this, &i->second, i->first };
	}

	void bind() const {
		vao_.bind();
	}

};

}
