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

#ifndef FMOSCILLATOR_H_
#define FMOSCILLATOR_H_

#include "fm_defines.h"
#include "OSCParam.h"
#include <cstdint>

class FMOscillator
{
    float* modmat; /**< Pointer to the modulation matrix*/
    float* output_volumes; /**< Volumes of the oscillators for the final output. */
    OSCParam* data; /**< Pointer to the Oscillator informations.*/
    float* output_pan; /**< Panning value for the output oscillators. 0 is center, -1 left and 1 right.*/

    float phases[N_OSC]; /**< Phase value for individual oscillators.*/

    float frequency; /**< Frequency of the oscillator. */

    float elapsed; /**< Elapsed time since sounding in ms.*/
    float releasepoint; /**< Timepoint of when the note was released. */

    float detune; /**< Oscillator detune amount in Cents. */
    float precalcDetuneFac; /**< Precalculated detuning factor. */

    float globalVol; /**< Global volume of this oscillator. */
    float globalPan; /*< Global panning of this oscillator. */

    float precalcVolLeft; /**< Precalculated global volume for the left channel. */
    float precalcVolRight; /**< Precalculated global volume for the right channel. */;

    bool isInit = false; /**< Is the oscillator considered initialized or not. */;

    float adsrs[N_OSC] = {0}; /**< Calculated ADSR values. */
    uint8_t counter = 0; /**< Counter used to update the adsr values every 16th sample. */


public:
    FMOscillator(float* modulationMatrix, OSCParam* oscData, float* volumes, float* pans);
    ~FMOscillator();

    /** \brief Sets all relevant values to default.
     *
     */
    void reset();

    /**
     * \brief Initializes everything to play a note at a certain frequency.
     *
     * \param[in] freq The frequency to play.
     * \param[in] oscVol The volume of the output.
     * \param[in] oscPan The panning of the output.
     * \param[in] phaseOffset The starting phase offset. Must be in [0, 1].
     *
     * \warning Assumes that everything has been reset beforehand.
     */
    void init(float freq, float oscVol=1.f, float oscPan=0.f, float phaseOffset=0.f);

    /** \brief Generates a Sample.
     *
     * This method generates a sample with the given phase.
     *
     * \param[in] isLeftChannel True if the desired value is for the left channel, false if for the right channel.
     *
     * \return The generated sample.
     */
    float generateSample(bool isLeftChannel=true);

    /**
     * \brief Increments the phases.
     *
     * \param[in] increment The time increment in ms.
     */
    void incrementPhase(float increment);

    /**
     * \brief Checks if the oscillator produces any sound.
     */
    bool isDone() const;

    /**
     * \brief Sets the detuning amount of the oscillator.
     *
     * \param[in] cents The detuning amount in cents.
     */
    inline void setDetune(float cents) {detune = cents; precalcDetuneFac = powf(2.f, detune/1200.f);}

    //cents to ratio formula: 2^(c/1200)
    //this can be easily verified by solving 440 * 2^((note*100 - 4900 + c)/1200) * b = 440 * 2^((note-49)/12)
    //for b

    /** \brief Marks the note as released to the oscillator.
     *
     * The currently stored timepoint will be used as the release timepoint.
     *
     * \note The storing of the timepoint will only take place if there is not one already set.
     *       This is done to prevent continuous release calls which let the oscillator
     *       play indefinitely.
     */
    inline void eventReleased() {
        if(releasepoint > elapsed){
            releasepoint = elapsed;
        }
        counter = 0;
        for(uint8_t i = 0; i < N_OSC; ++i){
            data[i].adsr.fastReleaseUpdate(adsrs[i]);
        }
    }

    inline float getElapsedTime() const {return elapsed;}

    /*
     * \brief Sets a new elapsed time value.
     *
     * This can be used to enable legato playing.
     */
    inline void overrideTimePos(float newPos) {elapsed = newPos;}

    /*
     * \brief Sets a new frequency to be played.
     *
     * Updates the Oscillator to play a new frequency.
     * This is useful for legato playing.
     */
    inline void overrideFrequency(float newFreq) {frequency = newFreq;}
};

#endif /* FMOSCILLATOR_H_ */
