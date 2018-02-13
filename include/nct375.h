/*
 * nct375.h
 *
 *  Created on: Feb 6, 2018
 *      Author: fg7fww
 */

#ifndef NCT375_H_
#define NCT375_H_

/* Selection between power save ONE-SHOT-MODE (temperature register values are updated only in the user specified time)
 * and NORMAL-MODE during the BLE connection time (every 80ms temperature register value is updated automatically by the new value).
 * In the case normal mode selection ONE_SHOT_MODE definition has to be comment. Out of BLE connection, chip is in Shutdown Mode.
 * All circuitry except interface are powered down. */
#define ONE_SHOT_MODE

/* When the FULL_POWER_MODE definition is uncommented then the nct375 chip is in the normal mode out of BLE connection time too.
 * It is the chip maximal power consumption mode */
#define FULL_POWER_MODE

struct NCT375_Reg_tag
{
	uint8_t Config;
	uint8_t Addr;
	uint8_t OneShot;
	short int Thyst;
	short int TOs;
	uint16_t Temp;
};

extern struct NCT375_Reg_tag nct375;

void NCT375_Received_Temperature(void);
void NCT375_ONEShot_ModeOn(void);
void NCT375_ONEShot_StartSample(void);
void NCT375_ONEShot_ModeOff(void);
void NCT375_ONEShotReg_Read(void);
void NCT375_ConfReg_Read(void);
void NCT375_PowerDown(void);
void NCT375_PowerUp(void);
void NCT375_I2C_Delay(void);
void NCT375_THYST_Write(short int);
short int NCT375_THYST_Read(void);
void NCT375_TOS_Write(short int);
short int NCT375_TOS_Read(void);

#endif /* NCT375_H_ */
