/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - main.c                                                  *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 CasualJames                                        *
 *   Copyright (C) 2008-2009 Richard Goedeken                              *
 *   Copyright (C) 2008 Ebenblues Nmn Okaygo Tillin9                       *
 *   Copyright (C) 2002 Hacktarux                                          *
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

/* This is MUPEN64's main entry point. It contains code that is common
 * to both the gui and non-gui versions of mupen64. See
 * gui subdirectories for the gui-specific code.
 * if you want to implement an interface, you should look here
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M64P_CORE_PROTOTYPES 1
#include "api/callbacks.h"
#include "api/config.h"
#include "api/debugger.h"
#include "api/m64p_config.h"
#include "api/m64p_types.h"
#include "api/m64p_vidext.h"
#include "api/vidext.h"
#include "backends/audio_out_backend.h"
#include "backends/clock_backend.h"
#include "backends/controller_input_backend.h"
#include "backends/rumble_backend.h"
#include "backends/storage_backend.h"
#include "main/cheat.h"
#include "main/device.h"
#include "main/eventloop.h"
#include "main/main.h"
#include "osal/files.h"
#include "osal/preproc.h"
#include "osd/osd.h"
#include "osd/screenshot.h"
#include "plugin/emulate_game_controller_via_input_plugin.h"
#include "plugin/emulate_speaker_via_audio_plugin.h"
#include "plugin/get_time_using_C_localtime.h"
#include "plugin/plugin.h"
#include "plugin/rumble_via_input_plugin.h"
#include "main/profile.h"
#include "r4300/r4300.h"
#include "r4300/reset.h"
#include "main/rom.h"
#include "main/savestates.h"
#include "main/storage_file.h"
#include "main/util.h"
#include "r4300/new_dynarec/new_dynarec.h"

#include <libretro.h>
#include <libretro_private.h>
#include <libretro_memory.h>

extern retro_input_poll_t poll_cb;
extern uint32_t CountPerOp;

/* version number for Core config section */
#define CONFIG_PARAM_VERSION 1.01

/** globals **/
m64p_handle g_CoreConfig = NULL;

m64p_frame_callback g_FrameCallback = NULL;

int         g_MemHasBeenBSwapped = 0;   // store byte-swapped flag so we don't swap twice when re-playing game
int         g_EmulatorRunning = 0;      // need separate boolean to tell if emulator is running, since --nogui doesn't use a thread

/* XXX: only global because of new dynarec linkage_x86.asm and plugin.c */
ALIGN(16, uint32_t g_rdram[RDRAM_MAX_SIZE/4]);
struct device g_dev;

int g_delay_si = 0;

int g_gs_vi_counter = 0;

/** static (local) variables **/
static int   l_CurrentFrame = 0;         // frame counter
static int   l_TakeScreenshot = 0;       // Tell OSD Rendering callback to take a screenshot just before drawing the OSD
static int   l_SpeedFactor = 100;        // percentage of nominal game speed at which emulator is running
static int   l_FrameAdvance = 0;         // variable to check if we pause on next frame
static int   l_MainSpeedLimit = 1;       // insert delay during vi_interrupt to keep speed at real-time

static osd_message_t *l_msgVol = NULL;
static osd_message_t *l_msgFF = NULL;
static osd_message_t *l_msgPause = NULL;

/*********************************************************************************************************
* helper functions
*/


const char *get_savestatepath(void)
{
    /* try to get the SaveStatePath string variable in the Core configuration section */
    return "";
}

void main_message(m64p_msg_level level, unsigned int corner, const char *format, ...)
{
    va_list ap;
    char buffer[2049];
    va_start(ap, format);
    vsnprintf(buffer, 2047, format, ap);
    buffer[2048]='\0';
    va_end(ap);

    DebugMessage(level, "%s", buffer);
}

void main_check_inputs(void)
{
    poll_cb();
}

/*********************************************************************************************************
* global functions, for adjusting the core emulator behavior
*/

m64p_error main_core_state_query(m64p_core_param param, int *rval)
{
    switch (param)
    {
        case M64CORE_EMU_STATE:
            if (!g_EmulatorRunning)
                *rval = M64EMU_STOPPED;
            else if (rompause)
                *rval = M64EMU_PAUSED;
            else
                *rval = M64EMU_RUNNING;
            break;
        default:
            return M64ERR_INPUT_INVALID;
    }

    return M64ERR_SUCCESS;
}

m64p_error main_core_state_set(m64p_core_param param, int val)
{
    switch (param)
    {
        case M64CORE_EMU_STATE:
            if (!g_EmulatorRunning)
                return M64ERR_INVALID_STATE;
            if (val == M64EMU_STOPPED)
            {
                /* this stop function is asynchronous.  The emulator may not terminate until later */
                main_stop();
                return M64ERR_SUCCESS;
            }
            else if (val == M64EMU_RUNNING)
            {
                return M64ERR_SUCCESS;
            }
            return M64ERR_INPUT_INVALID;
        default:
            return M64ERR_INPUT_INVALID;
    }
}

m64p_error main_reset(int do_hard_reset)
{
    if (do_hard_reset)
        reset_hard_job |= 1;
    else
        reset_soft();
    return M64ERR_SUCCESS;
}

/*********************************************************************************************************
* global functions, callbacks from the r4300 core or from other plugins
*/

void new_frame(void)
{
    if (g_FrameCallback != NULL)
        (*g_FrameCallback)(l_CurrentFrame);

    /* advance the current frame */
    l_CurrentFrame++;

    if (l_FrameAdvance) {
        rompause = 1;
        l_FrameAdvance = 0;
        StateChanged(M64CORE_EMU_STATE, M64EMU_PAUSED);
    }
}

