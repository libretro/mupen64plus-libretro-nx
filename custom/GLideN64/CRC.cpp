#include "CRC.h"
#include "xxhash.h"

void CRC_Init(){}

u32 CRC_Calculate( u32 crc, const void *buffer, u32 count )
{
	return XXH32(buffer, count, crc);
}

u32 CRC_CalculatePalette( u32 crc, const void *buffer, u32 count )
{
	return XXH32(buffer, count, crc);
}
