/*******************************************************************************
** File: shk_app.h
**
** Purpose:
**   This file is main hdr file for the SHK application.
**
**
*******************************************************************************/

#ifndef _shk_app_h_
#define _shk_app_h_

/*
** Required header files.
*/
#include "cfe.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/***********************************************************************/

#define SHK_APP_MAJOR_VERSION         1
#define SHK_APP_MINOR_VERSION         0
#define SHK_APP_REVISION              0
#define SHK_APP_MISSION_REV           0

#define SHK_RESERVED_EID              0
#define SHK_STARTUP_INF_EID           1
#define SHK_COMMAND_ERR_EID           2
#define SHK_COMMANDNOP_INF_EID        3
#define SHK_COMMANDRST_INF_EID        4
#define SHK_INVALID_MSGID_ERR_EID     5
#define SHK_LEN_ERR_EID               6
#define SHK_TASK_EXIT_EID             7
#define SHK_CAMERA_INF_EID            8
#define SHK_DEPLOY_START_INF_EID      9
#define SHK_DEPLOY_PRIMARY_INF_EID   10 
#define SHK_DEPLOY_SECONDARY_INF_EID 11 
#define SHK_MODE_CMD_INF_EID         12
#define SHK_MODE_CMD_ERR_EID         13

#define SHK_PIPE_DEPTH               16
#define SHK_STARTUP_SYNC_TIMEOUT     8000
#define SHK_DEPLOY_FILENAME          "/boot/deploy.me"

#define SHK_SC_START_RTS_CC          4
#define SHK_SC_ENABLE_RTS_CC         7

#define SHK_DEPLOY_RTS               3


#define SHK_IMAGE_TRANSFER_SB_DELAY  100


/************************************************************************
** Type Definitions
*************************************************************************/

typedef struct 
{
   uint32 ImageCaptureInProgress;
   uint32 ImageSize;
   int32  ImageFileFd;
   int32  ImageOffset;
   int32  ImageFileSize;
   uint8  ImagePath[OS_MAX_PATH_LEN];
} SHK_CameraData_t;


/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SHK_AppMain), these
**       functions are not called from any other source module.
*/
void  SHK_AppMain(void);
int32 SHK_AppInit(void);
void  SHK_ProcessCommandPacket(void);
void  SHK_ProcessGroundCommand(void);
void  SHK_ReportHousekeeping(void);
void  SHK_ResetCounters(void);
void  SHK_CollectScience(void);
int32 SHK_CameraCaptureImage(uint32 ImageSize);
void  SHK_CameraTransferData(void);
int32 SHK_CheckForDeployment(void);
void  SHK_ProcessDeployment(void);
void  SHK_ActivateDeploymentRTS(void);


boolean SHK_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

#endif /* _shk_app_h_ */
