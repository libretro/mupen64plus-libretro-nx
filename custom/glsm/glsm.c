/* Copyright (C) 2010-2016 The RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this libretro SDK code part (glsm).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glsym/glsym.h>
#include <glsm/glsm.h>
#include "glsl_optimizer.h"

#define MAX_UNIFORMS 500
#define MAX_TEXTURES 128000
#ifndef GL_DRAW_INDIRECT_BUFFER
#define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#endif

struct gl_cached_state
{
   struct
   {
      GLuint ids[32];
   } bind_textures;

   struct
   {
      bool used[MAX_ATTRIB];
      GLint size[MAX_ATTRIB];
      GLenum type[MAX_ATTRIB];
      GLboolean normalized[MAX_ATTRIB];
      GLsizei stride[MAX_ATTRIB];
      const GLvoid *pointer[MAX_ATTRIB];
      GLuint buffer[MAX_ATTRIB];
   } attrib_pointer;

#ifndef HAVE_OPENGLES
   GLenum colorlogicop;
#endif

   struct
   {
      bool enabled[MAX_ATTRIB];
   } vertex_attrib_pointer;

   struct
   {
      GLfloat v0[MAX_ATTRIB];
      GLfloat v1[MAX_ATTRIB];
      GLfloat v2[MAX_ATTRIB];
      GLfloat v3[MAX_ATTRIB];
   } vertex_attribs;

   struct
   {
      GLenum pname;
      GLint param;
   } pixelstore_i;

   struct
   {
      GLuint array;
   } bindvertex;

   struct
   {
      GLuint r;
      GLuint g;
      GLuint b;
      GLuint a;
   } clear_color;

   struct
   {
      bool used;
      GLint x;
      GLint y;
      GLsizei w;
      GLsizei h;
   } scissor;

   struct
   {
      GLint x;
      GLint y;
      GLsizei w;
      GLsizei h;
   } viewport;

   struct
   {
      bool used;
      GLenum sfactor;
      GLenum dfactor;
   } blendfunc;

   struct
   {
      bool used;
      GLenum srcRGB;
      GLenum dstRGB;
      GLenum srcAlpha;
      GLenum dstAlpha;
   } blendfunc_separate;

   struct
   {
      bool used;
      GLboolean red;
      GLboolean green;
      GLboolean blue;
      GLboolean alpha;
   } colormask;

   struct
   {
      bool used;
      GLenum func;
   } depthfunc;

   struct
   {
      bool used;
      GLfloat factor;
      GLfloat units;
   } polygonoffset;

   struct
   {
      bool used;
      GLenum func;
      GLint ref;
      GLuint mask;
   } stencilfunc;

   struct
   {
      bool used;
      GLenum sfail;
      GLenum dpfail;
      GLenum dppass;
   } stencilop;

   struct
   {
      bool used;
      GLenum mode;
   } frontface;

   struct
   {
      bool used;
      GLenum mode;
   } cullface;

   struct
   {
      bool used;
      GLuint mask;
   } stencilmask;

   struct
   {
      bool used;
      GLboolean mask;
   } depthmask;

   struct
   {
      GLenum mode;
   } readbuffer;

   struct
   {
      GLuint location;
      int has_depth;
   } framebuf[2];

   GLuint array_buffer;
   GLuint index_buffer;
   GLuint indirect_buffer;
   GLuint vao;
   GLuint program;
   int cap_state[SGL_CAP_MAX];
   int cap_translate[SGL_CAP_MAX];
};

struct gl_program_uniforms
{
   GLfloat uniform1f;
   GLfloat uniform2f[2];
   GLfloat uniform3f[3];
   GLfloat uniform4f[4];
   GLint uniform1i;
   GLint uniform2i[2];
   GLint uniform3i[3];
   GLint uniform4i[4];
};

struct gl_texture_params
{
   GLint min_filter;
   GLint mag_filter;
   GLint wrap_s;
   GLint wrap_t;
   GLint max_level;
};

static struct gl_texture_params* texture_params[MAX_TEXTURES];
static struct gl_program_uniforms* program_uniforms[MAX_UNIFORMS][MAX_UNIFORMS];
static GLenum active_texture;
static GLuint default_framebuffer;
static GLint glsm_max_textures;
static struct retro_hw_render_callback hw_render;
static struct gl_cached_state gl_state;
glslopt_ctx* ctx;
static int window_first = 0;
static int resetting_context = 0;
static const GLenum discards[]  = {GL_DEPTH_ATTACHMENT};

static void on_gl_error(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, void *userParam)
{
   printf("%s\n", message);
}

/* GL wrapper-side */

/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
GLenum rglGetError(void)
{
   return glGetError();
}

/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
void rglClear(GLbitfield mask)
{
   glClear(mask);
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglValidateProgram(GLuint program)
{
   glValidateProgram(program);
}

/*
 *
 * Core in:
 * OpenGL    : 1.0
 * OpenGLES  : N/A
 */
void rglPolygonMode(GLenum face, GLenum mode)
{
#ifndef HAVE_OPENGLES
   glPolygonMode(face, mode);
#endif
}

void rglTexImage2D(
	GLenum target,
 	GLint level,
 	GLint internalformat,
 	GLsizei width,
 	GLsizei height,
 	GLint border,
 	GLenum format,
 	GLenum type,
 	const GLvoid * data)
{
   //This is a fix for https://github.com/loganmc10/GLupeN64/issues/13
   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
}

void rglTexSubImage2D(
      GLenum target,
  	GLint level,
  	GLint xoffset,
  	GLint yoffset,
  	GLsizei width,
  	GLsizei height,
  	GLenum format,
  	GLenum type,
  	const GLvoid * pixels)
{
   //This is a fix for https://github.com/loganmc10/GLupeN64/issues/13
   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexSubImage2D(target, level, xoffset, yoffset,
         width, height, format, type, pixels);
}

/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
void rglLineWidth(GLfloat width)
{
   glLineWidth(width);
}

/*
 * Category: FBO
 *
 * Core in:
 * OpenGL    : 3.0
 * OpenGLES  : 3.0
 */
void rglBlitFramebuffer(
      GLint srcX0, GLint srcY0,
      GLint srcX1, GLint srcY1,
      GLint dstX0, GLint dstY0,
      GLint dstX1, GLint dstY1,
      GLbitfield mask, GLenum filter)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES3)
   glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1,
         dstX0, dstY0, dstX1, dstY1,
         mask, filter);
#endif
}

/*
 *
 * Core in:
 * OpenGLES  : 3.0
 */
void rglReadBuffer(GLenum mode)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES3)
   glReadBuffer(mode);
   gl_state.readbuffer.mode = mode;
#endif
}

void rglReadPixels(	GLint x,
 	GLint y,
 	GLsizei width,
 	GLsizei height,
 	GLenum format,
 	GLenum type,
 	GLvoid * data)
{
   glReadPixels(x, y, width, height, format, type, data);
}

/*
 *
 * Core in:
 * OpenGLES  : 2.0
 */

/*
 *
 * Core in:
 * OpenGLES  : 2.0
 */
void rglPixelStorei(GLenum pname, GLint param)
{
   if (gl_state.pixelstore_i.pname != pname || gl_state.pixelstore_i.param != param) {
      glPixelStorei(pname, param);
      gl_state.pixelstore_i.pname = pname;
      gl_state.pixelstore_i.param = param;
   }
}

/*
 *
 * Core in:
 * OpenGLES  : 2.0
 */
