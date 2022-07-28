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

#ifndef MIDITASK_H_
#define MIDITASK_H_

#include "MidiParser.h"
#include "uart_msp432.h"
#include "posix_io.h"

class MidiTask
{
    MidiParser& midiParser;
    uart_msp432 connection;

public:
    MidiTask(MidiParser& parser) : midiParser(parser) {
        connection.uartAttachIrq([this](char c){midiParser.consumeByte(c);});
        connection.uartEnableIrq();
    }
    ~MidiTask() {
        connection.uartDisableIrq();
        connection.uartDetachIrq();
    }

    MidiParser& getParser() {return midiParser;}
    const MidiParser& getParser() const { return midiParser;}


};

#endif /* MIDITASK_H_ */