/* TODO: make a GameShark module and move that there */
static void gs_apply_cheats(void)
{
    if(g_gs_vi_counter < 60)
    {
        if (g_gs_vi_counter == 0)
            cheat_apply_cheats(ENTRY_BOOT);
        g_gs_vi_counter++;
    }
    else
    {
        cheat_apply_cheats(ENTRY_VI);
    }
}

/* called on vertical interrupt.
 * Allow the core to perform various things */
void new_vi(void)
{
    gs_apply_cheats();

    main_check_inputs();

    retro_return();
}

static void open_mpk_file(struct storage_file* storage)
{
    storage->data = saved_memory.mempack;
}

static void open_fla_file(struct storage_file* storage)
{
    storage->data = saved_memory.flashram;
}

static void open_sra_file(struct storage_file* storage)
{
    storage->data = saved_memory.sram;
}

static void open_eep_file(struct storage_file* storage)
{
    storage->data = saved_memory.eeprom;
}

/*********************************************************************************************************
* emulation thread - runs the core
*/
struct storage_file eep;
struct storage_file fla;
struct storage_file mpk;
struct storage_file sra;
void save_storage_file_libretro(void* opaque)
{
}

m64p_error main_run(void)
{
    size_t i;
    unsigned int disable_extra_mem;
    int channels[GAME_CONTROLLERS_COUNT];
    struct audio_out_backend aout;
    struct clock_backend rtc;
    struct controller_input_backend cins[GAME_CONTROLLERS_COUNT];
    struct rumble_backend rumbles[GAME_CONTROLLERS_COUNT];
    struct storage_backend fla_storage;
    struct storage_backend sra_storage;
    struct storage_backend mpk_storages[GAME_CONTROLLERS_COUNT];
    uint8_t* mpk_data[GAME_CONTROLLERS_COUNT];
    struct storage_backend eep_storage;


    no_compiled_jump = 0;
#ifdef NEW_DYNAREC
    stop_after_jal = 1;
#endif
    g_delay_si = 1;
    disable_extra_mem = 0;
    count_per_op = CountPerOp;
    if (count_per_op <= 0)
        count_per_op = ROM_PARAMS.countperop;
    cheat_add_hacks();

    /* do byte-swapping if it's not been done yet */
    if (g_MemHasBeenBSwapped == 0)
    {
        swap_buffer(g_rom, 4, g_rom_size/4);
        g_MemHasBeenBSwapped = 1;
    }

    /* open storage files, provide default content if not present */
    open_mpk_file(&mpk);
    open_eep_file(&eep);
    open_fla_file(&fla);
    open_sra_file(&sra);

    /* setup backends */
    aout = (struct audio_out_backend){ &g_dev.ai, set_audio_format_via_libretro, push_audio_samples_via_libretro };
    rtc = (struct clock_backend){ NULL, get_time_using_C_localtime };
    fla_storage = (struct storage_backend){ &fla, save_storage_file_libretro };
    sra_storage = (struct storage_backend){ &sra, save_storage_file_libretro };
    eep_storage = (struct storage_backend){ &eep, save_storage_file_libretro };

    /* setup game controllers data */
    for(i = 0; i < GAME_CONTROLLERS_COUNT; ++i) {
        channels[i] = i;
        cins[i] = (struct controller_input_backend){ &channels[i], egcvip_is_connected, egcvip_get_input };
        mpk_storages[i] = (struct storage_backend){ &mpk, save_storage_file_libretro };
        mpk_data[i] = storage_file_ptr(&mpk, i * MEMPAK_SIZE);
        rumbles[i] = (struct rumble_backend){ &channels[i], rvip_rumble };
    }

    init_device(&g_dev,
                &aout,
                g_rom, g_rom_size,
                storage_file_ptr(&fla, 0), &fla_storage,
                storage_file_ptr(&sra, 0), &sra_storage,
                g_rdram, (disable_extra_mem == 0) ? 0x800000 : 0x400000,
                cins,
                mpk_data, mpk_storages,
                rumbles,
                storage_file_ptr(&eep, 0), (ROM_SETTINGS.savetype != EEPROM_16KB) ? 0x200 : 0x800, (ROM_SETTINGS.savetype != EEPROM_16KB) ? 0x8000 : 0xc000, &eep_storage,
                &rtc,
                vi_clock_from_tv_standard(ROM_PARAMS.systemtype), vi_expected_refresh_rate_from_tv_standard(ROM_PARAMS.systemtype), g_count_per_scanline, g_alternate_vi_timing);

    // Attach rom to plugins
    if (!gfx.romOpen())
    {
        return M64ERR_PLUGIN_FAIL;
    }
    if (!input.romOpen())
    {
        gfx.romClosed(); return M64ERR_PLUGIN_FAIL;
    }

    g_EmulatorRunning = 1;
    StateChanged(M64CORE_EMU_STATE, M64EMU_RUNNING);

    /* call r4300 CPU core and run the game */
    poweron_device(&g_dev);

    r4300_reset_soft();
    r4300_execute();

    return M64ERR_SUCCESS;
}

void main_stop(void)
{
    /* note: this operation is asynchronous.  It may be called from a thread other than the
       main emulator thread, and may return before the emulator is completely stopped */
    if (!g_EmulatorRunning)
        return;

    DebugMessage(M64MSG_STATUS, "Stopping emulation.");
    stop = 1;

    rsp.romClosed();
    input.romClosed();
    gfx.romClosed();

    // clean up
    g_EmulatorRunning = 0;
    StateChanged(M64CORE_EMU_STATE, M64EMU_STOPPED);
}
