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
 * uart.c
 * - RSL10 UART communication library.
 * - The DIOs used by the I2C interface are not configured by this library; they
 *   have to be configured on the application level.
 * - Known limitations:
 *   > DMA transfers are not supported.
 *   > TX and RX buffer sizes are fixed (by constants defined in uart.h)
 *   > No callback function hooks available
 * ----------------------------------------------------------------------------
 * $Revision: $
 * $Date: $
 * ------------------------------------------------------------------------- */

#include "uart.h"

/* Global variable definition */
struct uart_env_tag    uart_env;

/* Initialization and configuration */

/* ----------------------------------------------------------------------------
 * Function      : void UART_Initialize(uint32_t clk_speed, uint32_t baud)
 * ----------------------------------------------------------------------------
 * Description   : Initialize UART interface
 * Inputs        : - clk_speed - UART clock speed (see Sys_UART_Enable)
                   - baud      - Baud rate (see Sys_UART_Enable)
 * Outputs       : None
 * Assumptions   : The UART DIOs are not configured by this function. They have 
 *                 to be configured on the application level with 
 *                 Sys_UART_DIOConfig.
 * ------------------------------------------------------------------------- */
void UART_Initialize(uint32_t clk_speed, uint32_t baud)
{
    /* Reset the UART application environment */
    memset(&uart_env, 0, sizeof(uart_env));

    /* Enable device UART port */
    Sys_UART_Enable(clk_speed, baud, UART_DMA_MODE_DISABLE);

    /* Enable interrupts */
    NVIC_EnableIRQ(UART_RX_IRQn);
    NVIC_EnableIRQ(UART_TX_IRQn);
}


/**** RX functions ****/

/* ----------------------------------------------------------------------------
 * Function      : uint16_t UART_Available(void)
 * ----------------------------------------------------------------------------
 * Description   : Get the number of bytes available in the RX buffer.
 * Inputs        : None
 * Outputs       : Number of bytes available to read
 * Assumptions   : UART has been initialized with UART_Initialize.
 * ------------------------------------------------------------------------- */
uint16_t UART_Available(void)
{
    return (UART_RX_BUFFER_SIZE +
            uart_env.rx_buffer_write_index -
            uart_env.rx_buffer_read_index) % UART_RX_BUFFER_SIZE;
}

/* ----------------------------------------------------------------------------
 * Function      : uint16_t UART_Read(uint8_t *buffer, uint16_t length)
 * ----------------------------------------------------------------------------
 * Description   : Copies data from the UART RX buffer into a buffer provided 
 *                 by the application.
 * Inputs        : - buffer -   Application data buffer
 *                 - length -   Maximum number of bytes (=characters) to copy
 * Outputs       : Returns number of bytes placed in the application buffer. A
 *                 0 (zero) means that no valid data was found.
 * Assumptions   : UART has been initialized with UART_Initialize.
 * ------------------------------------------------------------------------- */
uint16_t UART_Read(uint8_t *buffer, uint16_t length)
{
    uint16_t i;

    /* Copy the bytes from the UART RX buffer to the application data buffer.
     * Handle the roll-over of the ring-buffer. */
    for (i=0; i<length; i++)
    {
        /* Stop copying the data if no more bytes are available in the RX 
           buffer, even if it was requested to provide more bytes. */
        if (uart_env.rx_buffer_read_index == uart_env.rx_buffer_write_index)
        {
            break;
        }
         
        /* Copy the byte to the application buffer. Update the ring buffer read 
           index */
        *buffer++ = uart_env.rx_buffer[uart_env.rx_buffer_read_index];
        uart_env.rx_buffer_read_index = (uart_env.rx_buffer_read_index+1)%UART_RX_BUFFER_SIZE;
    }
    
    /* Return the number of effectively copied bytes */
    return i;
}

/* ----------------------------------------------------------------------------
 * Function      : uint8_t UART_Peek(uint16_t position)
 * ----------------------------------------------------------------------------
 * Description   : Returns the byte of the RX buffer at the specified position. 
 *                 This position has to be smaller than the available bytes 
 *                 given by UART_Available.
 * Inputs        : - position - Position of the byte in the RX buffer
 * Outputs       : Returns the byte at the specified position. The returned
 *                 byte is invalid if the given position is equal or bigger
 *                 than the available bytes.
 * Assumptions   : UART has been initialized with UART_Initialize. The position
 *                 is valid (e.g. position<UART_Available())
 * ------------------------------------------------------------------------- */
