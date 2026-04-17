#ifndef COMM_SPI_HPP
#define COMM_SPI_HPP

#include "ISPI.hpp"
#include "stm32h7xx_hal.h"

namespace Comm
{

class PinMux;
class DigitalOut;

class SPI final : public ISPI
{
public:
	enum class Instance : uint8_t
	{
		Spi1 = 0u,
		Spi2 = 1U,
		Spi3 = 2U,
		Spi4 = 3U,
		Spi5 = 4U,
		Spi6 = 5U
	};

    explicit SPI(Instance instance, const Config& config, const PinMux& pm) noexcept;
    Err Init() noexcept override;
	Err Deinit() noexcept override;
	Err Transmit(const uint8_t* data, size_t size, uint32_t timeout_ms) noexcept override;
	Err Receive(uint8_t* data, size_t size, uint32_t timeout_ms) noexcept override;
	bool IsBusy() const noexcept override;
	bool IsInitialized() const noexcept override;
	Err Reset() noexcept override;
    Err TransmitReceive(const void* txData, void* rxData, size_t size, uint32_t timeout_ms) noexcept override;
	Err Configure(const Config& config) noexcept override;
	Config GetConfig() const noexcept override;
	Err AssertCS() noexcept override;
	Err DeassertCS() noexcept override;

    ~SPI() noexcept override = default;

private:
    const Instance instance_;
    SPI_HandleTypeDef* hspi_;
    const PinMux& pm_;
    bool initialized_ {false};

    [[nodiscard]] Err VerifyPinsReady()  const noexcept;
	[[nodiscard]] Err VerifyCsPinReady() const noexcept;

    SPI() = delete;
    SPI(const SPI&) = delete;
    SPI& operator=(const SPI&) = delete;
};

} // namespace Comm

#endif // COMM_SPI_HPP
