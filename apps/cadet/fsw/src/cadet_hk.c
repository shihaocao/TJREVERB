/*
** File: cadet_hk.c
**
** Purpose:
**     This file contains the source code for the CADET houskeeping routines, initialization routine, and 
**     interface functions.
**
*/



/*
**   Includes:
*/
#include "string.h"
#include "cfe.h"
#include "cadet_app.h"
#include "cadet_hk.h"
#include "cadet_cds.h"

#include "hs_msg.h"
#include "shk_msg.h"
#include "cfe_es_msg.h"



/*
** Defines
*/



/*
** Structures
*/



/*
** Exported Variables
*/
CADET_HkTlmPkt_t         CADET_HkTelemetryPkt;
CADET_UartMsg_t          CADET_UartMsg;



/*
** Local Variables (could be defined static)
*/
uint32                   CADET_SecondsAtStart;



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
** Function: CADET_HK_SendHousekeepingPacket
**
** Purpose: This function will gather remaining HK data then timestamp and send
**     the HK packet on the software bus.
**
*/
void CADET_HK_SendHousekeepingPacket ( void )
{
    CFE_TIME_SysTime_t  TempTime;
    uint32              SecondsUp;
    OS_count_sem_prop_t count_prop;

    TempTime = CFE_TIME_GetTime ( );
    SecondsUp = TempTime.Seconds  - CADET_SecondsAtStart;

    if ( SecondsUp > 0 )
    {
       CADET_HkTelemetryPkt.RadioData.cadet_high_bytes_per_sec = CADET_HkTelemetryPkt.RadioData.cadet_high_bytes_sent / SecondsUp;
       CADET_HkTelemetryPkt.RadioData.cadet_low_bytes_per_sec = CADET_HkTelemetryPkt.RadioData.cadet_low_bytes_sent / SecondsUp;
    }

    /*
    ** Report the current filter table filename
    */
    CFE_PSP_MemCpy ( &CADET_HkTelemetryPkt.CurrentTableFilename, &CADET_CritDS.LastTblFileLoaded, OS_MAX_PATH_LEN );

    /* 
    ** Report the current number of CADET/FDL semaphores in use
    */
    OS_CountSemGetInfo ( CADET_FileDownlinkSemaphore, &count_prop );
    CADET_HkTelemetryPkt.FDLSempahoresInUse = count_prop.value;  

//    printf ( "CADET HK: Temp:  %d    Hi:  %d    Lo:  %d    TxPower:  %d   string = %s\n", 
//             (int)CADET_HkTelemetryPkt.StatusInfo.Temperature, (int)CADET_HkTelemetryPkt.StatusInfo.HiFifoUsage, (int)CADET_HkTelemetryPkt.StatusInfo.LoFifoUsage, 
//             (int)CADET_HkTelemetryPkt.ConfigurationInfo.TransmitPower, CADET_HkTelemetryPkt.VersionInfo.BuildIdentifier );

    CADET_UartMsg.RunningTimer = SecondsUp; 
    printf ( ",$,UART,%04u,%01X,%01X,%04d,%04d,%04d,%04d,%04d,%04d,%05d,%04d,%4.1f,%4.1f,%01u%01u%01u%01u%01u%01u%01u%01u%01u%01u,%04X,%02X,%02X,%02u,%02u,%02X,%08lX,%08lX,\n", 
             (unsigned int)CADET_UartMsg.RunningTimer, 
             (unsigned int)CADET_UartMsg.SpacecraftMode,
             (unsigned int)CADET_UartMsg.DeploymentSwitchStatus,
             (int)CADET_UartMsg.EpsBatteryVoltage,
             (int)CADET_UartMsg.EpsBatteryCurrent/1000,
             (int)CADET_UartMsg.Eps33Voltage,
             (int)CADET_UartMsg.Eps33Current/1000,
             (int)CADET_UartMsg.Eps5Voltage,
             (int)CADET_UartMsg.Eps5Current/1000,
             (int)CADET_UartMsg.Eps12Voltage,
             (int)CADET_UartMsg.Eps12Current/1000,
             CADET_UartMsg.Batt1MotherboardTemp,
             CADET_UartMsg.Batt2MotherboardTemp,
             (unsigned int)CADET_UartMsg.EpsSwitchStatus [0],
             (unsigned int)CADET_UartMsg.EpsSwitchStatus [1],
             (unsigned int)CADET_UartMsg.EpsSwitchStatus [2],
             (unsigned int)CADET_UartMsg.EpsSwitchStatus [3],
             (unsigned int)CADET_UartMsg.EpsSwitchStatus [4],
             (unsigned int)CADET_UartMsg.EpsSwitchStatus [5],
             (unsigned int)CADET_UartMsg.EpsSwitchStatus [6],
             (unsigned int)CADET_UartMsg.EpsSwitchStatus [7],
             (unsigned int)CADET_UartMsg.EpsSwitchStatus [8],
             (unsigned int)CADET_UartMsg.EpsSwitchStatus [9],
             (unsigned int)CADET_UartMsg.EpsStatus,
             (unsigned int)CADET_UartMsg.Batt1Status,
             (unsigned int)CADET_UartMsg.Batt2Status,
             (unsigned int)CADET_UartMsg.CpuAverageIdle/100, 
             (unsigned int)CADET_UartMsg.ProcessorResetCounter, 
             (unsigned int)CADET_UartMsg.LastResetCause, 
             (unsigned long int)CADET_UartMsg.HiFifoBytesUsed, 
             (unsigned long int)CADET_UartMsg.LoFifoBytesUsed );

    /*
    ** The rest of the HK data is kept up to date as we process.
    ** Time stamp the packet and send it
    */
    CFE_SB_TimeStampMsg ( (CFE_SB_Msg_t *) &CADET_HkTelemetryPkt );
    CFE_SB_SendMsg ( (CFE_SB_Msg_t *) &CADET_HkTelemetryPkt );
}



