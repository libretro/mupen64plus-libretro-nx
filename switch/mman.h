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

Jit dynarec_jit;
void *jit_rx_addr = 0;
u_char *jit_dynrec = 0;
void *jit_rw_addr = 0;
void *jit_rw_buffer = 0;
void *jit_old_addr = 0;
size_t jit_len = 0;
bool jit_is_executable = false;
extern char __start__;

static inline void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    (void)fd;
    (void)offset;

    jit_len = len;
    if (R_SUCCEEDED(jitCreate(&dynarec_jit, jit_len)))
    {
        jit_len = dynarec_jit.size;
        jit_rw_buffer = malloc(jit_len);
        jit_rx_addr = (void*)(&__start__ - 0x1000 - jit_len);
        jit_old_addr = dynarec_jit.rx_addr;
        dynarec_jit.rx_addr = jit_rx_addr;
        jit_rw_addr = jitGetRwAddr(&dynarec_jit);
        jit_dynrec = (u_char*)jit_rw_addr;
        printf("Jit Initialized: RX %p, RW %p\n", jit_rx_addr, jit_rw_addr);
        printf("Transition to executable\n");
        jitTransitionToExecutable(&dynarec_jit);
        jit_is_executable = true;

        return jit_rx_addr;
    }
    else
    {
        printf("Jit failed!\n");
        return (void*)-1;
    }
}

static inline void jit_force_writeable()
{
  if(jit_is_executable){
    //printf("Setting the CodeMemory writable\n");
    svcSetProcessMemoryPermission(envGetOwnProcessHandle(), (u64) jit_rx_addr, jit_len, Perm_Rw);
    jit_is_executable = false;
  }
}

static inline void jit_force_executable()
{
  if(!jit_is_executable){
    //printf("Transition to executable\n");
    jitTransitionToWritable(&dynarec_jit);
    jitTransitionToExecutable(&dynarec_jit);
    jit_is_executable = true;
  }
}

static inline int mprotect(void *addr, size_t len, int prot)
{
    return 0;
}

static inline int munmap(void *addr, size_t len)
{
    jitTransitionToWritable(&dynarec_jit);

    if(jit_old_addr != 0)
        dynarec_jit.rx_addr = jit_old_addr;
    jit_old_addr = 0;

    jitClose(&dynarec_jit);
    free(jit_rw_buffer);
    jit_rw_buffer = 0;
    
    return 0;
}

#ifdef __cplusplus
};
#endif

#endif // MMAN_H
