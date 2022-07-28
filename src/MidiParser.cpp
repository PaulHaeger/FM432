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

#include "MidiParser.h"

MidiParser::MidiParser(bool isMidi2)
:midi2compliant(isMidi2)
{
    //Assign Empty events
    noteOnEvent = [](uint8_t a, uint8_t b){};
    noteOffEvent = [](uint8_t a, uint8_t b){};
    CC14BitEvent = [](uint8_t a, uint16_t b){};
    CC7BitEvent = [](uint8_t a, uint8_t b){};
    PitchBendEvent = [](uint16_t){};

}

MidiParser::~MidiParser()
{
    // TODO Auto-generated destructor stub
}

void MidiParser::consumeByte(uint8_t msg)
{
    if(msg & 128){
        //Status received
        if(msg < 0xF0){
            //Musical Command
            this->channel = msg & 0xF; //Channel number is in the second half of the byte
            msg &= 0xF0; //Remove channel from the message;
            lastUsed = msg;
        }
        currentStatus = msg; //Save the current status

        if(msg == 0xF0){
            //Sysex Begin
            nRead = 0;
            inSysEx = true;
            ignoreBytes = true;
        }
        if(msg == 0xF7){
            inSysEx = false;
            ignoreBytes = false;
        }
        switch(msg){
            case 0x80:
                //Note Off Event
            case 0x90:
                //Note On Event
            case 0xA0:
                //After Touch
            case 0xB0:
                //Continuous Controller
            case 0xC0:
                //Patch Change
            case 0xD0:
                //Channel Pressure
            case 0xE0:
                //Pitch Bend
                nRead = 0;
                expectedBytes = 2;
                ignoreBytes = false;
                break;
            case 0xF0:
                //Start System Exclusive
                nRead = 0;
                ignoreBytes = true;
                inSysEx = true;
                break;
            case 0xF1:
                //MIDI Time Code Quarter Frame
                nRead = 0;
                ignoreBytes = false;
                expectedBytes = 1;
                break;
            case 0xF2:
                //Song Position Pointer
                nRead = 0;
                ignoreBytes = false;
                expectedBytes = 2;
                break;
            case 0xF3:
                //Song Select
                nRead = 0;
                expectedBytes = 1;
                ignoreBytes = false;
                break;
            case 0xF4:
            case 0xF5:
                //Undefined(Reserved)
                break;
            case 0xF6:
                //Tune Request
                break;
            case 0xF7:
                //End of System Exclusive
                ignoreBytes = false;
                inSysEx = false;
                break;
            case 0xF8:
                //Timing Clock (Sys Realtime)
            case 0xF9:
                //??
            case 0xFA:
                //Start (Sys Realtime)
            case 0xFB:
                //Continue (Sys Realtime)
            case 0xFC:
                //Stop (Sys Realtime)
            case 0xFD:
                //Reserved
                break;
            case 0xFE:
                //Active Sensing (Sys Realtime)
                //TODO: Implement
                //Active sensing is like a Watchdog timer. If no Active Sensing Message is received within 300 ms the synth will
                //stop sounding and return to normal mode
                break;
            case 0xFF:
                //Systen Reset
                //Todo
                break;
        }

    }else{
        //Data byte received
        if(!ignoreBytes){
            buffer[nRead++] = msg;
            if(nRead == expectedBytes){
                nRead = 0;
                if(inSysEx){
                    ignoreBytes = true;
                }
                fireEvent();
                currentStatus = 0;
            }
        }

    }

}

void MidiParser::fireEvent()
{
    const uint8_t event = currentStatus < 128 ? lastUsed: currentStatus;
    if(event < 128){
        //No valid event to fire, ignore
        return;
    }
    if(eventChannel <= 16 && channel != eventChannel){
        //Message not for our channel, ignore
        return;
    }
    switch(event){
            case 0x80:
                noteOffEvent(buffer[0], buffer[1]);
                break;
            case 0x90:
                //Note On Event
                if (buffer[1] == 0){
                    //Velocity of 0 equals NoteOff
                    noteOffEvent(buffer[0], 255);
                }else{
                    noteOnEvent(buffer[0], buffer[1]);
                }
                break;
            case 0xA0:
                //After Touch
                break;
            case 0xB0:
                //Continuous Controller
                processCCEvent();
                break;
            case 0xC0:
                //Patch Change
                break;
            case 0xD0:
                //Channel Pressure
                break;
            case 0xE0: {
                //Pitch Bend
                uint16_t val = 0;
                val |= buffer[0]; //lsb
                val |= static_cast<uint16_t>(buffer[1]) << 7; //msb, shift by seven because we only get 7 bits per byte
                PitchBendEvent(val);
                break;
            }
            case 0xF0:
                //Begin System Exclusive
            case 0xF1:
                //MIDI Time Code Quarter Frame

            case 0xF2:
                //Song Position Pointer

            case 0xF3:
                //Song Select

            case 0xF4:
            case 0xF5:
                //Undefined(Reserved)

            case 0xF6:
                //Tune Request

            case 0xF7:
                //End of System Exclusive

            case 0xF8:
                //Timing Clock (Sys Realtime)
            case 0xF9:
                //??
            case 0xFA:
                //Start (Sys Realtime)
            case 0xFB:
                //Continue (Sys Realtime)
            case 0xFC:
                //Stop (Sys Realtime)
            case 0xFD:
                //Reserved
                break;
            case 0xFE:
                //Active Sensing (Sys Realtime)
                //TODO: Implement
                //Active sensing is like a Watchdog timer. If no Active Sensing Message is received within 300 ms the synth will
                //stop sounding and return to normal mode
                break;
            case 0xFF:
                //System Reset
                //Todo
                break;
        }
}

void MidiParser::processCCEvent(){
    uint8_t id = buffer[0];
    if(id > 127){
        //Invalid id, skip
        return;
    }
    if(id < 64){
        //14 Bit event received if midi 2.0
        if(midi2compliant){
            if(id < 32){
                //Received MSB for a 14 bit Controller
                tempCC[id] = static_cast<uint16_t>(buffer[1]) << 7;
                tempCCcounter[id] |= 0x2; //Set MSB set flag
            }else{
                id -= 32; //Shift id to the correct value
                //LSB for a 14 bit Controller received
                tempCC[id] &= 0xFF80; //Zero second half
                tempCC[id] |= buffer[1]; //Write LSB
                tempCCcounter[id] |= 0x1; //Set LSB set flag
            }
            if(tempCCcounter[id] == 0x3){
                //Both parts set, fire event
                CC14BitEvent(id, tempCC[id]);
                tempCCcounter[id] = 0x0; //Clear flags
            }
        }else{
            CC7BitEvent(id, buffer[1]);
        }
    }else{
        //7 Bit event received
        CC7BitEvent(id, buffer[1]);
    }
}
