/*
** File: cadet_output.c
**
** Purpose:
**     This file contains the source code for the radio output routines, initialization routine, and 
**     interface functions.
**
*/



/*
**   Includes:
*/
#include "cfe.h"
#include "cadet_app.h"
#include "cadet_hk.h"
#include "cadet_output.h"
#include "cadet_vcdu.h"
#include "cadet_fltrtbl.h"
#include "cadet_msg.h"
#include "cadet_radio.h"
#include "cadet_perfids.h"
#include "cadet_events.h"
#include "cfs_utils.h"
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
uint32                   CADET_PlaybackWaitingForResponse;
uint32                   CADET_PlaybackTimeout;
uint32                   CADET_AutoFlushMutex;
uint32                   CADET_AutoPlaybackMutex;



/*
** Local Variables (could be defined static)
*/
CFE_SB_PipeId_t          CADET_RadioCommandPipe;
uint32                   ExecutionLoop;



/*
** Exported Functions
*/



/*
** Local Functions (could be defined static)
*/



/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/



/*
** Function: CADET_PeriodicHkCommands
**
** Purpose: This function is a kludge.  We want to put the version information and configuration information 
**     from the CADET radio into routine housekeeping.  However, this data is not routinely sent
**     from the CADET radio to the nanomind FSW.  Hence, we need to periodically request it.  This
**     function will periodically request, exact timing is NOT necessary.
**
** NOTE: The status information is returned every time we send a HI FIFO add command and the 
**     command response is returned every time we send a request command, so these two don't
**     need to be periodically requested from the CADET radio.
*/
int32 CADET_PeriodicHkCommands ( void )
{
    int32  PktSent = FALSE;

    ExecutionLoop++;

    if ( ( ExecutionLoop % REQUEST_VERSION_COMMAND_RATE ) == REQUEST_VERSION_COMMAND_OFFSET )
    {
        CADET_GetVersionCmd ( );
        PktSent = TRUE;
    }
    if ( ( ExecutionLoop % REQUEST_CONFIG_COMMAND_RATE ) == REQUEST_CONFIG_COMMAND_OFFSET )
    {
        CADET_GetConfigCmd ( );
        PktSent = TRUE;
    }

    return ( PktSent );

}



