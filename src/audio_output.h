// ---------------------------------------------
//           This file is part of
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
//

#ifndef _AUDIO_OUTPUT_H_
#define _AUDIO_OUTPUT_H_

#define PCM_FIFO_SIZE 4096

#include <cstdint>
#include "gpio_msp432.h"
#include "spi_msp432.h"
#include "timer_msp432.h"
#include "dma_msp432.h"
#include "FIFO.h"

class audio_output
{
public:
    audio_output();
    virtual ~audio_output() = default;

    inline void enable_output(bool v) {
        _audio_en.gpioWrite(!v);
    }

    inline void start() { _pcm_timer.start(); }
    inline void stop()  { _pcm_timer.stop();  }

    inline void setRate(uint32_t kHz) {
        _pcm_timer.setNanoPeriod(1000000000/kHz+1, TIMER::PERIODIC);
    }

    inline int  fifo_available_put() { return _pcm_fifo.available_put(); }
    inline void fifo_put(uint16_t v) { _pcm_fifo.put(v); }

private:
    // BoostXL-Audio Objects
    gpio_msp432_pin _audio_en;
    gpio_msp432_pin _audio_cs;
    spi_msp432      _audio_spi;

    // The PCM output timer
    timer_msp432 _pcm_timer;

    // PCM FIFO buffer
    FIFO  <uint16_t> _pcm_fifo;
    uint16_t _pcm_value;

    // DMA stuff
    DMA::CH_CTRL_DATA _tasks[4];
    uint32_t _dma_ctrl0_backup;
    uint8_t  _zero;
    uint8_t  _one;
};

#endif // _AUDIO_OUTPUT_H_
