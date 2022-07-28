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

#ifndef MIDIPARSER_H_
#define MIDIPARSER_H_

#include <cstdint>
#include <functional>

class MidiParser
{
    uint8_t eventChannel = 0; /**< Midi Channel for which events will be generated. Numbers > 16 mean Omni mode. */

    uint8_t lastUsed = 255; /**< Last used key or controller, used for running status. Below 128 means invalid.*/
    uint8_t currentStatus = 0; /**< Saves the message type we are currently receiving. */
    uint8_t nRead = 0; /**< Number of Data bytes read.*/
    uint8_t expectedBytes = 0; /**< Number of Bytes expected for the message.*/
    uint8_t buffer[2];  /**< Buffer for the Data Bytes in order to assemble a message.*/
    uint8_t channel = 0; /**< The midi channel the message is received on. */
    bool inSysEx = false; /**< Are we in a SysEx Message. */
    bool ignoreBytes = false; /**< True if the received bytes should be ignored.*/

    uint16_t tempCC[32]; /**< Temporary Buffer for 14 bit CC Values. Needed since two messages are required before value can be assembled.*/
    uint8_t tempCCcounter[32]; /**< Counter to keep track which halves are still required.*/

    bool midi2compliant = false; /**< If true turns midi 2.0 compliant mode on.*/

    /*typedef void(*NoteOnEventFn)(uint8_t, uint8_t);
    typedef void(*NoteOffEventFn)(uint8_t, uint8_t);
    typedef void(*CCEvent7BitFn)(uint8_t, uint8_t);
    typedef void(*CCEvent14BitFn)(uint8_t, uint16_t);
    typedef void(*PitchBendEventFn)(uint16_t);*/

    typedef std::function<void(uint8_t, uint8_t)> NoteOnEventFn;
    typedef std::function<void(uint8_t, uint8_t)> NoteOffEventFn;
    typedef std::function<void(uint8_t, uint8_t)> CCEvent7BitFn;
    typedef std::function<void(uint8_t, uint16_t)> CCEvent14BitFn;
    typedef std::function<void(uint16_t)> PitchBendEventFn;


    NoteOnEventFn noteOnEvent;
    NoteOffEventFn noteOffEvent;
    CCEvent7BitFn CC7BitEvent;
    CCEvent14BitFn CC14BitEvent;
    PitchBendEventFn PitchBendEvent;

public:
    MidiParser(bool isMidi2 = false);
    ~MidiParser();

    void consumeByte(uint8_t msg);
    void fireEvent();
    void processCCEvent();

    void attachNoteOn(NoteOnEventFn fn) {noteOnEvent = fn;}
    void attachNoteOff(NoteOffEventFn fn) {noteOffEvent = fn;}
    void attachCCEvent7Bit(CCEvent7BitFn fn) {CC7BitEvent = fn;}
    void attachCCEvent14Bit(CCEvent14BitFn fn) {CC14BitEvent = fn;}
    void attachPitchBendEvent(PitchBendEventFn fn) {PitchBendEvent = fn;}

    inline uint8_t getChannel() const {return eventChannel;}
    inline void setChannel(uint8_t channel) {eventChannel = channel;}
};

#endif /* MIDIPARSER_H_ */
