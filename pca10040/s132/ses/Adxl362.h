#ifndef ADXL362_H
#define ADXL362_H

#define ADXL362_DEBUG     // when defined, prints accelerometer ID reg. (if communication with adxl362 is correct, should print 0xAD)

extern "C"
{
#include <stdint.h>
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"
}


// Hard reset Vcc fall and rise delay times:
const static uint32_t DISCHARGE_TIME = 30;
const static uint32_t VCC_RISE_TIME = 30;



// class handling communication with Adxl362.
// Template parameters:
// SS pin, mosi, miso, sck self explanatory
// p_vcc_pin - GPIO for powering Adxl362. Discharges Adxl362 VCC. Adxl362 requires VDD to be completely discharged, if it drops below <1,8V.
// See the datasheet or Adxl362::hardResetVcc() down below.

template <uint8_t p_ss_pin, uint8_t p_mosi_pin, uint8_t p_miso_pin, uint8_t p_sck_pin, uint8_t p_vcc_pin>
class Adxl362 {



    // Some register defines:
    // -------------------------- SPI COMMANDS -----------------------------
    const uint8_t READ_CMD = 0x0B;
    const uint8_t WRITE_CMD = 0x0A;

    // -------------------- CONTROL REGS ADDRESSES ------------------------
    const uint8_t SOFT_RESET = 0x1f;
    const uint8_t THRESH_ACTL = 0x20;
    const uint8_t THRESH_ACTH = 0x21;
    const uint8_t TIME_ACT = 0x22;
    const uint8_t THRESH_INACTL = 0x23;
    const uint8_t THRESH_INACTH = 0x24;
    const uint8_t TIME_INACTL = 0x25;
    const uint8_t TIME_INACTH = 0x26;
    const uint8_t ACT_INACT_CTL = 0x27;
    const uint8_t FIFO_CONTROL = 0x28;
    const uint8_t FIFO_SAMPLES = 0x29;
    const uint8_t INTMAP1 = 0x2a;
    const uint8_t INTMAP2 = 0x2b;
    const uint8_t FILTER_CTL = 0x2c;
    const uint8_t POWER_CTL = 0x2d;
    const uint8_t SELF_TEST = 0x2e;

    // -------------------------- BIT FIELDS -----------------------------
    const uint8_t SOFT_RESET_KEY = 0x52;

    // --------- Bit values in ACT_INACT_CTL  (6 bits allocated) ---------
    const uint8_t ACT_ENABLE = 0x01;
    const uint8_t ACT_DISABLE = 0x00;
    const uint8_t ACT_AC = 0x02;
    const uint8_t ACT_DC = 0x00;
    const uint8_t INACT_ENABLE = 0x04;
    const uint8_t INACT_DISABLE = 0x00;
    const uint8_t INACT_AC = 0x08;
    const uint8_t INACT_DC = 0x00;
    const uint8_t ACT_INACT_LINK = 0x10;
    const uint8_t ACT_INACT_LOOP = 0x20; 

    // --------- bit values in INTMAP1 and INTMAP2 (and status) ----------
    const uint8_t INT_OFF = 0x00;
    const uint8_t INT_ACT = 0x10;
    const uint8_t INT_INACT = 0x20;
    const uint8_t INT_AWAKE = 0x40;
    const uint8_t INT_LOW = 0x80;

    // --------- Bit values in FILTER_CTL  (8 bits allocated) ------------
    const uint8_t RATE_400 = 0x05;
    const uint8_t RATE_200 = 0x04;
    const uint8_t RATE_100 = 0x03;    // default sample rate - 100Hz
    const uint8_t RATE_50 = 0x02;
    const uint8_t RATE_25 = 0x01;
    const uint8_t RATE_12_5 = 0x00;

    const uint8_t EXT_TRIGGER = 0x08;

    const uint8_t AXIS_X = 0x00;
    const uint8_t AXIS_Y = 0x10;
    const uint8_t AXIS_Z = 0x20;

    const uint8_t RANGE_2G = 0x00;
    const uint8_t RANGE_4G = 0x40;
    const uint8_t RANGE_8G = 0x80;
    const uint8_t FILTER_CTL_DEFAULT = 0x13;

