#include "istm32_spi.h"
#include "stm32h7xx_hal.h"


STM32SPI::STM32SPI(SPI_HandleTypeDef* handle,
                   GPIO_TypeDef* cs_port,
                   uint16_t cs_pin) noexcept :
    handle_(handle),
    cs_port_(cs_port),
    cs_pin_(cs_pin)
{
}

void STM32SPI::csLow() noexcept
{
    HAL_GPIO_WritePin(cs_port_, cs_pin_, GPIO_PIN_RESET);
}

void STM32SPI::csHigh() noexcept
{
    HAL_GPIO_WritePin(cs_port_, cs_pin_, GPIO_PIN_SET);
}

SpiStatus STM32SPI::read(uint8_t* rx, uint16_t len) noexcept
{
    if (HAL_SPI_Receive(handle_, rx, len, 10) == HAL_OK)
    {
        return SpiStatus::OK;
    }
    return SpiStatus::ERROR;
}

SpiStatus STM32SPI::write(uint8_t* tx, uint16_t len) noexcept
{
    if (HAL_SPI_Transmit(handle_, tx, len, 10) == HAL_OK)
    {
        return SpiStatus::OK;
    }
    return SpiStatus::ERROR;
}

SpiStatus STM32SPI::transfer(uint8_t* tx, uint8_t* rx, uint16_t len) noexcept
{
    if (HAL_SPI_TransmitReceive(handle_, tx, rx, len, 10) == HAL_OK)
    {
        return SpiStatus::OK;
    }
    return SpiStatus::ERROR;
}
