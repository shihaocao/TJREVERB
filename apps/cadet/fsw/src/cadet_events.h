/*
** File: cadet_events.h
**
** Purpose:
**   Define CADET App event message numbers.
**
*/
#ifndef _cadet_events_h_
#define _cadet_events_h_



/*
** Defines
*/
#define CADET_STARTUP_INF_EID                       1 
#define CADET_COMMAND_ERR_EID                       2
#define CADET_COMMANDNOP_INF_EID                    3 
#define CADET_COMMANDRST_DBG_EID                    4
#define CADET_LEN_ERR_EID                           5 
#define CADET_CMDPIPE_ERR_EID                       8
#define CADET_TLMPIPE_ERR_EID                       9
#define CADET_SUBSCRIBE_ERR_EID                     10
#define CADET_FNCODE_ERR_EID                        11
#define CADET_CREATE_SEM_ERR_EID                    18
#define CADET_GIVESEMAPHORE_INF_EID                 19
#define CADET_GIVESEMAPHORE_ERR_EID                 20
#define CADET_TAKESEMAPHORE_INF_EID                 21
#define CADET_TAKESEMAPHORE_ERR_EID                 22
#define CADET_FILTER_TABLE_DBG_EID                  23
#define CADET_FILTER_TBL_ERR_EID                    24
#define CADET_JAM_FILTER_CMD_ERR_EID                25
#define CADET_JAM_FILTER_CMD_DBG_EID                26
#define CADET_CRITICAL_DATA_STORE_ERR_EID           27
#define CADET_UNABLE_TO_OPEN_SERIAL_PORT_ERR_EID    28
#define CADET_UNABLE_TO_SET_SERIAL_CALLBACK_ERR_EID 29
#define CADET_RECEIVED_ILLEGAL_MSGID_ERR_EID        30
#define CADET_ABORTPLAYBACK_INF_EID                 31
#define CADET_PLAYBACK_IN_PROGRESS_ERR_EID          32
#define CADET_PLAYBACK_PARAMETER_ERR_EID            33
#define CADET_FLUSH_IN_PROGRESS_ERR_EID             34 
#define CADET_FLUSH_PARAMETER_ERR_EID               35
#define CADET_CREATE_MUTEX_ERR_EID                  36
#define CADET_RESETCADETUART_INF_EID                37
#define CADET_RESETCADET_INF_EID                    38
#define CADET_EXIT_ERR_EID                          39



#endif /* _cadet_events_h_ */



/************************/
/*  End of File Comment */
/************************/
