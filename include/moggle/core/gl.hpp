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

#include <stdexcept>
#include <string>
#include <utility>

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#define glBindVertexArray glBindVertexArrayAPPLE
	#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
	#define glGenVertexArrays glGenVertexArraysAPPLE
#else
	#include <GL/glew.h>
#endif

#ifndef MOGGLE_CHECK_GL_ERRORS
#ifdef NDEBUG
#define MOGGLE_CHECK_GL_ERRORS 0
#else
#define MOGGLE_CHECK_GL_ERRORS 1
#endif
#endif

namespace moggle {

struct gl_error : std::runtime_error {
	gl_error(std::string const & function, std::string const & s)
		: std::runtime_error(function + ": " + s) {}
};

namespace gl {

	inline void init() {
		#ifdef GLEW_VERSION
		if (glewInit() != GLEW_OK) throw gl_error{"glewInit", "GLEW initialisation failed."};
		#endif
	}

	inline void throw_error(std::string const & function) {
		switch (glGetError()) {
			case GL_NO_ERROR                     : return;
			case GL_INVALID_VALUE                : throw gl_error{function, "Invalid value."};
			case GL_INVALID_ENUM                 : throw gl_error{function, "Invalid enumeration value."};
			case GL_INVALID_OPERATION            : throw gl_error{function, "Invalid operation."};
			case GL_INVALID_FRAMEBUFFER_OPERATION: throw gl_error{function, "Invalid framebuffer operation."};
			case GL_OUT_OF_MEMORY                : throw gl_error{function, "Out of memory."};
			default                              : throw gl_error{function, "Unknown error."};
		}
	}

	struct error_checker {
	#if MOGGLE_CHECK_GL_ERRORS
		char const * function;
		error_checker(char const * f) : function(f) {
			throw_error(std::string("Before ") + f);
		}
		~error_checker() {
			throw_error(function);
		}
	#else
		constexpr error_checker(char const *) {}
	#endif
	};

	#define X(name, gl) \
		template<typename... Args> \
		inline decltype(gl(std::declval<Args>()...)) \
		name(Args && ... args) { \
			error_checker c{#gl " (gl::" #name ")"}; \
			return gl(std::forward<Args>(args)...); \
		}

	X(active_texture                , glActiveTexture          )
	X(attach_shader                 , glAttachShader           )
	X(bind_attribute_location       , glBindAttribLocation     )
	X(bind_buffer                   , glBindBuffer             )
	X(bind_framebuffer              , glBindFramebuffer        )
	X(bind_renderbuffer             , glBindRenderbuffer       )
	X(bind_texture                  , glBindTexture            )
	X(bind_vertex_array             , glBindVertexArray        )
	X(blend_equation                , glBlendEquation          )
	X(blend_function                , glBlendFunc              )
	X(buffer_data                   , glBufferData             )
	X(clear                         , glClear                  )
	X(clear_color                   , glClearColor             )
	X(compile_shader                , glCompileShader          )
	X(create_program                , glCreateProgram          )
	X(create_shader                 , glCreateShader           )
	X(delete_buffers                , glDeleteBuffers          )
	X(delete_framebuffers           , glDeleteFramebuffers     )
	X(delete_program                , glDeleteProgram          )
	X(delete_renderbuffers          , glDeleteRenderbuffers    )
	X(delete_shader                 , glDeleteShader           )
	X(delete_textures               , glDeleteTextures         )
	X(delete_vertex_arrays          , glDeleteVertexArrays     )
	X(disable                       , glDisable                )
	X(draw_arrays                   , glDrawArrays             )
	X(draw_elements                 , glDrawElements           )
	X(enable                        , glEnable                 )
	X(enable_vertex_attribute_array , glEnableVertexAttribArray)
	X(framebuffer_renderbuffer      , glFramebufferRenderbuffer)
	X(framebuffer_texture_2d        , glFramebufferTexture2D   )
	X(generate_framebuffers         , glGenFramebuffers        )
	X(generate_renderbuffers        , glGenRenderbuffers       )
	X(generate_textures             , glGenTextures            )
	X(generate_vertex_arrays        , glGenVertexArrays        )
	X(generate_buffers              , glGenBuffers             )
	X(get_program_iv                , glGetProgramiv           )
	X(get_program_info_log          , glGetProgramInfoLog      )
	X(get_shader_info_log           , glGetShaderInfoLog       )
	X(get_shader_iv                 , glGetShaderiv            )
	X(get_uniform_location          , glGetUniformLocation     )
	X(link_program                  , glLinkProgram            )
	X(renderbuffer_storage          , glRenderbufferStorage    )
	X(shader_source                 , glShaderSource           )
	X(texture_image_2d              , glTexImage2D             )
	X(texture_parameter_f           , glTexParameterf          )
	X(texture_parameter_i           , glTexParameteri          )
	X(uniform_1f                    , glUniform1f              )
	X(uniform_1i                    , glUniform1i              )
	X(uniform_1ui                   , glUniform1ui             )
	X(uniform_2fv                   , glUniform2fv             )
	X(uniform_2iv                   , glUniform2iv             )
	X(uniform_2uiv                  , glUniform2uiv            )
	X(uniform_3fv                   , glUniform3fv             )
	X(uniform_3iv                   , glUniform3iv             )
	X(uniform_3uiv                  , glUniform3uiv            )
	X(uniform_4fv                   , glUniform4fv             )
	X(uniform_4iv                   , glUniform4iv             )
	X(uniform_4uiv                  , glUniform4uiv            )
	X(uniform_matrix_2fv            , glUniformMatrix2fv       )
	X(uniform_matrix_2x3fv          , glUniformMatrix2x3fv     )
	X(uniform_matrix_2x4fv          , glUniformMatrix2x4fv     )
	X(uniform_matrix_3fv            , glUniformMatrix3fv       )
	X(uniform_matrix_3x2fv          , glUniformMatrix3x2fv     )
	X(uniform_matrix_3x4fv          , glUniformMatrix3x4fv     )
	X(uniform_matrix_4fv            , glUniformMatrix4fv       )
	X(uniform_matrix_4x2fv          , glUniformMatrix4x2fv     )
	X(uniform_matrix_4x3fv          , glUniformMatrix4x3fv     )
	X(use_program                   , glUseProgram             )
	X(vertex_attribute_pointer      , glVertexAttribPointer    )
	X(viewport                      , glViewport               )

	#undef X
}
}
