#include "GLFunctions.h"

#ifdef OS_WINDOWS

#define glGetProcAddress wglGetProcAddress
#define GL_GET_PROC_ADR(proc_type, proc_name) g_##proc_name = (proc_type) glGetProcAddress(#proc_name)

#elif defined(VERO4K) || defined(ODROID) || defined(VC)

#define GL_GET_PROC_ADR(proc_type, proc_name) g_##proc_name = (proc_type) dlsym(gles2so, #proc_name);

#elif defined(EGL)

#define glGetProcAddress eglGetProcAddress
#define GL_GET_PROC_ADR(proc_type, proc_name) g_##proc_name = (proc_type) glGetProcAddress(#proc_name)

#elif defined(OS_LINUX)

#include <X11/Xutil.h>
typedef XID GLXContextID;
typedef XID GLXPixmap;
typedef XID GLXDrawable;
typedef XID GLXPbuffer;
typedef XID GLXWindow;
typedef XID GLXFBConfigID;
typedef struct __GLXcontextRec *GLXContext;
typedef struct __GLXFBConfigRec *GLXFBConfig;
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glxext.h>
#define glGetProcAddress glXGetProcAddress
#define GL_GET_PROC_ADR(proc_type, proc_name) g_##proc_name = (proc_type) glGetProcAddress((const GLubyte*)#proc_name)

#elif defined(OS_MAC_OS_X)
#include <dlfcn.h>

static void* AppleGLGetProcAddress (const char *name)
{
	static void* image = NULL;
	if (NULL == image)
		image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);

	return (image ? dlsym(image, name) : NULL);
}
#define glGetProcAddress AppleGLGetProcAddress
#define GL_GET_PROC_ADR(proc_type, proc_name) g_##proc_name = (proc_type) glGetProcAddress(#proc_name)

#elif defined(OS_IOS)
#include <dlfcn.h>

static void* IOSGLGetProcAddress (const char *name)
{
    return dlsym(RTLD_DEFAULT, name);
}

#define glGetProcAddress IOSGLGetProcAddress
#define GL_GET_PROC_ADR(proc_type, proc_name) g_##proc_name = (proc_type)glGetProcAddress(#proc_name)

#endif

//GL Fucntions

#ifdef OS_WINDOWS
PFNGLACTIVETEXTUREPROC g_glActiveTexture;
PFNGLBLENDCOLORPROC g_glBlendColor;
#elif defined(EGL) || defined(OS_IOS)
PFNGLBLENDFUNCPROC g_glBlendFunc;
PFNGLPIXELSTOREIPROC g_glPixelStorei;
PFNGLCLEARCOLORPROC g_glClearColor;
PFNGLCULLFACEPROC g_glCullFace;
PFNGLDEPTHFUNCPROC g_glDepthFunc;
PFNGLDEPTHMASKPROC g_glDepthMask;
PFNGLDISABLEPROC g_glDisable;
PFNGLENABLEPROC g_glEnable;
PFNGLPOLYGONOFFSETPROC g_glPolygonOffset;
PFNGLSCISSORPROC g_glScissor;
PFNGLVIEWPORTPROC g_glViewport;
PFNGLBINDTEXTUREPROC g_glBindTexture;
PFNGLTEXIMAGE2DPROC g_glTexImage2D;
PFNGLTEXPARAMETERIPROC g_glTexParameteri;
PFNGLGETINTEGERVPROC g_glGetIntegerv;
PFNGLGETSTRINGPROC g_glGetString;
PFNGLREADPIXELSPROC g_glReadPixels;
PFNGLTEXSUBIMAGE2DPROC g_glTexSubImage2D;
PFNGLDRAWARRAYSPROC g_glDrawArrays;
PFNGLGETERRORPROC g_glGetError;
PFNGLDRAWELEMENTSPROC g_glDrawElements;
PFNGLLINEWIDTHPROC g_glLineWidth;
PFNGLCLEARPROC g_glClear;
PFNGLGETFLOATVPROC g_glGetFloatv;
PFNGLDELETETEXTURESPROC g_glDeleteTextures;
PFNGLGENTEXTURESPROC g_glGenTextures;
PFNGLTEXPARAMETERFPROC g_glTexParameterf;
PFNGLACTIVETEXTUREPROC g_glActiveTexture;
PFNGLBLENDCOLORPROC g_glBlendColor;
PFNGLREADBUFFERPROC g_glReadBuffer;
PFNGLFINISHPROC g_glFinish;
#if defined(OS_ANDROID)
PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC g_eglGetNativeClientBufferANDROID;
#endif
#endif
PFNGLCREATESHADERPROC g_glCreateShader;
PFNGLCOMPILESHADERPROC g_glCompileShader;
PFNGLSHADERSOURCEPROC g_glShaderSource;
PFNGLCREATEPROGRAMPROC g_glCreateProgram;
PFNGLATTACHSHADERPROC g_glAttachShader;
PFNGLLINKPROGRAMPROC g_glLinkProgram;
PFNGLUSEPROGRAMPROC g_glUseProgram;
PFNGLGETUNIFORMLOCATIONPROC g_glGetUniformLocation;
PFNGLUNIFORM1IPROC g_glUniform1i;
PFNGLUNIFORM1FPROC g_glUniform1f;
PFNGLUNIFORM2FPROC g_glUniform2f;
PFNGLUNIFORM2IPROC g_glUniform2i;
PFNGLUNIFORM4IPROC g_glUniform4i;
PFNGLUNIFORM4FPROC g_glUniform4f;
PFNGLUNIFORM3FVPROC g_glUniform3fv;
PFNGLUNIFORM4FVPROC g_glUniform4fv;
PFNGLDETACHSHADERPROC g_glDetachShader;
PFNGLDELETESHADERPROC g_glDeleteShader;
PFNGLDELETEPROGRAMPROC g_glDeleteProgram;
PFNGLGETPROGRAMINFOLOGPROC g_glGetProgramInfoLog;
PFNGLGETSHADERINFOLOGPROC g_glGetShaderInfoLog;
PFNGLGETSHADERIVPROC g_glGetShaderiv;
PFNGLGETPROGRAMIVPROC g_glGetProgramiv;

