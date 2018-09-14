/*
** File: cadet_app.c
**
** Purpose:
**     This file contains the source code for the CADET Application main routine, initialization routine, and 
**     ground command functions.
**
*/



/*
**   Includes:
*/
#include "cfe.h"
#include "cadet_app.h"
#include "cadet_hk.h"
#include "cadet_output.h"
#include "cadet_cds.h"
#include "cadet_fltrtbl.h"
#include "cadet_input.h"
#include "cadet_radio.h"
#include "cadet_msg.h"
#include "cadet_perfids.h"
#include "cadet_events.h"
#include "cadet_version.h"
#include "mission_msgids.h"
#include "dhl_lib.h"



/*
** Defines
*/



/*
** Structures
*/



/*
** Exported Variables
*/
CFE_SB_PipeId_t          CADET_TlmPipe;  
CFE_SB_PipeId_t          CADET_FdlPipe;  
uint32                   CADET_FileDownlinkSemaphore = 0;
uint16                   CADET_HighFifoPacketsToAutoPlayback_Temp;



/*
** Local Variables (could be defined static)
*/
CFE_SB_PipeId_t          CADET_WakeupPipe;
CFE_SB_PipeId_t          CADET_CommandPipe;



/*
** Exported Functions
*/



/*
** Local Functions (could be defined static)
*/
void CADET_ProcessMainPipe ( CFE_SB_MsgPtr_t MsgPtr );
void CADET_ExecLocalCommand ( CFE_SB_MsgPtr_t cmd );
void CADET_ResetCadetCmd( void );



/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/



/*
** Function: CADET_AppMain
**
** Purpose: Main loop processing for the cadet application.
**
*/
void CADET_AppMain ( void )
{
    int32            Result;
    uint32           RunStatus = CFE_ES_APP_RUN;
    CFE_SB_MsgPtr_t  MsgPtr;

    /*
    ** Performance Log (start time counter)...
    */
    CFE_ES_PerfLogEntry ( CADET_APP_PERF_ID );

    /*
    ** Register the app with Executive services
    */
    Result = CFE_ES_RegisterApp ( );
    
    /*
    ** Perform application specific initialization...
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CADET_AppInit ( );
    }

    /*
    ** Check for start-up error...
    ** If negative, set request to terminate main loop...
    */
    if ( Result != CFE_SUCCESS )
    {
        RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** CADET App Main loop
    */
    while ( CFE_ES_RunLoop ( &RunStatus ) == TRUE )
    {
        /*
        ** Performance Log (stop time counter)...
        ** Wait for wakeup message...
        **     No error checking.  Assuming SCH/SWB work correctly.
        ** Performance Log (start time counter)...
        */
        CFE_ES_PerfLogExit ( CADET_APP_PERF_ID );
        (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_PEND_FOREVER );
        CFE_ES_PerfLogEntry ( CADET_APP_PERF_ID );

        /*
        ** Check for Cadet Radio Status Response Timeout.
        */
        CADET_HkTelemetryPkt.StatusResponseTimeout++;
        if ( CADET_HkTelemetryPkt.StatusResponseTimeout >= CADET_RADIO_STATUS_TIMEOUT )
        {
            /*
            ** This counter is reset back to 0 with the receipt of each cadet radio packet.
            ** If it reaches CADET_RADIO_STATUS_TIMEOUT then a major problem has occurred.
            */
            printf ( "\n\n\n\n\n\n\n\n\n\nRESETTING THE CADET RADIO\n\n\n\n\n\n\n\n\n" );
            CADET_HkTelemetryPkt.StatusResponseTimeout = 0;
            CADET_ResetCadetCmd ( );
        }

        /*
        ** Output data to radio if state is free
        */
        CADET_Output_TransmitDataToRadio ( );

        /*
        ** Process 1 application command from input pipe.
        ** NOTE: This could put application into or out of interleaving mode.
        */
        Result = CFE_SB_RcvMsg ( &MsgPtr, CADET_CommandPipe, CFE_SB_POLL );
        if ( Result == CFE_SUCCESS )
        {
            CADET_ProcessMainPipe ( MsgPtr );
        }

    }

    /*
    ** Send an event describing the reason for the termination...
    */
    CFE_EVS_SendEvent ( CADET_EXIT_ERR_EID, CFE_EVS_CRITICAL, "L%d CADET: Application terminating, RunStatus = 0x%08X, Result = 0x%08X", __LINE__, RunStatus, Result );

    /*
    ** In case cFE Event Services is not working...
    */
    CFE_ES_WriteToSysLog ( "CADET Application terminating, RunStatus = 0x%08X, Result = 0x%08X", RunStatus, Result );
  
    /*
    ** Performance Log (stop time counter)...
    */
    CFE_ES_PerfLogExit ( CADET_APP_PERF_ID );

    /*
    ** Let cFE kill the application (and any child tasks)...
    */
    CFE_ES_ExitApp ( RunStatus );
}



/*
** Function: CADET_AppInit
**
** Purpose: Initailizes the cadet application, including external modules.
**
*/
int32 CADET_AppInit ( void )
{
    int32              Result = CFE_SUCCESS;
    boolean            CriticalDataStorePresent = FALSE;

    /*
    ** Register the events
    */ 
    Result = CFE_EVS_Register ( NULL, 0, 0 );
    if ( Result != CFE_SUCCESS )
    {
        /* 
        ** Can't send event as registering with event services failed.
        */
        CFE_ES_WriteToSysLog ( "CADET unable to register for EVS services, err = 0x%08X", Result );
    }

    /*
    ** Create the Software Bus command pipe for the wakeup messages
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CFE_SB_CreatePipe ( &CADET_WakeupPipe, CADET_CMD_PIPE_DEPTH, "CADET_WAKEUP_SWPIPE" );
        if ( Result != CFE_SUCCESS )
        {
            CFE_EVS_SendEvent ( CADET_CMDPIPE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Unable to create wakeup pipe, err = 0x%08X", __LINE__, Result );
        }
    }

    /* 
    ** Subscriptions for the wakeup commands on wakeup pipe
    **     CADET Application Wakeup requests
    **     But only if creation of command pipe was successful.
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CFE_SB_Subscribe ( CADET_APP_WAKEUP_MID, CADET_WakeupPipe );
        if ( Result != CFE_SUCCESS )
        {
            CFE_EVS_SendEvent ( CADET_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Can't subscribe to stream 0x%x status %i", __LINE__, CADET_APP_WAKEUP_MID, Result );
        }
    }

    /*
    ** Create the Software Bus command pipe for the local commands
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CFE_SB_CreatePipe ( &CADET_CommandPipe, CADET_CMD_PIPE_DEPTH, "CADET_CMD_SWPIPE" );
        if ( Result != CFE_SUCCESS )
        {
            CFE_EVS_SendEvent ( CADET_CMDPIPE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Unable to create input pipe, err = 0x%08X", __LINE__, Result );
        }
    }

    /* 
    ** Subscriptions for the local commands on command pipe
    **     CADET Application Commands
    **     CADET Application HK requests
    **     But only if creation of command pipe was successful.
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CFE_SB_Subscribe ( CADET_APP_CMD_MID, CADET_CommandPipe );
        if ( Result != CFE_SUCCESS )
        { 
            CFE_EVS_SendEvent ( CADET_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Can't subscribe to stream 0x%x status %i", __LINE__, CADET_APP_CMD_MID, Result );
        }
    }

    if ( Result == CFE_SUCCESS )
    {
        Result = CFE_SB_Subscribe ( CADET_APP_SEND_HK_MID, CADET_CommandPipe );
        if ( Result != CFE_SUCCESS )
        {
            CFE_EVS_SendEvent ( CADET_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Can't subscribe to stream 0x%x status %i", __LINE__, CADET_APP_SEND_HK_MID, Result );
        }
    }

    /* 
    ** Create CADET FIFO Telemetry Pipe 
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CFE_SB_CreatePipe ( &CADET_TlmPipe, CADET_FIFO_PIPE_DEPTH, "CADET_FIFO_PIPE" );
        if ( Result != CFE_SUCCESS )
        {
            CFE_EVS_SendEvent ( CADET_TLMPIPE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Can't create fifo pipe status %i", __LINE__, Result );
        }
    }

    /* 
    ** Subscriptions for FIFO Telemetry Pipe
    */
    /* Done when loading filter table */

    /* 
    ** Create CADET FDL Telemetry Pipe 
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CFE_SB_CreatePipe ( &CADET_FdlPipe, CADET_FDL_PIPE_DEPTH, "CADET_FDL_PIPE" );
        if ( Result != CFE_SUCCESS )
        {
            CFE_EVS_SendEvent ( CADET_TLMPIPE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Can't create fdl pipe status %i", __LINE__, Result );
        }
    }

    /* 
    ** Subscriptions for FDL Telemetry Pipe
    */
    /* Done when loading filter table */

    /*
    ** Create CADET/FDL Counting Semaphore.
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = OS_CountSemCreate ( &CADET_FileDownlinkSemaphore, CADET_FDL_HANDSHAKE_SEM_NAME, CADET_FDL_HANDSHAKE_SEM_VALUE, 0 );
        if ( Result != CFE_SUCCESS )
        {        
            CFE_EVS_SendEvent ( CADET_CREATE_SEM_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Can't create FDL semaphore, status %i", __LINE__, Result );
        }
    }

    /*
    **  Setup and access available Critical Data Store values, or just init to default values
    **      NOTE: AccessCriticalDataStore must happen prior to AppTableInit.
    */
    CriticalDataStorePresent = CADET_CDS_AccessCriticalDataStore ( );
 
    /*
    ** Initialize application tables...
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CADET_AppTableInit ( );
    }

    /*
    ** Initialize Input Processing from radio.
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CADET_Input_Initialize ( );
    }

    /*
    ** Initialize Output Processing to radio.
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CADET_Output_Initialize ( );
    }

    /*
    ** Init the housekeeping telemetry packet.
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CADET_HK_InitializeHousekeepingPacket ( );
    }

    /*
    ** Init cadet status response timeout to 0 seconds (will expire if reaches 60).
    */
    CADET_HkTelemetryPkt.StatusResponseTimeout = 0;

    /*
    ** Init cadet high fifo packets to auto playback (must happen after init HK packet)
    */
    CADET_HighFifoPacketsToAutoPlayback_Temp = CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback;

    /*
    ** Send startup event message if initialization successful.
    */
    if ( Result == CFE_SUCCESS )
    {
        if ( CriticalDataStorePresent == TRUE )
        {
            CFE_EVS_SendEvent ( CADET_STARTUP_INF_EID, CFE_EVS_INFORMATION, "L%d CADET: App Initialized using CDS. Version %d.%d.%d.%d", __LINE__, 
                                CADET_APP_MAJOR_VERSION, CADET_APP_MINOR_VERSION, CADET_APP_REVISION, CADET_APP_MISSION_REV);
        }
        else
        {
            CFE_EVS_SendEvent ( CADET_STARTUP_INF_EID, CFE_EVS_INFORMATION, "L%d CADET: App Initialized using DEFAULTS. Version %d.%d.%d.%d", __LINE__, 
                                CADET_APP_MAJOR_VERSION, CADET_APP_MINOR_VERSION, CADET_APP_REVISION, CADET_APP_MISSION_REV);
        }
    }
		
    return ( Result );
}




/*
** Function: CADET_ProcessMainPipe
**
** Purpose: Process the input pipe with ground commands and housekeeping requests.
**
*/
void CADET_ProcessMainPipe ( CFE_SB_MsgPtr_t MsgPtr )
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId ( MsgPtr );

    switch ( MsgId )
    {
         case CADET_APP_CMD_MID:
              CADET_ExecLocalCommand ( MsgPtr );
              break;

         case CADET_APP_SEND_HK_MID:
              CADET_HK_SendHousekeepingPacket ( );
              CADET_GetFilterTableData ( );
              break;

         default:
              CADET_HkTelemetryPkt.command_error_count++;
              CFE_EVS_SendEvent ( CADET_COMMAND_ERR_EID, CFE_EVS_ERROR, "L%d CADET: invalid command packet,MID = 0x%x", __LINE__, MsgId ); 
              break;
    }
}



