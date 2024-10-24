/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - util.c                                                  *
 *   Mupen64Plus homepage: https://mupen64plus.org/                        *
 *   Copyright (C) 2020 Richard42                                          *
 *   Copyright (C) 2012 CasualJames                                        *
 *   Copyright (C) 2002 Hacktarux                                          *
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

/**
 * Provides common utilities to the rest of the code:
 *  -String functions
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "osal/files.h"
#include "osal/preproc.h"
#include "rom.h"
#include "util.h"

/**********************
     File utilities
 **********************/

file_status_t read_from_file(const char *filename, void *data, size_t size)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        return file_open_error;
    }

    if (fread(data, 1, size, f) != size)
    {
        fclose(f);
        return file_read_error;
    }

    fclose(f);
    return file_ok;
}

file_status_t write_to_file(const char *filename, const void *data, size_t size)
{
    FILE *f = fopen(filename, "wb");
    if (f == NULL)
    {
        return file_open_error;
    }

    if (fwrite(data, 1, size, f) != size)
    {
        fclose(f);
        return file_write_error;
    }

    fclose(f);
    return file_ok;
}


file_status_t write_chunk_to_file(const char *filename, const void *data, size_t size, size_t offset)
{
    FILE *f;

    /* first try to open with rb+ to avoid wiping existing content,
     * otherwise create file */
    if ((f = fopen(filename, "rb+")) == NULL) {
        if ((f = fopen(filename, "wb")) == NULL) {
            return file_open_error;
        }
    }

    /* According to POSIX fseek past file size is supported and will fill with zeros bytes
     * (and use sparse file if supported).
     * So we can use it to position next write operation at desired offset.
     */
    if (fseek(f, offset, SEEK_SET)) {
        fclose(f);
        return file_open_error;
    }

    if (fwrite(data, 1, size, f) != size)
    {
        fclose(f);
        return file_write_error;
    }

    fclose(f);
    return file_ok;
}


file_status_t load_file(const char* filename, void** buffer, size_t* size)
{
    FILE* fd;
    size_t l_size, bytes_read;
    void* l_buffer;
    int err;
    file_status_t ret;

    /* open file */
    ret = file_open_error;
    fd = fopen(filename, "rb");
    if (fd == NULL)
    {
        return file_open_error;
    }

    /* obtain file size */
    ret = file_size_error;
    err = fseek(fd, 0, SEEK_END);
    if (err != 0)
    {
        goto close_file;
    }

    err = ftell(fd);
    if (err == -1)
    {
        goto close_file;
    }
    l_size = (size_t)err;

    err = fseek(fd, 0, SEEK_SET);
    if (err != 0)
    {
        goto close_file;
    }

    /* allocate buffer */
    l_buffer = malloc(l_size);
    if (l_buffer == NULL)
    {
        goto close_file;
    }

    /* copy file content to buffer */
    ret = file_read_error;
    bytes_read = fread(l_buffer, 1, l_size, fd);
    if (bytes_read != l_size)
    {
        free(l_buffer);
        goto close_file;
    }

    /* commit buffer,size */
    ret = file_ok;
    *buffer = l_buffer;
    *size = l_size;

    /* close file */
close_file:
    fclose(fd);
    return ret;
}

file_status_t get_file_size(const char* filename, size_t* size)
{
    FILE* fd;
    int err;
    file_status_t ret;

    /* open file */
    ret = file_open_error;
    fd = fopen(filename, "rb");
    if (fd == NULL)
    {
        return file_open_error;
    }

    /* obtain file size */
    ret = file_size_error;
    err = fseek(fd, 0, SEEK_END);
    if (err != 0)
    {
        goto close_file;
    }

    err = ftell(fd);
    if (err == -1)
    {
        goto close_file;
    }

    ret = file_ok;
    *size = (size_t)err;

    /* close file */
close_file:
    fclose(fd);
    return ret;
}

/**********************
   Byte swap utilities
 **********************/
void swap_buffer(void *buffer, size_t length, size_t count)
{
    size_t i;
    if (length == 2)
    {
        uint16_t *pun = (uint16_t *) buffer;
        for (i = 0; i < count; i++)
            pun[i] = m64p_swap16(pun[i]);
    }
    else if (length == 4)
    {
        uint32_t *pun = (uint32_t *) buffer;
        for (i = 0; i < count; i++)
            pun[i] = m64p_swap32(pun[i]);
    }
    else if (length == 8)
    {
        uint64_t *pun = (uint64_t *) buffer;
        for (i = 0; i < count; i++)
            pun[i] = m64p_swap64(pun[i]);
    }
}

void to_little_endian_buffer(void *buffer, size_t length, size_t count)
{
#if defined(M64P_BIG_ENDIAN)
    swap_buffer(buffer, length, count);
#endif
}

void to_big_endian_buffer(void *buffer, size_t length, size_t count)
{
#if !defined(M64P_BIG_ENDIAN)
    swap_buffer(buffer, length, count);
#endif
}

/* Simple serialization primitives,
 * Use byte access to avoid alignment issues.
 */
uint8_t load_beu8(const unsigned char *ptr)
{
    return (uint8_t)ptr[0];
}

uint16_t load_beu16(const unsigned char *ptr)
{
    return ((uint16_t)ptr[0] << 8)
         | ((uint16_t)ptr[1] << 0);
}

uint32_t load_beu32(const unsigned char *ptr)
{
    return ((uint32_t)ptr[0] << 24)
         | ((uint32_t)ptr[1] << 16)
         | ((uint32_t)ptr[2] <<  8)
         | ((uint32_t)ptr[3] <<  0);
}

uint64_t load_beu64(const unsigned char *ptr)
{
    return ((uint64_t)ptr[0] << 56)
         | ((uint64_t)ptr[1] << 48)
         | ((uint64_t)ptr[2] << 40)
         | ((uint64_t)ptr[3] << 32)
         | ((uint64_t)ptr[4] << 24)
         | ((uint64_t)ptr[5] << 16)
         | ((uint64_t)ptr[6] <<  8)
         | ((uint64_t)ptr[7] <<  0);
}


uint8_t load_leu8(const unsigned char *ptr)
{
    return (uint8_t)ptr[0];
}

uint16_t load_leu16(const unsigned char *ptr)
{
    return ((uint16_t)ptr[0] << 0)
         | ((uint16_t)ptr[1] << 8);
}

uint32_t load_leu32(const unsigned char *ptr)
{
    return ((uint32_t)ptr[0] <<  0)
         | ((uint32_t)ptr[1] <<  8)
         | ((uint32_t)ptr[2] << 16)
         | ((uint32_t)ptr[3] << 24);
}

uint64_t load_leu64(const unsigned char *ptr)
{
    return ((uint64_t)ptr[0] <<  0)
         | ((uint64_t)ptr[1] <<  8)
         | ((uint64_t)ptr[2] << 16)
         | ((uint64_t)ptr[3] << 24)
         | ((uint64_t)ptr[4] << 32)
         | ((uint64_t)ptr[5] << 40)
         | ((uint64_t)ptr[6] << 48)
         | ((uint64_t)ptr[7] << 56);
}



void store_beu8(uint8_t value, unsigned char *ptr)
{
    ptr[0] = (uint8_t)value;
}

void store_beu16(uint16_t value, unsigned char *ptr)
{
    ptr[0] = (uint8_t)(value >> 8);
    ptr[1] = (uint8_t)(value >> 0);
}

void store_beu32(uint32_t value, unsigned char *ptr)
{
    ptr[0] = (uint8_t)(value >> 24);
    ptr[1] = (uint8_t)(value >> 16);
    ptr[2] = (uint8_t)(value >>  8);
    ptr[3] = (uint8_t)(value >>  0);
}

void store_beu64(uint64_t value, unsigned char *ptr)
{
    ptr[0] = (uint8_t)(value >> 56);
    ptr[1] = (uint8_t)(value >> 48);
    ptr[2] = (uint8_t)(value >> 40);
    ptr[3] = (uint8_t)(value >> 32);
    ptr[4] = (uint8_t)(value >> 24);
    ptr[5] = (uint8_t)(value >> 16);
    ptr[6] = (uint8_t)(value >>  8);
    ptr[7] = (uint8_t)(value >>  0);
}


void store_leu8(uint8_t value, unsigned char *ptr)
{
    ptr[0] = (uint8_t)value;
}

void store_leu16(uint16_t value, unsigned char *ptr)
{
    ptr[0] = (uint8_t)(value >> 0);
    ptr[1] = (uint8_t)(value >> 8);
}

void store_leu32(uint32_t value, unsigned char *ptr)
{
    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
}

void store_leu64(uint64_t value, unsigned char *ptr)
{
    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
    ptr[4] = (uint8_t)(value >> 32);
    ptr[5] = (uint8_t)(value >> 40);
    ptr[6] = (uint8_t)(value >> 48);
    ptr[7] = (uint8_t)(value >> 56);
}


/**********************
    Random utilities
 **********************/

