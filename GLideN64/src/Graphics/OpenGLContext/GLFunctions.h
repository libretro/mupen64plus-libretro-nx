#ifndef GLFUNCTIONS_H
#define GLFUNCTIONS_H

#include <glsm/glsmsym.h>

#ifdef OS_WINDOWS
#include <windows.h>
#elif defined(OS_LINUX)
//#define GL_GLEXT_PROTOTYPES
#include <winlnxdefs.h>
#endif

#ifdef EGL
#include <GL/glcorearb.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#elif defined(OS_MAC_OS_X)
#include <OpenGL/OpenGL.h>
#include <stddef.h>
#include <OpenGL/gl3.h>
//#include <OpenGL/gl3ext.h>
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
#endif

#include <GL/glext.h>
#include <stdexcept>
#include <sstream>
#include "Log.h"

#ifdef GL_ERROR_DEBUG
#define CHECKED_GL_FUNCTION(proc_name, ...) checked([&]() { proc_name(__VA_ARGS__);}, #proc_name)
#define CHECKED_GL_FUNCTION_WITH_RETURN(proc_name, ReturnType, ...) checkedWithReturn<ReturnType>([&]() { return proc_name(__VA_ARGS__);}, #proc_name)
#else
#define CHECKED_GL_FUNCTION(proc_name, ...) proc_name(__VA_ARGS__)
#define CHECKED_GL_FUNCTION_WITH_RETURN(proc_name, ReturnType, ...) proc_name(__VA_ARGS__)
#endif

#define IS_GL_FUNCTION_VALID(proc_name) g_##proc_name != nullptr
#define GET_GL_FUNCTION(proc_name) g_##proc_name

#if defined(EGL) || defined(OS_IOS)

#define glTexImage2D(...) CHECKED_GL_FUNCTION(g_glTexImage2D, __VA_ARGS__)
#define glTexParameteri(...) CHECKED_GL_FUNCTION(g_glTexParameteri, __VA_ARGS__)
#define glGetIntegerv(...) CHECKED_GL_FUNCTION(g_glGetIntegerv, __VA_ARGS__)
#define glGetString(...) CHECKED_GL_FUNCTION_WITH_RETURN(g_glGetString, const GLubyte*, __VA_ARGS__)
#define glGetFloatv(...) CHECKED_GL_FUNCTION(g_glGetFloatv, __VA_ARGS__)
#define glTexParameterf(...) CHECKED_GL_FUNCTION(g_glTexParameterf, __VA_ARGS__)
#define glFinish(...) CHECKED_GL_FUNCTION(g_glFinish, __VA_ARGS__)
#if defined(OS_ANDROID)
#define eglGetNativeClientBufferANDROID(...) CHECKED_GL_FUNCTION_WITH_RETURN(g_eglGetNativeClientBufferANDROID, EGLClientBuffer, __VA_ARGS__)
#endif

extern PFNGLBLENDFUNCPROC g_glBlendFunc;
extern PFNGLPIXELSTOREIPROC g_glPixelStorei;
extern PFNGLCLEARCOLORPROC g_glClearColor;
extern PFNGLCULLFACEPROC g_glCullFace;
extern PFNGLDEPTHFUNCPROC g_glDepthFunc;
extern PFNGLDEPTHMASKPROC g_glDepthMask;
extern PFNGLDISABLEPROC g_glDisable;
extern PFNGLENABLEPROC g_glEnable;
extern PFNGLPOLYGONOFFSETPROC g_glPolygonOffset;
extern PFNGLSCISSORPROC g_glScissor;
extern PFNGLVIEWPORTPROC g_glViewport;
extern PFNGLBINDTEXTUREPROC g_glBindTexture;
extern PFNGLTEXIMAGE2DPROC g_glTexImage2D;
extern PFNGLTEXPARAMETERIPROC g_glTexParameteri;
extern PFNGLGETINTEGERVPROC g_glGetIntegerv;
extern PFNGLGETSTRINGPROC g_glGetString;
extern PFNGLREADPIXELSPROC g_glReadPixels;
extern PFNGLTEXSUBIMAGE2DPROC g_glTexSubImage2D;
extern PFNGLDRAWARRAYSPROC g_glDrawArrays;
extern PFNGLGETERRORPROC g_glGetError;
extern PFNGLDRAWELEMENTSPROC g_glDrawElements;
extern PFNGLLINEWIDTHPROC g_glLineWidth;
extern PFNGLCLEARPROC g_glClear;
extern PFNGLGETFLOATVPROC g_glGetFloatv;
extern PFNGLDELETETEXTURESPROC g_glDeleteTextures;
extern PFNGLGENTEXTURESPROC g_glGenTextures;
extern PFNGLTEXPARAMETERFPROC g_glTexParameterf;
extern PFNGLACTIVETEXTUREPROC g_glActiveTexture;
extern PFNGLBLENDCOLORPROC g_glBlendColor;
extern PFNGLREADBUFFERPROC g_glReadBuffer;
extern PFNGLFINISHPROC g_glFinish;
#if defined(OS_ANDROID)
extern PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC g_eglGetNativeClientBufferANDROID;
#endif
#endif

