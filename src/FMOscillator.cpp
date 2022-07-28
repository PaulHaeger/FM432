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

#include "FMOscillator.h"
#include <cstring>
#include <cmath>
#include <cstdint>

FMOscillator::FMOscillator(float* modulationMatrix, OSCParam* oscData, float* volumes, float* pans)
    :modmat(modulationMatrix), output_volumes(volumes), data(oscData), output_pan(pans)
{
    data = oscData;
    reset();
}

void FMOscillator::reset(){
        elapsed = 0.f;
        frequency = 0.f;
        releasepoint = 1e8;
        detune = 0.f;
        precalcDetuneFac = 1.f;

        globalVol = 1.f;
        globalPan = 0.f;
        precalcVolLeft = .5f;
        precalcVolRight = .5f;

        for(uint8_t i = 0; i < N_OSC; ++i){
            phases[i] = 0.f;
        }

        isInit = false;
}

void FMOscillator::init(float freq, float oscVol, float oscPan, float phaseOffset)
{
    frequency = freq;
    globalVol = oscVol;
    globalPan = oscPan;

    //Precalculate values
    precalcVolLeft = oscVol * .25f * (-oscPan + 1.f); //0.25 instead of .5 to account for the factor of 2 in the generateSample function
    precalcVolRight = oscVol * .25f * (oscPan + 1.f);

    for(uint8_t i = 0; i < N_OSC; ++i){
        phases[i] = phaseOffset;
    }
    counter = 0;

    isInit = true;
}

FMOscillator::~FMOscillator()
{
}

float FMOscillator::generateSample(bool isLeftChannel)
{
    float shifts[N_OSC] = {0.f};
    float dummy = 0.f;

    if(counter & 16 || counter == 0){
        //Recalculate ADSR every 16 steps -> every 8ms
        for(uint8_t i=0; i < N_OSC; ++i){
            adsrs[i] = data[i].adsr.calc_vol(elapsed, releasepoint);
        }
        counter = 1;
    }
    ++counter;

    //Generate sample
    for(int8_t i = N_OSC; i > 0; --i){
        //Iterate from last to first row
        for(int8_t j = 0; j < N_OSC; ++j){
            float mod = modmat[(i-1)*N_OSC + j] * adsrs[j];
            if(fabs(mod) > 1e-5f){
                shifts[i-1] += mod * data[j].oscillator(phases[j] + shifts[j]);
                shifts[i-1] -= (int32_t)shifts[i-1]; //should be faster than modf
                shifts[i-1] = std::abs((shifts[i-1] < 0) - shifts[i-1]);
                //shifts[i-1] = std::modf(shifts[i-1]+phases[i-1], &dummy);
            }
        }
    }
    float output = 0.f;
    float sign = (!isLeftChannel > 0)*2.f - 1.f;

    for(uint8_t i=0; i < N_OSC; ++i){
        //Account for panning

        float pan = (sign * output_pan[i] + 1.0f); //panning vol 2 times too large, but we account for that in precalcVol
        //Calculate value
        //float tphase = modf(phases[i] + shifts[i], &dummy);
        output += pan * output_volumes[i] * data[i].oscillator(phases[i]+shifts[i]) * adsrs[i];
    }

    //Apply osc pan and volume
    output *= (isLeftChannel) * precalcVolLeft + (!isLeftChannel) * precalcVolRight;

    return output;
}

void FMOscillator::incrementPhase(float increment)
{
        //Increment time and phases
        elapsed += increment;
        float time = (increment/1000);

        //Account for detuning
        float real_freq = frequency * precalcDetuneFac;
        for(uint8_t i = 0; i < N_OSC; ++i){
            //divide by 1000 to scale for milliseconds
            phases[i] += time*(real_freq * data[i].ratio);
            phases[i] -= (int32_t)(phases[i]);
        }
}

bool FMOscillator::isDone() const
{
    if(!isInit){
        return true;
    }
    for(uint8_t i = 0; i < N_OSC; ++i){
        if(output_volumes[i] > 1e-3 && !data[i].adsr.isDone(elapsed, releasepoint)){
            return false;
        }
    }
    return true;
}
