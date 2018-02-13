/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2017 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * i2c.h
 * - RSL10 I2C communication library. Only master mode/auto acknowlege mode is
 *   currently supported.
 * - The DIOs used by the I2C interface are not configured by this library; they
 *   have to be configured on the application level.
 * - Known limitations:
 *   > DMA transfers are not supported.
 *   > I2C bus errors are not treated.
 * ----------------------------------------------------------------------------
 * $Revision: $
 * $Date: $
 * ------------------------------------------------------------------------- */

#ifndef I2C_H
#define I2C_H

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

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* Debug support: To toggle a DIO each time the I2C interrupt is called, define
 * I2C_DBG_DIO_NUM (un-comment the following line). */
/* #define I2C_DBG_DIO_NUM 9 */


/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/

/* I2C environment */
struct i2c_env_tag
{
	uint8_t last_status;
	uint8_t address;
	uint8_t *tx_buffer;
	int16_t tx_buffer_length;
	uint8_t *rx_buffer;
	int16_t rx_buffer_length;
	void *callbackfunction;
};
extern struct i2c_env_tag    i2c_env;

/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/
 
/**** Initialization and configuration *****/

/* I2C_Master_Init: Initialize the I2C interface in master mode */
void I2C_Master_Init(uint8_t speed);

/**** Write/read functions ****/

/* I2C_WriteRead: Initiates a write, read or combined write-read transaction, 
                  of one or multiple bytes */
void I2C_WriteRead(uint8_t address, uint8_t *txdata, uint16_t txlength,
		             uint8_t *rxdata, uint16_t rxlength, void *callback);

/* I2C_Write: Initiates a write transaction of one or multiple bytes */
void I2C_Write(uint8_t address, uint8_t *txdata, uint16_t txlength,
		         void *callback);

/* I2C_Read: Initiates a read transaction of one or multiple bytes */
void I2C_Read(uint8_t address, uint8_t *rxdata, uint16_t rxlength,
		        void *callback);

/**** Support functions (internally used by the library) ****/

/* I2C_IRQHandler: I2C interrupt service function to handle all read and write 
                   operations */
void I2C_IRQHandler(void);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* I2C_H_ */
