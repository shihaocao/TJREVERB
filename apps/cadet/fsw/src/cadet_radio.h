/*
** File:
**   cadet_radio.h 
**
** Purpose: 
**  Define CADET Radio defines, messages, exported variables, and interface routines.
**
** Notes:
**      L3 Communications Systems Group Cadet-U Software Interface Control Document (ICD) Rev J Oct 20, 2014
**          (filename "5071-0017 Software Interface Control Document_revJ.pdf")
**
** Messages From Cadet App to Cadet Radio:
**
**    GET_VERSION
**         Response: IS_VERSION
**    RESET
**         Response: RESULT_RESPONSE ( has message ID of reset in payload )
**    GET_CONFIGURATION
**         Response:  IS_CONFIGURATION 
**    SET_CONFIGURATION
**         Response : IS_CONFIGURATION, or RESULT_RESPONSE
**    FIFO_ADD_HIGH ( Q )
**         Response: IS_STATUS
**    FIFO_ADD_LOW ( Q )
**         Response: IS_STATUS
**    REQUEST_STATUS
**         Response: IS_STATUS
**    FIFO_REQUEST
**         Response: IS_STATUS
**    SET_TIME
**         Response: IS_STATUS
**    AUTH_ENABLE
**         Response: IS_STATUS
**    AUTH_DISABLE
**         Response: IS_STATUS
**    FIFO_CLEAR
**         Response: IS_STATUS
**
** Messages from Cadet Radio back to Cadet App:
**
**    Most of these are in response to a message that the App sent.
**
**    IS_VERSION  ( response )
**    RESULT_RESPONSE ( response ) 
**    IS_CONFIGURATION ( response ) 
**    IS_STATUS ( response for most cmds )
**    RELAY_DATA ( Async packet from uplink ) 
**    ( handle unknown/unexpected ) 
**
**
*******************************************************************************/
#ifndef _cadet_radio_h_
#define _cadet_radio_h_



/*
**   Includes:
*/
#include "cfe.h"



/*
** Defines
*/
#define CADET_PKT_MAX_PAYLOAD_BYTES             (4096)
#define CADET_FLASH_PACKET_SIZE                 (4096) 
#define CADET_PKT_ADDRESS_GUI                   0
#define CADET_PKT_ADDRESS_OBC                   1
#define CADET_PKT_ADDRESS_CADET                 2001
#define CADET_PKT_ADDRESS_GROUND                2003

#define CADET_PKT_MSGID_GET_VERSION             0
#define CADET_PKT_MSGID_IS_VERSION              1
#define CADET_PKT_MSGID_RESULT_RESPONSE         2
#define CADET_PKT_MSGID_IS_DEBUG                3
#define CADET_PKT_MSGID_SET_DEBUG_LEVEL         4
#define CADET_PKT_MSGID_GET_DEBUG_LEVEL         5
#define CADET_PKT_MSGID_IS_DEBUG_LEVEL          6
#define CADET_PKT_MSGID_IS_DEBUG_LEVELS         7
#define CADET_PKT_MSGID_RESET                   8
#define CADET_PKT_MSGID_IS_LOG                  9
#define CADET_PKT_MSGID_GET_CONFIGURATION       10
#define CADET_PKT_MSGID_SET_CONFIGURATION       11
#define CADET_PKT_MSGID_IS_CONFIGURATION        12
#define CADET_PKT_MSGID_FIFO_ADD_HIGH           13
#define CADET_PKT_MSGID_FIFO_ADD_LOW            14
#define CADET_PKT_MSGID_RELAY_DATA              15
#define CADET_PKT_MSGID_REQUEST_STATUS          16
#define CADET_PKT_MSGID_IS_STATUS               17
#define CADET_PKT_MSGID_SET_KEYS                18
#define CADET_PKT_MSGID_FIFO_REQUEST            19
#define CADET_PKT_MSGID_SET_TIME                20
#define CADET_PKT_MSGID_AUTHENTICATION_ENABLE   21
#define CADET_PKT_MSGID_AUTHENTICATION_DISABLE  22
#define CADET_PKT_MSGID_FIFO_CLEAR              23
#define CADET_PKT_MSGID_GET_FPGA_VERSION        27
#define CADET_PKT_MSGID_GET_UNIQUE_ID           29
#define CADET_PKT_MSGID_USB_TXDATA              44
#define CADET_PKT_MSGID_FIFO_REQUEST_EXT        65



/*
** Structures
*/
/* Cadet Message Header */
typedef struct
{
    uint16      Prefix;  /* 0x0AAF */
    uint16      Destination;  
    uint16      Source; 
    uint16      MessageId;
    uint32      PayloadLength; 
    uint32      HeaderCRC;

} OS_PACK CadetMessageHeader_t;

/* ----------- DEFINE CADET PAYLOAD MESSAGE TYPES --------*/