PFNGLENABLEVERTEXATTRIBARRAYPROC g_glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC g_glDisableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC g_glVertexAttribPointer;
PFNGLBINDATTRIBLOCATIONPROC g_glBindAttribLocation;
PFNGLVERTEXATTRIB1FPROC g_glVertexAttrib1f;
PFNGLVERTEXATTRIB4FPROC g_glVertexAttrib4f;
PFNGLVERTEXATTRIB4FVPROC g_glVertexAttrib4fv;

// multitexture functions
PFNGLDEPTHRANGEFPROC g_glDepthRangef;
PFNGLCLEARDEPTHFPROC g_glClearDepthf;

PFNGLDRAWBUFFERSPROC g_glDrawBuffers;
PFNGLBINDFRAMEBUFFERPROC g_glBindFramebuffer;
PFNGLDELETEFRAMEBUFFERSPROC g_glDeleteFramebuffers;
PFNGLGENFRAMEBUFFERSPROC g_glGenFramebuffers;
PFNGLFRAMEBUFFERTEXTURE2DPROC g_glFramebufferTexture2D;
PFNGLTEXIMAGE2DMULTISAMPLEPROC g_glTexImage2DMultisample;
PFNGLTEXSTORAGE2DMULTISAMPLEPROC g_glTexStorage2DMultisample;
PFNGLGENRENDERBUFFERSPROC g_glGenRenderbuffers;
PFNGLBINDRENDERBUFFERPROC g_glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC g_glRenderbufferStorage;
PFNGLFRAMEBUFFERRENDERBUFFERPROC g_glFramebufferRenderbuffer;
PFNGLDELETERENDERBUFFERSPROC g_glDeleteRenderbuffers;
PFNGLCHECKFRAMEBUFFERSTATUSPROC g_glCheckFramebufferStatus;
PFNGLBLITFRAMEBUFFERPROC g_glBlitFramebuffer;
PFNGLGENVERTEXARRAYSPROC g_glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC g_glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC g_glDeleteVertexArrays;
PFNGLGENBUFFERSPROC g_glGenBuffers;
PFNGLBINDBUFFERPROC g_glBindBuffer;
PFNGLBUFFERDATAPROC g_glBufferData;
PFNGLMAPBUFFERPROC g_glMapBuffer;
PFNGLMAPBUFFERRANGEPROC g_glMapBufferRange;
PFNGLUNMAPBUFFERPROC g_glUnmapBuffer;
PFNGLDELETEBUFFERSPROC g_glDeleteBuffers;
PFNGLBINDIMAGETEXTUREPROC g_glBindImageTexture;
PFNGLMEMORYBARRIERPROC g_glMemoryBarrier;
PFNGLGETSTRINGIPROC g_glGetStringi;
PFNGLINVALIDATEFRAMEBUFFERPROC g_glInvalidateFramebuffer;
PFNGLBUFFERSTORAGEPROC g_glBufferStorage;
PFNGLFENCESYNCPROC g_glFenceSync;
PFNGLCLIENTWAITSYNCPROC g_glClientWaitSync;
PFNGLDELETESYNCPROC g_glDeleteSync;

