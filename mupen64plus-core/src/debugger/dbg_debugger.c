/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - debugger.c                                              *
 *   Mupen64Plus homepage: https://mupen64plus.org/                        *
 *   Copyright (C) 2008 DarkJeztr                                          *
 *   Copyright (C) 2002 davFr                                              *
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

#ifdef __APPLE__
#include <stdlib.h>
#endif

#include <semaphore.h>
#include "api/callbacks.h"
#include "api/debugger.h"
#include "dbg_breakpoints.h"
#include "dbg_debugger.h"
#include "dbg_memory.h"

#ifdef DBG

int g_DebuggerActive = 0;    // whether the debugger is enabled or not

m64p_dbg_runstate g_dbg_runstate;

// Holds the number of pending steps the debugger needs to perform.
static sem_t *sem_pending_steps;

uint32_t previousPC;

uint32_t breakpointAccessed;
uint32_t breakpointFlag;

//]=-=-=-=-=-=-=-=-=-=-=[ Initialisation du Debugger ]=-=-=-=-=-=-=-=-=-=-=-=[

void init_debugger()
{
    if (!DebuggerCallbacksAreSet())
    {
        DebugMessage(M64MSG_WARNING, "Front-end debugger callbacks are not set, so debugger will remain disabled.");
        return;
    }
    
    g_DebuggerActive = 1;
    g_dbg_runstate = M64P_DBG_RUNSTATE_PAUSED;

    DebuggerCallback(DEBUG_UI_INIT, 0); /* call front-end to initialize user interface */

    init_host_disassembler();

    sem_pending_steps = (sem_t*)malloc(sizeof(sem_t));
    sem_init(sem_pending_steps, 0, 0);
}

void destroy_debugger()
{
    sem_destroy(sem_pending_steps);
    free(sem_pending_steps);
    sem_pending_steps = NULL;
    g_DebuggerActive = 0;
}

//]=-=-=-=-=-=-=-=-=-=-=-=-=[ Mise-a-Jour Debugger ]=-=-=-=-=-=-=-=-=-=-=-=-=[

void update_debugger(uint32_t pc)
// Update debugger state and display.
// Should be called after each R4300 instruction
// Checks for breakpoint hits on PC
{
    int bpt;

    if (g_dbg_runstate != M64P_DBG_RUNSTATE_PAUSED) {
        bpt = check_breakpoints(pc);
        if (bpt != -1) {
            g_dbg_runstate = M64P_DBG_RUNSTATE_PAUSED;

            breakpointAccessed = 0;
            breakpointFlag = M64P_BKP_FLAG_EXEC;
            if (BPT_CHECK_FLAG(g_Breakpoints[bpt], M64P_BKP_FLAG_LOG))
                log_breakpoint(pc, M64P_BKP_FLAG_EXEC, 0);
        }
    }

    if (g_dbg_runstate != M64P_DBG_RUNSTATE_RUNNING) {
        DebuggerCallback(DEBUG_UI_UPDATE, pc);  /* call front-end to notify user interface to update */
    }
    if (g_dbg_runstate == M64P_DBG_RUNSTATE_PAUSED) {
        // The emulation thread is blocked until a step call via the API.
        sem_wait(sem_pending_steps);
    }

    previousPC = pc;
}

void debugger_step()
{
    sem_post(sem_pending_steps);
}

#endif
