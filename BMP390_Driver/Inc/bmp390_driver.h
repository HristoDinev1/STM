#pragma once

#include "ISPI.h"
#include <cstdint>
#include "IBaro.h"
#include "bmp390_regs.h"
#include "bmp390_config.h"
#include "timing.h"

namespace BMP390
{

class Driver : public ::IPress, ::ITemp
{

public:
	Driver() = delete;
	Driver(const Driver&) = delete;
	Driver& operator=(const Driver&) = delete;
	explicit Driver(ISPI& spi) noexcept;

	Status init() noexcept;
	Status getTemp(float& out_celsius) const noexcept;
	Status getPress(float& out_pascal) const noexcept;

	~Driver() = default;
private:
	struct CalibrationData
	{
		uint16_t T1;
		uint16_t  T2;
		int8_t   T3;

		int16_t  P1;
		int16_t  P2;
		int8_t   P3;
		int8_t   P4;
		uint16_t P5;
		uint16_t P6;
		int8_t   P7;
		int8_t   P8;
		int16_t  P9;
		int8_t   P10;
		int8_t   P11;
	};
	Status checkChipId() const noexcept;
	Status softReset()const noexcept;
	Status setIIRFilter(IIR_Config coefficient)const noexcept; //Note:: pitai filip za IIR po vreme na izlitane i po vreme na polet (accelerometer ?)
	Status setMode(Mode mode)noexcept;
	Status configureSpi(SPI_Config conf) const noexcept; // configure for 4 wires
	Status setOSRTemp(osr_t ratio) const noexcept;
	Status setOSRPress(osr_p ratio) const noexcept;

	Status waitTempReady() const noexcept;
	Status waitPressReady() const noexcept;


	Status readBits(Register reg, uint8_t mask, uint8_t& out_value) const noexcept;
	Status writeBits(Register reg, uint8_t mask, uint8_t in_value) const noexcept;
	Status readRegister8(Register reg, uint8_t& out_value) const noexcept;
	Status writeRegister8(Register reg, uint8_t in_value) const noexcept;
	Status readRegister16(Register reg, uint16_t& out_value) const noexcept;
	Status readRegister24(Register reg, uint32_t& out_value) const noexcept;
	Status readRegisterS8(Register reg, int8_t& out_value) const noexcept;
	Status readRegisterS16(Register reg, int16_t& out_value) const noexcept;

	// Reads up to 3 bytes (BMP390 hardware limit).
	// len must be 1..3. Larger values return ERR_BAD_PARAM.
	Status readMulti(Register reg, uint8_t* buf, uint8_t len) const noexcept;

	Status readCalibration() noexcept;

	Status readRawTemp(uint32_t& outTemp) const noexcept;
	Status readRawPress(uint32_t& outPress) const noexcept;
	Status compensateTemperature(int32_t uncomp_temp, float& out_temp) const noexcept; //outside static ?
	Status compensatePressure(int32_t uncomp_press, float& out_press) const noexcept;


private:
	ISPI& spi_;
	mutable int64_t t_lin_;
	CalibrationData calib_;
	Mode currentMode;
	bool initialized_;
};
}
