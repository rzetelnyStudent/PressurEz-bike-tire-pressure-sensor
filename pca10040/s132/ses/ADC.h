#ifndef ADC_H
#define ADC_H

//#define ADC_DEBUG      // if ADC_DEBUG is defined, pressure readings get printed and pressure sensor bridge is powered all the time (useful for checking voltage with multimeter)

extern "C" {
#include "nrf_delay.h"
#include "nrf_gpio.h"

#ifdef ADC_DEBUG    
#include "math.h"
#endif //ADC_DEBUG
}

#include "sd_func_wrapper.h"


/*
 * Class representing adc job in my project. In my project adc is used to read pressure and Vbat
 */
template <uint32_t _bridge_pin, uint32_t positive_in_pin, uint32_t negative_in_pin>
class ADC
{

    void enable();
    void disable();
	void setupForPressure();
	void setupForVbat();

  public:
    void calibrate();	 // call during startup, or every time temperature changes significantly
	ADC();		// call once during startup, sets the ADC up.
    uint16_t analogReadPressure();		// call every time you want to read
	uint16_t analogReadVbat();
};

// enables the ADC for a conversion
template <uint32_t _bridge_pin, uint32_t positive_in_pin, uint32_t negative_in_pin>
void ADC<_bridge_pin, positive_in_pin, negative_in_pin>::enable()
{
    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);
}


// disables the ADC after a conversion
template <uint32_t _bridge_pin, uint32_t positive_in_pin, uint32_t negative_in_pin>
void ADC<_bridge_pin, positive_in_pin, negative_in_pin>::disable()
{
    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos);
}


// Recalibrates adc. Call during startup, or every time temperature changes 10*C
template <uint32_t _bridge_pin, uint32_t positive_in_pin, uint32_t negative_in_pin>
void ADC<_bridge_pin, positive_in_pin, negative_in_pin>::calibrate()
{
    enable();

    NRF_SAADC->TASKS_STOP = 0x01UL;
    while (!NRF_SAADC->EVENTS_STOPPED);
    NRF_SAADC->EVENTS_STOPPED = 0x00UL;

    NRF_SAADC->TASKS_CALIBRATEOFFSET = 0x01UL;
    while (!NRF_SAADC->EVENTS_CALIBRATEDONE);
    NRF_SAADC->EVENTS_CALIBRATEDONE = 0x00UL;

    NRF_SAADC->TASKS_STOP = 0x01UL;
    while (!NRF_SAADC->EVENTS_STOPPED);

    NRF_SAADC->EVENTS_STOPPED = 0x00UL;

    disable();
}


// Constructor. Configures bridge Vcc pin.
template <uint32_t _bridge_pin, uint32_t positive_in_pin, uint32_t negative_in_pin>
ADC<_bridge_pin, positive_in_pin, negative_in_pin>::ADC()
{
   // configures bridge supply pin as output high drive.
   nrf_gpio_cfg(_bridge_pin, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0H1, NRF_GPIO_PIN_NOSENSE);
}



/*
 * Private method for setting adc channel for reading pressure sensor bridge.
 */
