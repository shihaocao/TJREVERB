/*******************************************************************************
** File: inms_app.h
**
** Purpose:
**   This file is main hdr file for the INMS application.
**
**
*******************************************************************************/

#ifndef _inms_app_h_
#define _inms_app_h_

/*
** Required header files.
*/
#include "cfe.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "inms_lib.h"
#include "inms_msg.h"

/***********************************************************************/

//#define INMS_TEST

#define INMS_APP_MAJOR_VERSION         1
#define INMS_APP_MINOR_VERSION         0
#define INMS_APP_REVISION              0
#define INMS_APP_MISSION_REV           0

#define INMS_RESERVED_EID              0
#define INMS_STARTUP_INF_EID           1
#define INMS_COMMAND_ERR_EID           2
#define INMS_COMMANDNOP_INF_EID        3
#define INMS_COMMANDRST_INF_EID        4
#define INMS_INVALID_MSGID_ERR_EID     5
#define INMS_LEN_ERR_EID               6
#define INMS_TASK_EXIT_EID             7

#define INMS_PIPE_DEPTH               16
#define INMS_POLL_TIMEOUT             50
#define INMS_TELEMETRY_LENGTH       1651 // TODO: sizeof?

// TODO: Are these commands used?
#define INMS_START_CMD     {0x8A, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66}
#define INMS_PULSAR_ON_CMD {0x8A, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06}
#define INMS_HV_ON_CMD     {0xB1, 0x00, 0x00, 0x00, 0x00, 0x90, 0x2F, 0x90}
#define INMS_MV_ENABLE_CMD {0xB4, 0xA7, 0xA7, 0xA0, 0xA0, 0x00, 0x00, 0xBE} // TODO: Same as INMS_ESA_ON_CMD?
#define INMS_FLOAT_ON_CMD  {0xB1, 0x05, 0xAA, 0x05, 0x00, 0x90, 0x2F, 0xDC}
#define INMS_ESA_ON_CMD    {0xB4, 0xA7, 0xA0, 0xA7, 0xA0, 0x00, 0x00, 0xBE}
#define INMS_DISCRIM_CMD   {0x8F, 0x00, 0x90, 0xB9, 0xB9, 0x09, 0x00, 0x66}
// UTC Upload command: {0x89, XA, BB, CC, DD, 0x00, 0x00, CRC}

/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (INMS_AppMain), these
**       functions are not called from any other source module.
*/
void INMS_AppMain(void);
int32 INMS_AppInit(void);
void INMS_ProcessCommandPacket(void);
void INMS_ProcessGroundCommand(void);
void INMS_ProcessInstrumentCommand(void);

void INMS_PollInstrument(void);
void INMS_ReportHousekeeping(void);
void INMS_ResetCounters(void);
boolean INMS_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);
uint8 INMS_CalcChecksum(inms_telemetry_t* data);

// Testing
void INMS_LibPrintINMSHeader (inms_msg_header_t *header);
void INMS_LibPrintINMSData(inms_telemetry_t* data);


#endif /* _inms_app_h_ */