uint8_t UART_Peek(uint16_t position)
{
    return uart_env.rx_buffer[ (uart_env.rx_buffer_read_index+position)%
                               UART_RX_BUFFER_SIZE ];
}

/* ----------------------------------------------------------------------------
 * Function      : int16_t UART_SearchSequence(uint8_t *sequence, 
                                               uint16_t length)
 * ----------------------------------------------------------------------------
 * Description   : Returns the first location of the specified byte sequence 
                   in the RX buffer.
 * Inputs        : - sequence - Target sequence
 *                 - length   - Size of target sequence (in bytes)
 * Outputs       : Returns the location of the target sequence in the RX
 *                 buffer. 0 means that the target sequence is placed on the
 *                 beginning of the RX buffer. -1 means that the target
 *                 sequence is not present in the RX buffer.
 * Assumptions   : UART has been initialized with UART_Initialize.
 * ------------------------------------------------------------------------- */
int16_t UART_SearchSequence(uint8_t *sequence, uint16_t length)
{
    uint16_t read_index = 0;
    uint16_t buf_index1 = uart_env.rx_buffer_read_index;
    uint16_t buf_index2;
    uint16_t seq_index;

    /* Search for the target sequence. Loop the search position over the data
      in the RX buffer */
    while (buf_index1 != uart_env.rx_buffer_write_index)
    {
        /* Check if the current search position contains the target sequence */
        seq_index = 0;
        buf_index2 = buf_index1;
        while (uart_env.rx_buffer[buf_index2] == sequence[seq_index] &&
               buf_index2 != uart_env.rx_buffer_write_index)
        {
            seq_index++;
            buf_index2++;
            
            /* Return the search position if the target sequence has been found */
            if (seq_index == length)
            {
                return read_index;
            }
        }
        
        /* Update the indexes */
        read_index++;
        buf_index1 = (buf_index1+1)%UART_RX_BUFFER_SIZE;
    }
    
    /* The target sequence has been not found, indicated by -1 */
    return -1;
}

/* ----------------------------------------------------------------------------
 * Function      : int16_t UART_SearchBytes(uint8_t *bytes, uint16_t length)
 * ----------------------------------------------------------------------------
 * Description   : Returns the first location of one of the specified bytes 
                   in the RX buffer.
 * Inputs        : - bytes  -   Array of bytes to search
 *                 - length -   Size of the byte array (in bytes)
 * Outputs       : Returns the first location of the RX buffer that contains a
 *                 byte that exists in the provided byte array. 0 means that
 *                 the target sequence is placed on the beginning of the RX
 *                 buffer. -1 means that the target sequence is not present in
 *                 the RX buffer.
 * ------------------------------------------------------------------------- */
int16_t UART_SearchBytes(uint8_t *bytes, uint16_t length)
{
    uint16_t read_index = 0;
    uint16_t rx_buffer_read_index = uart_env.rx_buffer_read_index;
    uint16_t byte_index;

    /* Search for the target sequence. Loop the search position over the data
      in the RX buffer */
    while (rx_buffer_read_index != uart_env.rx_buffer_write_index)
    {
        /* Check if the RX buffer contains at the current position a byte 
           contained by the byte array */
        for (byte_index=0; byte_index<length; byte_index++)
        {
            /* A byte of the array matches, return the buffer location */
            if (uart_env.rx_buffer[rx_buffer_read_index] == bytes[byte_index])
            {
                return read_index;
            }
        }

        /* Update the indexes */
        read_index++;
        rx_buffer_read_index = (rx_buffer_read_index+1)%UART_RX_BUFFER_SIZE;
    }

    /* The target sequence has been not found, indicated by -1 */
    return -1;
}

/* ----------------------------------------------------------------------------
 * Function      : void UART_Empty(void)
 * ----------------------------------------------------------------------------
 * Description   : Empties the TX and RX buffers. The ongoing TX transfer is 
                   completed, but no new bytes are transferred (until the TX
                   buffer is again filled up by UART_Write***.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */

