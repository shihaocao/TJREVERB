/*******************************************************************************
** File: acs_app.h
**
** Purpose:
**   This file is main hdr file for the TLC application.
**
**
*******************************************************************************/

#ifndef _acs_app_h_
#define _acs_app_h_


/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/***********************************************************************/

#define ACS_PIPE_DEPTH                    16 

/************************************************************************
** Type Definitions
*************************************************************************/

typedef struct 
{
    uint32              SunPt_FirstCycle;
    uint32              Science_FirstCycle;
    uint32              AKF_FirstCycle;
} acs_state_t;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (ACS_AppMain), these
**       functions are not called from any other source module.
*/
void ACS_AppMain(void);
void ACS_AppInit(void);
void ACS_ProcessCommandPacket(CFE_SB_MsgPtr_t MessagePtr);
void ACS_ProcessGroundCommand(CFE_SB_MsgPtr_t MessagePtr);
void ACS_ReportDiagnostic(CFE_SB_MsgPtr_t MessagePtr);
void ACS_ReportHousekeeping(void);
void ACS_ReportSptTlmPkt(void);
void ACS_ReportSciTlmPkt(void);
void ACS_ReportSdpTlmPkt(void);
void ACS_ReportAdTlmPkt(void);
void ACS_ReportMpuTlmPkt(void);
void ACS_ResetCounters(void);
void ACS_Run(CFE_SB_MsgPtr_t MessagePtr);
void ACS_Mode(CFE_SB_MsgPtr_t MessagePtr);
void ACS_SciGain(CFE_SB_MsgPtr_t MessagePtr);
void ACS_SunGain(CFE_SB_MsgPtr_t MessagePtr);
void ACS_RateTarg(CFE_SB_MsgPtr_t MessagePtr);
void ACS_ProcessMagBoomPkt(CFE_SB_MsgPtr_t MessagePtr);
boolean ACS_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);
void ACS_FlagCheck(CFE_SB_MsgPtr_t MessagePtr);
void ACS_ProcessDagrBoomPkt(CFE_SB_MsgPtr_t MessagePtr);

#endif /* _acs_app_h_ */