GLboolean rglUnmapBuffer(GLenum target)
{
#ifndef HAVE_OPENGLES2
   return glUnmapBuffer(target);
#endif
}
/*
 *
 * Core in:
 * OpenGLES  : 2.0
 */
void rglFrontFace(GLenum mode)
{
   gl_state.frontface.used = true;
   if (gl_state.frontface.mode != mode) {
      glFrontFace(mode);
      gl_state.frontface.mode = mode;
   }
}

/*
 *
 * Core in:
 * OpenGLES  : 2.0
 */
void rglDepthFunc(GLenum func)
{
   gl_state.depthfunc.used = true;
   if (gl_state.depthfunc.func != func) {
      glDepthFunc(func);
      gl_state.depthfunc.func = func;
   }
}

/*
 *
 * Core in:
 * OpenGLES  : 2.0
 */
void rglColorMask(GLboolean red, GLboolean green,
      GLboolean blue, GLboolean alpha)
{
   gl_state.colormask.used  = true;
   if (gl_state.colormask.red != red || gl_state.colormask.green != green || gl_state.colormask.blue != blue || gl_state.colormask.alpha != alpha) {
       glColorMask(red, green, blue, alpha);
       gl_state.colormask.red   = red;
       gl_state.colormask.green = green;
       gl_state.colormask.blue  = blue;
       gl_state.colormask.alpha = alpha;
   }
}

/*
 *
 * Core in:
 * OpenGLES  : 2.0
 */
void rglCullFace(GLenum mode)
{
   gl_state.cullface.used = true;
   if (gl_state.cullface.mode != mode) {
      glCullFace(mode);
      gl_state.cullface.mode = mode;
   }
}

/*
 *
 * Core in:
 * OpenGLES  : 2.0
 */
void rglStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
{
   gl_state.stencilop.used   = true;
   if (gl_state.stencilop.sfail != sfail || gl_state.stencilop.dpfail != dpfail || gl_state.stencilop.dppass != dppass) {
      glStencilOp(sfail, dpfail, dppass);
      gl_state.stencilop.sfail  = sfail;
      gl_state.stencilop.dpfail = dpfail;
      gl_state.stencilop.dppass = dppass;
   }
}

/*
 *
 * Core in:
 * OpenGLES  : 2.0
 */
void rglStencilFunc(GLenum func, GLint ref, GLuint mask)
{
   gl_state.stencilfunc.used = true;
   if (gl_state.stencilfunc.func != func || gl_state.stencilfunc.ref != ref || gl_state.stencilfunc.mask != mask) {
      glStencilFunc(func, ref, mask);
      gl_state.stencilfunc.func = func;
      gl_state.stencilfunc.ref  = ref;
      gl_state.stencilfunc.mask = mask;
   }
}

/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
GLboolean rglIsEnabled(GLenum cap)
{
   return gl_state.cap_state[cap] ? GL_TRUE : GL_FALSE;
}

/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
void rglClearColor(GLclampf red, GLclampf green,
      GLclampf blue, GLclampf alpha)
{
   if (gl_state.clear_color.r != red || gl_state.clear_color.g != green || gl_state.clear_color.b != blue || gl_state.clear_color.a != alpha) {
      glClearColor(red, green, blue, alpha);
      gl_state.clear_color.r = red;
      gl_state.clear_color.g = green;
      gl_state.clear_color.b = blue;
      gl_state.clear_color.a = alpha;
   }
}

/*
 *
 * Core in:
 * OpenGLES    : 2.0 (maybe earlier?)
 */
void rglScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
   gl_state.scissor.used = true;
   if (gl_state.scissor.x != x || gl_state.scissor.y != y || gl_state.scissor.w != width || gl_state.scissor.h != height) {
      glScissor(x, y, width, height);
      gl_state.scissor.x = x;
      gl_state.scissor.y = y;
      gl_state.scissor.w = width;
      gl_state.scissor.h = height;
   }
}

/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
void rglViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   if (gl_state.viewport.x != x || gl_state.viewport.y != y || gl_state.viewport.w != width || gl_state.viewport.h != height) {
      glViewport(x, y, width, height);
      gl_state.viewport.x = x;
      gl_state.viewport.y = y;
      gl_state.viewport.w = width;
      gl_state.viewport.h = height;
   }
}

void rglBlendFunc(GLenum sfactor, GLenum dfactor)
{
   gl_state.blendfunc.used = true;
   if (gl_state.blendfunc.sfactor != sfactor || gl_state.blendfunc.dfactor != dfactor) {
      glBlendFunc(sfactor, dfactor);
      gl_state.blendfunc.sfactor = sfactor;
      gl_state.blendfunc.dfactor = dfactor;
   }
}

/*
 * Category: Blending
 *
 * Core in:
 * OpenGL    : 1.4
 */
void rglBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
   gl_state.blendfunc_separate.used     = true;
   if (gl_state.blendfunc_separate.srcRGB != srcRGB || gl_state.blendfunc_separate.dstRGB != dstRGB || gl_state.blendfunc_separate.srcAlpha != srcAlpha || gl_state.blendfunc_separate.dstAlpha != dstAlpha) {
      glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
      gl_state.blendfunc_separate.srcRGB   = srcRGB;
      gl_state.blendfunc_separate.dstRGB   = dstRGB;
      gl_state.blendfunc_separate.srcAlpha = srcAlpha;
      gl_state.blendfunc_separate.dstAlpha = dstAlpha;
   }
}

/*
 * Category: Textures
 *
 * Core in:
 * OpenGL    : 1.3
 */
void rglActiveTexture(GLenum texture)
{
   if (active_texture != texture - GL_TEXTURE0) {
      glActiveTexture(texture);
      active_texture = texture - GL_TEXTURE0;
   }
}

/*
 *
 * Core in:
 * OpenGL    : 1.1
 */
void rglBindTexture(GLenum target, GLuint texture)
{
   if (gl_state.bind_textures.ids[active_texture] != texture) {
      glBindTexture(target, texture);
      gl_state.bind_textures.ids[active_texture] = texture;
   }
}

/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
void rglDisable(GLenum cap)
{
   if (gl_state.cap_state[cap] != 0) {
      glDisable(gl_state.cap_translate[cap]);
      gl_state.cap_state[cap] = 0;
   }
}