/*
** Function: CADET_VerifyCmdLength
**
** Purpose: Verify the packet length matches the expected length.
**     NOTE: This function reports the error if the lengths do not match, not the calling function.
**
*/
boolean CADET_VerifyCmdLength ( CFE_SB_MsgPtr_t msg, uint16 ExpectedLength )
{     
    boolean result = TRUE;

    uint16 ActualLength = CFE_SB_GetTotalMsgLength ( msg );

    /*
    ** Verify the command packet length.
    */
    if ( ExpectedLength != ActualLength )
    {
        CFE_SB_MsgId_t MessageID   = CFE_SB_GetMsgId ( msg );
        uint16         CommandCode = CFE_SB_GetCmdCode ( msg );

        CFE_EVS_SendEvent ( CADET_LEN_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d", __LINE__, MessageID, CommandCode, ActualLength, ExpectedLength );
        result = FALSE;
        CADET_HkTelemetryPkt.command_error_count++;
    }

    return ( result );
}



/*
** Function: CADET_ResetCounters
**
** Purpose: Reset selected housekeeping counters to 0.
**
*/
void CADET_ResetCounters ( void )
{
    CADET_HkTelemetryPkt.command_count       = 0;
    CADET_HkTelemetryPkt.command_error_count = 0;

    CADET_HkTelemetryPkt.TableVerifySuccessCount = 0;
    CADET_HkTelemetryPkt.TableVerifyFailureCount = 0;

    CADET_HkTelemetryPkt.HighFifoPacketsDropped = 0;
    CADET_HkTelemetryPkt.LowFifoPacketsDropped  = 0;

    CADET_HkTelemetryPkt.RadioData.cadet_high_bytes_sent = 0;

    CADET_HkTelemetryPkt.RadioData.cadet_low_bytes_sent = 0;

    CADET_HkTelemetryPkt.RadioData.cadet_command_packets_sent = 0;

    CADET_HkTelemetryPkt.RadioData.cadet_result_response_packets_received = 0;
    CADET_HkTelemetryPkt.RadioData.cadet_version_packets_received = 0;
    CADET_HkTelemetryPkt.RadioData.cadet_status_packets_received = 0;
    CADET_HkTelemetryPkt.RadioData.cadet_configuration_packets_received = 0;
    CADET_HkTelemetryPkt.RadioData.cadet_relay_packets_received = 0;
    CADET_HkTelemetryPkt.RadioData.cadet_error_packets_received = 0;

    CFE_EVS_SendEvent ( CADET_COMMANDRST_DBG_EID, CFE_EVS_DEBUG, "L%d CADET: RESET counters command", __LINE__ );

}



/*
** Function: CADET_TakeSemaphore
**
** Purpose: Diagnostic feature.  The CADET/FDL handshake is setup
**     to handle 50 outstanding telemetry packets.  If for some
**     reason this is to many the CADET application can be
**     commanded to take some of the semaphores to reduce the
**     number of outstanding packets.  Obviously it can't be
**     reduced to 0.  Alternatively, if there is a handshake error
**     the CADET application can be commanded to give a semaphore.
**     The outstanding number cannot exceed the pipe depth.
**
*/
void CADET_TakeSemaphore ( void )
{
    int32 Status;

    Status = OS_CountSemTimedWait ( CADET_FileDownlinkSemaphore, 0 );  /* No wait */

    if ( Status == OS_SUCCESS )
    {
        CADET_HkTelemetryPkt.command_count++;
        CFE_EVS_SendEvent ( CADET_TAKESEMAPHORE_INF_EID, CFE_EVS_INFORMATION, "L%d CADET: Semaphore Taken", __LINE__ );
    }
    else
    {
        CADET_HkTelemetryPkt.command_error_count++;
        CFE_EVS_SendEvent ( CADET_TAKESEMAPHORE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Error taking Semaphore, status = %i", __LINE__, Status );
    }

}



/*
** Function: CADET_GiveSemaphore 
**
** Purpose: Diagnostic feature.  The CADET/FDL handshake is setup
**     to handle 50 outstanding telemetry packets.  If for some
**     reason this is to many the CADET application can be
**     commanded to take some of the semaphores to reduce the
**     number of outstanding packets.  Obviously it can't be
**     reduced to 0.  Alternatively, if there is a handshake error
**     the CADET application can be commanded to give a semaphore.
**     The outstanding number cannot exceed the pipe depth.
**
*/
void CADET_GiveSemaphore ( void )
{
    int32 Status;

    Status = OS_CountSemGive ( CADET_FileDownlinkSemaphore );

    if ( Status == OS_SUCCESS )
    {
        CADET_HkTelemetryPkt.command_count++;
        CFE_EVS_SendEvent ( CADET_GIVESEMAPHORE_INF_EID, CFE_EVS_INFORMATION, "L%d CADET: Semaphore Given", __LINE__ );
    }
    else
    {
        CADET_HkTelemetryPkt.command_error_count++;
        CFE_EVS_SendEvent ( CADET_GIVESEMAPHORE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Error giving Semaphore, status = %i", __LINE__, Status );
    }

}



/*
** Function: CADET_JamFilterCmd
**
** Purpose: The filter tables are normally loaded via table operations.
**     However, this command gives the option to change one entry in the table.
**
*/
void CADET_JamFilterCmd ( CFE_SB_MsgPtr_t  MessagePtr )
{
    CADET_JamFilterCmd_t* JamFilterCmd  =  (CADET_JamFilterCmd_t * ) MessagePtr;
    CFE_SB_PipeId_t       TlmPipe = CADET_INVALID_PIPE;
    boolean               ValidCommand   = FALSE;
    uint16                AppID;

    if ( ( JamFilterCmd->MessageID  < DELLINGR_MIN_TLM_MID ) || ( JamFilterCmd->MessageID  > DELLINGR_MAX_TLM_MID ) )
    {
        /*
        ** Invalid Message ID received
        */
        CFE_EVS_SendEvent ( CADET_JAM_FILTER_CMD_ERR_EID, CFE_EVS_ERROR,  "L%d CADET: Jam Filter command: invalid argument, Message ID = x%04x", __LINE__, JamFilterCmd->MessageID );
    }
    else if ( ( JamFilterCmd->Method != CADET_TIME_BASED_FILTER_TYPE ) && ( JamFilterCmd->Method != CADET_SEQUENCE_BASED_FILTER_TYPE ) )
    {
        /*
        ** Invalid Filter Method received
        */
        CFE_EVS_SendEvent ( CADET_JAM_FILTER_CMD_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Jam Filter command: invalid argument, Filter Method = %d", __LINE__, JamFilterCmd->Method );
    }
    else if ( ( JamFilterCmd->Pipe != FILTER_HI_FIFO ) && ( JamFilterCmd->Pipe != FILTER_LO_FIFO ) && ( JamFilterCmd->Pipe != FILTER_BOTH_FIFO ) )
    {
        /*
        ** Invalid Destination Pipe received
        */
        CFE_EVS_SendEvent ( CADET_JAM_FILTER_CMD_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Jam Filter command: invalid argument, Destination Pipe = %d", __LINE__, JamFilterCmd->Pipe );
    }
    else if ( ( JamFilterCmd->ValueN > JamFilterCmd->ValueX ) || ( ( JamFilterCmd->ValueO >= JamFilterCmd->ValueX ) && ( JamFilterCmd->ValueX  != 0) ) )
    {
        /*
        ** Invalid Filter values received
        */
        CFE_EVS_SendEvent ( CADET_JAM_FILTER_CMD_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Jam Filter command: invalid argument, Filter values N = %d X = %d O = %d", __LINE__, 
                            JamFilterCmd->ValueN, JamFilterCmd->ValueX, JamFilterCmd->ValueO );
    }
    else  /** Command appears good ***/
    {
        AppID = CCSDS_SID_APID(JamFilterCmd->MessageID);

        /* Pipe was validated above */
        if ( JamFilterCmd->Pipe == FILTER_BOTH_FIFO )
        {
            TlmPipe = FILTER_HI_FIFO;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].FilterMethod = JamFilterCmd->Method;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].N            = JamFilterCmd->ValueN;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].X            = JamFilterCmd->ValueX;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].O            = JamFilterCmd->ValueO;
            TlmPipe = FILTER_LO_FIFO;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].FilterMethod = JamFilterCmd->Method;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].N            = JamFilterCmd->ValueN;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].X            = JamFilterCmd->ValueX;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].O            = JamFilterCmd->ValueO;
        }
        else
        {
            TlmPipe = JamFilterCmd->Pipe;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].FilterMethod = JamFilterCmd->Method;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].N            = JamFilterCmd->ValueN;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].X            = JamFilterCmd->ValueX;
            CADET_FilterTablePtr->AppID[AppID].Fifo[TlmPipe].O            = JamFilterCmd->ValueO;
        }
 
        ValidCommand  = CADET_SubscribeToPacket ( AppID, CADET_FilterTablePtr->AppID[AppID].Fifo[FILTER_HI_FIFO].N, CADET_FilterTablePtr->AppID[AppID].Fifo[FILTER_HI_FIFO].X, 
                                                  CADET_FilterTablePtr->AppID[AppID].Fifo[FILTER_LO_FIFO].N, CADET_FilterTablePtr->AppID[AppID].Fifo[FILTER_LO_FIFO].X );

        /* 
        ** Tell Table Services that things have been updated 
        */
        CFE_TBL_Modified( CADET_FilterTableHandle );

    }
    
    if ( ValidCommand == TRUE )
    {
        CFE_EVS_SendEvent ( CADET_JAM_FILTER_CMD_DBG_EID, CFE_EVS_DEBUG, "L%d CADET: Jam Filter command (MsgID=%x Method =%x X=%d N=%d O=%d)", __LINE__, 
                            JamFilterCmd->MessageID, JamFilterCmd->Method, JamFilterCmd->ValueX, JamFilterCmd->ValueN,JamFilterCmd->ValueO );
        CADET_HkTelemetryPkt.command_count++;
    }
    else
    {
        CADET_HkTelemetryPkt.command_error_count++;
    }
}



