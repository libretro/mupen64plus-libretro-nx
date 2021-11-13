/* mupen64plus-input-raphnetraw
 *
 * Copyright (C) 2016 Raphael Assenat
 *
 * An input plugin that lets the game under emulation communicate with
 * the controllers directly using the direct controller communication
 * feature of raphnet V3 adapters[1].
 *
 * [1] http://www.raphnet.net/electronique/gcn64_usb_adapter_gen3/index_en.php
 *
 * Based on the Mupen64plus-input-sdl plugin (original banner below)
 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-input-sdl - plugin.c                                      *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008-2011 Richard Goedeken                              *
 *   Copyright (C) 2008 Tillin9                                            *
 *   Copyright (C) 2002 Blight                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define M64P_PLUGIN_PROTOTYPES 1
#include "config.h"
#include "m64p_common.h"
#include "m64p_config.h"
#include "m64p_plugin.h"
#include "m64p_types.h"
#include "osal_dynamiclib.h"
#include "plugin_front.h"
#include "plugin_back.h"
#include "version.h"

#ifdef __LIBRETRO__
#include <libretro_private.h>
#endif /* __LIBRETRO__ */

#ifdef __linux__
#endif /* __linux__ */

#include <errno.h>

#define MAX_CONTROLLERS	4

#ifdef __LIBRETRO__
static int emu2adap_portmap[MAX_CONTROLLERS] = { -1, -1, -1, -1 };
#undef PLUGIN_NAME
#define PLUGIN_NAME "raphnetraw libretro"

/* global data definitions */
struct
{
    CONTROL *control;               // pointer to CONTROL struct in Core library
    BUTTONS buttons;
} controller[4];

#elif PORTS_1_AND_4
static int emu2adap_portmap[MAX_CONTROLLERS] = { 0, 2, 3, 1 };
#undef PLUGIN_NAME
#define PLUGIN_NAME "raphnetraw ports 1 and 4"
#else
static int emu2adap_portmap[MAX_CONTROLLERS] = { 0, 1, 2, 3 };
#endif

#define EMU_2_ADAP_PORT(a)	((a) == -1 ? -1 : emu2adap_portmap[a])

/* static data definitions */
static void (*l_DebugCallback)(void *, int, const char *) = NULL;
static void *l_DebugCallContext = NULL;
static int l_PluginInit = 0;

/* Global functions */
static void DebugMessage(int level, const char *message, ...)
{
	char msgbuf[1024];
	va_list args;

	if (!l_DebugCallback) {
#ifdef __LIBRETRO__
		va_start(args, message);
		vsnprintf(msgbuf, sizeof(msgbuf), message, args);
		printf("%s\n", msgbuf);
		va_end(args);
#else
		printf("No debug callback!\n");
#endif
		return;
	}

	va_start(args, message);
	vsnprintf(msgbuf, sizeof(msgbuf), message, args);

	(*l_DebugCallback)(l_DebugCallContext, level, msgbuf);

	va_end(args);
}


/* Libretro related functions */
#ifdef __LIBRETRO__
void raphnetUpdatePortMap(int in_port, int device)
{
    int is_plugged = (int) (controller[in_port].control ? controller[in_port].control->Present : 0);
    int is_raphnet = (int) (device == RETRO_DEVICE_RAPHNET);

    if (pb_updatePortMap(in_port, is_plugged, is_raphnet) > 0)
        pb_scanControllers();
}
#endif


