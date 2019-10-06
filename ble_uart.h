#ifndef __ble_uart_h
#define __ble_uart_h

#include "stdint.h"
/*
uint16_t cheatsheat
send
	[0] = ( bytesToWrite >> 8 ) & 0xFF;	
	[1] = bytesToWrite & 0xFF;	
receive
	u16 = ( ( (uint16_t) [0] ) << 8 ) & 0xFF;	
	u16 |= [1];		
*/
#define USED_ON_MICRO 0

/* ------------------------------------------------------------------- */

#if USED_ON_MICRO == 1
	#include "bluenrg_types.h"
	#include "bluenrg_gatt_server.h"
	#include "bluenrg_gap.h"
	#include "string.h"
	#include "bluenrg_gap_aci.h"
	#include "bluenrg_gatt_aci.h"
	#include "hci_const.h"
	#include "bluenrg_hal_aci.h"
	#include "bluenrg_aci_const.h"   
	#include "hci.h"
	#include "hci_le.h"
	#include "sm.h"
	#include <stdlib.h>

//	/* For enabling the capability to handle BlueNRG Congestion */
//	//#define ACC_BLUENRG_CONGESTION
//	#ifdef ACC_BLUENRG_CONGESTION
//	/* For defining how many events skip when there is a congestion */
//	#define ACC_BLUENRG_CONGESTION_SKIP 30
//	#endif /* ACC_BLUENRG_CONGESTION */

	/* Stack Init Modes:*/
	#define DISCOVERY_MODE 		1u<<1u
	#define	ADV_MODE			 		1u<<2u
	#define BLE_UART_MODE 		1u<<3u
	#define USE_TX_POOL				1u<<4u
//	#define USE_SAFETY_DELAY 	1
//	#define MAX_BLE_UART_TRIES 255
	#define BLE_UART_TX_TIMEOUT	1000
	
	#define INDEX_SIZE	1u
	#define HC_USABLE_PAYLOAD ( CHAR_MAX_PAYLOAD - INDEX_SIZE )
	#define POOLED_HUGE_CHUNK_TIMEOUT		10000

#endif // USED ON MICRO

/* ------------------------------------------------------------------- */

#define CHAR_MAX_PAYLOAD 20

/* Simple Protocol: */

/*1st byte:
    CMD BYTE
**/
#define TRIGGERED                       0x06
#define GET_STATE                       0x03
#define STOP				  									0x02
#define START				  									0x01
#define IGNORE_LAST_X                   0x07 // not implemented yet
#define DATA_COLLECTED                  0x08 // used when data is saved to sd..
#define WRITE_CATCH_SUCCESS             0x09


#define REQUEST_SENSORDATA              0x0A // available when data is in ram
#define SENDING_SENSORDATA_FINISHED     0x0B // emitted when all the files were sent
#define SENSORDATA_AVAILABLE            0x0C // emitted wehn data can be requested.
#define SET_SHUT_UP                     0x33 // when set on device it should not send any notifications
/* REQUEST_SENSOR_DATA  -> byte[1] 0xFF */
/* SENDING_SENSORDATA_FINISHED   -> byte[1] 0xFF */
/* SENSORDATA_AVAILABLE					 -> byte[1] 0xFF */


#define SET_CONN_PERIOD									0xBB // msg to receive conn period
/*SET_CONN_PERIOD -> byte[1] min interv.

                                            -> byte[2] max interv.
                                            -> byte[3] & byte[4] timeout
*/


#define CONN_PARAM_INFO	0x12
typedef struct conn_param_info_s {
    uint8_t requested_mode;
    uint16_t interval;
    uint16_t supervision;
} __attribute__((packed)) conn_param_info_t;
//uint16_t interval
//uint16_t supervision

#define SLOW 1
#define MID  2
#define FAST 3

#define S_MIN   100
#define S_MAX   200
#define S_LAT   5
#define S_SUP   1000

#define M_MIN   50
#define M_MAX   85
#define M_LAT   0
#define M_SUP   500

#define F_MIN   7.5
#define F_MAX   7.5
#define F_LAT   0
#define F_SUP   100


#define HUGE_CHUNK_ACK_PROC							0x0D // HC FINISHED / REQUEST MISSED
#define HC1_BEGIN                                               HC_1_FIN
#define HC_1_FIN												11u
#define	HC_1_REQ												22u
#define HC_1_ACK												33u

/*HUGE_CHUNK_ACK_PROC -> byte[1]
                                                                 finished requesting approval to continue - 11
                                                                 request missed package 22
                                                                 hc was ok, continue 33

                                            -> byte[2] package Nr Hbyte
                                            -> byte[3] package Nr Lbyte
                                            -> byte[4]
                                            -> byte[5]
                                            -> byte[6]
*/

#define HUGE_CHUNK_MISSED_PACKAGE				0x0F// MSG Type for missed packages exchanged during huge chunk ac proc
/*HUGE_CHUNK_MISSED_PACKAGE -> byte[1] - byte[19] payload
*/

#define DIAG_INFO												0x0E
#define DIAG_1_TYPE_HC_STAT							1u
#define DIAG_1_TYPE_CONN_PARAM					2u
#define DIAG_1_TYPE_LENGTH_TEST					3u

#define DIAG_2_ARG1											2u
#define DIAG_3_ARG2											3u
#define DIAG_4_ARG3											4u
#define DIAG_5_ARG4											5u

