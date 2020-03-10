/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - fb.c                                                    *
 *   Mupen64Plus homepage: https://mupen64plus.org/                        *
 *   Copyright (C) 2014 Bobby Smiles                                       *
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

#include "fb.h"

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "device/memory/memory.h"
#include "device/device.h"
#include "device/r4300/r4300_core.h"
#include "device/rdram/rdram.h"
#include "osal/preproc.h"
#include "plugin/plugin.h"
#include <string.h>

static osal_inline size_t fb_buffer_size(const FrameBufferInfo* fb_info)
{
    return fb_info->width * fb_info->height * fb_info->size;
}

void pre_framebuffer_read(struct fb* fb, uint32_t address)
{
    if (!fb->infos[0].addr) {
        return;
    }

    size_t i;

    for (i = 0; i < FB_INFOS_COUNT; ++i) {

        /* skip empty fb info */
        if (fb->infos[i].addr == 0) {
            continue;
        }

        /* if address in within a fb and its page is dirty,
         * notify GFX plugin and mark page as not dirty */
        uint32_t begin = fb->infos[i].addr;
        uint32_t end   = fb->infos[i].addr + fb_buffer_size(&fb->infos[i]) - 1;

        if ((address >= begin) && (address <= end) && (fb->dirty_page[address >> 12])) {
            gfx.fBRead(address);
            fb->dirty_page[address >> 12] = 0;
        }
    }
}

void post_framebuffer_write(struct fb* fb, uint32_t address, uint32_t length)
{
    if (!fb->infos[0].addr) {
        return;
    }

    size_t i, j;
    unsigned char size;
    if (length % 4 == 0)
        size = 4;
    else if (length % 2 == 0)
        size = 2;
    else
        size = 1;

    for (i = 0; i < FB_INFOS_COUNT; ++i) {

        /* skip empty fb info */
        if (fb->infos[i].addr == 0) {
            continue;
        }

        /* if address in within a fb notify GFX plugin */
        uint32_t begin = fb->infos[i].addr;
        uint32_t end   = fb->infos[i].addr + fb_buffer_size(&fb->infos[i]) - 1;

        for (j = 0; j < length; j += size) {
            if ((address + j >= begin) && (address + j <= end)) {
                gfx.fBWrite(address + j, size);
            }
        }
    }
}

struct fb* _fb;
void init_fb(struct fb* fb,
             struct memory* mem,
             struct rdram* rdram,
             struct r4300_core* r4300)
{
    fb->mem = mem;
    fb->rdram = rdram;
    fb->r4300 = r4300;
    _fb = fb;
}

static PVOID handler;
LONG WINAPI ExceptionHandler(struct _EXCEPTION_POINTERS *ExceptionInfo);
void poweron_fb(struct fb* fb)
{
    memset(fb->dirty_page, 0, FB_DIRTY_PAGES_COUNT*sizeof(fb->dirty_page[0]));
    memset(fb->infos, 0, FB_INFOS_COUNT*sizeof(fb->infos[0]));
    fb->once = 1;
    handler = AddVectoredExceptionHandler(1, ExceptionHandler);
}

void read_rdram_fb(void* opaque, uint32_t address, uint32_t* value)
{
    struct fb* fb = (struct fb*)opaque;
    pre_framebuffer_read(fb, address);
    read_rdram_dram(fb->rdram, address, value);
}

void write_rdram_fb(void* opaque, uint32_t address, uint32_t value, uint32_t mask)
{
    printf("write_rdram_fb address %x value %x mask %x\n", address, value, mask);
    fflush(stdout);

    struct fb* fb = (struct fb*)opaque;
    write_rdram_dram(fb->rdram, address, value, mask);

    uint32_t addr = address & ~0x3;
    size_t size = 4;

    switch(mask)
    {
    case 0x000000ff:
        addr += (3 ^ S8);
        size = 1;
        break;

    case 0x0000ff00:
        addr += (2 ^ S8);
        size = 1;
        break;

    case 0x00ff0000:
        addr += (1 ^ S8);
        size = 1;
        break;

    case 0xff000000:
        addr += (0 ^ S8);
        size = 1;
        break;

    case 0x0000ffff:
        addr += (2 ^ S16);
        size = 2;
        break;

    case 0xffff0000:
        addr += (0 ^ S16);
        size = 2;
        break;

    case 0xffffffff:
        addr += 0;
        size = 4;
        break;

    default:
        DebugMessage(M64MSG_WARNING, "Unknown mask %08x !!!", mask);
    }

    post_framebuffer_write(fb, addr, size);
}


