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
 * app_system.c
 * - Application related system components - Initialization function, task
 *   handler definition and support processes
 * ----------------------------------------------------------------------------
 * $Revision: $
 * $Date: $
 * ------------------------------------------------------------------------- */

#include "app.h"


/* Custom service definitions */

const struct reak_att_desc reak_att[] =
{
    /**** Service 0 - Environment data ****/
	REAK_SERVICE_UUID_16(SVC_ENV_UUID),

    /* Temperature */
    REAK_CHAR_UUID_16(CHAR_TEMP_UUID,
                      PERM(RD,ENABLE) | PERM(NTF,ENABLE),
                      sizeof(app_env.temperature), &app_env.temperature, REAK_GenericDataAccess),
    REAK_CHAR_CCC(&app_env.temperature_cccd_value, REAK_GenericDataAccess),

	/**** Service 1 ****/
    REAK_SERVICE_UUID_128(SVC_RF_UUID),

    /*  Timeout */
    REAK_CHAR_UUID_128(CHAR_TIMEOUT_UUID,
                       PERM(RD,ENABLE) | PERM(NTF,ENABLE),
                       sizeof(app_env.timeout), &app_env.timeout, REAK_GenericDataAccess),
    REAK_CHAR_CCC(&app_env.timeout_cccd, REAK_GenericDataAccess),
    REAK_CHAR_USER_DESC(sizeof(CHAR_TIMEOUT_NAME)-1, CHAR_TIMEOUT_NAME, REAK_GenericDataAccess),

    /*  RSSI average */
    REAK_CHAR_UUID_128(CHAR_RSSI_AVG_UUID,
                       PERM(RD,ENABLE) | PERM(NTF,ENABLE),
                       sizeof(app_env.rssi_avg), &app_env.rssi_avg, REAK_GenericDataAccess),
    REAK_CHAR_CCC(&app_env.rssi_avg_cccd, REAK_GenericDataAccess),
    REAK_CHAR_USER_DESC(sizeof(CHAR_RSSI_AVG_NAME)-1, CHAR_RSSI_AVG_NAME, REAK_GenericDataAccess),

    /*  PA power */
    REAK_CHAR_UUID_128(CHAR_PA_PWR_UUID,
                       PERM(RD,ENABLE) | PERM(WRITE_REQ,ENABLE) | PERM(WRITE_COMMAND,ENABLE),
                       sizeof(app_env.pa_power), &app_env.pa_power, DataAccess_PaPower),
    REAK_CHAR_CCC(&app_env.pa_power_cccd, REAK_GenericDataAccess),
    REAK_CHAR_USER_DESC(sizeof(CHAR_PA_PWR_NAME)-1, CHAR_PA_PWR_NAME, REAK_GenericDataAccess),
};

uint8_t reak_att_desc_max_idx(void)
{
    return sizeof(reak_att)/sizeof(struct reak_att_desc);
}

/* ----------------------------------------------------------------------------
 * Function      : void DataAccess_PaPower(void *gattm_data, void *app_data,
 *                                         uint16_t length, uint8_t access)
 * ----------------------------------------------------------------------------
 * Description   : Function to transfer the PA power data between the application
 *                 and the GATTM. An update made by the GATTM will be written
 *                 to the PA power configuration register
 * Inputs        : - gattm_data : Pointer to the GATTM data structure
 *                 - app_data   : Pointer to the application data structure
 *                 - length     : Data length (in bytes)
 *                 - access     : Data access (reak_cb_read or reak_cb_write)
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void DataAccess_PaPower(void *gattm_data, void *app_data, uint16_t length, uint8_t access)
{
    REAK_GenericDataAccess(gattm_data, app_data, length, access);
    if (access != reak_cb_read)
    {
    	app_env.pa_power = (app_env.pa_power < -3 ? -3 : (app_env.pa_power > 12 ? 12 : app_env.pa_power));
        RF_REG19->PA_PWR_BYTE = (RF_REG19->PA_PWR_BYTE & ~RF_REG19_PA_PWR_PA_PWR_BYTE_Mask) |
        					    (app_env.pa_power & RF_REG19_PA_PWR_PA_PWR_BYTE_Mask);
    }
}
