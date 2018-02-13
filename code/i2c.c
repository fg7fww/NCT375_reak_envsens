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
 * i2c.c
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

#include <i2c.h>

/* Global variable definitions */

struct i2c_env_tag    i2c_env;


/* Initialization and configuration */

/* ----------------------------------------------------------------------------
 * Function      : void I2C_Master_Init(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the I2C interface in master mode
 * Inputs        : - speed  -   Speed configuration (e.g. 
 *                              I2C_MASTER_SPEED_768_BYTE)
 * Outputs       : None
 * Assumptions   : The I2C DIOs are not configured by this function. They have 
 *                 to be configured on the application level with
 *                 Sys_I2C_DIOConfig.
 * ------------------------------------------------------------------------- */
void I2C_Master_Init(uint8_t speed)
{
    /* Reset the I2C application environment */
    memset(&i2c_env, 0, sizeof(i2c_env));

    /* Configure the I2C interface */
    Sys_I2C_Config(((uint32_t)(speed << I2C_CTRL0_SPEED_Pos)) |
                   I2C_CONTROLLER_CM3 | I2C_STOP_INT_ENABLE |
                   I2C_AUTO_ACK_DISABLE  | I2C_SAMPLE_CLK_ENABLE | I2C_SLAVE_DISABLE );

    /* Configure I2C debug DIO */
    #ifdef I2C_DBG_DIO_NUM
        Sys_DIO_Config(I2C_DBG_DIO_NUM, DIO_MODE_GPIO_OUT_0);
        Sys_GPIO_Set_Low(I2C_DBG_DIO_NUM);
    #endif

    /* Enable interrupts */
    NVIC_EnableIRQ(I2C_IRQn);

 }

/**** Write/read functions ****/

/* ----------------------------------------------------------------------------
 * Function      : void I2C_WriteRead(uint8_t address, 
                                      uint8_t *txdata, uint16_t txlength,
                                      uint8_t *rxdata, uint16_t rxlength, 
                                      void *callback)
 * ----------------------------------------------------------------------------
 * Description   : Initiates a write, read or combined write-read transaction, 
                   of one or multiple bytes. Optionally a callback function 
                   can be provided that will be called once the tranasction is 
                   completed. If both, write and read parameters are provided 
                   (e.g txdata/txlength, rxdata/rxlength), the write sequence
                   is first performed followed by the read sequence. This 
                   allows running a sequence of 1) sending device address, 
                   2) sending memory address, 3) receiving memory data.
 * Inputs        : - address  - 7-bit slave address
                   - txdata   - Pointer to the TX data. Ignored if txlength=0
                   - txlength - Number of bytes to transfer. To be set to 0 for
                                read transactions.
                   - rxdata   - Pointer to the RX data. Ignored if rxlength=0
                   - rxlength - Number of bytes to receive. To be set to 0 for
                                write transactions.
                   - callback - Pointer to function that will be called if the
                                transaction is completed. To be set to NULL if 
                                no callback function is used.
 * Outputs       : None
 * Assumptions   : The I2C interface has previously been configured with 
                   I2C_Master_Init.
 * ------------------------------------------------------------------------- */