#define DIAG_2_CP_LAT										2u
#define DIAG_3_CP_INT										3u
#define DIAG_4_CP_SUP										4u
#define DIAG_4_CP_STA										5u

/*DIAG_INFO -> byte[1] type
											-> byte[2] arg1 
											-> byte[3] arg2
											-> byte[4] ...
											-> byte[5] ...
											-> byte[6] ...
*/

#define ALIVE                           0x10

#define HUGE_CHUNK_START                0x77
#define	HUGE_CHUNK_FINISH               0x88
#define SWITCH_RECEIVE_MODE             0x99
#define TURN_ON_SD_LOGGING              0x66
#define TURN_ON_BLE_SENDING             0x55

#define TS_MSG                          0xAA

/* ------------------------------------------------------------------- */

/* TS_MSG                               -> byte[1] CMD
 * TIME SYNC MSG FROM SERVER OR CLIENT  -> byte[2] timeStampOnServer / knownProcessingTime
                                        -> byte[3] timeStampOnServer / knownProcessingTime
                                        -> byte[4] timeStampOnServer / knownProcessingTime
                                        -> byte[5] timeStampOnServer / knownProcessingTime
*/

#define TS_CMD_TIMESTAMP_IN_PAYLOAD             1u
#define TS_CMD_LAST_ONE_WAS_GOOD_ONE            2u
#define TS_CMD_ACK                              3u
#define TS_CMD_KNOWN_PROCESSING_TIME_IN_PAYLOAD 4u
#define TS_CMD_SYNC_START                       5u
#define TS_CMD_SYNC_FINISH                      6u


/* REQUEST_SENSOR_DATA  -> byte[1] 0xFF */

/* IGNORE_LAST_X 		-> byte[1] how many to ignore */


/* ALIVE 							-> byte[1] mainState
                                            -> byte[2] subState
                                            -> byte[3] fileIndex
                                            -> byte[4] lastError
*/

/* HUGE_CHUNK_START		-> byte[1] bytesToSend HBYTE
                                            -> byte[2] bytesToSend LBYTE
                                            -> byte[3] TYPE
                                            -> byte[4] Used Charactertics as Channels (count)
*/
// THIS BLOCK HAS NOTHING TO DO HERE
#define TYPE_AUD 1u
#define	TYPE_ACC 2u
#define TYPE_GYR 3u
#define TYPE_MAG 4u
#define TYPE_PRS 5u
#define TYPE_LOG 0xAA

#define TYPE_COUNT 5 // different sensor types

#define FREQ_AUD 8000
#define FREQ_ACC 1000
#define FREQ_GYR 1000
#define FREQ_MAG 100
#define FREQ_PRS 100
// THIS BLOCK HAS NOTHING TO DO HERE

/* HUGE_CHUNK_FINISH	-> byte[1] maxRepeatCount */

/* SWITCH_RECEIVE_MODE		-> byte[1] 0x55
                            -> byte[2] 0xFF
                            -> byte[3] 0x55
                            -> byte[4] 0xFF
                            -> byte[5] 0x55
                            -> byte[6] MODE
*/

#define SW_REC_MODE_LENGTH 7
#define MODE_CMD 1u
#define MODE_HUGE_CHUNK 2u


/* TURN_ON_SD_LOGGING	-> byte[1] ON / OFF */


/* ------------------------------------------------------------------- */

#if USED_ON_MICRO==1
	#define BLE_POLL_DELAY 8
	
	#define COPY_RECEIVED_TO_BUF (1u<<3u)
	
	typedef struct t_ble_uart_struct
	{
		uint8_t state;
		uint32_t len;
		uint8_t rx_data[20];
		uint8_t tx_data[20];
		uint32_t Timer1;
		uint32_t Timer2;
		uint32_t Timer3;
		uint32_t mode_reg;
		uint8_t bnrg_sta_tx_pool_available;
	} t_ble_uart_struct;
	
	typedef struct
{
	uint8_t type;
	uint16_t req_pkg;
	uint8_t state;
}	hc_appr_proc_t;

//	extern hc_appr_proc_t hc_appr_proc;

	extern void ble_uart_timerproc (void);
	extern void init_blue_nrg_ms_stack(uint8_t mode_select);

	extern void HCI_Event_CB(void *pckt);

	extern uint8_t connected;
	
	extern tBleStatus ble_uart_tx (uint8_t *data, uint16_t len);
	extern tBleStatus ble_uart_tx_pool (uint8_t *data, uint16_t len);
	
	extern void hci_reinit(void);
//	extern void ble_poll( void );
	
	extern uint8_t ble_uart_tx_huge_chunk ( uint8_t* buff,	uint16_t bytesToWrite );
	extern uint8_t ble_uart_tx_huge_chunk_pool ( uint8_t* buff,	uint16_t bytesToWrite);
	extern uint8_t ble_uart_tx_huge_chunk_start ( uint8_t type, uint16_t bytesToWrite, uint16_t arg2, uint8_t channel_count );	
	extern uint8_t ble_uart_tx_huge_chunk_finish ( uint8_t type );

	extern uint8_t send_missing_package_hc_appr_proc (uint8_t* ptrToLastBuf, uint16_t packageCnt);

	extern uint8_t finish_hc_appr_proc(void);
	extern uint8_t start_hc_appr_proc(uint8_t* ptr_to_last_buffer);
	extern uint8_t send_diag_info (uint8_t type, uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t arg4);
#endif
	
/* ------------------------------------------------------------------- */

#endif
