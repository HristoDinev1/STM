/*
 * STM32SPI.h
 *
 *  Created on: Nov 23, 2025
 *      Author: icodi
 */

#pragma once
#include "stm32h755xx.h"
#include <stdint.h>

namespace SPI {
namespace Config {
enum class Comm : uint8_t {
  FULL_DUPLEX = 0U,
  TX_ONLY = 1U,
  RX_ONLY = 2U,
  HALF_DUPLEX = 3U
};

enum class CPHA : uint8_t { RISING_EDGE = 0U, TRAILING_EDGE = 1U };
enum class CPOL : uint8_t { inverted = 0U, notInverted = 1U };

enum class SpiPrescaler : uint16_t {
  div2 = 2U,
  div4 = 4U,
  div8 = 8U,
  div16 = 16U,
  div32 = 32U,
  div64 = 64U,
  div256 = 256U,
};

enum class SpiBaud : uint8_t {
  DIV_2 = 0b000,
  DIV_4 = 0b001,
  DIV_8 = 0b010,
  DIV_16 = 0b011,
  DIV_32 = 0b100,
  DIV_64 = 0b101,
  DIV_128 = 0b110,
  DIV_256 = 0b111
};
struct spiConfig {
  Config::SpiBaud baud;
  Config::CPOL cpol;
  Config::CPHA cpha;
  Config::Comm mode;
  Config::SpiPrescaler
      baudPrescaler; // values are : 2/4/8/..../256 - pomisli po-dobur design
  bool lsbFirst;
  bool ssmEnabled;
  bool isMaster;
};

class ISPI {};
enum class Status { OK = 1U, BUSY = 2U, ERR = 3U, ERR_TIMEOUT = 4U };

class STM32SPI : public ISPI {
public:
  STM32SPI(SPI_HandleTypeDef *hspi, GPIO_TypeDef *csPort,
           uint16_t csPin) noexcept;

  STM32SPI(const STM32SPI &other) = delete;
  STM32SPI operator=(const STM32SPI &other) = delete;

  Status init(const spiConfig &defaultConf) noexcept;

  Status read(uint8_t *rx, uint16_t len) const noexcept;
  Status write(uint8_t *tx, uint16_t len) const noexcept;
  Status transfer(uint8_t *tx, uint8_t *rx, uint16_t len) const noexcept;

  bool isSpiBusy() const noexcept;

  ~STM32SPI() noexcept = default;

private:
  // CFG1
  Status setBaudRate(Config::SpiBaud baud) noexcept;
  // CFG2
  Status setPrescaler(Config::SpiPrescaler value) noexcept; // baudPrescaler
  Status setClockPhase(Config::CPHA value) noexcept;
  Status setClockPolarity(Config::CPOL value) noexcept;
  Status setLsbFirst(bool isTrue) noexcept;
  Status setComm(Config::Comm mode) noexcept;
  Status setSSM(bool isOn) noexcept;
  // txdr
  // SR
  bool rxPacketAvailable() const noexcept;
  bool txSpaceAvailable() const noexcept;
  bool transmissionComplete() const noexcept;

  //
  Status csLow() const noexcept;
  Status csHigh() const noexcept;

  spiConfig config_;
  SPI_TypeDef *hspi_;
  GPIO_TypeDef *csPort_;
  uint16_t csPin_;
};

}
