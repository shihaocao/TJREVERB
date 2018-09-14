 /*
** File: cadet_output.h
**
** Purpose:
**   Define CADET radio output messages, exported variables, and interface routines.
**
*/
#ifndef _cadet_output_h_
#define _cadet_output_h_



/*
**   Includes:
*/
#include "cfe.h"
#include "cadet_radio.h"



/*
** Defines
*/
#define CADET_RADIO_PIPE_DEPTH                  ( 32 )

#define REQUEST_VERSION_COMMAND_RATE            ( 4*120 ) /* Every 4 minutes at 2 Hz */
#define REQUEST_VERSION_COMMAND_OFFSET          ( 0 )     /* 0 seconds into the minute at 2 Hz */
#define REQUEST_CONFIG_COMMAND_RATE             ( 1*120 ) /* Every minute at 2 Hz */
#define REQUEST_CONFIG_COMMAND_OFFSET           ( 60 )    /* 30 seconds into the minute at 2 Hz */

#if REQUEST_VERSION_COMMAND_OFFSET == REQUEST_CONFIG_COMMAND_OFFSET
    #error The version and config command offset cannot be the same
#endif

#define DELAY_TO_AUTOSTART_PLAYBACK             ( 20 + 1 ) /* 10 seconds + a spare */
#define CYCLE_BETWEEN_LO_HI_PLAYBACKS_REQUESTS  ( 10 ) /* During the hi/lo fifo playback make every 10th request a hi fifo */

#define CADET_AUTO_FLUSH_MUTEX_NAME     "CADET_FLUSH_MUTEX"
#define CADET_AUTO_PLAYBACK_MUTEX_NAME  "CADET_PBACK_MUTEX"

/* Command codes for commands received by cadet application but just relayed to radio for execution (why not start from 0) */
#define CADET_GET_VERSION_CC              9       /* Get and report Radio Version */ 
#define CADET_GET_CONFIG_CC               10      /* Get and report Radio configuraton */
#define CADET_GET_STATUS_CC               11      /* Get and report Radio Status */
#define CADET_SET_TIME_CC                 12      /* Set the Cadet Radio Time */
#define CADET_RESET_RADIO_CC              13      /* Reset the Cadet Radio */
#define CADET_FIFO_REQUEST_CC             14      /* Send a FIFO Downlink Request to the Radio */
#define CADET_FIFO_CLEAR_CC               15      /* Clear FIFO Command */
#define CADET_AUTH_ENABLE_CC              16      /* Enable Authentication */
#define CADET_AUTH_DISABLE_CC             17      /* Disable Authentication */
#define CADET_FIFO_REQUEST_EXT_CC         18      /* Send a FIFO Downlink Request EXT to the Radio */



/*
** Structures
*/
/* CADET_GET_VERSION_CC will use a no-args command */

/* CADET_GET_CONFIG_CC will use a no-args command */

/* CADET_GET_STATUS_CC will use a no-args command */

/* This is the command format for the CADET_SET_TIME command */
typedef struct
{
    uint8                        CmdHeader[CFE_SB_CMD_HDR_SIZE];
    CadetSetTimePayload_t        Date;

}  OS_PACK  CADET_RadioSetTimeCmd_t;

/* This is the command format for the CADET_RESET_RADIO command */
typedef struct
{
    uint8                        CmdHeader[CFE_SB_CMD_HDR_SIZE];
    CadetResetPayload_t          Reset;

}  OS_PACK  CADET_RadioResetCmd_t;

/* This is the command format for the CADET_FIFO_REQUEST command */
typedef struct
{
    uint8                        CmdHeader[CFE_SB_CMD_HDR_SIZE];
    CadetFifoRequestPayload_t    Request;

}  OS_PACK  CADET_RadioFifoRequestCmd_t;

/* This is the command format for the CADET_FIFO_CLEAR command */
typedef struct
{
    uint8                        CmdHeader[CFE_SB_CMD_HDR_SIZE];
    CadetFifoClearPayload_t      Clear;

}  OS_PACK  CADET_RadioFifoClearCmd_t;

/* This is the command format for the CADET_AUTH_ENABLE command */
typedef struct
{
    uint8                        CmdHeader[CFE_SB_CMD_HDR_SIZE];
    CadetAuthEnablePayload_t     KeySlot;

}  OS_PACK  CADET_RadioAuthenticationEnableCmd_t;

/* CADET_AUTH_DISABLE_CC will use a no-args command */

/* This is the command format for the CADET_FIFO_REQUEST_EXT command */
typedef struct
{
    uint8                        CmdHeader[CFE_SB_CMD_HDR_SIZE];
    CadetFifoRequestExtPayload_t RequestExt;

}  OS_PACK  CADET_RadioFifoRequestExtCmd_t;



/*
** Exported Variables
*/
extern uint32                   CADET_PlaybackWaitingForResponse;
extern uint32                   CADET_PlaybackTimeout;
extern uint32                   CADET_AutoFlushMutex;
extern uint32                   CADET_AutoPlaybackMutex;



/*
** Exported Functions
*/
extern void CADET_Output_TransmitDataToRadio ( void );
extern int32 CADET_Output_Initialize ( void );



#endif /* _cadet_output_h_ */



/************************/
/*  End of File Comment */
/************************/
