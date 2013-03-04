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

#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <moggle/math/matrix.hpp>
#include <moggle/core/shader.hpp>
#include <moggle/xxx/vertices.hpp>

namespace moggle {

namespace shader_pipeline {

struct parse_error : std::runtime_error {
	parse_error(std::string const & e) : std::runtime_error(e) {}
};

struct compile_error : std::runtime_error {
	compile_error(std::string const & e) : std::runtime_error(e) {}
};

struct variable {
	std::string type;
	std::string name;
	variable() {}
	variable(std::string const & name) : name(name) {}
	variable(char const * name) : name(name) {}
	variable(std::string const & type, std::string const &  name) : type(type), name(name) {}
	bool operator < (variable const & other) const {
		return name < other.name;
	}
};

struct parameter : variable {
	bool in = false;
	bool out = false;
	parameter() {}
	parameter(std::string type, std::string name, bool in, bool out) : variable(type, name), in(in), out(out) {}
};

struct operation_details : variable {
	std::vector<parameter> parameters;
};

class context {

private:
	unsigned int glsl_version_ = 0;

	std::string code_;

	std::map<std::string, operation_details> operations_;

	std::set<std::string> used_files_;

public:
	explicit context(unsigned int glsl_version = 0) : glsl_version_(glsl_version) {}

	unsigned int glsl_version() const { return glsl_version_; }
	std::string const & code() const { return code_; }
	std::map<std::string, operation_details> const & operations() const { return operations_; };
	std::set<std::string> const & used_files() const { return used_files_; }

	void add_code(std::string const & code, std::string const & name = "");
	void add_code(std::istream &, std::string const & name = "");
	void add_file(std::string const & filename);

	/* TODO (used files_)
	context & operator += (context const & other) {
		if (other.glsl_version_ > glsl_version_) glsl_version_ = other.glsl_version_;
		code_ += '\n' + other.code_;
		operations_.insert(other.operations_.begin(), other.operations_.end());
		return *this;
	}

	context operator + (context const & other) {
		context x = *this;
		return x += other;
	}
	*/

};

class operation_parameter {

public:
	enum parameter_type {
		variable,
		constant
	};

private:
	std::string code_;
	parameter_type type_;

public:
	operation_parameter(std::string const & name) : code_(name), type_(variable) {}
	operation_parameter(char const * name) : code_(name), type_(variable) {}

	std::string const & code() const { return code_; }
	parameter_type type() const { return type_; }

	operation_parameter(float v) : type_(constant) {
		std::ostringstream ss;
		ss << v;
		code_ = ss.str();
	}

	template<typename T, size_t N, size_t M>
	operation_parameter(matrix<T, N, M> const & v) : type_(constant) {
		std::ostringstream ss;
		if (M == 1) {
			ss << "vec" << N;
		} else {
			ss << "mat" << M;
			if (N != M) ss << "x" << N;
		}
		for (size_t i = 0; i < v.size(); ++i) ss << (i ? ',' : '(') << v[i];
		ss << ')';
		code_ = ss.str();
	}

};

struct operation {
	std::string name;
	std::map<std::string, operation_parameter> parameters;
	operation(std::string const & name, std::map<std::string, operation_parameter> const & parameters = {})
		: name(name), parameters(parameters) {}
	operation(char const * name) : name(name) {}
};

class pipeline {

	static pipeline const * active_pipeline_;

public:
	static pipeline const * active_pipeline() { return active_pipeline_; }

	class context const & context;

	pipeline(class context const & context) : context(context) {}

	std::set<variable> uniform_variables;

	std::list<operation>   vertex_operations;
	std::list<operation> fragment_operations;

	std::map<variable, std::string> special_vertex_outputs = {
		{{"vec4", "gl_Position"}, ""},
		{{"float", "gl_PointSize"}, ""}
	};
	std::map<variable, std::string> special_fragment_outputs = {
		{{"vec4", "gl_FragColor"}, ""},
		{{"float", "gl_FragDepth"}, ""}
	};
	std::set<variable> fragment_outputs;

private:
	std::string vertex_shader_source_;
	std::string fragment_shader_source_;

	std::vector<variable> vertex_attributes_;

	shader_program program_;

public:
	std::string const & vertex_shader_source() const { return vertex_shader_source_; }
	std::string const & fragment_shader_source() const { return fragment_shader_source_; }

	std::vector<variable> const & vertex_attributes() const { return vertex_attributes_; }

	shader_program const & program() const { return program_; }

	void compile_source();
	void compile_program();
	void compile();

	void use() const;

};

inline std::ostream & operator << (std::ostream & out, variable const & v) {
	return out << v.type << ' ' << v.name;
}

inline std::ostream & operator << (std::ostream & out, context const & c) {
	if (c.glsl_version()) out << "#version " << c.glsl_version() << std::endl;
	return out << c.code() << std::endl;
}

}

using pipeline = shader_pipeline::pipeline;
using shader_context = shader_pipeline::context;

}
