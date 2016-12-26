#include "GLideN64_mupenplus.h"
#include "GLideN64_libretro.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "../Config.h"
#include "../GLideN64.h"
#include "../OpenGL.h"
#include "../GBI.h"
#include "../RSP.h"
#include "../Log.h"
#include "main/util.h"

Config config;

void LoadCustomSettings()
{
	std::string myString = RSP.romname;
	for (size_t pos = myString.find(' '); pos != std::string::npos; pos = myString.find(' ', pos))
	{
		myString.replace(pos, 1, "%20");
	}
	for (size_t pos = myString.find('\''); pos != std::string::npos; pos = myString.find('\'', pos))
	{
		myString.replace(pos, 1, "%27");
	}
	FILE *fPtr;
	int lineno;
	bool found = false;
	char buffer[256];
	const char *pathname = ConfigGetSharedDataFilepath("GLideN64.custom.ini");
	if (pathname == NULL || (fPtr = fopen(pathname, "rb")) == NULL)
	{
		printf("Unable to open GLideN64 settings file '%s'.", pathname);
		return;
	}
	std::transform(myString.begin(), myString.end(), myString.begin(), ::toupper);
	for (lineno = 1; fgets(buffer, 255, fPtr) != NULL; lineno++)
	{
		char *line = buffer;
		ini_line l = ini_parse_line(&line);
		switch (l.type)
		{
			case INI_SECTION:
			{
				if (myString == l.name)
					found = true;
				else
					found = false;
			}
			case INI_PROPERTY:
			{
				if (found) {
					if (!strcmp(l.name, "frameBufferEmulation\\copyToRDRAM"))
						config.frameBufferEmulation.copyToRDRAM = atoi(l.value);
					else if (!strcmp(l.name, "frameBufferEmulation\\copyFromRDRAM"))
						config.frameBufferEmulation.copyFromRDRAM = atoi(l.value);
					else if (!strcmp(l.name, "frameBufferEmulation\\copyDepthToRDRAM"))
						config.frameBufferEmulation.copyDepthToRDRAM = atoi(l.value);
					else if (!strcmp(l.name, "frameBufferEmulation\\copyAuxToRDRAM"))
						config.frameBufferEmulation.copyAuxToRDRAM = atoi(l.value);
					else if (!strcmp(l.name, "frameBufferEmulation\\N64DepthCompare"))
						config.frameBufferEmulation.N64DepthCompare = atoi(l.value);
				}
			}
		}
	}
}

void Config_LoadConfig()
{
	u32 hacks = config.generalEmulation.hacks;
	config.resetToDefaults();
	config.frameBufferEmulation.aspect = AspectRatio;
	config.texture.bilinearMode = bilinearMode;
	config.generalEmulation.enableHWLighting = EnableHWLighting;
	config.generalEmulation.correctTexrectCoords = CorrectTexrectCoords;
	config.generalEmulation.enableNativeResTexrects = enableNativeResTexrects;
	config.generalEmulation.enableLegacyBlending = enableLegacyBlending;
	config.frameBufferEmulation.nativeResFactor = UseNativeResolutionFactor;
	config.frameBufferEmulation.copyDepthToRDRAM = EnableCopyDepthToRDRAM;
	config.frameBufferEmulation.copyFromRDRAM = EnableCopyColorFromRDRAM;
	config.frameBufferEmulation.copyAuxToRDRAM = EnableCopyAuxiliaryToRDRAM;
	config.frameBufferEmulation.copyToRDRAM = EnableCopyColorToRDRAM;
	config.frameBufferEmulation.bufferSwapMode = Config::bsOnColorImageChange;
#ifdef HAVE_OPENGLES2
	config.generalEmulation.enableFragmentDepthWrite = 0;
#else
	config.generalEmulation.enableFragmentDepthWrite = EnableFragmentDepthWrite;
#endif
	config.generalEmulation.enableShadersStorage = EnableShadersStorage;
	config.textureFilter.txFilterMode = txFilterMode;
	config.textureFilter.txEnhancementMode = txEnhancementMode;
	config.textureFilter.txFilterIgnoreBG = txFilterIgnoreBG;
	config.textureFilter.txHiresEnable = txHiresEnable;
	config.textureFilter.txHiresFullAlphaChannel = txHiresFullAlphaChannel;
	config.video.multisampling = MultiSampling;
	config.video.cropMode = Config::cmAuto;
	config.generalEmulation.hacks = hacks;
	LoadCustomSettings();
}
