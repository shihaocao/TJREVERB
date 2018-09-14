/*
** File: cadet_input.c
**
** Purpose:
**     This file contains the source code for the radio input routines, initialization routine, and 
**     interface functions.
**     NOTE: The input processing is largely asynchronous, since it is driven
**           by the USART receive callback on the Nanomind.
**
*/



/*
**   Includes:
*/
#include "cfe.h"
#include "cadet_app.h"
#include "cadet_hk.h"
#include "cadet_output.h"
#include "cadet_radio.h"
#include "cadet_msg.h"
#include "cadet_events.h"
#include "cadet_input.h"
#include "dhl_lib.h"



/*
** Defines
*/
#define CADET_HEADER_SEARCH    1
#define CADET_HEADER_PREFIX    2
#define CADET_HEADER_BUILD     3
#define CADET_HEADER_COMPLETE  4
#define CADET_PAYLOAD_BUILD    5



/*
** Structures
*/



/*
** Exported Variables
*/



/*
** Local Variables (could be defined static)
*/
uint8           CADET_HeaderBuffer[16];
uint8           CADET_PayloadBuffer[128];
uint8           CADET_HeaderPointer;
uint8           CADET_PayloadPointer;
uint8           CADET_PayloadLength;
uint8           CADET_InputState;



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
** Function: CADET_ShouldAutoFlushBeStarted
**
** Purpose: Determine if an auto flush should be started.
**
** The cadet status packet indicates the size of the hi fifo.
** If enough data on hi fifo then start auto flush.
** NOTE: If by chance we are in a mode that doesn't allow us to flush the hi fifo then just skip.  We'll catch it next time around.
*/
uint8 CADET_ShouldAutoFlushBeStarted ( CadetStatusPayload_t* CadetStatusPayloadPtr )
{
    uint8                     FlushStatus = FALSE;  /* Assume no auto flush */

    if ( ( CADET_HkTelemetryPkt.HighFifoMaxSizeForAutoFlush == 0 ) || ( CADET_HkTelemetryPkt.HighFifoPacketsToAutoFlush == 0 ) )
    {   /* Zero means auto flush is disabled */
        FlushStatus = FALSE;
    }
    else if ( ( ( CadetStatusPayloadPtr->HiFifoUsage / CADET_FLASH_PACKET_SIZE ) > CADET_HkTelemetryPkt.HighFifoMaxSizeForAutoFlush ) && ( CADET_HkTelemetryPkt.Mode == MODE_OPEN ) )
    {
        FlushStatus  = TRUE;
    }

    return ( FlushStatus );
}



/*
** Function: CADET_ShouldAutoPlaybackBeStarted
**
** Purpose: Determine if an auto playback should be started.
** 
** In general the answer is yes, but there are exceptions.
**    If the nubmer of packet to auto playback is 0 then this means the auto playback is disabled.
**    If the message ID is for cadet commands relayed through the FSW then do not start an auto playback.
**        We do not know what these commands are telling the radio to do.
**    IF the command is a cadet FSW application command that starts a hi fifo playback then do not start an auto playback.
*/
uint8 CADET_ShouldAutoPlaybackBeStarted ( CFE_SB_Msg_t *MsgPtr )
{
    CFE_SB_MsgId_t            MsgId          = CFE_SB_GetMsgId ( MsgPtr );
    uint16                    CommandCode    = CFE_SB_GetCmdCode ( MsgPtr );
    uint8                     PlaybackStatus = TRUE;  /* Assume auto playback */
    CADET_StartPlaybackCmd_t* PktPtr         = (CADET_StartPlaybackCmd_t *) MsgPtr;

    if ( CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback == 0 )
    {   /* Zero means auto playback is disabled */
        PlaybackStatus = FALSE;
    }
    else if ( MsgId == CADET_APP_RADIO_CMD_MID )
    {
        PlaybackStatus = FALSE;
    }
    else if ( MsgId == CADET_APP_CMD_MID )
    {
        if ( ( ( CommandCode == CADET_APP_START_PLAYBACK_CC ) && ( ( PktPtr->PlaybackFifo & FIFO_REQUEST_HI_FIFO ) == FIFO_REQUEST_HI_FIFO ) ) || 
             ( CommandCode == CADET_APP_PB_FLUSH_HI_FIFO_CC ) || 
             ( CommandCode == CADET_APP_PB_LO_HI_FIFO_CC    ) )
        {
            PlaybackStatus = FALSE;
        }
    }

    return ( PlaybackStatus );    
}



