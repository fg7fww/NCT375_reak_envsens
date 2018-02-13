# NCT375_reak_envsens
Enviromental BLE temperature sensor

2018.02.12
There is nct375.h header file inside the NCT375_reak_envsens project. It contains two macro definition:

/* Selection between power save ONE-SHOT-MODE (temperature register values are updated only in the user specified time)
 * and NORMAL-MODE during the BLE connection time (every 80ms temperature register value is updated automatically by the new value).
 * In the case normal mode selection ONE_SHOT_MODE definition has to be comment. Out of BLE connection, chip is in Shutdown Mode.
 * All circuitry except interface are powered down. */
#define ONE_SHOT_MODE

/* When the FULL_POWER_MODE definition is uncommented then the nct375 chip is in the normal mode out of BLE connection time too.
 * It is the chip maximal power consumption mode */
#define FULL_POWER_MODE


1)	Full Power Mode setting (maximal power consumption, normal power mode full time)
 
#define ONE_SHOT_MODE
..
..
#define FULL_POWER_MODE
 
OR

//#define ONE_SHOT_MODE
..
..
#define FULL_POWER_MODE

2)	Normal Power Mode setting ( only during BLE connection, out of connection there is shut down mode)

//#define ONE_SHOT_MODE
..
..
//#define FULL_POWER_MODE

3)	One Shot-Mode ( all time is shutting down only after sampling start event given by user, chip is powered up for taking sample, then it is shutting down again)

#define ONE_SHOT_MODE
..
..
//#define FULL_POWER_MODE


