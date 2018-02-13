/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2017 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * This module is derived in part from example code provided by RivieraWaves
 * and as such the underlying code is the property of RivieraWaves [a member
 * of the CEVA, Inc. group of companies], together with additional code which
 * is the property of ON Semiconductor. The code (in whole or any part) may not
 * be redistributed in any form without prior written permission from
 * ON Semiconductor.
 *
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * app.h
 * - Main application header
 * ----------------------------------------------------------------------------
 * $Revision: 1.41 $
 * $Date: 2017/06/12 13:54:08 $
 * ------------------------------------------------------------------------- */

#ifndef APP_H
#define APP_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/
#include <rsl10.h>
#include <rsl10_ke.h>
#include <rsl10_ble.h>
#include <rsl10_profiles.h>
#include <rsl10_map_nvr.h>
#include <stdbool.h>

#include "app_system.h"

#include "i2c.h"
#include "uart.h"

#include "ble_reak.h"
#include "ble_std.h"
#include "app_ble.h"
#include "nct375.h"

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* Minimum and maximum VBAT measurements */
#define VBAT_1p1V_MEASURED              0x1199
#define VBAT_3p3V_MEASURED              0x34CC

/* DIO number that is connected to LED of EVB */
#define LED_DIO_NUM                     6

/* DIO used for the I2C interface to interface the SI7042 sensor */
#define I2C_SDA_DIO_NUM                 12 /* 0 */
#define I2C_SCL_DIO_NUM                 11 /* 1 */
#define I2C_GND_DIO_NUM                 10 /* 2 */
#define I2C_PWR_DIO_NUM                 8  /* 4 */

#define UART_CFG_SYS_CLK                SystemCoreClock
#define UART_BAUD_RATE                  115200
#define UART_TX_DIO_NUM                 0
#define UART_RX_DIO_NUM                 1

#define SPI_SCLK_DIO_NUM                2
#define SPI_MOSI_DIO_NUM                3
#define SPI_CS_DIO_NUM                  4
#define SPI_MISO_DIO_NUM                7


/* NCT375 I2C commands */
#define NCT375_CMD_GET_TEMPERATURE					(uint8_t[]){0x00}
#define NCT375_CMD_GET_TEMPERATURE_ONE_SHOT			(uint8_t[]){0x04}

/* Temperature change notification thresholds, min value = xx */
#define NOTIF_THRES_TEMPERATURE  1

/* Set timer to 1000 ms (100 times the 10 ms kernel timer resolution) */
#define TIMER_1S_SETTING                100

/* Max connection time */
#define INIT_TIMEOUT_TIME_1S            (5*60)

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/

/* Application Environment Structure */

struct app_env_tag
{
	/* Indication to update the exposed BLE data */
	bool update_ble_data;

	/* SI7042 firmware revision code */
	//uint8_t si7042_firmware_rev_code;

	/* Temperature value and CCCD */
	int16_t temperature;
    uint16_t temperature_cccd_value;
    //float temperature2;

    /* Timeout value (in seconds) and CCCD*/
    int16_t timeout;
    uint16_t timeout_cccd;

    /* RSSI average value and CCCD*/
    int8_t rssi_avg;
    uint16_t rssi_avg_cccd;

    /* PA power value and CCCD*/
    int8_t pa_power;
    uint16_t pa_power_cccd;

    /* I2C reception buffer */
    uint8_t i2c_rx_buffer[8];

    /* I2C reception buffer */
    uint8_t i2c_tx_buffer[8];
};

extern struct app_env_tag app_env;

/* ---------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/
extern void System_Initialize(void);
extern int APP_Timer(ke_msg_id_t const msg_id, void const *param,
                     ke_task_id_t const dest_id, ke_task_id_t const src_id);
void App_Env_Initialize(void);
//void SI7042_Received_FwRevCode(void);
//void SI7042_Received_Humidity(void);
void UART_WriteEnvData(void);
//void LCD_ShowAll(void);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* APP_H */
