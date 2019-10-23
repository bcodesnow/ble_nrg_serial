#ifndef __catch_det_
#define __catch_det_

#include "stdint.h"

/* CATCH DETECTION */
#define CD_TOTAL_BUFFERED_DATA_IN_MS						2500
#define CD_PRE_TRIGGER_BUFFERED_DATA_IN_MS			1000

#define CD_MAGNETO_THRESHOLD_IN_uT							900.0f		// Threshold in uT					- set to 0.0 to deactivate it (hint: at least one must be active)

#define CD_ACC_THRESHOLD_IN_G										14.0f			// Threshold in 'value'*g		- set to 0.0 to deactivate it (hint: at least one must be active)

#define CD_MAGNETO_THRESHOLD										(CD_MAGNETO_THRESHOLD_IN_uT*10.0f/LSM303AGR_MAG_SENSITIVITY)	// Magneto threshold calculation
#define CD_ACC_THRESHOLD												(CD_ACC_THRESHOLD_IN_G*1000.0f/LSM6DSM_ACC_SENSITIVITY)					// Acceleration threshold calculation


#define CD_POST_TRIGGER_BUFFERED_DATA_IN_MS			(CD_TOTAL_BUFFERED_DATA_IN_MS - CD_PRE_TRIGGER_BUFFERED_DATA_IN_MS)

/* BLE */
#define ALIVE_MSG_PERIOD 1250
#define MAX_MS_IN_STATE	3500

/* BUFFERING */
// Buff Param for Magneto & Pressure
#define R_BUFF_SIZE_1KHZ CD_TOTAL_BUFFERED_DATA_IN_MS

// Buff Param for Accelero & Gyro
#define R_BUFF_SIZE_100HZ (CD_TOTAL_BUFFERED_DATA_IN_MS / 10)

// Buff Param for Audio
#define AUDIO_SAMPLING_FREQUENCY	8000

#define AUDIO_PCM_SAMPLES_PRO_MS              (AUDIO_SAMPLING_FREQUENCY/1000)
#define AUDIO_PCM_BYTES_PRO_MS                (AUDIO_PCM_SAMPLES_PRO_MS*2)
#define R_BUFF_SIZE_AUDIO			(AUDIO_PCM_SAMPLES_PRO_MS * CD_TOTAL_BUFFERED_DATA_IN_MS)


/* STATE MACHINE STUFF */
typedef struct
{
    uint16_t mainState;
    uint16_t lastMainState;

    uint8_t subState;

    uint8_t bleUplState;
    uint8_t lastBleUplState;

    uint8_t lastError;

    uint8_t isSDenabled;
    uint8_t isSendingOverBLEenabled;

    uint8_t notifyTriggered;
    uint8_t notifyHoldingData;

    uint8_t currentFileIndex;
    uint8_t* ptrToLastBuf;
}	catch_detection_state_machine_t;

/* Trigger Source */
#define MAGNETO_TRIG 		 1<<1
#define ACC_TRIG		 1<<2

/* Main States */
#define CDSM_STATE_FRESH_BOOT                                               ( 0u )
#define CDSM_STATE_INIT														( 1u )
#define CDSM_STATE_RUNNING                                                  ( 1u << 2u )
#define CDSM_STATE_READY_TO_BE_TRIGGERED                                    ( 1u << 3u )	/* There is already data for 1 sec in the buffer. */
#define CDSM_STATE_TRIGGERED                                                ( 1u << 4u )
#define CDSM_STATE_POST_TRIGGER_DATA_COLLECTED                              ( 1u << 5u )  /* The needed amount of data after the trigger has been collected, lets save it to the SD card. */
#define CDSM_STATE_STOPPING                                                 ( 1u << 6u )
#define CDSM_STATE_STOPPED                                                  ( 1u << 7u ) /* there are some xtra flags for stopped -> ts, dl, catch confirm */
#define CDSM_STATE_SAVING_DATA_TO_SD                                        ( 1u << 8u )
#define CDSM_STATE_SENDING_DATA_OVER_BLE                                    ( 1u << 9u )
#define CDSM_STATE_RESTARTING                                               ( 1u << 10u )
#define CDSM_STATE_ERROR 													( 1u << 11u )
#define CDSM_STATE_COLLECT_DATA                                             ( CDSM_STATE_RUNNING | CDSM_STATE_READY_TO_BE_TRIGGERED | CDSM_STATE_TRIGGERED )

#define STA_BLE_UPL_AUD  				0
#define STA_BLE_UPL_MAG  				1
#define STA_BLE_UPL_PRS  				2
#define STA_BLE_UPL_ACC  				3
#define STA_BLE_UPL_GYR  				4
#define STA_BLE_UPL_DONE 				5
#define STA_BLE_WAIT_AFTER_TEST 6
#define STA_BLE_WAIT_HC_ACK 		7

/* SENSORS STUFF */

#define LSM6DSM_ACC_ODR 3330.0f

#define LSM303AGR_MAG_ODR                       100.0f
#define LSM303AGR_MAG_FS 						50 /* FS = 50gauss */
#define LSM303AGR_MAG_SENSITIVITY  1.500f

#define LSM6DSM_ACC_FS 							16	/* FS | 2g, 4g, 8g, 16g  */
#define LSM6DSM_ACC_SENSITIVITY                 LSM6DSM_ACC_SENSITIVITY_FS_16G // lsm6dsm.h

#define LSM6DSM_GYRO_ODR 						1660.0f
#define LSM6DSM_GYRO_FS 						1000							/* FS: 250dps, 125dps, 500dps, 1000dps, 2000dps */
#define LSM6DSM_GYRO_SENSITIVITY                LSM6DSM_GYRO_SENSITIVITY_FS_1000DPS

#define LPS22HB_ODR 								75.0f /* ODR = 50.0Hz */

#define AUDIO_VOLUME_INPUT       64U
#define AUDIO_VOLUME_OUTPUT      50U
#define BSP_AUDIO_IN_IT_PRIORITY 6U


#endif /*__catch_det_*/
