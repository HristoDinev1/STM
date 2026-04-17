#pragma once
#include <stdint.h>

namespace BMP390
{
	enum class osr_p
	{
		none = 0U, //  == X1
		x2 = 1U,
		x4 = 2U,
		x8 = 3U,
		x16 = 4U,
		x32 = 5U
	};
	enum class osr_t
	{
		none = 0U, //  == X1
		x2 = 1U,
		x4 = 2U,
		x8 = 3U,
		x16 = 4U,
		x32 = 5U
	};
	enum class SPI_Config
	{
		ThreeWire = 0u,
		FourWire = 1u
	};
	enum class Register :uint8_t //outside in a namespace?
	{
		CHIP_ID = 0X00,
		REV_ID = 0x01,
		ERROR_REG = 0x02,
		STATUS_REG = 0x03,
		PRESSURE_DATA = 0x04,
		TEMPERATURE_DATA = 0x07,
		SENSOR_TIME = 0x0C, //to 0X0E
		EVENT = 0x01,
		INT_STATUS = 0x11,
		PWR_CTRL = 0x1B,
		IF_CONF = 0x1A,
		CMD = 0x7E,
		CONFIG = 0x1F,
		OSR = 0x1C,

		CALIB_T1 = 0x31,
		CALIB_T2 = 0x33,
		CALIB_T3 = 0x35,
		CALIB_P1 = 0x36,
		CALIB_P2 = 0x38,
		CALIB_P3 = 0x3A,
		CALIB_P4 = 0x3B,
		CALIB_P5 = 0x3C,
		CALIB_P6 = 0x3E,
		CALIB_P7 = 0x40,
		CALIB_P8 = 0x41,
		CALIB_P9 = 0x42,
		CALIB_P10 = 0x44,
		CALIB_P11 = 0x45
	};
	//Go back and reread what Inline on these variables do vs static
  /* // PWR_CTRL (0x1B)
	inline constexpr uint8_t MODE_MASK        = 0b00000011;
	inline constexpr uint8_t TEMP_EN_MASK     = 0b00000010;
	inline constexpr uint8_t PRESS_EN_MASK    = 0b00000100;

	// CONFIG (0x1F)
	inline constexpr uint8_t IIR_MASK         = 0b11110000;

	// OSR (0x1C)
	inline constexpr uint8_t OSR_P_MASK       = 0b00000111;
	inline constexpr uint8_t OSR_T_MASK       = 0b00111000;
	inline constexpr uint8_t OSR_P_POS        = 0U;
	inline constexpr uint8_t OSR_T_POS        = 3U;

	// IF_CONF (0x1A)
	inline constexpr uint8_t SPI_MODE_MASK    = 0b00000001;

	// STATUS (0x03)
	inline constexpr uint8_t STATUS_CMD_RDY_MASK   = 0b10000000;
	inline constexpr uint8_t STATUS_TEMP_RDY_MASK  = 0b01000000;
	inline constexpr uint8_t STATUS_PRESS_RDY_MASK = 0b00100000;

	// CMD
	inline constexpr uint8_t CMD_SOFTRESET = 0xB6;
*/ //inline works only for c++ 17+ so enum for now - trqbva da se opravq s docketa ;_

	enum : uint8_t { // anonymous enum - temporary fix
	        IIR_MASK         = 0b11110000,
	        ODR_MASK         = 0b00001111,
	        OSR_P_MASK       = 0b00000111,
	        OSR_T_MASK       = 0b00111000,
	        TEMP_EN_MASK     = 0b00000010,
	        PRESS_EN_MASK    = 0b00000100,
	        MODE_MASK        = 0b00000011,
	        SPI_MODE_MASK    = 0b00000001,
	        STATUS_CMD_RDY_MASK  = 0b10000000,
	        STATUS_TEMP_RDY_MASK = 0b01000000,
	        STATUS_PRESS_RDY_MASK= 0b00100000
	    };
	constexpr uint8_t EXPECTED_CHIP_ID = 0x60;
	constexpr uint8_t READ_CMD_MASK = 0X80U;
	constexpr uint8_t WRITE_CMD_MASK = 0x7FU;
	constexpr uint8_t OSR_P_POS = 0U; // bits 2..0
	constexpr uint8_t OSR_T_POS = 3U; // bits 5..3
	constexpr uint8_t SOFT_RESET_COMMAND = 0xB6;

	// ? inside readRegisers funcion?
	constexpr uint8_t READ_CMD_INDEX = 0U;
	constexpr uint8_t READ_DATA_INDEX = 1U;
	constexpr uint8_t WRITE_CMD_INDEX = 0U;
	constexpr uint8_t WRITE_DATA_INDEX = 1U;

	enum class Mode : uint8_t
	{
		SLEEP = 0U,
		FORCED = 2U,
		NORMAL = 3U
	};


	enum class IIR_Config
	{
		bypass = 0U,
		coef_1 = 1U,
		coef_3 = 2U,
		coef_7 = 3U,
		coef_15 = 4U,
		coef_31 = 5U,
		coef_63 = 6U,
		coef_127 = 7U
	};
}
