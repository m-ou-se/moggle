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

#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>

#include "gl.hpp"
#include "../math/matrix.hpp"

namespace moggle {

enum class shader_type : GLenum {
	uninitialized = 0,
	vertex = GL_VERTEX_SHADER,
	fragment = GL_FRAGMENT_SHADER
};

class shader {

private:
	GLuint id;
	shader_type type;

	friend class shader_program;

	shader() : id(0), type(shader_type::uninitialized) {}

	explicit shader(shader_type type) : id(glCreateShader(GLenum(type))), type(type) {
		if (!id) throw std::runtime_error("Unable to create shader.");
	}

public:
	~shader() {
		glDeleteShader(id);
	}

	shader(shader && s) : shader() {
		*this = std::move(s);
	}

	shader & operator = (shader && s) {
		this->~shader();
		id = s.id;
		type = s.type;
		s.id = 0;
		s.type = shader_type::uninitialized;
		return *this;
	}

	shader(shader const &) = delete;
	shader & operator = (shader const &) = delete;

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

private:
	void load(std::string const & source) {
		load(source.data());
	}

	void load(char const * source) {
		glShaderSource(id, 1, &source, nullptr);
	}

	void compile() {
		glCompileShader(id);
		GLint status;
		glGetShaderiv(id, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			std::string error = "Unable to compile shader.";
			GLint log_size;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_size);
			if (log_size) {
				std::vector<char> log(log_size);
				glGetShaderInfoLog(id, log_size, nullptr, log.data());
				error.back() = ':';
				error += '\n';
				error += log.data();
			}
			throw std::runtime_error(error);
		}
	}

};

template<typename T> class shader_uniform_setter;

class shader_program {

private:
	GLuint id;

	static GLuint create() {
		GLuint id = glCreateProgram();
		if (!id) throw std::runtime_error("Unable to create shader program.");
		return id;
	}

public:
	shader_program() : id(create()) {}

	~shader_program() {
		glDeleteProgram(id);
	}

	shader_program(shader_program && s) : shader_program() {
		*this = std::move(s);
	}

	shader_program & operator = (shader_program && s) {
		std::swap(id, s.id);
		return *this;
	}

	shader_program(shader_program const &) = delete;
	shader_program & operator = (shader_program const &) = delete;

	void clear() {
		glDeleteProgram(id);
		id = create();
	}

	void attach(shader const & shader) {
		glAttachShader(id, shader.id);
	}

	void link() {
		glLinkProgram(id);
		GLint status;
		glGetProgramiv(id, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			std::string error = "Unable to link shader program.";
			GLint log_size;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_size);
			if (log_size) {
				std::vector<char> log(log_size);
				glGetProgramInfoLog(id, log_size, nullptr, log.data());
				error.back() = ':';
				error += '\n';
				error += log.data();
			}
			throw std::runtime_error(error);
		}
	}

	void bind_attribute(GLuint attribute, char const * name) {
		glBindAttribLocation(id, GLuint(attribute), name);
	}

	void bind_attribute(GLuint attribute, std::string const & name) {
		bind_attribute(attribute, name.data());
	}

	void use() const {
		glUseProgram(id);
	}

	template<typename T>
	shader_uniform_setter<T> uniform(char const * name) const {
		return { glGetUniformLocation(id, name) };
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

X(GLfloat, glUniform1f (id, v));
X(GLint  , glUniform1i (id, v));
X(GLuint , glUniform1ui(id, v));

X(vector2<GLfloat>, glUniform2fv (id, 1, v.data()));
X(vector3<GLfloat>, glUniform3fv (id, 1, v.data()));
X(vector4<GLfloat>, glUniform4fv (id, 1, v.data()));
X(vector2<GLint  >, glUniform2iv (id, 1, v.data()));
X(vector3<GLint  >, glUniform3iv (id, 1, v.data()));
X(vector4<GLint  >, glUniform4iv (id, 1, v.data()));
X(vector2<GLuint >, glUniform2uiv(id, 1, v.data()));
X(vector3<GLuint >, glUniform3uiv(id, 1, v.data()));
X(vector4<GLuint >, glUniform4uiv(id, 1, v.data()));

X(matrix2<GLfloat>, glUniformMatrix2fv(id, 1, GL_TRUE, v.data()));
X(matrix3<GLfloat>, glUniformMatrix3fv(id, 1, GL_TRUE, v.data()));
X(matrix4<GLfloat>, glUniformMatrix4fv(id, 1, GL_TRUE, v.data()));

X(matrix3x2<GLfloat>, glUniformMatrix2x3fv(id, 1, GL_TRUE, v.data()));
X(matrix2x3<GLfloat>, glUniformMatrix3x2fv(id, 1, GL_TRUE, v.data()));
X(matrix4x2<GLfloat>, glUniformMatrix2x4fv(id, 1, GL_TRUE, v.data()));
X(matrix2x4<GLfloat>, glUniformMatrix4x2fv(id, 1, GL_TRUE, v.data()));
X(matrix4x3<GLfloat>, glUniformMatrix3x4fv(id, 1, GL_TRUE, v.data()));
X(matrix3x4<GLfloat>, glUniformMatrix4x3fv(id, 1, GL_TRUE, v.data()));

#undef X

}
