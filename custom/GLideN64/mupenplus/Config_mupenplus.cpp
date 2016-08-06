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
	config.resetToDefaults();
	config.frameBufferEmulation.enable = 0;
	config.texture.bilinearMode = bilinearMode;
	config.generalEmulation.enableNoise = EnableNoise;
	config.generalEmulation.enableLOD = EnableLOD;
	config.generalEmulation.enableHWLighting = EnableHWLighting;
	config.generalEmulation.correctTexrectCoords = CorrectTexrectCoords;
	config.generalEmulation.enableNativeResTexrects = enableNativeResTexrects;
	config.generalEmulation.enableLegacyBlending = enableLegacyBlending;
}