/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
void rglEnable(GLenum cap)
{
   if (gl_state.cap_state[cap] != 1) {
      glEnable(gl_state.cap_translate[cap]);
      gl_state.cap_state[cap] = 1;
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUseProgram(GLuint program)
{
   if (gl_state.program != program) {
      glUseProgram(program);
      gl_state.program = program;
   }
}

/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
void rglDepthMask(GLboolean flag)
{
   gl_state.depthmask.used = true;
   if (gl_state.depthmask.mask != flag) {
      glDepthMask(flag);
      gl_state.depthmask.mask = flag;
   }
}

/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
void rglStencilMask(GLenum mask)
{
   gl_state.stencilmask.used = true;
   if (gl_state.stencilmask.mask != mask) {
      glStencilMask(mask);
      gl_state.stencilmask.mask = mask;
   }
}

/*
 *
 * Core in:
 * OpenGL    : 1.5
 */
void rglBufferData(GLenum target, GLsizeiptr size,
      const GLvoid *data, GLenum usage)
{
   glBufferData(target, size, data, usage);
}

/*
 *
 * Core in:
 * OpenGL    : 1.5
 */
void rglBufferSubData(GLenum target, GLintptr offset,
      GLsizeiptr size, const GLvoid *data)
{
   glBufferSubData(target, offset, size, data);
}

/*
 *
 * Core in:
 * OpenGL    : 1.5
 */
void rglBindBuffer(GLenum target, GLuint buffer)
{
   if (target == GL_ARRAY_BUFFER) {
      if (gl_state.array_buffer != buffer) {
         gl_state.array_buffer = buffer;
         glBindBuffer(target, buffer);
      }
   }
   else if (target == GL_ELEMENT_ARRAY_BUFFER) {
      if (gl_state.index_buffer != buffer) {
         gl_state.index_buffer = buffer;
         glBindBuffer(target, buffer);
      }
   }
#ifndef HAVE_OPENGLES2
   else if (target == GL_DRAW_INDIRECT_BUFFER) {
      if (gl_state.indirect_buffer != buffer) {
         gl_state.indirect_buffer = buffer;
         glBindBuffer(target, buffer);
      }
   }
#endif
   else
      glBindBuffer(target, buffer);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglLinkProgram(GLuint program)
{
   glLinkProgram(program);
}

/*
 * Category: FBO
 *
 * Core in:
 * OpenGL    : 3.0
 * OpenGLES  : 2.0
 */
void rglFramebufferTexture2D(GLenum target, GLenum attachment,
      GLenum textarget, GLuint texture, GLint level)
{
   if (attachment == GL_DEPTH_ATTACHMENT) {
      if (target == GL_FRAMEBUFFER)
         gl_state.framebuf[0].has_depth = 1;
#ifndef HAVE_OPENGLES2
      else if (target == GL_DRAW_FRAMEBUFFER)
         gl_state.framebuf[0].has_depth = 1;
      else if (target == GL_READ_FRAMEBUFFER)
         gl_state.framebuf[1].has_depth = 1;
#endif
   }
   glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

/*
 * Category: FBO
 *
 * Core in:
 * OpenGL    : 3.0
 * OpenGLES  : 3.2
 */
void rglFramebufferTexture(GLenum target, GLenum attachment,
  	GLuint texture, GLint level)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES_3_2)
   glFramebufferTexture(target, attachment, texture, level);
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 1.1
 */
void rglDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   glDrawArrays(mode, first, count);
}

void rglDrawArraysIndirect(GLenum mode, const void *indirect)
{
#ifndef HAVE_OPENGLES2
   glDrawArraysIndirect(mode, indirect);
#endif
}
/*
 *
 * Core in:
 * OpenGL    : 1.1
 */
void rglDrawElements(GLenum mode, GLsizei count, GLenum type,
                           const GLvoid * indices)
{
   glDrawElements(mode, count, type, indices);
}

void rglDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect)
{
#ifndef HAVE_OPENGLES2
   glDrawElementsIndirect(mode, type, indirect);
#endif
}

void rglDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid *indices, GLint basevertex)
{
   #ifndef HAVE_OPENGLES
   glDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
   #endif
}

void rglDrawRangeElementsBaseVertexOES(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid *indices, GLint basevertex)
{
   #ifdef HAVE_OPENGLES
   glDrawRangeElementsBaseVertexOES(mode, start, end, count, type, indices, basevertex);
   #endif
}

void rglDrawRangeElementsBaseVertexEXT(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid *indices, GLint basevertex)
{
   #ifdef HAVE_OPENGLES
   glDrawRangeElementsBaseVertexEXT(mode, start, end, count, type, indices, basevertex);
   #endif
}

void rglCompressedTexImage2D(GLenum target, GLint level,
      GLenum internalformat, GLsizei width, GLsizei height,
      GLint border, GLsizei imageSize, const GLvoid *data)
{
   glCompressedTexImage2D(target, level, internalformat,
         width, height, border, imageSize, data);
}


void rglDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
   glDeleteFramebuffers(n, framebuffers);
   int i, p;
   for (i = 0; i < n; ++i) {
      for (p = 0; p < 2; ++p) {
         if (framebuffers[i] == gl_state.framebuf[p].location)
            gl_state.framebuf[p].location = 0;
      }
   }
}

void rglDeleteTextures(GLsizei n, const GLuint *textures)
{
   int i;
   for (i = 0; i < n; ++i) {
      free(texture_params[textures[i]]);
      if (textures[i] == gl_state.bind_textures.ids[active_texture])
         gl_state.bind_textures.ids[active_texture] = 0;
   }
   glDeleteTextures(n, textures);
}

/*
 *
 * Core in:
 * OpenGLES    : 2.0
 */
void rglRenderbufferStorage(GLenum target, GLenum internalFormat,
      GLsizei width, GLsizei height)
{
   glRenderbufferStorage(target, internalFormat, width, height);
}

void rglBufferStorage(GLenum target,
                       GLsizeiptr size,
                       const GLvoid * data,
                       GLbitfield flags)
{
#ifndef HAVE_OPENGLES
   glBufferStorage(target, size, data, flags);
#endif
}

void rglBufferStorageEXT(GLenum target,
                       GLsizeiptr size,
                       const GLvoid * data,
                       GLbitfield flags)
{
#ifdef HAVE_OPENGLES
   glBufferStorageEXT(target, size, data, flags);
#endif
}

/*
 *
 * Core in:
 *
 * OpenGL      : 3.0
 * OpenGLES    : 2.0
 */
void rglBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
   glBindRenderbuffer(target, renderbuffer);
}

/*
 *
 * Core in:
 *
 * OpenGLES    : 2.0
 */
void rglDeleteRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
   glDeleteRenderbuffers(n, renderbuffers);
}

/*
 *
 * Core in:
 *
 * OpenGL      : 3.0
 * OpenGLES    : 2.0
 */
void rglGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
   glGenRenderbuffers(n, renderbuffers);
}

/*
 *
 * Core in:
 *
 * OpenGL      : 3.0
 * OpenGLES    : 2.0
 */
void rglGenerateMipmap(GLenum target)
{
   glGenerateMipmap(target);
}

/*
 * Category: FBO
 *
 * Core in:
 * OpenGL    : 3.0
 */
GLenum rglCheckFramebufferStatus(GLenum target)
{
   return glCheckFramebufferStatus(target);
}

/*
 * Category: FBO
 *
 * Core in:
 * OpenGL    : 3.0
 * OpenGLES  : 2.0
 */
