#ifndef M64P_LIBRETRO_MEMORY_H
#define M64P_LIBRETRO_MEMORY_H

#include <stdint.h>
#include <main/eep_file.h>
#include <main/mpk_file.h>
#include <main/fla_file.h>
#include <main/sra_file.h>

typedef struct _save_memory_data
{
   uint8_t eeprom[EEPROM_MAX_SIZE];
   uint8_t mempack[4][MEMPAK_SIZE];
   uint8_t sram[SRAM_SIZE];
   uint8_t flashram[FLASHRAM_SIZE];
} save_memory_data;

extern save_memory_data saved_memory;

#endif
