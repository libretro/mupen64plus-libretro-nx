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

#define PROT_READ 0b001
#define PROT_WRITE 0b010
#define PROT_EXEC 0b100
#define MAP_PRIVATE 2
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20

#define MAP_FAILED ((void *)-1)
void* ptr_rw = NULL;

static inline void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    (void)fd;
    (void)offset;

    size_t size = (len + 0xFFF) &~ 0xFFF;
	ptr_rw = virtmemReserve(size);
    if (R_SUCCEEDED(svcMapProcessMemory(ptr_rw, envGetOwnProcessHandle(), (u64)addr, size)))
    {
        return ptr_rw;
    }
    else
    {
        printf("[NXJIT]: Jit failed!\n");
        return (void*)-1;
    }
}

static inline int mprotect(void *addr, size_t len, int prot)
{
    return 0;
}

static inline int munmap(void *addr, size_t len)
{
    size_t size = (len + 0xFFF) &~ 0xFFF;
    svcUnmapProcessMemory(ptr_rw, envGetOwnProcessHandle(), (u64)addr, size);
    printf("[NXJIT]: Jit closed\n");
    
    return 0;
}

#ifdef __cplusplus
};
#endif

#endif // MMAN_H