void rglFramebufferRenderbuffer(GLenum target, GLenum attachment,
      GLenum renderbuffertarget, GLuint renderbuffer)
{
   if (attachment == GL_DEPTH_ATTACHMENT) {
      if (target == GL_FRAMEBUFFER)
         gl_state.framebuf[0].has_depth = 1;
#ifndef HAVE_OPENGLES2
      else if (target == GL_DRAW_FRAMEBUFFER)
         gl_state.framebuf[0].has_depth = 1;
      else if (target == GL_READ_FRAMEBUFFER)
         gl_state.framebuf[1].has_depth = 1;
#endif
   }
   glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 3.0
 */


/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglGetProgramiv(GLuint shader, GLenum pname, GLint *params)
{
   glGetProgramiv(shader, pname, params);
}

void rglGetIntegerv(GLenum pname, GLint * data)
{
   glGetIntegerv(pname, data);
}

void rglGetFloatv(GLenum pname, GLfloat * params)
{
   glGetFloatv(pname, params);
}

const GLubyte* rglGetString(GLenum name)
{
   return glGetString(name);
}
/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 4.1
 * OpenGLES  : 3.0
 */
void rglProgramParameteri( 	GLuint program,
  	GLenum pname,
  	GLint value)
{
#ifndef HAVE_OPENGLES2
   glProgramParameteri(program, pname, value);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize,
      GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
   glGetActiveUniform(program, index, bufsize, length, size, type, name);
}

/*
 * Category: UBO
 *
 * Core in:
 *
 * OpenGL    : 2.0
 * OpenGLES  : 3.0
 */
void rglGetActiveUniformBlockiv(GLuint program,
  	GLuint uniformBlockIndex,
  	GLenum pname,
  	GLint *params)
{
#ifndef HAVE_OPENGLES2
   glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 *
 * Core in:
 *
 * OpenGLES  : 3.0
 */
void rglGetActiveUniformsiv( 	GLuint program,
  	GLsizei uniformCount,
  	const GLuint *uniformIndices,
  	GLenum pname,
  	GLint *params)
{
#ifndef HAVE_OPENGLES2
   glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 *
 * Core in:
 *
 * OpenGLES  : 3.0
 */
void rglGetUniformIndices(GLuint program,
  	GLsizei uniformCount,
  	const GLchar **uniformNames,
  	GLuint *uniformIndices)
{
#ifndef HAVE_OPENGLES2
   glGetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 * Category: UBO
 *
 * Core in:
 *
 * OpenGLES  : 3.0
 */
void rglBindBufferBase( 	GLenum target,
  	GLuint index,
  	GLuint buffer)
{
#ifndef HAVE_OPENGLES2
   glBindBufferBase(target, index, buffer);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 *
 * Category: UBO
 *
 * Core in:
 *
 * OpenGLES  : 3.0
 */
GLuint rglGetUniformBlockIndex( 	GLuint program,
  	const GLchar *uniformBlockName)
{
#ifndef HAVE_OPENGLES2
   return glGetUniformBlockIndex(program, uniformBlockName);
#else
   printf("WARNING! Not implemented.\n");
   return 0;
#endif
}

/*
 * Category: UBO
 *
 * Core in:
 *
 * OpenGLES  : 3.0
 */
void rglUniformBlockBinding( 	GLuint program,
  	GLuint uniformBlockIndex,
  	GLuint uniformBlockBinding)
{
#ifndef HAVE_OPENGLES2
   glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 * OpenGLES  : 3.0
 */
void rglUniform1ui(GLint location, GLuint v)
{
#ifndef HAVE_OPENGLES2
   glUniform1ui(location ,v);
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 * OpenGLES  : 3.0
 */
void rglUniform2ui(GLint location, GLuint v0, GLuint v1)
{
#ifndef HAVE_OPENGLES2
   glUniform2ui(location, v0, v1);
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 * OpenGLES  : 3.0
 */
void rglUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
#ifndef HAVE_OPENGLES2
   glUniform3ui(location, v0, v1, v2);
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 * OpenGLES  : 3.0
 */
void rglUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
#ifndef HAVE_OPENGLES2
   glUniform4ui(location, v0, v1, v2, v3);
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose,
      const GLfloat *value)
{
   glUniformMatrix4fv(location, count, transpose, value);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglDetachShader(GLuint program, GLuint shader)
{
   glDetachShader(program, shader);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
   glGetShaderiv(shader, pname, params);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglAttachShader(GLuint program, GLuint shader)
{
   glAttachShader(program, shader);
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 */
GLint rglGetAttribLocation(GLuint program, const GLchar *name)
{
   return glGetAttribLocation(program, name);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */

void rglShaderSource(GLuint shader, GLsizei count,
      const GLchar **string, const GLint *length)
{
#ifdef HAVE_OPENGLES
   glslopt_shader_type type;
   GLint _type;
   glGetShaderiv(shader, GL_SHADER_TYPE, &_type);
   if (_type == GL_VERTEX_SHADER)
      type = kGlslOptShaderVertex;
   else if (_type == GL_FRAGMENT_SHADER)
      type = kGlslOptShaderFragment;
   glslopt_shader* new_shader = glslopt_optimize (ctx, type, *string, 0);
   if (glslopt_get_status (new_shader)) {
      const char* newSource = glslopt_get_output (new_shader);
      glShaderSource(shader, count, &newSource, length);
   } else
      printf("%s\n",glslopt_get_log (new_shader));
   glslopt_shader_delete (new_shader);
#else
   glShaderSource(shader, count, string, length);
#endif
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglCompileShader(GLuint shader)
{
   glCompileShader(shader);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
GLuint rglCreateProgram(void)
{
   GLuint temp = glCreateProgram();
   int i;
   for (i = 0; i < MAX_UNIFORMS; ++i)
      program_uniforms[temp][i] = calloc(1, sizeof(struct gl_program_uniforms));
   return temp;
}

/*
 *
 * Core in:
 * OpenGL    : 1.1
 */
void rglGenTextures(GLsizei n, GLuint *textures)
{
   glGenTextures(n, textures);
   int i;
   for (i = 0; i < n; ++i) {
      texture_params[textures[i]] = calloc(1, sizeof(struct gl_texture_params));
      texture_params[textures[i]]->max_level = 1000;
   }
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglGetShaderInfoLog(GLuint shader, GLsizei maxLength,
      GLsizei *length, GLchar *infoLog)
{
   glGetShaderInfoLog(shader, maxLength, length, infoLog);
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglGetProgramInfoLog(GLuint shader, GLsizei maxLength,
      GLsizei *length, GLchar *infoLog)
{
   glGetProgramInfoLog(shader, maxLength, length, infoLog);
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 */
GLboolean rglIsProgram(GLuint program)
{
   return glIsProgram(program);
}


void rglTexCoord2f(GLfloat s, GLfloat t)
{
#ifdef HAVE_LEGACY_GL
   glTexCoord2f(s, t);
#endif
}

void rglTexParameteri(GLenum target, GLenum pname, GLint param)
{
   if (pname == GL_TEXTURE_MIN_FILTER && texture_params[gl_state.bind_textures.ids[active_texture]] != NULL) {
      if (texture_params[gl_state.bind_textures.ids[active_texture]]->min_filter != param) {
         texture_params[gl_state.bind_textures.ids[active_texture]]->min_filter = param;
         glTexParameteri(target, pname, param);
      }
   }
   else if (pname == GL_TEXTURE_MAG_FILTER && texture_params[gl_state.bind_textures.ids[active_texture]] != NULL) {
      if (texture_params[gl_state.bind_textures.ids[active_texture]]->mag_filter != param) {
         texture_params[gl_state.bind_textures.ids[active_texture]]->mag_filter = param;
         glTexParameteri(target, pname, param);
      }
   }
   else if (pname == GL_TEXTURE_WRAP_S && texture_params[gl_state.bind_textures.ids[active_texture]] != NULL) {
      if (texture_params[gl_state.bind_textures.ids[active_texture]]->wrap_s != param) {
         texture_params[gl_state.bind_textures.ids[active_texture]]->wrap_s = param;
         glTexParameteri(target, pname, param);
      }
   }
   else if (pname == GL_TEXTURE_WRAP_T && texture_params[gl_state.bind_textures.ids[active_texture]] != NULL) {
      if (texture_params[gl_state.bind_textures.ids[active_texture]]->wrap_t != param) {
         texture_params[gl_state.bind_textures.ids[active_texture]]->wrap_t = param;
         glTexParameteri(target, pname, param);
      }
   }
#ifndef HAVE_OPENGLES2
   else if (pname == GL_TEXTURE_MAX_LEVEL && texture_params[gl_state.bind_textures.ids[active_texture]] !=NULL) {
      if (texture_params[gl_state.bind_textures.ids[active_texture]]->max_level != param) {
         texture_params[gl_state.bind_textures.ids[active_texture]]->max_level = param;
         glTexParameteri(target, pname, param);
      }
   }
#endif
   else
      glTexParameteri(target, pname, param);
}

void rglTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
   glTexParameterf(target, pname, param);
}

/*
 * Category: Generic vertex attributes
 *
 * Core in:
 * OpenGL    : 2.0
 *
 */
void rglDisableVertexAttribArray(GLuint index)
{
   if (gl_state.vertex_attrib_pointer.enabled[index] != 0) {
      gl_state.vertex_attrib_pointer.enabled[index] = 0;
      glDisableVertexAttribArray(index);
   }
}

/*
 * Category: Generic vertex attributes
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglEnableVertexAttribArray(GLuint index)
{
   if (gl_state.vertex_attrib_pointer.enabled[index] != 1) {
      gl_state.vertex_attrib_pointer.enabled[index] = 1;
      glEnableVertexAttribArray(index);
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglVertexAttribIPointer(
      GLuint index,
      GLint size,
      GLenum type,
      GLsizei stride,
      const GLvoid * pointer)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES3)
   glVertexAttribIPointer(index, size, type, stride, pointer);
#endif
}

void rglVertexAttribLPointer(
      GLuint index,
      GLint size,
      GLenum type,
      GLsizei stride,
      const GLvoid * pointer)
{
#if defined(HAVE_OPENGL)
   glVertexAttribLPointer(index, size, type, stride, pointer);
#endif
}

/*
 * Category: Generic vertex attributes
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglVertexAttribPointer(GLuint name, GLint size,
      GLenum type, GLboolean normalized, GLsizei stride,
      const GLvoid* pointer)
{
   if (gl_state.attrib_pointer.size[name] != size || gl_state.attrib_pointer.type[name] != type || gl_state.attrib_pointer.normalized[name] != normalized || gl_state.attrib_pointer.stride[name] != stride || gl_state.attrib_pointer.pointer[name] != pointer || gl_state.attrib_pointer.buffer[name] != gl_state.array_buffer) {
      gl_state.attrib_pointer.used[name] = 1;
      gl_state.attrib_pointer.size[name] = size;
      gl_state.attrib_pointer.type[name] = type;
      gl_state.attrib_pointer.normalized[name] = normalized;
      gl_state.attrib_pointer.stride[name] = stride;
      gl_state.attrib_pointer.pointer[name] = pointer;
      gl_state.attrib_pointer.buffer[name] = gl_state.array_buffer;
      glVertexAttribPointer(name, size, type, normalized, stride, pointer);
   }
}

/*
 * Category: Generic vertex attributes
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
   glBindAttribLocation(program, index, name);
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglVertexAttrib4f(GLuint name, GLfloat x, GLfloat y,
      GLfloat z, GLfloat w)
{
   if (gl_state.vertex_attribs.v0[name] != x || gl_state.vertex_attribs.v1[name] != y || gl_state.vertex_attribs.v2[name] != z || gl_state.vertex_attribs.v3[name] != w) {
      glVertexAttrib4f(name, x, y, z, w);
      gl_state.vertex_attribs.v0[name] = x;
      gl_state.vertex_attribs.v1[name] = y;
      gl_state.vertex_attribs.v2[name] = z;
      gl_state.vertex_attribs.v3[name] = w;
   }
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglVertexAttrib4fv(GLuint name, GLfloat* v)
{
   if (gl_state.vertex_attribs.v0[name] != v[0] || gl_state.vertex_attribs.v1[name] != v[1] || gl_state.vertex_attribs.v2[name] != v[2] || gl_state.vertex_attribs.v3[name] != v[3]) {
      glVertexAttrib4fv(name, v);
      gl_state.vertex_attribs.v0[name] = v[0];
      gl_state.vertex_attribs.v1[name] = v[1];
      gl_state.vertex_attribs.v2[name] = v[2];
      gl_state.vertex_attribs.v3[name] = v[3];
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
GLuint rglCreateShader(GLenum shaderType)
{
   return glCreateShader(shaderType);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglDeleteProgram(GLuint program)
{
   int i;
   for (i = 0; i < MAX_UNIFORMS; ++i)
      free(program_uniforms[program][i]);
   if (!resetting_context)
      glDeleteProgram(program);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglDeleteShader(GLuint shader)
{
   if (!resetting_context)
      glDeleteShader(shader);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
GLint rglGetUniformLocation(GLuint program, const GLchar *name)
{
   return glGetUniformLocation(program, name);
}

/*
 * Category: VBO and PBO
 *
 * Core in:
 * OpenGL    : 1.5
 */
void rglDeleteBuffers(GLsizei n, const GLuint *buffers)
{
   glDeleteBuffers(n, buffers);
}

/*
 * Category: VBO and PBO
 *
 * Core in:
 * OpenGL    : 1.5
 */
void rglGenBuffers(GLsizei n, GLuint *buffers)
{
   glGenBuffers(n, buffers);
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform1f(GLint location, GLfloat v0)
{
   if (program_uniforms[gl_state.program][location]->uniform1f != v0) {
      glUniform1f(location, v0);
      program_uniforms[gl_state.program][location]->uniform1f = v0;
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform1fv(GLint location,  GLsizei count,  const GLfloat *value)
{
   if (program_uniforms[gl_state.program][location]->uniform1f != value[0]) {
      glUniform1fv(location, count, value);
      program_uniforms[gl_state.program][location]->uniform1f = value[0];
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform1iv(GLint location,  GLsizei count,  const GLint *value)
{
   if (program_uniforms[gl_state.program][location]->uniform1i != value[0]) {
      glUniform1iv(location, count, value);
      program_uniforms[gl_state.program][location]->uniform1i = value[0];
   }
}

void rglClearBufferfv( 	GLenum buffer,
  	GLint drawBuffer,
  	const GLfloat * value)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES_3)
   glClearBufferfv(buffer, drawBuffer, value);
#endif
}

void rglTexBuffer(GLenum target, GLenum internalFormat, GLuint buffer)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES_3_2)
   glTexBuffer(target, internalFormat, buffer);
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 * OpenGLES  : 3.0
 */
const GLubyte* rglGetStringi(GLenum name, GLuint index)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES_3)
   return glGetStringi(name, index);
#else
   return NULL;
#endif
}

void rglClearBufferfi( 	GLenum buffer,
  	GLint drawBuffer,
  	GLfloat depth,
  	GLint stencil)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES_3)
   glClearBufferfi(buffer, drawBuffer, depth, stencil);
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 3.0
 * OpenGLES  : 3.0
 */
void rglRenderbufferStorageMultisample( 	GLenum target,
  	GLsizei samples,
  	GLenum internalformat,
  	GLsizei width,
  	GLsizei height)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES_3)
   glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
#endif
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform1i(GLint location, GLint v0)
{
   if (program_uniforms[gl_state.program][location]->uniform1i != v0) {
      glUniform1i(location, v0);
      program_uniforms[gl_state.program][location]->uniform1i = v0;
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
   if (program_uniforms[gl_state.program][location]->uniform2f[0] != v0 || program_uniforms[gl_state.program][location]->uniform2f[1] != v1) {
      glUniform2f(location, v0, v1);
      program_uniforms[gl_state.program][location]->uniform2f[0] = v0;
      program_uniforms[gl_state.program][location]->uniform2f[1] = v1;
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform2i(GLint location, GLint v0, GLint v1)
{
   if (program_uniforms[gl_state.program][location]->uniform2i[0] != v0 || program_uniforms[gl_state.program][location]->uniform2i[1] != v1) {
      glUniform2i(location, v0, v1);
      program_uniforms[gl_state.program][location]->uniform2i[0] = v0;
      program_uniforms[gl_state.program][location]->uniform2i[1] = v1;
   }
}

void rglUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
   if (program_uniforms[gl_state.program][location]->uniform3i[0] != v0 || program_uniforms[gl_state.program][location]->uniform3i[1] != v1) {
      glUniform3i(location, v0, v1, v2);
      program_uniforms[gl_state.program][location]->uniform3i[0] = v0;
      program_uniforms[gl_state.program][location]->uniform3i[1] = v1;
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform2fv(GLint location, GLsizei count, const GLfloat *value)
{
   if (program_uniforms[gl_state.program][location]->uniform2f[0] != value[0] || program_uniforms[gl_state.program][location]->uniform2f[1] != value[1]) {
      glUniform2fv(location, count, value);
      program_uniforms[gl_state.program][location]->uniform2f[0] = value[0];
      program_uniforms[gl_state.program][location]->uniform2f[1] = value[1];
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
   if (program_uniforms[gl_state.program][location]->uniform3f[0] != v0 || program_uniforms[gl_state.program][location]->uniform3f[1] != v1 || program_uniforms[gl_state.program][location]->uniform3f[2] != v2) {
      glUniform3f(location, v0, v1, v2);
      program_uniforms[gl_state.program][location]->uniform3f[0] = v0;
      program_uniforms[gl_state.program][location]->uniform3f[1] = v1;
      program_uniforms[gl_state.program][location]->uniform3f[2] = v2;
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform3fv(GLint location, GLsizei count, const GLfloat *value)
{
   if (program_uniforms[gl_state.program][location]->uniform3f[0] != value[0] || program_uniforms[gl_state.program][location]->uniform3f[1] != value[1] || program_uniforms[gl_state.program][location]->uniform3f[2] != value[2]) {
      glUniform3fv(location, count, value);
      program_uniforms[gl_state.program][location]->uniform3f[0] = value[0];
      program_uniforms[gl_state.program][location]->uniform3f[1] = value[1];
      program_uniforms[gl_state.program][location]->uniform3f[2] = value[2];
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
   if (program_uniforms[gl_state.program][location]->uniform4i[0] != v0 || program_uniforms[gl_state.program][location]->uniform4i[1] != v1 || program_uniforms[gl_state.program][location]->uniform4i[2] != v2 || program_uniforms[gl_state.program][location]->uniform4i[3] != v3) {
      glUniform4i(location, v0, v1, v2, v3);
      program_uniforms[gl_state.program][location]->uniform4i[0] = v0;
      program_uniforms[gl_state.program][location]->uniform4i[1] = v1;
      program_uniforms[gl_state.program][location]->uniform4i[2] = v2;
      program_uniforms[gl_state.program][location]->uniform4i[3] = v3;
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
   if (program_uniforms[gl_state.program][location]->uniform4f[0] != v0 || program_uniforms[gl_state.program][location]->uniform4f[1] != v1 || program_uniforms[gl_state.program][location]->uniform4f[2] != v2 || program_uniforms[gl_state.program][location]->uniform4f[3] != v3) {
      glUniform4f(location, v0, v1, v2, v3);
      program_uniforms[gl_state.program][location]->uniform4f[0] = v0;
      program_uniforms[gl_state.program][location]->uniform4f[1] = v1;
      program_uniforms[gl_state.program][location]->uniform4f[2] = v2;
      program_uniforms[gl_state.program][location]->uniform4f[3] = v3;
   }
}

/*
 * Category: Shaders
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglUniform4fv(GLint location, GLsizei count, const GLfloat *value)
{
   if (program_uniforms[gl_state.program][location]->uniform4f[0] != value[0] || program_uniforms[gl_state.program][location]->uniform4f[1] != value[1] || program_uniforms[gl_state.program][location]->uniform4f[2] != value[2] || program_uniforms[gl_state.program][location]->uniform4f[3] != value[3]) {
      glUniform4fv(location, count, value);
      program_uniforms[gl_state.program][location]->uniform4f[0] = value[0];
      program_uniforms[gl_state.program][location]->uniform4f[1] = value[1];
      program_uniforms[gl_state.program][location]->uniform4f[2] = value[2];
      program_uniforms[gl_state.program][location]->uniform4f[3] = value[3];
   }
}


/*
 *
 * Core in:
 * OpenGL    : 1.0
 */
void rglPolygonOffset(GLfloat factor, GLfloat units)
{
   gl_state.polygonoffset.used = true;
   if (gl_state.polygonoffset.factor != factor || gl_state.polygonoffset.units != units) {
      glPolygonOffset(factor, units);
      gl_state.polygonoffset.factor = factor;
      gl_state.polygonoffset.units  = units;
   }
}

/*
 * Category: FBO
 *
 * Core in:
 * OpenGL    : 3.0
 */
void rglGenFramebuffers(GLsizei n, GLuint *ids)
{
   glGenFramebuffers(n, ids);
}

/*
 * Category: FBO
 *
 * Core in:
 * OpenGL    : 3.0
 */
void rglBindFramebuffer(GLenum target, GLuint framebuffer)
{
   if (framebuffer == 0)
      framebuffer = default_framebuffer;
   if (target == GL_FRAMEBUFFER) {
      if (gl_state.framebuf[0].location != framebuffer || gl_state.framebuf[1].location != framebuffer) {
#ifdef HAVE_OPENGLES
         if (gl_state.framebuf[0].has_depth) {
#ifdef HAVE_OPENGLES2
            glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards);
#else
            glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, discards);
#endif
            gl_state.framebuf[0].has_depth = 0;
         }
#ifndef HAVE_OPENGLES2
         if (gl_state.framebuf[1].has_depth) {
            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards);
            gl_state.framebuf[1].has_depth = 0;
         }
#endif
#endif
         glBindFramebuffer(target, framebuffer);
         gl_state.framebuf[0].location = framebuffer;
         gl_state.framebuf[1].location = framebuffer;
      }
   }
#ifndef HAVE_OPENGLES2
   else if (target == GL_DRAW_FRAMEBUFFER) {
      if (gl_state.framebuf[0].location != framebuffer) {
#ifdef HAVE_OPENGLES
         if (gl_state.framebuf[0].has_depth) {
            glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, discards);
            gl_state.framebuf[0].has_depth = 0;
         }
#endif
         glBindFramebuffer(target, framebuffer);
         gl_state.framebuf[0].location = framebuffer;
      }
   }
   else if (target == GL_READ_FRAMEBUFFER) {
      if (gl_state.framebuf[1].location != framebuffer) {
#ifdef HAVE_OPENGLES
         if (gl_state.framebuf[1].has_depth) {
            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards);
            gl_state.framebuf[1].has_depth = 0;
         }
#endif
         glBindFramebuffer(target, framebuffer);
         gl_state.framebuf[1].location = framebuffer;
      }
   }
#endif
}

/*
 * Category: FBO
 *
 * Core in:
 * OpenGL    : 2.0
 * OpenGLES  : 3.0
 */
void rglDrawBuffers(GLsizei n, const GLenum *bufs)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES3)
   glDrawBuffers(n, bufs);
#endif
}

/*
 * Category: FBO
 *
 * Core in:
 * OpenGL    : 2.0
 * OpenGLES  : 3.0
 */
void *rglMapBufferRange( 	GLenum target,
  	GLintptr offset,
  	GLsizeiptr length,
  	GLbitfield access)
{
#ifndef HAVE_OPENGLES2
   return glMapBufferRange(target, offset, length, access);
#endif
}

void rglFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
#ifndef HAVE_OPENGLES2
   glFlushMappedBufferRange(target, offset, length);
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 4.3
 * OpenGLES  : 3.1
 */
void rglTexStorage2DMultisample(GLenum target, GLsizei samples,
      GLenum internalformat, GLsizei width, GLsizei height,
      GLboolean fixedsamplelocations)
{
#if defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES_3_1)
   glTexStorage2DMultisample(target, samples, internalformat,
         width, height, fixedsamplelocations);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 *
 * Core in:
 * OpenGLES  : 3.0
 */
void rglTexStorage2D(GLenum target, GLsizei levels, GLenum internalFormat,
      GLsizei width, GLsizei height)
{
#ifndef HAVE_OPENGLES2
   glTexStorage2D(target, levels, internalFormat, width, height);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 4.2
 * OpenGLES  : 3.1
 */
void rglMemoryBarrier( 	GLbitfield barriers)
{
#if !defined(HAVE_OPENGLES) || defined(HAVE_OPENGLES3) && defined(HAVE_OPENGLES_3_1)
   glMemoryBarrier(barriers);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 4.2
 * OpenGLES  : 3.1
 */
void rglBindImageTexture( 	GLuint unit,
  	GLuint texture,
  	GLint level,
  	GLboolean layered,
  	GLint layer,
  	GLenum access,
  	GLenum format)
{
#if !defined(HAVE_OPENGLES) || defined(HAVE_OPENGLES3) && defined(HAVE_OPENGLES_3_1)
   glBindImageTexture(unit, texture, level, layered, layer, access, format);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 4.1
 * OpenGLES  : 3.1
 */
void rglGetProgramBinary( 	GLuint program,
  	GLsizei bufsize,
  	GLsizei *length,
  	GLenum *binaryFormat,
  	void *binary)
{
#ifndef HAVE_OPENGLES2
   glGetProgramBinary(program, bufsize, length, binaryFormat, binary);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 4.1
 * OpenGLES  : 3.1
 */
void rglProgramBinary(GLuint program,
  	GLenum binaryFormat,
  	const void *binary,
  	GLsizei length)
{
#ifndef HAVE_OPENGLES2
   glProgramBinary(program, binaryFormat, binary, length);
#else
   printf("WARNING! Not implemented.\n");
#endif
}

void rglTexImage2DMultisample( 	GLenum target,
  	GLsizei samples,
  	GLenum internalformat,
  	GLsizei width,
  	GLsizei height,
  	GLboolean fixedsamplelocations)
{
#ifndef HAVE_OPENGLES
   glTexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 1.5
 */

/*
 *
 * Core in:
 * OpenGL    : 1.5
 */

void rglBlendEquation(GLenum mode)
{
   glBlendEquation(mode);
}

void rglBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   glBlendColor(red, green, blue, alpha);
}

/*
 * Category: Blending
 *
 * Core in:
 * OpenGL    : 2.0
 */
void rglBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
   glBlendEquationSeparate(modeRGB, modeAlpha);
}

/*
 *
 * Core in:
 * OpenGL    : 2.0
 * OpenGLES  : 3.2
 */
void rglCopyImageSubData( 	GLuint srcName,
  	GLenum srcTarget,
  	GLint srcLevel,
  	GLint srcX,
  	GLint srcY,
  	GLint srcZ,
  	GLuint dstName,
  	GLenum dstTarget,
  	GLint dstLevel,
  	GLint dstX,
  	GLint dstY,
  	GLint dstZ,
  	GLsizei srcWidth,
  	GLsizei srcHeight,
  	GLsizei srcDepth)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES_3_2)
   glCopyImageSubData(srcName,
         srcTarget,
         srcLevel,
         srcX,
         srcY,
         srcZ,
         dstName,
         dstTarget,
         dstLevel,
         dstX,
         dstY,
         dstZ,
         srcWidth,
         srcHeight,
         srcDepth);
#endif
}

/*
 * Category: VAO
 *
 * Core in:
 * OpenGL    : 3.0
 * OpenGLES  : 3.0
 */
void rglBindVertexArray(GLuint array)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES3)
   gl_state.bindvertex.array = array;
   glBindVertexArray(array);
#endif
}

/*
 * Category: VAO
 *
 * Core in:
 * OpenGL    : 3.0
 * OpenGLES  : 3.0
 */
void rglGenVertexArrays(GLsizei n, GLuint *arrays)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES3)
   glGenVertexArrays(n, arrays);
#endif
}

/*
 * Category: VAO
 *
 * Core in:
 * OpenGL    : 3.0
 * OpenGLES  : 3.0
 */
void rglDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES3)
   glDeleteVertexArrays(n, arrays);
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 3.2
 * OpenGLES  : 3.0
 */
void *rglFenceSync(GLenum condition, GLbitfield flags)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES3)
   return (GLsync)glFenceSync(condition, flags);
#else
   return NULL;
#endif
}

/*
 *
 * Core in:
 * OpenGL    : 3.2
 * OpenGLES  : 3.0
 */
void rglWaitSync(void *sync, GLbitfield flags, uint64_t timeout)
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) && defined(HAVE_OPENGLES3)
   glWaitSync((GLsync)sync, flags, (GLuint64)timeout);
#endif
}

/* GLSM-side */

static void glsm_state_setup(void)
{
#ifdef OPENGL_DEBUG
#ifdef HAVE_OPENGLES
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR);
   glDebugMessageCallbackKHR(on_gl_error, NULL);
#else
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
   glDebugMessageCallback(on_gl_error, NULL);
#endif
#endif

   memset(&gl_state, 0, sizeof(struct gl_cached_state));

   gl_state.cap_translate[SGL_DEPTH_TEST]               = GL_DEPTH_TEST;
   gl_state.cap_translate[SGL_BLEND]                    = GL_BLEND;
   gl_state.cap_translate[SGL_POLYGON_OFFSET_FILL]      = GL_POLYGON_OFFSET_FILL;
   gl_state.cap_translate[SGL_FOG]                      = GL_FOG;
   gl_state.cap_translate[SGL_CULL_FACE]                = GL_CULL_FACE;
   gl_state.cap_translate[SGL_ALPHA_TEST]               = GL_ALPHA_TEST;
   gl_state.cap_translate[SGL_SCISSOR_TEST]             = GL_SCISSOR_TEST;
   gl_state.cap_translate[SGL_STENCIL_TEST]             = GL_STENCIL_TEST;
   gl_state.cap_translate[SGL_DITHER]                   = GL_DITHER;
   gl_state.cap_translate[SGL_SAMPLE_ALPHA_TO_COVERAGE] = GL_SAMPLE_ALPHA_TO_COVERAGE;
   gl_state.cap_translate[SGL_SAMPLE_COVERAGE]          = GL_SAMPLE_COVERAGE;
#ifndef HAVE_OPENGLES
   gl_state.cap_translate[SGL_COLOR_LOGIC_OP]       = GL_COLOR_LOGIC_OP;
   gl_state.cap_translate[SGL_CLIP_DISTANCE0]       = GL_CLIP_DISTANCE0;
   gl_state.cap_translate[SGL_DEPTH_CLAMP]          = GL_DEPTH_CLAMP;
#endif

   glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &glsm_max_textures);
   if (glsm_max_textures > 32)
      glsm_max_textures = 32;

   gl_state.array_buffer                = 0;
   gl_state.index_buffer                = 0;
   gl_state.bindvertex.array            = 0;
   default_framebuffer                  = hw_render.get_current_framebuffer();
   gl_state.framebuf[0].location        = default_framebuffer;
   gl_state.framebuf[1].location        = default_framebuffer;
   gl_state.framebuf[0].has_depth       = 0;
   gl_state.framebuf[1].has_depth       = 0;
   gl_state.cullface.mode               = GL_BACK;
   gl_state.frontface.mode              = GL_CCW;

   gl_state.blendfunc_separate.used     = false;
   gl_state.blendfunc_separate.srcRGB   = GL_ONE;
   gl_state.blendfunc_separate.dstRGB   = GL_ZERO;
   gl_state.blendfunc_separate.srcAlpha = GL_ONE;
   gl_state.blendfunc_separate.dstAlpha = GL_ZERO;

   gl_state.depthfunc.used              = false;

   gl_state.colormask.used              = false;
   gl_state.colormask.red               = GL_TRUE;
   gl_state.colormask.green             = GL_TRUE;
   gl_state.colormask.blue              = GL_TRUE;
   gl_state.colormask.alpha             = GL_TRUE;

   gl_state.polygonoffset.used          = false;
   gl_state.polygonoffset.factor        = 0;
   gl_state.polygonoffset.units         = 0;

   gl_state.depthfunc.func              = GL_LESS;

#ifndef HAVE_OPENGLES
   gl_state.colorlogicop                = GL_COPY;
#endif
}

static void glsm_state_bind(void)
{
   unsigned i;
#ifndef HAVE_OPENGLES2
   glBindVertexArray(gl_state.bindvertex.array);
#endif
   if (gl_state.array_buffer != 0)
      glBindBuffer(GL_ARRAY_BUFFER, gl_state.array_buffer);

   for (i = 0; i < MAX_ATTRIB; i++)
   {
      if (gl_state.vertex_attrib_pointer.enabled[i])
         glEnableVertexAttribArray(i);

      if (gl_state.attrib_pointer.used[i]) {
         if (gl_state.attrib_pointer.buffer[i] == gl_state.array_buffer) {
            glVertexAttribPointer(
               i,
               gl_state.attrib_pointer.size[i],
               gl_state.attrib_pointer.type[i],
               gl_state.attrib_pointer.normalized[i],
               gl_state.attrib_pointer.stride[i],
               gl_state.attrib_pointer.pointer[i]);
         } else
            gl_state.attrib_pointer.buffer[i] = 0;
      }
   }

   for(i = 0; i < SGL_CAP_MAX; i ++)
   {
      if (gl_state.cap_state[i])
         glEnable(gl_state.cap_translate[i]);
   }

#ifdef HAVE_OPENGLES2
   glBindFramebuffer(GL_FRAMEBUFFER, gl_state.framebuf[0].location);
#else
   if (gl_state.framebuf[0].location == gl_state.framebuf[1].location)
      glBindFramebuffer(GL_FRAMEBUFFER, gl_state.framebuf[0].location);
   else {
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl_state.framebuf[0].location);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, gl_state.framebuf[1].location);
   }
#endif

   if (gl_state.blendfunc.used)
      glBlendFunc(
            gl_state.blendfunc.sfactor,
            gl_state.blendfunc.dfactor);

   glClearColor(
         gl_state.clear_color.r,
         gl_state.clear_color.g,
         gl_state.clear_color.b,
         gl_state.clear_color.a);

   glUseProgram(gl_state.program);

   glViewport(
         gl_state.viewport.x,
         gl_state.viewport.y,
         gl_state.viewport.w,
         gl_state.viewport.h);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, gl_state.bind_textures.ids[0]);
   glActiveTexture(GL_TEXTURE0 + active_texture);
}

static void glsm_state_unbind(void)
{
   unsigned i;

   for (i = 0; i < SGL_CAP_MAX; i ++)
   {
      if (gl_state.cap_state[i])
         glDisable(gl_state.cap_translate[i]);
   }

   for (i = 0; i < MAX_ATTRIB; i++)
   {
      if (gl_state.vertex_attrib_pointer.enabled[i])
         glDisableVertexAttribArray(i);
   }
   glActiveTexture(GL_TEXTURE0);
}

static bool glsm_state_ctx_destroy(void *data)
{
#ifdef HAVE_OPENGLES
   glslopt_cleanup (ctx);
#endif

   return true;
}

static bool glsm_state_ctx_init(void *data)
{
   glsm_ctx_params_t *params = (glsm_ctx_params_t*)data;

   if (!params || !params->environ_cb)
      return false;

#ifdef HAVE_OPENGLES
#if defined(HAVE_OPENGLES_3_1)
   hw_render.context_type       = RETRO_HW_CONTEXT_OPENGLES_VERSION;
   hw_render.version_major      = 3;
   hw_render.version_minor      = 1;
#elif defined(HAVE_OPENGLES3)
   hw_render.context_type       = RETRO_HW_CONTEXT_OPENGLES3;
#else
   hw_render.context_type       = RETRO_HW_CONTEXT_OPENGLES2;
#endif
#else
#ifdef CORE
   hw_render.context_type       = RETRO_HW_CONTEXT_OPENGL_CORE;
   hw_render.version_major      = 3;
   hw_render.version_minor      = 3;
#else
   hw_render.context_type       = RETRO_HW_CONTEXT_OPENGL;
#endif
#endif
   hw_render.context_reset      = params->context_reset;
   hw_render.context_destroy    = params->context_destroy;
   hw_render.stencil            = params->stencil;
   hw_render.depth              = true;
   hw_render.bottom_left_origin = true;
   hw_render.cache_context      = true;
#ifdef OPENGL_DEBUG
   hw_render.debug_context      = true;
#endif

   if (!params->environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render))
      return false;

#ifdef HAVE_OPENGLES
#ifdef HAVE_OPENGLES2
   glslopt_target target = kGlslTargetOpenGLES20;
#else
   glslopt_target target = kGlslTargetOpenGLES30;
#endif
   ctx = glslopt_initialize(target);
#endif

   return true;
}

GLuint glsm_get_current_framebuffer(void)
{
   return hw_render.get_current_framebuffer();
}

extern void retroChangeWindow();

bool glsm_ctl(enum glsm_state_ctl state, void *data)
{
   switch (state)
   {
      case GLSM_CTL_IMM_VBO_DRAW:
         return false;
      case GLSM_CTL_IMM_VBO_DISABLE:
         return false;
      case GLSM_CTL_IS_IMM_VBO:
         return false;
      case GLSM_CTL_SET_IMM_VBO:
         break;
      case GLSM_CTL_UNSET_IMM_VBO:
         break;
      case GLSM_CTL_PROC_ADDRESS_GET:
         {
            glsm_ctx_proc_address_t *proc = (glsm_ctx_proc_address_t*)data;
            if (!hw_render.get_proc_address)
               return false;
            proc->addr = hw_render.get_proc_address;
         }
         break;
      case GLSM_CTL_STATE_CONTEXT_RESET:
         rglgen_resolve_symbols(hw_render.get_proc_address);
         if (window_first > 0) {
            resetting_context = 1;
            glsm_state_setup();
            retroChangeWindow();
            resetting_context = 0;
	 }
         else
            window_first = 1;
         break;
      case GLSM_CTL_STATE_CONTEXT_DESTROY:
         glsm_state_ctx_destroy(data);
         break;
      case GLSM_CTL_STATE_CONTEXT_INIT:
         return glsm_state_ctx_init(data);
      case GLSM_CTL_STATE_SETUP:
         glsm_state_setup();
         break;
      case GLSM_CTL_STATE_UNBIND:
         glsm_state_unbind();
         break;
      case GLSM_CTL_STATE_BIND:
         glsm_state_bind();
         break;
      case GLSM_CTL_NONE:
      default:
         break;
   }

   return true;
}