/*
** Function: CADET_HK_InitializeHousekeepingPacket
**
** Purpose: Initialize the HK processing including telemetry.
**
*/
int32 CADET_HK_InitializeHousekeepingPacket ( void )
{
    CFE_TIME_SysTime_t temp_time;

    /*
    ** Save the number of seconds when the app started (used to compute bit rates in HK data)
    */
    temp_time = CFE_TIME_GetTime();
    CADET_SecondsAtStart = temp_time.Seconds;

    /*
    ** Zero entire HK packet.
    **     Then set mode related items.
    **     then set commandable items to default values.
    */
    CFE_SB_InitMsg ( &CADET_HkTelemetryPkt, CADET_APP_HK_TLM_MID, sizeof (CADET_HkTlmPkt_t), TRUE );

    CADET_HkTelemetryPkt.Mode = MODE_OPEN;
    CADET_HkTelemetryPkt.Submode = SUBMODE_NONE;
    CADET_HkTelemetryPkt.SubmodeCommand = SUBMODE_CMD_NONE;

    CADET_HkTelemetryPkt.HighFifoMaxSizeForAutoFlush = DEFAULT_MAXIMUM_SIZE_TO_AUTO_FLUSH;
    CADET_HkTelemetryPkt.HighFifoPacketsToAutoFlush = 0; /* DEFAULT_NUMBER_PACKETS_TO_AUTO_FLUSH; */
    CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback = 0; /* DEFAULT_NUMBER_PACKETS_TO_AUTO_PLAYBACK; */

    memset ( (void*) &CADET_UartMsg, 0, sizeof(CADET_UartMsg_t) );

    return ( CFE_SUCCESS );  /* No errors */
}



