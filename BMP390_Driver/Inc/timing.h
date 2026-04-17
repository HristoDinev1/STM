#pragma once
#include <stdint.h>

namespace timing // review
{
	void init_for_flight() noexcept;
	void delay_us(uint32_t us) noexcept;

}
