/*
 * STM32SPI.cpp
 *
 *  Created on: Dec 4, 2025
 *      Author: icodi
 */

#include "STM32SPI.h"

using namespace SPI::Config;
using namespace SPI;

STM32SPI::STM32SPI(SPI_HandleTypeDef *hspi, GPIO_TypeDef *csPort,
                   uint16_t csPin) noexcept
    : hspi_(hspi->Instance), csPort_(csPort), csPin_(csPin) {}

Status STM32SPI::setBaudRate(Config::SpiBaud baud) noexcept {
  if (hspi_->CR1 & SPI_CR1_SPE) return Status::BUSY;
  hspi_->CFG1 &= ~SPI_CFG1_MBR_Msk;
  hspi_->CFG1 |= (static_cast<uint32_t>(baud) << SPI_CFG1_MBR_Pos);
  config_.baud = baud;
  return Status::OK;
}
Status STM32SPI::init(const spiConfig &defaultConf) noexcept {
  config_ = defaultConf;
  hspi_->CR1 &= ~SPI_CR1_SPE_Msk;
  hspi_->CFG1 &= ~(SPI_CFG1_DSIZE_Msk | SPI_CFG1_FTHLV_Msk);
  hspi_->CFG1 |= (7U << SPI_CFG1_DSIZE_Pos);
  Status s;
  // do CFG1 first when functions are done
  s = setPrescaler(config_.baudPrescaler); // FIX TO BE IN CFG1
  if (s != Status::OK)
    return s;

  // CFG2
  s = setBaudRate(config_.baud);
  if (s != Status::OK)
    return s;
  // set Master mode
  if (config_.isMaster) hspi_->CFG2 |= SPI_CFG2_MASTER;
  else hspi_->CFG2 &= ~SPI_CFG2_MASTER;
  s = setSSM(config_.ssmEnabled);
  if (s != Status::OK)
    return s;
  s = setClockPhase(config_.cpha);
  if (s != Status::OK)
    return s;
  s = setClockPolarity(config_.cpol);
  if (s != Status::OK)
    return s;
  s = setLsbFirst(config_.lsbFirst);
  if (s != Status::OK)
    return s;
  s = setComm(config_.mode);
  if (s != Status::OK)
    return s;

  hspi_->CR1 |= SPI_CR1_SPE_Msk;
  return Status::OK;
}
bool STM32SPI::rxPacketAvailable() const noexcept {
  return (hspi_->SR & SPI_SR_RXP_Msk) != 0;
}
bool STM32SPI::txSpaceAvailable() const noexcept {
  return (hspi_->SR & SPI_SR_TXP_Msk) != 0;
}
bool STM32SPI::transmissionComplete() const noexcept {
  return (hspi_->SR & SPI_SR_EOT) != 0;
}
Status STM32SPI::read(uint8_t *rx, uint16_t len) const noexcept {
  if (rx == nullptr || len == 0)
    return Status::ERR; // also check len > MAX_SPI_TRANSFER

  Status s = csLow();
  if (s != Status::OK)
    return s;

  hspi_->CR1 &= ~SPI_CR1_SPE;
  hspi_->CR2 = len & SPI_CR2_TSIZE_Msk;
  hspi_->CR1 |= SPI_CR1_SPE;
  hspi_->CR1 |= SPI_CR1_CSTART;

  constexpr uint32_t MAX_ITER = 1000000;
  uint32_t timeout = 0;

  volatile uint8_t *txReg = reinterpret_cast<volatile uint8_t *>(&hspi_->TXDR);
  volatile const uint8_t *rxReg =
      reinterpret_cast<volatile const uint8_t *>(&hspi_->RXDR);

  for (uint32_t i = 0; i < len; i++) {
    timeout = 0;
    while (!txSpaceAvailable()) {
      if (++timeout > MAX_ITER) {
        csHigh();
        return Status::ERR;
      }
    }
    *txReg = 0x00;
    timeout = 0;
    while (!rxPacketAvailable()) {
      if (++timeout > MAX_ITER) {
        csHigh();
        return Status::ERR;
      }
    }
    rx[i] = *rxReg;
  }

  timeout = 0;
  while (!transmissionComplete()) {
    if (++timeout > MAX_ITER) {
      csHigh();
      return Status::ERR;
    }
  }

  hspi_->IFCR |= SPI_IFCR_EOTC;
  csHigh();
  return Status::OK;
}
Status STM32SPI::write(uint8_t *tx, uint16_t len) const noexcept {
  if (tx == nullptr || len == 0)
    return Status::ERR; // also check len > MAX_SPI_TRANSFER

  Status s = csLow();
  if (s != Status::OK)
    return s;

  hspi_->CR1 &= ~SPI_CR1_SPE;
  hspi_->CR2 = len & SPI_CR2_TSIZE_Msk;
  hspi_->CR1 |= SPI_CR1_SPE;
  hspi_->CR1 |= SPI_CR1_CSTART;

  constexpr uint32_t MAX_ITER = 1000000;
  uint32_t timeout = 0;

  volatile uint8_t *txReg = reinterpret_cast<volatile uint8_t *>(&hspi_->TXDR);
  volatile const uint8_t *rxReg =
      reinterpret_cast<volatile const uint8_t *>(&hspi_->RXDR);

  for (uint32_t i = 0; i < len; i++) {
    timeout = 0;
    while (!txSpaceAvailable()) {
      if (++timeout > MAX_ITER) {
        csHigh();
        return Status::ERR;
      }
    }
    *txReg = tx[i];
    timeout = 0;
    while (!rxPacketAvailable()) {
      if (++timeout > MAX_ITER) {
        csHigh();
        return Status::ERR;
      }
    }
    (void)*rxReg; // research a bit on this
  }

  timeout = 0;
  while (!transmissionComplete()) {
    if (++timeout > MAX_ITER) {
      csHigh();
      return Status::ERR;
    }
  }

  hspi_->IFCR |= SPI_IFCR_EOTC;
  csHigh();
  return Status::OK;
}
Status STM32SPI::transfer(uint8_t *tx, uint8_t *rx,
                          uint16_t len) const noexcept {
  if (tx == nullptr || rx == nullptr || len == 0)
    return Status::ERR; // also check len > MAX_SPI_TRANSFER

  Status s = csLow();
  if (s != Status::OK)
    return s;

  hspi_->CR1 &= ~SPI_CR1_SPE;
  hspi_->CR2 = len & SPI_CR2_TSIZE_Msk;
  hspi_->CR1 |= SPI_CR1_SPE;
  hspi_->CR1 |= SPI_CR1_CSTART;

  constexpr uint32_t MAX_ITER = 1000000;
  uint32_t timeout = 0;

  volatile uint8_t *txReg = reinterpret_cast<volatile uint8_t *>(&hspi_->TXDR);
  volatile const uint8_t *rxReg =
      reinterpret_cast<volatile const uint8_t *>(&hspi_->RXDR);

  for (uint32_t i = 0; i < len; i++) {
    timeout = 0;
    while (!txSpaceAvailable()) {
      if (++timeout > MAX_ITER) {
        csHigh();
        return Status::ERR;
      }
    }
    *txReg = tx[i];
    timeout = 0;
    while (!rxPacketAvailable()) {
      if (++timeout > MAX_ITER) {
        csHigh();
        return Status::ERR;
      }
    }
    rx[i] = *rxReg;
  }

  timeout = 0;
  while (!transmissionComplete()) {
    if (++timeout > MAX_ITER) {
      csHigh();
      return Status::ERR;
    }
  }

  hspi_->IFCR |= SPI_IFCR_EOTC;
  csHigh();
  return Status::OK;
}

