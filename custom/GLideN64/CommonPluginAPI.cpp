#ifdef OS_WINDOWS
# include <windows.h>
#else
# include "winlnxdefs.h"
#endif // OS_WINDOWS

#include "PluginAPI.h"

#include "RSP.h"
#include "GLideN64.h"
extern u32 FrameSkip;
extern "C" {

int skip;
int render;
EXPORT BOOL CALL gln64InitiateGFX (GFX_INFO Gfx_Info)
{
	skip = 0;
	render = 1;
	return api().InitiateGFX(Gfx_Info);
}

EXPORT void CALL gln64MoveScreen (int xpos, int ypos)
{
	api().MoveScreen(xpos, ypos);
}

EXPORT void CALL gln64ProcessDList(void)
{
	if (skip && FrameSkip) {
		*REG.MI_INTR |= MI_INTR_DP;
		CheckInterrupts();
		skip = 0;
	} else {
		api().ProcessDList();
		skip = 1;
		render = 1;
	}
}

EXPORT void CALL gln64ProcessRDPList(void)
{
	api().ProcessRDPList();
}

EXPORT void CALL gln64RomClosed (void)
{
	api().RomClosed();
}

EXPORT void CALL gln64ShowCFB (void)
{
	api().ShowCFB();
}

EXPORT void CALL gln64UpdateScreen (void)
{
	if (render == 1) {
		api().UpdateScreen();
		if (FrameSkip)
			render = 0;
	}
}

EXPORT void CALL gln64ViStatusChanged (void)
{
	api().ViStatusChanged();
}

EXPORT void CALL gln64ViWidthChanged (void)
{
	api().ViWidthChanged();
}

EXPORT void CALL gln64ChangeWindow(void)
{
	api().ChangeWindow();
}

EXPORT void CALL gln64FBWrite(unsigned int addr, unsigned int size)
{
	api().FBWrite(addr, size);
}

EXPORT void CALL gln64FBRead(unsigned int addr)
{
	api().FBRead(addr);
}

EXPORT void CALL gln64FBGetFrameBufferInfo(void *pinfo)
{
	api().FBGetFrameBufferInfo(pinfo);
}

#ifndef MUPENPLUSAPI
EXPORT void CALL gln64FBWList(FrameBufferModifyEntry *plist, unsigned int size)
{
	api().FBWList(plist, size);
}
#endif
}
