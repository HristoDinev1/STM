#include "Spi.hpp"
#include "spi.h"

namespace Comm {

static SPI_HandleTypeDef *getInstance(SPI::Instance instance) {
  switch (instance) {
  case SPI::Instance::Spi1:
    return &hspi1; // Assuming hspi1 is defined in spi.h/c
  default:
    return nullptr;
  }
}

SPI::SPI(Instance instance, const Config &config, const PinMux &pm) noexcept
    : ISPI(config), instance_(instance), pm_(pm), initialized_(false) {
  hspi_ = getInstance(instance);
}

ICom::Err SPI::Init() noexcept {
  if (initialized_) {
    return ICom::Err::NONE;
  }

  if (hspi_ == nullptr) {
    return ICom::Err::INVALID_PARAM;
  }

  if (HAL_SPI_Init(hspi_) != HAL_OK) {
    return ICom::Err::ERROR;
  }

  initialized_ = true;
  return ICom::Err::NONE;
}

ICom::Err SPI::Deinit() noexcept {
  if (!initialized_) {
    return ICom::Err::NONE;
  }

  if (HAL_SPI_DeInit(hspi_) != HAL_OK) {
    return ICom::Err::ERROR;
  }

  initialized_ = false;
  return ICom::Err::NONE;
}

bool SPI::IsBusy() const noexcept {
  if (hspi_ == nullptr) {
    return false;
  }
  return HAL_SPI_GetState(hspi_) == HAL_SPI_STATE_BUSY;
}

bool SPI::IsInitialized() const noexcept { return initialized_; }

ICom::Err SPI::Reset() noexcept {
  ICom::Err err = Deinit();
  if (err != ICom::Err::NONE) {
    return err;
  }
  return Init();
}

ICom::Err SPI::Configure(const Config &config) noexcept {
  if (hspi_ == nullptr) {
    return ICom::Err::INVALID_PARAM;
  }

  // Configure logic here if needed, or re-init with new config
  if (HAL_SPI_Init(hspi_) != HAL_OK) {
    return ICom::Err::ERROR;
  }

  return ICom::Err::NONE;
}

ISPI::Config SPI::GetConfig() const noexcept { return config_; }

ICom::Err SPI::AssertCS() noexcept {
  ICom::Err err = VerifyCsPinReady();
  if (err != ICom::Err::NONE) {
    return err;
  }

  // TODO: Implement hardware-specific CS control (e.g., via GPIO or DigitalOut)
  return ICom::Err::NONE;
}

ICom::Err SPI::DeassertCS() noexcept {
  ICom::Err err = VerifyCsPinReady();
  if (err != ICom::Err::NONE) {
    return err;
  }

  // TODO: Implement hardware-specific CS control (e.g., via GPIO or DigitalOut)
  return ICom::Err::NONE;
}

ICom::Err SPI::Transmit(const uint8_t *data, size_t size,
                        uint32_t timeout_ms) noexcept {
  if (data == nullptr || size == 0U) {
    return ICom::Err::INVALID_PARAM;
  }

  ICom::Err err = VerifyPinsReady();
  if (err != ICom::Err::NONE) {
    return err;
  }

  HAL_StatusTypeDef status =
      HAL_SPI_Transmit(hspi_, const_cast<uint8_t *>(data),
                       static_cast<uint16_t>(size), timeout_ms);

  if (status == HAL_TIMEOUT) {
    return ICom::Err::TIMEOUT;
  }
  if (status != HAL_OK) {
    return ICom::Err::ERROR;
  }

  return ICom::Err::NONE;
}

ICom::Err SPI::Receive(uint8_t *data, size_t size,
                       uint32_t timeout_ms) noexcept {
  if (data == nullptr || size == 0U) {
    return ICom::Err::INVALID_PARAM;
  }

  ICom::Err err = VerifyPinsReady();
  if (err != ICom::Err::NONE) {
    return err;
  }

  HAL_StatusTypeDef status =
      HAL_SPI_Receive(hspi_, data, static_cast<uint16_t>(size), timeout_ms);

  if (status == HAL_TIMEOUT) {
    return ICom::Err::TIMEOUT;
  }
  if (status != HAL_OK) {
    return ICom::Err::ERROR;
  }

  return ICom::Err::NONE;
}

ICom::Err SPI::TransmitReceive(const void *txData, void *rxData, size_t size,
                               uint32_t timeout_ms) noexcept {
  if (txData == nullptr || rxData == nullptr || size == 0U) {
    return ICom::Err::INVALID_PARAM;
  }

  ICom::Err err = VerifyPinsReady();
  if (err != ICom::Err::NONE) {
    return err;
  }

  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(
      hspi_, const_cast<uint8_t *>(static_cast<const uint8_t *>(txData)),
      static_cast<uint8_t *>(rxData), static_cast<uint16_t>(size), timeout_ms);

  if (status == HAL_TIMEOUT) {
    return ICom::Err::TIMEOUT;
  }
  if (status != HAL_OK) {
    return ICom::Err::ERROR;
  }

  return ICom::Err::NONE;
}

ICom::Err SPI::VerifyPinsReady() const noexcept {
  if (!initialized_ || hspi_ == nullptr) {
    return ICom::Err::ERROR;
  }

  if (IsBusy()) {
    return ICom::Err::BUSY;
  }

  return ICom::Err::NONE;
}

ICom::Err SPI::VerifyCsPinReady() const noexcept {
  if (!initialized_) {
    return ICom::Err::ERROR;
  }

  return ICom::Err::NONE;
}

} // namespace Comm
