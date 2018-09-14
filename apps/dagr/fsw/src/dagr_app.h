/*******************************************************************************
** File: dagr_app.h
**
** Purpose:
**   This file is main hdr file for the DAGR application.
**
**
*******************************************************************************/

#ifndef _dagr_app_h_
#define _dagr_app_h_

/*
** Required header files.
*/
#include "cfe.h"

#include "dagr_msg.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/***********************************************************************/

// TODO: Need to remove test code after fully tested.
// #define DAGR_TEST

#define DAGR_APP_MAJOR_VERSION         1
#define DAGR_APP_MINOR_VERSION         0
#define DAGR_APP_REVISION              0
#define DAGR_APP_MISSION_REV           0

#define DAGR_RESERVED_EID              0
#define DAGR_STARTUP_INF_EID           1
#define DAGR_COMMAND_ERR_EID           2
#define DAGR_COMMANDNOP_INF_EID        3
#define DAGR_COMMANDRST_INF_EID        4
#define DAGR_INVALID_MSGID_ERR_EID     5
#define DAGR_INVALID_TLM_ERR_EID       6
#define DAGR_LEN_ERR_EID               7
#define DAGR_TASK_EXIT_EID             8

#define DAGR_PIPE_DEPTH               16

/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (DAGR_AppMain), these
**       functions are not called from any other source module.
*/
void DAGR_AppMain(void);
int32 DAGR_AppInit(void);
void DAGR_ProcessCommandPacket(void);
void DAGR_ProcessGroundCommand(void);
void DAGR_ReportTelemetry(void);
void DAGR_ReportHousekeeping(void);
void DAGR_ChangeMode(void);
void DAGR_ResetCounters(void);
void DAGR_ExtractAcs(dagr_sci_tlm_t *sci_tlm, dagr_acs_tlm_t *acs_tlm);

#ifdef DAGR_TEST
void DAGR_TEST_PrintTlm(dagr_sci_tlm_t *sci_tlm);
boolean DAGR_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);
#endif /* DAGR_TEST */

#endif /* _dagr_app_h_ */