void UART_Empty(void)
{
    uart_env.rx_buffer_read_index = uart_env.rx_buffer_write_index = 0;
}

/**** TX functions ****/

/* ----------------------------------------------------------------------------
 * Function      : void UART_Pending(void)
 * ----------------------------------------------------------------------------
 * Description   : Returns the number of bytes that are currently in the TX 
                   buffer waiting for being transmitted.
 * Inputs        : None
 * Outputs       : Number of bytes in the TX buffer
 * Assumptions   : None
 * Assumptions   : UART has been initialized with UART_Initialize.
 * ------------------------------------------------------------------------- */
uint16_t UART_Pending(void)
{
    return (UART_TX_BUFFER_SIZE +
            uart_env.tx_buffer_write_index -
            uart_env.tx_buffer_read_index) % UART_TX_BUFFER_SIZE;
}

/* ----------------------------------------------------------------------------
 * Function      : uint8_t UART_Write(uint8_t *data, uint16_t length)
 * ----------------------------------------------------------------------------
 * Description   : Copies the data from the byte array into the TX buffer.
 * Inputs        : - data   -   Pointer to the byte array to be transferred
 *                 - length -   Amount of bytes to transfer
 * Outputs       : Return TX buffer overflow status. UART_ERRNO_NONE is
 *                 returned if all bytes could be copied into the RX buffer.
 *                 Otherwise UART_ERRNO_OVERFLOW is returned and the RX buffer
 *                 is filled up.
 * Assumptions   : UART has been initialized with UART_Initialize.
 * ------------------------------------------------------------------------- */
uart_error_code_t UART_Write(uint8_t *data, uint16_t length)
{
    uint16_t i;
    uint16_t next_tx_buffer_write_index;
    uint32_t result = UART_ERRNO_NONE;
    uint32_t tempMask;

    /* Add the data to the TX buffer */
    for (i=0; i<length; i++)
    {
        next_tx_buffer_write_index = (uart_env.tx_buffer_write_index+1)%UART_TX_BUFFER_SIZE;
        if (next_tx_buffer_write_index == uart_env.tx_buffer_read_index)
        {
            result = UART_ERRNO_OVERFLOW;
            break;
        }
        uart_env.tx_buffer[uart_env.tx_buffer_write_index] = *data++;
        uart_env.tx_buffer_write_index = next_tx_buffer_write_index;
    }

    /* Initiate a transfer if no transfer is ongoing. For this create a short
     * critical section to make sure that an ongoing transaction is not
     * completed between the check of the ongoing transaction and the launch of
     * a new transaction. */
    tempMask = __get_PRIMASK();
    __set_PRIMASK(1);
    if (!uart_env.tx_transaction_ongoing && uart_env.tx_buffer_write_index!=uart_env.tx_buffer_read_index)
    {
        UART_TX_IRQHandler();
    }
    __set_PRIMASK(tempMask);

    return result;
}

/* ----------------------------------------------------------------------------
 * Function      : uint8_t UART_WriteString(char *String)
 * ----------------------------------------------------------------------------
 * Description   : Write a character string into the TX buffer.
 * Inputs        : - String -   Character string, zero-ending
 * Outputs       : Return TX buffer overflow status. UART_ERRNO_NONE is
 *                 returned if all bytes could be copied into the RX buffer.
 *                 Otherwise UART_ERRNO_OVERFLOW is returned and the RX buffer
 *                 is filled up.
 * Assumptions   : UART has been initialized with UART_Initialize.
 * ------------------------------------------------------------------------- */
uint8_t UART_WriteString(char *string)
{
    return UART_Write((uint8_t *)string, strlen(string));
}

/* ----------------------------------------------------------------------------
 * Function      : uint8_t UART_WriteInt32(int32_t Value, int8_t DecimalPos)
 * ----------------------------------------------------------------------------
 * Description   : Write a 32-bit integer into the UART TX buffer. The value 
                   can be negative and a decimal position can be specified.
 * Inputs        : - value   -  32-bit integer
 *                 - dec_pos -  Decimal position (0 if not required)
 * Outputs       : Return TX buffer overflow status. UART_ERRNO_NONE is
 *                 returned if all bytes could be copied into the RX buffer.
 *                 Otherwise UART_ERRNO_OVERFLOW is returned and the RX buffer
 *                 is filled up.
 * Assumptions   : UART has been initialized with UART_Initialize.
 * ------------------------------------------------------------------------- */