#ifdef OS_WINDOWS
extern PFNGLACTIVETEXTUREPROC g_glActiveTexture;
extern PFNGLBLENDCOLORPROC g_glBlendColor;
#endif

#define glEnablei(...) CHECKED_GL_FUNCTION(g_glEnablei, __VA_ARGS__)
#define glDisablei(...) CHECKED_GL_FUNCTION(g_glDisablei, __VA_ARGS__)
#define glTextureBarrier(...) CHECKED_GL_FUNCTION(g_glTextureBarrier, __VA_ARGS__)
#define glTextureBarrierNV(...) CHECKED_GL_FUNCTION(g_glTextureBarrierNV, __VA_ARGS__)

#define glTextureParameteri(...) CHECKED_GL_FUNCTION(g_glTextureParameteri, __VA_ARGS__)
#define glTextureParameterf(...) CHECKED_GL_FUNCTION(g_glTextureParameterf, __VA_ARGS__)
#define glCreateTextures(...) CHECKED_GL_FUNCTION(g_glCreateTextures, __VA_ARGS__)
#define glTextureStorage2D(...) CHECKED_GL_FUNCTION(g_glTextureStorage2D, __VA_ARGS__)
#define glTextureSubImage2D(...) CHECKED_GL_FUNCTION(g_glTextureSubImage2D, __VA_ARGS__)
#define glVertexAttrib1f(...) CHECKED_GL_FUNCTION(g_glVertexAttrib1f, __VA_ARGS__)
#define glCreateFramebuffers(...) CHECKED_GL_FUNCTION(g_glCreateFramebuffers, __VA_ARGS__)
#define glNamedFramebufferTexture(...) CHECKED_GL_FUNCTION(g_glNamedFramebufferTexture, __VA_ARGS__)

extern PFNGLCREATESHADERPROC g_glCreateShader;
extern PFNGLCOMPILESHADERPROC g_glCompileShader;
extern PFNGLSHADERSOURCEPROC g_glShaderSource;
extern PFNGLCREATEPROGRAMPROC g_glCreateProgram;
extern PFNGLATTACHSHADERPROC g_glAttachShader;
extern PFNGLLINKPROGRAMPROC g_glLinkProgram;
extern PFNGLUSEPROGRAMPROC g_glUseProgram;
extern PFNGLGETUNIFORMLOCATIONPROC g_glGetUniformLocation;
extern PFNGLUNIFORM1IPROC g_glUniform1i;
extern PFNGLUNIFORM1FPROC g_glUniform1f;
extern PFNGLUNIFORM2FPROC g_glUniform2f;
extern PFNGLUNIFORM2IPROC g_glUniform2i;
extern PFNGLUNIFORM4IPROC g_glUniform4i;

extern PFNGLUNIFORM4FPROC g_glUniform4f;
extern PFNGLUNIFORM3FVPROC g_glUniform3fv;
extern PFNGLUNIFORM4FVPROC g_glUniform4fv;
extern PFNGLDETACHSHADERPROC g_glDetachShader;
extern PFNGLDELETESHADERPROC g_glDeleteShader;
extern PFNGLDELETEPROGRAMPROC g_glDeleteProgram;
extern PFNGLGETPROGRAMINFOLOGPROC g_glGetProgramInfoLog;
extern PFNGLGETSHADERINFOLOGPROC g_glGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC g_glGetShaderiv;
extern PFNGLGETPROGRAMIVPROC g_glGetProgramiv;

extern PFNGLENABLEVERTEXATTRIBARRAYPROC g_glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC g_glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC g_glVertexAttribPointer;
extern PFNGLBINDATTRIBLOCATIONPROC g_glBindAttribLocation;
extern PFNGLVERTEXATTRIB1FPROC g_glVertexAttrib1f;
extern PFNGLVERTEXATTRIB4FPROC g_glVertexAttrib4f;
extern PFNGLVERTEXATTRIB4FVPROC g_glVertexAttrib4fv;

extern PFNGLDEPTHRANGEFPROC g_glDepthRangef;
extern PFNGLCLEARDEPTHFPROC g_glClearDepthf;

