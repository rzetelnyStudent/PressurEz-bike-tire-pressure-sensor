#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#include "Sensor_id.h"
#include "my_utility.h"
#include "nrf_sdh_ble.h"

namespace cfg
{


/////////////////////////////////////////////// SENSOR ID //////////////////////////////////////////////

constexpr const Sensor_id SENSOR_ID {{0x00, 0x00, 0x01}, Which_wheel::REAR_RIGHT};
// ^ sensor ID will be displayed in: Bluetooth name, Bluetooth address, manufacturer data in advertising packet.
// The android app ("TPMSII") does only care about the 3-byte ID coded in manufacturer data, when it identifies a sensor.

////////////////////////////////////////////// CALIBRATION ////////////////////////////////////////////////

//#define CALIBRATION	   // when defined, sensor prints raw pressure readings, without any checks or mappings


////////////////////////////////////////////// ADVERTISING DATA ///////////////////////////////////////////l

const uint16_t ADV_DATA_L = 27;
const uint16_t SCAN_DATA_L = 14;


// The advertising data packet contains: device ID, pressure, temperature, battery percentage
const uint8_t MY_ADV_DATA[ADV_DATA_L] = {

    0x02, BLE_GAP_AD_TYPE_FLAGS, BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,     // original sensor has 0x05 here (LE_LIMITED_DISC_MODE),    
	// so it advertises only for <180s. My sensor advertises as long as the movement is detected. The android app doesn't care about this byte.
    0x03, BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE, 0xB0, 0xFB,    // original sensor contains a service 0xfbb0. Weird, since it is non connetable
	// (one cannot discover its services) + 0xfbb0 is not a assigned bluetooth UUID (16-bit UUIDs are reserved).
    0x13, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, 0x00, 0x01,    // manufacturer TOMTOM international. Even more weird, since the original sensors are sold no - branded
	SENSOR_ID.which_wheel + 0x80, 0xEA, 0xCA, SENSOR_ID.id_hex[0], SENSOR_ID.id_hex[1], SENSOR_ID.id_hex[2], 
	0x00, 0x00, 0x00, 0x00,     // pressure (little endian) in kPa * 1000
	0x00, 0x0D, 0x00, 0x00,		// temperature (little endian) in *C * 100
	0x02,						// battery percentage
    0x00,						// leak flag (when set to true, app says that tire leaks)
};


const uint8_t MY_SCAN_RESP_DATA[SCAN_DATA_L] =
    {
        0x0D, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, char('T'), char('P'), char('M'), char('S'), 
		lowerHalfByteToAscii<SENSOR_ID.which_wheel + 1>(), char('_'), 
		upperHalfByteToAscii<SENSOR_ID.id_hex[0]>(), lowerHalfByteToAscii<SENSOR_ID.id_hex[0]>(), 
		upperHalfByteToAscii<SENSOR_ID.id_hex[1]>(), lowerHalfByteToAscii<SENSOR_ID.id_hex[1]>(), 
		upperHalfByteToAscii<SENSOR_ID.id_hex[2]>(), lowerHalfByteToAscii<SENSOR_ID.id_hex[2]>()
	};




////////////////////////////////////////////////// PINS //////////////////////////////////////////////////////////////

const uint32_t SS_PIN = 8;   // MDBT42V pin 12   // 28
const uint32_t MISO_PIN = 6; // MDBT42V pin 14   // 29
const uint32_t MOSI_PIN = 1; 
const uint32_t SCLK_PIN = 0; 
const uint32_t ACC_VCC_PIN = 14;
const uint32_t ACC_INT_PIN = 12;
const uint32_t ADC_POSITIVE_INPUT = SAADC_CH_PSELP_PSELP_AnalogInput0;  //P28    //A0  // 2
const uint32_t ADC_NEGATIVE_INPUT = SAADC_CH_PSELP_PSELP_AnalogInput2;  // P29    // A1 // 3     A2 - 4
const uint32_t BRIDGE_PIN = 16;



/////////////////////////////////////////// ACCELEROMETER PARAMETERS //////////////////////////////////////////////////

const uint8_t ACTIVITY_THRESHOLD = 170;   // 160mg
const uint8_t INACTIVITY_THRESHOLD = 160;  // 80mg
const uint16_t INACTIVITY_TIME = 120;      // 120s



/////////////////////////////////////////// PRESSURE CALIBRATION PARAMETERS ///////////////////////////////////////////

const float A_COEFFICIENT = 2.1333;
const float B_COEFFICIENT = -81.597;



/////////////////////////////////////////////////////// SENSITIVITY ///////////////////////////////////////////////////

const int16_t PRESSURE_SENSITIVITY_KPA = 6;     // 60 mBar
const int32_t TEMP_SENSITIVITY = 600;     // 6 *C



///////////////////////////////////////////////// ADC CALIBRATION //////////////////////////////////////////////////////

const int32_t TEMP_ADC_CALIBRATE = 750;     // = 7.5*C


///////////////////////////////////////////////// TIME INTERVALS ///////////////////////////////////////////////////////


#define READ_INTERVAL 1000      // defines how often the sensor should read, advertise and display pressure
#define READ_TIMER_INTERVAL APP_TIMER_TICKS(READ_INTERVAL)     // converts read interval to app timer ticks
#define ADVERTISING_INTERVAL MSEC_TO_UNITS(READ_INTERVAL, UNIT_0_625_MS)     // converts read interval to adverting interval
const uint8_t READ_VBAT_INTERVAL = 10;      // Vbat gets read every READ_INTERVAL * READ_VBAT_INTERVAL seconds

};


#endif