/*
** Function: CADET_StartPlayback
**
** Purpose: Start a FIFO request of the specified FIFO.
**
*/
void CADET_StartPlayback ( CFE_SB_MsgPtr_t  MessagePtr )
{
    CADET_StartPlaybackCmd_t   *PktPtr = (CADET_StartPlaybackCmd_t *) MessagePtr;
    uint32                     PktsOnFifo;

    /*
    ** Can only start playback when not in playback mode.
    */
    if ( CADET_HkTelemetryPkt.Mode == MODE_OPEN )
    {
        /* 
        ** The playback packets per request can't be zero, otherwise we'll enter infinite loop trying to playback 0 packets each time.
        ** The total packets to send can't be zero either, cause first packet to radio would request 0 packets and we'd timeout cause radio ignores command.
        */
        if ( ( PktPtr->PlaybackPktsPerRequest != 0 ) && ( PktPtr->PlaybackTotalPacketsToSend != 0 ) )
        {
            if ( ( ( ( PktPtr->PlaybackFifo & FIFO_REQUEST_HI_FIFO ) == FIFO_REQUEST_HI_FIFO ) ) ||
                 ( ( ( CADET_HkTelemetryPkt.StatusInfo.LoFifoUsage / CADET_FLASH_PACKET_SIZE ) >  0 ) && ( ( PktPtr->PlaybackFifo & FIFO_REQUEST_LO_FIFO ) == FIFO_REQUEST_LO_FIFO ) ) )
            {
                CADET_HkTelemetryPkt.Mode = MODE_INTERLEAVING;
                CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;
                CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_PLAYBACK;

                /* Set playback parameters */
                CADET_HkTelemetryPkt.PlaybackFifo = PktPtr->PlaybackFifo;
                CADET_HkTelemetryPkt.PlaybackPktsPerRequest = PktPtr->PlaybackPktsPerRequest;
                CADET_HkTelemetryPkt.PlaybackNextStartingPacket = 1; /* start with first packet to output, starts with 1, not 0 */
                CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend = PktPtr->PlaybackTotalPacketsToSend;
                CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend = CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend; /* Start with all to send */

                /* Clear flush parameters */
                CADET_HkTelemetryPkt.FlushFifo = 0;
                CADET_HkTelemetryPkt.FlushPktsPerRequest = 0;
                CADET_HkTelemetryPkt.FlushTotalPacketsToClear = 0;
                CADET_HkTelemetryPkt.FlushPacketsLeftToClear = 0;

                CADET_HkTelemetryPkt.command_count++;
           
                /*
                ** Cannot request more data than is on the FIFO.  
                ** Radio does not generate response if no data is sent to the ground.
                ** The timeout waiting for the response will cause problems if the radio continuously doesn't send responses.
                */
                if ( ( CADET_HkTelemetryPkt.PlaybackFifo & FIFO_REQUEST_HI_FIFO ) == FIFO_REQUEST_HI_FIFO )
                {
                    PktsOnFifo = ( CADET_HkTelemetryPkt.StatusInfo.HiFifoUsage / CADET_FLASH_PACKET_SIZE ) + 1;  /* Add one to get partial HI FIFO packet. */
                }
                else
                {
                    PktsOnFifo = CADET_HkTelemetryPkt.StatusInfo.LoFifoUsage / CADET_FLASH_PACKET_SIZE;
                }

                if ( CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend > PktsOnFifo )
                {
                    CFE_EVS_SendEvent ( CADET_PLAYBACK_PARAMETER_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Packets to send %d more than FIFO has.  Limited to what FIFO has %d.", __LINE__, 
                                        CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend, PktsOnFifo );
                    CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend = PktsOnFifo;
                    CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend = CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend; /* Start with all to send */
                }
            }
            else
            {
                CFE_EVS_SendEvent ( CADET_PLAYBACK_PARAMETER_ERR_EID, CFE_EVS_ERROR, "L%d CADET: No Packets on FIFO.  FIFO = %d, HiPKts = %d, LoPkts = %d", __LINE__,  
                                    PktPtr->PlaybackFifo, (CADET_HkTelemetryPkt.StatusInfo.HiFifoUsage/CADET_FLASH_PACKET_SIZE), (CADET_HkTelemetryPkt.StatusInfo.LoFifoUsage/CADET_FLASH_PACKET_SIZE) );
                CADET_HkTelemetryPkt.command_error_count++;
            }
        }
        else
        {
            CFE_EVS_SendEvent ( CADET_PLAYBACK_PARAMETER_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Invalid parameter.  PktsPerRequest = %d, PktsToSend = %d", __LINE__,  
                                PktPtr->PlaybackPktsPerRequest, PktPtr->PlaybackTotalPacketsToSend );
            CADET_HkTelemetryPkt.command_error_count++;
        }
    }
    else
    {
        CFE_EVS_SendEvent ( CADET_PLAYBACK_IN_PROGRESS_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Error, playback already in progress.", __LINE__ );
        CADET_HkTelemetryPkt.command_error_count++;
    }

    #ifdef _CADET_DEBUG_
        printf ( "CADET CMD: FIFO = %d, PktperReq = %d, PkttoSend = %d\n", (int)PktPtr->PlaybackFifo, (int)PktPtr->PlaybackPktsPerRequest, (int)PktPtr->PlaybackTotalPacketsToSend );
    #endif
}



/*
** Function: CADET_AbortPlaybackFlush
**
** Purpose: Abort a playback or flush of the cadet fifo.
**     NOTE: While the transmitter is on, commanding won't be possible.  Ideally if there is a problem then the transmitter won't be on.
**
*/
void CADET_AbortPlaybackFlush ( void )
{

    CADET_HkTelemetryPkt.Mode = MODE_OPEN;
    CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;
    CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_NONE;

    /* Clear playback parameters */
    CADET_HkTelemetryPkt.PlaybackFifo = 0;
    CADET_HkTelemetryPkt.PlaybackPktsPerRequest = 0;
    CADET_HkTelemetryPkt.PlaybackNextStartingPacket = 0;
    CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend = 0;
    CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend = 0; 

    CADET_HkTelemetryPkt.PlaybackNextStartingPacketHi = 0;

    /* Clear flush parameters */
    CADET_HkTelemetryPkt.FlushFifo = 0;
    CADET_HkTelemetryPkt.FlushPktsPerRequest = 0;
    CADET_HkTelemetryPkt.FlushTotalPacketsToClear = 0;
    CADET_HkTelemetryPkt.FlushPacketsLeftToClear = 0;

    CADET_PlaybackTimeout = 1;  /* Must be greater than 0 */

    CADET_HkTelemetryPkt.command_count++;
    CFE_EVS_SendEvent ( CADET_ABORTPLAYBACK_INF_EID, CFE_EVS_INFORMATION, "L%d CADET: Abort playback of cadet fifo", __LINE__ );

}



/*
** Function: CADET_FlushFifo
**
** Purpose: Start a FIFO flush of the specified FIFO.
**
*/
void CADET_FlushFifo ( CFE_SB_MsgPtr_t  MessagePtr )
{
    CADET_FlushFifoCmd_t   *PktPtr = (CADET_FlushFifoCmd_t *) MessagePtr;

    /*
    ** Can only start playback when not in playback mode.
    */
    if ( CADET_HkTelemetryPkt.Mode == MODE_OPEN )
    {
        /* 
        ** The flush packets per request can't be zero, otherwise we'll enter infinite loop trying to flush 0 packets each time.
        ** The total packets to clear can't be zero either, cause first packet to radio would clear 0 packets and we'd timeout cause radio ignores command.
        */
        if ( ( PktPtr->FlushPktsPerRequest != 0 ) && ( PktPtr->FlushTotalPacketsToClear != 0 ) )
        {
            CADET_HkTelemetryPkt.Mode = MODE_INTERLEAVING;
            CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;
            CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_FLUSH;

            /* Set flush parameters */
            CADET_HkTelemetryPkt.FlushFifo = PktPtr->FlushFifo;
            CADET_HkTelemetryPkt.FlushPktsPerRequest = PktPtr->FlushPktsPerRequest;
            CADET_HkTelemetryPkt.FlushTotalPacketsToClear = PktPtr->FlushTotalPacketsToClear;
            CADET_HkTelemetryPkt.FlushPacketsLeftToClear = CADET_HkTelemetryPkt.FlushTotalPacketsToClear; /* Start with all to clear */

            /* Clear playback parameters */
            CADET_HkTelemetryPkt.PlaybackFifo = 0;
            CADET_HkTelemetryPkt.PlaybackPktsPerRequest = 0;
            CADET_HkTelemetryPkt.PlaybackNextStartingPacket = 0;
            CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend = 0;
            CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend = 0; 

            CADET_HkTelemetryPkt.command_count++;
        }
        else
        {
            CFE_EVS_SendEvent ( CADET_FLUSH_PARAMETER_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Invalid parameter.  PktsPerRequest = %d, PktsToClear = %d", __LINE__, 
                                PktPtr->FlushPktsPerRequest, PktPtr->FlushTotalPacketsToClear );
            CADET_HkTelemetryPkt.command_error_count++;
        }
    }
    else
    {
        CFE_EVS_SendEvent ( CADET_FLUSH_IN_PROGRESS_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Error, flush already in progress.", __LINE__ );
        CADET_HkTelemetryPkt.command_error_count++;
    }

    #ifdef _CADET_DEBUG_
        printf ( "CADET CMD: FIFO = %d, PktperReq = %d, PkttoClear = %d\n", (int)PktPtr->FlushFifo, (int)PktPtr->FlushPktsPerRequest, (int)PktPtr->FlushTotalPacketsToClear );
    #endif
}



/*
** Function: CADET_FlushEntireHiFifo
**
** Purpose: Flush the entire hi FIFO.
**     NOTE: This allows a blind command to flush the entire hi FIFO without knowning the amount of data on the hi FIFO.
**
*/
void CADET_FlushEntireHiFifo ( CFE_SB_MsgPtr_t  MessagePtr )
{
    CADET_FlushEntireFifoHiCmd_t   *PktPtr = (CADET_FlushEntireFifoHiCmd_t *) MessagePtr;

    /*
    ** Can only start playback when not in playback mode.
    */
    if ( CADET_HkTelemetryPkt.Mode == MODE_OPEN )
    {
        /* 
        ** The flush packets per request can't be zero, otherwise we'll enter infinite loop trying to flush 0 packets each time.
        */
        if ( ( PktPtr->FlushPktsPerRequest != 0 ) )
        {
            CADET_HkTelemetryPkt.Mode = MODE_INTERLEAVING;
            CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;
            CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_FLUSH_HI_ALL;

            /* Set flush parameters */
            CADET_HkTelemetryPkt.FlushFifo = FIFO_CLEAR_HIGH;
            CADET_HkTelemetryPkt.FlushPktsPerRequest = PktPtr->FlushPktsPerRequest;
            CADET_HkTelemetryPkt.FlushTotalPacketsToClear = ( CADET_HkTelemetryPkt.StatusInfo.HiFifoUsage / CADET_FLASH_PACKET_SIZE ) + 1;   /* Add 1 to include partial final packet */
            CADET_HkTelemetryPkt.FlushPacketsLeftToClear = CADET_HkTelemetryPkt.FlushTotalPacketsToClear; /* Start with all to clear */

            /* Clear playback parameters */
            CADET_HkTelemetryPkt.PlaybackFifo = 0;
            CADET_HkTelemetryPkt.PlaybackPktsPerRequest = 0;
            CADET_HkTelemetryPkt.PlaybackNextStartingPacket = 0;
            CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend = 0;
            CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend = 0; 

            CADET_HkTelemetryPkt.command_count++;
        }
        else
        {
            CFE_EVS_SendEvent ( CADET_FLUSH_PARAMETER_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Invalid parameter.  PktsPerRequest = %d", __LINE__, PktPtr->FlushPktsPerRequest );
            CADET_HkTelemetryPkt.command_error_count++;
        }
    }
    else
    {
        CFE_EVS_SendEvent ( CADET_FLUSH_IN_PROGRESS_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Error, flush already in progress.", __LINE__ );
        CADET_HkTelemetryPkt.command_error_count++;
    }

    #ifdef _CADET_DEBUG_
        printf ( "CADET CMD: FIFO = %d, PktperReq = %d, PkttoClear = %d\n", (int)CADET_HkTelemetryPkt.FlushFifo, (int)PktPtr->FlushPktsPerRequest, (int)CADET_HkTelemetryPkt.FlushTotalPacketsToClear );
    #endif
}



/*
** Function: CADET_PlaybackAndFlushHiFifo 
**
** Purpose: Playback and Flush the entire hi FIFO.
**     NOTE: This allows a blind command to playback and flush the entire hi FIFO without knowning the amount of data on the hi FIFO.
**
*/
void CADET_PlaybackAndFlushHiFifo ( CFE_SB_MsgPtr_t  MessagePtr )
{
    CADET_PlaybackAndFlushHiFifoCmd_t   *PktPtr = (CADET_PlaybackAndFlushHiFifoCmd_t *) MessagePtr;

    /*
    ** Can only start playback when not in playback mode.
    */
    if ( CADET_HkTelemetryPkt.Mode == MODE_OPEN )
    {
        /* 
        ** The playback/flush packets per request can't be zero, otherwise we'll enter infinite loop trying to playback/flush 0 packets each time.
        */
        if ( ( PktPtr->PktsPerRequest != 0 ) )
        {
            CADET_HkTelemetryPkt.Mode = MODE_INTERLEAVING;
            CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;
            CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_PB_FLUSH_HI;

            /* Set playback parameters */
            CADET_HkTelemetryPkt.PlaybackFifo = FIFO_REQUEST_HI_FIFO;
            CADET_HkTelemetryPkt.PlaybackPktsPerRequest = PktPtr->PktsPerRequest;
            CADET_HkTelemetryPkt.PlaybackNextStartingPacket = 1;   /* start with first packet to output, starts with 1, not 0 */
            CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend = ( CADET_HkTelemetryPkt.StatusInfo.HiFifoUsage / CADET_FLASH_PACKET_SIZE ) + 1;  /* Only playback and flush what we know is there, including partial */
            CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend = CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend; /* Start with all to send */

            /* Set flush parameters */
            CADET_HkTelemetryPkt.FlushFifo = FIFO_CLEAR_HIGH;
            CADET_HkTelemetryPkt.FlushPktsPerRequest = PktPtr->PktsPerRequest;
            CADET_HkTelemetryPkt.FlushTotalPacketsToClear = ( CADET_HkTelemetryPkt.StatusInfo.HiFifoUsage / CADET_FLASH_PACKET_SIZE ) + 1;  /* Only playback and flush what we know is there, including partial */
            CADET_HkTelemetryPkt.FlushPacketsLeftToClear = CADET_HkTelemetryPkt.FlushTotalPacketsToClear; /* Start with all to clear */

            CADET_HkTelemetryPkt.command_count++;
        }
        else
        {
            CFE_EVS_SendEvent ( CADET_FLUSH_PARAMETER_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Invalid parameter.  PktsPerRequest = %d", __LINE__, PktPtr->PktsPerRequest );
            CADET_HkTelemetryPkt.command_error_count++;
        }
    }
    else
    {
        CFE_EVS_SendEvent ( CADET_PLAYBACK_IN_PROGRESS_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Error, playback already in progress.", __LINE__ );
        CADET_HkTelemetryPkt.command_error_count++;
    }

    #ifdef _CADET_DEBUG_
    printf ( "CADET CMD: FIFO = %d, PktperReq = %d, PkttoClear = %d\n", (int)CADET_HkTelemetryPkt.FlushFifo, (int)PktPtr->PktsPerRequest, (int)CADET_HkTelemetryPkt.FlushTotalPacketsToClear );
    #endif
}



/*
** Function: CADET_StartLoFifoPlayback
**
** Purpose: Playback the lo FIFO and intermittenly output some hi FIFO data.
**
*/
void CADET_StartLoFifoPlayback ( CFE_SB_MsgPtr_t  MessagePtr )
{
    CADET_StartLoFifoWithHiFifoPlaybackCmd_t   *PktPtr = (CADET_StartLoFifoWithHiFifoPlaybackCmd_t *) MessagePtr;
    uint32                                     PktsOnFifo;

    /*
    ** Can only start playback when not in playback mode.
    */
    if ( CADET_HkTelemetryPkt.Mode == MODE_OPEN )
    {
        /* 
        ** The flush packets per request can't be zero, otherwise we'll enter infinite loop trying to flush 0 packets each time.
        ** The total packets to clear can't be zero either, cause first packet to radio would clear 0 packets and we'd timeout cause radio ignores command.
        */
        if ( ( PktPtr->PlaybackPktsPerRequest != 0 ) && ( PktPtr->PlaybackTotalPacketsToSend != 0 ) )
        {
            /*
            ** The algorithm assumes a response packet will be sent back from radio.  THis will only happen if the radio has at least 1 packet to request.
            */
            if ( ( CADET_HkTelemetryPkt.StatusInfo.LoFifoUsage / CADET_FLASH_PACKET_SIZE ) > 0 )
            {
                CADET_HkTelemetryPkt.Mode = MODE_INTERLEAVING;
                CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;
                CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_PB_LOW_HI;

                /* Set playback parameters */
                CADET_HkTelemetryPkt.PlaybackFifo = FIFO_REQUEST_LO_FIFO;
                CADET_HkTelemetryPkt.PlaybackPktsPerRequest = PktPtr->PlaybackPktsPerRequest;
                CADET_HkTelemetryPkt.PlaybackNextStartingPacket = 1; /* start with first packet to output, starts with 1, not 0 */
                CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend = PktPtr->PlaybackTotalPacketsToSend;  /* Will be limited to just what radio has */
                CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend = CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend; /* Start with all to send */

                CADET_HkTelemetryPkt.PlaybackNextStartingPacketHi = 1; /* start with first packet to output, starts with 1, not 0 */
                CADET_HkTelemetryPkt.PlaybackNextStartingCycle = 0;

                /* Clear flush parameters */
                CADET_HkTelemetryPkt.FlushFifo = 0;
                CADET_HkTelemetryPkt.FlushPktsPerRequest = 0;
                CADET_HkTelemetryPkt.FlushTotalPacketsToClear = 0;
                CADET_HkTelemetryPkt.FlushPacketsLeftToClear = 0;

                CADET_HkTelemetryPkt.command_count++;
           
                /*
                ** Cannot request more data than is on the FIFO.  
                ** Radio does not generate response if no data is sent to the ground.
                ** The timeout waiting for the response will cause problems if the radio continuously doesn't send responses.
                */
                if ( ( CADET_HkTelemetryPkt.PlaybackFifo & FIFO_REQUEST_HI_FIFO ) == FIFO_REQUEST_HI_FIFO )
                {
                    PktsOnFifo = CADET_HkTelemetryPkt.StatusInfo.HiFifoUsage / CADET_FLASH_PACKET_SIZE;
                }
                else
                {
                    PktsOnFifo = CADET_HkTelemetryPkt.StatusInfo.LoFifoUsage / CADET_FLASH_PACKET_SIZE;
                }

                if ( CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend > PktsOnFifo )
                {
                    CFE_EVS_SendEvent ( CADET_PLAYBACK_PARAMETER_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Packets to send %d more than FIFO has.  Limited to what FIFO has %d.", __LINE__,  
                                        CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend, PktsOnFifo );
                    CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend = PktsOnFifo;
                    CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend = CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend; /* Start with all to send */
                }
            }
            else
            {
                CFE_EVS_SendEvent ( CADET_PLAYBACK_PARAMETER_ERR_EID, CFE_EVS_ERROR, "L%d CADET: No Packets on FIFO.  FIFO = LO, LoPkts = %d", __LINE__,  
                                    (CADET_HkTelemetryPkt.StatusInfo.LoFifoUsage/CADET_FLASH_PACKET_SIZE) );
                CADET_HkTelemetryPkt.command_error_count++;
            }
        }
        else
        {
            CFE_EVS_SendEvent ( CADET_PLAYBACK_PARAMETER_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Invalid parameter.  PktsPerRequest = %d, PktsToSend = %d", __LINE__,  
                                PktPtr->PlaybackPktsPerRequest, PktPtr->PlaybackTotalPacketsToSend );
            CADET_HkTelemetryPkt.command_error_count++;
        }
    }
    else
    {
        CFE_EVS_SendEvent ( CADET_PLAYBACK_IN_PROGRESS_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Error, playback already in progress.", __LINE__ );
        CADET_HkTelemetryPkt.command_error_count++;
    }

    #ifdef _CADET_DEBUG_
        printf ( "CADET CMD: PktperReq = %d, PkttoSend = %d\n", (int)PktPtr->PlaybackPktsPerRequest, (int)PktPtr->PlaybackTotalPacketsToSend );
    #endif
}



/*
** Function: CADET_SetMaxSizeToAutoFlush
**
** Purpose: Allow the ground the specify a new maximum size of the hi FIFO prior to automatically flushing it.
**     NOTE: In an ideal world this would be know prior to launch and never changed.
**
*/
void CADET_SetMaxSizeToAutoFlush ( CFE_SB_MsgPtr_t  MessagePtr )
{
    CADET_MaxiumuSizeToAutoFlushCmd_t   *PktPtr = (CADET_MaxiumuSizeToAutoFlushCmd_t *) MessagePtr;

    CADET_HkTelemetryPkt.HighFifoMaxSizeForAutoFlush = PktPtr->MaxSize;  /* all values valid (0 means disabled) */
    CADET_HkTelemetryPkt.command_count++;

    #ifdef _CADET_DEBUG_
        printf ( "CADET CMD: HighFifoMaxSizeForAutoFlush = %d\n", (int)PktPtr->MaxSize );
    #endif
}



/*
** Function: CADET_SetPacketsToAutoFlush 
**
** Purpose: Allow the ground the specify a new number of hi FIFO packets to automatically flush.
**     NOTE: In an ideal world this would be know prior to launch and never changed.
**
*/
void CADET_SetPacketsToAutoFlush ( CFE_SB_MsgPtr_t  MessagePtr )
{
    CADET_NumberPacketsToAutoFlushCmd_t   *PktPtr = (CADET_NumberPacketsToAutoFlushCmd_t *) MessagePtr;

    CADET_HkTelemetryPkt.HighFifoPacketsToAutoFlush = PktPtr->NumPackets;  /* all values valid (0 means disabled) */
    CADET_HkTelemetryPkt.command_count++;

    #ifdef _CADET_DEBUG_
        printf ( "CADET CMD: HighFifoPacketsToAutoFlush = %d\n", (int)PktPtr->NumPackets);
    #endif
}



/*
** Function: CADET_setPacketsToAutoPlayback 
**
** Purpose: Allow the ground the specify a new a new number of hi FIFO packets to automatically playback.
**     NOTE: In an ideal world this would be know prior to launch and never changed.
**     NOTE: Must let command that sets the number of packets to auto playback finish auto playback prior to changing values.
**
*/
void CADET_setPacketsToAutoPlayback ( CFE_SB_MsgPtr_t  MessagePtr )
{
    CADET_NumberPacketsToAutoPlaybackCmd_t   *PktPtr = (CADET_NumberPacketsToAutoPlaybackCmd_t *) MessagePtr;

    CADET_HighFifoPacketsToAutoPlayback_Temp = PktPtr->NumPackets;  /* all values valid (0 means disabled) */
    if ( CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback == 0 )
    {
        CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback = PktPtr->NumPackets;  /* if was disabled, set now */
    }

    CADET_HkTelemetryPkt.command_count++;

    #ifdef _CADET_DEBUG_
        printf ( "CADET CMD: HighFifoPacketsToAutoPlayback = %d\n", (int)PktPtr->NumPackets);
    #endif
}



/*
** Function: CADET_JamFilterRangeCmd
**
** Purpose: The filter tables are normally loaded via table operations.
**     However, this command gives the option to change a range of entries in the table.
**
*/
void CADET_JamFilterRangeCmd ( CFE_SB_MsgPtr_t  MessagePtr )
{
    CADET_JamFilterRangeCmd_t* JamFilterRangeCmd  =  (CADET_JamFilterRangeCmd_t * ) MessagePtr;
    CFE_SB_PipeId_t            TlmPipe = CADET_INVALID_PIPE;
    boolean                    ValidCommand   = FALSE;
    uint16                     StartAppID;
    uint16                     EndAppID;
    uint16                     i;

    if ( ( JamFilterRangeCmd->StartMID  < DELLINGR_MIN_TLM_MID ) || ( JamFilterRangeCmd->StartMID  > DELLINGR_MAX_TLM_MID ) ||
         ( JamFilterRangeCmd->EndMID    < DELLINGR_MIN_TLM_MID ) || ( JamFilterRangeCmd->EndMID    > DELLINGR_MAX_TLM_MID ) ||
         ( JamFilterRangeCmd->StartMID  > JamFilterRangeCmd->EndMID ) )
    {
        /*
        ** Invalid Message ID received
        */
        CFE_EVS_SendEvent ( CADET_JAM_FILTER_CMD_ERR_EID, CFE_EVS_ERROR,  "L%d CADET: Jam Filter Range command: invalid argument, StartMID = x%04x, EndMID = x%04x", __LINE__, 
                            JamFilterRangeCmd->StartMID, JamFilterRangeCmd->EndMID );
    }
    else if ( ( JamFilterRangeCmd->Method != CADET_TIME_BASED_FILTER_TYPE ) && ( JamFilterRangeCmd->Method != CADET_SEQUENCE_BASED_FILTER_TYPE ) )
    {
        /*
        ** Invalid Filter Method received
        */
        CFE_EVS_SendEvent ( CADET_JAM_FILTER_CMD_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Jam Filter Range command: invalid argument, Filter Method = %d", __LINE__, JamFilterRangeCmd->Method );
    }
    else if ( ( JamFilterRangeCmd->Pipe != FILTER_HI_FIFO ) && ( JamFilterRangeCmd->Pipe != FILTER_LO_FIFO ) && ( JamFilterRangeCmd->Pipe != FILTER_BOTH_FIFO ) )
    {
        /*
        ** Invalid Destination Pipe received
        */
        CFE_EVS_SendEvent ( CADET_JAM_FILTER_CMD_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Jam Filter Range command: invalid argument, Destination Pipe = %d", __LINE__, JamFilterRangeCmd->Pipe );
    }
    else if ( ( JamFilterRangeCmd->ValueN > JamFilterRangeCmd->ValueX ) || ( ( JamFilterRangeCmd->ValueO >= JamFilterRangeCmd->ValueX ) && ( JamFilterRangeCmd->ValueX  != 0) ) )
    {
        /*
        ** Invalid Filter values received
        */
        CFE_EVS_SendEvent ( CADET_JAM_FILTER_CMD_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Jam Filter Range command: invalid argument, Filter values N = %d X = %d O = %d", __LINE__, 
                            JamFilterRangeCmd->ValueN, JamFilterRangeCmd->ValueX, JamFilterRangeCmd->ValueO );
    }
    else  /** Command appears good ***/
    {
        StartAppID = CCSDS_SID_APID(JamFilterRangeCmd->StartMID);
        EndAppID = CCSDS_SID_APID(JamFilterRangeCmd->EndMID);

        for ( i = StartAppID; i <= EndAppID; i++ )
        {
            /* Pipe was validated above */
            if ( JamFilterRangeCmd->Pipe == FILTER_BOTH_FIFO )
            {
                TlmPipe = FILTER_HI_FIFO;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].FilterMethod = JamFilterRangeCmd->Method;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].N            = JamFilterRangeCmd->ValueN;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].X            = JamFilterRangeCmd->ValueX;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].O            = JamFilterRangeCmd->ValueO;
                TlmPipe = FILTER_LO_FIFO;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].FilterMethod = JamFilterRangeCmd->Method;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].N            = JamFilterRangeCmd->ValueN;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].X            = JamFilterRangeCmd->ValueX;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].O            = JamFilterRangeCmd->ValueO;
            }
            else
            {
                TlmPipe = JamFilterRangeCmd->Pipe;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].FilterMethod = JamFilterRangeCmd->Method;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].N            = JamFilterRangeCmd->ValueN;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].X            = JamFilterRangeCmd->ValueX;
                CADET_FilterTablePtr->AppID[i].Fifo[TlmPipe].O            = JamFilterRangeCmd->ValueO;
            }
            ValidCommand  = CADET_SubscribeToPacket ( i, CADET_FilterTablePtr->AppID[i].Fifo[FILTER_HI_FIFO].N, CADET_FilterTablePtr->AppID[i].Fifo[FILTER_HI_FIFO].X, 
                                                      CADET_FilterTablePtr->AppID[i].Fifo[FILTER_LO_FIFO].N, CADET_FilterTablePtr->AppID[i].Fifo[FILTER_LO_FIFO].X );
        }
 
        /* 
        ** Tell Table Services that things have been updated 
        */
        CFE_TBL_Modified( CADET_FilterTableHandle );

    }
    
    if ( ValidCommand == TRUE )
    {
        CFE_EVS_SendEvent ( CADET_JAM_FILTER_CMD_DBG_EID, CFE_EVS_DEBUG, "L%d CADET: Jam Filter Range command (StartMID=%x EndMID=%x Method =%x X=%d N=%d O=%d)", __LINE__, 
                            JamFilterRangeCmd->StartMID, JamFilterRangeCmd->EndMID, JamFilterRangeCmd->Method, JamFilterRangeCmd->ValueX, JamFilterRangeCmd->ValueN,JamFilterRangeCmd->ValueO );
        CADET_HkTelemetryPkt.command_count++;
    }
    else
    {
        CADET_HkTelemetryPkt.command_error_count++;
    }
}



