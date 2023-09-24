#ifndef SD_FUNC_WRAPPER_H
#define SD_FUNC_WRAPPER_H


#include <stdbool.h>
#include <stdint.h>
extern "C" 
{
#include "nrf_sdh.h"
#include "fds.h"
#include "nrf_pwr_mgmt.h"
}


#define APP_BLE_CONN_CFG_TAG 1 /**< A tag identifying the SoftDevice BLE configuration. */

#define DEAD_BEEF 0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */



/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}



/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
inline void sleepSysOffEnter()
{
    uint32_t err_code = sd_power_system_off(); // Go to system-off mode (this function will not return; wakeup will cause a reset)
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
inline void bleStackInit()
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

   
    uint32_t ram_start = 0;

    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);      // Configure the BLE stack using the default settings.
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_sdh_ble_enable(&ram_start);    // Enable BLE stack.
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for initializing power management.
 */
inline void powerManagementInit()
{
    ret_code_t err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
inline void idle_state_handle()
{
    nrf_pwr_mgmt_run();
}


/**@brief Function for enabling DC2DC converter built in the microcontroller.
 *
 */
inline void enableDC2DC()
{
    ret_code_t err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for disabling DC2DC converter built in the microcontroller.
 *
 */
inline void disableDC2DC()
{
    ret_code_t err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_DISABLE);
	APP_ERROR_CHECK(err_code);
}

#endif