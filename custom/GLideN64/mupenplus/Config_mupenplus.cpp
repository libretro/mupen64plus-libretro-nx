#include "GLideN64_mupenplus.h"
#include "GLideN64_libretro.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../Config.h"
#include "../GLideN64.h"
#include "../OpenGL.h"
#include "../GBI.h"
#include "../RSP.h"
#include "../Log.h"
extern "C" {
#include <sys/stat.h>
#include "osal_files.h"
#include <dirent.h>
}

Config config;

void Config_LoadConfig()
{
	u32 hacks = config.generalEmulation.hacks;
	config.resetToDefaults();
	config.frameBufferEmulation.aspect = AspectRatio;
	config.frameBufferEmulation.enable = EnableFBEmulation;
	config.texture.bilinearMode = bilinearMode;
	config.generalEmulation.enableNoise = EnableNoise;
	config.generalEmulation.enableLOD = EnableLOD;
	config.generalEmulation.enableHWLighting = EnableHWLighting;
	config.generalEmulation.correctTexrectCoords = CorrectTexrectCoords;
	config.generalEmulation.enableNativeResTexrects = enableNativeResTexrects;
	config.generalEmulation.enableLegacyBlending = enableLegacyBlending;
	config.frameBufferEmulation.nativeResFactor = UseNativeResolutionFactor;
	config.frameBufferEmulation.copyDepthToRDRAM = EnableCopyDepthToRDRAM;
	config.frameBufferEmulation.copyFromRDRAM = EnableCopyColorFromRDRAM;
	config.frameBufferEmulation.copyAuxToRDRAM = EnableCopyAuxiliaryToRDRAM;
	config.frameBufferEmulation.copyToRDRAM = EnableCopyColorToRDRAM;
#ifdef HAVE_OPENGLES
	config.frameBufferEmulation.bufferSwapMode = 2;
#endif
#ifndef GLES2
	config.generalEmulation.enableFragmentDepthWrite = EnableFragmentDepthWrite;
#else
	config.generalEmulation.enableFragmentDepthWrite = 0;
#endif
#ifdef ANDROID
	config.generalEmulation.forcePolygonOffset = 1;
	config.generalEmulation.polygonOffsetFactor = PolygonOffsetFactor;
	config.generalEmulation.polygonOffsetUnits = PolygonOffsetFactor;
#endif
	config.generalEmulation.hacks = hacks;
}

EXPORT int CALL osal_path_existsW(const wchar_t *_path)
{
    char path[PATH_MAX];
    wcstombs(path, _path, PATH_MAX);
    struct stat fileinfo;
    return stat(path, &fileinfo) == 0 ? 1 : 0;
}

EXPORT int CALL osal_is_directory(const wchar_t * _name)
{
    char name[PATH_MAX + 1];
    wcstombs(name, _name, PATH_MAX);
    DIR* dir;
    dir = opendir(name);
    if(dir != NULL)
    {
        closedir(dir);
        return 1;
    }
    return 0;
}
