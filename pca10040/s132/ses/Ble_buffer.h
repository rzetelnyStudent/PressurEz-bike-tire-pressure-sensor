#ifndef BLE_BUFFER_H
#define BLE_BUFFER_H

#include "my_config.h"
#include "nrf_sdh_ble.h"
#include <stdbool.h>
#include <stdint.h>

/* Class wrapping ble buffer. nrf SDK requires switching buffers to update advertising. This class does this. 
 * Everytime getBuffer() is called internal buffer is switched for the next update. 
 */
class Ble_buffer
{

    const uint8_t PRESS_POS = 12;	  // index of bytes representing pressure in advertising buffer
    const uint8_t TEMP_POS = 14;
    const uint8_t BAT_POS = 16;
    bool use_buffer1;      // bool used to toggle between buffers.
    uint8_t my_adv_data_1[cfg::ADV_DATA_L];      // advertising data buffer 1. 
    uint8_t my_adv_data_2[cfg::ADV_DATA_L];      // advertising data buffer 2.
    ble_gap_adv_data_t ble_adv_struct_1;
    ble_gap_adv_data_t ble_adv_struct_2;

    void setPressure(uint16_t p_pressure);
    void setTemp(int16_t p_temp);
    void setBat(uint8_t p_temp);

  public:
    Ble_buffer();
    ble_gap_adv_data_t *getBuffer();
    void setPressTempLeak(const uint16_t p_pressure, const int16_t p_temperature, const uint8_t p_bat_percentage);
};

#endif