Status STM32SPI::csLow() const noexcept {
  if (isSpiBusy())
    return Status::BUSY;
  constexpr unsigned BSRR_RESET_SHIFT = 16;
  csPort_->BSRR = static_cast<uint32_t>(csPin_) << BSRR_RESET_SHIFT;

  return Status::OK;
}

Status STM32SPI::csHigh() const noexcept {
  if (isSpiBusy())
    return Status::BUSY;
  csPort_->BSRR = static_cast<uint32_t>(csPin_);

  return Status::OK;
}
bool STM32SPI::isSpiBusy() const noexcept {
  return (hspi_->SR & SPI_SR_BSY_Msk) != 0;
}
Status STM32SPI::setSSM(bool isOn) noexcept {
  if (hspi_->CR1 & SPI_CR1_SPE) {
    return Status::BUSY;
  }

  hspi_->CFG2 &= ~SPI_CFG2_SSM_Msk;
  hspi_->CFG2 |= (isOn ? 1u : 0U) << SPI_CFG2_SSM_Pos;

  if (isOn) {
    hspi_->CR1 |= SPI_CR1_SSI_Msk;
  } else {
    hspi_->CR1 &= ~SPI_CR1_SSI_Msk;
  }

  // add SSIOP

  return Status::OK;
}
Status STM32SPI::setClockPhase(Config::CPHA value) noexcept {
  if (hspi_->CR1 & SPI_CR1_SPE) {
    return Status::BUSY;
  }
  hspi_->CFG2 &= ~SPI_CFG2_CPHA_Msk;
  hspi_->CFG2 |= static_cast<uint32_t>(value) << SPI_CFG2_CPHA_Pos;
  config_.cpha = value;

  return Status::OK;
}
Status STM32SPI::setClockPolarity(Config::CPOL value) noexcept {
  if (hspi_->CR1 & SPI_CR1_SPE) {
    return Status::BUSY;
  }
  hspi_->CFG2 &= ~SPI_CFG2_CPOL_Msk;
  hspi_->CFG2 |= static_cast<uint32_t>(value) << SPI_CFG2_CPOL_Pos;
  config_.cpol = value;

  return Status::OK;
}
Status STM32SPI::setLsbFirst(bool isTrue) noexcept {
  if (hspi_->CR1 & SPI_CR1_SPE) {
    return Status::BUSY;
  }
  hspi_->CFG2 &= ~SPI_CFG2_LSBFRST_Msk;
  hspi_->CFG2 |= static_cast<uint32_t>(isTrue) << SPI_CFG2_LSBFRST_Pos;
  config_.lsbFirst = isTrue;

  return Status::OK;
}
Status STM32SPI::setComm(Config::Comm mode) noexcept {
  if (hspi_->CR1 & SPI_CR1_SPE) {
    return Status::BUSY;
  }

  hspi_->CFG2 &= ~SPI_CFG2_COMM_Msk;
  hspi_->CFG2 |= static_cast<uint32_t>(mode) << SPI_CFG2_COMM_Pos;
  config_.mode = mode;

  return Status::OK;
}