/*
** Function: CADET_ProcessPayload
**
** Purpose: Process the received Payload message (we have the whole thing)
**
*/
void CADET_ProcessPayload ( uint32 MessageId )
{
    CadetResultResponsePayload_t  *CadetResponsePayloadPtr; 
    CadetStatusPayload_t          *CadetStatusPayloadPtr;
    CadetConfigurationPayload_t   *CadetConfigurationPayloadPtr; 
 
    /*
    ** Determine the message type 
    */
    if ( MessageId == CADET_PKT_MSGID_RESULT_RESPONSE )
    {
        /*
        ** Copy packet to cadet application housekeeping data.
        ** If by chance we were waiting for this response packet, indicate it was received.
        */
        CadetResponsePayloadPtr = (CadetResultResponsePayload_t *) CADET_PayloadBuffer;
        printf(" RESULT_RESPONSE:    Command Result:   %d    Extended Results: %d    Message Id:       %d\n",
               (int)CadetResponsePayloadPtr->CommandResult, (int)CadetResponsePayloadPtr->ExtendedResults, (int)CadetResponsePayloadPtr->MessageId );
        CADET_HkTelemetryPkt.RadioData.cadet_result_response_packets_received++;
        CADET_HkTelemetryPkt.StatusResponseTimeout = 0;
        CFE_PSP_MemCpy ( &CADET_HkTelemetryPkt.ResultResponse, CadetResponsePayloadPtr, sizeof ( CadetResultResponsePayload_t ) );
        CADET_PlaybackWaitingForResponse = FALSE;
    }
    else if ( MessageId == CADET_PKT_MSGID_IS_STATUS )
    { 
        /*
        ** Determine if an auto flush of the hi FIFO should be started.
        ** Copy packet to cadet application housekeeping data.
        */
        CadetStatusPayloadPtr = (CadetStatusPayload_t *) CADET_PayloadBuffer;
        if ( OS_MutSemTake ( CADET_AutoFlushMutex ) == OS_SUCCESS )
        {
            CADET_HkTelemetryPkt.StartAutoFlush = CADET_ShouldAutoFlushBeStarted ( CadetStatusPayloadPtr );
            OS_MutSemGive ( CADET_AutoFlushMutex );
        }
        CADET_HkTelemetryPkt.RadioData.cadet_status_packets_received++;
        CADET_HkTelemetryPkt.StatusResponseTimeout = 0;
        CFE_PSP_MemCpy ( &CADET_HkTelemetryPkt.StatusInfo, CadetStatusPayloadPtr, sizeof ( CadetStatusPayload_t ) );
    }
    else if ( MessageId == CADET_PKT_MSGID_IS_CONFIGURATION )
    { 
        /*
        ** Copy packet to cadet application housekeeping data.
        */
        CadetConfigurationPayloadPtr = (CadetConfigurationPayload_t *) CADET_PayloadBuffer;
        CADET_HkTelemetryPkt.RadioData.cadet_configuration_packets_received++;
        CADET_HkTelemetryPkt.StatusResponseTimeout = 0;
        CFE_PSP_MemCpy ( &CADET_HkTelemetryPkt.ConfigurationInfo, CadetConfigurationPayloadPtr, sizeof ( CadetConfigurationPayload_t ) );
    }
    else if ( MessageId == CADET_PKT_MSGID_IS_VERSION )
    {
        /*
        ** Copy packet to cadet application housekeeping data.
        */
        CADET_HkTelemetryPkt.RadioData.cadet_version_packets_received++;
        CADET_HkTelemetryPkt.StatusResponseTimeout = 0;
        CFE_PSP_MemCpy ( &CADET_HkTelemetryPkt.VersionInfo, &CADET_PayloadBuffer[0], sizeof ( CadetIsVersionPayload_t ) );
    }
    else if ( MessageId == CADET_PKT_MSGID_RELAY_DATA )
    {
        /*
        ** Received a nanomind FSW command.
        ** Determine if automatic hi fifo playback should be started.
        ** Validate command secondary header checksum.
        ** Forward to destination via software bus.
        */
        printf ( "RELAY_DATA: %02X%02X %02X%02X %02X%02X %02X%02X\n", CADET_PayloadBuffer[0], CADET_PayloadBuffer[1], CADET_PayloadBuffer[2], CADET_PayloadBuffer[3], 
                                                                      CADET_PayloadBuffer[4], CADET_PayloadBuffer[5], CADET_PayloadBuffer[6], CADET_PayloadBuffer[7] );
        if ( OS_MutSemTake ( CADET_AutoPlaybackMutex ) == OS_SUCCESS )
        {
            CADET_HkTelemetryPkt.StartAutoPlayback = CADET_ShouldAutoPlaybackBeStarted ( (CFE_SB_Msg_t *) &CADET_PayloadBuffer[0] );
            OS_MutSemGive ( CADET_AutoPlaybackMutex );
        }
        if ( CCSDS_ValidCheckSum ( (CCSDS_CmdPkt_t *) &CADET_PayloadBuffer[0] ) == TRUE )
        {  
            CFE_SB_SendMsg( (CFE_SB_Msg_t *) &CADET_PayloadBuffer[0] );  
        }
        else
        {
            printf ( "\n\n\n\n ERROR: Invalid Checksum received: MsgID = 0x%02X%02X, CC = %02X, CS = %02X\n", CADET_PayloadBuffer[0], CADET_PayloadBuffer[1], CADET_PayloadBuffer[6], CADET_PayloadBuffer[7] );
            CADET_HkTelemetryPkt.RadioData.cadet_error_packets_received++;
        }
        CADET_HkTelemetryPkt.RadioData.cadet_relay_packets_received++;
        CADET_HkTelemetryPkt.StatusResponseTimeout = 0;
    }
    else
    {
        /*
        ** Unknown message ID. 
        ** Nothing to do but drop data/packet.
        */
        printf ( "\n\n\n\n ERROR: Unknown Input Message ID: %d\n", (int)MessageId );
        CADET_HkTelemetryPkt.RadioData.cadet_error_packets_received++;
    }
}



