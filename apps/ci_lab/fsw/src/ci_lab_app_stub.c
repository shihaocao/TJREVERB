/*******************************************************************************
** File: ci_lab_app.c
**
** Purpose:
**   This file contains the source code for the Command Ingest task.
**
** $Log: ci_lab_app.c  $
** Revision 1.7 2010/09/20 13:27:19EDT wmoleski 
** Modified the CI_LAB, SCH_LAB and TO_LAB applications to use unique message IDs and Pipe Names. The "_LAB"
** was added to all definitions so that a mission can use these "Lab" apps as well as their own mission apps together.
** Revision 1.6 2010/09/02 09:53:55EDT wmoleski 
** Modifications to the CI_Lab code to capture, drop and/or manipulate CFDP PDUs that are uplinked
** from the ground. These changes were needed to test the CF fault detection requirements.
** Revision 1.5 2008/09/22 13:58:20EDT apcudmore 
** Added RunLoop call to CI_LAB app. Also added the task delete handler to close the CI socket.
** Revision 1.4 2008/09/19 15:30:42EDT rjmcgraw 
** DCR4337:1 Added #include version.h and display version after initialization is complete
** Revision 1.3 2008/05/01 11:49:21EDT rjmcgraw 
** DCR1718:1 Changed wording in intiialization event
** Revision 1.2 2008/04/30 15:24:31EDT rjmcgraw 
** DCR1718:1 Added version number in initialization event
** Revision 1.1 2008/04/30 13:56:37EDT rjmcgraw 
** Initial revision
** Member added to CFS project
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "ci_lab_app.h"
#include "ci_lab_perfids.h"
#include "ci_lab_msgids.h"
#include "ci_lab_msg.h"
#include "ci_lab_defs.h"
#include "ci_lab_events.h"
#include "ci_lab_version.h"

/*
** CI global data...
*/

ci_hk_tlm_t        CI_HkTelemetryPkt OS_ALIGN(4) ;
CFE_SB_MsgPtr_t    CIMsgPtr;
uint8              CI_IngestBuffer[CI_MAX_INGEST];
CFE_SB_Msg_t       *CI_IngestPointer = (CFE_SB_Msg_t *) &CI_IngestBuffer[0];
CFE_SB_PipeId_t    CI_CommandPipe;
uint32             CI_UplinkConnected;

static CFE_EVS_BinFilter_t  CI_EventFilters[] =
          {  /* Event ID    mask */
            {CI_STARTUP_INF_EID,       0x0000},
            {CI_COMMAND_ERR_EID,       0x0000},
            {CI_COMMANDNOP_INF_EID,    0x0000},
            {CI_COMMANDRST_INF_EID,    0x0000},
            {CI_INGEST_INF_EID,        0x0000},
            {CI_INGEST_ERR_EID,        0x0000}
          };

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* CI_Lab_AppMain() -- Application entry point and main process loop          */
/* Purpose: This is the Main task event loop for the Command Ingest Task      */
/*            The task handles all interfaces to the data system through      */
/*            the software bus. There is one pipeline into this task          */
/*            The task is sceduled by input into this pipeline.               */
/*            It can receive Commands over this pipeline                      */
/*            and acts accordingly to process them.                           */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void CI_Lab_AppMain( void )
{
    int32  status;
    uint32 RunStatus = CFE_ES_APP_RUN;

    CFE_ES_PerfLogEntry(CI_MAIN_TASK_PERF_ID);
    CI_TaskInit();

    /*
    ** CI Runloop
    */
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        CFE_ES_PerfLogExit(CI_MAIN_TASK_PERF_ID);

        /* Pend on receipt of command packet -- timeout set to 1 second  */
        status = CFE_SB_RcvMsg(&CIMsgPtr, CI_CommandPipe, 1000);
        
        CFE_ES_PerfLogEntry(CI_MAIN_TASK_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            CI_ProcessCommandPacket();
        }
        else if (status == CFE_SB_TIME_OUT )
        {
           /* Process uplink queue */
           CI_ReadUpLink();
        }
    }

    CFE_ES_ExitApp(RunStatus);

} /* End of CI_Lab_AppMain() */