/*
** Function: CADET_InitCadetUartCmd
**
** Purpose: Reset cadet UART.
**
*/
void CADET_InitCadetUartCmd( void )
{
    CFE_SB_MsgPtr_t  MsgPtr;

    /*
    ** Initialize Input Processing from radio.
    */
    OS_TaskDelay ( 1000 );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
    (void) CADET_Input_Initialize ( );

    CFE_EVS_SendEvent ( CADET_RESETCADETUART_INF_EID, CFE_EVS_INFORMATION, "L%d CADET: Initialize Cadet UART", __LINE__ );

}



/*
** Function: CADET_ResetCadetCmd
**
** Purpose: Reset cadet radio.
**
*/
void CADET_ResetCadetCmd( void )
{
    CFE_SB_MsgPtr_t  MsgPtr;

    /*
    ** This is somewhat cheesy
    **     Reset Radio
    **     Flush upto 4 wakeup messages from wakeup pipe
    **     Wait 1 second to let radio reset
    **     Flush upto 4 more wakeup messages from wakeup pipe
    */
    SSC_ResetCadetRadio ( );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
//    OS_TaskDelay ( 1000 );
//    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
//    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
//    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
//    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
//    (void) CADET_Input_Initialize ( );
    OS_TaskDelay ( 1000 );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );
    (void) CFE_SB_RcvMsg ( &MsgPtr, CADET_WakeupPipe, CFE_SB_POLL );

    CFE_EVS_SendEvent ( CADET_RESETCADET_INF_EID, CFE_EVS_INFORMATION, "L%d CADET: Reset Cadet Radio", __LINE__ );

}