/*
** Function: CADET_UsartCallback
**
** Purpose: Provides a state machine to process the response from the CADET radio.
**
*/
void CADET_UsartCallback ( uint8 *buf, int len, void *pxTaskWoken )
{
    CadetMessageHeader_t  *CadetMessageHdrPtr; 
    int                    buf_idx = 0;

    /* 
    ** Process each character given to the callback 
    */
    while ( buf_idx < len )
    {
        if ( CADET_InputState == CADET_HEADER_SEARCH )  
        {
            if ( buf[buf_idx] == 0xAF ) 
            {
                CADET_HeaderBuffer[CADET_HeaderPointer] = 0x0A;
                CADET_InputState = CADET_HEADER_PREFIX;
                CADET_HeaderPointer ++;
            } 
            buf_idx++;
        }
        else if ( CADET_InputState == CADET_HEADER_PREFIX )
        { 
            /* 
            ** The next character has to be the second prefix char . If not, reset
            */
            if ( buf[buf_idx] == 0x0A )
            {
                CADET_HeaderBuffer[CADET_HeaderPointer] = 0xAF;
                CADET_InputState = CADET_HEADER_BUILD;      
                CADET_HeaderPointer ++;
                buf_idx++;
            }
            else
            {   /* Don't increment buf idx, might be first byte of header */
                CADET_HeaderPointer = 0;
                CADET_InputState = CADET_HEADER_SEARCH;
            }
        }
        else if ( CADET_InputState == CADET_HEADER_BUILD )
        {
            /*
            ** the next characters have to be the cadet message header.
            */
            CADET_HeaderBuffer[CADET_HeaderPointer] = buf[buf_idx];
            CADET_HeaderPointer++;
            buf_idx++;

            if ( CADET_HeaderPointer == sizeof(CadetMessageHeader_t) )
            {
                CadetMessageHdrPtr = (CadetMessageHeader_t *) CADET_HeaderBuffer;
                CADET_PayloadLength = CadetMessageHdrPtr->PayloadLength;

                if ( CADET_PayloadLength == 0 )
                {
                    CADET_HeaderPointer = 0;
                    CADET_InputState = CADET_HEADER_SEARCH;
                }
                else 
                {
                    if ( CADET_PayloadLength > 128 ) 
                    {
                        printf ("\n\n\n\n Bad Payload Length %d, aborting current packet.\n\n\n\n", CADET_PayloadLength );
                        CADET_HeaderPointer = 0;
                        CADET_InputState = CADET_HEADER_SEARCH;
                    }
                    else
                    {
                        CADET_PayloadPointer = 0; 
                        CADET_InputState = CADET_PAYLOAD_BUILD;
                    }
                }
            } 
        }
        else if ( CADET_InputState == CADET_PAYLOAD_BUILD ) 
        {
            /*
            ** the next characters have to be the payload message.
            */
            CADET_PayloadBuffer[CADET_PayloadPointer] = buf[buf_idx];
            CADET_PayloadPointer++;
            buf_idx++;

            if ( CADET_PayloadPointer > 128 ) 
            {
                printf ("\n\n\n\n Bad Payload Pointer %d, aborting current packet.\n\n\n\n", CADET_PayloadPointer );
                CADET_HeaderPointer = 0;
                CADET_InputState = CADET_HEADER_SEARCH;
            }

            if ( CADET_PayloadPointer == CADET_PayloadLength )
            {
                CadetMessageHdrPtr = (CadetMessageHeader_t *) CADET_HeaderBuffer;
                CADET_ProcessPayload ( CadetMessageHdrPtr->MessageId );
                CADET_InputState = CADET_HEADER_SEARCH;
                CADET_PayloadPointer = 0;
                CADET_HeaderPointer = 0;
                CADET_PayloadLength = 0;
            } 
        }
        else
        {
            printf ("\n\n\n\n Bad Input State.\n\n\n\n" );
            CADET_HeaderPointer = 0;
            CADET_InputState = CADET_HEADER_SEARCH;
        }

    } /* end while */

}



