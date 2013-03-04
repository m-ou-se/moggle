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

#include <set>
#include <cctype>

#include "token_iterator.hpp"

namespace {
	bool isid(char c) {
		return std::isalnum(c) || c == '_';
	}
}

namespace moggle {
namespace shader_pipeline {

std::pair<std::string, token_type> token_iterator::next() {
	static const std::set<std::string> tokens = {
		"++", "--", "<<", ">>", "<=", ">=", "==", "!=", "&&", "||", "^^",
		"+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "|=", "^=", "//", "/*"
	};
	std::string result = next_whitespace_;
	next_whitespace_.erase();
	while (input_ && *input_ && std::isspace(input_->peek())) result += input_->get();
	if (!result.empty() || !input_) return { result, token_type::whitespace };
	while (true) {
		char c = input_->peek();
		if (!*input_) break;
		if (c == '\\') {
			input_->ignore();
			if (input_->get() == '\n') next_whitespace_ += '\n';
			continue;
		}
		if (!result.empty()) {
			if (isid(result[0])) {
				if (!isid(c) && !(c == '.' && isdigit(result[0]))) break;
			} else {
				if (!tokens.count(result + c)) break;
			}
		}
		result += input_->get();
		if (result == "//") {
			std::string c;
			std::getline(*input_, c);
			next_whitespace_ = "\n";
			return { result + c, token_type::comment };
		} else if (result == "/*") {
			while (true) {
				std::string c;
				std::getline(*input_, c, '*');
				result += c + '*';
				if (input_->peek() == '/') {
					result += input_->get();
					return { result, token_type::comment };
				}
			}
		}
	}
	if (!*input_) input_ = 0;
	if (result.empty())               return { result, token_type::whitespace };
	else if (std::isdigit(result[0])) return { result, token_type::number     };
	else if (isid(result[0]))         return { result, token_type::word       };
	else                              return { result, token_type::other      };
}

}
}
