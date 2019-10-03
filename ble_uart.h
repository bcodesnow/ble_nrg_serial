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


#define SET_CONN_PERIOD									0xBB // msg to receive conn period
/*SET_CONN_PERIOD -> byte[1] min interv.

                                            -> byte[2] max interv.
                                            -> byte[3] & byte[4] timeout
*/

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
#define DIAG_2_ARG1											2u
#define DIAG_2_ARG2											3u
#define DIAG_2_ARG3											4u
#define DIAG_2_ARG4											5u

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

#define TS_MSG                          0xAA
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
