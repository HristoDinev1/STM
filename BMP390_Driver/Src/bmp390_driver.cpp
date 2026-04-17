/*
* bmp390_driver.cpp
*
*  Created on: Nov 18, 2025
*      Author: icodi
*/

//Known issues - even if default mode is normal it will always go for forced mode
//No real dynamic setting for the IIR filter
//Not configured currently for its best behaviour - just random values for testing

#include "bmp390_driver.h"

namespace BMP390
{

Driver::Driver(ISPI& spi) noexcept : spi_(spi),t_lin_(0),calib_(),currentMode(Mode::SLEEP) , initialized_(false){}

Status Driver::init()noexcept
{
	Status s = softReset();
	if (s != Status::OK)
	{
		return s;
	}
	s = checkChipId();
	if (s != Status::OK)
	{
		return s;
	}
	s = configureSpi(BMP390_CONFIG::SPI_MODE);
	if (s != Status::OK)
	{
		return s;
	}
	s = readCalibration();
	if (s != Status::OK)
	{
		return s;
	}
	s = setOSRTemp(BMP390_CONFIG::OVERSAMPLING_TEMP);
	if (s != Status::OK)
	{
		return s;
	}
	s = setOSRPress(BMP390_CONFIG::OVERSAMPLING_PRESS);
	if (s != Status::OK)
	{
		return s;
	}
	s = setIIRFilter(BMP390_CONFIG::IIR_COEFF);
	if (s != Status::OK)
	{
		return s;
	}
	uint32_t initialTemp = 0;
	s = readRawTemp(initialTemp);
	if (s != Status::OK)
	{
		return s;
	}
	t_lin_ = static_cast<int64_t>(initialTemp);
	s = setMode(BMP390_CONFIG::DEFAULT_MODE);
	if (s != Status::OK)
	{
		return s;
	}
	currentMode = BMP390_CONFIG::DEFAULT_MODE;

	initialized_ = true;
	return Status::OK;
}

Status Driver::checkChipId() const noexcept
{
	uint8_t chipId;
	Status s = readRegister8(Register::CHIP_ID, chipId);
	if (s != Status::OK)
	{
		return s;
	}
	if (chipId != EXPECTED_CHIP_ID)
	{
		return Status::ERR_BAD_CHIPID;
	}
	return Status::OK;
}
Status Driver::getTemp(float& out_celsius) const noexcept
{
	if(!initialized_) return Status::ERR_NOT_INITIALIZED;
	Status s= Status::OK;
	uint32_t rawTemp = 0U;
	float compensatedTemp;
	s = setMode(Mode::FORCED);
	if (s != Status::OK)
	{
		(void)setMode(Mode::SLEEP);
		return s;
	}
	s = waitTempReady();
	if (s != Status::OK)
	{
		(void)setMode(Mode::SLEEP);
		return s;
	}
	s = readRawTemp(rawTemp);
	if (s != Status::OK)
	{
		(void)setMode(Mode::SLEEP);
		return s;
	}
	s = compensateTemperature(static_cast<int32_t>(rawTemp), compensatedTemp);
	if (s != Status::OK)
	{
		(void)setMode(Mode::SLEEP);
		return s;
	}
	out_celsius = compensatedTemp;

	s = setMode(Mode::SLEEP);
	if (s != Status::OK)
	{
		return s;
	}

	return Status::OK;

}
Status Driver::getPress(float& out_pascal) const noexcept
{
	if(!initialized_) return Status::ERR_NOT_INITIALIZED;
	Status s= Status::OK;
	uint32_t rawPress = 0;
	float compensatedPress = 0;
	s = setMode(Mode::FORCED);
	if (s != Status::OK)
	{
		(void)setMode(Mode::SLEEP);
		return s;
	}
	s = waitPressReady();
	if (s != Status::OK)
	{
		(void)setMode(Mode::SLEEP);
		return s;
	}
	s = readRawPress(rawPress);
	if (s != Status::OK)
	{
		(void)setMode(Mode::SLEEP);
		return s;
	}
	s = compensatePressure(static_cast<int32_t>(rawPress), compensatedPress);
	if (s != Status::OK)
	{
		(void)setMode(Mode::SLEEP);
		return s;
	}

	out_pascal = compensatedPress;

	s = setMode(Mode::SLEEP);
	if (s != Status::OK)
	{
		return s;
	}
	return Status::OK;

}

Status Driver::readMulti(Register reg, uint8_t* buf, uint8_t len) const noexcept
{
	if (buf == nullptr || (len == 0U) || (len > 3U))
	{
		return Status::ERR_BAD_PARAM;
	}
	const uint8_t total = static_cast<uint8_t>(len + 1U); //check on this again
	constexpr uint8_t READ_WRITE_SIZE = 4;
	if (total > READ_WRITE_SIZE) // so MISRA is happy
	{
		return Status::ERR_BAD_PARAM;
	}
	uint8_t tx[READ_WRITE_SIZE] {0U};
	uint8_t rx[READ_WRITE_SIZE] {0U};
	tx[0] = static_cast<uint8_t>(reg) | READ_CMD_MASK;
	if (spi_.transfer(tx, rx, total) != SpiStatus::OK)
	{
		return Status::ERR_COMM;
	}
	for (uint8_t i = 0U; i < len; i++)
	{
		buf[i] = rx[i + 1U];
	}

	return Status::OK;
}

Status Driver::readBits(Register reg, uint8_t mask, uint8_t& out_value) const noexcept
{
	uint8_t regValue = 0U;
	Status s = readRegister8(reg,regValue);
	if(s != Status::OK)
	{
		return s;
	}

	out_value = static_cast<uint8_t>(regValue & mask); //compiler stuff
	return Status::OK;

}
Status Driver::writeBits(Register reg, uint8_t mask, uint8_t out_value) const noexcept
{
	uint8_t regValue = 0U;
	Status s = readRegister8(reg, regValue);
	if(s != Status::OK)
	{
		return s;
	}
	regValue = (regValue & ~mask)  | (out_value & mask);
	return writeRegister8(reg, regValue);
}

Status Driver::readRegister8(Register reg, uint8_t& out_value) const noexcept
{
	uint8_t buf[1U] {0U};
	Status s = readMulti(reg, buf, 1U);

	if(s!= Status::OK)
	{
		return s;
	}
	out_value = buf[0];
	return Status::OK;
}
Status Driver::writeRegister8(Register reg, uint8_t in_value) const noexcept
{
	constexpr uint8_t WRITE_LEN = 2U; //
	uint8_t tx[WRITE_LEN]{ 0U };

	tx[WRITE_CMD_INDEX] = static_cast<uint8_t>(reg) | WRITE_CMD_MASK;
	tx[WRITE_DATA_INDEX] = in_value;

	if (spi_.write(tx, WRITE_LEN) != SpiStatus::OK)
	{
		return Status::ERR_COMM;
	}

	return Status::OK;
}
Status Driver::readRegister16(Register reg, uint16_t& out_value) const noexcept
{
	uint8_t buf[2U] {0U}; //maybe go constexpr uint8_t ?
	Status s = readMulti(reg, buf, 2U);

	if(s!= Status::OK)
	{
		return s;
	}
	out_value = (static_cast<uint16_t>(buf[0]))
			  | (static_cast<uint16_t>(buf[1])<<8U);
	return Status::OK;
}
Status Driver::readRegister24(Register reg, uint32_t& out_value) const noexcept
{
	uint8_t buf[3U] {0U};
	Status s = readMulti(reg, buf, 3U);

	if(s!= Status::OK)
	{
		return s;
	}

	out_value = (static_cast<uint32_t>(buf[0])<<16U)
				| (static_cast<uint32_t>(buf[1])<< 8U)
				| (static_cast<uint32_t>(buf[2]));
	return Status::OK;

}

Status Driver::readRegisterS8(Register reg, int8_t& out_value) const noexcept
{
	uint8_t regValue = 0U;
	Status s = readRegister8(reg, regValue);
	if (s != Status::OK)
	{
		return s;
	}

	out_value = static_cast<int8_t>(regValue);
	return Status::OK;
}

Status Driver::readRegisterS16(Register reg, int16_t& out_value) const noexcept
{
	uint16_t regValue = 0U;
	Status s = readRegister16(reg, regValue);
	if (s != Status::OK)
	{
		return s;
	}

	out_value = static_cast<int16_t>(regValue);
	return Status::OK;
}

Status Driver::readCalibration() noexcept
{ // Looks clean - reading them in one go will be faster but tjis is safer
	Status st;

	st = readRegisterS8(Register::CALIB_P11, calib_.P11);
	if (st != Status::OK) return st;

	st = readRegisterS8(Register::CALIB_P10, calib_.P10);
	if (st != Status::OK) return st;

	st = readRegisterS16(Register::CALIB_P9, calib_.P9);
	if (st != Status::OK) return st;

	st = readRegisterS8(Register::CALIB_P8, calib_.P8);
	if (st != Status::OK) return st;

	st = readRegisterS8(Register::CALIB_P7, calib_.P7);
	if (st != Status::OK) return st;

	st = readRegister16(Register::CALIB_P6, calib_.P6);
	if (st != Status::OK) return st;

	st = readRegister16(Register::CALIB_P5, calib_.P5);
	if (st != Status::OK) return st;

	st = readRegisterS8(Register::CALIB_P4, calib_.P4);
	if (st != Status::OK) return st;

	st = readRegisterS8(Register::CALIB_P3, calib_.P3);
	if (st != Status::OK) return st;

	st = readRegisterS16(Register::CALIB_P2, calib_.P2);
	if (st != Status::OK) return st;

	st = readRegisterS16(Register::CALIB_P1, calib_.P1);
	if (st != Status::OK) return st;

	st = readRegisterS8(Register::CALIB_T3, calib_.T3);
	if (st != Status::OK) return st;

	st = readRegister16(Register::CALIB_T2, calib_.T2);
	if (st != Status::OK) return st;

	st = readRegister16(Register::CALIB_T1, calib_.T1);
	if (st != Status::OK) return st;

	return Status::OK;
}
Status Driver::readRawTemp(uint32_t& outTemp) const noexcept
{
	uint32_t temp;
	Status s = readRegister24(Register::TEMPERATURE_DATA, temp);
	if (s != Status::OK)
	{
		return s;
	}
	outTemp = temp;
	return Status::OK;
}
Status Driver::readRawPress(uint32_t& outPress) const noexcept
{
	uint32_t press;
	Status s = readRegister24(Register::PRESSURE_DATA, press);
	if (s != Status::OK)
	{
		return s;
	}
	outPress = press;
	return Status::OK;
}

Status Driver::softReset()const noexcept
{
	Status s = Status::OK;
	s = writeRegister8(Register::CMD, SOFT_RESET_COMMAND);
	if (s != Status::OK)
	{
		return s;
	}

	timing::delay_us(BMP390_CONFIG::SOFT_RESET_DELAY_US);
	constexpr uint8_t CMD_RDY_MASK = 1 << 4;
	uint8_t cmd_rdy_byte = 0;
	for (uint32_t i = 0; i < BMP390_CONFIG::MAX_WAIT_ITER; i++)
	{
		s = readRegister8(Register::STATUS_REG, cmd_rdy_byte);
		if (s != Status::OK)
		{
			return s;
		}
		if (cmd_rdy_byte & CMD_RDY_MASK)
		{
			return Status::OK;
		}
		timing::delay_us(BMP390_CONFIG::WAIT_STEP_US); ////BMP390_CONFIG::WAIT_STEP_US
	}
	return Status::ERR_TIMEOUT;

}
Status Driver::setIIRFilter(IIR_Config coefficient) const noexcept
{
    return writeBits(Register::CONFIG,IIR_MASK,static_cast<uint8_t>(coefficient));
}
Status Driver::setMode(Mode mode) noexcept
{
	Status s = writeBits(Register::PWR_CTRL,MODE_MASK,static_cast<uint8_t>(mode));
	if (s != Status::OK)
	{
		return s;
	}

	timing::delay_us(5);
	this->currentMode = mode;
	return Status::OK;
}

Status Driver::configureSpi(SPI_Config conf) const noexcept
{
    return writeBits(Register::IF_CONF,SPI_MODE_MASK,static_cast<uint8_t>(conf));
}


Status Driver::waitTempReady() const noexcept //Implement in stm32Cube
{
	Status s= Status::OK;
	uint8_t temp_rdy_byte = 0;
	for (uint32_t i = 0; i < BMP390_CONFIG::MAX_WAIT_ITER; i++) // 10 000 -> ~250 ms time
	{
		s = readRegister8(Register::STATUS_REG, temp_rdy_byte);
		if (s != Status::OK)
		{
			return s;
		}
		if ((STATUS_TEMP_RDY_MASK & temp_rdy_byte)!= 0U)
		{
			return Status::OK;
		}
		timing::delay_us(BMP390_CONFIG::WAIT_STEP_US);
	}

	return Status::ERR_TIMEOUT;

}
Status Driver::waitPressReady() const noexcept//Implement in stm32Cube
{
	Status s= Status::OK;
	uint8_t press_rdy_byte = 0;
	for (uint32_t i = 0; i < BMP390_CONFIG::MAX_WAIT_ITER; i++) // 10 000 -> ~250 ms time
	{
		s = readRegister8(Register::STATUS_REG, press_rdy_byte);
		if (s != Status::OK)
		{
			return s;
		}
		if ((STATUS_PRESS_RDY_MASK  & press_rdy_byte) != 0U)
		{
			return Status::OK;
		}
		timing::delay_us(BMP390_CONFIG::WAIT_STEP_US);
	}

	return Status::ERR_TIMEOUT;
}

Status Driver::setOSRTemp(osr_t ratio) const noexcept
{
    const uint8_t value = static_cast<uint8_t>(ratio) << OSR_T_POS;

    return writeBits(Register::OSR, OSR_T_MASK,value);
}

Status Driver::setOSRPress(osr_p ratio) const noexcept
{
    const uint8_t value = static_cast<uint8_t>(ratio) << OSR_P_POS;

    return writeBits(Register::OSR,OSR_P_MASK,value);
}



//REVIEW THEEESE
Status Driver::compensateTemperature(int32_t uncomp_temp, float& out_temp) const noexcept
{
	int64_t partial_data1;
	int64_t partial_data2;
	int64_t partial_data3;
	int64_t partial_data4;
	int64_t partial_data5;
	int64_t partial_data6;
	int64_t comp_temp;

	partial_data1 = static_cast<int64_t>(uncomp_temp)
		- (INT64_C(256) * static_cast<int64_t>(calib_.T1));

	partial_data2 = static_cast<int64_t>(calib_.T2) * partial_data1;
	partial_data3 = partial_data1 * partial_data1;
	partial_data4 = partial_data3 * static_cast<int64_t>(calib_.T3);

	// 262144 = 2^18, 4294967296 = 2^32
	partial_data5 = (partial_data2 * INT64_C(262144)) + partial_data4;
	partial_data6 = partial_data5 / INT64_C(4294967296);

	// store t_lin for pressure comp
	t_lin_ = partial_data6;

	// 25/16384 = 25 / 2^14 ; result is temperature * 100 (°C * 100)
	comp_temp = (partial_data6 * INT64_C(25)) / INT64_C(16384);

	// You can clamp here if you define min/max constants, but
	// I'll just convert directly to float °C:
	out_temp = static_cast<float>(comp_temp) / 100.0F;

	return Status::OK;
}
Status Driver::compensatePressure(int32_t uncomp_press, float& out_press) const noexcept
{
	const int64_t t_lin = t_lin_;  // from last temperature compensation

	int64_t partial_data1;
	int64_t partial_data2;
	int64_t partial_data3;
	int64_t partial_data4;
	int64_t partial_data5;
	int64_t partial_data6;
	int64_t offset;
	int64_t sensitivity;

	// t_lin^2 and t_lin^3 terms
	partial_data1 = t_lin * t_lin;
	partial_data2 = partial_data1 / INT64_C(64);
	partial_data3 = (partial_data2 * t_lin) / INT64_C(256);

	partial_data4 = (static_cast<int64_t>(calib_.P8) * partial_data3) / INT64_C(32);
	partial_data5 = (static_cast<int64_t>(calib_.P7) * partial_data1) * INT64_C(16);
	partial_data6 = (static_cast<int64_t>(calib_.P6) * t_lin) * INT64_C(4194304);

	offset = (static_cast<int64_t>(calib_.P5) * INT64_C(140737488355328))
		+ partial_data4 + partial_data5 + partial_data6;

	partial_data2 = (static_cast<int64_t>(calib_.P4) * partial_data3) / INT64_C(32);
	partial_data4 = (static_cast<int64_t>(calib_.P3) * partial_data1) * INT64_C(4);
	partial_data5 = (static_cast<int64_t>(calib_.P2) - INT64_C(16384))
		* t_lin * INT64_C(2097152);

	sensitivity = ((static_cast<int64_t>(calib_.P1) - INT64_C(16384))
		* INT64_C(70368744177664))
		+ partial_data2 + partial_data4 + partial_data5;

	partial_data1 = (sensitivity / INT64_C(16777216))
		* static_cast<int64_t>(uncomp_press);

	partial_data2 = static_cast<int64_t>(calib_.P10) * t_lin;
	partial_data3 = partial_data2 + (INT64_C(65536) * static_cast<int64_t>(calib_.P9));
	partial_data4 = (partial_data3 * static_cast<int64_t>(uncomp_press)) / INT64_C(8192);

	// overflow-safe trick from Bosch: divide by 10, then multiply back
	partial_data5 = (static_cast<int64_t>(uncomp_press) * (partial_data4 / INT64_C(10)))
		/ INT64_C(512);
	partial_data5 = partial_data5 * INT64_C(10);

	partial_data6 = static_cast<int64_t>(uncomp_press) * static_cast<int64_t>(uncomp_press);
	partial_data2 = (static_cast<int64_t>(calib_.P11) * partial_data6) / INT64_C(65536);
	partial_data3 = (partial_data2 * static_cast<int64_t>(uncomp_press)) / INT64_C(128);

	partial_data4 = (offset / INT64_C(4))
		+ partial_data1 + partial_data5 + partial_data3;

	// 1099511627776 = 2^40; result is pressure * 100 (Pa * 100)
	uint64_t comp_press = (static_cast<uint64_t>(partial_data4) * UINT64_C(25))
		/ UINT64_C(1099511627776);

	out_press = static_cast<float>(comp_press) / 100.0F;

	return Status::OK;
}

}
