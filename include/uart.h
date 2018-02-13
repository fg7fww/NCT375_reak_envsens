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

#ifndef UART_H
#define UART_H

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
#define UART_TX_BUFFER_SIZE             0x40
#define UART_RX_BUFFER_SIZE             0x40

/* Define error codes */

typedef enum
{
	UART_ERRNO_NONE,
	UART_ERRNO_OVERFLOW
} uart_error_code_t;

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/

/* I2C environment */
struct uart_env_tag
{
	uint8_t tx_buffer[UART_TX_BUFFER_SIZE];
	uint16_t tx_buffer_write_index;
	volatile uint16_t tx_buffer_read_index;
	volatile bool tx_transaction_ongoing;

	uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
	volatile uint16_t rx_buffer_write_index;
	uint16_t rx_buffer_read_index;
};

extern struct uart_env_tag    env_tag;


/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/

 
/**** Initialization and configuration *****/

/* UART_Initialize: Initialize UART interface */
void UART_Initialize(uint32_t clk_speed, uint32_t baud);


/**** RX functions ****/

/* UART_Available: Get the number of bytes available in the RX buffer */
uint16_t UART_Available(void);

/* UART_Read: Copies data from the UART RX buffer into a buffer provided by the 
   application */
uint16_t UART_Read(uint8_t *buffer, uint16_t length);

/* UART_Peek: Returns the byte of the RX buffer at the specified position */
uint8_t UART_Peek(uint16_t position);

/* UART_SearchSequence: Returns the first location of the specified byte 
   sequence in the RX buffer */
int16_t UART_SearchSequence(uint8_t *sequence, uint16_t length);

/* UART_SearchBytes: Returns the first location of one of the specified bytes 
   in the RX buffer */
int16_t UART_SearchBytes(uint8_t *bytes, uint16_t length);

/* UART_Empty: Empties the TX and RX buffers */
void UART_Empty(void);


/**** TX functions ****/

/* UART_Pending: Returns the number of bytes that are currently in the TX 
   buffer waiting for being transmitted. */
uint16_t UART_Pending(void);

/* UART_Write: Copies the data from the byte array into the TX buffer */
uart_error_code_t UART_Write(uint8_t *data, uint16_t length);

/* UART_WriteString: Write a character string into the TX buffer */
uint8_t UART_WriteString(char *string);

/* UART_WriteInt32: Write a 32-bit integer into the UART TX buffer */
uint8_t UART_WriteInt32(int32_t value, int8_t dec_pos);

/* UART_Flush: Waits until all data in the TX buffer has been transmitted */
void UART_Flush(void);


/**** Support functions (internally used by the library ****/

/* UART_TX_IRQHandler: TX interrupt service function to handle the byte 
   transmissions */
void UART_TX_IRQHandler(void);

/* UART_RX_IRQHandler: RX interrupt service function to handle the byte 
   reception */
void UART_RX_IRQHandler(void);

/* Int32_to_String: Formats an integer value as a string */
char *Int32_to_String(int32_t value, int8_t dec_pos);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* UART_H_ */