    // -------- Bit values in POWER_CTL  (8 bits allocated) -----------------
    const uint8_t STANDBY = 0x00;
    const uint8_t MEASURE_1D = 0x01;
    const uint8_t MEASURE_3D = 0x02;
    const uint8_t AUTO_SLEEP = 0x04;
    const uint8_t WAKE_UP = 0x08;
    const uint8_t LOW_POWER = 0x00;
    const uint8_t LOW_NOISE1 = 0x10;
    const uint8_t LOW_NOISE2 = 0x20;
    const uint8_t LOW_NOISE3 = 0x30;

    const uint8_t EXT_CLOCK = 0x40;
    const uint8_t EXT_ADC = 0x80;

    // ------------ Bit values in SELF_TEST  (1 bit allocated) --------------
    const uint8_t SELFTEST_ON = 0x01;
    const uint8_t SELFTEST_OFF = 0x00;

    // ------- Bit values in FIFO_CTL  (4 bits allocated) -------------------
    const uint8_t XL362_FIFO_MODE_OFF = 0x00;
    // ---------------------------- OTHER -----------------------------------
    const uint8_t FIFO_SAMPLES_NOT_USED = 0x80;

    const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(0);		/**< SPI instance. */
	nrf_drv_spi_config_t m_spi_config;

	void setupSPI();
	void uninitSPI();
	void hardResetVcc();
	void initDelay();
	template <uint32_t p_time_ms>
	void sleepDelay();


public:
	template <uint32_t p_int_pin, uint8_t p_act_th, uint8_t p_inact_th, uint16_t p_inact_time>
	void setupMotionInterrupt();

	
#ifdef ADXL362_DEBUG
    /* Function for reading registers of adxl362. Only for debug purposes.
	 *
	 */
	uint8_t adxlReadRegister(uint8_t reg_address)
	{
    uint32_t err_code;

    uint8_t rx_buffer[3];

    uint8_t tx_buffer[3];      //Prepare tx_buffer
    tx_buffer[0] = READ_CMD;
    tx_buffer[1] = reg_address;
    tx_buffer[2] = 0x00;
    err_code = nrf_drv_spi_transfer(&spi, tx_buffer, sizeof(tx_buffer), rx_buffer, sizeof(rx_buffer));
    APP_ERROR_CHECK(err_code);
    return rx_buffer[2];
	}
#endif //ADXL362_DEBUG


};


/* Public method for setting adxl362 in autonomous motion switch mode, using SPI.
 * I recommend calling this method during startup (every time MCU resets).
 * This method satisfies Adxl362 supply requirements (Adxl362 requires VDD to be completely discharged, if it drops below <1,8V)
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * nrf_pwr_mgmt_init(), app_timer_init() HAVE TO BE CALLED AND LFCLK (OR SOFTDEVICE) HAVE TO BE STARTED BEFORE CALLING THIS METHOD!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * params:
 * - p_act_th - a threshold, that defines motion detection sensitivity (in [mg])
 * - p_inact_th - a threshold, that defines stationary state detection (in [mg])
 * - p_inact_time - time after which the motion interrupt goes low, when no activity is detected (in [s])
 */
