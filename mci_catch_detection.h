#ifndef MCI_CATCH_DETECTION_H
#define MCI_CATCH_DETECTION_H

#define CDSM_STATE_INIT													( 0u )
#define CDSM_STATE_RUNNING                                              ( 1u << 0u )
#define CDSM_STATE_READY_TO_BE_TRIGGERED                                ( 1u << 1u )	/* There is already data for 1 sec in the buffer. */
#define CDSM_STATE_TRIGGERED                                            ( 1u << 2u )
#define CDSM_STATE_POST_TRIGGER_DATA_COLLECTED                          ( 1u << 3u )  /* The needed amount of data after the trigger has been collected, lets save it to the SD card. */
#define CDSM_STATE_STOPPING                                             ( 1u << 4u )
#define CDSM_STATE_STOPPED                                              ( 1u << 5u )
#define CDSM_STATE_RESTARTING                                           ( 1u << 6u )
#define CDSM_STATE_ERROR 												( 1u << 7u )
#define CDSM_STATE_COLLECT_DATA                                         ( CDSM_STATE_RUNNING | CDSM_STATE_READY_TO_BE_TRIGGERED | CDSM_STATE_TRIGGERED )

#define CDSM_SUBSTATE_STOPPING                                            0u
#define	CDSM_SUBSTATE_SAVING_AUDIO                                        1u
#define CDSM_SUBSTATE_SAVING_MAGNETO                                      2u
#define CDSM_SUBSTATE_SAVING_PRESSURE                                     3u
#define CDSM_SUBSTATE_SAVING_ACC                                          4u
#define CDSM_SUBSTATE_SAVING_GYRO                                         5u
#define CDSM_SUBSTATE_SENDING_DATA_COLLECTED                              6u

#endif // MCI_CATCH_DETECTION_H
