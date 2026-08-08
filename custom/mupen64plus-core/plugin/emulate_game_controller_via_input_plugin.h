/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - emulate_game_controller_via_input_plugin.h              *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2014 Bobby Smiles                                       *
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

#ifndef M64P_PLUGIN_EMULATE_GAME_CONTROLLER_VIA_INPUT_PLUGIN_H
#define M64P_PLUGIN_EMULATE_GAME_CONTROLLER_VIA_INPUT_PLUGIN_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

enum pak_type;

int egcvip_is_connected(void* opaque, enum pak_type* pak);

uint32_t egcvip_get_input(void* opaque);

double setOperatingRange(int16_t position, double saturationRadius, double offset) {
    return position * saturationRadius / 32767.0 + offset;
}
  
double processDeadzoneAndResponseCurve(double position, double innerDeadzone, double cardinalMax, double offset) {
    double lengthAbsolute = abs(position - offset);
    if(lengthAbsolute <= innerDeadzone) {
        position = offset;
    } else {
        lengthAbsolute = (lengthAbsolute - innerDeadzone) * (cardinalMax) / (cardinalMax - innerDeadzone) / lengthAbsolute;
        position = (position - offset) * lengthAbsolute + offset;
    }
    return position;
}

double revisePosition(double position, double length, double saturationRadius, double offset) {
    if(length > saturationRadius) {
        length = saturationRadius / length;
        position = (position - offset) * length + offset;
    }
    return position;
}

void applyGateBoundaries(double innerDeadzone, double cardinalMax, double diagonalMax,
                         double positionX, double positionY, double offset,
                         double* boundedPositionX, double* boundedPositionY) {
    if (positionX != offset && positionY != offset) {
        double slope = (positionY - offset) / (positionX - offset);
        double edgex = copysign(cardinalMax / (fabs(slope) + (cardinalMax - diagonalMax) / diagonalMax), positionX);

        double edgey_numerator = 1.0 / fabs(slope) + (cardinalMax - diagonalMax) / diagonalMax;
        double edgey_candidate = fabs(edgex * slope);
        double edgey_limit = cardinalMax / edgey_numerator;

        double edgey = copysign((edgey_candidate < edgey_limit ? edgey_candidate : edgey_limit), positionY);
        edgex = edgey / slope;

        double distanceToEdge = hypot(edgex, edgey);
        double length = hypot(positionX - offset, positionY - offset);

        if (length > distanceToEdge) {
            positionX = edgex + offset;
            positionY = edgey + offset;
        }
    }

    if (boundedPositionX) *boundedPositionX = positionX;
    if (boundedPositionY) *boundedPositionY = positionY;
}

double clampAxisToNearestBoundary(double position, double offset, double cardinalMax) {
    if(fabs(position - offset) > cardinalMax)
        position = copysign(cardinalMax, position - offset) + offset;
    return position;
}

double counteractPrecisionError(double position) {
    return copysign(fabs(position) + 1e-09, position);
}

#endif
