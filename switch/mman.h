#ifndef MMAN_H
#define MMAN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <switch.h>
#include <stdlib.h>

//#include "3ds_utils.h"

#define PROT_READ 0b001
#define PROT_WRITE 0b010
#define PROT_EXEC 0b100
#define MAP_PRIVATE 2
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20

#define MAP_FAILED ((void *)-1)

static void *dynarec_cache = NULL;
static void *dynarec_cache_mapping = NULL;

static inline void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    (void)fd;
    (void)offset;

    //void* addr_out;
    Result rc = svcMapPhysicalMemory(addr, len);
    if (R_FAILED(rc))
    {
        printf("mmap failed\n");
        return malloc(len);
    }

    return addr;
}

static inline int mprotect(void *addr, size_t len, int prot)
{
    return 0;
}

static inline int munmap(void *addr, size_t len)
{
    Result rc = svcUnmapPhysicalMemory(addr, len);
    if (R_FAILED(rc))
    {
        printf("munmap failed\n");
        free(addr);
    }
    return 0;
}

#ifdef __cplusplus
};
#endif

#endif // MMAN_H
