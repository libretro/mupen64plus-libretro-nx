/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - clock_libretro.c                                        *
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

#include "clock_libretro.h"

#include <string.h>

#define CLOCK_LIBRETRO_MAX_DRIFT 2
#define CLOCK_LIBRETRO_MAX_HOLD 60

struct clock_libretro g_clock_libretro;

static struct retro_calendar_time_interface calendar_time;

static bool clock_libretro_far(int64_t delta)
{
    return delta >= CLOCK_LIBRETRO_MAX_DRIFT || delta <= -CLOCK_LIBRETRO_MAX_DRIFT;
}

bool clock_libretro_query(retro_environment_t cb)
{
    memset(&calendar_time, 0, sizeof(calendar_time));
    calendar_time.interface_version = RETRO_CALENDAR_TIME_INTERFACE_VERSION;

    if (cb(RETRO_ENVIRONMENT_GET_CALENDAR_TIME_INTERFACE, &calendar_time)
            && calendar_time.interface_version != 0
            && calendar_time.get_start_time != NULL
            && calendar_time.get_current_time != NULL)
        return true;

    memset(&calendar_time, 0, sizeof(calendar_time));
    return false;
}

bool clock_libretro_start(struct clock_libretro* clock)
{
    struct retro_calendar_time t;

    if (calendar_time.get_start_time == NULL || !calendar_time.get_start_time(&t))
        return false;

    clock->active = 1;
    clock->sec = t.sec;
    clock->utc_offset = t.utc_offset;
    clock->vi_count = 0;
    clock->host_offset = 0;
    clock->host_offset_valid = 1;
    clock->last_drift = 0;
    return true;
}

void clock_libretro_vi(struct clock_libretro* clock, unsigned int vi_rate)
{
    struct retro_calendar_time now;
    int64_t offset;
    int64_t drift;

    if (!clock->active)
        return;

    if (++clock->vi_count < vi_rate)
        return;

    clock->vi_count = 0;
    ++clock->sec;
    clock->last_drift = 0;

    if (calendar_time.get_current_time == NULL || !calendar_time.get_current_time(&now))
        return;

    offset = now.sec - clock->sec;
    if (!clock->host_offset_valid) {
        clock->host_offset = offset;
        clock->host_offset_valid = 1;
        return;
    }

    drift = offset - clock->host_offset;
    if (drift <= -CLOCK_LIBRETRO_MAX_DRIFT && drift > -CLOCK_LIBRETRO_MAX_HOLD) {
        /* hold rather than run the RTCs backwards */
        --clock->sec;
        clock->last_drift = -1;
    } else if (clock_libretro_far(drift)) {
        clock->sec += drift;
        clock->last_drift = drift;
    }
}

void clock_libretro_restore(struct clock_libretro* clock, int64_t sec, int32_t utc_offset, uint32_t vi_count)
{
    int64_t jump = sec - clock->sec;

    /* keep the reference across run-ahead and rewind, which roll back a few
     * frames, possibly to before the VI that applied a correction */
    if (clock_libretro_far(jump) && clock_libretro_far(jump + clock->last_drift))
        clock->host_offset_valid = 0;

    clock->last_drift = 0;
    clock->sec = sec;
    clock->utc_offset = utc_offset;
    clock->vi_count = vi_count;
}

bool clock_libretro_resync(struct clock_libretro* clock)
{
    struct retro_calendar_time t;

    if (!clock->active || calendar_time.get_current_time == NULL || !calendar_time.get_current_time(&t))
        return false;

    clock->sec = t.sec;
    clock->utc_offset = t.utc_offset;
    clock->vi_count = 0;
    clock->host_offset = 0;
    clock->host_offset_valid = 1;
    clock->last_drift = 0;
    return true;
}

static time_t clock_libretro_get_time(void* clock)
{
    return (time_t)((const struct clock_libretro*)clock)->sec;
}

static const struct tm* clock_libretro_local_time(void* clock, time_t t)
{
    t += ((const struct clock_libretro*)clock)->utc_offset;
    /* MSVC's gmtime returns NULL before 1970 */
    if (t < 0)
        t = 0;
    return gmtime(&t);
}

const struct clock_backend_interface g_iclock_libretro =
{
    clock_libretro_get_time,
    clock_libretro_local_time
};
