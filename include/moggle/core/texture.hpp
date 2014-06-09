#pragma once

#include <moggle/core/gl.hpp>
#include <moggle/core/gl_type_traits.hpp>

/*
 * NOTE:
 * in glTexImage2D, border MUST be 0, so this parameter is left out in this interface.
 */

namespace moggle {

class texture {

private:
	GLuint id = 0;

public:
	explicit texture(bool create_now = false) {
		if(create_now) create();
	}

	~texture() { destroy(); }

	texture(texture const &) = delete;
	texture & operator=(texture const&) = delete;

	texture(texture && t) : id(t.id) { t.id = 0; }
	texture & operator=(texture && t) { std::swap(id, t.id); return *this; }

	bool created() const { return id; }
	explicit operator bool() const { return created(); }

	void create(){ if(!id) gl::generate_textures(1, &id); }
	void destroy(){ gl::delete_textures(1, &id); id = 0; }

	void bind(GLenum target) {
		create();
		gl::bind_texture(target, id);
	}

	GLuint get_id() const { return id; }

	template <typename T>
	void image_2d(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLenum format, const T * data){
		bind(target);
		auto type = gl_type_traits<T>::gl_constant;
		gl::texture_image_2d(target, level, internalformat, width, height, 0, format, type, data);
	}

	void image_2d(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, std::nullptr_t){
		bind(target);
		gl::texture_image_2d(target, level, internalformat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	void image_2d_multisample(GLenum target, GLsizei samples , GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations){
		bind(target);
		gl::texture_image_2d_multisample(target, samples, internalformat, width, height, fixedsamplelocations);
	}

};

}
