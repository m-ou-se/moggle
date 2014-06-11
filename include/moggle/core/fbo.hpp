#pragma once

#include "gl.hpp"
#include "renderbuffer.hpp"
#include "texture.hpp"

#include <vector>

/*
 * The are two flavours of fbo's: bare_fbo and fbo.
 * The former expects the user to provide references to textures/renderbuffers
 * and does not retain those (so the user is responsible for good lifetimes).
 * The latter is able to own textures/renderbuffers, so that the user does
 * not have to care.
 *
 * Attachment can be one of: GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT, or GL_STENCIL_ATTACHMENT
 */

namespace moggle {

// A fbo wrapper which does not own attached textures/renderbuffers
class bare_fbo {

private:
	GLuint id = 0;

public:
	explicit bare_fbo(bool create_now = false) {
		if(create_now) create();
	}

	~bare_fbo() { destroy(); }

	bare_fbo(bare_fbo const &) = delete;
	bare_fbo & operator=(bare_fbo const&) = delete;

	bare_fbo(bare_fbo && r) : id(r.id) { r.id = 0; }
	bare_fbo & operator=(bare_fbo && r) { std::swap(id, r.id); return *this; }

	bool created() const { return id; }
	explicit operator bool() const { return created(); }

	void create(){ if(!id) gl::generate_framebuffers(1, &id); }
	void destroy(){ gl::delete_renderbuffers(1, &id); id = 0; }

	void bind() {
		create();
		gl::bind_framebuffer(GL_FRAMEBUFFER, id);
	}

	GLuint get_id() const { return id; }

	void attach(GLenum attachment, renderbuffer const & rb){
		bind();
		gl::framebuffer_renderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rb.get_id());
	}

	void attach(GLenum attachment, renderbuffer && rb) = delete;

	void attach(GLenum attachment, GLenum textarget, texture const & t){
		bind();
		gl::framebuffer_texture_2d(GL_FRAMEBUFFER, attachment, textarget, t.get_id(), 0);
	}

	void attach(GLenum attachment, GLenum textarget, texture && t) = delete;

	void clear(GLenum bufferbits){
		bind();
		gl::clear(bufferbits);
	}

};

// A fbo wrapper which owns attached textures/renderbuffers which are moved in
class fbo : public bare_fbo {

private:
	std::vector<renderbuffer> renderbuffers;
	std::vector<texture> textures;

public:
	using bare_fbo::bare_fbo;
	using bare_fbo::attach; // NOTE: you can still manage lifetime yourself

	void attach(GLenum attachment, renderbuffer && rb){
		renderbuffers.push_back(std::move(rb));
		attach(attachment, renderbuffers.back());
	}

	void attach(GLenum attachment, GLenum textarget, texture && t){
		textures.push_back(std::move(t));
		attach(attachment, textarget, textures.back());
	}

	// Only well defined if the fbo owns a texture
	texture & get_texture() {
		return textures.front();
	}

};

}
