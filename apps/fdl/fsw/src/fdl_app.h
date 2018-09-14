/*******************************************************************************
** File:
**   $Id: fdl_app.h $
**
** Purpose:
**   This file is main hdr file for the FDL application.
**
**
*******************************************************************************/

#ifndef _fdl_app_h_
#define _fdl_app_h_

/*
** Required header files.
*/
#include "cfe.h"

#include "fdl_perfids.h"
#include "fdl_msgids.h"
#include "fdl_msg.h"
#include "fdl_events.h"
#include "fdl_version.h"

#include <string.h>

/***********************************************************************/

#define FDL_PIPE_DEPTH            16

/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (FDL_AppMain), these
**       functions are not called from any other source module.
*/
void FDL_AppMain(void);
int32 FDL_AppInit(void);
void FDL_ProcessCommandPacket(CFE_SB_MsgPtr_t MsgPtr);
void FDL_ProcessGroundCommand(CFE_SB_MsgPtr_t MsgPtr);
void FDL_ReportHousekeeping(void);
void FDL_ResetCounters(void);
void FDL_StartFileTransfer(FDL_FileStartCmd_t *MsgPtr);
void FDL_SendDataFileDownlinkTlmPackets(void);
void FDL_CancelFileTransfer(void);
void FDL_PauseFileTransfer(void);
void FDL_ResumeFileTransfer(void);
boolean FDL_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

#endif /* _fdl_app_h_ */

/************************/
/*  End of File Comment */
/************************/
