#include "my_advertising.h"


/* 
 * Constructor, initializes advertising settings
 */
My_advertising::My_advertising()
{
    m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
    memset(&m_adv_params, 0, sizeof(m_adv_params));   // zero m_adv_params

    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;      // my sensor is unconnectable (it's a broadcaster only)
    m_adv_params.p_peer_addr = NULL;     // Undirected advertisement. Sends packets to everyone
    m_adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval = ADVERTISING_INTERVAL;     // ADVERTISING interval is defined in my_config.h
    m_adv_params.duration = 0;     // Advertising never times out
}



/* 
 * Method for configuring advertised ble mac address. Adress is compliant to ready-made sensors from aliexpress
 * Params: p_sensor_id - 3 byte sensor id, that gets added to an address. 
 */
void My_advertising::addIdToAddress(const Sensor_id &p_sensor_id)
{
    uint32_t err_code;
    ble_gap_addr_t my_addr;
    err_code = sd_ble_gap_addr_get(&my_addr);
    APP_ERROR_CHECK(err_code);
    my_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;	  // just mimic original sensor
    my_addr.addr[0] = p_sensor_id.id_hex[2];     // little endian encoding
    my_addr.addr[1] = p_sensor_id.id_hex[1];   
    my_addr.addr[2] = p_sensor_id.id_hex[0];   
    my_addr.addr[3] = 0xBE;     // all sensors sold has this EACA sequence in address
    my_addr.addr[4] = 0xEF;	    
    my_addr.addr[5] = 0x81;
    err_code = sd_ble_gap_addr_set(&my_addr);
    APP_ERROR_CHECK(err_code);
}



/* 
 * Method for initializing advertisining data.
 * Params: p_pressure - initial pressure to be advertised in [kPa]
 *		   p_temperature - initial temperature to be advertised in [1/100 *C]
 *         p_bat_percentage - initial battery percentage to be advertised
 *         p_leak - initial pressure leak flag to be advertised.
 */
void My_advertising::configureAdvertising(const uint16_t p_pressure, const int16_t p_temperature, const uint8_t p_bat_percentage)
{
    ble_buffer.setPressTempLeak(p_pressure, p_temperature, p_bat_percentage);
    uint32_t err_code;
    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, ble_buffer.getBuffer(), &m_adv_params);
    APP_ERROR_CHECK(err_code);
}



/*
 * Function for starting advertising.
 */
void My_advertising::startAdvertising()
{
    uint32_t err_code;
    err_code = sd_ble_gap_adv_start(m_adv_handle, BLE_CONN_CFG_TAG_DEFAULT);
    APP_ERROR_CHECK(err_code);
}



/*
 * Function for updating advertising.
 * Params: p_pressure - new pressure to be advertised in [kPa]
 *		   p_temperature - new temperature to be advertised in [1/100 *C]
 *         p_bat_percentage - new battery percentage to be advertised
 *         p_leak - new pressure leak flag to be advertised [true - leak detected, false - not].
 */
void My_advertising::updateAdvertising(const uint16_t p_pressure, const int16_t p_temperature, const uint8_t p_bat_percentage)
{
    ble_buffer.setPressTempLeak(p_pressure, p_temperature, p_bat_percentage);
    uint32_t err_code;
    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, ble_buffer.getBuffer(), NULL);
    APP_ERROR_CHECK(err_code);
}