/*
** Function: CADET_ProcessRadioCommandPipe
**
** Purpose: This function will service the ground command pipe for CADET radio commands.  These 
**     commands provide all the necessary data and the nanomind FSW simply puts a CADET radio header
**     in front of the data prior to sending.
*/
int32 CADET_ProcessRadioCommandPipe ( void )
{
    int32           CFE_SB_status;
    uint16          CommandCode;
    CFE_SB_Msg_t*   PktPtr;
    int32           PktSent = FALSE;
    CADET_RadioSetTimeCmd_t*              SetTimeCmd;
    CADET_RadioResetCmd_t*                ResetCmd;
    CADET_RadioFifoRequestCmd_t*          RequestCmd;
    CADET_RadioFifoClearCmd_t*            ClearCmd;
    CADET_RadioAuthenticationEnableCmd_t* AuthenticationEnableCmd;
    CADET_RadioFifoRequestExtCmd_t*       RequestExtCmd;

    /*
    ** Get packet from SWB (only messages for radio are on this pipe).
    ** Call correct function to build cadet header and send across the UART.
    */
    CFE_SB_status = CFE_SB_RcvMsg ( &PktPtr, CADET_RadioCommandPipe, CFE_SB_POLL );

    if ( CFE_SB_status == CFE_SUCCESS )
    {
        PktSent = TRUE; /* Will be true unless default case executed */
        CADET_HkTelemetryPkt.RadioData.cadet_command_packets_sent++;

        CommandCode = CFE_SB_GetCmdCode ( PktPtr );

        switch ( CommandCode )
        {
            case CADET_GET_VERSION_CC:
                 CADET_GetVersionCmd ();
                 break;

            case CADET_GET_CONFIG_CC:
                 CADET_GetConfigCmd ();
                 break;

            case CADET_GET_STATUS_CC:
                 CADET_GetStatusCmd ();
                 break;

            case CADET_SET_TIME_CC:
                 SetTimeCmd = ( (CADET_RadioSetTimeCmd_t *) PktPtr );
                 CADET_SetTimeCmd ( &(SetTimeCmd->Date) );
                 break;

            case CADET_RESET_RADIO_CC:
                 ResetCmd = ( (CADET_RadioResetCmd_t *) PktPtr );
                 CADET_ResetRadioCmd ( &(ResetCmd->Reset) );
                 break;

            case CADET_FIFO_REQUEST_CC:
                 RequestCmd = ( (CADET_RadioFifoRequestCmd_t *) PktPtr );
                 CADET_FifoRequestCmd ( &(RequestCmd->Request) );
                 break;

            case CADET_FIFO_CLEAR_CC:
                 ClearCmd = ( (CADET_RadioFifoClearCmd_t *) PktPtr );
                 CADET_FifoClearCmd ( &(ClearCmd->Clear) );
                 break;

            case CADET_AUTH_ENABLE_CC:
                 AuthenticationEnableCmd = ( (CADET_RadioAuthenticationEnableCmd_t *) PktPtr );
                 CADET_AuthEnableCmd ( &(AuthenticationEnableCmd->KeySlot) );
                 break;

            case CADET_AUTH_DISABLE_CC:
                 CADET_AuthDisableCmd ( );
                 break;

            case CADET_FIFO_REQUEST_EXT_CC:
                 RequestExtCmd = ( (CADET_RadioFifoRequestExtCmd_t *) PktPtr );
                 CADET_FifoRequestExtCmd ( &(RequestExtCmd->RequestExt) );
                 break;

            default:
                 CFE_EVS_SendEvent ( CADET_FNCODE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Invalid Function Code Rcvd In CADET Radio CMD 0x%x",__LINE__, CommandCode );
                 PktSent = FALSE;  /* Opps, didn't send packet to radio */
                 CADET_HkTelemetryPkt.RadioData.cadet_command_packets_sent--;  /* Opps, didn't send packet to radio */
        } /* End switch */
    }

    return ( PktSent );
}



/*
** Function: CADET_GenerateFifoRequestCmd
**
** Purpose: Determine packets to requested this cycle and build command.
**     NOTE: Do not try to determine if done.  That is calling functions responsibility.  This just sends the command.
**
*/
void CADET_GenerateFifoRequestCmd ( void )
{
    CadetFifoRequestPayload_t  Cmd;
    uint32                     PktCount;

    /* 
    ** Determine number of packets to request.
    **     Minimum from command or whatever is left.
    */
    if ( CADET_HkTelemetryPkt.PlaybackPktsPerRequest < CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend )
    {
        PktCount = CADET_HkTelemetryPkt.PlaybackPktsPerRequest;
    }
    else
    {
        PktCount = CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend;
    }
    CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend -= PktCount;

    if ( PktCount == 0 )
    {
        printf ( "\n\n\nERROR, trying to playback 0 frames\n" );
    }

    /*
    ** Now build command and send to Cadet 
    */
    Cmd.FLAGS = CADET_HkTelemetryPkt.PlaybackFifo | FIFO_REQUEST_START_BLOCK_VALID;
    Cmd.PacketCount = PktCount;
    Cmd.StartingPacketNumber = CADET_HkTelemetryPkt.PlaybackNextStartingPacket;
    Cmd.NewPacketSize = 0;
    Cmd.TransmitPowerLevel = 0;

#ifdef _CADET_DEBUG_
    printf ( "FIFO = %d, PktperReq = %d, StartPkt = %d, PktLeftToSend = %d\n", (int)CADET_HkTelemetryPkt.PlaybackFifo, (int)PktCount, 
             (int)CADET_HkTelemetryPkt.PlaybackNextStartingPacket, (int)CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend );
#endif
    /*
    ** Set internal values so we don't command another request to soon.
    */
    CADET_PlaybackWaitingForResponse = TRUE;
    CADET_PlaybackTimeout = ( Cmd.PacketCount / 100 ) + 10;  /* Must be greater than 0 */
    CADET_FifoRequestCmd ( &Cmd );

    CADET_HkTelemetryPkt.PlaybackNextStartingPacket += PktCount;

}



/*
** Function: CADET_GenerateFifoFlushCmd
**
** Purpose: Determine packets to flushed this cycle and build command.
**     NOTE: Do not try to determine if done.  That is calling functions responsibility.  This just sends the command.
**
*/
void CADET_GenerateFifoFlushCmd ( void )
{
    CadetFifoClearPayload_t    Cmd;
    uint32                     PktCount;

    /* 
    ** Determine number of packets to request.
    **     Minimum from command or whatever is left.
    */
    if ( CADET_HkTelemetryPkt.FlushPktsPerRequest < CADET_HkTelemetryPkt.FlushPacketsLeftToClear )
    {
        PktCount = CADET_HkTelemetryPkt.FlushPktsPerRequest;
    }
    else
    {
        PktCount = CADET_HkTelemetryPkt.FlushPacketsLeftToClear;
    }
    CADET_HkTelemetryPkt.FlushPacketsLeftToClear -= PktCount;

    if ( PktCount == 0 )
    {
        printf ( "\n\n\nERROR, trying to clear 0 frames\n" );
    }

    /*
    ** Now build command and send to Cadet 
    */
    Cmd.FLAGS = CADET_HkTelemetryPkt.FlushFifo;
    Cmd.PacketCount = PktCount;
    Cmd.StartingPacket = 0;
    Cmd.NewPacketSize = 0;

#ifdef _CADET_DEBUG_
    printf ( "FIFO = %d, PktperReq = %d, PkttoClear = %d\n", (int)CADET_HkTelemetryPkt.FlushFifo, (int)PktCount, 
             (int)CADET_HkTelemetryPkt.FlushPacketsLeftToClear );
#endif
    CADET_FifoClearCmd ( &Cmd );

}



/*
** Function: CADET_GenerateHiFifoPlaybackAndFlush
**
** Purpose: Playback the entire hi FIFO and then flush the entire hi FIFO.
**
*/
void CADET_GenerateHiFifoPlaybackAndFlush ( void )
{

    /*
    ** Send all playback request commands first, then send all clear commands last
    */
    if ( CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend > 0 )
    {
        /*
        ** Generate FIFO request command, but no check to see if done as will flush later.
        */
        CADET_GenerateFifoRequestCmd ( );
    }
    else if ( CADET_HkTelemetryPkt.FlushPacketsLeftToClear > 0 )
    {
        /*
        ** Generate FIFO clear and check to see if done.
        */
        CADET_GenerateFifoFlushCmd ( );
        if ( CADET_HkTelemetryPkt.FlushPacketsLeftToClear == 0 )
        {
            CADET_HkTelemetryPkt.Mode = MODE_OPEN;  /* Will not be interleaving anymore */
            CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;  
            CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_NONE;
            CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback = CADET_HighFifoPacketsToAutoPlayback_Temp; /* Update only after transition back to open mode */
        }
    }
    else
    {
        printf ( "\n\n\nERROR, playback/flush Hi FIFO should be done\n" );
    }
}



/*
** Function: CADET_PlaybackLowFifoWithHiFifo
**
** Purpose: Playback the lo FIFO but periodically output some hi FIFO packets to get real-time data.
**
*/
void CADET_PlaybackLowFifoWithHiFifo ( void )
{
    CadetFifoRequestPayload_t  Cmd;
    uint32                     PktCount;

    /*
    ** Check if last cycle was last Lo FIFO request.  If so, set flags indicating done.  Will send one more Hi FIFO packet later.
    */
    if ( CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend == 0 )
    {
        CADET_HkTelemetryPkt.Mode = MODE_OPEN;  /* Will not be interleaving anymore */
        CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;  
        CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_NONE;
        CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback = CADET_HighFifoPacketsToAutoPlayback_Temp; /* Update only after transition back to open mode */
    }

    /*
    ** Every tenth time, send Hi FIFO radio packets, otherwise keep blasting Low FIFO radio packets.
    ** Also on the very last cycle, send Hi FIFO radio packets.
    ** NOTE: Only do this if the HI FIFO has another packet to send.
    */
    if ( ( ( ( CADET_HkTelemetryPkt.PlaybackNextStartingCycle % CYCLE_BETWEEN_LO_HI_PLAYBACKS_REQUESTS ) == 0 ) || ( CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend == 0 ) ) &&
         ( CADET_HkTelemetryPkt.PlaybackNextStartingPacketHi <= ( CADET_HkTelemetryPkt.StatusInfo.HiFifoUsage / CADET_FLASH_PACKET_SIZE ) ) )
    {
        /*
        ** Determine packet count starting from next starting packet.
        **    Starting packet is the next packet to send, starting with 1 (not zero like normal c).
        **    Hi FIFO usage is the number of packets on the FIFO.
        **    Hence, add one.  50-50 means there is 1 packet to playback (packet number 50).  50 - 49 means there is 2 packets to playback (packets 49 and 50).
        ** NOTE: Limit to the ground command packets per request.
        */
        PktCount = ( CADET_HkTelemetryPkt.StatusInfo.HiFifoUsage / CADET_FLASH_PACKET_SIZE ) - CADET_HkTelemetryPkt.PlaybackNextStartingPacketHi + 1;
        if ( PktCount > CADET_HkTelemetryPkt.PlaybackPktsPerRequest )
        {
            PktCount = CADET_HkTelemetryPkt.PlaybackPktsPerRequest;
        }

        if ( PktCount == 0 )
        {
            printf ( "\n\n\nERROR, trying to playback 0 HI frames\n" );
        }

        /* 
        ** Send next Hi FIFO radio packet.
        */
        Cmd.FLAGS = FIFO_REQUEST_HI_FIFO | FIFO_REQUEST_START_BLOCK_VALID;
        Cmd.PacketCount = PktCount;
        Cmd.StartingPacketNumber = CADET_HkTelemetryPkt.PlaybackNextStartingPacketHi;
        Cmd.NewPacketSize = 0;
        Cmd.TransmitPowerLevel = 0;

#ifdef _CADET_DEBUG_
        printf ( "FIFO = %d, PktperReq = %d, StartPkt = %d\n", (int)FIFO_REQUEST_HI_FIFO, (int) 1, 
                 (int)CADET_HkTelemetryPkt.PlaybackNextStartingPacketHi );
#endif
        /*
        ** Set internal values so we don't command another request to soon.
        */
        CADET_PlaybackWaitingForResponse = TRUE;
        CADET_PlaybackTimeout = ( Cmd.PacketCount / 100 ) + 10;  /* Must be greater than 0 */
        CADET_FifoRequestCmd ( &Cmd );
  
        CADET_HkTelemetryPkt.PlaybackNextStartingPacketHi += PktCount;
    }
    else if ( CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend == 0 )
    {
        /* Special case.
        **     Done with LO FIFO.
        **     Suppose to send more HI FIFO packets.
        **     But HI FIFIO has no more packets.
        **
        ** Not an error, but nothing to do.
        */
    }
    else
    {
        /*
        ** More Low FIFO packets to send, but no check to see if done as will do at least Hi FIFO request later.
        */
        CADET_GenerateFifoRequestCmd ( );
    }

    CADET_HkTelemetryPkt.PlaybackNextStartingCycle++;
}



/*
** Function: CADET_GenerateCmdToRadio
**
** Purpose: Determine which radio command sequence is on-going and continue.
**
*/
void CADET_GenerateCmdToRadio ( void )
{
    switch ( CADET_HkTelemetryPkt.SubmodeCommand )
    {
        case SUBMODE_CMD_PLAYBACK:
             /*
             ** Generate FIFO request and check to see if done.
             */
             CADET_GenerateFifoRequestCmd ( );
             if ( CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend == 0 )
             {
                 CADET_HkTelemetryPkt.Mode = MODE_OPEN;  /* Will not be interleaving anymore */
                 CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;  
                 CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_NONE;
                 CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback = CADET_HighFifoPacketsToAutoPlayback_Temp; /* Update only after transition back to open mode */
             }
             break;
        case SUBMODE_CMD_FLUSH:
        case SUBMODE_CMD_FLUSH_HI_ALL:
             /*
             ** Generate FIFO clear and check to see if done.
             */
             CADET_GenerateFifoFlushCmd ( );
             if ( CADET_HkTelemetryPkt.FlushPacketsLeftToClear == 0 )
             {
                 CADET_HkTelemetryPkt.Mode = MODE_OPEN;  /* Will not be interleaving anymore */
                 CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;  
                 CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_NONE;
                 CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback = CADET_HighFifoPacketsToAutoPlayback_Temp; /* Update only after transition back to open mode */
             }
             break;
        case SUBMODE_CMD_PB_FLUSH_HI:
             CADET_GenerateHiFifoPlaybackAndFlush ( );
             break;
        case SUBMODE_CMD_PB_LOW_HI:
             CADET_PlaybackLowFifoWithHiFifo ( );
             break;
        default:
             break;
    }
}



/*
** Function: CADET_GetAndAddPktToVCDU
**
** Purpose: Put a packet onto the VCDUs both hi and low.
**
*/
int32 CADET_GetAndAddPktToVCDU ( CFE_SB_PipeId_t TlmPipe )
{
    int32                  CFE_SB_status;
    CFE_SB_Msg_t           *PktPtr;
    uint16                 AppID;
    uint16                 MsgID;
    boolean                OmitPacket;
    int32                  PacketDropped;

    // If file downlink segement packet then give FDL handshake semaphore
    // If not filtered on high fifo then add to high vcdu
    // If not filtered on low fifo then add to low vcdu

    CFE_SB_status = CFE_SB_RcvMsg ( &PktPtr, TlmPipe, CFE_SB_POLL );

    if ( CFE_SB_status == CFE_SUCCESS )
    {

       MsgID = CFE_SB_GetMsgId ( PktPtr );

       if ( ( MsgID  >= DELLINGR_MIN_TLM_MID ) && ( MsgID  <= DELLINGR_MAX_TLM_MID ) )
       {   
           /* Populate UART message */
           (void)CADET_HK_UartPopulateMsg ( PktPtr );

           /* Give semaphore if file dump packet */
           if ( MsgID == FDL_APP_SEGMENT_MID )
           {
               OS_CountSemGive ( CADET_FileDownlinkSemaphore );
           }

           /* Message Id within limits, OK to use as index */
           AppID = CCSDS_SID_APID(MsgID);

           /* Increment activity counter */
           CADET_FilterStatTablePtr->PacketRecvCnt [ AppID ]++;

           /*
           **   Determine if packet is included. 1 is added to the FilterMethod as the cadet method is 0
           **   indexed, and CFS indexes from 1. (this is so an all 0 filter table is valid for CADET).
           */
           OmitPacket =  CFS_IsPacketFiltered ( PktPtr, ((CADET_FilterTablePtr->AppID[ AppID ].Fifo[FILTER_HI_FIFO].FilterMethod) + 1),
                                                CADET_FilterTablePtr->AppID[ AppID ].Fifo[FILTER_HI_FIFO].N,             
                                                CADET_FilterTablePtr->AppID[ AppID ].Fifo[FILTER_HI_FIFO].X,             
                                                CADET_FilterTablePtr->AppID[ AppID ].Fifo[FILTER_HI_FIFO].O  );         

           if ( OmitPacket != TRUE )
           {
               /* Add to High FIFO VCDU (increment error counter if dropped) */
               PacketDropped = CADET_VCDU_AddPacketToVcdu ( PktPtr, VCDU_HI_FIFO );
               if ( PacketDropped == TRUE )
               {
                   CADET_HkTelemetryPkt.HighFifoPacketsDropped++;
               }
           }

           /*
           **   Determine if packet is included. 1 is added to the FilterMethod as the TO method is 0
           **   indexed, and CFS indexes from 1. (this is so an all 0 filter table is valid for CADET).
           */
           OmitPacket =  CFS_IsPacketFiltered ( PktPtr, ((CADET_FilterTablePtr->AppID[ AppID ].Fifo[FILTER_LO_FIFO].FilterMethod) + 1),
                                                CADET_FilterTablePtr->AppID[ AppID ].Fifo[FILTER_LO_FIFO].N,            
                                                CADET_FilterTablePtr->AppID[ AppID ].Fifo[FILTER_LO_FIFO].X,             
                                                CADET_FilterTablePtr->AppID[ AppID ].Fifo[FILTER_LO_FIFO].O  );         

           if ( OmitPacket != TRUE )
           {
               /* Add to Low FIFO VCDU (increment error counter if dropped) */
               PacketDropped = CADET_VCDU_AddPacketToVcdu ( PktPtr, VCDU_LO_FIFO );
               if ( PacketDropped == TRUE )
               {
                   CADET_HkTelemetryPkt.LowFifoPacketsDropped++;
               }
           }
       }
    }
    return ( CFE_SB_status );
}



/*
** Function: CADET_BuildVcdus
**
** Purpose: Pull the packets off the software bus and pack them into the VCDUs both hi and low.
**
*/
void CADET_BuildVcdus ( void )
{
    int32   Status;

    /* Get first packet to prime while loop */
    Status = CADET_GetAndAddPktToVCDU ( CADET_TlmPipe );
    while ( Status == CFE_SUCCESS )
    {
        /* Get next packet and continue/discontinue while loop */
        Status = CADET_GetAndAddPktToVCDU ( CADET_TlmPipe );
    }
}



/*
** Function: CADET_AddFileDownlinkData
**
** Purpose: If there is room in the VCDUs, also add some file downlink data.
**
*/
void CADET_AddFileDownlinkData ( void )
{
    uint32 HiSize;
    uint32 HiSizeHalf;
    uint32 HiSizeThreeForth;
    uint32 HiUsed;
    uint32 LoSize;
    uint32 LoSizeHalf;
    uint32 LoSizeThreeForth;
    uint32 LoUsed;
    boolean AddMoreDataToFifos;
    int32   Status;

    // While both hi and lo VCDUs less than half full (and more file downlink data), read and add more file downlink data to VCDUs.
    // If either already more than 3/4 full then don't add more data.
    // Precedence is given to the regular telemetry, but we don't want to starve the file downlinke pipe if possible.
    // It is assumed the file downlink packets are being stored on both FIFOs.  Technically this doesn't have to be true.
    HiSize = CADET_VcduTotalSize ( VCDU_HI_FIFO );
    HiSizeHalf = HiSize / 2;
    HiSizeThreeForth = ( HiSize * 3 ) / 4;
    LoSize = CADET_VcduTotalSize ( VCDU_LO_FIFO );
    LoSizeHalf = LoSize / 2;
    LoSizeThreeForth = ( LoSize * 3 ) / 4;

    AddMoreDataToFifos = TRUE;
    while ( AddMoreDataToFifos == TRUE )
    {
        HiUsed = HiSize - CADET_VcduTotalFreeSpace ( VCDU_HI_FIFO );
        LoUsed = LoSize - CADET_VcduTotalFreeSpace ( VCDU_LO_FIFO );

        if ( ( HiUsed > HiSizeThreeForth ) || ( LoUsed > LoSizeThreeForth ) )
        {
            /*
            ** One or the other FIFO VCDUs is too full.  No more dump data.
            */
            AddMoreDataToFifos = FALSE;
        }
        else if ( ( HiUsed < HiSizeHalf ) && ( LoUsed < LoSizeHalf ) )
        {
            /*
            ** Try to get one more packet.
            ** If none available then done.
            */
            Status = CADET_GetAndAddPktToVCDU ( CADET_FdlPipe );
            if ( Status != CFE_SUCCESS )
            {
                AddMoreDataToFifos = FALSE;
            }
        }
        else
        {
            /*
            ** Both FIFOs less than 3/4 full, but one of them is greater than half full.  No more dump data.
            */
            AddMoreDataToFifos = FALSE;
        }
    }
}



/*
** Function: CADET_TryToOutputHiFifoVcdu
**
** Purpose: If a hi FIFO VCDU is ready to go out then output it (calling function determined it was ok to output).
**
*/
int32 CADET_TryToOutputHiFifoVcdu ( void )
{
    int32     PktSent = FALSE;
    uint32    ReadIndex;
    uint8*    VcduPtr = NULL;
    uint32    VcduSize;
    int32     Status;

    // Determine if a VCDU buffer is full
    // If so, get the earliest one and output to radio and Update read index
    // If something sent then PktSent=TRUE

    Status = CADET_VCDU_GetNextBufferToOutput ( VCDU_HI_FIFO, &ReadIndex, &VcduPtr, &VcduSize );

    if ( Status == CDH_BUFFER_VCDU_READY_TO_OUTPUT )
    {
        /* Have something to output */
#ifdef _CADET_DEBUG_
//        printf ( "HI: %d %d\n", (int)VcduPtr, (int)VcduSize );
#endif
        CFE_ES_PerfLogEntry ( CADET_APP_SEND_HI_PERF_ID );
        CADET_FifoAdd ( CADET_PKT_MSGID_FIFO_ADD_HIGH /* Do NOT Use Silent Mode */, VcduPtr, VcduSize );
        CFE_ES_PerfLogExit ( CADET_APP_SEND_HI_PERF_ID );   
        PktSent = TRUE;

        CADET_VCDU_FreeReadIndex ( VCDU_HI_FIFO, ReadIndex );
        CADET_HkTelemetryPkt.RadioData.cadet_high_bytes_sent += VcduSize;

        /*
        ** If we were waiting for a high fifo packet to be output then update stats
        */
        if ( CADET_HkTelemetryPkt.StartAutoPlaybackWaitForHiVCDU == TRUE )
        {
            CADET_HkTelemetryPkt.StartAutoPlaybackWaitForHiVCDU = FALSE;
            CADET_HkTelemetryPkt.StartAutoPlaybackConfigureForPlayback = TRUE;
        }
    }

    return ( PktSent );
}



/*
** Function: CADET_TryToOutputLoFifoVcdu
**
** Purpose: If a lo FIFO VCDU is ready to go out then output it (calling function determined it was ok to output).
**
*/
int32 CADET_TryToOutputLoFifoVcdu ( void )
{
    int32     PktSent = FALSE;
    uint32    ReadIndex;
    uint8*    VcduPtr = NULL;
    uint32    VcduSize;
    int32     Status;

    // Determine if a VCDU buffer is full
    // If so, get the earliest one and output to radio and Update read index
    // If something sent then PktSent=TRUE

    Status = CADET_VCDU_GetNextBufferToOutput ( VCDU_LO_FIFO, &ReadIndex, &VcduPtr, &VcduSize );

    if ( Status == CDH_BUFFER_VCDU_READY_TO_OUTPUT )
    {
        /* Have something to output */
#ifdef _CADET_DEBUG_
//        printf ( "LO: %d %d\n", (int)VcduPtr, (int)VcduSize );
#endif
        CFE_ES_PerfLogEntry ( CADET_APP_SEND_LO_PERF_ID );
        CADET_FifoAdd ( CADET_PKT_MSGID_FIFO_ADD_LOW + 0x8000 /* Use Silent Mode */, VcduPtr, VcduSize );
        CFE_ES_PerfLogExit ( CADET_APP_SEND_LO_PERF_ID );   
        PktSent = TRUE;

        CADET_VCDU_FreeReadIndex ( VCDU_LO_FIFO, ReadIndex );
        CADET_HkTelemetryPkt.RadioData.cadet_low_bytes_sent += VcduSize;
    }

    return ( PktSent );
}




/*
** Function: CADET_Output_TransmitDataToRadio
**
** Purpose: Process telemetry to be sent to the radio (in priority order)
**
*/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                    */
/* CADET_Output_TransmitDataToRadio() -- Process telemetry to be sent to the */
/*                                  radio.                            */
/*                                                                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void CADET_Output_TransmitDataToRadio ( void )
{
    int32     PktSent = FALSE;

    /*
    ** Remove data from SW pipe and build Hi and Lo VCDUs.
    ** Also add File Downlink Data (if room).
    ** If waiting for response from radio then send no data to the radio (regardless of how bad we want to).
    ** IF we are nto waiting for a response then
    **     Can send one packet to the radio
    **     Send periodic request for data to include with our HK data.
    **     If in open mode then send radio data in priority order (cmd, hi fifo, lo fifo).
    **     If in interleaving mode then send radio data in different priority order.
    */
    CADET_BuildVcdus ( );
    CADET_AddFileDownlinkData ( );

    if ( CADET_PlaybackWaitingForResponse == FALSE ) 
    {
        PktSent = CADET_PeriodicHkCommands ( );

        if ( CADET_HkTelemetryPkt.Mode == MODE_OPEN )
        {
            /*
            ** Nothing was pre-designated for the radio.  Output whatever we have.
            */
            if ( PktSent == FALSE )
            {
                PktSent = CADET_ProcessRadioCommandPipe( );
            }

            if ( PktSent == FALSE )
            {
                PktSent = CADET_TryToOutputHiFifoVcdu ( );
            }

            if ( PktSent == FALSE )
            {
                PktSent = CADET_TryToOutputLoFifoVcdu ( );
            }
        }
        else
        {
            /*
            ** An on-going ground command is being interleaved with add data to fifo requests.
            **
            ** Determine if a fifo request or data should be sent to the radio.
            */
            if ( ( ( CADET_HkTelemetryPkt.Submode == SUBMODE_NONE ) || ( CADET_HkTelemetryPkt.Submode == SUBMODE_CMD ) ) && ( CADET_PlaybackWaitingForResponse == FALSE ) )
            {
                if ( PktSent == FALSE )
                {
                    CADET_GenerateCmdToRadio ( );
                    PktSent = TRUE; /* Generate command to radio will send packet to the radio */
                    CADET_HkTelemetryPkt.Submode = SUBMODE_DATA;  /* Next time give priority to hi/lo fifo add */
                }
            }
            else
            {
                if ( PktSent == FALSE )
                {
                    PktSent = CADET_ProcessRadioCommandPipe( );
                }

                /* Can't add data to HI FIFO until HI FIFO playback and flush complete. */
                if ( ( PktSent == FALSE ) && ( CADET_HkTelemetryPkt.SubmodeCommand != SUBMODE_CMD_PB_FLUSH_HI ) )
                {
                    PktSent = CADET_TryToOutputHiFifoVcdu ( );
                }
    
                if ( PktSent == FALSE )
                {
                    PktSent = CADET_TryToOutputLoFifoVcdu ( );
                }

                if ( ( PktSent == FALSE ) && ( CADET_PlaybackWaitingForResponse == FALSE ) )
                {
                    /* No data to send, output more ground requests */
                    CADET_GenerateCmdToRadio ( );
                    PktSent = TRUE; /* Generate command to radio will send packet to the radio */
                }
                CADET_HkTelemetryPkt.Submode = SUBMODE_CMD; /* Next time give priority to continuation of ground command */
            }
        }
    }
    else
    {
        CADET_PlaybackTimeout--;
        if ( CADET_PlaybackTimeout == 0 )
        {
            CADET_PlaybackWaitingForResponse = FALSE;
            CADET_PlaybackTimeout = 1;
            printf ( "\n\n\n\n\n\n\n\n\n\n\nERROR, Timeout waiting for playback response" );
        }
    }

    /*
    ** If auto flush requested by input task then start.
    **     If necessary this will start after the current interleaving mode is done.
    **
    ** This is a one shot request.  When this submode is entered, one flush command will get sent to the radio.
    */
    if ( OS_MutSemTake ( CADET_AutoFlushMutex ) == OS_SUCCESS )
    {
        if ( ( CADET_HkTelemetryPkt.StartAutoFlush == TRUE ) && ( CADET_HkTelemetryPkt.Mode == MODE_OPEN ) )
        {
            CADET_HkTelemetryPkt.StartAutoFlush = FALSE;

            CADET_HkTelemetryPkt.Mode = MODE_INTERLEAVING;
            CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;
            CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_FLUSH;

            /* Set flush parameters */
            CADET_HkTelemetryPkt.FlushFifo = FIFO_CLEAR_HIGH;
            CADET_HkTelemetryPkt.FlushPktsPerRequest = CADET_HkTelemetryPkt.HighFifoPacketsToAutoFlush;
            CADET_HkTelemetryPkt.FlushTotalPacketsToClear = CADET_HkTelemetryPkt.FlushPktsPerRequest;
            CADET_HkTelemetryPkt.FlushPacketsLeftToClear = CADET_HkTelemetryPkt.FlushPktsPerRequest;

            /* Clear playback parameters */
            CADET_HkTelemetryPkt.PlaybackFifo = 0;
            CADET_HkTelemetryPkt.PlaybackPktsPerRequest = 0;
            CADET_HkTelemetryPkt.PlaybackNextStartingPacket = 0;
            CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend = 0;
            CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend = 0; 
        }
        OS_MutSemGive ( CADET_AutoFlushMutex );
    }

    /*
    ** If auto playback requested by input task then start.
    **     If necessary this will start after the current interleaving mode is done.
    **
    ** This is a one shot request.  When this submode is entered, one playback command will get sent to the radio.
    **
    ** The algorithm for starting an automated playback is:
    **     Receive command that we will initiate an automated playback.
    **     Wait 10+ seconds to ensure destination task has time to execute command and update HK telemetry.
    **         During this 10+ second wait if a new command is received then restart counter.
    **         This allows ground to send commands fast without worrying about a playback turning on the transmitter.
    **     After 10+ second timeout then wait for HI fifo VCDU to be output to the radio.
    **         This does assume hi FIFO VCDUs are not backed up.
    **     After hi FIFO VCDU goes out and back in open mode start playback.
    */
    if ( OS_MutSemTake ( CADET_AutoPlaybackMutex ) == OS_SUCCESS )
    {
        if ( CADET_HkTelemetryPkt.StartAutoPlayback == TRUE )
        {
            CADET_HkTelemetryPkt.StartAutoPlayback = FALSE;
            CADET_HkTelemetryPkt.StartAutoPlaybackCountdown = DELAY_TO_AUTOSTART_PLAYBACK;
            CADET_HkTelemetryPkt.StartAutoPlaybackWaitForHiVCDU = FALSE;
            CADET_HkTelemetryPkt.StartAutoPlaybackConfigureForPlayback = FALSE;
        }
        OS_MutSemGive ( CADET_AutoPlaybackMutex );
    }

    if ( CADET_HkTelemetryPkt.StartAutoPlaybackCountdown != 0 )
    {
        CADET_HkTelemetryPkt.StartAutoPlaybackCountdown--;
        if ( CADET_HkTelemetryPkt.StartAutoPlaybackCountdown == 0 )
        {
            CADET_HkTelemetryPkt.StartAutoPlaybackWaitForHiVCDU = TRUE;
        }
    }

    if ( ( CADET_HkTelemetryPkt.StartAutoPlaybackConfigureForPlayback == TRUE ) && ( CADET_HkTelemetryPkt.Mode == MODE_OPEN ) )
    {
        CADET_HkTelemetryPkt.StartAutoPlaybackConfigureForPlayback = FALSE;
        CADET_HkTelemetryPkt.StartAutoPlayback = FALSE;
        CADET_HkTelemetryPkt.StartAutoPlaybackCountdown = 0;
        CADET_HkTelemetryPkt.StartAutoPlaybackWaitForHiVCDU = FALSE;

        CADET_HkTelemetryPkt.Mode = MODE_INTERLEAVING;
        CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;
        CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_PB_FLUSH_HI;

        /* Clear flush parameters */
        CADET_HkTelemetryPkt.FlushFifo = FIFO_CLEAR_HIGH;
        CADET_HkTelemetryPkt.FlushPktsPerRequest = CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback;
        CADET_HkTelemetryPkt.FlushTotalPacketsToClear = CADET_HkTelemetryPkt.FlushPktsPerRequest;
        CADET_HkTelemetryPkt.FlushPacketsLeftToClear = CADET_HkTelemetryPkt.FlushPktsPerRequest;

        /* Set playback parameters */
        CADET_HkTelemetryPkt.PlaybackFifo = FIFO_REQUEST_HI_FIFO;
        CADET_HkTelemetryPkt.PlaybackPktsPerRequest = CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback;
        CADET_HkTelemetryPkt.PlaybackNextStartingPacket = 1;
        CADET_HkTelemetryPkt.PlaybackTotalPacketsToSend = CADET_HkTelemetryPkt.PlaybackPktsPerRequest;
        CADET_HkTelemetryPkt.PlaybackPacketsLeftToSend = CADET_HkTelemetryPkt.PlaybackPktsPerRequest; 
    }

}



