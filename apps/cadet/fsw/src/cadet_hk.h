/*
** File:
**   cadet_hk.h 
**
** Purpose: 
**  Define CADET App Housekeeping Messages, exported variables, and interface routines.
**
*/
#ifndef _cadet_hk_h_
#define _cadet_hk_h_



/*
**   Includes:
*/
#include "cfe.h"
#include "cadet_radio.h"
#include "shk_msg.h"



/*
** Defines
*/
#define MODE_OPEN                ( 0 )
#define MODE_INTERLEAVING        ( 1 )

#define SUBMODE_NONE             ( 0 )
#define SUBMODE_DATA             ( 1 )
#define SUBMODE_CMD              ( 2 )

#define SUBMODE_CMD_NONE         ( 0 )
#define SUBMODE_CMD_PLAYBACK     ( 1 )
#define SUBMODE_CMD_FLUSH        ( 2 )
#define SUBMODE_CMD_FLUSH_HI_ALL ( 3 )
#define SUBMODE_CMD_PB_FLUSH_HI  ( 4 )
#define SUBMODE_CMD_PB_LOW_HI    ( 5 )

#define PLAYBACK_NONE            ( 0 )
#define PLAYBACK_STARTED         ( 1 )
#define PLAYBACK_FINISHED        ( 2 )

#define DEFAULT_MAXIMUM_SIZE_TO_AUTO_FLUSH      ( 50 )   /* When hi fifo has this many cadet packet we'll auto flush some */
#define DEFAULT_NUMBER_PACKETS_TO_AUTO_FLUSH    ( 25 )   /* This is the number we'll auto flush */
#define DEFAULT_NUMBER_PACKETS_TO_AUTO_PLAYBACK ( 75 )   /* Make sure get all in high FIFO */



/*
** Structures
*/
/* This structure is for the housekeeping data from the Cadet Radio. */
typedef struct
{
   uint32      cadet_high_bytes_sent;
   uint32      cadet_high_bytes_per_sec;

   uint32      cadet_low_bytes_sent;
   uint32      cadet_low_bytes_per_sec;

   uint32      cadet_command_packets_sent;

   uint16      cadet_result_response_packets_received;
   uint16      cadet_version_packets_received;
   uint16      cadet_status_packets_received;
   uint16      cadet_configuration_packets_received;
   uint16      cadet_relay_packets_received;
   uint16      cadet_error_packets_received;

} OS_PACK  CADET_RadioData_t;

/* Type definition (CADET App housekeeping) */
typedef struct 
{
    uint8                         TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8                         command_count;
    uint8                         command_error_count;

    uint8                         Mode;
    uint8                         Submode;
    uint8                         SubmodeCommand;
    uint8                         StartAutoFlush;
    uint8                         FlushFifo;
    uint8                         PlaybackFifo;

    uint8                         StartAutoPlayback;
    uint8                         StartAutoPlaybackCountdown;
    uint8                         StartAutoPlaybackWaitForHiVCDU;
    uint8                         StartAutoPlaybackConfigureForPlayback;

    uint32                        PlaybackPktsPerRequest;
    uint32                        PlaybackNextStartingPacket;
    uint32                        PlaybackTotalPacketsToSend;
    uint32                        PlaybackPacketsLeftToSend;

    uint32                        FlushPktsPerRequest;
    uint32                        FlushTotalPacketsToClear;
    uint32                        FlushPacketsLeftToClear;

    uint32                        PlaybackNextStartingPacketHi;
    uint32                        PlaybackNextStartingCycle;

    uint16                        NumberOfPacketSubscriptions;
    uint8                         TableVerifySuccessCount;
    uint8                         TableVerifyFailureCount;

    uint16                        HighFifoPacketsDropped;
    uint16                        LowFifoPacketsDropped;

    uint16                        HighFifoMaxSizeForAutoFlush;
    uint16                        HighFifoPacketsToAutoFlush;
    uint16                        HighFifoPacketsToAutoPlayback; 
    uint16                        FDLSempahoresInUse;

    CADET_RadioData_t             RadioData;

    /*
    ** The status, configuration, version, and error response info from the radio itself
    */
    CadetIsVersionPayload_t       VersionInfo;
    CadetConfigurationPayload_t   ConfigurationInfo;
    CadetStatusPayload_t          StatusInfo;
    CadetResultResponsePayload_t  ResultResponse;
       
    char                          CurrentTableFilename[OS_MAX_PATH_LEN]; 

    int32                         StatusResponseTimeout;

}   OS_PACK CADET_HkTlmPkt_t;



/* Type definition (UART Message) */
typedef struct 
{
     uint32    RunningTimer;
     uint32    SpacecraftMode;
     uint32    DeploymentSwitchStatus;
     int32     EpsBatteryVoltage;
     int32     EpsBatteryCurrent;
     int32     Eps33Voltage;
     int32     Eps33Current;
     int32     Eps5Voltage;
     int32     Eps5Current;
     int32     Eps12Voltage;
     int32     Eps12Current;
     float     Batt1MotherboardTemp;
     float     Batt2MotherboardTemp;
     uint32    EpsSwitchStatus[EPS_SWITCH_COUNT];
     uint32    EpsStatus;
     uint32    Batt1Status;
     uint32    Batt2Status;
     uint32    CpuAverageIdle;
     uint32    ProcessorResetCounter;
     uint32    LastResetCause;
     uint32    HiFifoBytesUsed;
     uint32    LoFifoBytesUsed;

}   CADET_UartMsg_t;



/*
** Exported Variables
*/
extern CADET_HkTlmPkt_t           CADET_HkTelemetryPkt;
extern CADET_UartMsg_t            CADET_UartMsg;



/*
** Exported Functions
*/
extern void CADET_HK_SendHousekeepingPacket ( void );
extern int32 CADET_HK_InitializeHousekeepingPacket ( void );
extern uint32 CADET_HK_UartPopulateMsg ( CFE_SB_Msg_t* PktPtr );



#endif /* _cadet_hk_h_ */



/************************/
/*  End of File Comment */
/************************/
