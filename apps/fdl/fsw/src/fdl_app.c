/*******************************************************************************
**File: 
**   $Id: fdl_app.c $
**
**Purpose:
**  This file contains the source code for the FDL App.
**
*******************************************************************************/

/*
**   Include Files:
*/
#include "fdl_app.h"
#include "cadet_app.h"
#include <sys/types.h>
#include <sys/stat.h>

/*
** global data
*/
fdl_hk_tlm_t            FDL_HkTelemetryPkt;
CFE_SB_PipeId_t         FDL_CommandPipe;
CFE_SB_MsgPtr_t         FDL_MsgPtr;
uint32                  FDL_SemID;

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* FDL_AppMain() -- Application entry point and main process loop             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_AppMain( void )
{
    int32             Result;
    uint32            RunStatus = CFE_ES_APP_RUN;
    
    /*
    ** Performance Log (start time counter)...
    */
    CFE_ES_PerfLogEntry(FDL_APP_PERF_ID);

    /*
    ** Register the app with Executive services
    */
    Result = CFE_ES_RegisterApp();
    
    /*
    ** Perform application specific initialization...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = FDL_AppInit();
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
        CFE_ES_PerfLogExit(FDL_APP_PERF_ID);
        Result= CFE_SB_RcvMsg(&FDL_MsgPtr, FDL_CommandPipe, CFE_SB_PEND_FOREVER);
        CFE_ES_PerfLogEntry(FDL_APP_PERF_ID);
        if (Result == CFE_SUCCESS)
        {
            FDL_ProcessCommandPacket(FDL_MsgPtr);
        }
    }
    
    /*
    ** Send an event describing the reason for the termination...
    */
    CFE_EVS_SendEvent(FDL_EXIT_ERR_EID, CFE_EVS_CRITICAL, "FDL Application terminating, RunStatus = 0x%08X, Result = 0x%08X", RunStatus, Result);

    /*
    ** In case cFE Event Services is not working...
    */
    CFE_ES_WriteToSysLog("FDL Application terminating, RunStatus = 0x%08X, Result = 0x%08X", RunStatus, Result);
  
    /*
    ** Performance Log (stop time counter)...
    */
    CFE_ES_PerfLogExit(FDL_APP_PERF_ID);

    /*
    ** Let cFE kill the application (and any child tasks)...
    */
    CFE_ES_ExitApp(RunStatus);
    
} /* End of FDL_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* FDL_AppInit() --  initialization                                           */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 FDL_AppInit(void)
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
        CFE_ES_WriteToSysLog("FDL unable to register for EVS services, err = 0x%08X", Result);
    }
    
    /*
    ** Create the Software Bus command pipe and subscribe to housekeeping messages
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_SB_CreatePipe(&FDL_CommandPipe, FDL_PIPE_DEPTH,"FDL_CMD_PIPE");

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(FDL_INIT_ERR_EID, CFE_EVS_ERROR, "FDL Unable to create input pipe, err = 0x%08X", Result);
        }
    }

    /*
    ** Subscribe to application housekeeping request messages...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_SB_Subscribe(FDL_APP_SEND_HK_MID, FDL_CommandPipe);

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(FDL_INIT_ERR_EID, CFE_EVS_ERROR, "FDL Unable to subscribe to HK request, err = 0x%08X", Result);
        }
    }

    /*
    ** Subscribe to application commands...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_SB_Subscribe(FDL_APP_CMD_MID, FDL_CommandPipe);

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(FDL_INIT_ERR_EID, CFE_EVS_ERROR, "FDL Unable to subscribe to FDL commands, err = 0x%08X", Result);
        }
    }
    
    /*
    ** Subscribe to application wakeup messages...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_SB_Subscribe(FDL_APP_WAKEUP_MID, FDL_CommandPipe);

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(FDL_INIT_ERR_EID, CFE_EVS_ERROR, "FDL Unable to subscribe to wakeup request, err = 0x%08X", Result);
        }
    }

    /*
    ** Get Semaphore ID...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = OS_CountSemGetIdByName(&FDL_SemID, CADET_FDL_HANDSHAKE_SEM_NAME );

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(FDL_INIT_ERR_EID, CFE_EVS_ERROR, "FDL Unable to get semaphore ID, err = 0x%08X", Result);
        }
    }
    
    /*
    ** Initialize HK packet, this also sets all points to default values of 0, FALSE, NULL.
    */     
    CFE_SB_InitMsg(&FDL_HkTelemetryPkt, FDL_APP_HK_TLM_MID, sizeof(fdl_hk_tlm_t), TRUE);
    
    /*
    ** Generate application startup event message...
    */
    if (Result == CFE_SUCCESS)
    {
        CFE_EVS_SendEvent (FDL_STARTUP_INF_EID, CFE_EVS_INFORMATION, "FDL App Initialized. Version %d.%d.%d.%d",
                           FDL_APP_MAJOR_VERSION, FDL_APP_MINOR_VERSION, FDL_APP_REVISION,FDL_APP_MISSION_REV);
    }

    return(Result);
				
} /* End of FDL_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_ProcessCommandPacket                                           */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the FDL       */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_ProcessCommandPacket(CFE_SB_MsgPtr_t MsgPtr)
{
    CFE_SB_MsgId_t  MsgId;
    
    MsgId = CFE_SB_GetMsgId(MsgPtr);
    
    switch (MsgId)
    {
        case FDL_APP_CMD_MID:
            FDL_ProcessGroundCommand(MsgPtr);
            break;
            
        case FDL_APP_SEND_HK_MID:
            FDL_ReportHousekeeping();
            break;
            
        case FDL_APP_WAKEUP_MID:
            FDL_SendDataFileDownlinkTlmPackets();
            break;
            
        default:
            FDL_HkTelemetryPkt.fdl_command_error_count++;
            CFE_EVS_SendEvent(FDL_INVALID_MSGID_ERR_EID,CFE_EVS_ERROR, "FDL: Unknown Message ID, ID = 0x%x", MsgId);
            break;
    }
    
    return;
    
} /* End FDL_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* FDL_ProcessGroundCommand() -- FDL ground commands                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void FDL_ProcessGroundCommand(CFE_SB_MsgPtr_t MsgPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(MsgPtr);
    uint16 CommandCode = CFE_SB_GetCmdCode(MsgPtr);
    
    switch (CommandCode)
    {
        case FDL_APP_NOOP_CC:
            if ( FDL_VerifyCmdLength(MsgPtr, sizeof(FDL_NoArgsCmd_t) ) )
            {
                FDL_HkTelemetryPkt.fdl_command_count++;
                CFE_EVS_SendEvent(FDL_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION, "FDL: NOOP command");
            } /* Error messages handled inside VerifyCmdLength function */
            break;
            
        case FDL_APP_RESET_COUNTERS_CC:
            if ( FDL_VerifyCmdLength(MsgPtr, sizeof(FDL_NoArgsCmd_t) ) )
            {
                FDL_ResetCounters();
            } /* Error messages handled inside VerifyCmdLength function */
            break;
            
        case FDL_APP_FILE_START_CC:
            if ( FDL_VerifyCmdLength(MsgPtr, sizeof(FDL_FileStartCmd_t) ) )
            {
                FDL_StartFileTransfer((FDL_FileStartCmd_t *)MsgPtr);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

        case FDL_APP_FILE_CANCEL_CC:
            if ( FDL_VerifyCmdLength(MsgPtr, sizeof(FDL_NoArgsCmd_t) ) )
            {
                FDL_CancelFileTransfer();
            } /* Error messages handled inside VerifyCmdLength function */
            break;
            
        case FDL_APP_FILE_PAUSE_CC:
            if ( FDL_VerifyCmdLength(MsgPtr, sizeof(FDL_NoArgsCmd_t) ) )
            {
                FDL_PauseFileTransfer();
            } /* Error messages handled inside VerifyCmdLength function */
            break;

        case FDL_APP_FILE_RESUME_CC:
            if ( FDL_VerifyCmdLength(MsgPtr, sizeof(FDL_NoArgsCmd_t) ) )
            {
                FDL_ResumeFileTransfer();
            } /* Error messages handled inside VerifyCmdLength function */
            break;

        default:
            FDL_HkTelemetryPkt.fdl_command_error_count++;
            CFE_EVS_SendEvent(FDL_COMMAND_ERR_EID,CFE_EVS_ERROR, "FDL: Unknown command code, MID = 0x%x, CC = %d", MsgId,CommandCode);
            break;
    }
    
} /* End of FDL_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_ReportHousekeeping                                             */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_ReportHousekeeping(void)
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &FDL_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &FDL_HkTelemetryPkt);
    
} /* End of FDL_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_ResetCounters                                                  */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_ResetCounters(void)
{
    /* Status of commands processed by the FDL App */
    FDL_HkTelemetryPkt.fdl_command_count       = 0;
    FDL_HkTelemetryPkt.fdl_command_error_count = 0;
    
    CFE_EVS_SendEvent(FDL_COMMANDRST_DBG_EID, CFE_EVS_DEBUG, "FDL: RESET command");

} /* End of FDL_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_SkipToDesiredOffset                                            */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function reads bytes to skip to the desired start of the      */
/*         file to be downlinked.                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 FDL_SkipToDesiredOffset ( void )
{
    uint8   TempArray [FDL_MAX_SEGMENT_SIZE];
    uint32  bytesToSkip = FDL_HkTelemetryPkt.Offset*FDL_HkTelemetryPkt.SegmentSize;
    uint32  bytesRead;
    uint32  bytesToBeRead;

    /*
    ** Loop until desired bytes are skipped.
    */
    while( bytesToSkip > 0 )
    {
        /*
        ** Determine bytes to be read, ideally everything, but cap at array size of FDL_MAX_SEGMENT_SIZE
        */
        bytesToBeRead = bytesToSkip;
        if ( bytesToBeRead > FDL_MAX_SEGMENT_SIZE )
        {
            bytesToBeRead = FDL_MAX_SEGMENT_SIZE;
        }

        /*
        ** Read the bytes, Update CRC across range just read, and update bytes to skip control variable.
        */
        bytesRead = OS_read(FDL_HkTelemetryPkt.CurrentFileFd, TempArray, bytesToBeRead);
        if ( bytesRead == bytesToBeRead )
        {
            FDL_HkTelemetryPkt.CurrentFileCrc = CFE_ES_CalculateCRC(TempArray, bytesRead, FDL_HkTelemetryPkt.CurrentFileCrc, CFE_ES_DEFAULT_CRC);
            bytesToSkip -= bytesRead;
        }
        else
        {
            /* 
            ** Error with OS_read.  Don't know how to recover.  Just abort. 
            */
            return ( FALSE );
        }
    }

    return ( TRUE );

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_SendStartFileDownlinkTlmPacket                                 */
/*                                                                            */
/*  Purpose:                                                                  */
/*         Initialize and send the start file downlink telemetry packet.      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_SendStartFileDownlinkTlmPacket ( void )
{
    FDL_FileStart_t   FDL_FileStartPkt;

    CFE_SB_InitMsg(&FDL_FileStartPkt, FDL_APP_STARTFILE_MID, sizeof(FDL_FileStart_t), TRUE);

    FDL_FileStartPkt.CurrentTransferSize = FDL_HkTelemetryPkt.CurrentTransferSize;
    strncpy((char *)FDL_FileStartPkt.FileName, (char *)FDL_HkTelemetryPkt.CurrentFileName, OS_MAX_PATH_LEN);

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &FDL_FileStartPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &FDL_FileStartPkt);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_SendCompleteFileDownlinkTlmPacket                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         Initialize and send the complete file downlink telemetry packet.   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_SendCompleteFileDownlinkTlmPacket ( void )
{
    FDL_FileComplete_t   FDL_FileCompletePkt;

    CFE_SB_InitMsg(&FDL_FileCompletePkt, FDL_APP_COMPLETEFILE_MID, sizeof(FDL_FileComplete_t), TRUE);

    FDL_FileCompletePkt.CurrentTransferSize = FDL_HkTelemetryPkt.CurrentTransferSize;
    FDL_FileCompletePkt.FileCrc = FDL_HkTelemetryPkt.CurrentFileCrc;
    FDL_FileCompletePkt.LastSegmentSent= FDL_HkTelemetryPkt.LastSegmentSent;

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &FDL_FileCompletePkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &FDL_FileCompletePkt);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_StartFileTransfer                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function starts the transfer of the file. It opens the file   */
/*         and initialize the variables.                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_StartFileTransfer(FDL_FileStartCmd_t *MsgPtr)
{
    os_fstat_t     StatBuf;
    int            stat_status;

    /*
    ** Check to see if a file transfer is already in progress.
    ** Return if there is a file transfer in progress.
    */
    if ( FDL_HkTelemetryPkt.TransferInProgress == TRUE )
    {
        FDL_HkTelemetryPkt.fdl_command_error_count++;
        CFE_EVS_SendEvent(FDL_START_FILE_XFER_ERR_EID,CFE_EVS_ERROR, "FDL: Start Transfer Rejected, already in progress");
        return;
    }

    /*
    ** Copy data to start file transfer
    */
    FDL_HkTelemetryPkt.LastSegmentSent = 0;
    FDL_HkTelemetryPkt.SegmentSize= 0;
    FDL_HkTelemetryPkt.BytesTransferred = 0;
    FDL_HkTelemetryPkt.Offset= 0;
    FDL_HkTelemetryPkt.CurrentTransferSize = 0;
    FDL_HkTelemetryPkt.CurrentFileSize = 0;
    FDL_HkTelemetryPkt.CurrentFileCrc = 0;
    FDL_HkTelemetryPkt.CurrentFileFd = 0;
    memset((char *)FDL_HkTelemetryPkt.CurrentFileName, '\0', OS_MAX_PATH_LEN);
    strncpy((char *)FDL_HkTelemetryPkt.CurrentFileName, (char *)MsgPtr->FileName, OS_MAX_PATH_LEN);

    /*
    ** Open the file. Return if there is an error.
    */
    FDL_HkTelemetryPkt.CurrentFileFd = OS_open((char*)FDL_HkTelemetryPkt.CurrentFileName, OS_READ_ONLY, 0);
    if ( FDL_HkTelemetryPkt.CurrentFileFd < 0 )
    {
        FDL_HkTelemetryPkt.fdl_command_error_count++;
        CFE_EVS_SendEvent(FDL_START_FILE_XFER_ERR_EID,CFE_EVS_ERROR, "FDL: Start Transfer Rejected, unable to open file %s", FDL_HkTelemetryPkt.CurrentFileName);
        return;
    }

    /*
    ** Determine file size. Return if there is an error.
    */
    stat_status = OS_stat((char *)FDL_HkTelemetryPkt.CurrentFileName, &StatBuf);
    if ( stat_status < 0 )
    {
        FDL_HkTelemetryPkt.fdl_command_error_count++;
        CFE_EVS_SendEvent(FDL_START_FILE_XFER_ERR_EID,CFE_EVS_ERROR, "FDL: Start Transfer Rejected, unable to determine file size %s", FDL_HkTelemetryPkt.CurrentFileName);
        return;
    }
    FDL_HkTelemetryPkt.CurrentFileSize = StatBuf.st_size;

    /*
    ** Check if a segment size was specified in the request
    */
    if((FDL_MIN_SEGMENT_SIZE <= MsgPtr->SegmentSize) && (MsgPtr->SegmentSize <= FDL_MAX_SEGMENT_SIZE))
    {
        FDL_HkTelemetryPkt.SegmentSize = MsgPtr->SegmentSize;
    }
    else
    {
        FDL_HkTelemetryPkt.SegmentSize = FDL_DEFAULT_SEGMENT_SIZE;
        CFE_EVS_SendEvent(FDL_STARTXFER_INF_EID, CFE_EVS_INFORMATION, "FDL: Invalid segment size (%d), using default (%d).", MsgPtr->SegmentSize, FDL_DEFAULT_SEGMENT_SIZE);
    }

    /*
    ** Check if an offset was specified in the request. If the offset is 
    ** greater than the file size, then send the whole file.
    */
    if((MsgPtr->Offset*MsgPtr->SegmentSize) < FDL_HkTelemetryPkt.CurrentFileSize)
    {
        FDL_HkTelemetryPkt.Offset = MsgPtr->Offset;
    }
    else
    {
        FDL_HkTelemetryPkt.Offset = 0;
        CFE_EVS_SendEvent(FDL_STARTXFER_INF_EID, CFE_EVS_INFORMATION, "FDL: Invalid offset size (%d), sending entire file.", MsgPtr->Offset*MsgPtr->SegmentSize);
    }
    FDL_HkTelemetryPkt.CurrentTransferSize = FDL_HkTelemetryPkt.CurrentFileSize - (FDL_HkTelemetryPkt.Offset*MsgPtr->SegmentSize);
    
    if ( FDL_SkipToDesiredOffset ( ) == TRUE )
    {
        /*
        ** All checks passed.
        ** Start sending file data on the next wakeup.
        */
        FDL_HkTelemetryPkt.fdl_command_count++;
        FDL_HkTelemetryPkt.TransferInProgress = TRUE;
        FDL_HkTelemetryPkt.TransferPaused = FALSE;
        if ( FDL_HkTelemetryPkt.Offset == 0 )
        {
            FDL_SendStartFileDownlinkTlmPacket ( );
        }
        else
        {
            FDL_HkTelemetryPkt.LastSegmentSent = FDL_HkTelemetryPkt.Offset;
        }

        CFE_EVS_SendEvent(FDL_STARTXFER_INF_EID, CFE_EVS_INFORMATION, "FDL: File Transfer Started: Name = %s", FDL_HkTelemetryPkt.CurrentFileName);
    }
    else
    {
        FDL_HkTelemetryPkt.fdl_command_error_count++;
        CFE_EVS_SendEvent(FDL_START_FILE_XFER_ERR_EID,CFE_EVS_ERROR, "FDL: Start Transfer Rejected, unable to reach desired offset %s", FDL_HkTelemetryPkt.CurrentFileName);
        return;
    }
    
} /* End of FDL_StartFileTransfer() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_TransferFile                                                   */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function sends the segments of the file.                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_SendDataFileDownlinkTlmPackets (void)
{
    FDL_FileSegment_t FDL_FileSegmentPkt;
    uint8             MoreData;
    uint8             FileComplete;
    uint16            Length;
    uint32            bytesRead;

    /*
    ** First verify we are allowed to do anything.
    */
    if ((FDL_HkTelemetryPkt.TransferInProgress == TRUE) && (FDL_HkTelemetryPkt.TransferPaused == FALSE))
    {
        MoreData = TRUE;
        FileComplete = FALSE;
    }
    else
    {
        MoreData = FALSE;
        FileComplete = FALSE;
    }

    /*
    ** While allowed to output a packet keep outputting packets.
    */
    while (MoreData == TRUE)
    {
        if (OS_CountSemTimedWait ( FDL_SemID, 0 ) == OS_SUCCESS)
        {
            /*
            ** Init entire packet from scatch and fill out all entries.
            ** SWB will handle sequence count.
            */
            CFE_SB_InitMsg(&FDL_FileSegmentPkt, FDL_APP_SEGMENT_MID, sizeof(FDL_FileSegment_t), TRUE);
            CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &FDL_FileSegmentPkt);

            /*
            ** Set segment number in file packet and HK.
            */
            FDL_FileSegmentPkt.SegmentNumber = FDL_HkTelemetryPkt.LastSegmentSent + 1;
            FDL_HkTelemetryPkt.LastSegmentSent++;

            /* 
            ** Set data size in this file packet.
            ** If not enough data left then cap at remaining data.
            */
            FDL_FileSegmentPkt.SegmentDataSize = FDL_HkTelemetryPkt.SegmentSize;
            if ( FDL_FileSegmentPkt.SegmentDataSize > FDL_HkTelemetryPkt.CurrentTransferSize - FDL_HkTelemetryPkt.BytesTransferred )
            {
                FDL_FileSegmentPkt.SegmentDataSize = FDL_HkTelemetryPkt.CurrentTransferSize - FDL_HkTelemetryPkt.BytesTransferred;
                MoreData = FALSE;  /* This is the last packet */
                FileComplete = TRUE;  /* Cleanup after last packet is sent */
            }
            FDL_HkTelemetryPkt.BytesTransferred += FDL_FileSegmentPkt.SegmentDataSize;

            /*
            ** Check if this is last data segment.
            */
            if ( FDL_HkTelemetryPkt.BytesTransferred == FDL_HkTelemetryPkt.CurrentTransferSize )
            {
                MoreData = FALSE;
                FileComplete = TRUE;  /* Cleanup after last packet is sent */
            }

            /*
            ** Now get the file data.
            */
            bytesRead = OS_read(FDL_HkTelemetryPkt.CurrentFileFd, FDL_FileSegmentPkt.SegmentData, FDL_FileSegmentPkt.SegmentDataSize);

            if ( bytesRead == FDL_FileSegmentPkt.SegmentDataSize )
            {
                FDL_HkTelemetryPkt.CurrentFileCrc = CFE_ES_CalculateCRC(FDL_FileSegmentPkt.SegmentData, bytesRead, FDL_HkTelemetryPkt.CurrentFileCrc, CFE_ES_DEFAULT_CRC);
                /*
                ** Update CCSDS header packet length to match actual data output and send packet.
                */
                Length = sizeof(FDL_FileSegment_t) - FDL_MAX_SEGMENT_SIZE + FDL_FileSegmentPkt.SegmentDataSize;
                CFE_SB_SetTotalMsgLength ((CFE_SB_Msg_t *) &FDL_FileSegmentPkt, Length);
                CFE_SB_SendMsg((CFE_SB_Msg_t *) &FDL_FileSegmentPkt);
            }
            else
            {
                /*
                ** OS_read failed.  Nothing we can do about it, except abort file downlink.
                */
                MoreData = FALSE;
                OS_close(FDL_HkTelemetryPkt.CurrentFileFd);
                FDL_HkTelemetryPkt.TransferInProgress = FALSE;
                FDL_HkTelemetryPkt.TransferPaused = FALSE;
                FDL_SendCompleteFileDownlinkTlmPacket ( );
                CFE_EVS_SendEvent(FDL_XFERCOMPLETE_ERR_EID, CFE_EVS_ERROR, "FDL: File Transfer read error, aborted, %s", FDL_HkTelemetryPkt.CurrentFileName);
            }
        }
        else
        {
            /*
            ** Timeout or error, either way we are done.
            */
            MoreData = FALSE;
        }
    }

    /*
    ** If file just completed then cleanup HK
    */
    if ( FileComplete == TRUE )
    {
        OS_close(FDL_HkTelemetryPkt.CurrentFileFd);
        FDL_HkTelemetryPkt.TransferInProgress = FALSE;
        FDL_HkTelemetryPkt.TransferPaused = FALSE;
        FDL_SendCompleteFileDownlinkTlmPacket ( );
        CFE_EVS_SendEvent(FDL_XFERCOMPLETE_INF_EID, CFE_EVS_INFORMATION, "FDL: File Transfer Completed, %s", FDL_HkTelemetryPkt.CurrentFileName);
    }

} /* End of FDL_TransferFile() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_CancelFileTransfer                                             */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function cancels a file transfer. Only one transfer can be    */
/*         active at a time.                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_CancelFileTransfer(void)
{

    /*
    ** It is not considered an error if no file is in progress.
    ** This allows a cleanup of the flags should the need arise.
    */
    if ( FDL_HkTelemetryPkt.TransferInProgress == TRUE )
    {
        /*
        ** But can only Close the file it a transfer was in progress.
        */
        OS_close(FDL_HkTelemetryPkt.CurrentFileFd);
    }
    FDL_HkTelemetryPkt.TransferInProgress = FALSE;
    FDL_HkTelemetryPkt.TransferPaused = FALSE;

    FDL_HkTelemetryPkt.fdl_command_count++;
    CFE_EVS_SendEvent(FDL_CANCELXFER_INF_EID, CFE_EVS_INFORMATION, "FDL: File Transfer Cancelled");

    /* TODO: Send cancel command to Cadet Application */

} /* End of FDL_CancelFileTransfer() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_PauseFileTransfer                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function pauses the file transfer.                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_PauseFileTransfer(void)
{

    /*
    ** Can only pause if a transfer is active.
    */
    if ( FDL_HkTelemetryPkt.TransferInProgress == TRUE )
    {
        FDL_HkTelemetryPkt.TransferPaused = TRUE;
        FDL_HkTelemetryPkt.fdl_command_count++;
        CFE_EVS_SendEvent(FDL_PAUSEXFER_INF_EID, CFE_EVS_INFORMATION, "FDL: File Transfer Paused");
    }
    else
    {
        FDL_HkTelemetryPkt.fdl_command_error_count++;
        CFE_EVS_SendEvent(FDL_PAUSEXFER_ERR_EID, CFE_EVS_ERROR, "FDL: Can't Pause-> No Transfer In Progress");
    }
    
} /* End of FDL_PauseFileTransfer() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  FDL_ResumeFileTransfer                                             */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resumes the paused file transfer.                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FDL_ResumeFileTransfer(void)
{

    /*
    ** Can only resume if transfer was paused.
    */
    if ( FDL_HkTelemetryPkt.TransferPaused == TRUE )
    {
        FDL_HkTelemetryPkt.TransferPaused = FALSE;
        FDL_HkTelemetryPkt.fdl_command_count++;
        CFE_EVS_SendEvent(FDL_RESUMEXFER_INF_EID, CFE_EVS_INFORMATION, "FDL: File Transfer Resumed");
    }
    else
    {
        FDL_HkTelemetryPkt.fdl_command_error_count++;
        CFE_EVS_SendEvent(FDL_RESUMEXFER_ERR_EID, CFE_EVS_ERROR, "FDL: Can't Resume-> No Paused Transfer Found");
    }
    
} /* End of FDL_ResumeFileTransfer() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* FDL_VerifyCmdLength() -- Verify command packet length                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
boolean FDL_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
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
        
        CFE_EVS_SendEvent(FDL_LEN_ERR_EID, CFE_EVS_ERROR,
                          "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
                          MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        FDL_HkTelemetryPkt.fdl_command_error_count++;
    }
    
    return(result);
    
} /* End of FDL_VerifyCmdLength() */

/************************/
/*  End of File Comment */
/************************/