/*
** Function: CADET_Output_Initialize
**
** Purpose: Initialize the output processing including creating radio pipe, initializing VCDU library, and creating mutexes.
**
*/
int32 CADET_Output_Initialize ( void )
{
    int32              Result = CFE_SUCCESS;

    /*
    ** Initialize file global variables
    */
    CADET_PlaybackWaitingForResponse = FALSE;
    CADET_PlaybackTimeout = 1;
    CADET_AutoFlushMutex = 0;
    CADET_AutoPlaybackMutex = 0;
    ExecutionLoop = 0;

    /* 
    ** Create CADET Radio Command Pipe 
    ** This will allow the app to queue up high priority commands to the Cadet radio.
    ** Does not have to be big, since there will not be many commands to the cadet
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CFE_SB_CreatePipe ( &CADET_RadioCommandPipe, CADET_RADIO_PIPE_DEPTH, "CADET_RADIO_SWPIPE" );
        if (Result!= CFE_SUCCESS)
        {
            CFE_EVS_SendEvent ( CADET_TLMPIPE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Can't create cadet command pipe status %i", __LINE__, Result );
        }
    }

    /* 
    ** Subscriptions for Cadet Radio Commands 
    **     Radio Commands
    **     But only if creation of cadet radio command pipe was successful.
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = CFE_SB_Subscribe ( CADET_APP_RADIO_CMD_MID, CADET_RadioCommandPipe );
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent ( CADET_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,"L%d CADET: Can't subscribe to stream 0x%x status %i", __LINE__, CADET_APP_RADIO_CMD_MID, Result );
        }
    }

    /*
    ** Initialized VCDU buffer library
    */
    if ( Result == CFE_SUCCESS )
    {
        CADET_VCDU_InitailizeVcduBufferLibrary ( );
    }

    /*
    ** Create CADET Auto Flush Mutex.
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = OS_MutSemCreate ( &CADET_AutoFlushMutex,  CADET_AUTO_FLUSH_MUTEX_NAME, 0 );
        if ( Result != CFE_SUCCESS )
        {        
            CFE_EVS_SendEvent ( CADET_CREATE_MUTEX_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Can't create flush mutex, status %i", __LINE__, Result );
        }
    }

    /*
    ** Create CADET Auto Playback Mutex.
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = OS_MutSemCreate ( &CADET_AutoPlaybackMutex,  CADET_AUTO_PLAYBACK_MUTEX_NAME, 0 );
        if ( Result != CFE_SUCCESS )
        {        
            CFE_EVS_SendEvent ( CADET_CREATE_MUTEX_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Can't create playback mutex, status %i", __LINE__, Result );
        }
    }

    return ( Result );
}



/************************/
/*  End of File Comment */
/************************/
