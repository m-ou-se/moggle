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

#include <initializer_list>
#include <memory>
#include <vector>
#include "../core/vbo.hpp"

namespace moggle {

class generic_buffer {

protected:
	generic_vbo vbo_;

public:
	generic_buffer() {}

	generic_buffer(generic_buffer const &) {}

	generic_buffer(generic_buffer && other) : vbo_(std::move(other.vbo_)) {}

protected:
	generic_buffer & operator = (generic_buffer const &) {
		return *this;
	}

	generic_buffer & operator = (generic_buffer && other) {
		vbo_ = std::move(other.vbo_);
		return *this;
	}

public:
	void bind(GLenum b) const {
		vbo_.bind(b);
	}

	virtual void sync() = 0;

	generic_vbo const & vbo() const { return vbo_; }
	operator generic_vbo const & () const { return vbo(); }

	virtual ~generic_buffer() {}

};

template<typename T>
class buffer : public generic_buffer, public std::vector<T> {

private:
	using vbo_t = moggle::vbo<T>;

	bool dirty_ = !std::vector<T>::empty();

public:
	buffer() : std::vector<T>() {}
	explicit buffer(typename std::vector<T>::size_type count, T const & value) : std::vector<T>(count, value) {}
	explicit buffer(typename std::vector<T>::size_type count) : std::vector<T>(count) {}
	template<class I> buffer(I begin, I end) : std::vector<T>(begin, end) {}
	buffer(std::vector<T> const & other) : std::vector<T>(other) {}
	buffer(std::vector<T> && other) : std::vector<T>(std::move(other)) {}
	buffer(std::initializer_list<T> init) : std::vector<T>(init) {}

	buffer(buffer const & other) : generic_buffer(other), std::vector<T>(other) {}

	buffer & operator = (buffer const & other) {
		std::vector<T>::operator = (other);
		dirty_ = true;
	}

	buffer(buffer &&) = default;
	buffer & operator = (buffer &&) = default;

	void mark_dirty() { dirty_ = true; }

	bool is_dirty() const { return dirty_; }

	virtual void sync() override {
		if (dirty_) {
			static_cast<vbo_t &>(vbo_).data(*this);
			dirty_ = false;
		}
	}

	void sync_back(); // TODO

	vbo_t const & vbo() const { return static_cast<vbo_t const &>(generic_buffer::vbo()); }
	operator vbo_t const & () const { return vbo(); }

	std::shared_ptr<buffer> make_shared() {
		return std::make_shared<buffer>(std::move(*this));
	}
};

}
