/*
** File: cadet_radio.c
**
** Purpose:
**     This file contains the source code for the radio routines, initialization routine, and 
**     interface functions.
**
*/



/*
**   Includes:
*/
#include "cfe.h"
#include "cadet_radio.h"
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



/*
** Local Variables (could be defined static)
*/



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
** Function: CADET_GetVersionCmd
**
** Purpose: This function Queries the CADET radio for the version number
**
*/
void CADET_GetVersionCmd ( void )
{
    CadetMessageHeader_t         CadetMessageHdr; 
    uint32                       ComputedCrc;

    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = CADET_PKT_MSGID_GET_VERSION;
    CadetMessageHdr.PayloadLength = 0;

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), NULL, 0 );
}



/*
** Function: CADET_GetStatusCmd
**
** Purpose: This function requests the CADET status
**
*/
void CADET_GetStatusCmd ( void )
{
    CadetMessageHeader_t         CadetMessageHdr; 
    uint32                       ComputedCrc;

    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = CADET_PKT_MSGID_REQUEST_STATUS;
    CadetMessageHdr.PayloadLength = 0;

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), NULL, 0 );
}



/*
** Function: CADET_GetConfigCmd
**
** Purpose: This function gets the configuration from the cadet
**
*/
void CADET_GetConfigCmd ( void )
{
    CadetMessageHeader_t         CadetMessageHdr; 
    uint32                       ComputedCrc;

    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = CADET_PKT_MSGID_GET_CONFIGURATION;
    CadetMessageHdr.PayloadLength = 0;

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), NULL, 0 );

}



/*
** Function: CADET_SetTimeCmd
**
** Purpose: This function sets the time on the CADET
**
*/
void CADET_SetTimeCmd ( CadetSetTimePayload_t* SetTimePayload )
{
    CadetMessageHeader_t   CadetMessageHdr;
    uint32                 ComputedCrc;

    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = CADET_PKT_MSGID_SET_TIME;
    CadetMessageHdr.PayloadLength = sizeof(CadetSetTimePayload_t);

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), (uint8 *) SetTimePayload, sizeof(CadetSetTimePayload_t) );
}



/*
** Function: CADET_ResetRadioCmd
**
** Purpose: This function sends the command to resets the Radio
**
*/
void CADET_ResetRadioCmd ( CadetResetPayload_t* ResetPayload )
{
    CadetMessageHeader_t         CadetMessageHdr;
    uint32                       ComputedCrc;

    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = CADET_PKT_MSGID_RESET;
    CadetMessageHdr.PayloadLength = sizeof(CadetResetPayload_t);

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), (uint8 *) ResetPayload, sizeof(CadetResetPayload_t) );
}



/*
** Function: CADET_FifoRequestCmd
**
** Purpose: This function sends a FIFO request command to the Radio
**
*/
void CADET_FifoRequestCmd ( CadetFifoRequestPayload_t* FifoRequestPayload )
{
    CadetMessageHeader_t         CadetMessageHdr;
    uint32                       ComputedCrc;
    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = CADET_PKT_MSGID_FIFO_REQUEST;
    CadetMessageHdr.PayloadLength = sizeof(CadetFifoRequestPayload_t);

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), (uint8 *) FifoRequestPayload, sizeof(CadetFifoRequestPayload_t) );
}



/*
** Function: CADET_FifoClearCmd
**
** Purpose: This function sends the command to clear the FIFO on the Radio
**
*/
void CADET_FifoClearCmd ( CadetFifoClearPayload_t* FifoClearPayload )
{
    CadetMessageHeader_t         CadetMessageHdr;
    uint32                       ComputedCrc;

    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = CADET_PKT_MSGID_FIFO_CLEAR;
    CadetMessageHdr.PayloadLength = sizeof(CadetFifoClearPayload_t);

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), (uint8 *) FifoClearPayload, sizeof(CadetFifoClearPayload_t) );

}



/*
** Function: CADET_AuthEnableCmd
**
** Purpose: This function will enable command authentication on the Radio
**
*/
void CADET_AuthEnableCmd ( CadetAuthEnablePayload_t* AuthEnablePayload )
{
    CadetMessageHeader_t         CadetMessageHdr;
    uint32                       ComputedCrc;

    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = CADET_PKT_MSGID_AUTHENTICATION_ENABLE;
    CadetMessageHdr.PayloadLength = sizeof(CadetAuthEnablePayload_t);

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), (uint8 *) AuthEnablePayload,    sizeof(CadetAuthEnablePayload_t) );
}



/*
** Function: CADET_AuthDisableCmd
**
** Purpose: This function will disable the command authentication on the Radio
**
*/
void CADET_AuthDisableCmd ( void )
{
    CadetMessageHeader_t   CadetMessageHdr; 
    uint32                 ComputedCrc;

    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = CADET_PKT_MSGID_AUTHENTICATION_DISABLE;
    CadetMessageHdr.PayloadLength = 0;

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), NULL, 0 );
}



/*
** Function: CADET_FifoRequestExtCmd
**
** Purpose: This function sends a FIFO request command to the Radio
**
*/
void CADET_FifoRequestExtCmd ( CadetFifoRequestExtPayload_t* FifoRequestExtPayload )
{
    CadetMessageHeader_t         CadetMessageHdr;
    uint32                       ComputedCrc;

    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = CADET_PKT_MSGID_FIFO_REQUEST_EXT;
    CadetMessageHdr.PayloadLength = sizeof(CadetFifoRequestExtPayload_t);

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), (uint8 *) FifoRequestExtPayload, sizeof(CadetFifoRequestExtPayload_t) );
}



/*
** Function: CADET_FifoAdd
**
** Purpose: This function sends to the specified FIFO (specified by the parameter message ID).
**     NOTE: The caller must specify the size as this function only knows the maximum value.
**
*/
void CADET_FifoAdd ( uint16 MessageID, uint8* PayloadDataPtr, uint32 PayloadSize )
{
    CadetMessageHeader_t         CadetMessageHdr;
    uint32                       ComputedCrc;

    /*
    ** Format the message header
    */
    CadetMessageHdr.Prefix = 0x0AAF;
    CadetMessageHdr.Destination = CADET_PKT_ADDRESS_CADET; 
    CadetMessageHdr.Source = CADET_PKT_ADDRESS_OBC; 
    CadetMessageHdr.MessageId = MessageID;
    CadetMessageHdr.PayloadLength = PayloadSize;

    /* 
    ** Compute CRC 
    */
    ComputedCrc = L3C_crc32 ( (uint8 *) &(CadetMessageHdr), sizeof(CadetMessageHeader_t) );
    CadetMessageHdr.HeaderCRC = ComputedCrc;
    CadetMessageHdr.HeaderCRC = 0;   /* TODO 0 means don't check crc */

    /*
    ** Send the message
    */
    ( void ) L3C_SendMessageToRadio ( (uint8 *) &CadetMessageHdr, sizeof(CadetMessageHeader_t), PayloadDataPtr, PayloadSize );
}



/************************/
/*  End of File Comment */
/************************/