/*
** Function: 
**
** Purpose: To populate the structure for the periodic UART message.
**
** Already know message ID is valid.
*/
uint32 CADET_HK_UartPopulateMsg ( CFE_SB_Msg_t* PktPtr )
{
    uint16                 MsgID;
    CFE_ES_HkPacket_t*     EsHkPtr;
    HS_HkPacket_t*         HsHkPtr;
    CADET_HkTlmPkt_t*      CadetHkPtr;
    shk_hk_tlm_t*          ShkHkPtr;
    shk_deploy_tlm_t*      ShkDeployPtr;
    uint32                 Switch;

    MsgID = CFE_SB_GetMsgId ( PktPtr );

    switch ( MsgID )
    {
        case CFE_ES_HK_TLM_MID:
             EsHkPtr = (CFE_ES_HkPacket_t *) PktPtr;
             CADET_UartMsg.ProcessorResetCounter = EsHkPtr->ProcessorResets;
             CADET_UartMsg.LastResetCause = ((EsHkPtr->ResetType & 0x0000000F) << 4) | (EsHkPtr->ResetSubtype & 0x0000000F);
             break;

        case HS_HK_TLM_MID:
             HsHkPtr = (HS_HkPacket_t *) PktPtr;
             CADET_UartMsg.CpuAverageIdle = HsHkPtr->UtilCpuAvg;
             break;

        case CADET_APP_HK_TLM_MID:
             CadetHkPtr = (CADET_HkTlmPkt_t *) PktPtr;
             CADET_UartMsg.HiFifoBytesUsed = CadetHkPtr->StatusInfo.HiFifoUsage;
             CADET_UartMsg.LoFifoBytesUsed = CadetHkPtr->StatusInfo.LoFifoUsage;
             break;

        case SHK_APP_HK_TLM_MID:
             ShkHkPtr = (shk_hk_tlm_t *) PktPtr;
             CADET_UartMsg.SpacecraftMode = ShkHkPtr->spacecraft_mode;
             /* TODO This has rounding error */
             CADET_UartMsg.EpsBatteryVoltage = (int32)((((float)(ShkHkPtr->eps_Batt_Bus_voltage) * 0.009964114) - 0.080909434) * 1000.0);
             CADET_UartMsg.EpsBatteryCurrent = (int32)((((float)(ShkHkPtr->eps_Batt_Bus_current) * 5.23863978) - 13.74982342) * 1000.0);
             CADET_UartMsg.Eps33Voltage = (int32)((((float)(ShkHkPtr->eps_3v_voltage) * 0.006929752) - 2.513363636) * 1000.0);
             CADET_UartMsg.Eps33Current = (int32)((((float)(ShkHkPtr->eps_3v_current) * 5.204367599) - 13.83722316) * 1000.0);
             CADET_UartMsg.Eps5Voltage = (int32)((((float)(ShkHkPtr->eps_5v_voltage) * 0.013111111) - 6.321444444) * 1000.0);
             CADET_UartMsg.Eps5Current = (int32)((((float)(ShkHkPtr->eps_5v_current) * 5.229378646) - 4.302292833) * 1000.0);
             CADET_UartMsg.Eps12Voltage = (int32)((((float)(ShkHkPtr->eps_12v_voltage) * 0.012597403) + 0.691168831) * 1000.0);
             CADET_UartMsg.Eps12Current = (int32)((((float)(ShkHkPtr->eps_12v_current) * 2.05889948) - 10.01446102) * 1000.0);
             for ( Switch = 0; Switch < EPS_SWITCH_COUNT; Switch++ )
             {
                 CADET_UartMsg.EpsSwitchStatus [Switch] = 0;
                 if ( ShkHkPtr->eps_switch_voltage_array [Switch] > 443 )
                 {
                     CADET_UartMsg.EpsSwitchStatus [Switch] = 1;
                 }
             }
             CADET_UartMsg.Batt1MotherboardTemp = (((float)(ShkHkPtr->bat_temp_array_p0[0]) * 0.324592) -243.024 );
             CADET_UartMsg.Batt2MotherboardTemp = (((float)(ShkHkPtr->bat_temp_array_p1[0]) * 0.376647057) -277.4637162 );
             CADET_UartMsg.EpsStatus = ShkHkPtr->eps_status;
             CADET_UartMsg.Batt1Status = ShkHkPtr->bat_status_p0;
             CADET_UartMsg.Batt2Status = ShkHkPtr->bat_status_p0;
             break;

        case SHK_APP_DEPLOY_TLM_MID:
             ShkDeployPtr= (shk_deploy_tlm_t *) PktPtr;
             CADET_UartMsg.DeploymentSwitchStatus = 0;
             if ( ShkDeployPtr->pre_deploy_state_ant  != 0 ) CADET_UartMsg.DeploymentSwitchStatus |= 0x1;
             if ( ShkDeployPtr->pre_deploy_state_mag  != 0 ) CADET_UartMsg.DeploymentSwitchStatus |= 0x2;
             if ( ShkDeployPtr->post_deploy_state_ant != 0 ) CADET_UartMsg.DeploymentSwitchStatus |= 0x4;
             if ( ShkDeployPtr->post_deploy_state_mag != 0 ) CADET_UartMsg.DeploymentSwitchStatus |= 0x8;
             break;

        default:
             break; /* Nothing to do, other values are valid, but not used here */
    }

    return (0);
}



/************************/
/*  End of File Comment */
/************************/




