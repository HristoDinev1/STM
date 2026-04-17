#ifndef ISPI_HPP
#define ISPI_HPP

#include "ICom.hpp"
#include <cstdint>
#include <cstddef>

namespace Comm
{

class ISPI : public ICom
{
public:
	using enum_underlying_t = std::uint8_t;
	enum class WireMode : enum_underlying_t
	{
		THREE_WIRE = 0U,
		FOUR_WIRE = 1U
	};
	enum class Mode : enum_underlying_t
	{
	    MASTER = 0U,
	    SLAVE  = 1U
	};

	enum class ClockPolarity : enum_underlying_t
	{
	    LOW  = 0U,
	    HIGH = 1U
	};

	enum class ClockPhase : enum_underlying_t
	{
	    FIRST_EDGE  = 0U,
	    SECOND_EDGE = 1U
	};

	enum class Protocol : enum_underlying_t
	{
	    MOTOROLA           = 0U,
	    TEXAS_INSTRUMENTS  = 1U
	};

	enum class DataWidth : enum_underlying_t
	{
	    DATA_8BIT  = 0U,
	    DATA_16BIT = 1U
	};

	enum class FirstBit : enum_underlying_t
	{
	    MSB_FIRST = 0U,
	    LSB_FIRST = 1U
	};

	enum class NssMode : enum_underlying_t
	{
	    HARDWARE = 0U,
	    SOFTWARE = 1U
	};
	struct Config final
	{
		WireMode wireMode_;
		Mode mode_;
		ClockPhase clockPhase_;
		ClockPolarity clockPolarity_;
		Protocol protocol_;
		DataWidth dataWidth_;
		FirstBit firstBit_;
		uint32_t speedHz_;
		NssMode nssMode_;

		constexpr Config(WireMode wireMode, Mode mode,ClockPhase clockPhase, ClockPolarity clockPolarity, Protocol protocol, DataWidth dataWidth, FirstBit firstBit, uint32_t speedHz, NssMode nssMode) noexcept :
		wireMode_(wireMode), mode_(mode),clockPhase_(clockPhase), clockPolarity_(clockPolarity), protocol_(protocol), dataWidth_(dataWidth), firstBit_(firstBit), speedHz_(speedHz), nssMode_(nssMode)
		{}
	};

	[[nodiscard]] virtual Err TransmitReceive(const void* txData, void* rxData, size_t size, uint32_t timeout_ms) noexcept = 0;
	[[nodiscard]] virtual Err Configure(const Config& config) noexcept = 0;
	[[nodiscard]] virtual Config GetConfig() const noexcept = 0;

	[[nodiscard]] virtual Err AssertCS() noexcept = 0;
	[[nodiscard]] virtual Err DeassertCS() noexcept = 0;

    virtual ~ISPI() noexcept = default;
protected:
    Config config_;

    constexpr explicit ISPI(const Config& config) noexcept
    	: config_(config) {};
    ISPI(const ISPI&) = delete;
    ISPI& operator=(const ISPI&) = delete;

private:

};

}
#endif
