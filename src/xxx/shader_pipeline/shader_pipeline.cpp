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

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <deque>
#include <map>

#include <moggle/xxx/shader_pipeline.hpp>
#include "token_iterator.hpp"

namespace moggle {
namespace shader_pipeline {

pipeline const * pipeline::active_pipeline_;

void context::add_file(std::string const & name) {
	std::ifstream file;
	file.exceptions(std::ifstream::badbit);
	file.open(name);
	if (!file) throw compile_error{"Unable to open '" + name + "'."};
	used_files_.insert(name);
	add_code(file, name);
}

void context::add_code(std::string const & code, std::string const & name) {
	std::istringstream ss{code};
	add_code(ss, name);
}

void context::add_code(std::istream & in, std::string const & name) {

	token_iterator i{in};

	this->code_ += "// --------------------";
	if (!name.empty()) this->code_ += "// " + name;
	this->code_ +="\n\n";

	while (i) {

		if (i->second == token_type::other && i->first == "#") {
			++i;
			i.skip_whitespace();
			if (i->second == token_type::word) {
				if (i->first == "import") {
					std::string filename;
					std::getline(in, filename);
					filename.erase(0, filename.find_first_not_of(" \t"));
					this->code_ += "//#import " + filename + "\n";
					if (!used_files_.count(filename)) add_file(filename);
					++i;
					continue;
				} else if (i->first == "version") {
					unsigned int version;
					in >> version;
					if (version > this->glsl_version_) this->glsl_version_ = version;
					++i;
					continue;
				} else {
					this->code_ += '#';
				}
			}
		}

		if (i->second == token_type::word && i->first == "operation") {
			this->code_ += "/*operation*/ void";
			++i;
			this->code_ += i.skip_whitespace();
			if (i->second != token_type::word) throw parse_error{"Expected name after 'operation'."};
			this->code_ += i->first;
			std::string operation_name = i->first;
			operation_details operation;
			++i;
			this->code_ += i.skip_whitespace();
			if (i->first != "(") throw parse_error{"Expected '(' after 'operation " + operation_name + "'."};
			this->code_ += i->first;
			while (true) {
				++i;
				parameter p;
				this->code_ += i.skip_whitespace();
				this->code_ += i->first;
				p.in = i->first == "in" || i->first == "inout";
				p.out = i->first == "out" || i->first == "inout";
				if (!p.in && !p.out) throw parse_error{"Parameters (to 'operation " + operation_name + "') need to be 'in', 'out' or 'inout' (not '" + i->first + "')."};
				++i;
				this->code_ += i.skip_whitespace();
				std::string whitespace, whitespace2;
				while (i && i->first != "," && i->first != ")") {
					this->code_ += i->first;
					if (i->second == token_type::whitespace) {
						whitespace += i->first;
					} else {
						if (!p.type.empty()) p.type += whitespace2;
						whitespace2 = whitespace;
						whitespace.erase();
						p.type += p.name;
						p.name = i->first;
					}
					++i;
				}
				operation.parameters.push_back(std::move(p));
				if (!i) throw parse_error{"Expected ')' to end parameter list (of 'operation " + operation_name + "'."};
				this->code_ += i->first;
				if (i->first == ")") break;
			}
			++i;
			if (!operations_.insert({operation_name, operation}).second) throw parse_error{"Multiple definitions of 'operation " + operation_name + "'."};
			continue;
		}

		this->code_ += i->first;
		++i;
	}

	this->code_ += "\n";

}

namespace {
	struct stream_splitter {
		std::ostream & a;
		std::ostream & b;
		template<typename T>
		stream_splitter & operator << (T && v) {
			a << v;
			b << std::forward<T>(v);
			return *this;
		}
	};
}

void pipeline::compile_source() {
	std::ostringstream vertex_shader;
	std::ostringstream fragment_shader;

	stream_splitter both_shaders { vertex_shader, fragment_shader };

	both_shaders << context << '\n';
	both_shaders << "// --------------------" << '\n';
	both_shaders << "// generated shader" << '\n' << '\n';

	for (auto const & v : uniform_variables) {
		both_shaders << "uniform " << v << ";" << '\n';
	}

	std::set<variable> required_variables;
	std::set<variable> local_variables;
	std::set<variable> varying_variables;

	for (auto const & v : fragment_outputs) {
		required_variables.insert(v);
	}
	for (auto const & v : special_fragment_outputs) {
		if (v.second.empty()) continue;
		required_variables.insert(variable(v.first.type, v.second));
	}

	auto process_operations = [&](std::list<operation> const & operations) {
		for (auto i = operations.rbegin(); i != operations.rend(); ++i) {
			if (i->name == "=") {
				for (auto const & p : i->parameters) {
					auto v = required_variables.find(p.first);
					if (v == required_variables.end()) throw compile_error{"Unused variable '" + p.first + "'."};
					local_variables.insert(*v);
					std::string type = v->type;
					required_variables.erase(v);
					if (p.second.type() == operation_parameter::variable) {
						required_variables.insert(variable{type, p.second.code()});
					}
				}
			} else {
				auto op = context.operations().find(i->name);
				if (op == context.operations().end()) throw compile_error{"Undefined reference to 'operation " + i->name + "'."};
				for (auto p : op->second.parameters) {
					auto s = i->parameters.find(p.name);
					if (s != i->parameters.end()) {
						if (s->second.type() == operation_parameter::variable) {
							p.name = s->second.code();
						} else {
							if (p.out) throw compile_error{"Can't use a constant (" + s->second.code() + ") for 'out " + p.type + " " + p.name + "'."};
							continue;
						}
					}
					if (p.out) {
						local_variables.insert(p);
						required_variables.erase(p);
					}
					if (p.in) required_variables.insert(p);
				}
			}
		}
	};

	auto var_name = [&](std::string const & v) -> std::string {
		if (local_variables.count(v)) return "_l_" + v;
		if (varying_variables.count(v)) return "_v_" + v;
		return v;
	};

	auto generate_statements = [&](std::list<operation> const & operations) -> std::deque<std::string> {
		std::deque<std::string> statements;
		for (auto i = operations.rbegin(); i != operations.rend(); ++i) {
			std::ostringstream statement;
			if (i->name == "=") {
				for (auto const & p : i->parameters) {
					statement << "_l_" << p.first << " = ";
					if (p.second.type() == operation_parameter::variable) {
						statement << var_name(p.second.code());
					} else {
						statement << p.second.code();
					}
					statement << ';';
				}
			} else {
				auto op = context.operations().find(i->name);
				statement << i->name;
				bool first = true;
				for (auto p : op->second.parameters) {
					statement << (first ? "(" : ", ");
					first = false;
					auto s = i->parameters.find(p.name);
					if (s != i->parameters.end()) {
						if (s->second.type() == operation_parameter::variable) {
							p.name = s->second.code();
						} else {
							statement << s->second.code();
							continue;
						}
					}
					statement << var_name(p.name);
				}
				statement << ");";
			}
			if (!statement.str().empty()) statements.push_front(statement.str());
		}
		return statements;
	};

	process_operations(fragment_operations);

	varying_variables = required_variables;

	for (auto const & v : uniform_variables) {
		varying_variables.erase(v);
	}

	auto fragment_operations_statements = generate_statements(fragment_operations);

	for (auto const & v : varying_variables) {
		both_shaders << "varying " << v.type << " _v_" << v.name << ";" << '\n';
	}

	for (auto const & v : fragment_outputs) {
		fragment_shader << "out " << v << '\n';
	}

	fragment_shader << "void main() {" << '\n';

	for (auto const & v : local_variables) {
		fragment_shader << '\t' << v.type << " _l_" << v.name;
		if (required_variables.count(v)) {
			fragment_shader << " = " << (uniform_variables.count(v) ? "" : "_v_") << v.name;
		}
		fragment_shader << ";" << '\n';
	}
	for (auto const & s : fragment_operations_statements) {
		fragment_shader << '\t' << s << '\n';
	}
	for (auto const & v : fragment_outputs) {
		fragment_shader << '\t' << v.name << " = " << var_name(v.name) << ";" << '\n';
	}
	for (auto const & v : special_fragment_outputs) {
		if (v.second.empty()) continue;
		fragment_shader << '\t' << v.first.name << " = " << var_name(v.second) << ";" << '\n';
	}

	fragment_shader << '}' << '\n';

	auto varying_outputs = std::move(varying_variables);
	varying_variables.clear();

	for (auto const & v : special_vertex_outputs) {
		if (v.second.empty()) continue;
		required_variables.insert(variable(v.first.type, v.second));
	}

	local_variables.clear();

	process_operations(vertex_operations);
	auto vertex_operations_statements = generate_statements(vertex_operations);

	vertex_attributes_.clear();
	for (auto const & v : required_variables) {
		if (uniform_variables.count(v)) continue;
		vertex_attributes_.push_back(v);
		vertex_shader << "attribute " << v << ";" << '\n';
	}

	vertex_shader << "void main() {" << '\n';

	for (auto const & v : local_variables) {
		vertex_shader << '\t' << v.type << " _l_" << v.name;
		if (required_variables.count(v)) {
			vertex_shader << " = " << v.name;
		}
		vertex_shader << ";" << '\n';
	}
	for (auto const & s : vertex_operations_statements) {
		vertex_shader << '\t' << s << '\n';
	}
	for (auto const & v : special_vertex_outputs) {
		if (v.second.empty()) continue;
		vertex_shader << '\t' << v.first.name << " = " << var_name(v.second) << ";" << '\n';
	}
	for (auto const & v : varying_outputs) {
		vertex_shader << '\t' << "_v_" << v.name << " = " << var_name(v.name) << ";" << '\n';
	}

	vertex_shader << "}" << '\n';

	vertex_shader_source_ = vertex_shader.str();
	fragment_shader_source_ = fragment_shader.str();
}

void pipeline::compile_program() {
	program_.clear();
	program_.attach(moggle::shader::from_source(shader_type::  vertex,   vertex_shader_source_));
	program_.attach(moggle::shader::from_source(shader_type::fragment, fragment_shader_source_));
	for (size_t i = 0; i < vertex_attributes_.size(); ++i) {
		program_.bind_attribute(i, vertex_attributes_[i].name);
	}
	program_.link();
}

void pipeline::compile() {
	compile_source();
	compile_program();
}

void pipeline::use() const {
	program_.use();
	active_pipeline_ = this;
}

}
}
