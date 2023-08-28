#ifndef MEASURMENTS_H
#define MEASURMENTS_H

#include <stdint.h>
#include <cmath>


/* Class for checking if new readings should be displayed. The goal is to prevent displaying readings with noisy last 
 * digit. It is achieved by comparing the difference between current readings and previous readings against some delta
 * (sensitivity) and displaying only those that pass this test.
 *
 * Template parameters:
 * PRESSURE_SENSITIVITY_KPA - the difference between pressure readings has to be larger than this to 
 * make checkForChanges return true 
 * TEMP_SENSITIVITY - the difference between temperature readings has to be larger than this to 
 * make checkForChanges return true 
 * ADC_CAL_THRESHOLD - the difference between temperature readings has to be larger than this to 
 * make checkIfAdcNeedsCal return true
 */
template <int16_t PRESSURE_SENSITIVITY_KPA, int16_t TEMP_SENSITIVITY, int16_t ADC_CAL_THRESHOLD>
class Measurments
{
    uint16_t prev_pressure_kPa;      // remembers last pressure advertised
	int16_t prev_temperature;
	uint8_t prev_bat_percentage;
	int16_t temp_adc_last_cal;

  public:

	/*
	 * Constructor initializes Measurments with: pressure, temperature, bat percentage.
	 */
    Measurments(uint16_t prev_pressure_kPa, int16_t prev_temperature, uint8_t prev_bat_percentage)
    {
        this->prev_pressure_kPa = prev_pressure_kPa;
        this->prev_temperature = prev_temperature;
        this->prev_bat_percentage = prev_bat_percentage;
		this->temp_adc_last_cal = prev_temperature;
    }

	uint16_t getPressure() {
		return prev_pressure_kPa;
	}

	int16_t getTemperature() {
		return prev_temperature;
	}

	uint8_t getBatPercentage() {
		return prev_bat_percentage;
	}


	/* 
	 * Method for checking, if new readings are different than remembered, more than some delta (sensitivity).
	 * Call every time new readings are acquired. If current readings (parameters) are different,
	 * old readings are updated with new readings and true is returned.
	 */
    bool checkForChanges(uint16_t pressure_kPa, int16_t temperature, uint8_t bat_percentage)
    {
        bool pressure_changed, temp_changed, bat_perc_changed = false;
        if (abs((int32_t)prev_pressure_kPa - (int32_t)pressure_kPa) > PRESSURE_SENSITIVITY_KPA)
        {
            prev_pressure_kPa = pressure_kPa;
            pressure_changed = true;
        }
        if (abs(prev_temperature - temperature) > TEMP_SENSITIVITY)
        {
            prev_temperature = temperature;
            temp_changed = true;
        }
		if (bat_percentage < prev_bat_percentage)	 // bat percentage should only decrease (there's no charger or anything like that)
		{
		prev_bat_percentage = bat_percentage;
		bat_perc_changed = true;
		}

        return (pressure_changed || temp_changed || bat_perc_changed);
	}



    /* Checks if adc needs calibration (if current temperature is different than the temperature of last calibration 
	 * by some threshold (ADC_CAL_THRESHOLD).
	 */
	bool checkIfAdcNeedsCal(int16_t temperature)
	{
		if (abs(temp_adc_last_cal - temperature) > ADC_CAL_THRESHOLD)
		{
			temp_adc_last_cal = temperature;
			return true;
		}
		return false;
	}
};

#endif