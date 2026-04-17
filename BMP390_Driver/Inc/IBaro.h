/*
 * IBaro.h
 *
 *  Created on: Nov 12, 2025
 *      Author: icodi
 */
#pragma once
#ifndef INC_IBARO_H_
#define INC_IBARO_H_
#include <stdint.h>


namespace BMP390
{
	enum class Status :uint8_t
	{ //Example error - fix later
		OK = 0U,
		ERR_UNKNOWN = 1U,
		ERR_COMM = 2U,
		ERR_TIMEOUT = 3U,
		ERR_BAD_CHIPID = 4U,
		ERR_CALIB = 5U,
		ERR_STALE = 6U,
		ERR_BAD_PARAM = 7U,
		ERR_NOT_INITIALIZED = 8U,
		ERR_MODE = 9U,
		ERR_OVERRUN = 10U
	};
}


class ITemp {
public:
    ~ITemp() = default;
    BMP390::Status getTemp(float& out) const noexcept;
};

// Pressure interface
class IPress {
public:
    ~IPress() = default;
    BMP390::Status getPress(float& out) const noexcept;
};











#endif /* INC_IBARO_H_ */