/* RESULT_RESPONSE Payload used for a number of commands */
typedef struct
{
    uint8       CommandResult;
    uint32      ExtendedResults;
    uint32      MessageId;

} OS_PACK CadetResultResponsePayload_t;

/* IS_VERSION Payload, in response to the GET_VERSION command */
typedef struct
{
    uint16      MajorVersion;
    uint16      MinorVersion;
    uint32      BuildVersion;
    uint8       BuildIdentifier[32];

} OS_PACK CadetIsVersionPayload_t;

/* SET_DEBUG_LEVEL Payload */
typedef struct 
{
    uint8       Level;    /* See debug level defines below */
    uint8       Enabled;  /* 0 is FALSE, 1 is TRUE */

} OS_PACK CadetDebugLevelPayload_t;

/* Debug level identifiers */
#define DEBUG_LEVEL_DEFAULT         0x00
#define DEBUG_LEVEL_HOST_IO         0x01
#define DEBUG_LEVEL_MESSAGE_SYSTEM  0x02
#define DEBUG_LEVEL_HW_IF           0x03
#define DEBUG_LEVEL_USB_DRV         0x04
#define DEBUG_LEVEL_SDIO_DRV        0x05
#define DEBUG_LEVEL_APPLICATION     0x06
#define DEBUG_LEVEL_STORAGE_MGR     0x07
#define DEBUG_LEVEL_OBC_MGR         0x08
#define DEBUG_LEVEL_ALL             0xFF

/* RESET payload */
typedef struct
{
    uint8 ResetId;

} OS_PACK CadetResetPayload_t;

/* Reset identifiers */
#define RESET_ERASE_FIFOS   0x01
#define RESET_OBC           0x02
#define RESET_RECEIVER      0x04
#define RESET_TRANSMITTER   0x08
#define RESET_MODEM_CFG     0x10

/* CONFIGURATION_MESSAGE Payload */
typedef struct
{
    uint16  Version;
    uint32  Address; 
    uint32  ReceiveFrequency; 
    uint32  TransmitFrequency;
    uint32  TransmitPower;
    uint8   ActiveAESKey;
    uint32  OBCBaudRate;
    uint8   DebugPortEnable;
    uint8   USBPortEnable;
    uint8   OBCFlowControlEnable;
    uint8   AuthenticationEnable;
    uint8   UplinkDecryptionEnable;
    uint8   FECEnable; 
    uint8   DataWhiteningEnable;
    uint32  PacketSize;
    uint8   Reserved[24];
    uint32  CRC32;

} OS_PACK CadetConfigurationPayload_t;

/* Configuration identifiers */
#define SET_CONFIGURATION_SUCCESS             0
#define SET_CONFIGURATION_INVALID_CRC         1000
#define SET_CONFIGURATION_INVALID_BAUD        1001
#define SET_CONFIGURATION_INVALID_KEY_INDEX   1002
#define SET_CONFIGURATION_INVALID_PORT_ENABLE 1003
#define SET_CONFIGURATION_INVALID_USB_ENABLE  1004
#define SET_CONFIGURATION_INVALID_XMIT_POWER  1005
#define SET_CONFIGURATION_INVALID_RX_FREQ     1006
#define SET_CONFIGURATION_INVALID_TX_FREQ     1007
#define SET_CONFIGURATION_INVALID_ADDRESS     1008
#define SET_CONFIGURATION_STORAGE_FAILURE     1009

/* STATUS_MESSAGE payload */
typedef struct
{
    uint16  Temperature;
    uint32  HiFifoUsage;
    uint32  LoFifoUsage;
    uint32  StorageSpaceFree;
    uint16  RSSI;
    uint32  ModemErrorIndicator;
    uint32  Time;
    uint16  Year;
    uint8   CurrentMode;
    uint16  ReceiverTemperature;
    uint16  CurrentLevel;

} OS_PACK CadetStatusPayload_t;

/* FIFO Payload - up to 4096 bytes */
typedef struct 
{
    uint8  FifoPayload[CADET_PKT_MAX_PAYLOAD_BYTES];

} OS_PACK CadetFifoPayload_t;

/* SET_KEYS message payload */
typedef struct 
{
    uint8   Key_1[32];
    uint32  CRC32_1;
    uint8   Key_2[32];
    uint32  CRC32_2;
    uint8   Key_3[32];
    uint32  CRC32_3;

} OS_PACK CadetKeysPayload_t;

/* FIFO_REQUEST Payload */
typedef struct
{
    uint8   FLAGS;
    uint32  PacketCount;            /* 0 = all packets */
    uint32  StartingPacketNumber;   /* For retransmit , flag must be set */
    uint32  NewPacketSize;          /* How much data per chunk, flag must be set */
    uint8   TransmitPowerLevel;     /* 0 - 31, flag must be set */

} OS_PACK CadetFifoRequestPayload_t;

