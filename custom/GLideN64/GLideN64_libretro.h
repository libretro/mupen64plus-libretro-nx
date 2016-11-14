#include "Types.h"

#ifndef GLIDEN64_LIBRETRO_H
#define GLIDEN64_LIBRETRO_H

#ifdef GLES2
#define GL_RGBA8 GL_RGBA
#define NO_BLIT_BUFFER_COPY
#define GL_DRAW_FRAMEBUFFER GL_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER GL_FRAMEBUFFER
#define GLESX
#ifdef PANDORA
typedef char GLchar;
#endif
#elif defined(GLES3)
#define GLESX
#define GL_UNIFORMBLOCK_SUPPORT
#elif defined(GLES3_1)
#define GLESX
#define GL_IMAGE_TEXTURES_SUPPORT
#define GL_MULTISAMPLING_SUPPORT
#define GL_UNIFORMBLOCK_SUPPORT
#elif defined(EGL)
#define GL_IMAGE_TEXTURES_SUPPORT
#define GL_MULTISAMPLING_SUPPORT
#define GL_UNIFORMBLOCK_SUPPORT
#else
#if defined(OS_MAC_OS_X)
#define GL_GLEXT_PROTOTYPES
#elif defined(OS_LINUX)
#define GL_GLEXT_PROTOTYPES
#define GL_IMAGE_TEXTURES_SUPPORT
#define GL_MULTISAMPLING_SUPPORT
#define GL_UNIFORMBLOCK_SUPPORT
#elif defined(OS_WINDOWS)
#define GL_IMAGE_TEXTURES_SUPPORT
#define GL_MULTISAMPLING_SUPPORT
#define GL_UNIFORMBLOCK_SUPPORT
#endif // OS_MAC_OS_X
#endif // GLES2

#ifdef __cplusplus
struct GLState {
	GLState() { reset(); }
	void reset();
};

extern GLState glState;
#endif

extern u32 bilinearMode;
extern u32 EnableHWLighting;
extern u32 CorrectTexrectCoords;
extern u32 enableNativeResTexrects;
extern u32 enableLegacyBlending;
extern u32 EnableFBEmulation;
extern u32 UseNativeResolutionFactor;
extern u32 EnableCopyAuxiliaryToRDRAM;
extern u32 EnableCopyColorToRDRAM;
extern u32 EnableCopyDepthToRDRAM;
extern u32 EnableCopyColorFromRDRAM;
extern u32 AspectRatio;
extern u32 txFilterMode;
extern u32 txEnhancementMode;
extern u32 txHiresEnable;
extern u32 MultiSampling;

#ifdef GLESX
#undef GL_NUM_EXTENSIONS
#endif

#endif
