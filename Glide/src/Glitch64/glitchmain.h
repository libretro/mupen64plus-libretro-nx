/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2002  Dave2001
* Copyright (c) 2003-2009  Sergey 'Gonetz' Lipski
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef MAIN_H
#define MAIN_H

#include <m64p_types.h>
#include <m64p_config.h>
#include <libretro.h>
#include <boolean.h>

extern retro_log_printf_t log_cb;

#define LOG_TO_STDOUT

#if defined(LOG_TO_STDERR)
#define LOG_TYPE stderr
#elif defined(LOG_TO_STDOUT)
#define LOG_TYPE stdout
#else
#define LOG_TYPE stderr
#endif

//#define DEBUGLOG
#ifdef DEBUGLOG
#define LOG(...) WriteLog(M64MSG_VERBOSE, __VA_ARGS__)
#define LOGINFO(...) WriteLog(M64MSG_INFO, __VA_ARGS__)
#else
#define LOG(...)
#define LOGINFO(...)
#endif

#ifdef TEXTUREMANAGEMENT_LOG
#define TEXLOG(...) fprintf(LOG_TYPE, __VA_ARGS__)
#else
#define TEXLOG(...)
#endif

void WriteLog(m64p_msg_level level, const char *msg, ...);

#define zscale 1.0f

extern int packed_pixels_support;

void set_depth_shader(void);

#include <stdio.h>

#ifdef EGL
#include <GL/glcorearb.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#elif defined(OS_MAC_OS_X)
#include <OpenGL/OpenGL.h>
#include <stddef.h>
#include <OpenGL/gl3.h>

#elif defined(OS_IOS)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
// Add missing type defintions for iOS
typedef double GLclampd;
typedef double GLdouble;
// These will get redefined by other GL headers.
#undef GL_DRAW_FRAMEBUFFER_BINDING
#undef GL_COPY_READ_BUFFER_BINDING
#undef GL_COPY_WRITE_BUFFER_BINDING
#include <GL/glcorearb.h>
#else
#include <GL/gl.h>
#include <GL/glcorearb.h>
#endif

#define GL_LUMINANCE 0x1909
#include <GL/glext.h>

#if !defined(EGL) && !defined(OS_IOS)
typedef void (APIENTRYP PFNGLPOLYGONOFFSETPROC) (GLfloat factor, GLfloat units);
typedef void (APIENTRYP PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC) (GLenum mode, GLint first, GLsizei count);
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void (APIENTRYP PFNGLDELETETEXTURESPROC) (GLsizei n, const GLuint *textures);
typedef void (APIENTRYP PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
typedef void (APIENTRYP PFNGLCOPYTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
#endif

#include <glsm/glsm.h>
#include <glsm/glsmsym.h>

// Defined in GLideN64's GLFunctions.cpp, initialized via glsm
extern PFNGLGETSTRINGPROC ptrGetString;
extern PFNGLTEXIMAGE2DPROC ptrTexImage2D;
extern PFNGLTEXPARAMETERIPROC ptrTexParameteri;

void init_textures(void);
void free_textures(void);

void init_geometry(void);
void free_geometry(void);

void set_lambda(void);

void init_combiner(void);
void updateCombiner(int i);
void updateCombinera(int i);
void check_compile(GLuint shader);
void check_link(GLuint program);
void free_combiners(void);
void compile_shader(void);
void set_copy_shader(void);

//Vertex Attribute Locations
#define POSITION_ATTR 0
#define COLOUR_ATTR 1
#define TEXCOORD_0_ATTR 2
#define TEXCOORD_1_ATTR 3
#define FOG_ATTR 4

extern int width, height;
extern int tex_width[2], tex_height[2];
extern int tex_exactWidth[2], tex_exactHeight[2];
extern float texture_env_color[4];
extern int fog_enabled;
extern float lambda;
extern int need_lambda[2];
extern float lambda_color[2][4];
extern int culling_mode;
extern int need_to_compile;
extern int three_point_filter[2];

extern int bgra8888_support;
extern int glsl_support;

extern GLuint program_object_default;
extern GLuint glitch_vbo;

#ifndef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS // TODO: Not present
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS 18283
#endif

#define CHECK_FRAMEBUFFER_STATUS() \
{\
 GLenum status; \
 status = glCheckFramebufferStatus(GL_FRAMEBUFFER); \
 /*DISPLAY_WARNING("%x\n", status);*/\
 switch(status) { \
 case GL_FRAMEBUFFER_COMPLETE: \
   /*DISPLAY_WARNING("framebuffer complete!\n");*/\
   break; \
 case GL_FRAMEBUFFER_UNSUPPORTED: \
   DISPLAY_WARNING("framebuffer GL_FRAMEBUFFER_UNSUPPORTED_EXT\n");\
    /* you gotta choose different formats */ \
   /*assert(0);*/ \
   break; \
 case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: \
   DISPLAY_WARNING("framebuffer INCOMPLETE_ATTACHMENT\n");\
   break; \
 case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: \
   DISPLAY_WARNING("framebuffer FRAMEBUFFER_MISSING_ATTACHMENT\n");\
   break; \
 case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: \
   DISPLAY_WARNING("framebuffer FRAMEBUFFER_DIMENSIONS\n");\
   break; \
 default: \
   break; \
   /* programming error; will fail on all hardware */ \
   /*assert(0);*/ \
 }\
}

#endif