#define R(x) read_ ## x
#define W(x) write_ ## x
#define RW(x) R(x), W(x)

extern struct device g_dev;
static uint64_t dummy[10];
LONG WINAPI ExceptionHandler(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
        uintptr_t fault_addr = ExceptionInfo->ExceptionRecord->ExceptionInformation[1];

        if ((fault_addr < (uintptr_t)g_dev.r4300.mem->base)
            || (fault_addr >= (uintptr_t)g_dev.r4300.mem->base + 0x20000000))
        {
            // Ghetto Stepping :shrek:
            ExceptionInfo->ContextRecord->Rip++;
            return EXCEPTION_CONTINUE_EXECUTION;
        }

		// TODO: fix non full mem base
		uint32_t offset = (uint32_t)(fault_addr - (uintptr_t)g_dev.r4300.mem->base);
              
#define READ_FLAG 0
#define WRITE_FLAG 1
        int rw = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
        printf("Fault Addr: %p, Offset: 0x%x RW: %d\n", fault_addr, offset, rw);
        printf("RAX: %p\n", ExceptionInfo->ContextRecord->Rax);
        printf("RBX: %p\n", ExceptionInfo->ContextRecord->Rbx);
        printf("RCX: %p\n", ExceptionInfo->ContextRecord->Rcx);
        printf("RDX: %p\n", ExceptionInfo->ContextRecord->Rdx);
        printf("RSI: %p\n", ExceptionInfo->ContextRecord->Rsi);
        printf("RDI: %p\n", ExceptionInfo->ContextRecord->Rdi);
        printf("RSP: %p\n", ExceptionInfo->ContextRecord->Rsp);
        printf("RSP[0]: 0x%x\n", ((uint32_t*)ExceptionInfo->ContextRecord->Rsp)[0]);
        printf("RSP[1]: 0x%x\n", ((uint32_t*)ExceptionInfo->ContextRecord->Rsp)[1]);
        printf("RSP[2]: 0x%x\n", ((uint32_t*)ExceptionInfo->ContextRecord->Rsp)[2]);
        printf("RSP[3]: 0x%x\n", ((uint32_t*)ExceptionInfo->ContextRecord->Rsp)[3]);
        printf("RSP[4]: 0x%x\n", ((uint32_t*)ExceptionInfo->ContextRecord->Rsp)[4]);
        printf("RSP[4]: 0x%x\n", ((uint32_t*)ExceptionInfo->ContextRecord->Rsp)[4]);
        printf("RSP[4]: 0x%x\n", ((uint32_t*)ExceptionInfo->ContextRecord->Rsp)[5]);
		printf("RIP: %p\n", ExceptionInfo->ContextRecord->Rip);
        printf("RIP[0]: 0x%x\n", ((uint32_t*)ExceptionInfo->ContextRecord->Rip)[0]);
		printf("PC: 0x%x\n", g_dev.r4300.new_dynarec_hot_state.pcaddr);
        fflush(stdout);
		DWORD old_protect;

        if(rw) // write
        {
            unprotect_framebuffers(_fb);
            int matched = 0;

            for (int i = 0; i < FB_INFOS_COUNT; ++i) {

                /* skip empty fb info */
                if (_fb->infos[i].addr == 0) {
                    continue;
                }

                if(offset >= _fb->infos[i].addr && offset <= (_fb->infos[i].addr + fb_buffer_size(&_fb->infos[i]) - 1))
                {
                    matched = 1;
                    uint32_t mask = ExceptionInfo->ContextRecord->Rbx - 1;
                    // MOV DWORD PTR
                    if((((uint32_t*)ExceptionInfo->ContextRecord->Rip)[0] & 0x00FF0000) == 0x00890000)
                    {
                        write_rdram_fb(_fb, offset, ExceptionInfo->ContextRecord->Rax, mask);
                        write_rdram_fb(_fb, offset, ExceptionInfo->ContextRecord->Rax << 16, 0xffff0000);
                    } else {
                        write_rdram_fb(_fb, offset, ExceptionInfo->ContextRecord->Rax, mask);     
                    }
                    ExceptionInfo->ContextRecord->Rip++;
                    break;
                }
            }
            
            if(matched)
            {
                // Prevents issues due to forced internal page alignment in VirtualProtect
                protect_framebuffers(_fb);
            }
        }

		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else
	{
            // Ghetto Stepping :shrek:
            ExceptionInfo->ContextRecord->Rip++;
            return EXCEPTION_CONTINUE_EXECUTION;
	}
}