PFNGLGETUNIFORMBLOCKINDEXPROC g_glGetUniformBlockIndex;
PFNGLUNIFORMBLOCKBINDINGPROC g_glUniformBlockBinding;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC g_glGetActiveUniformBlockiv;
PFNGLGETUNIFORMINDICESPROC g_glGetUniformIndices;
PFNGLGETACTIVEUNIFORMSIVPROC g_glGetActiveUniformsiv;
PFNGLBINDBUFFERBASEPROC g_glBindBufferBase;
PFNGLBUFFERSUBDATAPROC g_glBufferSubData;

PFNGLGETPROGRAMBINARYPROC g_glGetProgramBinary;
PFNGLPROGRAMBINARYPROC g_glProgramBinary;
PFNGLPROGRAMPARAMETERIPROC g_glProgramParameteri;

PFNGLTEXSTORAGE2DPROC g_glTexStorage2D;
PFNGLTEXTURESTORAGE2DPROC g_glTextureStorage2D;
PFNGLTEXTURESUBIMAGE2DPROC g_glTextureSubImage2D;
PFNGLTEXTURESTORAGE2DMULTISAMPLEEXTPROC g_glTextureStorage2DMultisample;
PFNGLTEXTUREPARAMETERIPROC g_glTextureParameteri;
PFNGLTEXTUREPARAMETERFPROC g_glTextureParameterf;
PFNGLCREATETEXTURESPROC g_glCreateTextures;
PFNGLCREATEBUFFERSPROC g_glCreateBuffers;
PFNGLCREATEFRAMEBUFFERSPROC g_glCreateFramebuffers;
PFNGLNAMEDFRAMEBUFFERTEXTUREPROC g_glNamedFramebufferTexture;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC g_glDrawRangeElementsBaseVertex;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC g_glFlushMappedBufferRange;
PFNGLTEXTUREBARRIERPROC g_glTextureBarrier;
PFNGLTEXTUREBARRIERNVPROC g_glTextureBarrierNV;
PFNGLCLEARBUFFERFVPROC g_glClearBufferfv;
PFNGLENABLEIPROC g_glEnablei;
PFNGLDISABLEIPROC g_glDisablei;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC g_glEGLImageTargetTexture2DOES;

void initGLFunctions()
{
#ifdef VC
	void *gles2so = dlopen("/opt/vc/lib/libbrcmGLESv2.so", RTLD_NOW);
#elif defined(ODROID)
	void *gles2so = dlopen("/usr/lib/arm-linux-gnueabihf/libGLESv2.so", RTLD_NOW);
#elif defined(VERO4K)
       void *gles2so = dlopen("/opt/vero3/lib/libGLESv2.so", RTLD_NOW);
#endif

#if defined(EGL) || defined(OS_IOS)
	GL_GET_PROC_ADR(PFNGLTEXIMAGE2DPROC, glTexImage2D);
	GL_GET_PROC_ADR(PFNGLTEXPARAMETERIPROC, glTexParameteri);
	GL_GET_PROC_ADR(PFNGLGETINTEGERVPROC, glGetIntegerv);
	GL_GET_PROC_ADR(PFNGLGETSTRINGPROC, glGetString);
	GL_GET_PROC_ADR(PFNGLGETFLOATVPROC, glGetFloatv);
	GL_GET_PROC_ADR(PFNGLTEXPARAMETERFPROC, glTexParameterf);
	GL_GET_PROC_ADR(PFNGLFINISHPROC, glFinish);
#ifdef OS_ANDROID
	GL_GET_PROC_ADR(PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC, eglGetNativeClientBufferANDROID);
#endif
#endif

	GL_GET_PROC_ADR(PFNGLENABLEIPROC, glEnablei);
	GL_GET_PROC_ADR(PFNGLDISABLEIPROC, glDisablei);
	GL_GET_PROC_ADR(PFNGLTEXTUREBARRIERPROC, glTextureBarrier);
	GL_GET_PROC_ADR(PFNGLTEXTUREBARRIERNVPROC, glTextureBarrierNV);
	GL_GET_PROC_ADR(PFNGLTEXTUREPARAMETERIPROC, glTextureParameteri);
	GL_GET_PROC_ADR(PFNGLTEXTUREPARAMETERFPROC, glTextureParameterf);
	GL_GET_PROC_ADR(PFNGLCREATETEXTURESPROC, glCreateTextures);
	GL_GET_PROC_ADR(PFNGLTEXTURESTORAGE2DPROC, glTextureStorage2D);
	GL_GET_PROC_ADR(PFNGLTEXTURESUBIMAGE2DPROC, glTextureSubImage2D);
	GL_GET_PROC_ADR(PFNGLVERTEXATTRIB1FPROC, glVertexAttrib1f);
	GL_GET_PROC_ADR(PFNGLCREATEFRAMEBUFFERSPROC, glCreateFramebuffers);
	GL_GET_PROC_ADR(PFNGLNAMEDFRAMEBUFFERTEXTUREPROC, glNamedFramebufferTexture);
}
