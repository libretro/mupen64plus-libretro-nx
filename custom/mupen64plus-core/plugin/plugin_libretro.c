/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - plugin.c                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
 *   Copyright (C) 2009 Richard Goedeken                                   *
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

#include <stdio.h>
#include <stdlib.h>

#include "plugin.h"

#include "r4300/r4300_core.h"
#include "../rdp/rdp_core.h"
#include "../rsp/rsp_core.h"

#include "../vi/vi_controller.h"

#include "api/callbacks.h"
#include "api/m64p_common.h"
#include "api/m64p_plugin.h"
#include "api/m64p_types.h"

#include "main/main.h"
#include "main/rom.h"
#include "main/version.h"
#include "memory/memory.h"

static unsigned int dummy;

/* local functions */
static void EmptyFunc(void)
{
}

static m64p_error EmptyGetVersionFunc(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
   return M64ERR_SUCCESS;
}
/* local data structures and functions */
#define DEFINE_GFX(X) \
    EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type *, int *, int *, const char **, int *); \
    EXPORT void CALL ChangeWindow(void); \
    EXPORT int  CALL InitiateGFX(GFX_INFO Gfx_Info); \
    EXPORT void CALL MoveScreen(int x, int y); \
    EXPORT void CALL ProcessDList(void); \
    EXPORT void CALL ProcessRDPList(void); \
    EXPORT void CALL RomClosed(void); \
    EXPORT int  CALL RomOpen(void); \
    EXPORT void CALL ShowCFB(void); \
    EXPORT void CALL UpdateScreen(void); \
    EXPORT void CALL ViStatusChanged(void); \
    EXPORT void CALL ViWidthChanged(void); \
    EXPORT void CALL ReadScreen2(void *dest, int *width, int *height, int front); \
    EXPORT void CALL SetRenderingCallback(void (*callback)(int)); \
    EXPORT void CALL ResizeVideoOutput(int width, int height); \
    EXPORT void CALL FBRead(unsigned int addr); \
    EXPORT void CALL FBWrite(unsigned int addr, unsigned int size); \
    EXPORT void CALL FBGetFrameBufferInfo(void *p); \
    \
    static const gfx_plugin_functions gfx_##X = { \
        PluginGetVersion, \
        ChangeWindow, \
        InitiateGFX, \
        MoveScreen, \
        ProcessDList, \
        ProcessRDPList, \
        RomClosed, \
        RomOpen, \
        ShowCFB, \
        UpdateScreen, \
        ViStatusChanged, \
        ViWidthChanged, \
        ReadScreen2, \
        SetRenderingCallback, \
        FBRead, \
        FBWrite, \
        FBGetFrameBufferInfo \
    }

DEFINE_GFX(gln64);

gfx_plugin_functions gfx;
GFX_INFO gfx_info;

static m64p_error plugin_start_gfx(void)
{
   /* fill in the GFX_INFO data structure */
   gfx_info.HEADER = (unsigned char *) g_rom;
   gfx_info.RDRAM = (unsigned char *) g_rdram;
   gfx_info.DMEM = (unsigned char *) g_sp.mem;
   gfx_info.IMEM = (unsigned char *) g_sp.mem + 0x1000;
   gfx_info.MI_INTR_REG = &(g_r4300.mi.regs[MI_INTR_REG]);
   gfx_info.DPC_START_REG = &(g_dp.dpc_regs[DPC_START_REG]);
   gfx_info.DPC_END_REG = &(g_dp.dpc_regs[DPC_END_REG]);
   gfx_info.DPC_CURRENT_REG = &(g_dp.dpc_regs[DPC_CURRENT_REG]);
   gfx_info.DPC_STATUS_REG = &(g_dp.dpc_regs[DPC_STATUS_REG]);
   gfx_info.DPC_CLOCK_REG = &(g_dp.dpc_regs[DPC_CLOCK_REG]);
   gfx_info.DPC_BUFBUSY_REG = &(g_dp.dpc_regs[DPC_BUFBUSY_REG]);
   gfx_info.DPC_PIPEBUSY_REG = &(g_dp.dpc_regs[DPC_PIPEBUSY_REG]);
   gfx_info.DPC_TMEM_REG = &(g_dp.dpc_regs[DPC_TMEM_REG]);
   gfx_info.VI_STATUS_REG = &(g_vi.regs[VI_STATUS_REG]);
   gfx_info.VI_ORIGIN_REG = &(g_vi.regs[VI_ORIGIN_REG]);
   gfx_info.VI_WIDTH_REG = &(g_vi.regs[VI_WIDTH_REG]);
   gfx_info.VI_INTR_REG = &(g_vi.regs[VI_V_INTR_REG]);
   gfx_info.VI_V_CURRENT_LINE_REG = &(g_vi.regs[VI_CURRENT_REG]);
   gfx_info.VI_TIMING_REG = &(g_vi.regs[VI_BURST_REG]);
   gfx_info.VI_V_SYNC_REG = &(g_vi.regs[VI_V_SYNC_REG]);
   gfx_info.VI_H_SYNC_REG = &(g_vi.regs[VI_H_SYNC_REG]);
   gfx_info.VI_LEAP_REG = &(g_vi.regs[VI_LEAP_REG]);
   gfx_info.VI_H_START_REG = &(g_vi.regs[VI_H_START_REG]);
   gfx_info.VI_V_START_REG = &(g_vi.regs[VI_V_START_REG]);
   gfx_info.VI_V_BURST_REG = &(g_vi.regs[VI_V_BURST_REG]);
   gfx_info.VI_X_SCALE_REG = &(g_vi.regs[VI_X_SCALE_REG]);
   gfx_info.VI_Y_SCALE_REG = &(g_vi.regs[VI_Y_SCALE_REG]);
   gfx_info.CheckInterrupts = EmptyFunc;

   /* call the audio plugin */
   if (!gfx.initiateGFX(gfx_info))
   {
      printf("plugin_start_gfx fail.\n");
      return M64ERR_PLUGIN_FAIL;
   }

   printf("plugin_start_gfx success.\n");

   return M64ERR_SUCCESS;
}

