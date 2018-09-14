/*
** File: cadet_app.h
**
** Purpose:
**   Define CADET App Main Routine messages, exported variables, and interface routines.
**
*/
#ifndef _cadet_app_h_
#define _cadet_app_h_



/*
**   Includes:
*/
#include "cfe.h"



/*
** Defines
*/
#define CADET_CMD_PIPE_DEPTH            ( 32 )
#define CADET_FIFO_PIPE_DEPTH           ( 64 )
#define CADET_FDL_PIPE_DEPTH            ( 64 )
#define CADET_FDL_HANDSHAKE_SEM_NAME    "CADET_FDL_HANDSHAKE"
#define CADET_FDL_HANDSHAKE_SEM_VALUE   ( 50 )
#define CADET_RADIO_STATUS_TIMEOUT      ( 120 )             /* 60 seconds at 2 Hz */



/*
** Structures
*/



/*
** Exported Variables
*/
extern CFE_SB_PipeId_t          CADET_TlmPipe;  
extern CFE_SB_PipeId_t          CADET_FdlPipe;  
extern uint32                   CADET_FileDownlinkSemaphore;
extern uint16                   CADET_HighFifoPacketsToAutoPlayback_Temp;



/*
** Exported Functions
*/
extern void CADET_AppMain ( void );
extern int32 CADET_AppInit ( void );



#endif /* _cadet_app_h_ */



/************************/
/*  End of File Comment */
/************************/
