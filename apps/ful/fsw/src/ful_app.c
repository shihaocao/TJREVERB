/*******************************************************************************
** File: 
**   $Id: ful_app.c  $
**
** Purpose:
**   This file contains the source code for the FUL App.
**
*******************************************************************************/

/*
**   Include Files:
*/
#include "ful_app.h"


/*
** global data
*/

ful_hk_tlm_t      FUL_HkTelemetryPkt;
CFE_SB_PipeId_t   FUL_CommandPipe;
CFE_SB_MsgPtr_t   FULMsgPtr;

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* FUL_AppMain() -- Application entry point and main process loop          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void FUL_AppMain( void )
{
    int32             Result;
    uint32            RunStatus = CFE_ES_APP_RUN;
    
    /*
    ** Performance Log (start time counter)...
    */
    CFE_ES_PerfLogEntry(FUL_APP_PERF_ID);

    /*
    ** Register the app with Executive services
    */
    Result = CFE_ES_RegisterApp();
    
    /*
    ** Perform application specific initialization...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = FUL_AppInit();
    }

    /*
    ** Check for start-up error...
    ** If negative, set request to terminate main loop...
    */
    if (Result != CFE_SUCCESS)
    {
        RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** Main process loop...
    */
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        /*
        ** Performance Log (stop time counter)...
        ** Wait for next Software Bus message...
        ** Performance Log (start time counter)...
        ** Process Software Bus messages... (intentionally ignore errors from CFE/SB)
        */
        CFE_ES_PerfLogExit(FUL_APP_PERF_ID);
        Result= CFE_SB_RcvMsg(&FULMsgPtr, FUL_CommandPipe, CFE_SB_PEND_FOREVER);
        CFE_ES_PerfLogEntry(FUL_APP_PERF_ID);
        if (Result == CFE_SUCCESS)
        {
            FUL_ProcessCommandPacket(FULMsgPtr);
        }
    }
    
    /*
    ** Send an event describing the reason for the termination...
    */
    CFE_EVS_SendEvent(FUL_EXIT_ERR_EID, CFE_EVS_CRITICAL, "FUL Application terminating, RunStatus = 0x%08X, Result = 0x%08X", RunStatus, Result);

    /*
    ** In case cFE Event Services is not working...
    */
    CFE_ES_WriteToSysLog("FUL Application terminating, RunStatus = 0x%08X, Result = 0x%08X", RunStatus, Result);
  
    /*
    ** Performance Log (stop time counter)...
    */
    CFE_ES_PerfLogExit(FUL_APP_PERF_ID);

    /*
    ** Let cFE kill the application (and any child tasks)...
    */
    CFE_ES_ExitApp(RunStatus);
    
} /* End of FUL_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* FUL_AppInit() --  initialization                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 FUL_AppInit(void)
{
    int32 Result = CFE_SUCCESS;
    
    /*
    ** Register the events
    */
    Result = CFE_EVS_Register(NULL, 0, 0);

    if (Result != CFE_SUCCESS)
    {
        /* 
        ** Can't send event as registering with event services failed.
        */
        CFE_ES_WriteToSysLog("FUL unable to register for EVS services, err = 0x%08X", Result);
    }
    
    /*
    ** Create the Software Bus command pipe and subscribe to housekeeping messages
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_SB_CreatePipe(&FUL_CommandPipe, FUL_PIPE_DEPTH,"FUL_CMD_PIPE");

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(FUL_INIT_ERR_EID, CFE_EVS_ERROR, "FUL Unable to create input pipe, err = 0x%08X", Result);
        }
    }

    /*
    ** Subscribe to application housekeeping request messages...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_SB_Subscribe(FUL_APP_SEND_HK_MID, FUL_CommandPipe);

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(FUL_INIT_ERR_EID, CFE_EVS_ERROR, "FUL Unable to subscribe to HK request, err = 0x%08X", Result);
        }
    }

    /*
    ** Subscribe to application commands...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_SB_Subscribe(FUL_APP_CMD_MID, FUL_CommandPipe);

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(FUL_INIT_ERR_EID, CFE_EVS_ERROR, "FUL Unable to subscribe to FDL commands, err = 0x%08X", Result);
        }
    }

    /*
    ** Initialize HK packet, this also sets all points to default values of 0, FALSE, NULL.
    */     
    CFE_SB_InitMsg(&FUL_HkTelemetryPkt, FUL_APP_HK_TLM_MID, sizeof(ful_hk_tlm_t), TRUE);
    
    /*
    ** Generate application startup event message...
    */
    if (Result == CFE_SUCCESS)
    {
        CFE_EVS_SendEvent (FUL_STARTUP_INF_EID, CFE_EVS_INFORMATION, "FUL App Initialized. Version %d.%d.%d.%d",
                           FUL_APP_MAJOR_VERSION, FUL_APP_MINOR_VERSION, FUL_APP_REVISION, FUL_APP_MISSION_REV);
    }

    return(Result);
				
} /* End of FUL_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FUL_ProcessCommandPacket                                        */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the FUL    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void FUL_ProcessCommandPacket(CFE_SB_MsgPtr_t MsgPtr)
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId(MsgPtr);

    switch (MsgId)
    {
        case FUL_APP_CMD_MID:
            FUL_ProcessGroundCommand(MsgPtr);
            break;

        case FUL_APP_SEND_HK_MID:
            FUL_ReportHousekeeping();
            break;

        default:
            FUL_HkTelemetryPkt.ful_command_error_count++;
            CFE_EVS_SendEvent(FUL_COMMAND_ERR_EID,CFE_EVS_ERROR, "FUL: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End FUL_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* FUL_ProcessGroundCommand() -- FUL ground commands                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void FUL_ProcessGroundCommand(CFE_SB_MsgPtr_t MsgPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(MsgPtr);
    uint16 CommandCode = CFE_SB_GetCmdCode(MsgPtr);
    
    switch (CommandCode)
    {
        case FUL_APP_NOOP_CC:
            if ( FUL_VerifyCmdLength(MsgPtr, sizeof(FUL_NoArgsCmd_t) ) )
            {
                FUL_HkTelemetryPkt.ful_command_count++;
                CFE_EVS_SendEvent(FUL_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION, "FUL: NOOP command");
            } /* Error messages handled inside VerifyCmdLength function */
            break;

        case FUL_APP_RESET_COUNTERS_CC:
            if ( FUL_VerifyCmdLength(MsgPtr, sizeof(FUL_NoArgsCmd_t) ) )
            {
                FUL_ResetCounters();
            } /* Error messages handled inside VerifyCmdLength function */
            break;

        case FUL_APP_FILE_START_CC:
            if ( FUL_VerifyCmdLength(MsgPtr, sizeof(FUL_FileStartCmd_t) ) )
            {
                FUL_StartFileTransfer((FUL_FileStartCmd_t *)MsgPtr);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

        case FUL_APP_FILE_SEGMENT_CC:
            if ( FUL_VerifyCmdLength(MsgPtr, sizeof(FUL_FileSegmentCmd_t) ) )
            {
                FUL_ProcessFileSegment((FUL_FileSegmentCmd_t *)MsgPtr);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

        case FUL_APP_FILE_FINISH_CC:
            if ( FUL_VerifyCmdLength(MsgPtr, sizeof(FUL_FileFinishCmd_t) ) )
            {
                FUL_FinishFileTransfer((FUL_FileFinishCmd_t *)MsgPtr);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

        case FUL_APP_FILE_CANCEL_CC:
            if ( FUL_VerifyCmdLength(MsgPtr, sizeof(FUL_NoArgsCmd_t) ) )
            {
                FUL_CancelFileTransfer();
            } /* Error messages handled inside VerifyCmdLength function */
            break;

        default:
            FUL_HkTelemetryPkt.ful_command_error_count++;
            CFE_EVS_SendEvent(FUL_COMMAND_ERR_EID,CFE_EVS_ERROR, "FUL: Unknown command code, MID = 0x%x, CC = %d", MsgId,CommandCode);
            break;
    }
    return;

} /* End of FUL_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FUL_ReportHousekeeping                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void FUL_ReportHousekeeping(void)
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &FUL_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &FUL_HkTelemetryPkt);

} /* End of FUL_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FUL_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void FUL_ResetCounters(void)
{
    /* Status of commands processed by the FUL App */
    FUL_HkTelemetryPkt.ful_command_count       = 0;
    FUL_HkTelemetryPkt.ful_command_error_count = 0;

    CFE_EVS_SendEvent(FUL_COMMANDRST_DBG_EID, CFE_EVS_DEBUG, "FUL: RESET command");

} /* End of FUL_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FUL_StartFileTransfer                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function starts a file transfer. Only one transfer can be     */
/*         active at a time.                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void FUL_StartFileTransfer(FUL_FileStartCmd_t *MsgPtr)
{

    /*
    ** Check to see if a file transfer is already in progress
    */
    if ( FUL_HkTelemetryPkt.FileInProgress == TRUE )
    {
        FUL_HkTelemetryPkt.ful_command_error_count++;
        CFE_EVS_SendEvent(FUL_STARTXFER_ERR_EID, CFE_EVS_ERROR, "FUL: Start Transfer Rejected, already in progress");
    }
    else
    {
        /*
        ** Copy data to start file transfer
        */
        FUL_HkTelemetryPkt.LastSegmentAccepted = 0;
        FUL_HkTelemetryPkt.SegmentsRejected = 0;
        FUL_HkTelemetryPkt.BytesTransferred = 0;
        FUL_HkTelemetryPkt.CurrentFileCrc = 0;
        strncpy((char *)FUL_HkTelemetryPkt.CurrentFileName, (char *)MsgPtr->FileName, OS_MAX_PATH_LEN);

        /*
        ** Open the file
        */
        FUL_HkTelemetryPkt.CurrentFileFd = OS_remove((char*)FUL_HkTelemetryPkt.CurrentFileName);
        FUL_HkTelemetryPkt.CurrentFileFd = OS_open((char*)FUL_HkTelemetryPkt.CurrentFileName, OS_WRITE_ONLY, 0);
        if ( FUL_HkTelemetryPkt.CurrentFileFd < 0 ) 
        {
            FUL_HkTelemetryPkt.FileInProgress = FALSE;
            FUL_HkTelemetryPkt.ful_command_error_count++;
            CFE_EVS_SendEvent(FUL_STARTXFER_ERR_EID, CFE_EVS_ERROR, "FUL: File Transfer Rejected, could not open file: Error = %d, Name = %s\n", FUL_HkTelemetryPkt.CurrentFileFd, FUL_HkTelemetryPkt.CurrentFileName);
        }
        else
        {
            FUL_HkTelemetryPkt.FileInProgress = TRUE;
            FUL_HkTelemetryPkt.ful_command_count++;
            CFE_EVS_SendEvent(FUL_STARTXFER_INF_EID, CFE_EVS_INFORMATION, "FUL: File Transfer Started: Name = %s\n", FUL_HkTelemetryPkt.CurrentFileName);
        }
    }

} /* End of FUL_StartFileTransfer() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FUL_CancelFileTransfer                                             */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function cancels a file transfer. Only one transfer can be    */
/*         active at a time.                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void FUL_CancelFileTransfer(void)
{
    
    if ( FUL_HkTelemetryPkt.FileInProgress == TRUE )
    {
         OS_close(FUL_HkTelemetryPkt.CurrentFileFd);
    }
    FUL_HkTelemetryPkt.FileInProgress = FALSE;
    FUL_HkTelemetryPkt.ful_command_count++;
    CFE_EVS_SendEvent(FUL_CANCELXFER_INF_EID, CFE_EVS_INFORMATION, "FUL: File Transfer Cancelled");

    return;

} /* End of FUL_CancelFileTransfer() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FUL_FinishFileTransfer                                             */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function finishes a file transfer. Only one transfer can be   */
/*         active at a time.                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void FUL_FinishFileTransfer(FUL_FileFinishCmd_t *MsgPtr)
{
    
    if ( FUL_HkTelemetryPkt.FileInProgress == TRUE )
    {
        /* 
        ** Validate file size and CRC.
        **    TODO: Should file be deleted on error?
        **    TODO: Not validating last segment accepted.  
        */
        if ( FUL_HkTelemetryPkt.CurrentFileCrc != MsgPtr->FileCrc )
        {
            FUL_HkTelemetryPkt.ful_command_error_count++;
            CFE_EVS_SendEvent(FUL_FINISHXFER_ERR_EID, CFE_EVS_ERROR, "FUL: File Transfer CRC Mismatch : Flt: 0x%08lX, Gnd: 0x%08lX, File: %s", FUL_HkTelemetryPkt.CurrentFileCrc, MsgPtr->FileCrc, FUL_HkTelemetryPkt.CurrentFileName );
            /* Did not clear FileInProgress, give chance to send correct parameter. */
        }
        else if ( FUL_HkTelemetryPkt.BytesTransferred != MsgPtr->FileSize )
        {
            FUL_HkTelemetryPkt.ful_command_error_count++;
            CFE_EVS_SendEvent(FUL_FINISHXFER_ERR_EID, CFE_EVS_ERROR, "FUL: File Transfer Size Mismatch : Flt: %d, Gnd: %d, File: %s", FUL_HkTelemetryPkt.BytesTransferred, MsgPtr->FileSize, FUL_HkTelemetryPkt.CurrentFileName );
            /* Did not clear FileInProgress, give chance to send correct parameter. */
        }
        else
        {
            FUL_HkTelemetryPkt.ful_command_count++;
            FUL_HkTelemetryPkt.SuccessfulFileUploads++;
            CFE_EVS_SendEvent(FUL_FINISHXFER_INF_EID, CFE_EVS_INFORMATION, "FUL: File Transfer Finished : %s", FUL_HkTelemetryPkt.CurrentFileName );
            OS_close(FUL_HkTelemetryPkt.CurrentFileFd);
            FUL_HkTelemetryPkt.FileInProgress = FALSE;
        }
    }
    else
    {
        FUL_HkTelemetryPkt.ful_command_error_count++;
        CFE_EVS_SendEvent(FUL_FINISHXFER_ERR_EID, CFE_EVS_ERROR, "FUL: No File Transfer In Progress" );
    }

} /* End of FUL_FinishFileTransfer() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FUL_ProcessFileSegment                                             */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function processes a segment on the current file transfer.    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void FUL_ProcessFileSegment(FUL_FileSegmentCmd_t *MsgPtr)
{

    int32 NumBytes;

    if ( FUL_HkTelemetryPkt.FileInProgress == TRUE )
    {
        if ( MsgPtr->SegmentDataSize <= FUL_FILE_SEGMENT_SIZE )
        {
            if ( MsgPtr->SegmentNumber == (FUL_HkTelemetryPkt.LastSegmentAccepted + 1))
            {
                FUL_HkTelemetryPkt.LastSegmentAccepted++;
                /*
                ** Write data
                **    If successful, update stats and wait for next data packet
                **    If unsuccessful, abort uplink.
                */
                NumBytes = OS_write(FUL_HkTelemetryPkt.CurrentFileFd, (void *)&(MsgPtr->SegmentData[0]), MsgPtr->SegmentDataSize);   
  
                if ( NumBytes == MsgPtr->SegmentDataSize ) 
                {
                    FUL_HkTelemetryPkt.BytesTransferred += MsgPtr->SegmentDataSize;
                    FUL_HkTelemetryPkt.ful_command_count++;
                    FUL_HkTelemetryPkt.CurrentFileCrc = CFE_ES_CalculateCRC(MsgPtr->SegmentData, MsgPtr->SegmentDataSize, FUL_HkTelemetryPkt.CurrentFileCrc, CFE_ES_DEFAULT_CRC);
                }
                else
                {
                    OS_close(FUL_HkTelemetryPkt.CurrentFileFd);
                    FUL_HkTelemetryPkt.FileInProgress = FALSE;
                    FUL_HkTelemetryPkt.ful_command_error_count++;
                    CFE_EVS_SendEvent(FUL_WRITEERROR_ERR_EID, CFE_EVS_ERROR, "FUL: File Write Error, aborted.  Size: %d, Wrote: %d, File: %s", MsgPtr->SegmentDataSize, NumBytes, FUL_HkTelemetryPkt.CurrentFileName );  
                }
            }
            else
            {
                /* No event here to prevent from flooding the downlink */
                FUL_HkTelemetryPkt.SegmentsRejected++;
                FUL_HkTelemetryPkt.ful_command_error_count++;
            }
        }
        else
        {
            /* No event here to prevent from flooding the downlink */
            FUL_HkTelemetryPkt.SegmentsRejected++;
            FUL_HkTelemetryPkt.ful_command_error_count++;
        }
    }
    else
    {
        /* No event here to prevent from flooding the downlink */
        FUL_HkTelemetryPkt.ful_command_error_count++;
    }

} /* End of FUL_ProcessFileSegment() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* FUL_VerifyCmdLength() -- Verify command packet length                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
boolean FUL_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
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

        CFE_EVS_SendEvent(FUL_LEN_ERR_EID, CFE_EVS_ERROR,
                          "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
                          MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        FUL_HkTelemetryPkt.ful_command_error_count++;
    }

    return(result);

} /* End of FUL_VerifyCmdLength() */

/************************/
/*  End of File Comment */
/************************/

