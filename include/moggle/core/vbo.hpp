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

#include <utility>
#include <vector>

#include "gl.hpp"

namespace moggle {

template<typename> class vbo;

class generic_vbo {

private:
	mutable GLuint id = 0;

public:
	explicit generic_vbo(bool create_now = false) {
		if (create_now) create();
	}

	~generic_vbo() { destroy(); }

	generic_vbo(generic_vbo const &) = delete;
	generic_vbo & operator = (generic_vbo const &) = delete;

	generic_vbo(generic_vbo && v) : id(v.id) { v.id = 0; }
	generic_vbo & operator = (generic_vbo && v) { std::swap(id, v.id); return *this; }

	bool created() const { return id; }
	explicit operator bool() const { return created(); }

	void create() const { if (!id) gl::generate_buffers(1, &id); }
	void destroy() { gl::delete_buffers(1, &id); id = 0; }

	void bind(GLenum buffer) const {
		create();
		gl::bind_buffer(buffer, id);
	}

};

template<typename T>
class vbo_mapping {
private:
	T * const data_;
	template<typename> friend class vbo;
	vbo_mapping(void * data) : data_(static_cast<T *>(data)) {}
public:
	T * data() const { return data_; }
	~vbo_mapping() { gl::unmap_buffer(GL_ARRAY_BUFFER); }
};

template<typename T>
class vbo : public generic_vbo {

public:
	explicit vbo(bool create_now = false) : generic_vbo(create_now) {}

	vbo(T const * begin, T const * end, GLenum usage = GL_STATIC_DRAW) {
		data(begin, end, usage);
	}

	vbo(T const * begin, size_t size, GLenum usage = GL_STATIC_DRAW) {
		data(begin, size, usage);
	}

	vbo(std::vector<T> const & v, GLenum usage = GL_STATIC_DRAW) {
		data(v, usage);
	}

	template<size_t N>
	vbo(std::array<T, N> const & v, GLenum usage = GL_STATIC_DRAW) {
		data(v, usage);
	}

	template<size_t N>
	vbo(T const (&v)[N], GLenum usage = GL_STATIC_DRAW) {
		data(v, usage);
	}

	vbo(std::initializer_list<T> list, GLenum usage = GL_STATIC_DRAW) {
		data(list, usage);
	}

	size_t size() const {
		GLint s;
		bind(GL_ARRAY_BUFFER);
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &s);
		return s / sizeof(T);
	}

	void data(T const * begin, size_t size, GLenum usage = GL_STATIC_DRAW) {
		bind(GL_ARRAY_BUFFER);
		gl::buffer_data(
			GL_ARRAY_BUFFER,
			size * sizeof(T),
			begin,
			usage
		);
	}

	void resize(size_t size, GLenum usage = GL_STATIC_DRAW) {
		bind(nullptr, size, usage);
	}

	void clear(GLenum usage = GL_STATIC_DRAW) {
		resize(0, usage);
	}

	void data(T const * begin, T const * end, GLenum usage = GL_STATIC_DRAW) {
		data(begin, end - begin, usage);
	}

	void data(std::vector<T> const & v, GLenum usage = GL_STATIC_DRAW) {
		data(v.data(), v.size(), usage);
	}

	template<size_t N>
	void data(std::array<T, N> const & v, GLenum usage = GL_STATIC_DRAW) {
		data(v.data(), N, usage);
	}

	template<size_t N>
	void data(T const (&v)[N], GLenum usage = GL_STATIC_DRAW) {
		data(v, N, usage);
	}

	void data(std::initializer_list<T> list, GLenum usage = GL_STATIC_DRAW) {
		data(list.begin(), list.size(), usage);
	}

	vbo_mapping<T const> map_read_only() const {
		bind(GL_ARRAY_BUFFER);
		return { gl::map_buffer(GL_ARRAY_BUFFER, GL_READ_ONLY) };
	}

	vbo_mapping<T> map_write_only() const {
		bind(GL_ARRAY_BUFFER);
		return { gl::map_buffer(GL_ARRAY_BUFFER, GL_READ_ONLY) };
	}

	vbo_mapping<T> map_read_write() const {
		bind(GL_ARRAY_BUFFER);
		return { gl::map_buffer(GL_ARRAY_BUFFER, GL_READ_ONLY) };
	}

};

}
