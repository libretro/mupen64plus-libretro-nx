/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-Next - detours.h                                          *
 *   Copyright (C) 2022 M4xw <m4x@m4xw.net>                                *
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

// HLE Detour Backend for New Dynarec
#ifndef HLE_DETOURS_H
#define HLE_DETOURS_H 1

#include <api/m64p_types.h>

#define DETOUR_SUCCESS 0
#define DETOUR_FAIL    1
#define DETOUR_IGNORE  2

struct regstat;
int ujump_detour(int i, struct regstat *i_regs, struct regstat *regs, u_int *ba, u_int start);

#endif // HLE_DETOURS_H