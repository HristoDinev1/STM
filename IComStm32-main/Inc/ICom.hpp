#ifndef ICOM_HPP
#define ICOM_HPP

#include <cstdint>
#include <cstddef>

namespace Comm
{



class ICom
{
public:
	enum class Err : uint8_t
	{
		NONE = 0U,
		ERROR = 1U,
		BUSY = 2U,
		TIMEOUT = 3U,
		INVALID_PARAM =4U
	};

    [[nodiscard]] virtual Err Init() noexcept = 0;
    [[nodiscard]] virtual Err Deinit() noexcept = 0;
    [[nodiscard]] virtual Err Transmit(const uint8_t* data, size_t size, uint32_t timeout_ms) noexcept = 0;
    [[nodiscard]] virtual Err Receive(uint8_t* data, size_t size, uint32_t timeout_ms) noexcept = 0;
    [[nodiscard]] virtual bool IsBusy() const noexcept = 0;
    [[nodiscard]] virtual bool IsInitialized() const noexcept = 0;
    [[nodiscard]] virtual Err Reset() noexcept = 0;

    virtual ~ICom() = default;
protected:

    ICom() = default;
    ICom(const ICom&) = delete;
    ICom& operator=(const ICom&) = delete;
};

}
#endif