uint8_t UART_WriteInt32(int32_t value, int8_t dec_pos)
{
    return UART_WriteString( Int32_to_String(value,dec_pos) );
}

/* ----------------------------------------------------------------------------
 * Function      : void UART_Flush(void)
 * ----------------------------------------------------------------------------
 * Description   : Waits for the transmission of outgoing serial data to 
                   complete.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : UART has been initialized with UART_Initialize.
 * ------------------------------------------------------------------------- */
void UART_Flush(void)
{
    while (uart_env.tx_transaction_ongoing)
    {
        SYS_WAIT_FOR_EVENT;
    }
}

/**** UART support functions (internally used by the library ****/

/* ----------------------------------------------------------------------------
 * Function      : void UART_TX_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : TX interrupt service function to handle the byte 
                   transmissions. It is called each time the UART has read the 
                   TX_DATA register to initiate a new transmission. This
                   function fills then the TX_DATA register up with a new byte
                   from the RX buffer. The function can also be called after
                   filling the RX buffer to initiate the transmission of a
                   first byte.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : UART has been initialized with UART_Initialize.
 * ------------------------------------------------------------------------- */
void UART_TX_IRQHandler(void)
{
    if (uart_env.tx_buffer_read_index != uart_env.tx_buffer_write_index)
    {
        uart_env.tx_transaction_ongoing = true;
        UART->TX_DATA = uart_env.tx_buffer[uart_env.tx_buffer_read_index];
        uart_env.tx_buffer_read_index = (uart_env.tx_buffer_read_index+1)%UART_TX_BUFFER_SIZE;
    }
    else
    {
        uart_env.tx_transaction_ongoing = false;
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void UART_RX_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : RX interrupt service function to handle the byte 
                   reception. Each received byte is stored inside the RX 
                   buffer, unless this buffer is full.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : UART has been initialized with UART_Initialize.
 * ------------------------------------------------------------------------- */
void UART_RX_IRQHandler(void)
{
    uint16_t next_rx_buffer_write_index;

    /* Check if the buffer has available space, if if this is the case add the
     * available byte to the buffer */
    next_rx_buffer_write_index = (uart_env.rx_buffer_write_index+1)%UART_RX_BUFFER_SIZE;
    if (next_rx_buffer_write_index != uart_env.rx_buffer_read_index)
    {
        uart_env.rx_buffer[uart_env.rx_buffer_write_index] = UART->RX_DATA;
        uart_env.rx_buffer_write_index = next_rx_buffer_write_index;
    }
}

/* ----------------------------------------------------------------------------
 * Function      : char *Int32_to_String(int32_t value, int8_t dec_pos)
 * ----------------------------------------------------------------------------
 * Description   : Formats an integer value as a string. The value can be
 *                 negative, and a decimal position can be specified.
 * Inputs        : - value   -  Value
 *                 - dec_pos -  Decimal position. Set to 0 if no decimal
 *                              separator is used
 * Outputs       : The pointer to the value string is returned.
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
char *Int32_to_String(int32_t value, int8_t dec_pos)
{
    /* Character array, 10 decimals + sign + decimal pos + zero end char */
    static char Str[13];
    char *p = &Str[13];

    /* Evaluate the sign, and get the absolute value */
    bool Neg = (value<0);
    value = (value<0 ? -value : value);

    /* Last character is the 0-ending character */
    *--p=0;

    /* Loop from the LSD to the MSD */
    while (1)
    {
        /* Add the next digit to the string, update the value */
        *--p=(char)('0'+(value%10));
        value /= 10;
        
        /* Stop the loop if no further characters have to be written */
        if (value==0 && dec_pos<=0)
        {
            break;
        }
        
        /* Add the decimal position if the current position matches */
        if (--dec_pos==0)
        {
            *--p='.';
        }
    }

    /* Add a leading negative sign if necssary */
    if (Neg)
    {
        *--p='-';
    }

    /* Return the pointer to the (effective) string begin */
    return p;
}