template <uint32_t _bridge_pin, uint32_t positive_in_pin, uint32_t negative_in_pin>
void ADC<_bridge_pin, positive_in_pin, negative_in_pin>::setupForPressure()
{
    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_12bit;		// found empirically to be the most optimal. Doesn't include useless noise. 
    NRF_SAADC->OVERSAMPLE = (SAADC_OVERSAMPLE_OVERSAMPLE_Over32x << SAADC_OVERSAMPLE_OVERSAMPLE_Pos) & SAADC_OVERSAMPLE_OVERSAMPLE_Msk;     // found empirically to be the most optimal

    for (int i = 0; i < 8; i++)		// disable unused channels (so that ADC doesn't enter scan mode)
    {
        NRF_SAADC->CH[i].PSELN = SAADC_CH_PSELP_PSELP_NC;
        NRF_SAADC->CH[i].PSELP = SAADC_CH_PSELP_PSELP_NC;
    }
	
	// Channel configuration. Oversampling is on (x32) (higher settings don't seem to decrease noise, while they make conversion take longer.
	// Refence is VDD, because sensor is powered by VDD (via GPIO). 
	// Gain 4, sets input limit +-VDD/16. For example if VDD = 3.0V, ADC can measure +-0.188V (enough for MD-PS002)
	// Gain 1, sets input limit to +-VDD/4. For example if VDD = 3.0V, ADC can measure +-0.75V (enough for MS5407)
	// TACQ is 3us, because bridge output resistance is ~5kOhm (see datasheet)
	NRF_SAADC->CH[0].CONFIG =   ((SAADC_CH_CONFIG_RESP_Bypass   << SAADC_CH_CONFIG_RESP_Pos)   & SAADC_CH_CONFIG_RESP_Msk)
							 | ((SAADC_CH_CONFIG_RESP_Bypass    << SAADC_CH_CONFIG_RESN_Pos)   & SAADC_CH_CONFIG_RESN_Msk)
							 | ((SAADC_CH_CONFIG_GAIN_Gain1     << SAADC_CH_CONFIG_GAIN_Pos)   & SAADC_CH_CONFIG_GAIN_Msk)
							 | ((SAADC_CH_CONFIG_REFSEL_VDD1_4  << SAADC_CH_CONFIG_REFSEL_Pos) & SAADC_CH_CONFIG_REFSEL_Msk)
							 | ((SAADC_CH_CONFIG_TACQ_3us       << SAADC_CH_CONFIG_TACQ_Pos)   & SAADC_CH_CONFIG_TACQ_Msk)
							 | ((SAADC_CH_CONFIG_BURST_Enabled  << SAADC_CH_CONFIG_BURST_Pos)  & SAADC_CH_CONFIG_BURST_Msk)
							 | ((SAADC_CH_CONFIG_MODE_Diff      << SAADC_CH_CONFIG_MODE_Pos)   & SAADC_CH_CONFIG_MODE_Msk);
                             
  NRF_SAADC->CH[0].PSELN = negative_in_pin;     // differential mode
  NRF_SAADC->CH[0].PSELP = positive_in_pin;
}



/*
 * Private method for setting adc channel for reading Vbat.
 */
template <uint32_t _bridge_pin, uint32_t positive_in_pin, uint32_t negative_in_pin>
void ADC<_bridge_pin, positive_in_pin, negative_in_pin>::setupForVbat()
{
    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_8bit;		// found empirically to be the most optimal 
    NRF_SAADC->OVERSAMPLE = (SAADC_OVERSAMPLE_OVERSAMPLE_Over8x << SAADC_OVERSAMPLE_OVERSAMPLE_Pos) & SAADC_OVERSAMPLE_OVERSAMPLE_Msk;    // oversampling x8 found to be enough.

    for (int i = 0; i < 8; i++)		// disable unused channels (so that ADC doesn't enter scan mode)
    {
        NRF_SAADC->CH[i].PSELN = SAADC_CH_PSELP_PSELP_NC;
        NRF_SAADC->CH[i].PSELP = SAADC_CH_PSELP_PSELP_NC;
    }
	

	// Channel configuration. Oversampling is on (x8) (higher settings don't seem to decrease noise, while they make conversion take longer.
	// Refence is internal 0,6V reference. 
	// Current gain (1/6), sets input limit to 3,6V
	// TACQ is 3us
	NRF_SAADC->CH[0].CONFIG =   ((SAADC_CH_CONFIG_RESP_Bypass   << SAADC_CH_CONFIG_RESP_Pos)   & SAADC_CH_CONFIG_RESP_Msk)
							 | ((SAADC_CH_CONFIG_RESP_Bypass    << SAADC_CH_CONFIG_RESN_Pos)   & SAADC_CH_CONFIG_RESN_Msk)
							 | ((SAADC_CH_CONFIG_GAIN_Gain1_6     << SAADC_CH_CONFIG_GAIN_Pos)   & SAADC_CH_CONFIG_GAIN_Msk)
							 | ((SAADC_CH_CONFIG_REFSEL_Internal  << SAADC_CH_CONFIG_REFSEL_Pos) & SAADC_CH_CONFIG_REFSEL_Msk)
							 | ((SAADC_CH_CONFIG_TACQ_3us       << SAADC_CH_CONFIG_TACQ_Pos)   & SAADC_CH_CONFIG_TACQ_Msk)
							 | ((SAADC_CH_CONFIG_BURST_Enabled  << SAADC_CH_CONFIG_BURST_Pos)  & SAADC_CH_CONFIG_BURST_Msk)
							 | ((SAADC_CH_CONFIG_MODE_SE      << SAADC_CH_CONFIG_MODE_Pos)   & SAADC_CH_CONFIG_MODE_Msk);
                            
    NRF_SAADC->CH[0].PSELP = SAADC_CH_PSELP_PSELP_VDD;       // single ended mode
}




