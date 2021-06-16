#pragma once
#include <memory>

#if defined(IOS) || defined(__APPLE__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif // IOS || __APPLE__

struct CachedTexture;

class PaletteTexture
{
public:
	PaletteTexture();

	void init();
	void destroy();
	void update();

private:
	CachedTexture * m_pTexture;
	u8* m_pbuf;
	u64 m_paletteCRC256;
};

extern PaletteTexture g_paletteTexture;
