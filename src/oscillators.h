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

/* \brief Approximation of sin(2pi*phi).
 *
 * This uses the Bhaskara I's sine approximation formula
 * with phi replaced by 2pi*x.
 * The Function assumes that the parameter value is within bounds.
 * Adding values that are outside the bounds will result in
 * a wrong approximation
 *
 * \param[in] phase Argument of sin. Must be in [0, 1].
 *
 * \return The approximate value of sin(2*pi*phi).
 */
inline float sine(float phase){
    float sign = -1 * (phase > .5) + 1 * (phase < .5);
    phase = phase - .5 * (phase > .5);
    return sign * 32*phase*(1-2*phase)/(5-8*phase*(1-2*phase));
}

/* \brief Computes a triangle wave.
 *
 * One Cycle will go from [0, 1].
 * The Argument has to be inbetween [0, 1]. Using numbers
 * outside that may result in wrong values.
 *
 * \param[in] phase The phase of the triangle wave.
 *
 * \return The value of the triangle wave at position phi.
 */
inline float triangle(float phase){
    //boolean arithmetic is faster than branching
    return (phase <= .5) * (4*phase - 1) + (phase > .5) * (3 - 4*phase);
}

/* \brief Computes a saw wave.
 *
 * One Cycle will go from [0, 1].
 * The Argument has to be inbetween [0, 1]. Using numbers
 * outside that may result in wrong values.
 *
 * \param[in] phase The phase of the saw wave.
 *
 * \return The value of the saw wave at position phi.
 */
inline float saw(float phase){
    //boolean arithmetic is faster than branching
    return 2*phase - 1;
}

/* \brief Computes a square wave.
 *
 * One Cycle will go from [0, 1].
 * The Argument has to be inbetween [0, 1]. Using numbers
 * outside that may result in wrong values.
 *
 * \param[in] phase The phase of the square wave.
 *
 * \return The value of the saw wave at position phi.
 */
inline float square(float phase){
    //boolean arithmetic is faster than branching
    return -1 * (phase <= .5) + 1 * (phase > .5);
}

/* \brief Computes a square wave with 25% duty cycle.
 *
 * One Cycle will go from [0, 1].
 * The Argument has to be inbetween [0, 1]. Using numbers
 * outside that may result in wrong values.
 *
 * \param[in] phase The phase of the square wave.
 *
 * \return The value of the saw wave at position phi.
 */
inline float square25pwm(float phase){
    //boolean arithmetic is faster than branching
    return -1 * (phase <= .75) + 1 * (phase > .75);
}

/* \brief Computes a square wave with 10% duty cycle.
 *
 * One Cycle will go from [0, 1].
 * The Argument has to be inbetween [0, 1]. Using numbers
 * outside that may result in wrong values.
 *
 * \param[in] phase The phase of the square wave.
 *
 * \return The value of the saw wave at position phi.
 */
inline float square10pwm(float phase){
    //boolean arithmetic is faster than branching
    return -1 * (phase <= .9) + 1 * (phase > .9);
}

