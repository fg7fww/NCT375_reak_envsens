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

#ifndef BLE_APP_H
#define BLE_APP_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* Length of Bluetooth Address (in bytes) */
#define BDADDR_LENGTH                   6

/* Advertising channel map - 37, 38, 39 */
#define APP_ADV_CHMAP                   0x07

/* Advertising minimum interval - 40ms (64*0.625ms) */
#define APP_ADV_INT_MIN                 64

/* Advertising maximum interval - 40ms (64*0.625ms) */
#define APP_ADV_INT_MAX                 64

/* Non-resolvable private Bluetooth device address.
 * If public address does not exist, the two MSBs must be zero. */
#define PRIVATE_BDADDR                  {0x94,0x11,0x11,0xff,0xff,0x77}

/* Slave preferred connection parameters */
#define PREF_SLV_MIN_CON_INTERVAL       8
#define PREF_SLV_MAX_CON_INTERVAL       10
#define PREF_SLV_LATENCY                0
#define PREF_SLV_SUP_TIMEOUT            200

/* Set the device name */
#define APP_DEVICE_NAME_LENGTH_MAX      20
#define APP_DFLT_DEVICE_NAME            "OnSemi Coin Temp sensor"

/* ON SEMICONDUCTOR Company ID */
#define APP_COMPANY_ID_DATA             {0x4, 0xff, 0x62, 0x3, 0x3}
#define APP_COMPANY_ID_DATA_LEN         (0x4 + 1)

/* Vendor specific scan response data (ON SEMICONDUCTOR Company ID) */
#define APP_SCNRSP_DATA                 APP_COMPANY_ID_DATA
#define APP_SCNRSP_DATA_LEN             APP_COMPANY_ID_DATA_LEN

/* Custom service UUIDs and characteristics */
#define SVC_RF_UUID                     {0x24,0xdc,0x0e,0x6e,0x01,0x40,0xca,0x9e,0xe5,0xa9,0xa3,0x00,0xb5,0xf3,0x93,0xe0}

#define CHAR_TIMEOUT_UUID              {0x24,0xdc,0x0e,0x6e,0x02,0x40,0xca,0x9e,0xe5,0xa9,0xa3,0x00,0xb5,0xf3,0x93,0xe0}
#define CHAR_TIMEOUT_NAME              "TIMEOUT"

#define CHAR_RSSI_AVG_UUID              {0x24,0xdc,0x0e,0x6e,0x03,0x40,0xca,0x9e,0xe5,0xa9,0xa3,0x00,0xb5,0xf3,0x93,0xe0}
#define CHAR_RSSI_AVG_NAME              "RSSI AVG"

#define CHAR_PA_PWR_UUID                {0x24,0xdc,0x0e,0x6e,0x04,0x40,0xca,0x9e,0xe5,0xa9,0xa3,0x00,0xb5,0xf3,0x93,0xe0}
#define CHAR_PA_PWR_NAME                "PA POWER"

#define SVC_ENV_UUID                    {0x1A,0x18}

#define CHAR_TEMP_UUID                  {0x6E,0x2A}
#define CHAR_TEMP_NAME                  "Temp (°C)"


/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/
uint8_t reak_att_desc_max_idx(void);
void DataAccess_PaPower(void *gattm_data, void *app_data, uint16_t length, uint8_t access);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* BLE_APP_H */
