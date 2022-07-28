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

#ifndef FMSYNTH_H_
#define FMSYNTH_H_

#include "fm_defines.h"
#include "FMOscillator.h"
#include "OSCParam.h"
#include <cstdint>
#include <vector>
#include <list>


/** \brief Class Coordinating the different Oscillators.
 *
 * This class contains the modulation info and paths for the different oscillators.
 * The note playing, polyphony and unison are managed here.
 */
class FMSynth
{

    float centerTune = 440.f; /**< Center Tuning for the Synth in Hz. */
    float globalDetune = 0.f; /**< Global Detune in Cents. */
    float globalVolume = 1.f; /**< Global Volume. */

    bool isMono = false; /**< Whether playing in monophonic or in polyphonic mode. */
    bool isLegato = false; /**< Only relevant if Mono is enabled. */
    //bool portaEnabled = false /**< Is Portamento enabled? Only relevant if Mono is enabled. */

    uint8_t unison = 0; /**< How many unison voices should play per keypress*/
    float unisonVol = 0.f; /**< The volume of the unison voices. */
    float unisonPitch = 0.f; /** The pitch variation of the unison voices, in cent, */
    float unisonPhase = 0.f; /** The phase variation of the unison voices. Between 0 and 1. */
    float unisonPan = 0.f; /** The panning spread of the voices. Between 0 and 1. */

    uint8_t nPolyphony = MAX_POLYPHONY; /**< How many keys are allowed to be pressed at once. */

    /**
     * \bief Modulation Matrix.
     *
     * This array presents a modulation matrix. Its stored as one
     * Continuous array. The Algorithm will go from the last
     * row to the first.
     * Each row the oscilator results are summed and stored as a final shift
     * parameter, which will be added to the corresponding oscillator if in the
     * next row it is used again.
     * Then the values for all selected output rows will be computed.
     */
    float modMatrix[N_OSC*N_OSC] = {0.f};
    float outputVols[N_OSC] = {0.f}; /**< The output volumes of the individual oscillators. */
    float outputPans[N_OSC] = {1.f}; /**< The output panning of the individual oscillators. */
    OSCParam oscParams[N_OSC];      /**< The Parameters for the different oscillators. */


    /*
     * \brief Structure containing info for a voice.
     */
    struct Voice{
        bool inUse = false; /**< Indicates whether the Voice is being Used or not. */
        FMOscillator osc; /**< The audio generator for the voice. */

        Voice(float* modulationMatrix, OSCParam* oscData, float* volumes, float* pans)
            : inUse(false), osc(modulationMatrix, oscData, volumes, pans)
        {}
    };

    //FMOscillator(modMatrix, oscParams, outputVols, outputPans)
    std::vector<Voice> voices; /**< Voice Pool. */
    uint8_t voicesUsed = 0; /**< Number of Voices in active use. */

    /**
     * \brief Structure keeping a midi Key event in memory.
     *
     * The Structure associates a midi key event and the voices
     * involved. This is necessary to find out when keys
     * are released.
     */
    struct KeyEvent{
        uint8_t note; /** The pressed midi note. */
        uint8_t velocity; /** The velocity of the pressed note. */
        uint8_t nVoices; /** Number of the involved voices. */
        FMOscillator** voices; /** An Array of pointer to the involved voices. */

        /**
         * \brief Creates a KeyEvent Class.
         *
         * It preallocates a pointerarray for nVoices. The pointers have to be added manually.
         */
        KeyEvent(uint8_t n, uint8_t vel, uint8_t nVoices)
            : note(n), velocity(vel), nVoices(nVoices)
        {
            voices = new FMOscillator*[nVoices];
        }

        ~KeyEvent(){
            delete[] voices;
        }
    };

        //uint16_t midiKeyEvents[MAX_POLYPHONY]
    std::list<KeyEvent> midiKeyEvents;


    void PlayNote(KeyEvent& info, float elapsed=0.f);


    /**
     * \brief Finds the next free Oscillator.
     *
     * If there are currently no voices marked as free, then a cleanup is
     * attempted. If still nothing is found, a nullptr is returned.
     *
     * \warning Marks the returned oscillator as used.
     *
     * \return The pointer to the Oscillator, or nullptr if search failed.
     */
    FMOscillator* findFreeOscillator();

    /**
     * \brief Calculates the frequency of the midi note.
     *
     * The tuning used is the equal temperment tuning centered at the selected frequency.
     *
     * \param[in] note The midi note.
     *
     * \return The corresponding frequency.
     */
    inline float calcHzFromMidi(uint8_t note){
        return centerTune * powf(2.f, (note - 64.f)/12.f);
    }




public:
    FMSynth();
    ~FMSynth();


    /**
     * \brief Tries to find any finished voices and marks them as unused.
     */
    void cleanVoicePool();

    /**
     * \brief Deals with a note pressed midi event.
     *
     * \param[in] midiVal The pressed midi note.
     * \param[in] velocity The strength the note was pressed with. Range is 0-127.
     */
    void notePressedEvent(uint8_t midiVal, uint8_t velocity);

    /**
     * \brief Releases the key with the selected velocity.
     *
     * The most significant bit of the velocity indicates if every event
     * matching the key should be released. Else just the press with the
     * matching velocity and key will be released.
     *
     * \param[in] key The key to release.
     * \param[in] velocity The velocity of the key to release.
     */
    void noteReleasedEvent(uint8_t key, uint8_t velocity);

    void setDetune(float cents);

    /*
        * \brief Sets the modulation amount of the modulator for the carrier.
        *
        * \param[in] carrier The oscillator id of the carrier.
        * \param[in] modulator The oscillator id of the modulator.
        * \param[in] modAmount The modulation amount to be set.
        */
       inline void setMod(uint8_t carrier, uint8_t modulator, float modAmount){
           if(carrier < N_OSC && modulator < N_OSC){
               modMatrix[carrier * N_OSC + modulator] = modAmount;
           }
       }

       /*
        * \brief Sets the Output Volume for the Oscillator.
        *
        * \param[in] oscillator The oscillator id.
        * \param[in] vol The volume to be set. Must be >= 0.
        */
       inline void setOutputVolume(uint8_t oscillator, float vol){
           if(vol >= 0.f && oscillator < N_OSC){
               outputVols[oscillator] = vol;
           }
       }

       /*
        * \brief Sets the panning of the output.
        *
        * \param[in] oscillator The oscillator id.
        * \param[in] pan The panning. Clamped into the range [-1, 1].
        */
       inline void setOutputPan(uint8_t oscillator, float pan){
               if(oscillator < N_OSC){
                   //Clamp panning between -1 and 1
                   outputVols[oscillator] = (pan < -1.f) * -1.f + (pan > 1.f) * 1.f + (pan <= 1.f && pan >= -1.f) * pan;
               }
       }

       inline OSCParam& getParam(uint8_t oscillator){
           return oscParams[oscillator];
       }

       /**
        * \brief Generates a Sample for the selected channel.
        *
        * \param[in] isLeftChannel True if the sample is for the left channel, false if for the right.
        *
        * \return The generated sample.
        */
       float getSample(bool isLeftChannel=true);

       /**
        * \brief Increments the Phases of the Voices with deltatime.
        *
        * \param[in] delta Time increment in ms.
        */
       void incrementPhases(float delta);

       /**
        * \brief Enables or disables the monotonic mode.
        *
        * \param[in] val Monotonic mode on or off.
        */
       void setMono(bool val){
           isMono = val;
       }

       void setLegato(bool val){
          isLegato = val;
       }
};

#endif /* FMSYNTH_H_ */
