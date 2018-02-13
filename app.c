/* ----------------------------------------------------------------------------
 * Copyright (c) 2016 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 * ----------------------------------------------------------------------------
 * app.c
 * - Main application file
 * ----------------------------------------------------------------------------
 * $Revision: 1.19 $
 * $Date: 2017/06/14 16:09:21 $
 * ------------------------------------------------------------------------- */

#include "app.h"

/* Application Environment Structure */
struct app_env_tag app_env;
struct NCT375_Reg_tag nct375;
extern void TIMER0_IRQHandler(void);
extern void TIMER1_IRQHandler(void);

/* ----------------------------------------------------------------------------
 * Function      : void LCD_ShowAll()
 * ----------------------------------------------------------------------------
 * Description   : Display on the LCD the full status
 * Inputs        : None
 * Outputs       : void
 * Assumptions   : None
 * ------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
 * Function      : void UART_WriteEnvData()
 * ----------------------------------------------------------------------------
 * Description   : Write the environment data to the UART
 * Inputs        : None
 * Outputs       : void
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void UART_WriteEnvData(void)
{
	UART_WriteInt32(app_env.temperature, 2);
	UART_WriteString("C  ");
//	UART_WriteInt32(app_env.humidity, 2);
	UART_WriteString("%\n\r");
}


/* ----------------------------------------------------------------------------
 * Function      : int APP_Timer(ke_msg_idd_t const msg_id,
 *                               void const *param,
 *                               ke_task_id_t const dest_id,
 *                               ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle timer event message
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter (unused)
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int APP_Timer(ke_msg_id_t const msg_id,
              void const *param,
              ke_task_id_t const dest_id,
              ke_task_id_t const src_id)
{
    int16_t rssi_avg;

    /* Restart timer */
    ke_timer_set(APP_TIMER, TASK_APP, TIMER_1S_SETTING);

    /* Turn on LED of EVB if the link is established and
     * blinking when it is advertising */
    switch(ble_env.state)
    {
        case APPM_CONNECTED:
            Sys_GPIO_Set_High(LED_DIO_NUM);
            /* Disconnect a connection that takes longer than 5 minutes */
            switch (app_env.timeout)
            {
            	case 0:
            		//Connection_Disconnect();
            		break;
            	case -1:
            		break;
            	default:
                    ;//app_env.timeout--;
            }
            break;
        case APPM_ADVERTISING:
            Sys_GPIO_Toggle(LED_DIO_NUM);
            app_env.timeout = INIT_TIMEOUT_TIME_1S;
            break;
        default:
            Sys_GPIO_Set_Low(LED_DIO_NUM);
            app_env.timeout = INIT_TIMEOUT_TIME_1S;
    }

    /* Update some service characteristics */

    /* Update the timeout */
   	if ( ble_env.state==APPM_CONNECTED && (app_env.timeout_cccd & ATT_CCC_START_NTF) )
   	{
   		REAK_SendNotification(&app_env.timeout);
   	}

   	/* Update the RSSI and notify an eventual change if notification is enabled
   	 * RSSI[dBm] = 0.317 * RF_REG32->RSSI_AVG - 107.9 */
    rssi_avg = (RF_REG32->RSSI_AVG_RSSI_AVG_BYTE * 81 - 27622)>>8;
    if ( ble_env.state==APPM_CONNECTED && rssi_avg != app_env.rssi_avg )
    {
        app_env.rssi_avg = rssi_avg;
        if ( app_env.rssi_avg_cccd & ATT_CCC_START_NTF )
        {
            REAK_SendNotification(&app_env.rssi_avg);
        }
    }

    app_env.update_ble_data = true;

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void App_Env_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize application environment
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void App_Env_Initialize(void)
{
    /* Reset the application manager environment */
    memset(&app_env, 0, sizeof(app_env));
	app_env.pa_power = (RF_REG19->PA_PWR_BYTE & RF_REG19_PA_PWR_PA_PWR_BYTE_Mask);

    /* Configure DIOs */
    Sys_DIO_Config(LED_DIO_NUM, DIO_MODE_GPIO_OUT_0);

    /* Configure I2C as Master, increase its interrupt priority to manage
     * quicker the transferred data */
    I2C_Master_Init(0x80U);
    NVIC_SetPriority(I2C_IRQn,2);

    /* Configure the DIOs for I2C */
    Sys_I2C_DIOConfig(DIO_6X_DRIVE | DIO_LPF_ENABLE | DIO_STRONG_PULL_UP,
    		          I2C_SCL_DIO_NUM,
    		          I2C_SDA_DIO_NUM);

    /* Configure the DIO used as ground and power pins for the SI7042 */
    Sys_DIO_Config(I2C_GND_DIO_NUM, DIO_MODE_GPIO_OUT_0);
    Sys_DIO_Config(I2C_PWR_DIO_NUM, DIO_MODE_GPIO_OUT_1);

    /* Configure the UART and the DIO used by it */
    Sys_UART_DIOConfig(DIO_2X_DRIVE | DIO_WEAK_PULL_UP | DIO_LPF_ENABLE,
                       UART_TX_DIO_NUM, UART_RX_DIO_NUM);
    UART_Initialize( UART_CFG_SYS_CLK, UART_BAUD_RATE );

    /* Timer 0 start taking sample value */
    Sys_Timer_Set_Control(0,  TIMER_MULTI_COUNT_1 |
                              TIMER_FREE_RUN      |
                              TIMER_SLOWCLK_DIV2  |
                              TIMER_PRESCALE_32   | 14000);
    //Sys_Timers_Stop(SELECT_TIMER0);
#ifndef FULL_POWER_MODE
    NVIC_EnableIRQ(TIMER0_IRQn);
#endif

    /* Timer 1 read sample value */
    Sys_Timer_Set_Control(1,  TIMER_MULTI_COUNT_1 |
                              TIMER_SHOT_MODE     |
                              TIMER_SLOWCLK_DIV2  |
                              TIMER_PRESCALE_32   | 10000);
#ifndef FULL_POWER_MODE
    NVIC_EnableIRQ(TIMER1_IRQn);
#endif
}

