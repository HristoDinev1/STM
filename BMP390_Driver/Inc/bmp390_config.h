#pragma once
#include "bmp390_regs.h"

namespace BMP390_CONFIG {

// Default oversampling
constexpr BMP390::osr_p OVERSAMPLING_PRESS = BMP390::osr_p::x32;
constexpr BMP390::osr_t OVERSAMPLING_TEMP  = BMP390::osr_t::x2;

// Default IIR filter
constexpr BMP390::IIR_Config IIR_COEFF = BMP390::IIR_Config::bypass;

// Default SPI mode (4-wire for STM32)
constexpr BMP390::SPI_Config SPI_MODE = BMP390::SPI_Config::FourWire;

// Default startup mode
constexpr BMP390::Mode DEFAULT_MODE = BMP390::Mode::SLEEP;

// -------- Forced-mode conversion times (us) --------
constexpr uint32_t TEMP_CONV_TIME_US  = 2500;
constexpr uint32_t PRESS_CONV_TIME_US = 2500;

// -------- Timeouts for wait loops --------
constexpr uint32_t MAX_WAIT_ITER = 10000;
constexpr uint32_t WAIT_STEP_US  = 5;

// -------- Soft reset timing --------
constexpr uint32_t SOFT_RESET_DELAY_US = 3000;
}
