/*
 *  FM 432: A FM-Synthesizer implemented on the MSP432
 *  Copyright (C) 2022  Paul Häger
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef FM_DEFINES_H_
#define FM_DEFINES_H_

/*\file fm_defines.h
 * \brief This file contains common definitions used in the project.
 */

/*\brief How many oscillators should there be in total
 *
 */
#define N_OSC 2

/*
 * \brief Maximum amount of Voices playing at once.
 */
#define MAX_POLYPHONY 4


inline float pan2vol(float pan, bool isLeftChannel){
    return isLeftChannel * (-5. * pan + .5) + !isLeftChannel * (.5 * pan + .5);
}

inline float clampSignal(float val){
    return val * (val < 1.f && val > -1.f) + (val < -1.f) * -1.f + (val > 1.f) * 1.f;
}

#endif /* FM_DEFINES_H_ */