/* ----------------------------------------------------------------------------
 * Function      : int main()
 * ----------------------------------------------------------------------------
 * Description   : Main application function
 * ------------------------------------------------------------------------- */
int main()
{
    System_Initialize();
    App_Env_Initialize();
    /* Start a timer to be used as a periodic tick timer for application */
    Kernel_Schedule();
    ke_timer_set(APP_TIMER, TASK_APP, TIMER_1S_SETTING);

    /* Main application loop:
     * - Run the kernel scheduler
     * - Perform some application stuff
     * - Refresh the watchdog and wait for an interrupt before continuing */
    Sys_Timers_Start(SELECT_TIMER0);
#ifndef FULL_POWER_MODE
#ifdef ONE_SHOT_MODE
    NCT375_ONEShot_ModeOn();
#else
    NCT375_PowerDown();
#endif
    //NCT375_ONEShot_ModeOff();
    NCT375_I2C_Delay();
#endif
    while (1)
    {
        Kernel_Schedule();

#ifdef FULL_POWER_MODE
        /* Update temperature in a regular interval from temperatire sensor */
        if (app_env.update_ble_data)
        {
        	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 1, app_env.i2c_rx_buffer, 2, NCT375_Received_Temperature);
        	app_env.update_ble_data = false;
        }
#endif
        /* Refresh the watchdog timer */
        Sys_Watchdog_Refresh();

        /* Wait for an event before executing the scheduler again */
        SYS_WAIT_FOR_EVENT;
    }
}

void TIMER0_IRQHandler(void)
{
	//NCT375_I2C_Delay();
	//NCT375_THYST_Write((short int) 27); // Test
	//NCT375_I2C_Delay();
	//NCT375_TOS_Write((short int) -30);	// Test
	//NCT375_I2C_Delay();
#ifndef FULL_POWER_MODE
#ifndef ONE_SHOT_MODE
	NCT375_ConfReg_Read();
	NCT375_I2C_Delay();
#endif
	if(ble_env.state==APPM_CONNECTED)
	{
#ifdef ONE_SHOT_MODE
		NCT375_ONEShot_StartSample();
#else
		if(nct375.Config & 0x01)
		{
			NCT375_PowerUp();
			NCT375_I2C_Delay();
			// Update state
			NCT375_ConfReg_Read();
			NCT375_I2C_Delay();
		}
#endif
		Sys_Timers_Start(SELECT_TIMER1);
	}
	//Sys_Timers_Start(SELECT_TIMER1);	// Test
#ifndef ONE_SHOT_MODE
	else
	{
		if(!(nct375.Config & 0x01))
		{
			NCT375_PowerDown();
			NCT375_I2C_Delay();
			// Update state
			NCT375_ConfReg_Read();
			NCT375_I2C_Delay();
		}

	}
#endif
#endif
}

// Temperature value register reading
void TIMER1_IRQHandler(void)
{
#ifndef FULL_POWER_MODE
	app_env.i2c_tx_buffer[0]=0x00;	// Address pointer register
	I2C_WriteRead(0x48, app_env.i2c_tx_buffer, 1, NULL, 0, NULL);
	NCT375_I2C_Delay();
	I2C_WriteRead(0x48, NCT375_CMD_GET_TEMPERATURE, 0, app_env.i2c_rx_buffer, 2, NCT375_Received_Temperature);
	/*
	nct375.Thyst=NCT375_THYST_Read();	// Test
	if(nct375.Thyst == (short int) 27) 	{	}	// Test
	*/
	/*
	nct375.TOs=NCT375_TOS_Read();	// Test
	if(nct375.Thyst == (short int) -30) {	}	// Test
	*/
#endif
}

