/*******************************************************************************
** File: sec_app.h
*******************************************************************************/

#ifndef _sec_app_h_
#define _sec_app_h_

/*
** Required header files.
*/
#include "cfe.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/***********************************************************************/

#define SEC_MAJOR_VERSION             1
#define SEC_MINOR_VERSION             0
#define SEC_REVISION                  0
#define SEC_MISSION_REV               0

#define SEC_RESERVED_EID              0
#define SEC_STARTUP_INF_EID           1
#define SEC_COMMAND_ERR_EID           2
#define SEC_COMMANDNOP_INF_EID        3
#define SEC_COMMANDRST_INF_EID        4
#define SEC_INVALID_MSGID_ERR_EID     5
#define SEC_LEN_ERR_EID               6
#define SEC_TASK_EXIT_EID             7

#define SEC_PIPE_DEPTH		     32

#define SEC_CHILD_TASK_NAME          "SEC_GPS_TASK"
#define SEC_CHILD_TASK_STACK_SIZE    4096
#define SEC_CHILD_TASK_PRIORITY      200
#define SEC_CHILD_SEM_NAME           "SEC_GPS_SEM"

/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/

/*
** Local function prototypes.
**
** Note: Except for the entry point (SAMPLE_AppMain), these
**       functions are not called from any other source module.
*/
void  SEC_AppMain(void);
int32 SEC_AppInit(void);
void  SEC_ProcessCommandPacket(void);
void  SEC_ProcessGroundCommand(void);
void  SEC_ReportHousekeeping(void);
void  SEC_ResetCounters(void);
void  SEC_ProcessGPS(void);
void  SEC_ConfigCommand(void);
void  SEC_GPSChildTask(void);


boolean SEC_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

#endif /* _sec_app_h_ */
