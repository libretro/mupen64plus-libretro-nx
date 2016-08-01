#include "GLideN64_mupenplus.h"
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
}