/*
** Function: CADET_ExecLocalCommand
**
** Purpose: Determine which ground command was received and call appropriate subfunction.
**
*/
void CADET_ExecLocalCommand ( CFE_SB_MsgPtr_t cmd )
{
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(cmd);
    uint16 CommandCode = CFE_SB_GetCmdCode(cmd);

    switch (CommandCode)
    {
        case CADET_APP_NOOP_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_NoArgsCmd_t) ) )
            {
                CADET_HkTelemetryPkt.command_count++;
                CFE_EVS_SendEvent ( CADET_COMMANDNOP_INF_EID, CFE_EVS_INFORMATION, "L%d CADET: NOOP command", __LINE__ );
            } /* Error messages handled inside VerifyCmdLength function */
            break;

        case CADET_APP_RESET_COUNTERS_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_NoArgsCmd_t) ) )
            {
                CADET_ResetCounters();
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_TAKE_DOWNLINK_SEM_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_NoArgsCmd_t) ) )
            {
                CADET_TakeSemaphore();
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_GIVE_DOWNLINK_SEM_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_NoArgsCmd_t) ) )
            {
                CADET_GiveSemaphore();
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_JAM_FILTER_FACTOR_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_JamFilterCmd_t) ) )
            {
                CADET_JamFilterCmd(cmd);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_START_PLAYBACK_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_StartPlaybackCmd_t) ) )
            {
                CADET_StartPlayback(cmd);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_ABORT_PLAYBACK_FLUSH_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_NoArgsCmd_t) ) )
            {
                CADET_AbortPlaybackFlush();
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_FLUSH_FIFO_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_FlushFifoCmd_t) ) )
            {
                CADET_FlushFifo(cmd);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_FLUSH_HI_FIFO_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_FlushEntireFifoHiCmd_t) ) )
            {
                CADET_FlushEntireHiFifo(cmd);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_PB_FLUSH_HI_FIFO_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_PlaybackAndFlushHiFifoCmd_t) ) )
            {
                CADET_PlaybackAndFlushHiFifo(cmd);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_PB_LO_HI_FIFO_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_StartLoFifoWithHiFifoPlaybackCmd_t) ) )
            {
                CADET_StartLoFifoPlayback(cmd);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_MAX_SIZE_TO_AUTO_FLUSH_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_MaxiumuSizeToAutoFlushCmd_t) ) )
            {
                CADET_SetMaxSizeToAutoFlush (cmd);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_PACKETS_TO_AUTO_FLUSH_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_NumberPacketsToAutoFlushCmd_t) ) )
            {
                CADET_SetPacketsToAutoFlush (cmd);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_PACKETS_TO_AUTO_PLAYBACK_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_NumberPacketsToAutoPlaybackCmd_t) ) )
            {
                CADET_setPacketsToAutoPlayback (cmd);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_JAM_FILTER_RANGE_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_JamFilterRangeCmd_t) ) )
            {
                CADET_JamFilterRangeCmd(cmd);
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_INIT_CADET_UART_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_NoArgsCmd_t) ) )
            {
                CADET_InitCadetUartCmd();
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       case CADET_APP_RESET_CADET_CC:
            if ( CADET_VerifyCmdLength(cmd, sizeof(CADET_NoArgsCmd_t) ) )
            {
                CADET_ResetCadetCmd();
            } /* Error messages handled inside VerifyCmdLength function */
            break;

       default:
            CADET_HkTelemetryPkt.command_error_count++;
            CFE_EVS_SendEvent ( CADET_FNCODE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Unknown command code, MID = 0x%x, CC = %d", __LINE__, MsgId, CommandCode );
    }
}



/************************/
/*  End of File Comment */
/************************/




