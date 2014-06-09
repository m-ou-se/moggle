#pragma once

#include <moggle/core/gl.hpp>

namespace moggle {

struct renderbuffer {

private:
	GLuint id = 0;

public:
	explicit renderbuffer(bool create_now = false) {
		if(create_now) create();
	}

	~renderbuffer() { destroy(); }

	renderbuffer(renderbuffer const &) = delete;
	renderbuffer & operator=(renderbuffer const&) = delete;

	renderbuffer(renderbuffer && r) : id(r.id) { r.id = 0; }
	renderbuffer & operator=(renderbuffer && r) { std::swap(id, r.id); return *this; }

	bool created() const { return id; }
	explicit operator bool() const { return created(); }

	void create(){ if(!id) gl::generate_renderbuffers(1, &id); }
	void destroy(){ gl::delete_renderbuffers(1, &id); id = 0; }

	void bind() {
		create();
		gl::bind_renderbuffer(GL_RENDERBUFFER, id);
	}

	void storage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height){
		bind();
		gl::renderbuffer_storage(target, internalformat, width, height);
	}

	void storage_multisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height){
		bind();
		gl::renderbuffer_storage_multisample(target, samples, internalformat, width, height);
	}

};

}