/* Request Identifiers */
#define FIFO_REQUEST_HI_FIFO              0x01
#define FIFO_REQUEST_LO_FIFO              0x02
#define FIFO_REQUEST_START_BLOCK_VALID    0x04
#define FIFO_REQUEST_PACKET_SIZE_VALID    0x08
#define FIFO_REQUEST_TX_POWER_LEVEL_VALID 0x10
#define FIFO_REQUEST_FLUSH_LOW_BUFFER     0x20
#define FIFO_REQUEST_USE_USB              0x40
#define FIFO_REQUEST_RESERVED             0x80

/* FIFO_REQUEST_EXT Payload */
typedef struct
{
    uint8   FLAGS;
    uint32  PacketCount;
    uint32  StartingPacketNumber;
    uint32  NewPacketSize;
    uint8   TransmitPowerLevel;
    uint8   PreambleRepeat;
    uint16  Delay;

} OS_PACK CadetFifoRequestExtPayload_t;

/* SET_TIME Payload  */
typedef struct 
{
    uint32 Time;    /* Number of seconds */
    uint16 Year;    /* Calendar year in decimal format */

} OS_PACK CadetSetTimePayload_t;

/* AUTHENTICATION_ENABLE Payload */
typedef struct
{
    uint8  ActiveKeySlot;

} OS_PACK CadetAuthEnablePayload_t;

/* FIFO_CLEAR Payload  */
typedef struct
{
    uint8  FLAGS;
    uint32 PacketCount;
    uint32 StartingPacket;
    uint32 NewPacketSize;

} OS_PACK CadetFifoClearPayload_t;

/* Clear identifiers */
#define FIFO_CLEAR_HIGH           0x01
#define FIFO_CLEAR_LOW            0x02
#define FIFO_CLEAR_START_VALID    0x04
#define FIFO_CLEAR_NEW_SIZE_VALID 0x08
#define FIFO_CLEAR_RESERVED       0x10
#define FIFO_CLEAR_FLUSH_LOW      0x20
#define FIFO_CLEAR_RESERVED2      0x40
#define FIFO_CLEAR_RESERVED3      0x80

/* ----------- DEFINE CADET MESSAGES -----------------*/

/* Cadet Fifo Message type */
typedef struct 
{
    CadetMessageHeader_t Header;
    CadetFifoPayload_t   Payload;

} OS_PACK CadetFifoMessage_t;

/* Cadet Reset message  */
typedef struct
{
    CadetMessageHeader_t   Header;
    CadetResetPayload_t    Payload;

} OS_PACK CadetResetMessage_t;

/* Get configuration message does not have a payload  */

/* Request status message does not have a payload */

/* FIFO Request message */
typedef struct
{
    CadetMessageHeader_t      Header;
    CadetFifoRequestPayload_t Payload;

} OS_PACK CadetFifoRequestMessage_t;
   
/* Set time message */
typedef struct
{
    CadetMessageHeader_t  Header;
    CadetSetTimePayload_t Payload;

} OS_PACK CadetSetTimeMessage_t;

/* Authentication Enable  */
typedef struct
{
    CadetMessageHeader_t     Header;
    CadetAuthEnablePayload_t Payload;

} OS_PACK CadetAuthEnableMessage_t;

/* Authentication Disable does not have a playload */

/* FIFO Clear */
typedef struct
{
    CadetMessageHeader_t     Header;
    CadetFifoClearPayload_t  Payload;

} OS_PACK CadetFifoClearMessage_t;

/* Get FPGA Version does not have a playload */

/* Get Unique ID does not have a playload */

/* FIFO Request message  */
typedef struct
{
    CadetMessageHeader_t         Header;
    CadetFifoRequestExtPayload_t Payload;

} OS_PACK CadetFifoRequestExtMessage_t;



/*
** Exported Variables
*/



/*
** Exported Functions
*/
extern void CADET_GetVersionCmd ( void );
extern void CADET_GetStatusCmd ( void );
extern void CADET_GetConfigCmd ( void );
extern void CADET_SetTimeCmd ( CadetSetTimePayload_t* SetTimePayload );
extern void CADET_ResetRadioCmd ( CadetResetPayload_t* ResetPayload );
extern void CADET_FifoRequestCmd ( CadetFifoRequestPayload_t* FifoRequestPayload );
extern void CADET_FifoClearCmd ( CadetFifoClearPayload_t* FifoClearPayload );
extern void CADET_AuthEnableCmd ( CadetAuthEnablePayload_t* AuthEnablePayload );
extern void CADET_AuthDisableCmd ( void );
extern void CADET_FifoRequestExtCmd ( CadetFifoRequestExtPayload_t* FifoRequestExtPayload );
extern void CADET_FifoAdd ( uint16 MessageID, uint8* PayloadDataPtr, uint32 PayloadSize );


#endif /* _cadet_radio_h_ */



/************************/
/*  End of File Comment */
/************************/
