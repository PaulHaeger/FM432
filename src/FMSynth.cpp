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

#include "FMSynth.h"


FMSynth::FMSynth()
{
    //Init voice pool
    voices.reserve(MAX_POLYPHONY);
    for(uint8_t i = 0; i < N_OSC; ++i){
        oscParams[i].adsr.precalc();
    }
    for(uint8_t i = 0; i < MAX_POLYPHONY; ++i){
        voices.push_back(Voice(modMatrix, oscParams, outputVols, outputPans));
    }

}

void FMSynth::PlayNote(KeyEvent &info, float elapsed)
{
    float hz = calcHzFromMidi(info.note);

    if(unison > 0){
        //Create spread out voices
        float stepsize = 1.f/(unison); //1/(1 + unison - 1) = 1/(unison)
        uint8_t nCenter = unison & 1 ? 1 : 2; //Use 2 center voices for the volume if the number of voices is even, 1 if odd.
        for(uint8_t i = 0; i < unison; ++i){
            FMOscillator* voice = findFreeOscillator();
            if(!voice){ //Should be unnecessary ;)
                //Fatal Error, abort.
                throw std::exception();
            }

            //If the Voices belong to the center use normal loudness, else use unisonVol as loudness.
            float vel_fac = (i >= unison/2 && i < unison/2+nCenter) ? 1.f : unisonVol;

            voice->init(hz, vel_fac * info.velocity/127.f, //
                        -unisonPan + i * 2 * unisonPan * stepsize, //Set panning from -unisonPan to +unisonPan
                        unisonPhase * i * stepsize); //Set Phase from  0 to unisonPhase.
            voice->setDetune(-.5f * unisonPitch + i * unisonPitch * stepsize + globalDetune); //Spread detune evenly from -1/2 unisonPitch to 1/2 unisonPitch.
            //NOTE: Currently global pitch automation and unison wont work together!
            voice->overrideTimePos(elapsed);
            info.voices[i] = voice;
        }
    }else{
        //Only one voice to deal with
        FMOscillator* voice = findFreeOscillator();
        if(!voice){ //Should be unnecessary ;)
            //Fatal Error, abort.
            //throw std::exception();
            return;
        }
        voice->init(hz, info.velocity/127.f);
        voice->overrideTimePos(elapsed);
        voice->setDetune(globalDetune);
        info.voices[0] = voice;
    }
}

void FMSynth::cleanVoicePool()
{
    for(Voice& voice: voices){
        if(voice.inUse && voice.osc.isDone()){
            voice.inUse = false;
            voice.osc.reset();
            --voicesUsed;
        }
    }
}

FMOscillator* FMSynth::findFreeOscillator()
{
    if(voicesUsed == voices.size()){
        //Attempt cleanup
        cleanVoicePool();
        if(voicesUsed == voices.size()){
            //Still no osc aviable
            return nullptr;
        }
    }
    for(Voice& voice : voices){
        if(!voice.inUse){
            voice.inUse = true;
            ++voicesUsed;
            return &(voice.osc);
        }
    }
    return nullptr;
}

FMSynth::~FMSynth()
{
}

void FMSynth::notePressedEvent(uint8_t midiVal, uint8_t velocity)
{
    if(isMono){
        //Monophonic mode enabled

        auto key = midiKeyEvents.front();
        if(isLegato){
            //Update involved oscillators
            float newFreq = calcHzFromMidi(midiVal);
            for(uint8_t i = 0; i < key.nVoices; ++i){
                FMOscillator* voice = key.voices[i];
                voice->overrideFrequency(newFreq);
                voice->setDetune(globalDetune);
            }
            key.note = midiVal;
            key.velocity = velocity; //Set velocity to make the key event releasable. The oscillator loudness is not updated.
        }else{
            //Release previous held key
            noteReleasedEvent(key.note, 0xFFU);
            KeyEvent& newEvent = midiKeyEvents.emplace_back(midiVal, velocity, unison+1);
            PlayNote(newEvent);
        }
    }else{
        //Polyphonic mode
        if(voicesUsed >= nPolyphony){
              //No free voice left, ignore event
              return;
        }
        KeyEvent& newEvent = midiKeyEvents.emplace_back(midiVal, velocity, unison+1);
        PlayNote(newEvent);
    }
}

void FMSynth::noteReleasedEvent(uint8_t key, uint8_t velocity)
{
    bool allRelease = true;//(velocity > 127 || velocity == 0);
    for(auto it = midiKeyEvents.begin(); it != midiKeyEvents.end(); ++it){
        if(it->note == key && (allRelease || it->velocity == velocity)){
            //Let the Oscillators stop playing
            for(uint8_t i = 0; i < it->nVoices; ++i){
                FMOscillator* osc = it->voices[i];
                osc->eventReleased();
            }

            //Delete the Keyevent
            auto ref = it;
            //Shift iterator away, as only iterators to the deleted element get invalidated.
            ++it;
            midiKeyEvents.erase(ref);
            //shift iterator back, this is done to not skip an element in the iteration
            --it;
        }
    }
    cleanVoicePool();
}

float FMSynth::getSample(bool isLeftChannel)
{
    float sum = 0.f;
    for(Voice& vc : voices){
        if(vc.inUse && !vc.osc.isDone()){
            sum += vc.osc.generateSample(isLeftChannel);
        }
    }
    return sum;
}

void FMSynth::setDetune(float cents)
{
    globalDetune = cents;
    for(Voice& vc: voices){
        if(vc.inUse){
            vc.osc.setDetune(cents);
        }
    }
}

void FMSynth::incrementPhases(float delta){
    for(Voice& vc : voices){
        if(vc.inUse){
            vc.osc.incrementPhase(delta);
        }
    }
}