/* INPUT */
extern m64p_error inputPluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion,
                                              int *APIVersion, const char **PluginNamePtr, int *Capabilities);
extern void inputInitiateControllers (CONTROL_INFO ControlInfo);
extern void inputControllerCommand(int Control, unsigned char *Command);
extern void inputInitiateControllers(CONTROL_INFO ControlInfo);
extern void inputReadController(int Control, unsigned char *Command);
extern int  inputRomOpen(void);
extern void inputRomClosed(void);

input_plugin_functions input = {
    inputPluginGetVersion,
    inputControllerCommand,
    NULL,
    inputInitiateControllers,
    inputReadController,
    inputRomClosed,
    inputRomOpen,
};

static CONTROL_INFO control_info;
CONTROL Controls[4];

static m64p_error plugin_start_input(void)
{
   int i;

   /* fill in the CONTROL_INFO data structure */
   control_info.Controls = Controls;
   for (i=0; i<4; i++)
   {
      Controls[i].Present = 0;
      Controls[i].RawData = 0;
      Controls[i].Plugin = PLUGIN_NONE;
   }

   /* call the input plugin */
   input.initiateControllers(control_info);

   return M64ERR_SUCCESS;
}

/* RSP */
#define DEFINE_RSP(X) \
    EXPORT m64p_error CALL X##PluginGetVersion(m64p_plugin_type *, int *, int *, const char **, int *); \
    EXPORT unsigned int CALL X##DoRspCycles(unsigned int Cycles); \
    EXPORT void CALL X##InitiateRSP(RSP_INFO Rsp_Info, unsigned int *CycleCount); \
    EXPORT void CALL X##RomClosed(void); \
    \
    static const rsp_plugin_functions rsp_##X = { \
        X##PluginGetVersion, \
        X##DoRspCycles, \
        X##InitiateRSP, \
        X##RomClosed \
    }

DEFINE_RSP(hle);
#ifndef VC
DEFINE_RSP(lle);
#endif

rsp_plugin_functions rsp;
RSP_INFO rsp_info;

static m64p_error plugin_start_rsp(void)
{
   /* fill in the RSP_INFO data structure */
   rsp_info.RDRAM = (unsigned char *) g_rdram;
   rsp_info.DMEM = (unsigned char *) g_sp.mem;
   rsp_info.IMEM = (unsigned char *) g_sp.mem + 0x1000;
   rsp_info.MI_INTR_REG = &g_r4300.mi.regs[MI_INTR_REG];
   rsp_info.SP_MEM_ADDR_REG = &g_sp.regs[SP_MEM_ADDR_REG];
   rsp_info.SP_DRAM_ADDR_REG = &g_sp.regs[SP_DRAM_ADDR_REG];
   rsp_info.SP_RD_LEN_REG = &g_sp.regs[SP_RD_LEN_REG];
   rsp_info.SP_WR_LEN_REG = &g_sp.regs[SP_WR_LEN_REG];
   rsp_info.SP_STATUS_REG = &g_sp.regs[SP_STATUS_REG];
   rsp_info.SP_DMA_FULL_REG = &g_sp.regs[SP_DMA_FULL_REG];
   rsp_info.SP_DMA_BUSY_REG = &g_sp.regs[SP_DMA_BUSY_REG];
   rsp_info.SP_PC_REG = &g_sp.regs2[SP_PC_REG];
   rsp_info.SP_SEMAPHORE_REG = &g_sp.regs[SP_SEMAPHORE_REG];
   rsp_info.DPC_START_REG = &g_dp.dpc_regs[DPC_START_REG];
   rsp_info.DPC_END_REG = &g_dp.dpc_regs[DPC_END_REG];
   rsp_info.DPC_CURRENT_REG = &g_dp.dpc_regs[DPC_CURRENT_REG];
   rsp_info.DPC_STATUS_REG = &g_dp.dpc_regs[DPC_STATUS_REG];
   rsp_info.DPC_CLOCK_REG = &g_dp.dpc_regs[DPC_CLOCK_REG];
   rsp_info.DPC_BUFBUSY_REG = &g_dp.dpc_regs[DPC_BUFBUSY_REG];
   rsp_info.DPC_PIPEBUSY_REG = &g_dp.dpc_regs[DPC_PIPEBUSY_REG];
   rsp_info.DPC_TMEM_REG = &g_dp.dpc_regs[DPC_TMEM_REG];
   rsp_info.CheckInterrupts = EmptyFunc;
   rsp_info.ProcessDlistList = gfx.processDList;
   rsp_info.ProcessAlistList = NULL;
   rsp_info.ProcessRdpList = gfx.processRDPList;
   rsp_info.ShowCFB = gfx.showCFB;

   /* call the RSP plugin  */
   rsp.initiateRSP(rsp_info, NULL);

   return M64ERR_SUCCESS;
}

extern int rspMode;

/* global functions */
void plugin_connect_all()
{
   gfx = gfx_gln64;
   if (rspMode == 0)
      rsp = rsp_hle;
#ifndef VC
   else
      rsp = rsp_lle;
#endif
   plugin_start_gfx();
   plugin_start_input();
   plugin_start_rsp();
}
