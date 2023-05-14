#include "Ble_buffer.h"


/* 
 * Constructor, initializes buffers with data from my_config.h
 */
Ble_buffer::Ble_buffer()
{
    use_buffer1 = true;
    memcpy(my_adv_data_1, cfg::MY_ADV_DATA, cfg::ADV_DATA_L);
    memcpy(my_scan_data_1, cfg::MY_SCAN_RESP_DATA, cfg::SCAN_DATA_L);
    memcpy(my_adv_data_2, cfg::MY_ADV_DATA, cfg::ADV_DATA_L);
    memcpy(my_scan_data_2, cfg::MY_SCAN_RESP_DATA, cfg::SCAN_DATA_L);

    ble_adv_struct_1.adv_data.p_data = my_adv_data_1;
    ble_adv_struct_1.adv_data.len = cfg::ADV_DATA_L;
    ble_adv_struct_1.scan_rsp_data.p_data = my_scan_data_1;
    ble_adv_struct_1.scan_rsp_data.len = cfg::SCAN_DATA_L;

    ble_adv_struct_2.adv_data.p_data = my_adv_data_2;
    ble_adv_struct_2.adv_data.len = cfg::ADV_DATA_L;
    ble_adv_struct_2.scan_rsp_data.p_data = my_scan_data_2;
    ble_adv_struct_2.scan_rsp_data.len = cfg::SCAN_DATA_L;
}



/*
 * Function for returning pointer to buffer to be advertised.
 * Returns: pointer to buffer that holds new advertising data.
 */
ble_gap_adv_data_t *Ble_buffer::getBuffer()
{
    ble_gap_adv_data_t *return_value;
    if (use_buffer1)
    {
        return_value = &ble_adv_struct_1;
    }
    else
    {
        return_value = &ble_adv_struct_2;
    }
    use_buffer1 = !use_buffer1;
    return return_value;
}



/*
 * Function for updating Ble_buffer with new data.
 * Params: p_pressure - new pressure to be advertised in [kPa]
 *		   p_temperature - new temperature to be advertised in [1/100 *C]
 *         p_bat_percentage - new battery percentage to be advertised
 *         p_leak - new pressure leak flag to be advertised [true - leak detected, false - not].
 */
void Ble_buffer::setPressTempLeak(const uint32_t p_pressure, const int32_t p_temperature, const uint8_t p_bat_percentage, const bool p_leak)
{
    setPressure(p_pressure);
    setTemp(p_temperature);
    setBat(p_bat_percentage);
    setLeakFlag(p_leak);
}



/*
 * Function for updating Ble_buffer with new pressure data.
 * Params: p_pressure - new pressure to be advertised in [kPa]
 */
void Ble_buffer::setPressure(uint32_t p_pressure)
{
    p_pressure = p_pressure * 1000;      // sensors send pressure in 1/1000 kPa units. App remaps it.
    if (use_buffer1)
    {
        my_adv_data_1[PRESS_POS] = p_pressure & 0x000000FF;      // converts uint32_t to 4 uint8_t array, little endian
        my_adv_data_1[PRESS_POS + 1] = (p_pressure & 0x0000FF00) >> 8;    
        my_adv_data_1[PRESS_POS + 2] = (p_pressure & 0x00FF0000) >> 16;
        my_adv_data_1[PRESS_POS + 3] = (p_pressure & 0xFF000000) >> 24;
    }
    else
    {
        my_adv_data_2[PRESS_POS] = p_pressure & 0x000000FF;      // converts uint32_t to 4 uint8_t array, little endian
        my_adv_data_2[PRESS_POS + 1] = (p_pressure & 0x0000FF00) >> 8;
        my_adv_data_2[PRESS_POS + 2] = (p_pressure & 0x00FF0000) >> 16;
        my_adv_data_2[PRESS_POS + 3] = (p_pressure & 0xFF000000) >> 24;
    }
}




/*
 * Function for updating Ble_buffer with new temperature data.
 * Params: p_temp - new temperature to be advertised in [1/100 *C]
 */
void Ble_buffer::setTemp(int32_t p_temp)
{
    if (use_buffer1)
    {
        my_adv_data_1[TEMP_POS] = p_temp & 0x000000FF;         // converts uint32_t to 4 uint8_t array, little endian
        my_adv_data_1[TEMP_POS + 1] = (p_temp & 0x0000FF00) >> 8;
        my_adv_data_1[TEMP_POS + 2] = (p_temp & 0x00FF0000) >> 16;
        my_adv_data_1[TEMP_POS + 3] = (p_temp & 0xFF000000) >> 24;
    }
    else
    {
        my_adv_data_2[TEMP_POS] = p_temp & 0x000000FF;        // converts uint32_t to 4 uint8_t array, little endian
        my_adv_data_2[TEMP_POS + 1] = (p_temp & 0x0000FF00) >> 8;
        my_adv_data_2[TEMP_POS + 2] = (p_temp & 0x00FF0000) >> 16;
        my_adv_data_2[TEMP_POS + 3] = (p_temp & 0xFF000000) >> 24;
    }
}



/*
 * Function for updating Ble_buffer with new bat percentage data.
 * Params: p_percentage - new bat percentage to be advertised in [%]
 */
void Ble_buffer::setBat(uint8_t p_percentage)
{
    if (p_percentage > 100)     // check for reasonability. Btw Android app crashes if the battery percentage is >100%...
    {
        return;
    }
    if (use_buffer1)
    {
        my_adv_data_1[BAT_POS] = p_percentage;
    }
    else
    {
        my_adv_data_2[BAT_POS] = p_percentage;
    }
}



/*
 * Function for updating Ble_buffer with new pressure leak flag data.
 * Params: p_leak - new pressure leak flag. True - pressure leak detected, false - undetected
 */
void Ble_buffer::setLeakFlag(bool p_leak)
{
    if (use_buffer1)
    {
        my_adv_data_1[LEAK_FLAG_POS] = (uint8_t)p_leak;
    }
    else
    {
        my_adv_data_2[LEAK_FLAG_POS] = (uint8_t)p_leak;
    }
}