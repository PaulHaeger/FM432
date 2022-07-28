# FM 432

A FM-Synthesizer implemented on the MSP432.

This is the result of a school project.
The objective of this project is to implement a FM-Synthesizer on the MSP432p401r Chip
with the BOOSTXL-AUDIO booster pack.

This Synth is able to play 4 note polyphony in short bursts and 2 note polyphony sustained when
one carrier and one modulator is used.

There are two oscillators available by default which allows for 16 different Modulation paths in total.

# NOTE

This was done as a school project, so I will most likely be not further developing this project.
I published the code in the hope that it will be useful to someone.

# Usage

This Synth does barely make use of hardware inputs (knobs, sliders, buttons, etc.).
It reacts to MIDI Commands. The MIDI commands should be sent to Serial UART interface by default. This can be changed
in the code however. You can either use a hardware connection or send the commands over the serial interface
if UART 0 is used.

The left button on the board (P1.1) increases the used MIDI Channel, the right button (P1.4) decreases it.
A channel number of 16 is interpreted as OMNI mode. The channel numbers go from 0 to 16.

The parameters are controlled using MIDI **C**ontroller **C**hange events.

|ID  | Parameter |
|----|-----------|
| 11 | Osc 0 to 0 mod amount |
| 12 | Osc 0 to 1 mod amount |
| 13 | Osc 1 to 0 mod amount |
| 14 | Osc 1 to 1 mod amount |
|----|-----------------------|
| 30 | OSC 0 Ratio           |
| 31 | OSC 1 Ratio           |
|----|-----------------------|
| 15 | Output Volume OSC 0  |
| 16 | Output Volume OSC 1 |
| 17 | Global Volume |
| 18 | Bitcrush Amount |
|----|-----------------------|
| 19 | OSC 0 Attack |
| 20 | OSC 0 Decay |
| 21 | OSC 0 Sustain |
| 22 | OSC 0 Release |
|    |     |
| 23 | OSC 1 Attack |
| 24 | OSC 1 Decay |
| 25 | OSC 1 Sustain |
| 26 | OSC 1 Release |
|----|---------|
| 27 | OSC 0 Waveform Select |
| 28 | OSC 1 Waveform Select |
|----|----------|

The FM-Ratio is calculated with `2^((val-63)/16)` where val is the MIDI CC value going from 0-127.
The highest setting will result in a ratio of 16 (4 Octaves up), the lowest will result in a ratio of 1/16 (4 Octaves down).
The value 63 will result in a ratio of 1.

Here is a table for the values of the power of two ratios:

| MIDI Value | Ratio | Octave |
|------------|-------|--------|
| 127  | 16 | 4 Up |
| 111  | 8 | 3 Up |
| 95 | 4 | 2 Up |
|79 | 2 | 1 Up |
| 63 | 1 | No Octave change |
| 47 | 1/2 | 1 Down |
| 31 | 1/4 | 2 Down |
| 15 | 1/8 | 3 Down |
| 0 | approx 1/16 | 4 Down |
|---|---|---|

The values for the waveform select:

|Range | Waveform |
|------|----------|
| 0-31  | Sine |
| 32-63 | Triangle |
| 64-95 | Saw |
| 95-127| Square |
|------|------|


Since this Synthesizer uses internal limiting you can achieve a hard-limiting distortion by
setting the Global volume to a high value.

Pitch Bend is supported.

Mod wheel has no effect.

Sustain Pedal is not supported.

This Synth runs with a sampling rate of 20kHz which will mean any frequency over 10kHz will alias.
Due to hardware limitations no oversampling is used.

# Build

To build this for the MSP432 you will need `arm-none-eabi-gcc` and potentially `arm-none-eabi-newlib` depening
on your package manager.
If you have this not installed in your /usr/ path, then you will need to edit the sed command in the
`build_deps.sh` to point towards your desired path.

Then you can use cmake to build the project. The cached option `ARM_TOOLCHAIN_PATH` will point
towards the correct directory for cmake.

The project can then be build with your selected build tool.

To flash the binary you can use the target `flash`. If you use make, then the command will be `make flash`.
For the flashing to work you need to have DSLITE from Texas Instrument installed. It comes with code compositor studio.
The Path to DSLITE can be set in the `DSLITE` Cache variable.

If you dont want to use the TI Tool it is also possible to flash the application using OpenOCD, but I havent figured out
yet how.

# LICENSE

This Project is licensed under the GPLv3.

The YAHAL library is used in this project, which is licensed under BSD:

    Copyright 2016-2021 Andreas Terstegge

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
    OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
    OF SUCH DAMAGE.

Files by Texas Instrument will have thier own copyright information in the header of the file.
