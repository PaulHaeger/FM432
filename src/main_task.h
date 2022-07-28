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

// ---------------------------------------------
//           This file contains code from
//      _  _   __    _   _    __    __
//     ( \/ ) /__\  ( )_( )  /__\  (  )
//      \  / /(__)\  ) _ (  /(__)\  )(__
//      (__)(__)(__)(_) (_)(__)(__)(____)
//
//     Yet Another HW Abstraction Library
//      Copyright (C) Andreas Terstegge
//      BSD Licensed (see file LICENSE)
//
// ---------------------------------------------

#ifndef _MAIN_TASK_H_
#define _MAIN_TASK_H_

#include <cmath>

#include "gpio_msp432.h"
#include "spi_msp432.h"
#include "sd_spi_drv.h"
#include "FMSynth.h"
#include "audio_output.h"
#include "oscillators.h"

#include "task.h"

#include "MidiParser.h"
#include "MidiTask.h"

class main_task : public task
{
public:
    main_task() : task("Main", 6000) {

    }

    void run() override {

        audio_output audio_output;
        FMSynth synth;

        synth.setMono(false);
        //synth.setLegato(true);
        synth.setMod(0, 1, 2.0f);
        synth.setOutputVolume(0, 1.f);
        OSCParam& mod1 = synth.getParam(0);
        mod1.ratio = 1.f;
        mod1.oscillator = &sine;
        mod1.adsr.setAttack(20.f);
        mod1.adsr.setSustain(1.f);
        mod1.adsr.setDecay(800.f);
        mod1.adsr.setRelease(20.f);

        OSCParam& mod2 = synth.getParam(1);
        mod2.ratio = 2.f;
        mod2.oscillator = &triangle;
        mod2.adsr.setAttack(10.f);
        mod2.adsr.setDecay(700.f);
        mod2.adsr.setSustain(0.7f);

        //Increment per step in ms
        constexpr float delta = (1000/20000.f);
        //Volume
        float vol = 1.f;

        //Bitcrusher values
        uint16_t bc_val = 1;
        float ibc_val = 1.f;

        MidiParser parser;
        //Set up Callbacks
        parser.attachNoteOn([&synth](uint8_t a, uint8_t b){
            synth.notePressedEvent(a, b);
        });

        parser.attachNoteOff([&synth](uint8_t a, uint8_t b){
            synth.noteReleasedEvent(a, b);
        });

        parser.attachCCEvent7Bit([&synth, &bc_val, &ibc_val, &vol, &mod1, &mod2](uint8_t id, uint8_t val){
            // Modulation Parameters
            if(id == 11){
                synth.setMod(0, 0, val/127.f * 3.f);
            }else if(id == 12){
                synth.setMod(0, 1, .3f+val/127.f * 3.f);
            }else if(id == 13){
                synth.setMod(1, 0, val/127.f * 3.f);
            }else if(id == 14){
                synth.setMod(1, 1, val/127.f * 3.f);
            }else if(id == 15){
                //Output Parameters
                synth.setOutputVolume(0, val/127.f);
            }else if(id == 16){
                synth.setOutputVolume(1, val/127.f);
            }else if(id == 17){
                //Vol
                vol = val/64.f;
            }else if(id == 18){
                bc_val = 30*(uint16_t)val + 1;
                ibc_val = 1.f/(float)bc_val;
            }else if (id == 19) {
                //OSC 0 Attack
                mod1.adsr.setAttack(std::exp(val/100.f) * 7000.f - 7000.f);
            }else if (id == 20) {
                //OSC 0 Decay
                mod1.adsr.setDecay(std::exp(val/100.f) * 7000.f - 7000.f);
            }else if (id == 21) {
                //OSC 0 Sustain
                mod1.adsr.setSustain(val/127.f);
            }else if (id == 22) {
                //OSC 0 Release
                mod1.adsr.setRelease(std::exp(val/100.f) * 7000.f - 7000.f);
            }else if (id == 23) {
                //OSC 1 Attack
                mod2.adsr.setAttack(std::exp(val/100.f) * 7000.f - 7000.f);
            }else if (id == 24) {
                //OSC 1 Decay
                mod2.adsr.setDecay(std::exp(val/100.f) * 7000.f - 7000.f);
            }else if (id == 25) {
                //OSC 1 Sustain
                mod2.adsr.setSustain(val/127.f);
            }else if (id == 26) {
                //OSC 1 Release
                mod2.adsr.setRelease(std::exp(val/100.f) * 7000.f - 7000.f);
            }else if (id == 27) {
                //OSC 0 Waveform
                if(val < 32) {
                    mod1.oscillator = &sine;
                }else if (id < 64) {
                    mod1.oscillator = &triangle;
                }else if (id < 96) {
                    mod1.oscillator = &saw;
                }else {
                    mod1.oscillator = &square;
                }
            }else if (id == 28) {
                //OSC 1 Waveform
                if(val < 32) {
                    mod2.oscillator = &sine;
                }else if (id < 64) {
                    mod2.oscillator = &triangle;
                }else if (id < 96) {
                    mod2.oscillator = &saw;
                }else {
                    mod2.oscillator = &square;
                }
            }else if (id == 30) {
                //OSC 0 Ratio
                //2^((val-63)/16)
                mod1.ratio = std::pow(2.f, (val -63.f)/16);
            }else if (id == 31) {
                //OSC 1 Ratio
                mod2.ratio = std::pow(2.f, (val -63.f)/16);
            }
        });

        parser.attachPitchBendEvent([&synth](uint16_t val){
           float b = (val/8192.f - 1.f)*1200.f;
           synth.setDetune(b);
        });

        //Set up side buttons for channel switching
        gpio_msp432& gpios = gpio_msp432::inst;
        //If the channel id after incrementing is above 17, reset to 0
        gpios.gpioAttachIrq(PORT_PIN(1, 1), GPIO::RISING, [&parser](){
                                                                        auto chan = parser.getChannel();
                                                                        chan = (++chan > 17) ? 0 : chan;
                                                                        parser.setChannel(chan);
                                                                      }); //left button
        //If the channel id after decrementing is below 0, set to 17
        gpios.gpioAttachIrq(PORT_PIN(1, 4), GPIO::RISING, [&parser](){
                                                                        int8_t chan = parser.getChannel();
                                                                        chan = (--chan < 0) ? 17 : chan;
                                                                        parser.setChannel(chan);
                                                                    }); //right button

        gpios.gpioEnableIrq(PORT_PIN(1, 1));
        gpios.gpioEnableIrq(PORT_PIN(1, 4));

        //Initialize Midi Task
        MidiTask midiT(parser);

        //Set up Audio output
        audio_output.setRate(20000); //20kHz samplerate -> 10kHz max freq
        audio_output.enable_output(true);


        float premul = 6191;
        //Start output
        audio_output.start();

        while(true) {
            while(audio_output.fifo_available_put() > 0){
                float val = vol * synth.getSample(false);
                val = clampSignal(val);
                audio_output.fifo_put(8192 + bc_val * static_cast<int16_t>(val *premul * ibc_val));
                synth.incrementPhases(delta);
            }
            synth.cleanVoicePool(); //Clean up Voicepool, this improves performance.

            task::sleep(50); //Sleep if nothing to do.
        }
    }
};

#endif // _MAIN_TASK_H_
