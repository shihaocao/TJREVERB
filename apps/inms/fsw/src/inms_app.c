/*******************************************************************************
** File: inms_app.c
**
** Purpose:
**   This file contains the source code for the INMS App.
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "inms_app.h"
#include "inms_perfids.h"
#include "inms_msgids.h"
#include "dhl_lib.h"
#include "shk_msgids.h"

/*
** Defines
*/

/*
** global data
*/

inms_hk_tlm_t            INMS_HkTelemetryPkt;
inms_instrument_hk_tlm_t INMS_InstTelemetryPkt;
CFE_SB_PipeId_t          INMS_CommandPipe;
CFE_SB_MsgPtr_t          INMSMsgPtr;
uint32                   INMSQueueId;

char                     INMSOutputBuffer[INMS_TELEMETRY_LENGTH];

static CFE_EVS_BinFilter_t  INMS_EventFilters[] =
       {  /* Event ID    mask */
          {INMS_STARTUP_INF_EID,       0x0000},
          {INMS_COMMAND_ERR_EID,       0x0000},
          {INMS_COMMANDNOP_INF_EID,    0x0000},
          {INMS_COMMANDRST_INF_EID,    0x0000},
       };

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* INMS_AppMain() -- Application entry point and main process loop          */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void INMS_AppMain( void )
{
    int32  Status    = CFE_SUCCESS;
    uint32 RunStatus = CFE_ES_APP_RUN;

    CFE_ES_PerfLogEntry(INMS_APP_PERF_ID);

    Status = INMS_AppInit();
    if (Status != CFE_SUCCESS)
    {
        RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** INMS Runloop
    */
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        CFE_ES_PerfLogExit(INMS_APP_PERF_ID);

        Status = CFE_SB_RcvMsg(&INMSMsgPtr, INMS_CommandPipe, INMS_POLL_TIMEOUT);
        
        CFE_ES_PerfLogEntry(INMS_APP_PERF_ID);

        if (Status == CFE_SUCCESS)
        {
            INMS_ProcessCommandPacket();
        } else if (Status == CFE_SB_TIME_OUT) {
            // Timeouts are okay.
        } else {
            RunStatus = CFE_ES_APP_ERROR;
        }
    }

    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(INMS_TASK_EXIT_EID, CFE_EVS_CRITICAL,
                          "Task terminating, err = 0x%08X", Status);
        CFE_ES_WriteToSysLog("INMS task terminating, err = 0x%08X\n", Status);        
    }

    CFE_ES_PerfLogExit(INMS_APP_PERF_ID);
    CFE_ES_ExitApp(RunStatus);
} /* End of INMS_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/* INMS_AppInit() --  initialization                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 INMS_AppInit(void)
{
    int32 Status;

    /*
    ** Register the app with Executive services
    */
    Status = CFE_ES_RegisterApp() ;
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    /*
    ** Register the events
    */
    Status = CFE_EVS_Register(INMS_EventFilters,
                     sizeof(INMS_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                     CFE_EVS_BINARY_FILTER);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    /*
    ** Create the Software Bus command pipe and subscribe to housekeeping
    **  messages
    */
    Status = CFE_SB_CreatePipe(&INMS_CommandPipe, INMS_PIPE_DEPTH,"INMS_CMD_PIPE");
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(INMS_APP_CMD_MID, INMS_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(INMS_APP_SEND_HK_MID, INMS_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(INMS_APP_2HZ_WAKEUP_MID, INMS_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    INMS_ResetCounters();

    CFE_SB_InitMsg(&INMS_HkTelemetryPkt,
                   INMS_APP_HK_TLM_MID,
                   INMS_APP_HK_TLM_LNGTH, TRUE);

    CFE_SB_InitMsg(&INMS_InstTelemetryPkt,
                   INMS_APP_INST_TLM_MID,
                   INMS_APP_INST_TLM_LNGTH, TRUE);

    Status = OS_QueueCreate ( &INMSQueueId, "INMS_CMD", 12, 8, 0);
    if ( Status != OS_SUCCESS )
    {
        CFE_ES_WriteToSysLog("INMS QueueCreate failed, err = 0x%08X", Status);
        return Status;
    }


    /* 
    ** Setup the SPI bus for the INMS instrument
    ** TODO: This may have to move to the task_init.c before
    ** the cFE/CFS is started.
    */
    INMS_LibInitDevice();

    /* Clear output buffer */
    CFE_PSP_MemSet(INMSOutputBuffer,0x00,sizeof(INMSOutputBuffer));

    CFE_EVS_SendEvent (INMS_STARTUP_INF_EID, CFE_EVS_INFORMATION,
               "INMS App Initialized. Version %d.%d.%d.%d",
                INMS_APP_MAJOR_VERSION,
                INMS_APP_MINOR_VERSION, 
                INMS_APP_REVISION, 
                INMS_APP_MISSION_REV);

    return CFE_SUCCESS;
} /* End of INMS_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  INMS_ProcessCommandPacket                                        */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the INMS    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void INMS_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId(INMSMsgPtr);

    switch (MsgId)
    {
        case INMS_APP_CMD_MID:
            INMS_ProcessGroundCommand();
            break;

        case INMS_APP_SEND_HK_MID:
            INMS_ReportHousekeeping();
            break;

        case INMS_APP_2HZ_WAKEUP_MID:
            if (MODE_GetSpacecraftMode() == MODE_INMS) {
                INMS_PollInstrument();
            }
            break;

        default:
            INMS_HkTelemetryPkt.inms_command_error_count++;
            CFE_EVS_SendEvent(INMS_COMMAND_ERR_EID,CFE_EVS_ERROR,
            "INMS: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End INMS_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* INMS_ProcessGroundCommand() -- INMS ground commands                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void INMS_ProcessGroundCommand(void)
{
    uint16 CommandCode;

    CommandCode = CFE_SB_GetCmdCode(INMSMsgPtr);

    /* Process "known" INMS app ground commands */
    switch (CommandCode)
    {
        case INMS_APP_NOOP_CC:
            INMS_HkTelemetryPkt.inms_command_count++;
            CFE_EVS_SendEvent(INMS_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION,
            "INMS: NOOP command");
            break;

        case INMS_APP_INSTRUMENT_COMMAND_CC:
            //INMS_HkTelemetryPkt.inms_command_count++;
            CFE_EVS_SendEvent(INMS_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION,
            "INMS: INST command");

            INMS_ProcessInstrumentCommand();
            break;

	case INMS_APP_RESET_COUNTERS_CC:
            INMS_ResetCounters();
            break;

        /* default case already found during FC vs length test */
        default:
            break;
    }
    return;

} /* End of INMS_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  INMS_ReportHousekeeping                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void INMS_ReportHousekeeping(void)
{

#ifdef INMS_TEST
    OS_printf("INMS Report HK - INMS Instrument packets collected = %d\n",
                INMS_HkTelemetryPkt.inms_inst_tlm_packets);
#endif 
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &INMS_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &INMS_HkTelemetryPkt);

    return;

} /* End of INMS_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  INMS_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void INMS_ResetCounters(void)
{
    /* Status of commands processed by the INMS App */
    INMS_HkTelemetryPkt.inms_command_count       = 0;
    INMS_HkTelemetryPkt.inms_command_error_count = 0;

    CFE_EVS_SendEvent(INMS_COMMANDRST_INF_EID, CFE_EVS_INFORMATION,
		"INMS: RESET command");
    return;

} /* End of INMS_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  INMS_ProcessInstrumentCommand                                      */
/*                                                                            */
/*  Purpose:                                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void INMS_ProcessInstrumentCommand(void)
{
    int32 QueueStatus;
    INMS_CMDInst_t *inms_command;
    inms_command = (INMS_CMDInst_t *)INMSMsgPtr;

    OS_printf("Process Instrument command: Opcode 0x%02X\n", inms_command->opcode);

    /*
    ** When an INMS instrument command is received,
    ** put it on the queue, so the instrument poll function can read it.
    */
    if (MODE_GetSpacecraftMode() == MODE_INMS) {
        QueueStatus = OS_QueuePut (INMSQueueId, (void *)&(inms_command->opcode), 8, 0);
	if ( QueueStatus != OS_SUCCESS ) {
	    OS_printf("Error putting INMS Command on Queue\n");
	    INMS_HkTelemetryPkt.inms_command_error_count++;
	} else {
	    INMS_HkTelemetryPkt.inms_command_count++;
	}
    } else {
        OS_printf("Attempted to enqueue INMS Command while not in INMS mode\n");
	INMS_HkTelemetryPkt.inms_command_error_count++;
    }
} /* End of INMS_ProcessInstrumentCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* INMS_VerifyCmdLength() -- Verify command packet length                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
// TODO: Check if used.
boolean INMS_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
{     
    boolean result = TRUE;

    uint16 ActualLength = CFE_SB_GetTotalMsgLength(msg);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID   = CFE_SB_GetMsgId(msg);
        uint16         CommandCode = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(INMS_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        INMS_HkTelemetryPkt.inms_command_error_count++;
    }

    return(result);

} /* End of INMS_VerifyCmdLength() */

