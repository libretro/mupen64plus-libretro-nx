/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - clock_libretro.h                                        *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2026 davey                                              *
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

#ifndef M64P_BACKENDS_CLOCK_LIBRETRO_H
#define M64P_BACKENDS_CLOCK_LIBRETRO_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <libretro.h>

#include "backends/api/clock_backend.h"

struct clock_libretro
{
    uint32_t active;     /* the frontend's calendar clock drives the RTCs */
    int64_t sec;         /* emulated clock, UTC seconds */
    int32_t utc_offset;  /* seconds east of UTC */
    uint32_t vi_count;   /* VIs since sec last advanced */
    int64_t host_offset; /* frontend time minus sec at the drift reference; not saved */
    uint32_t host_offset_valid;
    int64_t last_drift;  /* correction added at the last advance, 0 if none; not saved */
};

extern struct clock_libretro g_clock_libretro;
extern const struct clock_backend_interface g_iclock_libretro;

bool clock_libretro_query(retro_environment_t cb);
bool clock_libretro_start(struct clock_libretro* clock);
void clock_libretro_vi(struct clock_libretro* clock, unsigned int vi_rate);
void clock_libretro_restore(struct clock_libretro* clock, int64_t sec, int32_t utc_offset, uint32_t vi_count);
bool clock_libretro_resync(struct clock_libretro* clock);

#endif
