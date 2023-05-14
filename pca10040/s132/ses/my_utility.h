#ifndef MY_UTILITY_H
#define MY_UTILITY_H


#include <stdbool.h>
#include <stdint.h>
#include "nrf_sdh_soc.h"


/*
 * Function for converting hex letter coding upper half byte to ascii code. Example: If you pass 0xAE, function will return 'A'.
 * Params: in_byte byte, whose upper half gets converted.
 * Returns: char cointaining ascii code.
 */
template <uint8_t in_byte>
inline char upperHalfByteToAscii()
{
	char p_in_byte = in_byte;
    p_in_byte = p_in_byte >> 4;
    if (p_in_byte <= 9)
    {
        return p_in_byte + '0';
    }
    else
    {
        return p_in_byte + 55;
    }
}



/*
 * Function for converting hex letter coding upper lower byte to ascii code. Example: If you pass 0xAE, function will return 'E'.
 * Params: in_byte byte, whose lower half gets converted.
 * Returns: char cointaining ascii code.
 */
template <uint8_t in_byte>
inline char lowerHalfByteToAscii()
{
	char p_in_byte = in_byte;
    p_in_byte = p_in_byte & 0b00001111;
    if (p_in_byte <= 9)
    {
        return p_in_byte + '0';
    }
    else
    {
        return p_in_byte + 55;
    }
}


/*
 * Function returning temperature in 1/100 degrees Celsius
 * Returns: temperature in 1/100 Celsius degrees 
 */
inline int32_t getTemperature()
{
	int32_t raw_temp;
	sd_temp_get(&raw_temp);
	return raw_temp * 25;
}


/*
 * Function for constraing input value x in range a to b. Example: if You want to have x to be in range <2, 5>, call constrain(x, 2, 5)
 * Params: a - constrained range lower bound
 *		   b - constrained range upper bound
 */
template<class T>
const T constrain(const T x, const T a, const T b) {
    if (x < a) {
        return a;
    }
    else if (b < x) {
        return b;
    }
    else
        return x;
}

#endif