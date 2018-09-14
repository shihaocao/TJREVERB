/*******************************************************************************
** File: 
**   $Id: ful_app.h  $
**
** Purpose:
**   This file is main hdr file for the FUL application.
**
**
*******************************************************************************/

#ifndef _ful_app_h_
#define _ful_app_h_

/*
** Required header files.
*/
#include "cfe.h"

#include "ful_perfids.h"
#include "ful_msgids.h"
#include "ful_msg.h"
#include "ful_events.h"
#include "ful_version.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/***********************************************************************/

#define FUL_PIPE_DEPTH            16 


/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (FUL_AppMain), these
**       functions are not called from any other source module.
*/
void FUL_AppMain(void);
int32 FUL_AppInit(void);
void FUL_ProcessCommandPacket(CFE_SB_MsgPtr_t MsgPtr);
void FUL_ProcessGroundCommand(CFE_SB_MsgPtr_t MsgPtr);
void FUL_ReportHousekeeping(void);
void FUL_ResetCounters(void);
void FUL_StartFileTransfer(FUL_FileStartCmd_t *MsgPtr);
void FUL_CancelFileTransfer(void);
void FUL_FinishFileTransfer(FUL_FileFinishCmd_t *MsgPtr);
void FUL_ProcessFileSegment(FUL_FileSegmentCmd_t *MsgPtr);
boolean FUL_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

#endif /* _ful_app_h_ */

/************************/
/*  End of File Comment */
/************************/
