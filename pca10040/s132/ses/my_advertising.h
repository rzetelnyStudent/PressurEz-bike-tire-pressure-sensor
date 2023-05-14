#ifndef MY_ADVERTISING_H
#define MY_ADVERTISING_H



#include "Ble_buffer.h"
#include "Sensor_id.h"
#include "my_config.h"
#include "fds.h"

/*
 * Class responsible for bluetooth advertising.
 */
class My_advertising
{
    Ble_buffer ble_buffer;	    // Advertising data buffer wrapper
    uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
    ble_gap_adv_params_t m_adv_params;

  public:
    My_advertising();
    void addIdToAddress(const Sensor_id &p_sensor_id);
    void configureAdvertising(const uint32_t p_pressure, const uint32_t p_temperature, const uint8_t p_bat_percentage, const bool p_leak);
    void startAdvertising();
	void updateAdvertising(const uint32_t p_pressure, const int32_t p_temperature, const uint8_t p_bat_percentage, const bool p_leak);
};

#endif