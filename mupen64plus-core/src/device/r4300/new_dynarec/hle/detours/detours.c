/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-Next - detours.c                                          *
 *   Copyright (C) 2022 M4xw <m4x@m4xw.net>                                *
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
 *   auint32_t with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

//#include <Windows.h>
//#define NEW_DYNAREC NEW_DYNAREC_X64

#define HIREG 32 // hi
#define LOREG 33 // lo

// HLE Detour Backend for New Dynarec
#include "detours.h"
#include <mupen64plus-next_common.h>

#include <device/r4300/r4300_core.h>
#include <device/r4300/new_dynarec/new_dynarec.h>
#include <api/callbacks.h>
#include <main/main.h>
#include <device/memory/memory.h>
#include <device/r4300/cached_interp.h>
#include <device/r4300/cp0.h>
#include <device/r4300/cp1.h>
#include <device/r4300/interrupt.h>
#include <device/r4300/tlb.h>
#include <device/r4300/fpu.h>
#include <device/rcp/mi/mi_controller.h>
#include <device/rcp/rsp/rsp_core.h>

// From new_dynarec.c
#define UPDATE_COUNT_IN \
  struct r4300_core* r4300 = &g_dev.r4300; \
  struct new_dynarec_hot_state* state = &r4300->new_dynarec_hot_state; \
  state->cycle_count += count; \
  state->pending_exception = 0;

extern u_char *out;
extern void add_to_linker(intptr_t addr, u_int target, int ext);

// From assem files
extern void emit_jmp(intptr_t a);
extern void emit_call(intptr_t a);
extern void emit_loadreg(int r, int hr);
extern void emit_storereg(int r, int hr);
extern void emit_addimm64(int rs,int imm,int rt);
extern void emit_pushreg(u_int r);
extern void emit_popreg(u_int r);

// WIP Stuff
#define DETOUR_LINK_LR add_to_linker((intptr_t)out, start + i * 4 + 8, 0), emit_jmp(0)

static void ultra_bcopy(uint32_t src, uint32_t dst, int length)
{
    // printf("ultra_bcopy: %x -> %x (%d)\n",(intptr_t)src,(intptr_t)dst,length);
    // fflush(stdout);

    char *mem_src = fast_mem_access(&g_dev.r4300, src);
    char *mem_dst = fast_mem_access(&g_dev.r4300, dst);
    memcpy(mem_dst, mem_src, length);
}

static void ultra_bzero(uint32_t dst, int length)
{
    // printf("ultra_bzero: %x (%d)\n",(intptr_t)dst,length);
    // fflush(stdout);

    char *mem_dst = fast_mem_access(&g_dev.r4300, dst);
    memset(mem_dst, 0, length);
}

static void detour_stub(void)
{
    // Stub
}

static void SqrtF(void)
{
    float root = *g_dev.r4300.new_dynarec_hot_state.cp1_regs_simple[12];
    *g_dev.r4300.new_dynarec_hot_state.cp1_regs_simple[0] = sqrtf(root);
}

static void CosF(void)
{
    float input = *g_dev.r4300.new_dynarec_hot_state.cp1_regs_simple[12];
    *g_dev.r4300.new_dynarec_hot_state.cp1_regs_simple[0] = cosf(input);
}

static void SinF(void)
{
    float input = *g_dev.r4300.new_dynarec_hot_state.cp1_regs_simple[12];
    *g_dev.r4300.new_dynarec_hot_state.cp1_regs_simple[0] = sinf(input);
}

static void osVirtualToPhysical(void *addr)
{
    uint32_t ret = -1;

    if ((uintptr_t)addr >= 0x80000000 && (uintptr_t)addr < 0xa0000000)
    {
        ret = ((uintptr_t)addr & 0x1fffffff);
    }
    else if ((uintptr_t)addr >= 0xa0000000 && (uintptr_t)addr < 0xc0000000)
    {
        ret = ((uintptr_t)addr & 0x1fffffff);
    }
    else if (((uintptr_t)addr & UINT32_C(0xc0000000)) != UINT32_C(0x80000000))
    {
        ret = virtual_to_physical_address(&g_dev.r4300, addr, 0);
    }

    g_dev.r4300.new_dynarec_hot_state.regs[2] = (int64_t)ret;
}