template <uint8_t p_ss_pin, uint8_t p_mosi_pin, uint8_t p_miso_pin, uint8_t p_sck_pin, uint8_t p_vcc_pin>
template <uint32_t p_int_pin, uint8_t p_act_th, uint8_t p_inact_th, uint16_t p_inact_time>
void Adxl362 <p_ss_pin, p_mosi_pin, p_miso_pin, p_sck_pin, p_vcc_pin>::setupMotionInterrupt()
{
	nrf_gpio_cfg_sense_input(p_int_pin, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
	initDelay();
	hardResetVcc();		// Adxl362 requires VDD to be completely discharged, if it drops below <1,8V. I decided to just discharge VDD every time
	setupSPI();
    const uint8_t DO_SOFT_RESET[]     // prepare SPI buffer 1
    {
        WRITE_CMD,
        SOFT_RESET,         // soft - reset register address
        SOFT_RESET_KEY      // soft - resets adxl362
    };
    uint32_t err_code = nrf_drv_spi_transfer(&spi, DO_SOFT_RESET, sizeof(DO_SOFT_RESET), NULL, 0);	   // transfers will be performed in blocking mode.
    APP_ERROR_CHECK(err_code);
	sleepDelay<5>();     // "A latency of approximately 0.5 ms is required after soft reset" ~datasheet P. 26

    const uint8_t SENSOR_SETTINGS[]     // prepare SPI buffer 2, in my configuration Adxl362 is placed in wake-up mode, motion interrupt on pin 1
    {
        WRITE_CMD,
        THRESH_ACTL,    // burst writing starting from this register (address 0x20)                        Address | Register name
        p_act_th,       // ------------------------------------------------------------------------------- 0x20 (THRESH_ACTL)
        0,     // ---------------------------------------------------------------------------------------- 0x21 (THRESH_ACTH)
        0,     // ---------------------------------------------------------------------------------------- 0x22	(TIME_ACT) <- it doesn't matter (timer is off during sleep in wake-up mode)
        p_inact_th,    // -------------------------------------------------------------------------------- 0x23 (THRESH_INACTL)
        0,     // ---------------------------------------------------------------------------------------- 0x24 (THRESH_INACTH)
        (p_inact_time * 6) & 0xFF,    // ----------------------------------------------------------------- 0x25 (TIME_INACTL)
        (p_inact_time * 6) >> 8,    // ------------------------------------------------------------------- 0x26 (TIME_INACTH)
        uint8_t(ACT_INACT_LINK | ACT_INACT_LOOP | INACT_AC | INACT_ENABLE | ACT_AC | ACT_ENABLE),   // --- 0x27 (ACT_INACT_CTL)
        XL362_FIFO_MODE_OFF,    // ----------------------------------------------------------------------- 0x28 (FIFO_CONTROL)
        FIFO_SAMPLES_NOT_USED,     // -------------------------------------------------------------------- 0x29 (FIFO_SAMPLES)
        INT_AWAKE,    // --------------------------------------------------------------------------------- 0x2A (INTMAP1)
        INT_OFF,     // ---------------------------------------------------------------------------------- 0x2B (INTMAP2)
        FILTER_CTL_DEFAULT,    // ------------------------------------------------------------------------ 0x2C (FILTER_CTL)
        uint8_t(MEASURE_3D | WAKE_UP)    // -------------------------------------------------------------- 0x2D	(POWER_CTL)		
																									   //  [Alternatively: AUTO_SLEEP | MEASURE_3D, if you want to sample acceleration when motion is detected]
    };
    err_code = nrf_drv_spi_transfer(&spi, SENSOR_SETTINGS, sizeof(SENSOR_SETTINGS), NULL, 0);	 // no RX buffer needed
    APP_ERROR_CHECK(err_code);

#ifdef ADXL362_DEBUG // Print out device_id register. If 0xAD is printed, communication with ADXL_362 is succesful.
	printf("%d\n", adxlReadRegister(0x00));
#endif //ADXL362_DEBUG

#ifndef ADXL362_DEBUG   
	uninitSPI();	  // Once the Adxl362 is set MCU can forget about it (I don't sample any acceleration data)
#endif

}


//private method for initializing the SPI driver
template <uint8_t p_ss_pin, uint8_t p_mosi_pin, uint8_t p_miso_pin, uint8_t p_sck_pin, uint8_t p_vcc_pin>
void Adxl362 <p_ss_pin, p_mosi_pin, p_miso_pin, p_sck_pin, p_vcc_pin>::setupSPI() {
    m_spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    m_spi_config.ss_pin = p_ss_pin; 
    m_spi_config.miso_pin = p_miso_pin;
    m_spi_config.mosi_pin = p_mosi_pin;
    m_spi_config.sck_pin = p_sck_pin;
    m_spi_config.frequency = NRF_DRV_SPI_FREQ_4M;
    uint32_t err_code = nrf_drv_spi_init(&spi, &m_spi_config, NULL, NULL);		// transfers will be performed in blocking mode
    APP_ERROR_CHECK(err_code);
}


// private method for uninitializing SPI driver and reconfiguring SPI pins (nrf_drv_spi_uninit() doc. says "configuration of pins is kept")
template <uint8_t p_ss_pin, uint8_t p_mosi_pin, uint8_t p_miso_pin, uint8_t p_sck_pin, uint8_t p_vcc_pin>
void Adxl362 <p_ss_pin, p_mosi_pin, p_miso_pin, p_sck_pin, p_vcc_pin>::uninitSPI()
{
	nrf_drv_spi_uninit(&spi);	  // Once motion interrupt is set, MCU can forget about the sensor. SPI bus will no longer be needed
}


// Private method for discharging Adxl362 VCC. The datasheet says:
/* It [Adxl362] must always be started up from 0 V. 
 * When the device is in operation, any time power is removed 
 * from the ADXL362, or falls below the operating voltage range, 
 * the supplies (VS, VDD I/O, and any bypass capacitors) must be 
 * discharged completely before power is reapplied. To enable 
 * supply discharge, it is recommended to power the device from 
 * a microcontroller GPIO [...].
 *
 * I'm doing the trick with GPIO in my project
 */
template <uint8_t p_ss_pin, uint8_t p_mosi_pin, uint8_t p_miso_pin, uint8_t p_sck_pin, uint8_t p_vcc_pin>
void Adxl362 <p_ss_pin, p_mosi_pin, p_miso_pin, p_sck_pin, p_vcc_pin>::hardResetVcc()
{

    nrf_gpio_cfg_input(p_ss_pin, NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_input(p_miso_pin, NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_input(p_sck_pin, NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_input(p_mosi_pin, NRF_GPIO_PIN_NOPULL);
	
	nrf_gpio_cfg(p_vcc_pin, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_pin_clear(p_vcc_pin);		// pull adxl362 VCC to GND

    sleepDelay<DISCHARGE_TIME>();	    // wait for VCC caps to discharge (should be plenty time for 4,7uF 2 x 100nF, like it is in my schematic

    nrf_gpio_pin_set(p_vcc_pin);		// pull VCC high

	sleepDelay<VCC_RISE_TIME>();        // wait for VCC to rise

}



APP_TIMER_DEF(adxl362_timer_id);	 // This class uses app timer for delay purposes


/* Timer handler. Used for delay purposes.
 */
static void VCC_rise_handler(void * p_context)
{
	*(bool *)p_context = true;
}



/* 
 * Private function for initializing my delay functionality. Call before sleepDelay().
 */
template <uint8_t p_ss_pin, uint8_t p_mosi_pin, uint8_t p_miso_pin, uint8_t p_sck_pin, uint8_t p_vcc_pin>
void Adxl362 <p_ss_pin, p_mosi_pin, p_miso_pin, p_sck_pin, p_vcc_pin>::initDelay()
{
	uint32_t err_code = app_timer_create(&adxl362_timer_id, APP_TIMER_MODE_SINGLE_SHOT, VCC_rise_handler);		// create the timer
    APP_ERROR_CHECK(err_code);
}



/*
 * Private method for that delays (blocks) processor for certain time in [ms] given by p_time_ms. 
 * During delay the processor is sleeping in system on mode, so it doesn't consume much power. Make sure to call initDelay() before using this function. 
 * Warning: This function uses app_timer so make sure to call app_timer_init() and lfclk is enabled before call.
 * Params: - p_time_ms - time in ms that processor will sleep.
 */
template <uint8_t p_ss_pin, uint8_t p_mosi_pin, uint8_t p_miso_pin, uint8_t p_sck_pin, uint8_t p_vcc_pin>
template <uint32_t p_time_ms>
void Adxl362 <p_ss_pin, p_mosi_pin, p_miso_pin, p_sck_pin, p_vcc_pin>::sleepDelay()
{
	static bool vcc_rised = false;		// prepare flag
	vcc_rised = false;	   // vcc_rise is static, so have to reinitialize it every call.
    uint32_t err_code = app_timer_start(adxl362_timer_id, APP_TIMER_TICKS(p_time_ms), (void *)&vcc_rised);	   // start the timer
	APP_ERROR_CHECK(err_code);
	do {
		nrf_pwr_mgmt_run();		// sleep in system on in meantime
	}
	while (!vcc_rised);		// in case of any other event woke MCU, go to sleep (nrf_pwr_mgmt_run()) again
}


#endif 