/* Mupen64Plus plugin functions */
EXPORT m64p_error CALL raphnetPluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,
                                   void (*DebugCallback)(void *, int, const char *))
{
    ptr_CoreGetAPIVersions CoreAPIVersionFunc;

    int ConfigAPIVersion, DebugAPIVersion, VidextAPIVersion;

    if (l_PluginInit) {
    	return M64ERR_ALREADY_INIT;
	}

	l_DebugCallback = DebugCallback;
	l_DebugCallContext = Context;

    /* attach and call the CoreGetAPIVersions function, check Config API version for compatibility */
    CoreAPIVersionFunc = (ptr_CoreGetAPIVersions) osal_dynlib_getproc(CoreLibHandle, "CoreGetAPIVersions");
    if (CoreAPIVersionFunc == NULL)
    {
        DebugMessage(M64MSG_ERROR, "Core emulator broken; no CoreAPIVersionFunc() function found.");
        return M64ERR_INCOMPATIBLE;
    }

    (*CoreAPIVersionFunc)(&ConfigAPIVersion, &DebugAPIVersion, &VidextAPIVersion, NULL);
    if ((ConfigAPIVersion & 0xffff0000) != (CONFIG_API_VERSION & 0xffff0000) || ConfigAPIVersion < CONFIG_API_VERSION)
    {
        DebugMessage(M64MSG_ERROR, "Emulator core Config API (v%i.%i.%i) incompatible with plugin (v%i.%i.%i)",
                VERSION_PRINTF_SPLIT(ConfigAPIVersion), VERSION_PRINTF_SPLIT(CONFIG_API_VERSION));
        return M64ERR_INCOMPATIBLE;
    }

	pb_init(DebugMessage);

    l_PluginInit = 1;
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL raphnetPluginShutdown(void)
{
    if (!l_PluginInit) {
		return M64ERR_NOT_INIT;
	}

	/* reset some local variables */
	l_DebugCallback = NULL;
	l_DebugCallContext = NULL;

	pb_shutdown();

    l_PluginInit = 0;

    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL raphnetPluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    /* set version info */
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_INPUT;

    if (PluginVersion != NULL)
        *PluginVersion = PLUGIN_VERSION;

    if (APIVersion != NULL)
        *APIVersion = INPUT_PLUGIN_API_VERSION;

    if (PluginNamePtr != NULL)
        *PluginNamePtr = PLUGIN_NAME;

    if (Capabilities != NULL)
    {
        *Capabilities = 0;
    }

    return M64ERR_SUCCESS;
}

/******************************************************************
  Function: InitiateControllers
  Purpose:  This function initialises how each of the controllers
            should be handled.
  input:    - The handle to the main window.
            - A controller structure that needs to be filled for
              the emulator to know how to handle each controller.
  output:   none
*******************************************************************/
EXPORT void CALL raphnetInitiateControllers(CONTROL_INFO ControlInfo)
{
#ifdef __LIBRETRO__
    // we need the built-in plugin for GetKeys, this also correctly
    // sets ControlInfo to what the user selected in libretro
    inputInitiateControllers(ControlInfo);
    pb_init(DebugMessage);
#else
    int i, n_controllers, adap_port;

	n_controllers = pb_scanControllers();

	if (n_controllers <= 0) {
    	DebugMessage(PB_MSG_ERROR, "No adapters detected\n");
		return;
	}

	for (i=0; i<MAX_CONTROLLERS; i++) {
		adap_port = EMU_2_ADAP_PORT(i);

		if (adap_port < n_controllers) {
			ControlInfo.Controls[i].RawData = 1;

			/* Setting this is currently required or we
			 * won't be called at all.
			 *
			 * Look at pif.c update_pif_write() to see why.
			 */
			ControlInfo.Controls[i].Present = 1;
		}
	}
#endif

    DebugMessage(PB_MSG_INFO, "%s version %i.%i.%i %s(compiled "__DATE__" "__TIME__") initialized.", PLUGIN_NAME, VERSION_PRINTF_SPLIT(PLUGIN_VERSION),
#ifdef _DEBUG
	"DEBUG "
#else
	""
#endif
	);
}


/******************************************************************
  Function: ReadController
  Purpose:  To process the raw data in the pif ram that is about to
            be read.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none
  note:     This function is only needed if the DLL is allowing raw
            data.
*******************************************************************/
EXPORT void CALL raphnetReadController(int Control, unsigned char *Command)
{
    pb_readController(Control, Command);
}

EXPORT void CALL raphnetControllerCommand(int Control, unsigned char *Command)
{
	pb_controllerCommand(Control, Command);
}

EXPORT void CALL raphnetGetKeys_default( int Control, BUTTONS *Keys )
{
#ifdef __LIBRETRO__
    // pass GetKeys requests back to the built-in plugin
    inputGetKeys_default(Control, Keys);
#endif
}

EXPORT void CALL raphnetRomClosed(void)
{
	pb_romClosed();
}

EXPORT int CALL raphnetRomOpen(void)
{
    pb_romOpen();
	return 1;
}

/******************************************************************
  Function: raphnetSDL_KeyDown
  Purpose:  To pass the SDL_KeyDown message from the emulator to the
            plugin.
  input:    keymod and keysym of the SDL_KEYDOWN message.
  output:   none
*******************************************************************/
EXPORT void CALL raphnetSDL_KeyDown(int keymod, int keysym)
{
}

/******************************************************************
  Function: SDL_KeyUp
  Purpose:  To pass the SDL_KeyUp message from the emulator to the
            plugin.
  input:    keymod and keysym of the SDL_KEYUP message.
  output:   none
*******************************************************************/
EXPORT void CALL raphnetSDL_KeyUp(int keymod, int keysym)
{
}
