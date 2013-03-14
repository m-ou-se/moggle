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

#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>

#include "gl.hpp"
#include "../math/matrix.hpp"

namespace moggle {

enum class shader_type : GLenum {
	vertex = GL_VERTEX_SHADER,
	fragment = GL_FRAGMENT_SHADER
};

class shader {

private:
	GLuint id = 0;

	friend class shader_program;

public:
	shader() : id(0) {}

	explicit shader(shader_type type) {
		create(type);
	}

	~shader() { destroy(); }

	shader(shader && s) : id(s.id) { s.id = 0; }
	shader & operator = (shader && s) { std::swap(id, s.id); return *this; }

	shader(shader const &) = delete;
	shader & operator = (shader const &) = delete;

	bool created() const { return id; }
	explicit operator bool() const { return created(); }

	void create(shader_type t) {
		if (id && type() != t) destroy();
		if (!id) id = gl::create_shader(GLenum(t));
	}

	void destroy() {
		gl::delete_shader(id);
		id = 0;
	}

	shader_type type() const {
		GLint t;
		gl::get_shader_iv(id, GL_SHADER_TYPE, &t);
		return shader_type(t);
	}

	static shader from_source(shader_type type, char const * source) {
		shader shader(type);
		shader.load(source);
		shader.compile();
		return shader;
	}

	static shader from_source(shader_type type, std::string const & source) {
		return from_source(type, source.data());
	}

	static shader from_file(shader_type type, char const * file_name) {
		std::ifstream f(file_name);
		if (!f) throw std::runtime_error(std::string("Unable to open file: ") + file_name);
		return from_source(
			type,
			{ std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() }
		);
	}

	static shader from_file(shader_type type, std::string const & source) {
		return from_file(type, source.data());
	}

	void load(std::string const & source) {
		load(source.data());
	}

	void load(char const * source) {
		gl::shader_source(id, 1, &source, nullptr);
	}

	bool compiled() const {
		GLint status;
		gl::get_shader_iv(id, GL_COMPILE_STATUS, &status);
		return status != GL_FALSE;
	}

	std::string log() const {
		GLint log_size;
		gl::get_shader_iv(id, GL_INFO_LOG_LENGTH, &log_size);
		std::string log(log_size, ' ');
		if (log_size) gl::get_shader_info_log(id, log_size, nullptr, &log[0]);
		return log;
	}

	void try_compile() {
		gl::compile_shader(id);
	}

	void compile() {
		try_compile();
		if (!compiled()) throw gl_error{"gl::compile_shader", "Unable to compile shader:\n" + log()};
	}

};

template<typename T> class shader_uniform_setter;

class shader_program {

private:
	GLuint id = 0;

public:
	explicit shader_program(bool create_now = false) {
		if (create_now) create();
	}

	~shader_program() { destroy(); }

	shader_program(shader_program && s) : id(s.id) { s.id = 0; }
	shader_program & operator = (shader_program && s) { std::swap(id, s.id); return *this; }

	shader_program(shader_program const &) = delete;
	shader_program & operator = (shader_program const &) = delete;

	bool created() const { return id; }
	explicit operator bool() const { return created(); }

	void create() {
		if (!id) id = gl::create_program();
	}

	void destroy() {
		gl::delete_program(id);
		id = 0;
	}

	void clear() {
		destroy();
		create();
	}

	void attach(shader const & shader) {
		create();
		gl::attach_shader(id, shader.id);
	}

	bool linked() const {
		GLint status;
		gl::get_program_iv(id, GL_LINK_STATUS, &status);
		return status != GL_FALSE;
	}

	std::string log() const {
		GLint log_size;
		gl::get_program_iv(id, GL_INFO_LOG_LENGTH, &log_size);
		std::string log(log_size, ' ');
		if (log_size) gl::get_program_info_log(id, log_size, nullptr, &log[0]);
		return log;
	}

	void try_link() {
		gl::link_program(id);
	}

	void link() {
		try_link();
		if (!linked()) throw gl_error{"gl::link_program", "Unable to link program:\n" + log()};
	}

	void bind_attribute(GLuint attribute, char const * name) {
		gl::bind_attribute_location(id, attribute, name);
	}

	void bind_attribute(GLuint attribute, std::string const & name) {
		bind_attribute(attribute, name.data());
	}

	void use() const {
		gl::use_program(id);
	}

	template<typename T>
	shader_uniform_setter<T> uniform(char const * name) const {
		return { gl::get_uniform_location(id, name) };
	}

	template<typename T>
	shader_uniform_setter<T> uniform(std::string const & name) const {
		return uniform<T>(name.data());
	}

};

#define X(T,...)                                    \
	template<> class shader_uniform_setter<T> {     \
		GLint id;                                   \
		shader_uniform_setter(GLint id) : id(id) {} \
		friend class shader_program;                \
	public:                                         \
		void set(T const & v) {                     \
			__VA_ARGS__;                            \
		}                                           \
	}

X(GLfloat, gl::uniform_1f (id, v));
X(GLint  , gl::uniform_1i (id, v));
X(GLuint , gl::uniform_1ui(id, v));

X(vector2<GLfloat>, gl::uniform_2fv (id, 1, v.data()));
X(vector3<GLfloat>, gl::uniform_3fv (id, 1, v.data()));
X(vector4<GLfloat>, gl::uniform_4fv (id, 1, v.data()));
X(vector2<GLint  >, gl::uniform_2iv (id, 1, v.data()));
X(vector3<GLint  >, gl::uniform_3iv (id, 1, v.data()));
X(vector4<GLint  >, gl::uniform_4iv (id, 1, v.data()));
X(vector2<GLuint >, gl::uniform_2uiv(id, 1, v.data()));
X(vector3<GLuint >, gl::uniform_3uiv(id, 1, v.data()));
X(vector4<GLuint >, gl::uniform_4uiv(id, 1, v.data()));

X(matrix2<GLfloat>, gl::uniform_matrix_2fv(id, 1, GL_TRUE, v.data()));
X(matrix3<GLfloat>, gl::uniform_matrix_3fv(id, 1, GL_TRUE, v.data()));
X(matrix4<GLfloat>, gl::uniform_matrix_4fv(id, 1, GL_TRUE, v.data()));

X(matrix3x2<GLfloat>, gl::uniform_matrix_2x3fv(id, 1, GL_TRUE, v.data()));
X(matrix2x3<GLfloat>, gl::uniform_matrix_3x2fv(id, 1, GL_TRUE, v.data()));
X(matrix4x2<GLfloat>, gl::uniform_matrix_2x4fv(id, 1, GL_TRUE, v.data()));
X(matrix2x4<GLfloat>, gl::uniform_matrix_4x2fv(id, 1, GL_TRUE, v.data()));
X(matrix4x3<GLfloat>, gl::uniform_matrix_3x4fv(id, 1, GL_TRUE, v.data()));
X(matrix3x4<GLfloat>, gl::uniform_matrix_4x3fv(id, 1, GL_TRUE, v.data()));

#undef X

}
