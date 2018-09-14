/*******************************************************************************
** File: rw_app.h
*******************************************************************************/

#ifndef _rw_app_h_
#define _rw_app_h_

/*
** Required header files.
*/
#include "cfe.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/***********************************************************************/

#define RW_MAJOR_VERSION             1
#define RW_MINOR_VERSION             0
#define RW_REVISION                  0
#define RW_MISSION_REV               0

#define RW_RESERVED_EID              0
#define RW_STARTUP_INF_EID           1
#define RW_COMMAND_ERR_EID           2
#define RW_COMMANDNOP_INF_EID        3
#define RW_COMMANDRST_INF_EID        4
#define RW_INVALID_MSGID_ERR_EID     5
#define RW_LEN_ERR_EID               6
#define RW_TASK_EXIT_EID             7

#define RW_PIPE_DEPTH			    32

/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/
void RWS_SetTach(float wheel_speed_1, float wheel_speed_2, float wheel_speed_3);

void RWS_GetTach(float *wheel_speed_1, float *wheel_speed_2, float *wheel_speed_3);
/*
** Local function prototypes.
**
** Note: Except for the entry point (SAMPLE_AppMain), these
**       functions are not called from any other source module.
*/
void RW_AppMain(void);
int32 RW_AppInit(void);
void RW_ProcessCommandPacket(void);
void RW_ProcessGroundCommand(void);
void RW_UpdateTach(void);
void RW_TorqueCommand(void);
void RW_CurrentCommand(void);
void RW_ReportHousekeeping(void);
void RW_ResetCounters(void);

boolean RW_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

#endif /* _rw_app_h_ */