/*
** CI delete callback function.
** This function will be called in the event that the CI app is killed. 
*/
void CI_delete_callback(void)
{
    OS_printf("CI delete callback.\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* CI_TaskInit() -- CI initialization                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void CI_TaskInit(void)
{
    CFE_ES_RegisterApp() ;

    CFE_EVS_Register(CI_EventFilters,
                     sizeof(CI_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                     CFE_EVS_BINARY_FILTER);

    CFE_SB_CreatePipe(&CI_CommandPipe, CI_PIPE_DEPTH,"CI_LAB_CMD_PIPE");
    CFE_SB_Subscribe(CI_LAB_CMD_MID, CI_CommandPipe);
    CFE_SB_Subscribe(CI_LAB_SEND_HK_MID, CI_CommandPipe);

    CI_ResetCounters();

    /*
    ** Install the delete handler
    */
    OS_TaskInstallDeleteHandler((void *)(&CI_delete_callback)); 

    CFE_SB_InitMsg(&CI_HkTelemetryPkt,
                   CI_LAB_HK_TLM_MID,
                   CI_LAB_HK_TLM_LNGTH, TRUE);

				
    CFE_EVS_SendEvent (CI_STARTUP_INF_EID, CFE_EVS_INFORMATION,
               "CI Lab Initialized.  Version %d.%d.%d.%d",
                CI_LAB_MAJOR_VERSION,
                CI_LAB_MINOR_VERSION, 
                CI_LAB_REVISION, 
                CI_LAB_MISSION_REV);

				
} /* End of CI_TaskInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  CI_ProcessCommandPacket                                            */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the CI command*/
/*     pipe. The packets received on the CI command pipe are listed here:     */
/*                                                                            */
/*        1. NOOP command (from ground)                                       */
/*        2. Request to reset telemetry counters (from ground)                */
/*        3. Request for housekeeping telemetry packet (from HS task)         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void CI_ProcessCommandPacket(void)
{
CFE_SB_MsgId_t  MsgId;
MsgId = CFE_SB_GetMsgId(CIMsgPtr);

    switch (MsgId)
    {
        case CI_LAB_CMD_MID:
            CI_ProcessGroundCommand();
            break;

        case CI_LAB_SEND_HK_MID:
            CI_ReportHousekeeping();
            break;

        default:
            CI_HkTelemetryPkt.ci_command_error_count++;
            CFE_EVS_SendEvent(CI_COMMAND_ERR_EID,CFE_EVS_ERROR,
			"CI: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End CI_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* CI_ProcessGroundCommand() -- CI ground commands                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void CI_ProcessGroundCommand(void)
{
uint16 CommandCode;

    CommandCode = CFE_SB_GetCmdCode(CIMsgPtr);

    /* Process "known" CI task ground commands */
    switch (CommandCode)
    {
        case CI_NOOP_CC:
            CI_HkTelemetryPkt.ci_command_count++;
            CFE_EVS_SendEvent(CI_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION,
			"CI: NOOP command");
            break;

        case CI_RESET_COUNTERS_CC:
            CI_ResetCounters();
            break;

        /* default case already found during FC vs length test */
        default:
            break;
    }


    return;

} /* End of CI_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  CI_ReportHousekeeping                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the CI task  */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void CI_ReportHousekeeping(void)
{
    CI_HkTelemetryPkt.UplinkConnected = CI_UplinkConnected;
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &CI_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &CI_HkTelemetryPkt);
    return;

} /* End of CI_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  CI_ResetCounters                                                   */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void CI_ResetCounters(void)
{
    /* Status of commands processed by CI task */
    CI_HkTelemetryPkt.ci_command_count       = 0;
    CI_HkTelemetryPkt.ci_command_error_count = 0;

    /* Status of packets ingested by CI task */
    CI_HkTelemetryPkt.IngestPackets = 0;
    CI_HkTelemetryPkt.IngestErrors  = 0;

    CFE_EVS_SendEvent(CI_COMMANDRST_INF_EID, CFE_EVS_INFORMATION,
		"CI: RESET command");
    
    return;

} /* End of CI_ResetCounters() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* CI_ReadUpLink() --                                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void CI_ReadUpLink(void)
{

    int32 sbstatus;

    OS_printf("CI_LAB: Read Uplink.\n");

    #if 0
    sbstatus = CFE_SB_SendMsg(CI_IngestPointer);
    #endif

    return;

} /* End of CI_ReadUpLink() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* CI_VerifyCmdLength() -- Verify command packet length                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
boolean CI_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
{     
    boolean result = TRUE;
    uint16 ActualLength = CFE_SB_GetTotalMsgLength(msg);

    /*
    ** Verify the command packet length...
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID = CFE_SB_GetMsgId(msg);
        uint16 CommandCode = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(CI_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        CI_HkTelemetryPkt.ci_command_error_count++;
    }

    return(result);

} /* End of CI_VerifyCmdLength() */

