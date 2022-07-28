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

#ifndef OSCPARAM_H_
#define OSCPARAM_H_

#include <cmath>

/**\brief Struct containing the ADSR info.
 *
 * ADSR stands for *A*ttack, *D*ecay, *S*ustain, *R*elease.
 * The attack represents how long it takes the Oscillator to get to a volume of 1
 * if a note is sounded.
 * The decay parameter represents how long it takes oscillator to return
 * to the sustain volume after it reached the max of the initial attack.
 * The sustain as indicated above is the volume level after the decay. The
 * Oscillator will continuously hold the note volume until the note is released.
 * The release paramater represents how long it takes the oscillator volume to
 * revert to 0 after a note off event was received.
 */
struct ADSRParam{

private:
    float attack = 1e-5;  /*< Attack time in ms. */
    float decay =  1e-5;  /*< Decay time in ms. */
    float sustain = 1.f; /*< Sustain Volume. */
    float release = 1e-5; /*< Release time in ms. */

    //PRECALCULATED VALUES
    float a_steepness = 0.f;
    float d_steepness = 0.f;
    float r_steepness = 0.f;
    float r_val = sustain;

    float t_ad = 0.f;
public:

    inline void setAttack(float a){attack = a; precalc();}
    inline void setDecay(float d){decay = d; precalc();}
    inline void setSustain(float s){sustain = s; precalc();}
    inline void setRelease(float r){release = r; precalc();}

    /**
     * \brief Recalculates the slope values.
     *
     * This Method is needed when the note is released before the decay is
     * finished. This will make it so that the Note releases from the
     * last held value.
     */
    inline void fastReleaseUpdate(float last_held){
        r_val = last_held;
        if(release > 1e-3){
            r_steepness = -r_val/release;
        }
    }

    /**
     * \brief Precalculates steepness values.
     *
     * This is done because a division costs 14 Cycles opposed to 1-2 cycles with multiplication in MSP432.
     */
    void precalc(){
        if(attack > 1e-3){
            a_steepness = 1/attack;
        }
        if(decay > 1e-3){
            d_steepness = (sustain-1)/decay;
        }
        if(release > 1e-3){
            r_steepness = -r_val/release;
        }

        t_ad = attack + decay;
    }

    /**
     * \brief Calculates the ADS Volume Value.
     *
     * Evaluates the ADSR Function. The attack is linearly increasing until 1,
     * the decay linearly decreases until sustain and sustain stays flat until
     * the note release.
     * The note release is a linear decay.
     *
     * The time positions are relative to the note start.
     *
     * \warning To avoid NaN values the time parameter should not directly be zero but very small numbers.
     *
     * \param[in] timepos The time position for the evaluation in ms.
     * \param[in] releaseTime Timepoint in ms when the Note was released. Infinity by default.
     *
     * \return The ADSR Value.
     */
    float calc_vol (float timepos, float releaseTime=1e9){
        const bool in_a = (timepos < attack);
        const bool in_d = !in_a && timepos < (t_ad);
        const bool in_s = !in_d && timepos < releaseTime;
        const bool in_r = timepos >= releaseTime && timepos <= (releaseTime + release);
        return   (in_a) * timepos * a_steepness \
                +(in_d) * (1+ d_steepness * (timepos - attack)) \
                +(in_s) * sustain \
                +(in_r) * (r_steepness * (timepos - releaseTime) + r_val);
    }

    /**\brief Returns whether the sound has reached a volume of zero or not.
     *
     */
    float isDone(float timepos, float releaseTime){
        return timepos > releaseTime + release;
    }
};

float empty_osc_fn(float);

struct OSCParam
{
    typedef float(*osc_fn)(float);
    osc_fn oscillator = &empty_osc_fn; /**< Oscillator evaluator. */
    float ratio = 1.f; /** < Frequency Ratio; */
    float vol = 1.f;  /** < Global Volume of the Oscillator */
    ADSRParam adsr;
};

/**
 * \brief Evaluator that does nothing. Used for the empty OSCParam.
 */
inline float dummy_evaluator(float){return 0.f;}

//const OSCParam __emptyOSC = {&dummy_evaluator, 0.f, 0.f, 0.f};

#endif /* OSCPARAM_H_ */
