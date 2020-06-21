#include "N64.h"

u8 *HEADER;

/* 
    DMEM and IMEM conflict with CXD4 with GCC 10.x
    It's defined as pu8 so it's the same type as used in GLN64, just typedef'd.
    Since we never run both at the same time, let's just link CXD4's to this
    Also we always assume that we will build with GLideN64 at all times
*/
extern "C" {
    u8 *DMEM;
    u8 *IMEM;
}

u64 TMEM[512];
u8 *RDRAM;

u32 RDRAMSize = 0;

N64Regs REG;

bool ConfigOpen = false;