extern PFNGLDRAWBUFFERSPROC g_glDrawBuffers;
extern PFNGLGENFRAMEBUFFERSPROC g_glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC g_glBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC g_glDeleteFramebuffers;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC g_glFramebufferTexture2D;
extern PFNGLTEXIMAGE2DMULTISAMPLEPROC g_glTexImage2DMultisample;
extern PFNGLTEXSTORAGE2DMULTISAMPLEPROC g_glTexStorage2DMultisample;
extern PFNGLGENRENDERBUFFERSPROC g_glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC g_glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC g_glRenderbufferStorage;
extern PFNGLDELETERENDERBUFFERSPROC g_glDeleteRenderbuffers;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC g_glFramebufferRenderbuffer;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC g_glCheckFramebufferStatus;
extern PFNGLBLITFRAMEBUFFERPROC g_glBlitFramebuffer;
extern PFNGLGENVERTEXARRAYSPROC g_glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC g_glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC g_glDeleteVertexArrays;
extern PFNGLGENBUFFERSPROC g_glGenBuffers;
extern PFNGLBINDBUFFERPROC g_glBindBuffer;
extern PFNGLBUFFERDATAPROC g_glBufferData;
extern PFNGLMAPBUFFERPROC g_glMapBuffer;
extern PFNGLMAPBUFFERRANGEPROC g_glMapBufferRange;
extern PFNGLUNMAPBUFFERPROC g_glUnmapBuffer;
extern PFNGLDELETEBUFFERSPROC g_glDeleteBuffers;
extern PFNGLBINDIMAGETEXTUREPROC g_glBindImageTexture;
extern PFNGLMEMORYBARRIERPROC g_glMemoryBarrier;
extern PFNGLGETSTRINGIPROC g_glGetStringi;
extern PFNGLINVALIDATEFRAMEBUFFERPROC g_glInvalidateFramebuffer;
extern PFNGLBUFFERSTORAGEPROC g_glBufferStorage;
extern PFNGLFENCESYNCPROC g_glFenceSync;
extern PFNGLCLIENTWAITSYNCPROC g_glClientWaitSync;
extern PFNGLDELETESYNCPROC g_glDeleteSync;

extern PFNGLGETUNIFORMBLOCKINDEXPROC g_glGetUniformBlockIndex;
extern PFNGLUNIFORMBLOCKBINDINGPROC g_glUniformBlockBinding;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC g_glGetActiveUniformBlockiv;
extern PFNGLGETUNIFORMINDICESPROC g_glGetUniformIndices;
extern PFNGLGETACTIVEUNIFORMSIVPROC g_glGetActiveUniformsiv;
extern PFNGLBINDBUFFERBASEPROC g_glBindBufferBase;
extern PFNGLBUFFERSUBDATAPROC g_glBufferSubData;

extern PFNGLGETPROGRAMBINARYPROC g_glGetProgramBinary;
extern PFNGLPROGRAMBINARYPROC g_glProgramBinary;
extern PFNGLPROGRAMPARAMETERIPROC g_glProgramParameteri;

extern PFNGLTEXSTORAGE2DPROC g_glTexStorage2D;
extern PFNGLTEXTURESTORAGE2DPROC g_glTextureStorage2D;
extern PFNGLTEXTURESUBIMAGE2DPROC g_glTextureSubImage2D;
extern PFNGLTEXTURESTORAGE2DMULTISAMPLEEXTPROC g_glTextureStorage2DMultisample;
extern PFNGLTEXTUREPARAMETERIPROC g_glTextureParameteri;
extern PFNGLTEXTUREPARAMETERFPROC g_glTextureParameterf;
extern PFNGLCREATETEXTURESPROC g_glCreateTextures;
extern PFNGLCREATEBUFFERSPROC g_glCreateBuffers;
extern PFNGLCREATEFRAMEBUFFERSPROC g_glCreateFramebuffers;
extern PFNGLNAMEDFRAMEBUFFERTEXTUREPROC g_glNamedFramebufferTexture;
extern PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC g_glDrawRangeElementsBaseVertex;
extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC g_glFlushMappedBufferRange;
extern PFNGLTEXTUREBARRIERPROC g_glTextureBarrier;
extern PFNGLTEXTUREBARRIERNVPROC g_glTextureBarrierNV;
extern PFNGLCLEARBUFFERFVPROC g_glClearBufferfv;
extern PFNGLENABLEIPROC g_glEnablei;
extern PFNGLDISABLEIPROC g_glDisablei;

typedef void (APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, void* image);
extern PFNGLEGLIMAGETARGETTEXTURE2DOESPROC g_glEGLImageTargetTexture2DOES;

void initGLFunctions();

template<typename F> void checked(F fn, const char* _functionName)
{
	fn();
	auto error = glGetError();
	if (error != GL_NO_ERROR) {
		std::stringstream errorString;
		errorString << _functionName << " OpenGL error: 0x" << std::hex << error;
		LOG(LOG_ERROR, errorString.str().c_str());
		throw std::runtime_error(errorString.str().c_str());
	}
}

template<typename R, typename F> R checkedWithReturn(F fn, const char* _functionName)
{
	R returnValue = fn();
	auto error = glGetError();
	if (error != GL_NO_ERROR) {
		std::stringstream errorString;
		errorString << _functionName << " OpenGL error: 0x" << std::hex << error;
		LOG(LOG_ERROR, errorString.str().c_str());
		throw std::runtime_error(errorString.str().c_str());
	}

	return returnValue;
}

#endif // GLFUNCTIONS_H
