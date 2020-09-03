/**********************************************************************************
Common gfx plugin spec, version #1.3 maintained by zilmar (zilmar@emulation64.com)

All questions or suggestions should go through the mailing list.
http://www.egroups.com/group/Plugin64-Dev
***********************************************************************************

Notes:
------

Setting the approprate bits in the MI_INTR_REG and calling CheckInterrupts which 
are both passed to the DLL in InitiateGFX will generate an Interrupt from with in 
the plugin.

The Setting of the RSP flags and generating an SP interrupt  should not be done in
the plugin

**********************************************************************************/
#ifndef _GFX_H_INCLUDED__
#define _GFX_H_INCLUDED__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/param.h>

#define PLUGIN_TYPE_RSP             1
#define PLUGIN_TYPE_GFX             2
#define PLUGIN_TYPE_AUDIO           3
#define PLUGIN_TYPE_CONTROLLER      4
#define PLUGIN_VERSION              0x020000
#define VIDEO_PLUGIN_API_VERSION	0x020200
#define CONFIG_API_VERSION          0x020000
#define VIDEXT_API_VERSION          0x030000

/* endianness */
#ifdef MSB_FIRST
    #define BYTE_XOR_DWORD_SWAP 4
    #define WORD_XOR_DWORD_SWAP 2
    #define BYTE_ADDR_XOR       0
    #define WORD_ADDR_XOR       0
    #define BYTE4_XOR_BE(a)     (a)
#else
    #define BYTE_ADDR_XOR       3
    #define WORD_ADDR_XOR       1
    #define BYTE4_XOR_BE(a)     ((a) ^ BYTE_ADDR_XOR)
    #define BYTE_XOR_DWORD_SWAP 7
    #define WORD_XOR_DWORD_SWAP 3
#endif

extern bool no_dlist;
extern int exception;
extern uint32_t BMASK;
extern uint32_t update_screen_count;
#define LRDP(...)

#include "m64p_plugin.h"


/***** Structures *****/
typedef struct {
    uint16_t Version;        /* Set to 0x0103 */
    uint16_t Type;           /* Set to PLUGIN_TYPE_GFX */
    char Name[100];      /* Name of the DLL */

    /* If DLL supports memory these memory options then set them to TRUE or FALSE
       if it does not support it */
    int NormalMemory;    /* a normal uint8_t array */ 
    int MemoryBswaped;  /* a normal uint8_t array where the memory has been pre
                              bswap on a dword (32 bits) boundry */
} PLUGIN_INFO;

extern GFX_INFO gfx_info;

#ifdef __cplusplus
extern "C" {
#endif

EXPORT m64p_error CALL PluginGetVersionVideo(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities);

#ifdef __cplusplus
}
#endif


#endif



