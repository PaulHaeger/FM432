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

#include "audio_output.h"

audio_output::audio_output()
    : _audio_en (PORT_PIN(5,0)),
      _audio_cs (PORT_PIN(5,2)),
      _audio_spi(EUSCI_B0_SPI, _audio_cs),
      _pcm_fifo (PCM_FIFO_SIZE),
      _zero(0), _one(BIT2)
{
    // Configure BoostXL-audio objects
    _audio_en.gpioMode(GPIO::OUTPUT);
    _audio_spi.setSpeed(24000000);
    enable_output(true);

    // Configure PCM timer. Set a default time of
    // 1 ms. The correct value will be set via
    // the setRate() method by libmad.
    _pcm_timer.setCallback([this]() {
        if (_pcm_fifo.get(_pcm_value)) {
            // Trigger a DMA transfer ...
            dma_msp432::inst().ctrl_data[0].CTRL = _dma_ctrl0_backup;
            DMA_Control->ENASET    = BIT0;
            DMA_Control->ALTCLR    = BIT0;
            DMA_Channel->SW_CHTRIG = BIT0;

            // Send data via SPI ...
            // uint16_t sample = __builtin_bswap16(_pcm_value);
            // _audio_spi.spiTx((uint8_t *)&sample, 2);
        }
    });
    _pcm_timer.setPeriod(1000, TIMER::PERIODIC);

    // Setup DMA configuration
    // Task 1: Set Audio CS to LOW
    _tasks[0].SRC_DATA_END_PTR = &_zero;
    _tasks[0].DST_DATA_END_PTR = &P5->OUT;
    _tasks[0].CTRL.SRC_INC     = DMA::NO_INCREMENT;
    _tasks[0].CTRL.SRC_SIZE    = DMA::BYTE;
    _tasks[0].CTRL.DST_INC     = DMA::NO_INCREMENT;
    _tasks[0].CTRL.DST_SIZE    = DMA::BYTE;
    _tasks[0].CTRL.R_POWER     = DMA::ARB_AFTER_4;
    _tasks[0].CTRL.N_MINUS_1   = 0;
    _tasks[0].CTRL.CYCLE_CTRL  = DMA::CYCLE_MEM_SCATTER_GATHER_ALT;

    // Task 2: Send first byte (MSB) via SPI
    _tasks[1].SRC_DATA_END_PTR = (uint8_t *)(&_pcm_value)+1;
    _tasks[1].DST_DATA_END_PTR = &EUSCI_B0_SPI->TXBUF;
    _tasks[1].CTRL.SRC_INC     = DMA::NO_INCREMENT;
    _tasks[1].CTRL.SRC_SIZE    = DMA::BYTE;
    _tasks[1].CTRL.DST_INC     = DMA::NO_INCREMENT;
    _tasks[1].CTRL.DST_SIZE    = DMA::BYTE;
    _tasks[1].CTRL.R_POWER     = DMA::ARB_AFTER_4;
    _tasks[1].CTRL.N_MINUS_1   = 0;
    _tasks[1].CTRL.CYCLE_CTRL  = DMA::CYCLE_MEM_SCATTER_GATHER_ALT;

    // Task 3: Send second byte (LSB) via SPI
    _tasks[2].SRC_DATA_END_PTR = &_pcm_value;
    _tasks[2].DST_DATA_END_PTR = &EUSCI_B0_SPI->TXBUF;
    _tasks[2].CTRL.SRC_INC     = DMA::NO_INCREMENT;
    _tasks[2].CTRL.SRC_SIZE    = DMA::BYTE;
    _tasks[2].CTRL.DST_INC     = DMA::NO_INCREMENT;
    _tasks[2].CTRL.DST_SIZE    = DMA::BYTE;
    _tasks[2].CTRL.R_POWER     = DMA::ARB_AFTER_4;
    _tasks[2].CTRL.N_MINUS_1   = 0;
    _tasks[2].CTRL.CYCLE_CTRL  = DMA::CYCLE_MEM_SCATTER_GATHER_ALT;

    // Task 4: Set Audio CS to HIGH
    _tasks[3].SRC_DATA_END_PTR = &_one;
    _tasks[3].DST_DATA_END_PTR = &P5->OUT;
    _tasks[3].CTRL.SRC_INC     = DMA::NO_INCREMENT;
    _tasks[3].CTRL.SRC_SIZE    = DMA::BYTE;
    _tasks[3].CTRL.DST_INC     = DMA::NO_INCREMENT;
    _tasks[3].CTRL.DST_SIZE    = DMA::BYTE;
    _tasks[3].CTRL.R_POWER     = DMA::ARB_AFTER_4;
    _tasks[3].CTRL.N_MINUS_1   = 0;
    _tasks[3].CTRL.CYCLE_CTRL  = DMA::CYCLE_BASIC;

    dma_msp432 & dma = dma_msp432::inst();
    dma.ctrl_data[0].SRC_DATA_END_PTR = &_tasks[3].unused;
    dma.ctrl_data[0].DST_DATA_END_PTR = &dma.ctrl_data[8].unused;
    dma.ctrl_data[0].CTRL.CYCLE_CTRL  = DMA::CYCLE_MEM_SCATTER_GATHER_PRI;
    dma.ctrl_data[0].CTRL.N_MINUS_1   = 4 * 4 - 1;
    dma.ctrl_data[0].CTRL.R_POWER     = DMA::ARB_AFTER_4;
    dma.ctrl_data[0].CTRL.SRC_SIZE    = DMA::WORD;
    dma.ctrl_data[0].CTRL.SRC_INC     = DMA::WORD;
    dma.ctrl_data[0].CTRL.DST_SIZE    = DMA::WORD;
    dma.ctrl_data[0].CTRL.DST_INC     = DMA::WORD;
    _dma_ctrl0_backup = dma.ctrl_data[0].CTRL;

    DMA_Channel->CH_SRCCFG[0] = 0;
    DMA_Control->CFG          = DMA_CFG_MASTEN;
}
