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

#include <iostream>
#include <string>

namespace moggle {
namespace shader_pipeline {

enum class token_type {
	whitespace,
	comment,
	word,
	number,
	other
};

class token_iterator : public std::iterator<std::input_iterator_tag, std::pair<std::string, token_type>> {

private:
	std::istream * input_;

	std::string next_whitespace_;

	std::pair<std::string, token_type> current_;

	std::pair<std::string, token_type> next();

public:
	token_iterator() : input_(nullptr) {}
	explicit token_iterator(std::istream & input) : input_(&input), current_(next()) {}

	std::istream & input() const { return *input_; }

	bool operator == (token_iterator const & t) { return input_ == t.input_; }
	bool operator != (token_iterator const & t) { return input_ != t.input_; }

	explicit operator bool () { return input_; }

	token_iterator & operator ++ (   ) { current_ = next(); return *this; }
	token_iterator   operator ++ (int) { auto p = *this; current_ = next(); return p; }

	std::pair<std::string, token_type> const & operator * () { return current_; }
	std::pair<std::string, token_type> const * operator -> () { return &current_; }

	std::string skip_whitespace() {
		std::string s;
		while (current_.second == token_type::whitespace || current_.second == token_type::comment) {
			s += current_.first;
			current_ = next();
			if (!input_) break;
		}
		return s;
	}

};

}
}
