/* Bluetooth tire pressure sensor project. Measures pressure, tempreture and battery percentage.
 * Readings could be displayed with TPMSII android app 
 */


#include "Sensor_id.h"
#include "Adxl362.h"
#include "my_advertising.h"
#include "my_config.h"
#include "ADC.h"
#include "mapper.h"
#include "measurments.h"
#include "sd_func_wrapper.h"



static bool timer_flag = false;
static uint8_t read_vbat_counter = 0;
APP_TIMER_DEF(m_app_timer_id);



/*
 * Main timer handler function that gets called every timer interval. I put all the application code in main().
 */
static void app_timer_handler(void *p_context)
{
    timer_flag = true;
    read_vbat_counter++;
}



/**@brief Function for starting timers.
 */
static void reading_timer_start(void)
{
    uint32_t err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, app_timer_handler); // Create timers
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(m_app_timer_id, READ_TIMER_INTERVAL, NULL);     // start main timer
    APP_ERROR_CHECK(err_code);
}



/*
 * Function for initializing app timer library. Call in setup.
 */
static void appTimerInit(void)
{
    uint32_t err_code = app_timer_init();     // Initialize timer module
    APP_ERROR_CHECK(err_code);
}



// main function

int main(void)
{

    ///////////////////////////////////// SETUP /////////////////////////////////////////

    appTimerInit();
    powerManagementInit();

    bleStackInit();
	enableDC2DC();
    My_advertising advertiser;
    advertiser.addIdToAddress(cfg::SENSOR_ID);		// attach sensor id to advertising buffer

    ADC<cfg::BRIDGE_PIN, cfg::ADC_POSITIVE_INPUT, cfg::ADC_NEGATIVE_INPUT> adc;

    adc.calibrate();       // do initial calibration

    uint8_t bat_percentage = mapVbat(adc.analogReadVbat());		// bat percentage has to be "main global", since it is not read every loop iteration
    Measurments<cfg::PRESSURE_SENSITIVITY_KPA, cfg::TEMP_SENSITIVITY, cfg::TEMP_ADC_CALIBRATE>
        measurments(map(adc.analogReadPressure()), getTemperature(), bat_percentage);    // initialize measurments with real data

    advertiser.configureAdvertising(measurments.getPressure(), measurments.getTemperature(), measurments.getBatPercentage());     // setup advertising

    reading_timer_start();     // start system main timer
    Adxl362<cfg::SS_PIN, cfg::MOSI_PIN, cfg::MISO_PIN, cfg::SCLK_PIN, cfg::ACC_VCC_PIN> adxl362;
    adxl362.setupMotionInterrupt<cfg::ACC_INT_PIN, cfg::ACTIVITY_THRESHOLD, cfg::INACTIVITY_THRESHOLD, cfg::INACTIVITY_TIME>();     // setup accelerometer for motion interrupt
    advertiser.startAdvertising();      // lastly: become visible (start advertising)

#ifdef ADXL362_DEBUG
	int i = 0;
#endif
    ///////////////////////////////////// LOOP /////////////////////////////////////////
    while (1)
    {

        if (timer_flag)       // do every second
        {

#ifdef ADXL362_DEBUG
			i++;
			printf("Acc ID reg: %d\n", adxl362.adxlReadRegister(0x00));
			printf("%d\n", adxl362.adxlReadRegister(0x09));
			printf("%d\n", adxl362.adxlReadRegister(0x0A));
			printf("%d\n", i);
#endif

            timer_flag = false;      // reset timer flag
            int16_t temperature = getTemperature();		 // read temperature
            if (measurments.checkIfAdcNeedsCal(temperature))       // if the temperature changed sufficiently, calibrate ADC
            {
                adc.calibrate();
            }

            if (read_vbat_counter > cfg::READ_VBAT_INTERVAL)	   // battery percentage is read less often than pressure or temperature
            {
                read_vbat_counter = 0;      // reset Vbat reading counter
                bat_percentage = mapVbat(adc.analogReadVbat());      // read Vbat (Vcc) and map it to %s
            }

#ifdef CALIBRATION
			
			// if calibration is enabled, raw readings (without any mappings or checkings are advertised)
			uint16_t pressure = map(adc.analogReadPressure());
            //printf("Pressure: %d\n", pressure);

            advertiser.updateAdvertising(adc.analogReadPressure(), temperature, bat_percentage);

#else	// advertise converted and filtered readings
			
			// check if measured values changed since the previous readings. It is done to prevent displaying noisy readings
            if (measurments.checkForChanges(map(adc.analogReadPressure()), temperature, bat_percentage))
            {
				// if they changed, update transmitted data
                advertiser.updateAdvertising(measurments.getPressure(), measurments.getTemperature(), measurments.getBatPercentage());
            }


#endif // CALIBRATION

#ifndef CALIBRATION
            if (0 == nrf_gpio_pin_read(cfg::ACC_INT_PIN))	 // check if there's no motion detected for 2 mins (accelerometer signals an inactivity interrupt)
            {
                sleepSysOffEnter();     // go to system off sleep mode (the wake up source is accelerometer pin)
            }
#endif // CALIBRATION
        }

        idle_state_handle();       // go to system ON sleep mode (until next timer interrupt)
    }
}