// Call every time you want to read pressure.
// Returns pressure raw reading. To convert it into Bar, you have map sensor reading, using some coefficients calculated by excel's linear regression
template <uint32_t _bridge_pin, uint32_t positive_in_pin, uint32_t negative_in_pin>
uint16_t ADC<_bridge_pin, positive_in_pin, negative_in_pin>::analogReadPressure()
{

    disableDC2DC();    // I found disabling DCDC during the reading minimized noise a bit.

	setupForPressure();

	nrf_gpio_pin_set(_bridge_pin);

	//nrf_delay_us(50);		// this delay, could compensate for GPIO rise time, but empirically I didn't found it nessesery
    enable();

    volatile int16_t pressure_raw;		// ADC outputs 16 bit signed result
    NRF_SAADC->RESULT.PTR = (uint32_t)&pressure_raw;	 // pointer to 16 bit int with result, that is stored in 32bit register
    NRF_SAADC->RESULT.MAXCNT = 1;		// One sample

    NRF_SAADC->TASKS_START = 0x01UL;
    while (!NRF_SAADC->EVENTS_STARTED);
    NRF_SAADC->EVENTS_STARTED = 0x00UL;

    NRF_SAADC->TASKS_SAMPLE = 0x01UL;
    while (!NRF_SAADC->EVENTS_END);
    NRF_SAADC->EVENTS_END = 0x00UL;

    NRF_SAADC->TASKS_STOP = 0x01UL;
    while (!NRF_SAADC->EVENTS_STOPPED);
    NRF_SAADC->EVENTS_STOPPED = 0x00UL;

	enableDC2DC();

	pressure_raw = pressure_raw >> 1;	// just to remove useless noisy LSB

#ifdef ADC_DEBUG
	printf("%d\n", pressure_raw);
	pressure_raw = abs(pressure_raw);
#else
    if (pressure_raw < 0)	// not super - neccesery (pressure reading shouldn't be negative on it's own)
        pressure_raw = 0;
#endif //ADC_DEBUG

    disable();

#ifndef ADC_DEBUG      // if ADC_DEBUG is defined, pressure sensor is powered all the time. Useful for checking its output with multimeter
	nrf_gpio_pin_clear(_bridge_pin);		// turn off the sensor bridge
#endif //ADC_DEBUG

    return (uint16_t)pressure_raw;
}


// vvv edit comment
// call every time you want to read.
// Returns pressure raw reading. To convert it into Bar, you have map sensor reading, using for example excel linear regression
template <uint32_t _bridge_pin, uint32_t positive_in_pin, uint32_t negative_in_pin>
uint16_t ADC<_bridge_pin, positive_in_pin, negative_in_pin>::analogReadVbat()
{
	setupForVbat();
    enable();

    volatile int16_t pressure_raw;		// ADC outputs 16 bit signed result
    NRF_SAADC->RESULT.PTR = (uint32_t)&pressure_raw;	 // pointer to 16 bit int with result, that is stored in 32bit register
    NRF_SAADC->RESULT.MAXCNT = 1;		// One sample

    NRF_SAADC->TASKS_START = 0x01UL;
    while (!NRF_SAADC->EVENTS_STARTED);
    NRF_SAADC->EVENTS_STARTED = 0x00UL;

    NRF_SAADC->TASKS_SAMPLE = 0x01UL;
    while (!NRF_SAADC->EVENTS_END);
    NRF_SAADC->EVENTS_END = 0x00UL;

    NRF_SAADC->TASKS_STOP = 0x01UL;
    while (!NRF_SAADC->EVENTS_STOPPED);
    NRF_SAADC->EVENTS_STOPPED = 0x00UL;

	//pressure_raw = pressure_raw >> 1;	// just to remove useless noisy LSB
    if (pressure_raw < 0)	// not super - neccesery (pressure reading shouldn't be negative on it's own)
    {
        pressure_raw = 0;
    }

    disable();

    return (uint16_t)pressure_raw;
}


// useful for debug:
/*
inline void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
	printf("\n");
    puts("");
}
*/


#endif