static inline uint64_t rotl(uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

struct splitmix64_state { uint64_t x; };

static uint64_t splitmix64_next(struct splitmix64_state* s) {
	uint64_t z = (s->x += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

struct xoshiro256pp_state xoshiro256pp_seed(uint64_t seed)
{
    struct xoshiro256pp_state xs;
    struct splitmix64_state ss = { seed };

    size_t i;
    for (i = 0; i < 4; ++i) {
        xs.s[i] = splitmix64_next(&ss);
    }

    return xs;
}

uint64_t xoshiro256pp_next(struct xoshiro256pp_state* s) {
    uint64_t result = rotl(s->s[0] + s->s[3], 23) + s->s[0];

    uint64_t t = s->s[1] << 17;
    s->s[2] ^= s->s[0];
    s->s[3] ^= s->s[1];
    s->s[1] ^= s->s[2];
    s->s[0] ^= s->s[3];
    s->s[2] ^= t;
    s->s[3] = rotl(s->s[3], 45);

    return result;
}

/**********************
     GUI utilities
 **********************/
void countrycodestring(uint16_t countrycode, char *string)
{
    switch (countrycode)
    {
    case 0:    /* Demo */
        strcpy(string, "Demo");
        break;

    case '7':  /* Beta */
        strcpy(string, "Beta");
        break;

    case 0x41: /* Japan / USA */
        strcpy(string, "USA/Japan");
        break;

    case 0x44: /* Germany */
        strcpy(string, "Germany");
        break;

    case 0x45: /* USA */
        strcpy(string, "USA");
        break;

    case 0x46: /* France */
        strcpy(string, "France");
        break;

    case 'I':  /* Italy */
        strcpy(string, "Italy");
        break;

    case 0x4A: /* Japan */
        strcpy(string, "Japan");
        break;

    case 'S':  /* Spain */
        strcpy(string, "Spain");
        break;

    case 0x55: case 0x59:  /* Australia */
        sprintf(string, "Australia (0x%02" PRIX16 ")", countrycode);
        break;

    case 0x50: case 0x58: case 0x20:
    case 0x21: case 0x38: case 0x70:
        sprintf(string, "Europe (0x%02" PRIX16 ")", countrycode);
        break;

    default:
        sprintf(string, "Unknown (0x%02" PRIX16 ")", countrycode);
        break;
    }
}

void imagestring(unsigned char imagetype, char *string)
{
    switch (imagetype)
    {
    case Z64IMAGE:
        strcpy(string, ".z64 (native)");
        break;
    case V64IMAGE:
        strcpy(string, ".v64 (byteswapped)");
        break;
    case N64IMAGE:
        strcpy(string, ".n64 (wordswapped)");
        break;
    default:
        string[0] = '\0';
    }
}

/**********************
     Path utilities
 **********************/

const char* namefrompath(const char* path)
{
    const char* last_separator_ptr = strpbrk_reverse(OSAL_DIR_SEPARATORS, (char*)path, strlen(path));

    if (last_separator_ptr != NULL)
        return last_separator_ptr + 1;
    else
        return path;
}

static int is_path_separator(char c)
{
    return strchr(OSAL_DIR_SEPARATORS, c) != NULL;
}

char* combinepath(const char* first, const char *second)
{
    size_t len_first, off_second = 0;

    if (first == NULL || second == NULL)
        return NULL;

    len_first = strlen(first);

    while (is_path_separator(first[len_first-1]))
        len_first--;

    while (is_path_separator(second[off_second]))
        off_second++;

    return formatstr("%.*s%c%s", (int) len_first, first, OSAL_DIR_SEPARATORS[0], second + off_second);
}

/**********************
    String utilities
 **********************/

char* strpbrk_reverse(const char* needles, char* haystack, size_t haystack_len)
{
    size_t counter;

    for (counter = haystack_len; counter > 0; --counter)
    {
        if (strchr(needles, haystack[counter-1]))
            break;
    }

    if (counter == 0)
        return NULL;

    return haystack + counter - 1;
}

char *trim(char *str)
{
    char *start = str, *end = str + strlen(str);

    while (start < end && isspace((unsigned char)(*start)))
        start++;

    while (end > start && isspace((unsigned char)(*(end-1))))
        end--;

    memmove(str, start, end - start);
    str[end - start] = '\0';

    return str;
}

int string_replace_chars(char* str, const char* chars, const char r)
{
    int i, y;
    int str_size, chars_size;
    int replacements = 0;

    str_size   = strlen(str);
    chars_size = strlen(chars);

    for (i = 0; i < str_size; i++) {
        for (y = 0; y < chars_size; y++) {
            if (str[i] == chars[y]) {
                str[i] = r;
                replacements++;
                break;
            }
        }
    }

    return replacements;
}

int string_to_int(const char *str, int *result)
{
    char *endptr;
    long int n;
    if (*str == '\0' || isspace((unsigned char)(*str)))
        return 0;
    errno = 0;
    n = strtol(str, &endptr, 10);
    if (*endptr != '\0' || errno != 0 || n < INT_MIN || n > INT_MAX)
        return 0;
    *result = (int)n;
    return 1;
}

static unsigned char char2hex(char c)
{
    c = tolower(c);
    if(c >= '0' && c <= '9')
        return c - '0';
    else if(c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else
        return 0xFF;
}

int parse_hex(const char *str, unsigned char *output, size_t output_size)
{
    size_t i, j;
    for (i = 0; i < output_size; i++)
    {
        output[i] = 0;
        for (j = 0; j < 2; j++)
        {
            unsigned char h = char2hex(*str++);
            if (h == 0xFF)
                return 0;

            output[i] = (output[i] << 4) | h;
        }
    }

    if (*str != '\0')
        return 0;

    return 1;
}

char *formatstr(const char *fmt, ...)
{
	int size = 128, ret;
	char *str = (char *)malloc(size), *newstr;
	va_list args;

	/* There are two implementations of vsnprintf we have to deal with:
	 * C99 version: Returns the number of characters which would have been written
	 *              if the buffer had been large enough, and -1 on failure.
	 * Windows version: Returns the number of characters actually written,
	 *                  and -1 on failure or truncation.
	 * NOTE: An implementation equivalent to the Windows one appears in glibc <2.1.
	 */
	while (str != NULL)
	{
		va_start(args, fmt);
		ret = vsnprintf(str, size, fmt, args);
		va_end(args);

		// Successful result?
		if (ret >= 0 && ret < size)
			return str;

		// Increment the capacity of the buffer
		if (ret >= size)
			size = ret + 1; // C99 version: We got the needed buffer size
		else
			size *= 2; // Windows version: Keep guessing

		newstr = (char *)realloc(str, size);
		if (newstr == NULL)
			free(str);
		str = newstr;
	}

	return NULL;
}

ini_line ini_parse_line(char **lineptr)
{
    char *line = *lineptr, *endline = strchr(*lineptr, '\n'), *equal;
    ini_line l;

    // Null terminate the current line and point to the next line
    if (endline != NULL)
        *endline = '\0';
    *lineptr = line + strlen(line) + 1;

    // Parse the line contents
    trim(line);

    if (line[0] == '#' || line[0] == ';')
    {
        line++;

        l.type = INI_COMMENT;
        l.name = NULL;
        l.value = trim(line);
    }
    else if (line[0] == '[' && line[strlen(line)-1] == ']')
    {
        line[strlen(line)-1] = '\0';
        line++;

        l.type = INI_SECTION;
        l.name = trim(line);
        l.value = NULL;
    }
    else if ((equal = strchr(line, '=')) != NULL)
    {
        char *name = line, *value = equal + 1;
        *equal = '\0';

        l.type = INI_PROPERTY;
        l.name = trim(name);
        l.value = trim(value);
    }
    else
    {
        l.type = (*line == '\0') ? INI_BLANK : INI_TRASH;
        l.name = NULL;
        l.value = NULL;
    }

    return l;
}


static uint8_t pucTable[25088] = {   0,  0,  0,  1,  0,  2,  0,  3,  0,  4,  0,  5,  0,  6,  0,  7,  0,  8,  0,  9,  0, 10,  0, 11,
                                     0, 12,  0, 13,  0, 14,  0, 15,  0, 16,  0, 17,  0, 18,  0, 19,  0, 20,  0, 21,  0, 22,  0, 23,
                                     0, 24,  0, 25,  0, 26,  0, 27,  0, 28,  0, 29,  0, 30,  0, 31,  0, 32,  0, 33,  0, 34,  0, 35,
                                     0, 36,  0, 37,  0, 38,  0, 39,  0, 40,  0, 41,  0, 42,  0, 43,  0, 44,  0, 45,  0, 46,  0, 47,
                                     0, 48,  0, 49,  0, 50,  0, 51,  0, 52,  0, 53,  0, 54,  0, 55,  0, 56,  0, 57,  0, 58,  0, 59,
                                     0, 60,  0, 61,  0, 62,  0, 63,  0, 64,  0, 65,  0, 66,  0, 67,  0, 68,  0, 69,  0, 70,  0, 71,
                                     0, 72,  0, 73,  0, 74,  0, 75,  0, 76,  0, 77,  0, 78,  0, 79,  0, 80,  0, 81,  0, 82,  0, 83,
                                     0, 84,  0, 85,  0, 86,  0, 87,  0, 88,  0, 89,  0, 90,  0, 91,  0,165,  0, 93,  0, 94,  0, 95,
                                     0, 96,  0, 97,  0, 98,  0, 99,  0,100,  0,101,  0,102,  0,103,  0,104,  0,105,  0,106,  0,107,
                                     0,108,  0,109,  0,110,  0,111,  0,112,  0,113,  0,114,  0,115,  0,116,  0,117,  0,118,  0,119,
                                     0,120,  0,121,  0,122,  0,123,  0,124,  0,125, 32, 62,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,255, 97,255, 98,255, 99,255,100,255,101,255,102,255,103,
                                   255,104,255,105,255,106,255,107,255,108,255,109,255,110,255,111,255,112,255,113,255,114,255,115,
                                   255,116,255,117,255,118,255,119,255,120,255,121,255,122,255,123,255,124,255,125,255,126,255,127,
                                   255,128,255,129,255,130,255,131,255,132,255,133,255,134,255,135,255,136,255,137,255,138,255,139,
                                   255,140,255,141,255,142,255,143,255,144,255,145,255,146,255,147,255,148,255,149,255,150,255,151,
                                   255,152,255,153,255,154,255,155,255,156,255,157,255,158,255,159,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                    48,  0, 48,  1, 48,  2,255, 12,255, 14, 48,251,255, 26,255, 27,255, 31,255,  1, 48,155, 48,156,
                                     0,180,255, 64,  0,168,255, 62,255,227,255, 63, 48,253, 48,254, 48,157, 48,158, 48,  3, 78,221,
                                    48,  5, 48,  6, 48,  7, 48,252, 32, 21, 32, 16,255, 15,  0, 92, 48, 28, 32, 22,255, 92, 32, 38,
                                    32, 37, 32, 24, 32, 25, 32, 28, 32, 29,255,  8,255,  9, 48, 20, 48, 21,255, 59,255, 61,255, 91,
                                   255, 93, 48,  8, 48,  9, 48, 10, 48, 11, 48, 12, 48, 13, 48, 14, 48, 15, 48, 16, 48, 17,255, 11,
                                    34, 18,  0,177,  0,215,  0, 32,  0,247,255, 29, 34, 96,255, 28,255, 30, 34,102, 34,103, 34, 30,
                                    34, 52, 38, 66, 38, 64,  0,176, 32, 50, 32, 51, 33,  3,255,229,255,  4,  0,162,  0,163,255,  5,
                                   255,  3,255,  6,255, 10,255, 32,  0,167, 38,  6, 38,  5, 37,203, 37,207, 37,206, 37,199, 37,198,
                                    37,161, 37,160, 37,179, 37,178, 37,189, 37,188, 32, 59, 48, 18, 33,146, 33,144, 33,145, 33,147,
                                    48, 19,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                    34,  8, 34, 11, 34,134, 34,135, 34,130, 34,131, 34, 42, 34, 41,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32, 34, 39, 34, 40,  0,172, 33,210, 33,212, 34,  0, 34,  3,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32, 34, 32, 34,165,
                                    35, 18, 34,  2, 34,  7, 34, 97, 34, 82, 34,106, 34,107, 34, 26, 34, 61, 34, 29, 34, 53, 34, 43,
                                    34, 44,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32, 33, 43, 32, 48, 38,111, 38,109,
                                    38,106, 32, 32, 32, 33,  0,182,  0, 32,  0, 32,  0, 32,  0, 32, 37,239,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,255, 16,255, 17,255, 18,255, 19,255, 20,
                                   255, 21,255, 22,255, 23,255, 24,255, 25,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                   255, 33,255, 34,255, 35,255, 36,255, 37,255, 38,255, 39,255, 40,255, 41,255, 42,255, 43,255, 44,
                                   255, 45,255, 46,255, 47,255, 48,255, 49,255, 50,255, 51,255, 52,255, 53,255, 54,255, 55,255, 56,
                                   255, 57,255, 58,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,255, 65,255, 66,255, 67,
                                   255, 68,255, 69,255, 70,255, 71,255, 72,255, 73,255, 74,255, 75,255, 76,255, 77,255, 78,255, 79,
                                   255, 80,255, 81,255, 82,255, 83,255, 84,255, 85,255, 86,255, 87,255, 88,255, 89,255, 90,  0, 32,
                                     0, 32,  0, 32,  0, 32, 48, 65, 48, 66, 48, 67, 48, 68, 48, 69, 48, 70, 48, 71, 48, 72, 48, 73,
                                    48, 74, 48, 75, 48, 76, 48, 77, 48, 78, 48, 79, 48, 80, 48, 81, 48, 82, 48, 83, 48, 84, 48, 85,
                                    48, 86, 48, 87, 48, 88, 48, 89, 48, 90, 48, 91, 48, 92, 48, 93, 48, 94, 48, 95, 48, 96, 48, 97,
                                    48, 98, 48, 99, 48,100, 48,101, 48,102, 48,103, 48,104, 48,105, 48,106, 48,107, 48,108, 48,109,
                                    48,110, 48,111, 48,112, 48,113, 48,114, 48,115, 48,116, 48,117, 48,118, 48,119, 48,120, 48,121,
                                    48,122, 48,123, 48,124, 48,125, 48,126, 48,127, 48,128, 48,129, 48,130, 48,131, 48,132, 48,133,
                                    48,134, 48,135, 48,136, 48,137, 48,138, 48,139, 48,140, 48,141, 48,142, 48,143, 48,144, 48,145,
                                    48,146, 48,147,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32, 48,161, 48,162, 48,163, 48,164,
                                    48,165, 48,166, 48,167, 48,168, 48,169, 48,170, 48,171, 48,172, 48,173, 48,174, 48,175, 48,176,
                                    48,177, 48,178, 48,179, 48,180, 48,181, 48,182, 48,183, 48,184, 48,185, 48,186, 48,187, 48,188,
                                    48,189, 48,190, 48,191, 48,192, 48,193, 48,194, 48,195, 48,196, 48,197, 48,198, 48,199, 48,200,
                                    48,201, 48,202, 48,203, 48,204, 48,205, 48,206, 48,207, 48,208, 48,209, 48,210, 48,211, 48,212,
                                    48,213, 48,214, 48,215, 48,216, 48,217, 48,218, 48,219, 48,220, 48,221, 48,222, 48,223,  0, 32,
                                    48,224, 48,225, 48,226, 48,227, 48,228, 48,229, 48,230, 48,231, 48,232, 48,233, 48,234, 48,235,
                                    48,236, 48,237, 48,238, 48,239, 48,240, 48,241, 48,242, 48,243, 48,244, 48,245, 48,246,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  3,145,  3,146,  3,147,  3,148,  3,149,
                                     3,150,  3,151,  3,152,  3,153,  3,154,  3,155,  3,156,  3,157,  3,158,  3,159,  3,160,  3,161,
                                     3,163,  3,164,  3,165,  3,166,  3,167,  3,168,  3,169,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  3,177,  3,178,  3,179,  3,180,  3,181,  3,182,  3,183,  3,184,  3,185,
                                     3,186,  3,187,  3,188,  3,189,  3,190,  3,191,  3,192,  3,193,  3,195,  3,196,  3,197,  3,198,
                                     3,199,  3,200,  3,201,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     4, 16,  4, 17,  4, 18,  4, 19,  4, 20,  4, 21,  4,  1,  4, 22,  4, 23,  4, 24,  4, 25,  4, 26,
                                     4, 27,  4, 28,  4, 29,  4, 30,  4, 31,  4, 32,  4, 33,  4, 34,  4, 35,  4, 36,  4, 37,  4, 38,
                                     4, 39,  4, 40,  4, 41,  4, 42,  4, 43,  4, 44,  4, 45,  4, 46,  4, 47,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     4, 48,  4, 49,  4, 50,  4, 51,  4, 52,  4, 53,  4, 81,  4, 54,  4, 55,  4, 56,  4, 57,  4, 58,
                                     4, 59,  4, 60,  4, 61,  0, 32,  4, 62,  4, 63,  4, 64,  4, 65,  4, 66,  4, 67,  4, 68,  4, 69,
                                     4, 70,  4, 71,  4, 72,  4, 73,  4, 74,  4, 75,  4, 76,  4, 77,  4, 78,  4, 79,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32, 37,  0,
                                    37,  2, 37, 12, 37, 16, 37, 24, 37, 20, 37, 28, 37, 44, 37, 36, 37, 52, 37, 60, 37,  1, 37,  3,
                                    37, 15, 37, 19, 37, 27, 37, 23, 37, 35, 37, 51, 37, 43, 37, 59, 37, 75, 37, 32, 37, 47, 37, 40,
                                    37, 55, 37, 63, 37, 29, 37, 48, 37, 37, 37, 56, 37, 66,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32, 78,156, 85, 22, 90,  3,150, 63, 84,192, 97, 27, 99, 40, 89,246,144, 34,
                                   132,117,131, 28,122, 80, 96,170, 99,225,110, 37,101,237,132,102,130,166,155,245,104,147, 87, 39,
                                   101,161, 98,113, 91,155, 89,208,134,123,152,244,125, 98,125,190,155,142, 98, 22,124,159,136,183,
                                    91,137, 94,181, 99,  9,102,151,104, 72,149,199,151,141,103, 79, 78,229, 79, 10, 79, 77, 79,157,
                                    80, 73, 86,242, 89, 55, 89,212, 90,  1, 92,  9, 96,223, 97, 15, 97,112,102, 19,105,  5,112,186,
                                   117, 79,117,112,121,251,125,173,125,239,128,195,132, 14,136, 99,139,  2,144, 85,144,122, 83, 59,
                                    78,149, 78,165, 87,223,128,178,144,193,120,239, 78,  0, 88,241,110,162,144, 56,122, 50,131, 40,
                                   130,139,156, 47, 81, 65, 83,112, 84,189, 84,225, 86,224, 89,251, 95, 21,152,242,109,235,128,228,
                                   133, 45,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,150, 98,150,112,150,160,151,251,
                                    84, 11, 83,243, 91,135,112,207,127,189,143,194,150,232, 83,111,157, 92,122,186, 78, 17,120,147,
                                   129,252,110, 38, 86, 24, 85,  4,107, 29,133, 26,156, 59, 89,229, 83,169,109,102,116,220,149,143,
                                    86, 66, 78,145,144, 75,150,242,131, 79,153, 12, 83,225, 85,182, 91, 48, 95,113,102, 32,102,243,
                                   104,  4,108, 56,108,243,109, 41,116, 91,118,200,122, 78,152, 52,130,241,136, 91,138, 96,146,237,
                                   109,178,117,171,118,202,153,197, 96,166,139,  1,141,138,149,178,105,142, 83,173, 81,134,  0, 32,
                                    87, 18, 88, 48, 89, 68, 91,180, 94,246, 96, 40, 99,169, 99,244,108,191,111, 20,112,142,113, 20,
                                   113, 89,113,213,115, 63,126,  1,130,118,130,209,133,151,144, 96,146, 91,157, 27, 88,105,101,188,
                                   108, 90,117, 37, 81,249, 89, 46, 89,101, 95,128, 95,220, 98,188,101,250,106, 42,107, 39,107,180,
                                   115,139,127,193,137, 86,157, 44,157, 14,158,196, 92,161,108,150,131,123, 81,  4, 92, 75, 97,182,
                                   129,198,104,118,114, 97, 78, 89, 79,250, 83,120, 96,105,110, 41,122, 79,151,243, 78, 11, 83, 22,
                                    78,238, 79, 85, 79, 61, 79,161, 79,115, 82,160, 83,239, 86,  9, 89, 15, 90,193, 91,182, 91,225,
                                   121,209,102,135,103,156,103,182,107, 76,108,179,112,107,115,194,121,141,121,190,122, 60,123,135,
                                   130,177,130,219,131,  4,131,119,131,239,131,211,135,102,138,178, 86, 41,140,168,143,230,144, 78,
                                   151, 30,134,138, 79,196, 92,232, 98, 17,114, 89,117, 59,129,229,130,189,134,254,140,192,150,197,
                                   153, 19,153,213, 78,203, 79, 26,137,227, 86,222, 88, 74, 88,202, 94,251, 95,235, 96, 42, 96,148,
                                    96, 98, 97,208, 98, 18, 98,208,101, 57,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                   155, 65,102,102,104,176,109,119,112,112,117, 76,118,134,125,117,130,165,135,249,149,139,150,142,
                                   140,157, 81,241, 82,190, 89, 22, 84,179, 91,179, 93, 22, 97,104,105,130,109,175,120,141,132,203,
                                   136, 87,138,114,147,167,154,184,109,108,153,168,134,217, 87,163,103,255,134,206,146, 14, 82,131,
                                    86,135, 84,  4, 94,211, 98,225,100,185,104, 60,104, 56,107,187,115,114,120,186,122,107,137,154,
                                   137,210,141,107,143,  3,144,237,149,163,150,148,151,105, 91,102, 92,179,105,125,152, 77,152, 78,
                                    99,155,123, 32,106, 43,  0, 32,106,127,104,182,156, 13,111, 95, 82,114, 85,157, 96,112, 98,236,
                                   109, 59,110,  7,110,209,132, 91,137, 16,143, 68, 78, 20,156, 57, 83,246,105, 27,106, 58,151,132,
                                   104, 42, 81, 92,122,195,132,178,145,220,147,140, 86, 91,157, 40,104, 34,131,  5,132, 49,124,165,
                                    82,  8,130,197,116,230, 78,126, 79,131, 81,160, 91,210, 82, 10, 82,216, 82,231, 93,251, 85,154,
                                    88, 42, 89,230, 91,140, 91,152, 91,219, 94,114, 94,121, 96,163, 97, 31, 97, 99, 97,190, 99,219,
                                   101, 98,103,209,104, 83,104,250,107, 62,107, 83,108, 87,111, 34,111,151,111, 69,116,176,117, 24,
                                   118,227,119, 11,122,255,123,161,124, 33,125,233,127, 54,127,240,128,157,130,102,131,158,137,179,
                                   138,204,140,171,144,132,148, 81,149,147,149,145,149,162,150,101,151,211,153, 40,130, 24, 78, 56,
                                    84, 43, 92,184, 93,204,115,169,118, 76,119, 60, 92,169,127,235,141, 11,150,193,152, 17,152, 84,
                                   152, 88, 79,  1, 79, 14, 83,113, 85,156, 86,104, 87,250, 89, 71, 91,  9, 91,196, 92,144, 94, 12,
                                    94,126, 95,204, 99,238,103, 58,101,215,101,226,103, 31,104,203,104,196,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,106, 95, 94, 48,107,197,108, 23,108,125,117,127,121, 72, 91, 99,
                                   122,  0,125,  0, 95,189,137,143,138, 24,140,180,141,119,142,204,143, 29,152,226,154, 14,155, 60,
                                    78,128, 80,125, 81,  0, 89,147, 91,156, 98, 47, 98,128,100,236,107, 58,114,160,117,145,121, 71,
                                   127,169,135,251,138,188,139,112, 99,172,131,202,151,160, 84,  9, 84,  3, 85,171,104, 84,106, 88,
                                   138,112,120, 39,103,117,158,205, 83,116, 91,162,129, 26,134, 80,144,  6, 78, 24, 78, 69, 78,199,
                                    79, 17, 83,202, 84, 56, 91,174, 95, 19, 96, 37,101, 81,  0, 32,103, 61,108, 66,108,114,108,227,
                                   112,120,116,  3,122,118,122,174,123,  8,125, 26,124,254,125,102,101,231,114, 91, 83,187, 92, 69,
                                    93,232, 98,210, 98,224, 99, 25,110, 32,134, 90,138, 49,141,221,146,248,111,  1,121,166,155, 90,
                                    78,168, 78,171, 78,172, 79,155, 79,160, 80,209, 81, 71,122,246, 81,113, 81,246, 83, 84, 83, 33,
                                    83,127, 83,235, 85,172, 88,131, 92,225, 95, 55, 95, 74, 96, 47, 96, 80, 96,109, 99, 31,101, 89,
                                   106, 75,108,193,114,194,114,237,119,239,128,248,129,  5,130,  8,133, 78,144,247,147,225,151,255,
                                   153, 87,154, 90, 78,240, 81,221, 92, 45,102,129,105,109, 92, 64,102,242,105,117,115,137,104, 80,
                                   124,129, 80,197, 82,228, 87, 71, 93,254,147, 38,101,164,107, 35,107, 61,116, 52,121,129,121,189,
                                   123, 75,125,202,130,185,131,204,136,127,137, 95,139, 57,143,209,145,209, 84, 31,146,128, 78, 93,
                                    80, 54, 83,229, 83, 58,114,215,115,150,119,233,130,230,142,175,153,198,153,200,153,210, 81,119,
                                    97, 26,134, 94, 85,176,122,122, 80,118, 91,211,144, 71,150,133, 78, 50,106,219,145,231, 92, 81,
                                    92, 72,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32, 99,152,122,159,108,147,151,116,
                                   143, 97,122,170,113,138,150,136,124,130,104, 23,126,112,104, 81,147,108, 82,242, 84, 27,133,171,
                                   138, 19,127,164,142,205,144,225, 83,102,136,136,121, 65, 79,194, 80,190, 82, 17, 81, 68, 85, 83,
                                    87, 45,115,234, 87,139, 89, 81, 95, 98, 95,132, 96,117, 97,118, 97,103, 97,169, 99,178,100, 58,
                                   101,108,102,111,104, 66,110, 19,117,102,122, 61,124,251,125, 76,125,153,126, 75,127,107,131, 14,
                                   131, 74,134,205,138,  8,138, 99,139,102,142,253,152, 26,157,143,130,184,143,206,155,232,  0, 32,
                                    82,135, 98, 31,100,131,111,192,150,153,104, 65, 80,145,107, 32,108,122,111, 84,122,116,125, 80,
                                   136, 64,138, 35,103,  8, 78,246, 80, 57, 80, 38, 80,101, 81,124, 82, 56, 82, 99, 85,167, 87, 15,
                                    88,  5, 90,204, 94,250, 97,178, 97,248, 98,243, 99,114,105, 28,106, 41,114,125,114,172,115, 46,
                                   120, 20,120,111,125,121,119, 12,128,169,137,139,139, 25,140,226,142,210,144, 99,147,117,150,122,
                                   152, 85,154, 19,158,120, 81, 67, 83,159, 83,179, 94,123, 95, 38,110, 27,110,144,115,132,115,254,
                                   125, 67,130, 55,138,  0,138,250,150, 80, 78, 78, 80, 11, 83,228, 84,124, 86,250, 89,209, 91,100,
                                    93,241, 94,171, 95, 39, 98, 56,101, 69,103,175,110, 86,114,208,124,202,136,180,128,161,128,225,
                                   131,240,134, 78,138,135,141,232,146, 55,150,199,152,103,159, 19, 78,148, 78,146, 79, 13, 83, 72,
                                    84, 73, 84, 62, 90, 47, 95,140, 95,161, 96,159,104,167,106,142,116, 90,120,129,138,158,138,164,
                                   139,119,145,144, 78, 94,155,201, 78,164, 79,124, 79,175, 80, 25, 80, 22, 81, 73, 81,108, 82,159,
                                    82,185, 82,254, 83,154, 83,227, 84, 17,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                    84, 14, 85,137, 87, 81, 87,162, 89,125, 91, 84, 91, 93, 91,143, 93,229, 93,231, 93,247, 94,120,
                                    94,131, 94,154, 94,183, 95, 24, 96, 82, 97, 76, 98,151, 98,216, 99,167,101, 59,102,  2,102, 67,
                                   102,244,103,109,104, 33,104,151,105,203,108, 95,109, 42,109,105,110, 47,110,157,117, 50,118,135,
                                   120,108,122, 63,124,224,125,  5,125, 24,125, 94,125,177,128, 21,128,  3,128,175,128,177,129, 84,
                                   129,143,130, 42,131, 82,136, 76,136, 97,139, 27,140,162,140,252,144,202,145,117,146,113,120, 63,
                                   146,252,149,164,150, 77,  0, 32,152,  5,153,153,154,216,157, 59, 82, 91, 82,171, 83,247, 84,  8,
                                    88,213, 98,247,111,224,140,106,143, 95,158,185, 81, 75, 82, 59, 84, 74, 86,253,122, 64,145,119,
                                   157, 96,158,210,115, 68,111,  9,129,112,117, 17, 95,253, 96,218,154,168,114,219,143,188,107,100,
                                   152,  3, 78,202, 86,240, 87,100, 88,190, 90, 90, 96,104, 97,199,102, 15,102,  6,104, 57,104,177,
                                   109,247,117,213,125, 58,130,110,155, 66, 78,155, 79, 80, 83,201, 85,  6, 93,111, 93,230, 93,238,
                                   103,251,108,153,116,115,120,  2,138, 80,147,150,136,223, 87, 80, 94,167, 99, 43, 80,181, 80,172,
                                    81,141,103,  0, 84,201, 88, 94, 89,187, 91,176, 95,105, 98, 77, 99,161,104, 61,107,115,110,  8,
                                   112,125,145,199,114,128,120, 21,120, 38,121,109,101,142,125, 48,131,220,136,193,143,  9,150,155,
                                    82,100, 87, 40,103, 80,127,106,140,161, 81,180, 87, 66,150, 42, 88, 58,105,138,128,180, 84,178,
                                    93, 14, 87,252,120,149,157,250, 79, 92, 82, 74, 84,139,100, 62,102, 40,103, 20,103,245,122,132,
                                   123, 86,125, 34,147, 47,104, 92,155,173,123, 57, 83, 25, 81,138, 82, 55,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32, 91,223, 98,246,100,174,100,230,103, 45,107,186,133,169,150,209,
                                   118,144,155,214, 99, 76,147,  6,155,171,118,191,102, 82, 78,  9, 80,152, 83,194, 92,113, 96,232,
                                   100,146,101, 99,104, 95,113,230,115,202,117, 35,123,151,126,130,134,149,139,131,140,219,145,120,
                                   153, 16,101,172,102,171,107,139, 78,213, 78,212, 79, 58, 79,127, 82, 58, 83,248, 83,242, 85,227,
                                    86,219, 88,235, 89,203, 89,201, 89,255, 91, 80, 92, 77, 94,  2, 94, 43, 95,215, 96, 29, 99,  7,
                                   101, 47, 91, 92,101,175,101,189,101,232,103,157,107, 98,  0, 32,107,123,108, 15,115, 69,121, 73,
                                   121,193,124,248,125, 25,125, 43,128,162,129,  2,129,243,137,150,138, 94,138,105,138,102,138,140,
                                   138,238,140,199,140,220,150,204,152,252,107,111, 78,139, 79, 60, 79,141, 81, 80, 91, 87, 91,250,
                                    97, 72, 99,  1,102, 66,107, 33,110,203,108,187,114, 62,116,189,117,212,120,193,121, 58,128, 12,
                                   128, 51,129,234,132,148,143,158,108, 80,158,127, 95, 15,139, 88,157, 43,122,250,142,248, 91,141,
                                   150,235, 78,  3, 83,241, 87,247, 89, 49, 90,201, 91,164, 96,137,110,127,111,  6,117,190,140,234,
                                    91,159,133,  0,123,224, 80,114,103,244,130,157, 92, 97,133, 74,126, 30,130, 14, 81,153, 92,  4,
                                    99,104,141,102,101,156,113,110,121, 62,125, 23,128,  5,139, 29,142,202,144,110,134,199,144,170,
                                    80, 31, 82,250, 92, 58,103, 83,112,124,114, 53,145, 76,145,200,147, 43,130,229, 91,194, 95, 49,
                                    96,249, 78, 59, 83,214, 91,136, 98, 75,103, 49,107,138,114,233,115,224,122, 46,129,107,141,163,
                                   145, 82,153,150, 81, 18, 83,215, 84,106, 91,255, 99,136,106, 57,125,172,151,  0, 86,218, 83,206,
                                    84,104,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32, 91,151, 92, 49, 93,222, 79,238,
                                    97,  1, 98,254,109, 50,121,192,121,203,125, 66,126, 77,127,210,129,237,130, 31,132,144,136, 70,
                                   137,114,139,144,142,116,143, 47,144, 49,145, 75,145,108,150,198,145,156, 78,192, 79, 79, 81, 69,
                                    83, 65, 95,147, 98, 14,103,212,108, 65,110, 11,115, 99,126, 38,145,205,146,131, 83,212, 89, 25,
                                    91,191,109,209,121, 93,126, 46,124,155, 88,126,113,159, 81,250,136, 83,143,240, 79,202, 92,251,
                                   102, 37,119,172,122,227,130, 28,153,255, 81,198, 95,170,101,236,105,111,107,137,109,243,  0, 32,
                                   110,150,111,100,118,254,125, 20, 93,225,144,117,145,135,152,  6, 81,230, 82, 29, 98, 64,102,145,
                                   102,217,110, 26, 94,182,125,210,127,114,102,248,133,175,133,247,138,248, 82,169, 83,217, 89,115,
                                    94,143, 95,144, 96, 85,146,228,150,100, 80,183, 81, 31, 82,221, 83, 32, 83, 71, 83,236, 84,232,
                                    85, 70, 85, 49, 86, 23, 89,104, 89,190, 90, 60, 91,181, 92,  6, 92, 15, 92, 17, 92, 26, 94,132,
                                    94,138, 94,224, 95,112, 98,127, 98,132, 98,219, 99,140, 99,119,102,  7,102, 12,102, 45,102,118,
                                   103,126,104,162,106, 31,106, 53,108,188,109,136,110,  9,110, 88,113, 60,113, 38,113,103,117,199,
                                   119,  1,120, 93,121,  1,121,101,121,240,122,224,123, 17,124,167,125, 57,128,150,131,214,132,139,
                                   133, 73,136, 93,136,243,138, 31,138, 60,138, 84,138,115,140, 97,140,222,145,164,146,102,147,126,
                                   148, 24,150,156,151,152, 78, 10, 78,  8, 78, 30, 78, 87, 81,151, 82,112, 87,206, 88, 52, 88,204,
                                    91, 34, 94, 56, 96,197,100,254,103, 97,103, 86,109, 68,114,182,117,115,122, 99,132,184,139,114,
                                   145,184,147, 32, 86, 49, 87,244,152,254,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                    98,237,105, 13,107,150,113,237,126, 84,128,119,130,114,137,230,152,223,135, 85,143,177, 92, 59,
                                    79, 56, 79,225, 79,181, 85,  7, 90, 32, 91,221, 91,233, 95,195, 97, 78, 99, 47,101,176,102, 75,
                                   104,238,105,155,109,120,109,241,117, 51,117,185,119, 31,121, 94,121,230,125, 51,129,227,130,175,
                                   133,170,137,170,138, 58,142,171,143,155,144, 50,145,221,151,  7, 78,186, 78,193, 82,  3, 88,117,
                                    88,236, 92, 11,117, 26, 92, 61,129, 78,138, 10,143,197,150, 99,151,109,123, 37,138,207,152,  8,
                                   145, 98, 86,243, 83,168,  0, 32,144, 23, 84, 57, 87,130, 94, 37, 99,168,108, 52,112,138,119, 97,
                                   124,139,127,224,136,112,144, 66,145, 84,147, 16,147, 24,150,143,116, 94,154,196, 93,  7, 93,105,
                                   101,112,103,162,141,168,150,219, 99,110,103, 73,105, 25,131,197,152, 23,150,192,136,254,111,132,
                                   100,122, 91,248, 78, 22,112, 44,117, 93,102, 47, 81,196, 82, 54, 82,226, 89,211, 95,129, 96, 39,
                                    98, 16,101, 63,101,116,102, 31,102,116,104,242,104, 22,107, 99,110,  5,114,114,117, 31,118,219,
                                   124,190,128, 86, 88,240,136,253,137,127,138,160,138,147,138,203,144, 29,145,146,151, 82,151, 89,
                                   101,137,122, 14,129,  6,150,187, 94, 45, 96,220, 98, 26,101,165,102, 20,103,144,119,243,122, 77,
                                   124, 77,126, 62,129, 10,140,172,141,100,141,225,142, 95,120,169, 82,  7, 98,217, 99,165,100, 66,
                                    98,152,138, 45,122,131,123,192,138,172,150,234,125,118,130, 12,135, 73, 78,217, 81, 72, 83, 67,
                                    83, 96, 91,163, 92,  2, 92, 22, 93,221, 98, 38, 98, 71,100,176,104, 19,104, 52,108,201,109, 69,
                                   109, 23,103,211,111, 92,113, 78,113,125,101,203,122,127,123,173,125,218,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,126, 74,127,168,129,122,130, 27,130, 57,133,166,138,110,140,206,
                                   141,245,144,120,144,119,146,173,146,145,149,131,155,174, 82, 77, 85,132,111, 56,113, 54, 81,104,
                                   121,133,126, 85,129,179,124,206, 86, 76, 88, 81, 92,168, 99,170,102,254,102,253,105, 90,114,217,
                                   117,143,117,142,121, 14,121, 86,121,223,124,151,125, 32,125, 68,134,  7,138, 52,150, 59,144, 97,
                                   159, 32, 80,231, 82,117, 83,204, 83,226, 80,  9, 85,170, 88,238, 89, 79,114, 61, 91,139, 92,100,
                                    83, 29, 96,227, 96,243, 99, 92, 99,131, 99, 63, 99,187,  0, 32,100,205,101,233,102,249, 93,227,
                                   105,205,105,253,111, 21,113,229, 78,137,117,233,118,248,122,147,124,223,125,207,125,156,128, 97,
                                   131, 73,131, 88,132,108,132,188,133,251,136,197,141,112,144,  1,144,109,147,151,151, 28,154, 18,
                                    80,207, 88,151, 97,142,129,211,133, 53,141,  8,144, 32, 79,195, 80,116, 82, 71, 83,115, 96,111,
                                    99, 73,103, 95,110, 44,141,179,144, 31, 79,215, 92, 94,140,202,101,207,125,154, 83, 82,136,150,
                                    81,118, 99,195, 91, 88, 91,107, 92, 10,100, 13,103, 81,144, 92, 78,214, 89, 26, 89, 42,108,112,
                                   138, 81, 85, 62, 88, 21, 89,165, 96,240, 98, 83,103,193,130, 53,105, 85,150, 64,153,196,154, 40,
                                    79, 83, 88,  6, 91,254,128, 16, 92,177, 94, 47, 95,133, 96, 32, 97, 75, 98, 52,102,255,108,240,
                                   110,222,128,206,129,127,130,212,136,139,140,184,144,  0,144, 46,150,138,158,219,155,219, 78,227,
                                    83,240, 89, 39,123, 44,145,141,152, 76,157,249,110,221,112, 39, 83, 83, 85, 68, 91,133, 98, 88,
                                    98,158, 98,211,108,162,111,239,116, 34,138, 23,148, 56,111,193,138,254,131, 56, 81,231,134,248,
                                    83,234,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32, 83,233, 79, 70,144, 84,143,176,
                                    89,106,129, 49, 93,253,122,234,143,191,104,218,140, 55,114,248,156, 72,106, 61,138,176, 78, 57,
                                    83, 88, 86,  6, 87,102, 98,197, 99,162,101,230,107, 78,109,225,110, 91,112,173,119,237,122,239,
                                   123,170,125,187,128, 61,128,198,134,203,138,149,147, 91, 86,227, 88,199, 95, 62,101,173,102,150,
                                   106,128,107,181,117, 55,138,199, 80, 36,119,229, 87, 48, 95, 27, 96,101,102,122,108, 96,117,244,
                                   122, 26,127,110,129,244,135, 24,144, 69,153,179,123,201,117, 92,122,249,123, 81,132,196,  0, 32,
                                   144, 16,121,233,122,146,131, 54, 90,225,119, 64, 78, 45, 78,242, 91,153, 95,224, 98,189,102, 60,
                                   103,241,108,232,134,107,136,119,138, 59,145, 78,146,243,153,208,106, 23,112, 38,115, 42,130,231,
                                   132, 87,140,175, 78,  1, 81, 70, 81,203, 85,139, 91,245, 94, 22, 94, 51, 94,129, 95, 20, 95, 53,
                                    95,107, 95,180, 97,242, 99, 17,102,162,103, 29,111,110,114, 82,117, 58,119, 58,128,116,129, 57,
                                   129,120,135,118,138,191,138,220,141,133,141,243,146,154,149,119,152,  2,156,229, 82,197, 99, 87,
                                   118,244,103, 21,108,136,115,205,140,195,147,174,150,115,109, 37, 88,156,105, 14,105,204,143,253,
                                   147,154,117,219,144, 26, 88, 90,104,  2, 99,180,105,251, 79, 67,111, 44,103,216,143,187,133, 38,
                                   125,180,147, 84,105, 63,111,112, 87,106, 88,247, 91, 44,125, 44,114, 42, 84, 10,145,227,157,180,
                                    78,173, 79, 78, 80, 92, 80,117, 82, 67,140,158, 84, 72, 88, 36, 91,154, 94, 29, 94,149, 94,173,
                                    94,247, 95, 31, 96,140, 98,181, 99, 58, 99,208,104,175,108, 64,120,135,121,142,122, 11,125,224,
                                   130, 71,138,  2,138,230,142, 68,144, 19,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                   144,184,145, 45,145,216,159, 14,108,229,100, 88,100,226,101,117,110,244,118,132,123, 27,144,105,
                                   147,209,110,186, 84,242, 95,185,100,164,143, 77,143,237,146, 68, 81,120, 88,107, 89, 41, 92, 85,
                                    94,151,109,251,126,143,117, 28,140,188,142,226,152, 91,112,185, 79, 29,107,191,111,177,117, 48,
                                   150,251, 81, 78, 84, 16, 88, 53, 88, 87, 89,172, 92, 96, 95,146,101,151,103, 92,110, 33,118,123,
                                   131,223,140,237,144, 20,144,253,147, 77,120, 37,120, 58, 82,170, 94,166, 87, 31, 89,116, 96, 18,
                                    80, 18, 81, 90, 81,172,  0, 32, 81,205, 82,  0, 85, 16, 88, 84, 88, 88, 89, 87, 91,149, 92,246,
                                    93,139, 96,188, 98,149,100, 45,103,113,104, 67,104,188,104,223,118,215,109,216,110,111,109,155,
                                   112,111,113,200, 95, 83,117,216,121,119,123, 73,123, 84,123, 82,124,214,125,113, 82, 48,132, 99,
                                   133,105,133,228,138, 14,139,  4,140, 70,142, 15,144,  3,144, 15,148, 25,150,118,152, 45,154, 48,
                                   149,216, 80,205, 82,213, 84, 12, 88,  2, 92, 14, 97,167,100,158,109, 30,119,179,122,229,128,244,
                                   132,  4,144, 83,146,133, 92,224,157,  7, 83, 63, 95,151, 95,179,109,156,114,121,119, 99,121,191,
                                   123,228,107,210,114,236,138,173,104,  3,106, 97, 81,248,122,129,105, 52, 92, 74,156,246,130,235,
                                    91,197,145, 73,112, 30, 86,120, 92,111, 96,199,101,102,108,140,140, 90,144, 65,152, 19, 84, 81,
                                   102,199,146, 13, 89, 72,144,163, 81,133, 78, 77, 81,234,133,153,139, 14,112, 88, 99,122,147, 75,
                                   105, 98,153,180,126,  4,117,119, 83, 87,105, 96,142,223,150,227,108, 93, 78,140, 92, 60, 95, 16,
                                   143,233, 83,  2,140,209,128,137,134,121, 94,255,101,229, 78,115, 81,101,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32, 89,130, 92, 63,151,238, 78,251, 89,138, 95,205,138,141,111,225,
                                   121,176,121, 98, 91,231,132,113,115, 43,113,177, 94,116, 95,245, 99,123,100,154,113,195,124,152,
                                    78, 67, 94,252, 78, 75, 87,220, 86,162, 96,169,111,195,125, 13,128,253,129, 51,129,191,143,178,
                                   137,151,134,164, 93,244, 98,138,100,173,137,135,103,119,108,226,109, 62,116, 54,120, 52, 90, 70,
                                   127,117,130,173,153,172, 79,243, 94,195, 98,221, 99,146,101, 87,103,111,118,195,114, 76,128,204,
                                   128,186,143, 41,145, 77, 80, 13, 87,249, 90,146,104,133,  0, 32,105,115,113,100,114,253,140,183,
                                    88,242,140,224,150,106,144, 25,135,127,121,228,119,231,132, 41, 79, 47, 82,101, 83, 90, 98,205,
                                   103,207,108,202,118,125,123,148,124,149,130, 54,133,132,143,235,102,221,111, 32,114,  6,126, 27,
                                   131,171,153,193,158,166, 81,253,123,177,120,114,123,184,128,135,123, 72,106,232, 94, 97,128,140,
                                   117, 81,117, 96, 81,107,146, 98,110,140,118,122,145,151,154,234, 79, 16,127,112, 98,156,123, 79,
                                   149,165,156,233, 86,122, 88, 89,134,228,150,188, 79, 52, 82, 36, 83, 74, 83,205, 83,219, 94,  6,
                                   100, 44,101,145,103,127,108, 62,108, 78,114, 72,114,175,115,237,117, 84,126, 65,130, 44,133,233,
                                   140,169,123,196,145,198,113,105,152, 18,152,239, 99, 61,102,105,117,106,118,228,120,208,133, 67,
                                   134,238, 83, 42, 83, 81, 84, 38, 89,131, 94,135, 95,124, 96,178, 98, 73, 98,121, 98,171,101,144,
                                   107,212,108,204,117,178,118,174,120,145,121,216,125,203,127,119,128,165,136,171,138,185,140,187,
                                   144,127,151, 94,152,219,106, 11,124, 56, 80,153, 92, 62, 95,174,103,135,107,216,116, 53,119,  9,
                                   127,142,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,159, 59,103,202,122, 23, 83, 57,
                                   117,139,154,237, 95,102,129,157,131,241,128,152, 95, 60, 95,197,117, 98,123, 70,144, 60,104,103,
                                    89,235, 90,155,125, 16,118,126,139, 44, 79,245, 95,106,106, 25,108, 55,111,  2,116,226,121,104,
                                   136,104,138, 85,140,121, 94,223, 99,207,117,197,121,210,130,215,147, 40,146,242,132,156,134,237,
                                   156, 45, 84,193, 95,108,101,140,109, 92,112, 21,140,167,140,211,152, 59,101, 79,116,246, 78, 13,
                                    78,216, 87,224, 89, 43, 90,102, 91,204, 81,168, 94,  3, 94,156, 96, 22, 98,118,101,119,  0, 32,
                                   101,167,102,110,109,110,114, 54,123, 38,129, 80,129,154,130,153,139, 92,140,160,140,230,141,116,
                                   150, 28,150, 68, 79,174,100,171,107,102,130, 30,132, 97,133,106,144,232, 92,  1,105, 83,152,168,
                                   132,122,133, 87, 79, 15, 82,111, 95,169, 94, 69,103, 13,121,143,129,121,137,  7,137,134,109,245,
                                    95, 23, 98, 85,108,184, 78,207,114,105,155,146, 82,  6, 84, 59, 86,116, 88,179, 97,164, 98,110,
                                   113, 26, 89,110,124,137,124,222,125, 27,150,240,101,135,128, 94, 78, 25, 79,117, 81,117, 88, 64,
                                    94, 99, 94,115, 95, 10,103,196, 78, 38,133, 61,149,137,150, 91,124,115,152,  1, 80,251, 88,193,
                                   118, 86,120,167, 82, 37,119,165,133, 17,123,134, 80, 79, 89,  9,114, 71,123,199,125,232,143,186,
                                   143,212,144, 77, 79,191, 82,201, 90, 41, 95,  1,151,173, 79,221,130, 23,146,234, 87,  3, 99, 85,
                                   107,105,117, 43,136,220,143, 20,122, 66, 82,223, 88,147, 97, 85, 98, 10,102,174,107,205,124, 63,
                                   131,233, 80, 35, 79,248, 83,  5, 84, 70, 88, 49, 89, 73, 91,157, 92,240, 92,239, 93, 41, 94,150,
                                    98,177, 99,103,101, 62,101,185,103, 11,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                   108,213,108,225,112,249,120, 50,126, 43,128,222,130,179,132, 12,132,236,135,  2,137, 18,138, 42,
                                   140, 74,144,166,146,210,152,253,156,243,157,108, 78, 79, 78,161, 80,141, 82, 86, 87, 74, 89,168,
                                    94, 61, 95,216, 95,217, 98, 63,102,180,103, 27,103,208,104,210, 81,146,125, 33,128,170,129,168,
                                   139,  0,140,140,140,191,146,126,150, 50, 84, 32,152, 44, 83, 23, 80,213, 83, 92, 88,168,100,178,
                                   103, 52,114,103,119,102,122, 70,145,230, 82,195,108,161,107,134, 88,  0, 94, 76, 89, 84,103, 44,
                                   127,251, 81,225,118,198,  0, 32,100,105,120,232,155, 84,158,187, 87,203, 89,185,102, 39,103,154,
                                   107,206, 84,233,105,217, 94, 85,129,156,103,149,155,170,103,254,156, 82,104, 93, 78,166, 79,227,
                                    83,200, 98,185,103, 43,108,171,143,196, 79,173,126,109,158,191, 78,  7, 97, 98,110,128,111, 43,
                                   133, 19, 84,115,103, 42,155, 69, 93,243,123,149, 92,172, 91,198,135, 28,110, 74,132,209,122, 20,
                                   129,  8, 89,153,124,141,108, 17,119, 32, 82,217, 89, 34,113, 33,114, 95,119,219,151, 39,157, 97,
                                   105, 11, 90,127, 90, 24, 81,165, 84, 13, 84,125,102, 14,118,223,143,247,146,152,156,244, 89,234,
                                   114, 93,110,197, 81, 77,104,201,125,191,125,236,151, 98,158,186,100,120,106, 33,131,  2, 89,132,
                                    91, 95,107,219,115, 27,118,242,125,178,128, 23,132,153, 81, 50,103, 40,158,217,118,238,103, 98,
                                    82,255,153,  5, 92, 36, 98, 59,124,126,140,176, 85, 79, 96,182,125, 11,149,128, 83,  1, 78, 95,
                                    81,182, 89, 28,114, 58,128, 54,145,206, 95, 37,119,226, 83,132, 95,121,125,  4,133,172,138, 51,
                                   142,141,151, 86,103,243,133,174,148, 83, 97,  9, 97,  8,108,185,118, 82,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,138,237,143, 56, 85, 47, 79, 81, 81, 42, 82,199, 83,203, 91,165,
                                    94,125, 96,160, 97,130, 99,214,103,  9,103,218,110,103,109,140,115, 54,115, 55,117, 49,121, 80,
                                   136,213,138,152,144, 74,144,145,144,245,150,196,135,141, 89, 21, 78,136, 79, 89, 78, 14,138,137,
                                   143, 63,152, 16, 80,173, 94,124, 89,150, 91,185, 94,184, 99,218, 99,250,100,193,102,220,105, 74,
                                   105,216,109, 11,110,182,113,148,117, 40,122,175,127,138,128,  0,132, 73,132,201,137,129,139, 33,
                                   142, 10,144,101,150,125,153, 10, 97,126, 98,145,107, 50,  0, 32,108,131,109,116,127,204,127,252,
                                   109,192,127,133,135,186,136,248,103,101,131,177,152, 60,150,247,109, 27,125, 97,132, 61,145,106,
                                    78,113, 83,117, 93, 80,107,  4,111,235,133,205,134, 45,137,167, 82, 41, 84, 15, 92,101,103, 78,
                                   104,168,116,  6,116,131,117,226,136,207,136,225,145,204,150,226,150,120, 95,139,115,135,122,203,
                                   132, 78, 99,160,117,101, 82,137,109, 65,110,156,116,  9,117, 89,120,107,124,146,150,134,122,220,
                                   159,141, 79,182, 97,110,101,197,134, 92, 78,134, 78,174, 80,218, 78, 33, 81,204, 91,238,101,153,
                                   104,129,109,188,115, 31,118, 66,119,173,122, 28,124,231,130,111,138,210,144,124,145,207,150,117,
                                   152, 24, 82,155,125,209, 80, 43, 83,152,103,151,109,203,113,208,116, 51,129,232,143, 42,150,163,
                                   156, 87,158,159,116, 96, 88, 65,109,153,125, 47,152, 94, 78,228, 79, 54, 79,139, 81,183, 82,177,
                                    93,186, 96, 28,115,178,121, 60,130,211,146, 52,150,183,150,246,151, 10,158,151,159, 98,102,166,
                                   107,116, 82, 23, 82,163,112,200,136,194, 94,201, 96, 75, 97,144,111, 35,113, 73,124, 62,125,244,
                                   128,111,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,132,238,144, 35,147, 44, 84, 66,
                                   155,111,106,211,112,137,140,194,141,239,151, 50, 82,180, 90, 65, 94,202, 95,  4,103, 23,105,124,
                                   105,148,109,106,111, 15,114, 98,114,252,123,237,128,  1,128,126,135, 75,144,206, 81,109,158,147,
                                   121,132,128,139,147, 50,138,214, 80, 45, 84,140,138,113,107,106,140,196,129,  7, 96,209,103,160,
                                   157,242, 78,153, 78,152,156, 16,138,107,133,193,133,104,105,  0,110,126,120,151,129, 85,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32, 95, 12, 78, 16, 78, 21, 78, 42, 78, 49,
                                    78, 54, 78, 60, 78, 63, 78, 66, 78, 86, 78, 88, 78,130, 78,133,140,107, 78,138,130, 18, 95, 13,
                                    78,142, 78,158, 78,159, 78,160, 78,162, 78,176, 78,179, 78,182, 78,206, 78,205, 78,196, 78,198,
                                    78,194, 78,215, 78,222, 78,237, 78,223, 78,247, 79,  9, 79, 90, 79, 48, 79, 91, 79, 93, 79, 87,
                                    79, 71, 79,118, 79,136, 79,143, 79,152, 79,123, 79,105, 79,112, 79,145, 79,111, 79,134, 79,150,
                                    81, 24, 79,212, 79,223, 79,206, 79,216, 79,219, 79,209, 79,218, 79,208, 79,228, 79,229, 80, 26,
                                    80, 40, 80, 20, 80, 42, 80, 37, 80,  5, 79, 28, 79,246, 80, 33, 80, 41, 80, 44, 79,254, 79,239,
                                    80, 17, 80,  6, 80, 67, 80, 71,103,  3, 80, 85, 80, 80, 80, 72, 80, 90, 80, 86, 80,108, 80,120,
                                    80,128, 80,154, 80,133, 80,180, 80,178,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                    80,201, 80,202, 80,179, 80,194, 80,214, 80,222, 80,229, 80,237, 80,227, 80,238, 80,249, 80,245,
                                    81,  9, 81,  1, 81,  2, 81, 22, 81, 21, 81, 20, 81, 26, 81, 33, 81, 58, 81, 55, 81, 60, 81, 59,
                                    81, 63, 81, 64, 81, 82, 81, 76, 81, 84, 81, 98,122,248, 81,105, 81,106, 81,110, 81,128, 81,130,
                                    86,216, 81,140, 81,137, 81,143, 81,145, 81,147, 81,149, 81,150, 81,164, 81,166, 81,162, 81,169,
                                    81,170, 81,171, 81,179, 81,177, 81,178, 81,176, 81,181, 81,189, 81,197, 81,201, 81,219, 81,224,
                                   134, 85, 81,233, 81,237,  0, 32, 81,240, 81,245, 81,254, 82,  4, 82, 11, 82, 20, 82, 14, 82, 39,
                                    82, 42, 82, 46, 82, 51, 82, 57, 82, 79, 82, 68, 82, 75, 82, 76, 82, 94, 82, 84, 82,106, 82,116,
                                    82,105, 82,115, 82,127, 82,125, 82,141, 82,148, 82,146, 82,113, 82,136, 82,145,143,168,143,167,
                                    82,172, 82,173, 82,188, 82,181, 82,193, 82,205, 82,215, 82,222, 82,227, 82,230,152,237, 82,224,
                                    82,243, 82,245, 82,248, 82,249, 83,  6, 83,  8,117, 56, 83, 13, 83, 16, 83, 15, 83, 21, 83, 26,
                                    83, 35, 83, 47, 83, 49, 83, 51, 83, 56, 83, 64, 83, 70, 83, 69, 78, 23, 83, 73, 83, 77, 81,214,
                                    83, 94, 83,105, 83,110, 89, 24, 83,123, 83,119, 83,130, 83,150, 83,160, 83,166, 83,165, 83,174,
                                    83,176, 83,182, 83,195,124, 18,150,217, 83,223,102,252,113,238, 83,238, 83,232, 83,237, 83,250,
                                    84,  1, 84, 61, 84, 64, 84, 44, 84, 45, 84, 60, 84, 46, 84, 54, 84, 41, 84, 29, 84, 78, 84,143,
                                    84,117, 84,142, 84, 95, 84,113, 84,119, 84,112, 84,146, 84,123, 84,128, 84,118, 84,132, 84,144,
                                    84,134, 84,199, 84,162, 84,184, 84,165, 84,172, 84,196, 84,200, 84,168,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32, 84,171, 84,194, 84,164, 84,190, 84,188, 84,216, 84,229, 84,230,
                                    85, 15, 85, 20, 84,253, 84,238, 84,237, 84,250, 84,226, 85, 57, 85, 64, 85, 99, 85, 76, 85, 46,
                                    85, 92, 85, 69, 85, 86, 85, 87, 85, 56, 85, 51, 85, 93, 85,153, 85,128, 84,175, 85,138, 85,159,
                                    85,123, 85,126, 85,152, 85,158, 85,174, 85,124, 85,131, 85,169, 85,135, 85,168, 85,218, 85,197,
                                    85,223, 85,196, 85,220, 85,228, 85,212, 86, 20, 85,247, 86, 22, 85,254, 85,253, 86, 27, 85,249,
                                    86, 78, 86, 80,113,223, 86, 52, 86, 54, 86, 50, 86, 56,  0, 32, 86,107, 86,100, 86, 47, 86,108,
                                    86,106, 86,134, 86,128, 86,138, 86,160, 86,148, 86,143, 86,165, 86,174, 86,182, 86,180, 86,194,
                                    86,188, 86,193, 86,195, 86,192, 86,200, 86,206, 86,209, 86,211, 86,215, 86,238, 86,249, 87,  0,
                                    86,255, 87,  4, 87,  9, 87,  8, 87, 11, 87, 13, 87, 19, 87, 24, 87, 22, 85,199, 87, 28, 87, 38,
                                    87, 55, 87, 56, 87, 78, 87, 59, 87, 64, 87, 79, 87,105, 87,192, 87,136, 87, 97, 87,127, 87,137,
                                    87,147, 87,160, 87,179, 87,164, 87,170, 87,176, 87,195, 87,198, 87,212, 87,210, 87,211, 88, 10,
                                    87,214, 87,227, 88, 11, 88, 25, 88, 29, 88,114, 88, 33, 88, 98, 88, 75, 88,112,107,192, 88, 82,
                                    88, 61, 88,121, 88,133, 88,185, 88,159, 88,171, 88,186, 88,222, 88,187, 88,184, 88,174, 88,197,
                                    88,211, 88,209, 88,215, 88,217, 88,216, 88,229, 88,220, 88,228, 88,223, 88,239, 88,250, 88,249,
                                    88,251, 88,252, 88,253, 89,  2, 89, 10, 89, 16, 89, 27,104,166, 89, 37, 89, 44, 89, 45, 89, 50,
                                    89, 56, 89, 62,122,210, 89, 85, 89, 80, 89, 78, 89, 90, 89, 88, 89, 98, 89, 96, 89,103, 89,108,
                                    89,105,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32, 89,120, 89,129, 89,157, 79, 94,
                                    79,171, 89,163, 89,178, 89,198, 89,232, 89,220, 89,141, 89,217, 89,218, 90, 37, 90, 31, 90, 17,
                                    90, 28, 90,  9, 90, 26, 90, 64, 90,108, 90, 73, 90, 53, 90, 54, 90, 98, 90,106, 90,154, 90,188,
                                    90,190, 90,203, 90,194, 90,189, 90,227, 90,215, 90,230, 90,233, 90,214, 90,250, 90,251, 91, 12,
                                    91, 11, 91, 22, 91, 50, 90,208, 91, 42, 91, 54, 91, 62, 91, 67, 91, 69, 91, 64, 91, 81, 91, 85,
                                    91, 90, 91, 91, 91,101, 91,105, 91,112, 91,115, 91,117, 91,120,101,136, 91,122, 91,128,  0, 32,
                                    91,131, 91,166, 91,184, 91,195, 91,199, 91,201, 91,212, 91,208, 91,228, 91,230, 91,226, 91,222,
                                    91,229, 91,235, 91,240, 91,246, 91,243, 92,  5, 92,  7, 92,  8, 92, 13, 92, 19, 92, 32, 92, 34,
                                    92, 40, 92, 56, 92, 57, 92, 65, 92, 70, 92, 78, 92, 83, 92, 80, 92, 79, 91,113, 92,108, 92,110,
                                    78, 98, 92,118, 92,121, 92,140, 92,145, 92,148, 89,155, 92,171, 92,187, 92,182, 92,188, 92,183,
                                    92,197, 92,190, 92,199, 92,217, 92,233, 92,253, 92,250, 92,237, 93,140, 92,234, 93, 11, 93, 21,
                                    93, 23, 93, 92, 93, 31, 93, 27, 93, 17, 93, 20, 93, 34, 93, 26, 93, 25, 93, 24, 93, 76, 93, 82,
                                    93, 78, 93, 75, 93,108, 93,115, 93,118, 93,135, 93,132, 93,130, 93,162, 93,157, 93,172, 93,174,
                                    93,189, 93,144, 93,183, 93,188, 93,201, 93,205, 93,211, 93,210, 93,214, 93,219, 93,235, 93,242,
                                    93,245, 94, 11, 94, 26, 94, 25, 94, 17, 94, 27, 94, 54, 94, 55, 94, 68, 94, 67, 94, 64, 94, 78,
                                    94, 87, 94, 84, 94, 95, 94, 98, 94,100, 94, 71, 94,117, 94,118, 94,122,158,188, 94,127, 94,160,
                                    94,193, 94,194, 94,200, 94,208, 94,207,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                    94,214, 94,227, 94,221, 94,218, 94,219, 94,226, 94,225, 94,232, 94,233, 94,236, 94,241, 94,243,
                                    94,240, 94,244, 94,248, 94,254, 95,  3, 95,  9, 95, 93, 95, 92, 95, 11, 95, 17, 95, 22, 95, 41,
                                    95, 45, 95, 56, 95, 65, 95, 72, 95, 76, 95, 78, 95, 47, 95, 81, 95, 86, 95, 87, 95, 89, 95, 97,
                                    95,109, 95,115, 95,119, 95,131, 95,130, 95,127, 95,138, 95,136, 95,145, 95,135, 95,158, 95,153,
                                    95,152, 95,160, 95,168, 95,173, 95,188, 95,214, 95,251, 95,228, 95,248, 95,241, 95,221, 96,179,
                                    95,255, 96, 33, 96, 96,  0, 32, 96, 25, 96, 16, 96, 41, 96, 14, 96, 49, 96, 27, 96, 21, 96, 43,
                                    96, 38, 96, 15, 96, 58, 96, 90, 96, 65, 96,106, 96,119, 96, 95, 96, 74, 96, 70, 96, 77, 96, 99,
                                    96, 67, 96,100, 96, 66, 96,108, 96,107, 96, 89, 96,129, 96,141, 96,231, 96,131, 96,154, 96,132,
                                    96,155, 96,150, 96,151, 96,146, 96,167, 96,139, 96,225, 96,184, 96,224, 96,211, 96,180, 95,240,
                                    96,189, 96,198, 96,181, 96,216, 97, 77, 97, 21, 97,  6, 96,246, 96,247, 97,  0, 96,244, 96,250,
                                    97,  3, 97, 33, 96,251, 96,241, 97, 13, 97, 14, 97, 71, 97, 62, 97, 40, 97, 39, 97, 74, 97, 63,
                                    97, 60, 97, 44, 97, 52, 97, 61, 97, 66, 97, 68, 97,115, 97,119, 97, 88, 97, 89, 97, 90, 97,107,
                                    97,116, 97,111, 97,101, 97,113, 97, 95, 97, 93, 97, 83, 97,117, 97,153, 97,150, 97,135, 97,172,
                                    97,148, 97,154, 97,138, 97,145, 97,171, 97,174, 97,204, 97,202, 97,201, 97,247, 97,200, 97,195,
                                    97,198, 97,186, 97,203,127,121, 97,205, 97,230, 97,227, 97,246, 97,250, 97,244, 97,255, 97,253,
                                    97,252, 97,254, 98,  0, 98,  8, 98,  9, 98, 13, 98, 12, 98, 20, 98, 27,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32, 98, 30, 98, 33, 98, 42, 98, 46, 98, 48, 98, 50, 98, 51, 98, 65,
                                    98, 78, 98, 94, 98, 99, 98, 91, 98, 96, 98,104, 98,124, 98,130, 98,137, 98,126, 98,146, 98,147,
                                    98,150, 98,212, 98,131, 98,148, 98,215, 98,209, 98,187, 98,207, 98,255, 98,198,100,212, 98,200,
                                    98,220, 98,204, 98,202, 98,194, 98,199, 98,155, 98,201, 99, 12, 98,238, 98,241, 99, 39, 99,  2,
                                    99,  8, 98,239, 98,245, 99, 80, 99, 62, 99, 77,100, 28, 99, 79, 99,150, 99,142, 99,128, 99,171,
                                    99,118, 99,163, 99,143, 99,137, 99,159, 99,181, 99,107,  0, 32, 99,105, 99,190, 99,233, 99,192,
                                    99,198, 99,227, 99,201, 99,210, 99,246, 99,196,100, 22,100, 52,100,  6,100, 19,100, 38,100, 54,
                                   101, 29,100, 23,100, 40,100, 15,100,103,100,111,100,118,100, 78,101, 42,100,149,100,147,100,165,
                                   100,169,100,136,100,188,100,218,100,210,100,197,100,199,100,187,100,216,100,194,100,241,100,231,
                                   130,  9,100,224,100,225, 98,172,100,227,100,239,101, 44,100,246,100,244,100,242,100,250,101,  0,
                                   100,253,101, 24,101, 28,101,  5,101, 36,101, 35,101, 43,101, 52,101, 53,101, 55,101, 54,101, 56,
                                   117, 75,101, 72,101, 86,101, 85,101, 77,101, 88,101, 94,101, 93,101,114,101,120,101,130,101,131,
                                   139,138,101,155,101,159,101,171,101,183,101,195,101,198,101,193,101,196,101,204,101,210,101,219,
                                   101,217,101,224,101,225,101,241,103,114,102, 10,102,  3,101,251,103,115,102, 53,102, 54,102, 52,
                                   102, 28,102, 79,102, 68,102, 73,102, 65,102, 94,102, 93,102,100,102,103,102,104,102, 95,102, 98,
                                   102,112,102,131,102,136,102,142,102,137,102,132,102,152,102,157,102,193,102,185,102,201,102,190,
                                   102,188,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,102,196,102,184,102,214,102,218,
                                   102,224,102, 63,102,230,102,233,102,240,102,245,102,247,103, 15,103, 22,103, 30,103, 38,103, 39,
                                   151, 56,103, 46,103, 63,103, 54,103, 65,103, 56,103, 55,103, 70,103, 94,103, 96,103, 89,103, 99,
                                   103,100,103,137,103,112,103,169,103,124,103,106,103,140,103,139,103,166,103,161,103,133,103,183,
                                   103,239,103,180,103,236,103,179,103,233,103,184,103,228,103,222,103,221,103,226,103,238,103,185,
                                   103,206,103,198,103,231,106,156,104, 30,104, 70,104, 41,104, 64,104, 77,104, 50,104, 78,  0, 32,
                                   104,179,104, 43,104, 89,104, 99,104,119,104,127,104,159,104,143,104,173,104,148,104,157,104,155,
                                   104,131,106,174,104,185,104,116,104,181,104,160,104,186,105, 15,104,141,104,126,105,  1,104,202,
                                   105,  8,104,216,105, 34,105, 38,104,225,105, 12,104,205,104,212,104,231,104,213,105, 54,105, 18,
                                   105,  4,104,215,104,227,105, 37,104,249,104,224,104,239,105, 40,105, 42,105, 26,105, 35,105, 33,
                                   104,198,105,121,105,119,105, 92,105,120,105,107,105, 84,105,126,105,110,105, 57,105,116,105, 61,
                                   105, 89,105, 48,105, 97,105, 94,105, 93,105,129,105,106,105,178,105,174,105,208,105,191,105,193,
                                   105,211,105,190,105,206, 91,232,105,202,105,221,105,187,105,195,105,167,106, 46,105,145,105,160,
                                   105,156,105,149,105,180,105,222,105,232,106,  2,106, 27,105,255,107, 10,105,249,105,242,105,231,
                                   106,  5,105,177,106, 30,105,237,106, 20,105,235,106, 10,106, 18,106,193,106, 35,106, 19,106, 68,
                                   106, 12,106,114,106, 54,106,120,106, 71,106, 98,106, 89,106,102,106, 72,106, 56,106, 34,106,144,
                                   106,141,106,160,106,132,106,162,106,163,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                   106,151,134, 23,106,187,106,195,106,194,106,184,106,179,106,172,106,222,106,209,106,223,106,170,
                                   106,218,106,234,106,251,107,  5,134, 22,106,250,107, 18,107, 22,155, 49,107, 31,107, 56,107, 55,
                                   118,220,107, 57,152,238,107, 71,107, 67,107, 73,107, 80,107, 89,107, 84,107, 91,107, 95,107, 97,
                                   107,120,107,121,107,127,107,128,107,132,107,131,107,141,107,152,107,149,107,158,107,164,107,170,
                                   107,171,107,175,107,178,107,177,107,179,107,183,107,188,107,198,107,203,107,211,107,223,107,236,
                                   107,235,107,243,107,239,  0, 32,158,190,108,  8,108, 19,108, 20,108, 27,108, 36,108, 35,108, 94,
                                   108, 85,108, 98,108,106,108,130,108,141,108,154,108,129,108,155,108,126,108,104,108,115,108,146,
                                   108,144,108,196,108,241,108,211,108,189,108,215,108,197,108,221,108,174,108,177,108,190,108,186,
                                   108,219,108,239,108,217,108,234,109, 31,136, 77,109, 54,109, 43,109, 61,109, 56,109, 25,109, 53,
                                   109, 51,109, 18,109, 12,109, 99,109,147,109,100,109, 90,109,121,109, 89,109,142,109,149,111,228,
                                   109,133,109,249,110, 21,110, 10,109,181,109,199,109,230,109,184,109,198,109,236,109,222,109,204,
                                   109,232,109,210,109,197,109,250,109,217,109,228,109,213,109,234,109,238,110, 45,110,110,110, 46,
                                   110, 25,110,114,110, 95,110, 62,110, 35,110,107,110, 43,110,118,110, 77,110, 31,110, 67,110, 58,
                                   110, 78,110, 36,110,255,110, 29,110, 56,110,130,110,170,110,152,110,201,110,183,110,211,110,189,
                                   110,175,110,196,110,178,110,212,110,213,110,143,110,165,110,194,110,159,111, 65,111, 17,112, 76,
                                   110,236,110,248,110,254,111, 63,110,242,111, 49,110,239,111, 50,110,204,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,111, 62,111, 19,110,247,111,134,111,122,111,120,111,129,111,128,
                                   111,111,111, 91,111,243,111,109,111,130,111,124,111, 88,111,142,111,145,111,194,111,102,111,179,
                                   111,163,111,161,111,164,111,185,111,198,111,170,111,223,111,213,111,236,111,212,111,216,111,241,
                                   111,238,111,219,112,  9,112, 11,111,250,112, 17,112,  1,112, 15,111,254,112, 27,112, 26,111,116,
                                   112, 29,112, 24,112, 31,112, 48,112, 62,112, 50,112, 81,112, 99,112,153,112,146,112,175,112,241,
                                   112,172,112,184,112,179,112,174,112,223,112,203,112,221,  0, 32,112,217,113,  9,112,253,113, 28,
                                   113, 25,113,101,113, 85,113,136,113,102,113, 98,113, 76,113, 86,113,108,113,143,113,251,113,132,
                                   113,149,113,168,113,172,113,215,113,185,113,190,113,210,113,201,113,212,113,206,113,224,113,236,
                                   113,231,113,245,113,252,113,249,113,255,114, 13,114, 16,114, 27,114, 40,114, 45,114, 44,114, 48,
                                   114, 50,114, 59,114, 60,114, 63,114, 64,114, 70,114, 75,114, 88,114,116,114,126,114,130,114,129,
                                   114,135,114,146,114,150,114,162,114,167,114,185,114,178,114,195,114,198,114,196,114,206,114,210,
                                   114,226,114,224,114,225,114,249,114,247, 80, 15,115, 23,115, 10,115, 28,115, 22,115, 29,115, 52,
                                   115, 47,115, 41,115, 37,115, 62,115, 78,115, 79,158,216,115, 87,115,106,115,104,115,112,115,120,
                                   115,117,115,123,115,122,115,200,115,179,115,206,115,187,115,192,115,229,115,238,115,222,116,162,
                                   116,  5,116,111,116, 37,115,248,116, 50,116, 58,116, 85,116, 63,116, 95,116, 89,116, 65,116, 92,
                                   116,105,116,112,116, 99,116,106,116,118,116,126,116,139,116,158,116,167,116,202,116,207,116,212,
                                   115,241,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,116,224,116,227,116,231,116,233,
                                   116,238,116,242,116,240,116,241,116,248,116,247,117,  4,117,  3,117,  5,117, 12,117, 14,117, 13,
                                   117, 21,117, 19,117, 30,117, 38,117, 44,117, 60,117, 68,117, 77,117, 74,117, 73,117, 91,117, 70,
                                   117, 90,117,105,117,100,117,103,117,107,117,109,117,120,117,118,117,134,117,135,117,116,117,138,
                                   117,137,117,130,117,148,117,154,117,157,117,165,117,163,117,194,117,179,117,195,117,181,117,189,
                                   117,184,117,188,117,177,117,205,117,202,117,210,117,217,117,227,117,222,117,254,117,255,  0, 32,
                                   117,252,118,  1,117,240,117,250,117,242,117,243,118, 11,118, 13,118,  9,118, 31,118, 39,118, 32,
                                   118, 33,118, 34,118, 36,118, 52,118, 48,118, 59,118, 71,118, 72,118, 70,118, 92,118, 88,118, 97,
                                   118, 98,118,104,118,105,118,106,118,103,118,108,118,112,118,114,118,118,118,120,118,124,118,128,
                                   118,131,118,136,118,139,118,142,118,150,118,147,118,153,118,154,118,176,118,180,118,184,118,185,
                                   118,186,118,194,118,205,118,214,118,210,118,222,118,225,118,229,118,231,118,234,134, 47,118,251,
                                   119,  8,119,  7,119,  4,119, 41,119, 36,119, 30,119, 37,119, 38,119, 27,119, 55,119, 56,119, 71,
                                   119, 90,119,104,119,107,119, 91,119,101,119,127,119,126,119,121,119,142,119,139,119,145,119,160,
                                   119,158,119,176,119,182,119,185,119,191,119,188,119,189,119,187,119,199,119,205,119,215,119,218,
                                   119,220,119,227,119,238,119,252,120, 12,120, 18,121, 38,120, 32,121, 42,120, 69,120,142,120,116,
                                   120,134,120,124,120,154,120,140,120,163,120,181,120,170,120,175,120,209,120,198,120,203,120,212,
                                   120,190,120,188,120,197,120,202,120,236,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                   120,231,120,218,120,253,120,244,121,  7,121, 18,121, 17,121, 25,121, 44,121, 43,121, 64,121, 96,
                                   121, 87,121, 95,121, 90,121, 85,121, 83,121,122,121,127,121,138,121,157,121,167,159, 75,121,170,
                                   121,174,121,179,121,185,121,186,121,201,121,213,121,231,121,236,121,225,121,227,122,  8,122, 13,
                                   122, 24,122, 25,122, 32,122, 31,121,128,122, 49,122, 59,122, 62,122, 55,122, 67,122, 87,122, 73,
                                   122, 97,122, 98,122,105,159,157,122,112,122,121,122,125,122,136,122,151,122,149,122,152,122,150,
                                   122,169,122,200,122,176,  0, 32,122,182,122,197,122,196,122,191,144,131,122,199,122,202,122,205,
                                   122,207,122,213,122,211,122,217,122,218,122,221,122,225,122,226,122,230,122,237,122,240,123,  2,
                                   123, 15,123, 10,123,  6,123, 51,123, 24,123, 25,123, 30,123, 53,123, 40,123, 54,123, 80,123,122,
                                   123,  4,123, 77,123, 11,123, 76,123, 69,123,117,123,101,123,116,123,103,123,112,123,113,123,108,
                                   123,110,123,157,123,152,123,159,123,141,123,156,123,154,123,139,123,146,123,143,123, 93,123,153,
                                   123,203,123,193,123,204,123,207,123,180,123,198,123,221,123,233,124, 17,124, 20,123,230,123,229,
                                   124, 96,124,  0,124,  7,124, 19,123,243,123,247,124, 23,124, 13,123,246,124, 35,124, 39,124, 42,
                                   124, 31,124, 55,124, 43,124, 61,124, 76,124, 67,124, 84,124, 79,124, 64,124, 80,124, 88,124, 95,
                                   124,100,124, 86,124,101,124,108,124,117,124,131,124,144,124,164,124,173,124,162,124,171,124,161,
                                   124,168,124,179,124,178,124,177,124,174,124,185,124,189,124,192,124,197,124,194,124,216,124,210,
                                   124,220,124,226,155, 59,124,239,124,242,124,244,124,246,124,250,125,  6,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,125,  2,125, 28,125, 21,125, 10,125, 69,125, 75,125, 46,125, 50,
                                   125, 63,125, 53,125, 70,125,115,125, 86,125, 78,125,114,125,104,125,110,125, 79,125, 99,125,147,
                                   125,137,125, 91,125,143,125,125,125,155,125,186,125,174,125,163,125,181,125,199,125,189,125,171,
                                   126, 61,125,162,125,175,125,220,125,184,125,159,125,176,125,216,125,221,125,228,125,222,125,251,
                                   125,242,125,225,126,  5,126, 10,126, 35,126, 33,126, 18,126, 49,126, 31,126,  9,126, 11,126, 34,
                                   126, 70,126,102,126, 59,126, 53,126, 57,126, 67,126, 55,  0, 32,126, 50,126, 58,126,103,126, 93,
                                   126, 86,126, 94,126, 89,126, 90,126,121,126,106,126,105,126,124,126,123,126,131,125,213,126,125,
                                   143,174,126,127,126,136,126,137,126,140,126,146,126,144,126,147,126,148,126,150,126,142,126,155,
                                   126,156,127, 56,127, 58,127, 69,127, 76,127, 77,127, 78,127, 80,127, 81,127, 85,127, 84,127, 88,
                                   127, 95,127, 96,127,104,127,105,127,103,127,120,127,130,127,134,127,131,127,136,127,135,127,140,
                                   127,148,127,158,127,157,127,154,127,163,127,175,127,178,127,185,127,174,127,182,127,184,139,113,
                                   127,197,127,198,127,202,127,213,127,212,127,225,127,230,127,233,127,243,127,249,152,220,128,  6,
                                   128,  4,128, 11,128, 18,128, 24,128, 25,128, 28,128, 33,128, 40,128, 63,128, 59,128, 74,128, 70,
                                   128, 82,128, 88,128, 90,128, 95,128, 98,128,104,128,115,128,114,128,112,128,118,128,121,128,125,
                                   128,127,128,132,128,134,128,133,128,155,128,147,128,154,128,173, 81,144,128,172,128,219,128,229,
                                   128,217,128,221,128,196,128,218,128,214,129,  9,128,239,128,241,129, 27,129, 41,129, 35,129, 47,
                                   129, 75,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,150,139,129, 70,129, 62,129, 83,
                                   129, 81,128,252,129,113,129,110,129,101,129,102,129,116,129,131,129,136,129,138,129,128,129,130,
                                   129,160,129,149,129,164,129,163,129, 95,129,147,129,169,129,176,129,181,129,190,129,184,129,189,
                                   129,192,129,194,129,186,129,201,129,205,129,209,129,217,129,216,129,200,129,218,129,223,129,224,
                                   129,231,129,250,129,251,129,254,130,  1,130,  2,130,  5,130,  7,130, 10,130, 13,130, 16,130, 22,
                                   130, 41,130, 43,130, 56,130, 51,130, 64,130, 89,130, 88,130, 93,130, 90,130, 95,130,100,  0, 32,
                                   130, 98,130,104,130,106,130,107,130, 46,130,113,130,119,130,120,130,126,130,141,130,146,130,171,
                                   130,159,130,187,130,172,130,225,130,227,130,223,130,210,130,244,130,243,130,250,131,147,131,  3,
                                   130,251,130,249,130,222,131,  6,130,220,131,  9,130,217,131, 53,131, 52,131, 22,131, 50,131, 49,
                                   131, 64,131, 57,131, 80,131, 69,131, 47,131, 43,131, 23,131, 24,131,133,131,154,131,170,131,159,
                                   131,162,131,150,131, 35,131,142,131,135,131,138,131,124,131,181,131,115,131,117,131,160,131,137,
                                   131,168,131,244,132, 19,131,235,131,206,131,253,132,  3,131,216,132, 11,131,193,131,247,132,  7,
                                   131,224,131,242,132, 13,132, 34,132, 32,131,189,132, 56,133,  6,131,251,132,109,132, 42,132, 60,
                                   133, 90,132,132,132,119,132,107,132,173,132,110,132,130,132,105,132, 70,132, 44,132,111,132,121,
                                   132, 53,132,202,132, 98,132,185,132,191,132,159,132,217,132,205,132,187,132,218,132,208,132,193,
                                   132,198,132,214,132,161,133, 33,132,255,132,244,133, 23,133, 24,133, 44,133, 31,133, 21,133, 20,
                                   132,252,133, 64,133, 99,133, 88,133, 72,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                   133, 65,134,  2,133, 75,133, 85,133,128,133,164,133,136,133,145,133,138,133,168,133,109,133,148,
                                   133,155,133,234,133,135,133,156,133,119,133,126,133,144,133,201,133,186,133,207,133,185,133,208,
                                   133,213,133,221,133,229,133,220,133,249,134, 10,134, 19,134, 11,133,254,133,250,134,  6,134, 34,
                                   134, 26,134, 48,134, 63,134, 77, 78, 85,134, 84,134, 95,134,103,134,113,134,147,134,163,134,169,
                                   134,170,134,139,134,140,134,182,134,175,134,196,134,198,134,176,134,201,136, 35,134,171,134,212,
                                   134,222,134,233,134,236,  0, 32,134,223,134,219,134,239,135, 18,135,  6,135,  8,135,  0,135,  3,
                                   134,251,135, 17,135,  9,135, 13,134,249,135, 10,135, 52,135, 63,135, 55,135, 59,135, 37,135, 41,
                                   135, 26,135, 96,135, 95,135,120,135, 76,135, 78,135,116,135, 87,135,104,135,110,135, 89,135, 83,
                                   135, 99,135,106,136,  5,135,162,135,159,135,130,135,175,135,203,135,189,135,192,135,208,150,214,
                                   135,171,135,196,135,179,135,199,135,198,135,187,135,239,135,242,135,224,136, 15,136, 13,135,254,
                                   135,246,135,247,136, 14,135,210,136, 17,136, 22,136, 21,136, 34,136, 33,136, 49,136, 54,136, 57,
                                   136, 39,136, 59,136, 68,136, 66,136, 82,136, 89,136, 94,136, 98,136,107,136,129,136,126,136,158,
                                   136,117,136,125,136,181,136,114,136,130,136,151,136,146,136,174,136,153,136,162,136,141,136,164,
                                   136,176,136,191,136,177,136,195,136,196,136,212,136,216,136,217,136,221,136,249,137,  2,136,252,
                                   136,244,136,232,136,242,137,  4,137, 12,137, 10,137, 19,137, 67,137, 30,137, 37,137, 42,137, 43,
                                   137, 65,137, 68,137, 59,137, 54,137, 56,137, 76,137, 29,137, 96,137, 94,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,137,102,137,100,137,109,137,106,137,111,137,116,137,119,137,126,
                                   137,131,137,136,137,138,137,147,137,152,137,161,137,169,137,166,137,172,137,175,137,178,137,186,
                                   137,189,137,191,137,192,137,218,137,220,137,221,137,231,137,244,137,248,138,  3,138, 22,138, 16,
                                   138, 12,138, 27,138, 29,138, 37,138, 54,138, 65,138, 91,138, 82,138, 70,138, 72,138,124,138,109,
                                   138,108,138, 98,138,133,138,130,138,132,138,168,138,161,138,145,138,165,138,166,138,154,138,163,
                                   138,196,138,205,138,194,138,218,138,235,138,243,138,231,  0, 32,138,228,138,241,139, 20,138,224,
                                   138,226,138,247,138,222,138,219,139, 12,139,  7,139, 26,138,225,139, 22,139, 16,139, 23,139, 32,
                                   139, 51,151,171,139, 38,139, 43,139, 62,139, 40,139, 65,139, 76,139, 79,139, 78,139, 73,139, 86,
                                   139, 91,139, 90,139,107,139, 95,139,108,139,111,139,116,139,125,139,128,139,140,139,142,139,146,
                                   139,147,139,150,139,153,139,154,140, 58,140, 65,140, 63,140, 72,140, 76,140, 78,140, 80,140, 85,
                                   140, 98,140,108,140,120,140,122,140,130,140,137,140,133,140,138,140,141,140,142,140,148,140,124,
                                   140,152, 98, 29,140,173,140,170,140,189,140,178,140,179,140,174,140,182,140,200,140,193,140,228,
                                   140,227,140,218,140,253,140,250,140,251,141,  4,141,  5,141, 10,141,  7,141, 15,141, 13,141, 16,
                                   159, 78,141, 19,140,205,141, 20,141, 22,141,103,141,109,141,113,141,115,141,129,141,153,141,194,
                                   141,190,141,186,141,207,141,218,141,214,141,204,141,219,141,203,141,234,141,235,141,223,141,227,
                                   141,252,142,  8,142,  9,141,255,142, 29,142, 30,142, 16,142, 31,142, 66,142, 53,142, 48,142, 52,
                                   142, 74,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,142, 71,142, 73,142, 76,142, 80,
                                   142, 72,142, 89,142,100,142, 96,142, 42,142, 99,142, 85,142,118,142,114,142,124,142,129,142,135,
                                   142,133,142,132,142,139,142,138,142,147,142,145,142,148,142,153,142,170,142,161,142,172,142,176,
                                   142,198,142,177,142,190,142,197,142,200,142,203,142,219,142,227,142,252,142,251,142,235,142,254,
                                   143, 10,143,  5,143, 21,143, 18,143, 25,143, 19,143, 28,143, 31,143, 27,143, 12,143, 38,143, 51,
                                   143, 59,143, 57,143, 69,143, 66,143, 62,143, 76,143, 73,143, 70,143, 78,143, 87,143, 92,  0, 32,
                                   143, 98,143, 99,143,100,143,156,143,159,143,163,143,173,143,175,143,183,143,218,143,229,143,226,
                                   143,234,143,239,144,135,143,244,144,  5,143,249,143,250,144, 17,144, 21,144, 33,144, 13,144, 30,
                                   144, 22,144, 11,144, 39,144, 54,144, 53,144, 57,143,248,144, 79,144, 80,144, 81,144, 82,144, 14,
                                   144, 73,144, 62,144, 86,144, 88,144, 94,144,104,144,111,144,118,150,168,144,114,144,130,144,125,
                                   144,129,144,128,144,138,144,137,144,143,144,168,144,175,144,177,144,181,144,226,144,228, 98, 72,
                                   144,219,145,  2,145, 18,145, 25,145, 50,145, 48,145, 74,145, 86,145, 88,145, 99,145,101,145,105,
                                   145,115,145,114,145,139,145,137,145,130,145,162,145,171,145,175,145,170,145,181,145,180,145,186,
                                   145,192,145,193,145,201,145,203,145,208,145,214,145,223,145,225,145,219,145,252,145,245,145,246,
                                   146, 30,145,255,146, 20,146, 44,146, 21,146, 17,146, 94,146, 87,146, 69,146, 73,146,100,146, 72,
                                   146,149,146, 63,146, 75,146, 80,146,156,146,150,146,147,146,155,146, 90,146,207,146,185,146,183,
                                   146,233,147, 15,146,250,147, 68,147, 46,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                   147, 25,147, 34,147, 26,147, 35,147, 58,147, 53,147, 59,147, 92,147, 96,147,124,147,110,147, 86,
                                   147,176,147,172,147,173,147,148,147,185,147,214,147,215,147,232,147,229,147,216,147,195,147,221,
                                   147,208,147,200,147,228,148, 26,148, 20,148, 19,148,  3,148,  7,148, 16,148, 54,148, 43,148, 53,
                                   148, 33,148, 58,148, 65,148, 82,148, 68,148, 91,148, 96,148, 98,148, 94,148,106,146, 41,148,112,
                                   148,117,148,119,148,125,148, 90,148,124,148,126,148,129,148,127,149,130,149,135,149,138,149,148,
                                   149,150,149,152,149,153,  0, 32,149,160,149,168,149,167,149,173,149,188,149,187,149,185,149,190,
                                   149,202,111,246,149,195,149,205,149,204,149,213,149,212,149,214,149,220,149,225,149,229,149,226,
                                   150, 33,150, 40,150, 46,150, 47,150, 66,150, 76,150, 79,150, 75,150,119,150, 92,150, 94,150, 93,
                                   150, 95,150,102,150,114,150,108,150,141,150,152,150,149,150,151,150,170,150,167,150,177,150,178,
                                   150,176,150,180,150,182,150,184,150,185,150,206,150,203,150,201,150,205,137, 77,150,220,151, 13,
                                   150,213,150,249,151,  4,151,  6,151,  8,151, 19,151, 14,151, 17,151, 15,151, 22,151, 25,151, 36,
                                   151, 42,151, 48,151, 57,151, 61,151, 62,151, 68,151, 70,151, 72,151, 66,151, 73,151, 92,151, 96,
                                   151,100,151,102,151,104, 82,210,151,107,151,113,151,121,151,133,151,124,151,129,151,122,151,134,
                                   151,139,151,143,151,144,151,156,151,168,151,166,151,163,151,179,151,180,151,195,151,198,151,200,
                                   151,203,151,220,151,237,159, 79,151,242,122,223,151,246,151,245,152, 15,152, 12,152, 56,152, 36,
                                   152, 33,152, 55,152, 61,152, 70,152, 79,152, 75,152,107,152,111,152,112,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,152,113,152,116,152,115,152,170,152,175,152,177,152,182,152,196,
                                   152,195,152,198,152,233,152,235,153,  3,153,  9,153, 18,153, 20,153, 24,153, 33,153, 29,153, 30,
                                   153, 36,153, 32,153, 44,153, 46,153, 61,153, 62,153, 66,153, 73,153, 69,153, 80,153, 75,153, 81,
                                   153, 82,153, 76,153, 85,153,151,153,152,153,165,153,173,153,174,153,188,153,223,153,219,153,221,
                                   153,216,153,209,153,237,153,238,153,241,153,242,153,251,153,248,154,  1,154, 15,154,  5,153,226,
                                   154, 25,154, 43,154, 55,154, 69,154, 66,154, 64,154, 67,  0, 32,154, 62,154, 85,154, 77,154, 91,
                                   154, 87,154, 95,154, 98,154,101,154,100,154,105,154,107,154,106,154,173,154,176,154,188,154,192,
                                   154,207,154,209,154,211,154,212,154,222,154,223,154,226,154,227,154,230,154,239,154,235,154,238,
                                   154,244,154,241,154,247,154,251,155,  6,155, 24,155, 26,155, 31,155, 34,155, 35,155, 37,155, 39,
                                   155, 40,155, 41,155, 42,155, 46,155, 47,155, 50,155, 68,155, 67,155, 79,155, 77,155, 78,155, 81,
                                   155, 88,155,116,155,147,155,131,155,145,155,150,155,151,155,159,155,160,155,168,155,180,155,192,
                                   155,202,155,185,155,198,155,207,155,209,155,210,155,227,155,226,155,228,155,212,155,225,156, 58,
                                   155,242,155,241,155,240,156, 21,156, 20,156,  9,156, 19,156, 12,156,  6,156,  8,156, 18,156, 10,
                                   156,  4,156, 46,156, 27,156, 37,156, 36,156, 33,156, 48,156, 71,156, 50,156, 70,156, 62,156, 90,
                                   156, 96,156,103,156,118,156,120,156,231,156,236,156,240,157,  9,157,  8,156,235,157,  3,157,  6,
                                   157, 42,157, 38,157,175,157, 35,157, 31,157, 68,157, 21,157, 18,157, 65,157, 63,157, 62,157, 70,
                                   157, 72,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,157, 93,157, 94,157,100,157, 81,
                                   157, 80,157, 89,157,114,157,137,157,135,157,171,157,111,157,122,157,154,157,164,157,169,157,178,
                                   157,196,157,193,157,187,157,184,157,186,157,198,157,207,157,194,157,217,157,211,157,248,157,230,
                                   157,237,157,239,157,253,158, 26,158, 27,158, 30,158,117,158,121,158,125,158,129,158,136,158,139,
                                   158,140,158,146,158,149,158,145,158,157,158,165,158,169,158,184,158,170,158,173,151, 97,158,204,
                                   158,206,158,207,158,208,158,212,158,220,158,222,158,221,158,224,158,229,158,232,158,239,  0, 32,
                                   158,244,158,246,158,247,158,249,158,251,158,252,158,253,159,  7,159,  8,118,183,159, 21,159, 33,
                                   159, 44,159, 62,159, 74,159, 82,159, 84,159, 99,159, 95,159, 96,159, 97,159,102,159,103,159,108,
                                   159,106,159,119,159,114,159,118,159,149,159,156,159,160, 88, 47,105,199,144, 89,116,100, 81,220,
                                   113,153,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,
                                     0, 32,  0, 32,  0, 32,  0, 32 };

void ShiftJis2UTF8(const unsigned char *pccInput, unsigned char *pucOutput, int outputLength)
{
    int idxIn = 0, idxOut = 0;

    while (pccInput[idxIn] != 0 && idxOut < outputLength)
    {
        char arraySection = pccInput[idxIn] >> 4;

        int arrayOffset = 0;
        if (arraySection == 0x8)
            arrayOffset = 0x100; //these are two-byte shiftjis
        else if(arraySection == 0x9)
            arrayOffset = 0x1100;
        else if(arraySection == 0xE)
            arrayOffset = 0x2100;

        //determining real array offset
        if (arrayOffset)
        {
            arrayOffset += (pccInput[idxIn] & 0xf) << 8;
            idxIn++;
            if (pccInput[idxIn] == 0)
                break;
        }
        arrayOffset += pccInput[idxIn++];
        arrayOffset <<= 1;

        //unicode number is...
        uint16_t unicodeValue = (pucTable[arrayOffset] << 8) | pucTable[arrayOffset + 1];

        //converting to UTF8
        if (unicodeValue < 0x80)
        {
            pucOutput[idxOut++] = unicodeValue;
        }
        else if (unicodeValue < 0x800)
        {
            pucOutput[idxOut++] = 0xC0 | (unicodeValue >> 6);
            pucOutput[idxOut++] = 0x80 | (unicodeValue & 0x3f);
        }
        else
        {
            pucOutput[idxOut++] = 0xE0 | (unicodeValue >> 12);
            pucOutput[idxOut++] = 0x80 | ((unicodeValue & 0xfff) >> 6);
            pucOutput[idxOut++] = 0x80 | (unicodeValue & 0x3f);
        }
    }

    if (idxOut < outputLength)
        pucOutput[idxOut] = 0;
    else
        pucOutput[outputLength-1] = 0;
}

