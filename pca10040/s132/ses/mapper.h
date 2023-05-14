#ifndef MAPPER_H
#define MAPPER_H

#include "my_config.h"
#include "my_utility.h"
#include <stdint.h>


/*
 * Function for mapping pressure using linear equation, with a, b coefficients taken from my_config.h.
 * Params: pressure raw pressure reading from adc.
 */
inline uint32_t map(uint16_t pressure)
{
	int32_t result = (int32_t)((float)pressure * cfg::A_COEFFICIENT + cfg::B_COEFFICIENT);
	if (result < 20)     // constrain to 0, values < 20kPa (0.2bar), so that can cast to int and the sensor doesn't show something like 0.01 insted of 0
		result = 0;
	return (uint32_t)result;
}



// look up table for converting Vbat reading into battery percentage.
static const uint8_t VBAT_LOOK_UP[61] = {
1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 6, 6, 7, 8, 9, 10, 11, 12, 12, 13, 14,
15, 16, 17, 18, 19, 19, 20, 21, 22, 23, 24, 25, 27, 30, 33, 36, 39, 42, 45, 48, 51, 57, 65, 70, 75,
80, 85, 90, 95, 100
};



/*
 * Function for mapping Vbat from adc reading to %s.
 * Params: vbat_raw - raw adc reading. Should be in 8bit resolution from 151 to 211 range.
 * Returns: battery percentage from 1 to 100%.
 */
inline uint8_t mapVbat(uint16_t vbat_raw)
{
    vbat_raw = constrain(vbat_raw, (uint16_t)151, (uint16_t)211);
    vbat_raw = vbat_raw - 151;
    return VBAT_LOOK_UP[vbat_raw];
}




#endif