/*
** Function: CADET_Input_Initialize
**
** Purpose: Initialize the radio interface including opening the library L3C serial port and installing the callback function.
**
*/
int32 CADET_Input_Initialize ( void )
{
    int32              Result = CFE_SUCCESS;
    int32              status;

    /*
    ** Initialize file variables prior to installing call back function.
    */
    CADET_HeaderPointer = 0;
    CADET_PayloadPointer = 0;
    CADET_PayloadLength = 0;
    CADET_InputState = CADET_HEADER_SEARCH;

    /*
    ** Open the USART
    */
    if ( Result == CFE_SUCCESS )
    {
        status = L3C_OpenSerialPort ( );
        if ( status != 0 ) 
        {
            CFE_EVS_SendEvent ( CADET_UNABLE_TO_OPEN_SERIAL_PORT_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Unable to open L3C serial port %i", __LINE__, Result );
            Result = !CFE_SUCCESS;
        }
    }

    /*
    ** Install the USART callback function to receive data from the CADET
    */
    if ( Result == CFE_SUCCESS )
    {
        status = L3C_InstallCallback ( CADET_UsartCallback );
        if ( status != 0 ) 
        {
            CFE_EVS_SendEvent ( CADET_UNABLE_TO_SET_SERIAL_CALLBACK_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Unable to install L3C callback function %i", __LINE__, Result );
            Result = !CFE_SUCCESS;
        }
    }

    return ( Result );
}



/************************/
/*  End of File Comment */
/************************/

