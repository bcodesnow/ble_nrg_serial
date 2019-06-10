#ifndef BLE_UART_H
#define BLE_UART_H


/* Simple Protocol: */

/*1st byte:
    CMD BYTE
**/
#define TRIGGERED                       0x06
#define GET_STATE                       0x03
#define STOP				  			0x02
#define START				  			0x01
#define IGNORE_LAST_X                   0x07
#define DATA_COLLECTED                  0x08
#define WRITE_CATCH_SUCCESS             0x09


#define REQUEST_SENSORDATA              0x0A // available when data is in ram
#define SENDING_SENSORDATA_FINISHED     0x0B // emitted when all the files were sent
#define SENSORDATA_AVAILABLE            0x0C // emitted wehn data can be requested.

/* REQUEST_SENSOR_DATA  -> byte[1] 0xFF */
/* SENDING_SENSORDATA_FINISHED   -> byte[1] 0xFF */
/* SENSORDATA_AVAILABLE					 -> byte[1] 0xFF */

#define ALIVE                           0x10

#define HUGE_CHUNK_START                0x77
#define	HUGE_CHUNK_FINISH               0x88
#define SWITCH_RECEIVE_MODE             0x99
#define TURN_ON_SD_LOGGING              0x66

#define TIME_SYNC_START                 0xA1
#define TIME_SYNC_FINISHED              0xA2

#define TIME_SYNC_FROM_SERVER           0xAA
/* TIME_SYNC_FROM_SERVER                    -> byte[1] timeStampOnServer byte[0]
                                            -> byte[2] timeStampOnServer byte[1]
                                            -> byte[3] timeStampOnServer byte[2]
                                            -> byte[4] timeStampOnServer byte[3]
*/
#define TIME_SYNC_FROM_CLIENT           0xBB
/* TIME_SYNC_FROM_CLIENT                    -> byte[1] timeStampOnClientWhenSrvMsgReceived byte[0]
                                            -> byte[2] timeStampOnClientWhenSrvMsgReceived byte[1]
                                            -> byte[3] timeStampOnClientWhenSrvMsgReceived byte[2]
                                            -> byte[4] timeStampOnClientWhenSrvMsgReceived byte[3]
*/


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
#define TYPE_AUD 1u
#define	TYPE_ACC 2u
#define TYPE_GYR 3u
#define TYPE_MAG 4u
#define TYPE_PRS 5u
#define TYPE_LOG 0xAA


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

#endif // BLE_UART_H