void protect_framebuffers(struct fb* fb)
{
    DWORD old_protect;
    size_t i, j;
    struct mem_mapping fb_mapping = { 0, 0, M64P_MEM_RDRAM, { fb, RW(rdram_fb) } };

    /* check API support */
    if (!(gfx.fBGetFrameBufferInfo && gfx.fBRead && gfx.fBWrite)) {
        return;
    }

    /* ask fb info to gfx plugin */
    gfx.fBGetFrameBufferInfo(fb->infos);

    /* return early if not FB info is present */
    if (fb->infos[0].addr == 0) {
        return;
    }

    for (i = 0; i < FB_INFOS_COUNT; ++i) {

        /* skip empty fb info */
        if (fb->infos[i].addr == 0) {
            continue;
        }

        /* map fb rw handlers */
        fb_mapping.begin = fb->infos[i].addr;
        fb_mapping.end   = fb->infos[i].addr + fb_buffer_size(&fb->infos[i]) - 1;
        printf("FB Begin: 0x%x, FB End: 0x%x\n", fb_mapping.begin, fb_mapping.end);
        fflush(stdout);

        //apply_mem_mapping(fb->mem, &fb_mapping);
        if(!VirtualProtect((LPVOID)g_dev.r4300.mem->base + fb->infos[i].addr, fb_buffer_size(&fb->infos[i]) - 1, PAGE_READONLY, &old_protect))
        {
            printf("protect_framebuffers VirtualProtect failed!\n");
            fflush(stdout);
        }

        /* mark all pages that are within a fb as dirty */
        for (j = fb_mapping.begin >> 12; j <= (fb_mapping.end >> 12); ++j) {
            fb->dirty_page[j] = 1;
        }

        /* disable dynarec "fast memory" code generation to avoid direct memory accesses */
        if (fb->once) {
            fb->once = 0;
#ifndef NEW_DYNAREC
            fb->r4300->recomp.fast_memory = 0;
#endif

            /* also need to invalidate cached code to regen non fast memory code path */
            invalidate_r4300_cached_code(fb->r4300, 0, 0);
        }
    }
}

void unprotect_framebuffers(struct fb* fb)
{
    size_t i;
    DWORD old_protect;
    struct mem_mapping ram_mapping = { 0, 0, M64P_MEM_RDRAM, { fb->rdram, RW(rdram_dram) } };

    /* return early if FB info is not supported or empty */
    if (!fb->infos[0].addr) {
        return;
    }

    for (i = 0; i < FB_INFOS_COUNT; ++i) {

        /* skip empty fb info */
        if (fb->infos[i].addr == 0) {
            continue;
        }

        /* restore ram rw handlers */
        ram_mapping.begin = fb->infos[i].addr;
        ram_mapping.end   = fb->infos[i].addr + fb_buffer_size(&fb->infos[i]) - 1;
        //apply_mem_mapping(fb->mem, &ram_mapping);
        if(!VirtualProtect((LPVOID)g_dev.r4300.mem->base + fb->infos[i].addr, fb_buffer_size(&fb->infos[i]) - 1, PAGE_READWRITE, &old_protect))
        {
            printf("unprotect_framebuffers VirtualProtect failed!\n");
            fflush(stdout);
        }
    }
}
