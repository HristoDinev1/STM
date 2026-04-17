#ifndef STM32SPI_H_
#define STM32SPI_H_

#include "ISPI.h"
#include "stm32h7xx_hal.h"


class STM32SPI : public ISPI
{
public:
    STM32SPI(SPI_HandleTypeDef* handle,
             GPIO_TypeDef* cs_port,
             uint16_t cs_pin) noexcept;

    SpiStatus read(uint8_t* rx, uint16_t len) noexcept override;
    SpiStatus write(uint8_t* tx, uint16_t len) noexcept override;
    SpiStatus transfer(uint8_t* tx, uint8_t* rx, uint16_t len) noexcept override;

    void csLow() noexcept override;
    void csHigh() noexcept override;

private:
    SPI_HandleTypeDef* handle_;
    GPIO_TypeDef* cs_port_;
    uint16_t cs_pin_;
};

#endif
