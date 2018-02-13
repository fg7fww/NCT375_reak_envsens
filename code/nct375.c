/*
 * nct375.c
 *
 *  Created on: Feb 6, 2018
 *      Author: fg7fww
 */

#include "app.h"

void NCT375_Received_Temperature(void)
{
	/* Temperature(°C) = (TempCode*100)/16 */

	//int32_t temp = (app_env.i2c_rx_buffer[0]<<4) + (app_env.i2c_rx_buffer[0]>>4);
	int32_t temp = app_env.i2c_rx_buffer[0];
	bool SGN = temp > 127;
	temp = (temp<<4);
	temp += (app_env.i2c_rx_buffer[1]>>4);
	/**/
	if(SGN)	// temperatures less than zero
	{
		temp -=4096;
	}
	/* numbers behind the decimal point */
	temp *= 100;
	/* To obtain real temperatur in (°C) value should be converted to float
	 * and devided by 16 then result is valid
	 */
	temp /= 16;

		app_env.temperature = temp;
		if (app_env.temperature_cccd_value & ATT_CCC_START_NTF)
		{
			REAK_SendNotification(&app_env.temperature);
		}

	UART_WriteEnvData();
}

void NCT375_ONEShot_ModeOn(void)
{
	app_env.i2c_tx_buffer[0]=0x01;	// Configuration register
    app_env.i2c_tx_buffer[1]=0x20;	// OneShot mode DO5 = 1
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 2, NULL, 0, NULL);
}

void NCT375_ONEShot_ModeOff(void)
{
	app_env.i2c_tx_buffer[0]=0x01;	// Configuration register
    app_env.i2c_tx_buffer[1]=0x00;	// OneShot mode DO5 = 0
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 2, NULL, 0, NULL);
}

void NCT375_ONEShot_StartSample(void)
{
	app_env.i2c_tx_buffer[0]=0x04;	// Address pointer register
	app_env.i2c_tx_buffer[1]=0x01;	// irrelevant data
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 2, NULL, 0, NULL);
}

void NCT375_ONEShotReg_Read(void)
{
	void NCT375_ONEShotReg(void)
	{
		nct375.OneShot = app_env.i2c_rx_buffer[0];
	}
	// ONEShot register address writing
	app_env.i2c_tx_buffer[0]=0x04;
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 1, app_env.i2c_rx_buffer, 0, NULL);
	// ONEShot register content reading
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 0, app_env.i2c_rx_buffer, 1, NCT375_ONEShotReg);
}

void NCT375_PowerDown(void)
{
	app_env.i2c_tx_buffer[0]=0x01;	// Configuration register
    app_env.i2c_tx_buffer[1]=0x01;	// Power Down DO0 = 1
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 2, NULL, 0, NULL);
}

void NCT375_PowerUp(void)
{
	app_env.i2c_tx_buffer[0]=0x01;	// Configuration register
    app_env.i2c_tx_buffer[1]=0x00;	// Power Up DO0 = 0
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 2, NULL, 0, NULL);
}

void NCT375_ConfReg_Read(void)
{
	void NCT375_ConfReg(void)
	{
		nct375.Config = app_env.i2c_rx_buffer[0];
	}
	// Configuration register address writing
	app_env.i2c_tx_buffer[0]=0x01;
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 1, app_env.i2c_rx_buffer, 0, NULL);
	// Configuration register content reading
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 0, app_env.i2c_rx_buffer, 1, NCT375_ConfReg);
}

void NCT375_I2C_Delay(void)
{
	// important wait between two i2c data frames
	uint32_t tmp=3000;
	while(tmp){tmp--;}
}

/* temperature hysteresis and  over set register are used in comparasion and interrupt modes
 * (bit D1 configuration register) but chip has to be working in power NORMAL-MODE
 */
// temperature hysteresis register
void NCT375_THYST_Write(short int temp_hyst)
{
	union
	{
		short int hyst;
		uint8_t buffer[2];
	} to_buff;
	to_buff.hyst=temp_hyst;

	// only upper 12 bit is valid limit value
	to_buff.hyst= (to_buff.hyst<<4);

	app_env.i2c_tx_buffer[0]=0x02;	// Address pointer register
	app_env.i2c_tx_buffer[1]=to_buff.buffer[1];
	app_env.i2c_tx_buffer[2]=to_buff.buffer[0];
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 3, NULL, 0, NULL);
}

short int NCT375_THYST_Read(void)
{
	union
	{
		short int hyst;
		uint8_t buffer[2];
	} from_buff;

	short int temp_hyst=0;

	void NCT375_THYSTReg(void)
	{

		from_buff.buffer[0] = app_env.i2c_rx_buffer[1];
		from_buff.buffer[1] = app_env.i2c_rx_buffer[0];
		// only upper 12 bit is valid limit value
		from_buff.hyst = from_buff.hyst>>4;
		/* negative value correction for 16 bits */
		if(app_env.i2c_rx_buffer[1] & 0x80)
		{
			from_buff.buffer[1]=from_buff.buffer[1]|0xF0;
		}
		/**/
		temp_hyst=from_buff.hyst;
	}

	app_env.i2c_tx_buffer[0]=0x02;	// Address pointer register
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 1, NULL, 0, NULL);
	NCT375_I2C_Delay();
	// THYST register content reading
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 0, app_env.i2c_rx_buffer, 2, NCT375_THYSTReg);
	return temp_hyst;
}

// temperature over set alert value register
void NCT375_TOS_Write(short int temp_tos)
{
	union
	{
		short int tos;
		uint8_t buffer[2];
	} to_buff;
	to_buff.tos= temp_tos;
	// only upper 12 bit is valid limit value
	to_buff.tos= (to_buff.tos<<4);

	app_env.i2c_tx_buffer[0]=0x03;	// Address pointer register
	app_env.i2c_tx_buffer[1]=to_buff.buffer[1];
	app_env.i2c_tx_buffer[2]=to_buff.buffer[0];
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 3, NULL, 0, NULL);
}

short int NCT375_TOS_Read(void)
{
	union
	{
		short int tos;
		uint8_t buffer[2];
	} from_buff;

	short int temp_tos=0;

	void NCT375_TOSReg(void)
	{

		from_buff.buffer[0] = app_env.i2c_rx_buffer[1];
		from_buff.buffer[1] = app_env.i2c_rx_buffer[0];
		// only upper 12 bit is valid limit value
		from_buff.tos = from_buff.tos>>4;
		/* negative value correction for 16 bits */
		if(app_env.i2c_rx_buffer[1] & 0x80)
		{
			from_buff.buffer[1]=from_buff.buffer[1]|0xF0;
		}
		temp_tos=from_buff.tos;
	}
	app_env.i2c_tx_buffer[0]=0x03;	// Address pointer register
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 1, NULL, 0, NULL);
	NCT375_I2C_Delay();
	// TOS register content reading
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 0, app_env.i2c_rx_buffer, 2, NCT375_TOSReg);
	return temp_tos;
}