typedef struct
{
    /* 0x0 */ uint64_t quot;
    /* 0x8 */ uint64_t rem;
} detour_lldiv_t;

static void detour__ll_div(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    detour_lldiv_t ret_out;
    //printf("detour__ll_div: %p %p %p %p\n", a0, a1, a2, a3);
    //fflush(stdout);
    
    int64_t numer = (int64_t)((uint64_t)a0 << 32 | (uint64_t)a1);
    int64_t denom = (int64_t)((uint64_t)a2 << 32 | (uint64_t)a3);
    
    //printf("numer %p denom %p\n", numer, denom);
    //fflush(stdout);

    int64_t result = numer / denom;

    g_dev.r4300.new_dynarec_hot_state.regs[2] = (int64_t)(int32_t)(result >> 32);
    g_dev.r4300.new_dynarec_hot_state.regs[3] = (int64_t)(int32_t)(result & 0xffffffff);

    return;
}

// UJUMP Assemble Hook
int ujump_detour(int i, struct regstat *i_regs, u_int *ba, u_int start)
{
    // Quake2 hook for bcopy
    if (ba[i] == 0x8005AD60)
    {
        // Call to our hook
        emit_loadreg(4, ARG1_REG);
        emit_loadreg(5, ARG2_REG);
        emit_loadreg(6, ARG3_REG);
        emit_call((intptr_t)ultra_bcopy);
        // Link back to Return addr
        DETOUR_LINK_LR;

        return DETOUR_SUCCESS;
    }

    // Quake2 hook for bzero
    if (ba[i] == 0x8005B080)
    {
        // Call to our hook
        emit_loadreg(4, ARG1_REG);
        emit_loadreg(5, ARG2_REG);
        emit_call((intptr_t)ultra_bzero);
        // Link back to Return addr
        DETOUR_LINK_LR;

        return DETOUR_SUCCESS;
    }

    // Quake2 hook for osVirtualToPhysical
    if (ba[i] == 0x80052360)
    {
        // Call to our hook
        emit_loadreg(4, ARG1_REG);
        emit_call((intptr_t)osVirtualToPhysical);
        // Link back to Return addr
        DETOUR_LINK_LR;

        return DETOUR_SUCCESS;
    }

    // Quake2 hook for detour_stub/osWritebackDCache (stub)
    if (ba[i] == 0x80051980 || ba[i] == 0x8005BCC0)
    {
        // Call to our hook
        emit_call((intptr_t)detour_stub);
        // Link back to Return addr
        DETOUR_LINK_LR;

        return DETOUR_SUCCESS;
    }

    // Quake2 hook for sqrtf
    if (ba[i] == 0x80051680)
    {
        // Call to our hook
        emit_call((intptr_t)SqrtF);
        // Link back to Return addr
        DETOUR_LINK_LR;

        return DETOUR_SUCCESS;
    }

    // Quake2 hook for sinf
    if (ba[i] == 0x800517E0)
    {
        // Call to our hook
        emit_call((intptr_t)SinF);
        // Link back to Return addr
        DETOUR_LINK_LR;

        return DETOUR_SUCCESS;
    }

    // Quake2 hook for __cosf
    if (ba[i] == 0x80051690)
    {
        // Call to our hook
        emit_call((intptr_t)CosF);
        // Link back to Return addr
        DETOUR_LINK_LR;

        return DETOUR_SUCCESS;
    }

    // Quake2 hook for __ll_div
    if (ba[i] == 0x80060A20)
    {
        //emit_addimm64(ESP, -32, ESP);
        emit_loadreg(4, ARG1_REG);
        emit_loadreg(5, ARG2_REG);
        emit_loadreg(6, ARG3_REG);
        emit_loadreg(7, ARG4_REG);
        // Call to our hook
        emit_call((intptr_t)detour__ll_div);
        //emit_addimm64(ESP, 32, ESP);
        // Link back to Return addr
        DETOUR_LINK_LR;

        return DETOUR_SUCCESS;
    }

    return DETOUR_IGNORE;
}
