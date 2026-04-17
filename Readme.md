#ifndef ISPI_H_
#define ISPI_H_

#include <stdint.h>

enum class SpiStatus : uint8_t
{
    OK = 0U,
    ERROR = 1U
};

class ISPI
{
public:
    virtual ~ISPI() = default;

    virtual SpiStatus read(uint8_t* rx, uint16_t len) noexcept = 0;
    virtual SpiStatus write(uint8_t* tx, uint16_t len) noexcept = 0;
    virtual SpiStatus transfer(uint8_t* tx, uint8_t* rx, uint16_t len) noexcept = 0;

    virtual void csLow() noexcept = 0;
    virtual void csHigh() noexcept = 0;
};

#endif