void I2C_WriteRead(uint8_t address, uint8_t *txdata, uint16_t txlength,
              uint8_t *rxdata, uint16_t rxlength, void *callback)
{
    /* Copy the transaction parameters to the I2C environment structure */
    i2c_env.address = address;
    i2c_env.tx_buffer = txdata;
    i2c_env.tx_buffer_length = txlength;
    i2c_env.rx_buffer = rxdata;
    i2c_env.rx_buffer_length = rxlength;
    i2c_env.callbackfunction = callback;

     /* Start the transaction by reseting the interface */
    Sys_I2C_Reset();

    /* Start either a TX or RX transaction with the device selected with the 
       provided address. */
    if (txlength > 0)
    {
        Sys_I2C_StartWrite(address);
    }
    else if (rxlength > 0)
    {
        i2c_env.tx_buffer_length--;
        Sys_I2C_StartRead(address);
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void I2C_Write(uint8_t address, 
                                  uint8_t *txdata, uint16_t txlength,
                                  void *callback)
 * ----------------------------------------------------------------------------
 * Description   : Initiates a write transaction of one or multiple 
                   bytes. Optionally a callback function can be provided that
                   will be called once the tranasction is completed.
 * Inputs        : - address  - 7-bit slave address
                   - txdata   - Pointer to the TX data. Ignored if txlength=0
                   - txlength - Number of bytes to transfer.
                   - callback - Pointer to function that will be called if the
                                transaction is completed. To be set to NULL if 
                                no callback function is used.
 * Outputs       : None
 * Assumptions   : The I2C interface has previously been configured with 
                   I2C_Master_Init.
 * ------------------------------------------------------------------------- */
void I2C_Write(uint8_t address, uint8_t *txdata, uint16_t txlength,
               void *callback)
{
    I2C_WriteRead(address, txdata, txlength, NULL, 0, callback);
}

/* ----------------------------------------------------------------------------
 * Function      : void I2C_Read(uint8_t address, 
                                 uint8_t *rxdata, uint16_t rxlength, 
                                 void *callback)
 * ----------------------------------------------------------------------------
 * Description   : Initiates a read transaction of one or multiple 
                   bytes. Optionally a callback function can be provided that
                   will be called once the tranasction is completed.
 * Inputs        : - address  - 7-bit slave address
                   - rxdata   - Pointer to the RX data.
                   - rxlength - Number of bytes to receive. To be set to 0 for
                                write transactions.
                   - callback - Pointer to function that will be called if the
                                transaction is completed. To be set to NULL if 
                                no callback function is used.
 * Outputs       : None
 * Assumptions   : The I2C interface has previously been configured with 
                   I2C_Master_Init.
 * ------------------------------------------------------------------------- */
void I2C_Read(uint8_t address, uint8_t *rxdata, uint16_t rxlength,
              void *callback)
{
    I2C_WriteRead(address, NULL, 0, rxdata, rxlength, callback);
}


/**** Support functions (internally used by the library) ****/

/* ----------------------------------------------------------------------------
 * Function      : void I2C_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : I2C interrupt service function to handle all read and write 
                   operations. It is called for each received or transmitted 
                   byte. It transfers the bytes from the TX buffer to the I2C 
                   interface, and from this latest one to the RX buffer.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : The I2C interface has previously been configured with 
                   I2C_Master_Init and a transaction has been initiated with
                   one of the read or write functions.
 * ------------------------------------------------------------------------- */
void I2C_IRQHandler(void)
{
    /* Toggle the debug IO in debug mode */
    #ifdef I2C_DBG_DIO_NUM
        Sys_GPIO_Toggle(I2C_DBG_DIO_NUM);
    #endif
     
     /* Read the current I2C interface status */
    i2c_env.last_status = Sys_I2C_Get_Status();

    /* Handle write/TX transfers (priority over read transaction) */
    if ((i2c_env.last_status & (1<<I2C_STATUS_READ_WRITE_Pos)) == I2C_IS_WRITE)
    {
        /* As long as the TX buffer contains data, transfer the next byte */
        if (i2c_env.tx_buffer_length > 0)
        {
            i2c_env.tx_buffer_length--;
            I2C->DATA = *i2c_env.tx_buffer++;
            if (i2c_env.tx_buffer_length == 0)
            {
                I2C_CTRL1->LAST_DATA_ALIAS = I2C_LAST_DATA_BITBAND;
            }
        }

        /* If the last byte has been transfered, initiate the read sequence if 
           required. If not required call the callback function if defined. */
        else if (i2c_env.tx_buffer_length == 0)
        {
            i2c_env.tx_buffer_length--;
            if (i2c_env.rx_buffer_length > 0)
            {
                Sys_I2C_StartRead(i2c_env.address);
            }
            else if (i2c_env.callbackfunction != NULL)
            {
                ((void(*)())i2c_env.callbackfunction)();
            }
        }
    }
    
    /* Handle the read/RX transfers (this happens once the TX transfers have
       been completed */
    else if ((i2c_env.last_status & (1<<I2C_STATUS_READ_WRITE_Pos)) == I2C_IS_READ)
    {
        /* Initiate a new read/RX transfer without preceeding write/TX transfer. 
           This is indicated by the I2C_BUFFER_FULL flag that is not set. Send 
           an ACK to start the read operation. Indicate the last byte to 
           receive if only a single by has to be read. */
        if ((i2c_env.last_status & (1<<I2C_STATUS_BUFFER_FULL_Pos)) != I2C_BUFFER_FULL)
        {
            Sys_I2C_ACK();
            if (i2c_env.rx_buffer_length == 1)
            {
                I2C_CTRL1->LAST_DATA_ALIAS = I2C_LAST_DATA_BITBAND;
            }
        }
          
        /* As soon as not the last byte has been received, acknowlege the 
           received byte and initiate the read transfer of the next byte. Send 
           for each received byte an ACK. Indicate to read the last byte if 
           necessary. */
        else if (i2c_env.rx_buffer_length > 1)
        {
            i2c_env.rx_buffer_length--;
            Sys_I2C_ACK();
            *i2c_env.rx_buffer++ = I2C->DATA;
            if (i2c_env.rx_buffer_length == 1)
            {
                I2C_CTRL1->LAST_DATA_ALIAS = I2C_LAST_DATA_BITBAND;
            }
        }
          
        /* If the last byte has been received, read it from the buffer, and 
           call the callback function if such one is defined */
        else if (i2c_env.rx_buffer_length == 1)
        {
            i2c_env.rx_buffer_length--;
            *i2c_env.rx_buffer++ = I2C->DATA;
            if (i2c_env.callbackfunction != NULL)
            {
                ((void(*)())i2c_env.callbackfunction)();
            }
        }
    }
}
