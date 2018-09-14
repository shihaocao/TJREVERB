/*******************************************************************************
** File: shk_app.c
**
** Purpose:
**   Spacecraft HouseKeeping. Manages batteries, EPS, and camera.
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "shk_app.h"
#include "shk_perfids.h"
#include "shk_msgids.h"
#include "shk_msg.h"

#include "sc_msgids.h"
#include "deployable.h"
#include "dhl_lib.h"
#include <util/error.h>

#define GPIO_EXT_A  0x48     // needed for deployables.
#define GPIO_EXT_B  0x49
#define DEPLOY_COUNT_START 4 // Wait for 4 SHK loops between turning on and off deploy switches.

// ADC I2C addresses - See ssc_lib.h for actual defs.
// ADC_1 0x18
// ADC_2 0x19
// ADC_3 0x1A
// ADC_4 0x1B

/*
** Local Prototypes
*/
int     SHK_read_hk_data(void);
int     new_mode = 0;
void    SHK_CameraCaptureImageCmd(SHK_CameraCaptureCmd_t *CameraCaptureCmd);
boolean SHK_UpdateMode(void); 
void    SHK_UpdateEPSSwitches(uint8 mode);
void    read_ltc_bank(uint8 i2c_address, uint16 *data); 


/*
** global data
*/
shk_hk_tlm_t       SHK_HkTelemetryPkt;
CFE_SB_PipeId_t    SHK_CommandPipe;
CFE_SB_MsgPtr_t    SHKMsgPtr;
SHK_CameraData_t   SHK_CameraData;
boolean            groundCommandedModeUpdate = true; // Forces switches update at startup.
int                groundCommandedMode = MODE_SAFE;

char               SHK_ModeStrings[4][10] = {"CHARGING", "SAFE", "DAGR", "INMS"};
                            
/*
** Deployment related Globals
*/
int                SHK_deploy_counter = 0;
shk_deploy_tlm_t   SHK_DeployPkt_t;

// dagr_change_mode_t DAGR_ChangeModePkt; // Not sure if this will be done here or in DAGR

static CFE_EVS_BinFilter_t  SHK_EventFilters[] =
       {  /* Event ID    mask */
          {SHK_STARTUP_INF_EID,         0x0000},
          {SHK_COMMAND_ERR_EID,         0x0000},
          {SHK_COMMANDNOP_INF_EID,      0x0000},
          {SHK_INVALID_MSGID_ERR_EID,   0x0000},
          {SHK_LEN_ERR_EID,             0x0000},
          {SHK_TASK_EXIT_EID,           0x0000},
          {SHK_CAMERA_INF_EID,          0x0000},
          {SHK_DEPLOY_START_INF_EID,    0x0000},
          {SHK_DEPLOY_PRIMARY_INF_EID,  0x0000},
          {SHK_DEPLOY_SECONDARY_INF_EID,0x0000},
          {SHK_MODE_CMD_INF_EID,        0x0000},
          {SHK_MODE_CMD_ERR_EID,        0x0000}
       };


/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* SHK_AppMain() -- Application entry point and main process loop          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void SHK_AppMain( void )
{
    int32  Status       = CFE_SUCCESS;
    uint32 RunStatus    = CFE_ES_APP_RUN;
    uint32 SbDelay      = CFE_SB_PEND_FOREVER;
    int32  DeployStatus = FALSE;

    CFE_ES_PerfLogEntry(SHK_APP_PERF_ID);

    Status = SHK_AppInit();
    if (Status != CFE_SUCCESS)
    {
        RunStatus = CFE_ES_APP_ERROR;
    }

    CFE_ES_WaitForStartupSync(SHK_STARTUP_SYNC_TIMEOUT);

    /*
    ** Check for deployment
    */
    DeployStatus = SHK_CheckForDeployment();
    if ( DeployStatus == TRUE ) 
    {
       SHK_ActivateDeploymentRTS();
    } 

    /*
    ** SHK Runloop
    */
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        /* 
        ** Check and / or change current mode
        */
        boolean modeChanged = SHK_UpdateMode();

        /* 
        ** If mode has changed, update EPS switch
        */
        if (modeChanged) 
        {
          SHK_UpdateEPSSwitches(groundCommandedMode);
          MODE_SetSpacecraftMode(groundCommandedMode);
        }

        /* 
        ** If there is an image transfer in progress
        ** set a low timeout to keep the transfer going
        ** Otherwise, delay
        */
        if ( SHK_CameraData.ImageCaptureInProgress > 0 ) 
        { 
           SbDelay = SHK_IMAGE_TRANSFER_SB_DELAY;
        }
        else 
        {
           SbDelay = CFE_SB_PEND_FOREVER;
        }

        CFE_ES_PerfLogExit(SHK_APP_PERF_ID);
        Status = CFE_SB_RcvMsg(&SHKMsgPtr, SHK_CommandPipe, SbDelay);
        CFE_ES_PerfLogEntry(SHK_APP_PERF_ID);
 
        /* 
        ** On SB return, check for timeout. 
        ** The normal case is when a packet arrives
        ** The timeout case is for processing camera data
        ** Otherwise, it is an error  
        */
        if (Status == CFE_SUCCESS)
        {
            SHK_ProcessCommandPacket();
        } 
        else if (Status == CFE_SB_TIME_OUT) 
        {
            SHK_CameraTransferData();
        } 
        else 
        {
            RunStatus = CFE_ES_APP_ERROR;
        }

        /* 
        ** Deployment code.
        ** If Deployment is active, then process it 
        ** This will only if the following conditions are met:
        ** 1. Upon SHK startup the file /boot/deploy.me is present
        ** 2. A SHK deploy command is received 
        */
        if ( DeployStatus == TRUE && SHK_deploy_counter > 0 ) 
        {
            SHK_ProcessDeployment();
        }
           
    }

    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SHK_TASK_EXIT_EID, CFE_EVS_CRITICAL,
                          "Task terminating, err = 0x%08X", Status);
        CFE_ES_WriteToSysLog("SHK task terminating, err = 0x%08X\n", Status);
    }

    CFE_ES_PerfLogExit(SHK_APP_PERF_ID);
    CFE_ES_ExitApp(RunStatus);

} /* End of SHK_AppMain() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* SHK_AppInit() --  initialization                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 SHK_AppInit(void)
{
    int32 Status;

    /*
    ** Register the app with Executive services
    */
    Status = CFE_ES_RegisterApp();
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    /*
    ** Register the events
    */
    Status = CFE_EVS_Register(SHK_EventFilters,
                              sizeof(SHK_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                              CFE_EVS_BINARY_FILTER);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    /*
    ** Create the Software Bus command pipe and subscribe to housekeeping
    **  messages
    */
    Status = CFE_SB_CreatePipe(&SHK_CommandPipe, SHK_PIPE_DEPTH,"SHK_CMD_PIPE");
    if ( Status != CFE_SUCCESS)
    {
        OS_printf("SHK: CFE_SB_CreatePipe returned an error!\n");
        return Status;
    }

    Status = CFE_SB_Subscribe(SHK_APP_CMD_MID, SHK_CommandPipe);
    if (Status != CFE_SUCCESS) {
        return Status;
    }

    Status = CFE_SB_Subscribe(SHK_APP_SEND_HK_MID, SHK_CommandPipe);
    if (Status != CFE_SUCCESS) {
        return Status;
    }

    SHK_ResetCounters();

    CFE_SB_InitMsg(&SHK_HkTelemetryPkt,
                   SHK_APP_HK_TLM_MID,
                   SHK_APP_HK_TLM_LNGTH, TRUE);

    CFE_SB_InitMsg(&SHK_DeployPkt_t,
                   SHK_APP_DEPLOY_TLM_MID,
                   SHK_APP_DEPLOY_TLM_LNGTH, TRUE);

    CFE_EVS_SendEvent (SHK_STARTUP_INF_EID, CFE_EVS_INFORMATION,
               "SHK App Initialized. Version %d.%d.%d.%d",
                SHK_APP_MAJOR_VERSION,
                SHK_APP_MINOR_VERSION,
                SHK_APP_REVISION,
                SHK_APP_MISSION_REV);

    return CFE_SUCCESS;
} /* End of SHK_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SHK_ProcessCommandPacket                                           */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SHK       */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SHK_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId(SHKMsgPtr);

    switch (MsgId)
    {
        case SHK_APP_CMD_MID:
            SHK_ProcessGroundCommand();
            break;

        case SHK_APP_SEND_HK_MID:
            SHK_ReportHousekeeping();
            break;

        default:
            SHK_HkTelemetryPkt.shk_command_error_count++;
            CFE_EVS_SendEvent(SHK_COMMAND_ERR_EID,CFE_EVS_ERROR,
			"SHK: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End SHK_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SHK_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SHK_ResetCounters(void)
{
    /* Status of commands processed by the SHK App */
    SHK_HkTelemetryPkt.shk_command_count       = 0;
    SHK_HkTelemetryPkt.shk_command_error_count = 0;

    CFE_EVS_SendEvent(SHK_COMMANDRST_INF_EID, CFE_EVS_INFORMATION,
                "SHK: RESET command");
    return;

} /* End of SHK_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* SHK_VerifyCmdLength() -- Verify command packet length                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

boolean SHK_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
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

        CFE_EVS_SendEvent(SHK_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        SHK_HkTelemetryPkt.shk_command_error_count++;
    }

    return(result);

} /* End of SHK_VerifyCmdLength() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* SHK_ProcessGroundCommand() -- SHK ground commands                          */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void SHK_ProcessGroundCommand(void)
{
    uint16 CommandCode;

    CommandCode = CFE_SB_GetCmdCode(SHKMsgPtr);

    /* Process "known" SHK app ground commands */
    switch (CommandCode)
    {
        case SHK_APP_NOOP_CC:
            SHK_HkTelemetryPkt.shk_command_count++;
            CFE_EVS_SendEvent(SHK_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION,
			"SHK: NOOP command %d.%d.%d.%d",
                SHK_APP_MAJOR_VERSION,
                SHK_APP_MINOR_VERSION,
                SHK_APP_REVISION,
                SHK_APP_MISSION_REV);
            break;

        case SHK_APP_RESET_COUNTERS_CC:
            SHK_ResetCounters();
            break;

        case SHK_APP_CHANGE_SC_MODE_CC:
            SHK_HkTelemetryPkt.shk_command_count++;
            groundCommandedModeUpdate = true;
            groundCommandedMode = (((SHK_ChangeSpacecraftModeCmd_t *)SHKMsgPtr)->SpacecraftMode);
            CFE_EVS_SendEvent(SHK_MODE_CMD_INF_EID,CFE_EVS_INFORMATION,
			"SHK: CHANGE Mode to: %s", SHK_ModeStrings[groundCommandedMode]);
            break;

        case SHK_APP_BUS_RESET_CC:
            OS_printf("SHK Cycling power on 0x%X\n", ((SHK_ResetCmd_t*)SHKMsgPtr)->reset_mask);
            SHK_HkTelemetryPkt.shk_command_count++;
            EPS_reset_bus(((SHK_ResetCmd_t*)SHKMsgPtr)->reset_mask);
            // CLY_SendCommand(EPS_ADDRESS, EPS_COMMANDS[EPS_CMD_SET_PCM_RESET_I][0], EPS_COMMANDS[EPS_CMD_SET_PCM_RESET_I][1], ((SHK_ResetCmd_t *)SHKMsgPtr)->reset_mask, 0, NULL, NULL, FAST_TELEM);
            break;

        case SHK_APP_DEVICE_ON_CC:
            SHK_HkTelemetryPkt.shk_command_count++;
            OS_printf("SHK Device Turn on : %d\n", ((SHK_DeviceCmd_t *)SHKMsgPtr)->DeviceToSwitch);
            switch (((SHK_DeviceCmd_t *)SHKMsgPtr)->DeviceToSwitch)
            {
                case 1: //DAGR
                    OS_printf("DAGR switched on by ground command\n");
                    EPS_set_switch(1,  true); // DAGR (12V)
                    EPS_set_switch(5,  true);  // DAGR (5.0V)
                    break;
                case 2: // INMS
                    //OS_printf("INMS GROUND COMMAND ON DISABLED\n");
                    // Disabled for now
                    // Order from ICD
                    EPS_set_switch(8,  true);  // INMS (3.3V)
                    EPS_set_switch(2,  true);  // INMS (12V)
                    EPS_set_switch(7,  true);  // INMS (5.0V & -> -5.0V)
                    break;
                case 3: // WFSS
                    OS_printf("WFSS switched on by ground command\n");
                    EPS_set_switch(3,  true);  // WFSS (VBAT -> 3.3V)
                    break;
                case 4: // Gyro
                    OS_printf("Sensonor Gyro switched on by ground command\n");
                    EPS_set_switch(4,  true);  // Gyro (VBAT -> 5.0V)
                    break;
                case 5: // Reaction Wheels
                    OS_printf("Reaction Wheels switched on by ground command\n");
                    EPS_set_switch(6,  true);  // Reaction Wheels (5.0V)
                    break;
                case 6: // GomSpace FSSs
                    OS_printf("GFSS switched on by ground command\n");
                    EPS_set_switch(9,  true);  // Both GFSS (3.3V)
                    break;
                case 7: // GPS
                    OS_printf("GPS switched on by ground command\n");
                    EPS_set_switch(10, true);  // GPS (3.3V & -> 6.0V)
                    break;
                default:
                    OS_printf("No matching device to switch on for %d\n", ((SHK_DeviceCmd_t *)SHKMsgPtr)->DeviceToSwitch);
                    break;
            }
            break;

        case SHK_APP_DEVICE_OFF_CC:
            SHK_HkTelemetryPkt.shk_command_count++;
            OS_printf("SHK Device Turn OFF : %d\n", ((SHK_DeviceCmd_t *)SHKMsgPtr)->DeviceToSwitch);
            switch (((SHK_DeviceCmd_t *)SHKMsgPtr)->DeviceToSwitch)
            {
                case 1: //DAGR
                    OS_printf("DAGR switched OFF by ground command\n");
                    EPS_set_switch(1,  false);  // DAGR (12V)
                    EPS_set_switch(5,  false);  // DAGR (5.0V)
                    break;
                case 2: // INMS
                    OS_printf("INMS switched OFF by ground command\n");
                    // Order from ICD
                    EPS_set_switch(2,  false);  // INMS (12V)
                    EPS_set_switch(7,  false);  // INMS (5.0V & -> -5.0V)
                    EPS_set_switch(8,  false);  // INMS (3.3V)
                    break;
                case 3: // WFSS
                    OS_printf("WFSS switched OFF by ground command\n");
                    EPS_set_switch(3,  false);  // WFSS (VBAT -> 3.3V)
                    break;
                case 4: // Gyro
                    OS_printf("Sensonor Gyro switched OFF by ground command\n");
                    EPS_set_switch(4,  false);  // Gyro (VBAT -> 5.0V)
                    break;
                case 5: // Reaction Wheels
                    OS_printf("Reaction Wheels switched OFF by ground command\n");
                    EPS_set_switch(6,  false);  // Reaction Wheels (5.0V)
                    break;
                case 6: // GomSpace FSSs
                    OS_printf("GFSS switched OFF by ground command\n");
                    EPS_set_switch(9,  false);  // Both GFSS (3.3V)
                    break;
                case 7: // GPS
                    OS_printf("GPS switched OFF by ground command\n");
                    EPS_set_switch(10, false);  // GPS (3.3V & -> 6.0V)
                    break;
                default:
                    OS_printf("No matching device to switch OFF for %d\n", ((SHK_DeviceCmd_t *)SHKMsgPtr)->DeviceToSwitch);
                    break;
            }
            break;

        case SHK_APP_PRIMARY_DEPLOY_CC:
            SHK_HkTelemetryPkt.shk_command_count++;
            SHK_DeployPkt_t.gpio_ext_addr = GPIO_EXT_A;
            SHK_deploy_counter = DEPLOY_COUNT_START;
            break;

        case SHK_APP_SECONDARY_DEPLOY_CC:
            SHK_HkTelemetryPkt.shk_command_count++;
            SHK_DeployPkt_t.gpio_ext_addr = GPIO_EXT_B;
            SHK_deploy_counter = DEPLOY_COUNT_START;
            break;

        case SHK_APP_CAM_CAPTURE_CC:
            SHK_CameraCaptureImageCmd((SHK_CameraCaptureCmd_t *)SHKMsgPtr);
            break;

        default:
            SHK_HkTelemetryPkt.shk_command_error_count++;
            CFE_EVS_SendEvent(SHK_COMMAND_ERR_EID, CFE_EVS_ERROR,
                             "SHK invalid command code: %d", CommandCode);
            break;
    }
    return;
} /* End of SHK_ProcessGroundCommand() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SHK_ReportHousekeeping                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
static int eps_console_dump = 0;
void SHK_ReportHousekeeping(void)
{
    int  epsStatus;
    uint16 toss;

    /*
    ** is this needed??
    ** (AC) - does not seem to be needed, although I am running eps status on the shell first. 
    */

    SHK_HkTelemetryPkt.spacecraft_mode = MODE_GetSpacecraftMode();
    epsStatus = SHK_read_hk_data();
    
    // Read thermister data
    // Bank 1 uses all channels.
    read_ltc_bank(LTC_I2C_ADDR0, &SHK_HkTelemetryPkt.THM_BP1);

    // Bank 2 skips THM_INMS_IF
    SSC_LTC2309_Read(LTC_I2C_ADDR1, LTC_CH0 | LTC_UNIPOLAR_MODE, &toss);
    SSC_LTC2309_Read(LTC_I2C_ADDR1, LTC_CH1 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.THM_RW_Hsg_Side);
    SSC_LTC2309_Read(LTC_I2C_ADDR1, LTC_CH2 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.THM_L3);
    SSC_LTC2309_Read(LTC_I2C_ADDR1, LTC_CH3 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.THM_Bill_Board);
    SSC_LTC2309_Read(LTC_I2C_ADDR1, LTC_CH4 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.THM_Bill_Brkt);
    SSC_LTC2309_Read(LTC_I2C_ADDR1, LTC_CH5 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.THM_INMS_Back);
    SSC_LTC2309_Read(LTC_I2C_ADDR1, LTC_CH7 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.THM_INMS_Side);
    // SSC_LTC2309_Read(LTC_I2C_ADDR1, LTC_CH7 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.THM_INMS_IF); NOT USED
    SSC_LTC2309_Read(LTC_I2C_ADDR1, LTC_CH0 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.THM_GYRO); // NOT an error

    // Bank 3 uses all channels
    read_ltc_bank(LTC_I2C_ADDR2, &SHK_HkTelemetryPkt.THM_Todd_Mag);

    // Bank 4 skips an unassigned channel
    SSC_LTC2309_Read(LTC_I2C_ADDR3, LTC_CH0 | LTC_UNIPOLAR_MODE, &toss);
    SSC_LTC2309_Read(LTC_I2C_ADDR3, LTC_CH1 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.TL_VOUT);
    SSC_LTC2309_Read(LTC_I2C_ADDR3, LTC_CH2 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.TL_TEMP_1);
    SSC_LTC2309_Read(LTC_I2C_ADDR3, LTC_CH6 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.TL_TEMP_2);
    // SSC_LTC2309_Read(LTC_I2C_ADDR3, LTC_CH4 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.ADC_UNASSIGNED);
    // SSC_LTC2309_Read(LTC_I2C_ADDR3, LTC_CH5 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.INMS_P5V_I);
    // SSC_LTC2309_Read(LTC_I2C_ADDR3, LTC_CH6 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.INMS_P5V_V);
    SSC_LTC2309_Read(LTC_I2C_ADDR3, LTC_CH0 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.INMS_M5V_I);
    // SSC_LTC2309_Read(LTC_I2C_ADDR3, LTC_CH0 | LTC_UNIPOLAR_MODE, &SHK_HkTelemetryPkt.INMS_M5V_V); // NOT an error

    // Deployable data? Different packet probably.

    // NM Temperature
    SOL_GetTemps(&SHK_HkTelemetryPkt.NM_TEMP_1, &SHK_HkTelemetryPkt.NM_TEMP_2);

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SHK_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &SHK_HkTelemetryPkt);

    return;

} /* End of SHK_ReportHousekeeping() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* SHK_UpdateMode()                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

boolean SHK_UpdateMode(void) {
  // TODO: Doesn't check if the new mode is different than the current.
  // But this may be fine. It might be best to reset the switches on ground commands regardless.
  if (groundCommandedModeUpdate) {
    groundCommandedModeUpdate = false;
    return true;
  }

  return false;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SHK_CheckForDeployment                                             */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will check to see if the spacecraft deployments should    */
/*     be activated.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 SHK_CheckForDeployment(void)
{
   int32 FileHandle;
   int32 ReturnCode = FALSE;

   FileHandle = OS_open(SHK_DEPLOY_FILENAME, OS_READ_ONLY, 0);

   if ( FileHandle < 0 )
   {
      ReturnCode = FALSE;
   }
   else
   {
      printf("--------  SHK: Deployment file exists, Deploy! -------\n");
      OS_close(FileHandle);
      ReturnCode = TRUE;
   } 
   return(ReturnCode);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SHK_ActivateDeploymentRTS                                          */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will send a command to the SC App to start the deployment */
/*     Relative Time Sequence (RTS).                                          */
/*     Potential issue: Is SC up and receiving commands when this is called ? */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SHK_ActivateDeploymentRTS(void)
{
   SHK_RTSRequest_t   RTSRequest;

   /*
   ** Send deployment event
   */ 
   CFE_EVS_SendEvent (SHK_DEPLOY_START_INF_EID, CFE_EVS_INFORMATION,
                      "SHK Deployment, Start Deployment RTS");

   /*
   ** First enable the RTS
   */
   CFE_SB_InitMsg((CFE_SB_Msg_t *) ((uint32) &RTSRequest),
           SC_CMD_MID, sizeof(SHK_RTSRequest_t), TRUE);
   CFE_SB_SetCmdCode((CFE_SB_Msg_t *) ((uint32) &RTSRequest), SHK_SC_ENABLE_RTS_CC);
   RTSRequest.RTSId = SHK_DEPLOY_RTS;
   CFE_SB_SendMsg((CFE_SB_Msg_t *) ((uint32) &RTSRequest));

   /*
   ** Next, start it
   */
   CFE_SB_InitMsg((CFE_SB_Msg_t *) ((uint32) &RTSRequest),
           SC_CMD_MID, sizeof(SHK_RTSRequest_t), TRUE);
   CFE_SB_SetCmdCode((CFE_SB_Msg_t *) ((uint32) &RTSRequest), SHK_SC_START_RTS_CC);
   RTSRequest.RTSId = SHK_DEPLOY_RTS;
   CFE_SB_SendMsg((CFE_SB_Msg_t *) ((uint32) &RTSRequest));

   /*
   ** Error checks?
   */
 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SHK_ProcessDeployment                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process the deployments                              */
/*                                                                            */
/*  Globals used:                                                             */
/*     SHK_deploy_counter -- used to count the number of Housekeeping cycles      */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SHK_ProcessDeployment(void)
{
    /*
    ** Start the deployment
    ** The command sets the GPIO address ( primary/secondary ) 
    */
    if (DEPLOY_COUNT_START == SHK_deploy_counter) 
    {
        if (SHK_DeployPkt_t.gpio_ext_addr == GPIO_EXT_A)
        {
           CFE_EVS_SendEvent (SHK_DEPLOY_PRIMARY_INF_EID, CFE_EVS_INFORMATION,
                              "SHK Deployment: Activate Primary Deployments");
        }
        else
        {
           CFE_EVS_SendEvent (SHK_DEPLOY_SECONDARY_INF_EID, CFE_EVS_INFORMATION,
                              "SHK Deployment: Activate Secondary Deployments");
        }
           
        DeployOn(SHK_DeployPkt_t.gpio_ext_addr, 
                 &SHK_DeployPkt_t.pre_deploy_state_ant, 
                 &SHK_DeployPkt_t.pre_deploy_state_mag);
    }

    SHK_deploy_counter--;

    /*
    ** Turn the deployment back off, and send telemetry
    */
    if (0 == SHK_deploy_counter) 
    {

        DeployOff(SHK_DeployPkt_t.gpio_ext_addr, 
                  &SHK_DeployPkt_t.post_deploy_state_ant, 
                  &SHK_DeployPkt_t.post_deploy_state_mag);
        /* 
        ** Send deploy tlm
        */
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SHK_DeployPkt_t);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &SHK_DeployPkt_t);
    }
}

/*
** SHK_CameraCaptureImageCmd
** This function will capture an image from the camera and
** prepare to save it to a file
**
*/ 
void SHK_CameraCaptureImageCmd(SHK_CameraCaptureCmd_t *CameraCaptureCmd)
{
   int32 ImageFileSize = 0;

   if ( SHK_CameraData.ImageCaptureInProgress > 0 ) 
   {
      CFE_EVS_SendEvent(SHK_COMMAND_ERR_EID, CFE_EVS_ERROR,
                        "SHK Image Capture is Currently in Progress");
      SHK_HkTelemetryPkt.shk_command_error_count++;

   }
   else
   {
      SHK_CameraData.ImageCaptureInProgress = 1;
      SHK_CameraData.ImageSize = CameraCaptureCmd->ImageSize;
      strncpy((char *)SHK_CameraData.ImagePath, (char *)(CameraCaptureCmd->ImagePath), OS_MAX_PATH_LEN);

      if ( SHK_CameraData.ImageSize < 1 || SHK_CameraData.ImageSize > 3 )
      {
         CFE_EVS_SendEvent(SHK_COMMAND_ERR_EID, CFE_EVS_ERROR,
                        "SHK Invalid Camera Image Size Specified: %d", SHK_CameraData.ImageSize);
         SHK_HkTelemetryPkt.shk_command_error_count++;
         SHK_CameraData.ImageCaptureInProgress = 0;
      }
      else
      {
         SHK_CameraData.ImageFileFd = OS_open((char *)SHK_CameraData.ImagePath, OS_READ_WRITE, 0); 

         if ( SHK_CameraData.ImageFileFd < 0 ) 
         {
            CFE_EVS_SendEvent(SHK_COMMAND_ERR_EID, CFE_EVS_ERROR,
                        "SHK Camera Image File Open Failed %s", SHK_CameraData.ImagePath);
            SHK_HkTelemetryPkt.shk_command_error_count++;
            SHK_CameraData.ImageCaptureInProgress = 0;

         }
         else
         {
             /*
             ** All in one image capture command
             ** 
             ** This will turn on camera, init UART, reset camera, set image size, and capture image
             **
             */
             ImageFileSize = SHK_CameraCaptureImage(SHK_CameraData.ImageSize);

             if ( ImageFileSize > 0 ) 
             {
                 CFE_EVS_SendEvent (SHK_CAMERA_INF_EID, CFE_EVS_INFORMATION,
                                    "SHK Camera Image Capture Started: %s",
                                    SHK_CameraData.ImagePath);
                 SHK_HkTelemetryPkt.shk_command_count++;
                 SHK_CameraData.ImageFileSize = ImageFileSize;            
                   
             }
             else
             {
                CFE_EVS_SendEvent(SHK_COMMAND_ERR_EID, CFE_EVS_ERROR,
                        "SHK Camera Image Capture Failed %s", SHK_CameraData.ImagePath);
                SHK_HkTelemetryPkt.shk_command_error_count++;
                SHK_CameraData.ImageCaptureInProgress = 0;
             
             } 

         }
  
      } 

   }

}


/*
** SHK_CameraCaptureImage
**  This function will do the following:
**  Turn on deployment camera
**  Init UART
**  Reset camera
**  Set desired image size
**  capture image
**  return captured image size
*/
int32 SHK_CameraCaptureImage(uint32 ImageSize)
{
   int32  ReturnCode;
   uint16 ImageFileSize;

   /*
   ** Power on the Camera
   */
   CAM_PowerOnCamera(); 

   /* 
   ** Init the camera
   */
   CAM_InitCamera();

   /*
   ** Reset Camera
   */
   ReturnCode = CAM_ResetCamera();

   /*
   ** Set the desired image size
   */
   if ( ImageSize == 2 ) 
   {
      CAM_SetPictureSize(0x11); /* 320x240 */
   }
   else if ( ImageSize == 3 )
   {
      CAM_SetPictureSize(0x00); /* 640x480 */
   }
   else
   {
      CAM_SetPictureSize(0x22); /* 160x120 -- default */
   }

   /*
   ** Take the Picture
   */
   ReturnCode = CAM_SnapImage();
   if ( ReturnCode == 0 )
   {
      /*
      ** Get the size from the camera
      */
      ImageFileSize = CAM_GetPictureSize();
      SHK_CameraData.ImageOffset = 0;
      ReturnCode = ImageFileSize;
   }
   else
   {
      /*
      ** Power the camera off
      */
      CAM_PowerOffCamera(); 
      
      ReturnCode = -1;

   }
   return(ReturnCode);
}

/*
** SHK_CameraTransferData
**  This function will transfer a chunk of data from the camera to an image file. 
*/
void  SHK_CameraTransferData(void)
{
   int32 ReturnCode;
   
   if ( SHK_CameraData.ImageCaptureInProgress > 0 )
   {
      /*
      ** At this point, the file should be open and the image should have
      ** been captured to the camera.
      ** Transfer a chunk of data at a time and save it to the file
      */
      ReturnCode = CAM_GetNextImageChunk(SHK_CameraData.ImageFileFd,
                                        SHK_CameraData.ImageOffset);

      if ( ReturnCode == CAM_END_OF_FILE ) 
      {
          OS_close(SHK_CameraData.ImageFileFd);
          SHK_CameraData.ImageCaptureInProgress = 0;
          CFE_EVS_SendEvent (SHK_CAMERA_INF_EID, CFE_EVS_INFORMATION,
               "SHK Camera Image Capture Complete: %s",
                SHK_CameraData.ImagePath);

          /*
          ** Turn the camera off 
          */
          CAM_PowerOffCamera(); 

      }
      else
      {
          SHK_CameraData.ImageOffset = ReturnCode;
          /* printf("SHK--> Next Camera Image Chunk Read: %d\n", (int)ReturnCode); */
      }
   }

}
 
void SHK_UpdateEPSSwitches(uint8 mode) {
  switch (mode)
  {
    case MODE_CHARGING:
      // Full ACS suite “on”
      // Internal Mags and INMS “off” (set by DAGR app power mode 3)
      EPS_set_switch(1,  true);  // DAGR (12V)
      EPS_set_switch(5,  true);  // DAGR (5.0V)

      EPS_set_switch(2,  false); // INMS (12V)
      EPS_set_switch(7,  false); // INMS (5.0V & -> -5.0V)
      EPS_set_switch(8,  false); // INMS (3.3V)

      EPS_set_switch(3,  true);  // WFSS (VBAT -> 3.3V)
      EPS_set_switch(4,  true);  // Gyro (VBAT -> 5.0V)
      EPS_set_switch(6,  true);  // Reaction Wheels (5.0V)
      EPS_set_switch(9,  true);  // Both GFSS (3.3V)
      EPS_set_switch(10, true);  // GPS (3.3V & -> 6.0V)
      break;
    case MODE_SAFE:
      // Torquers, RW, Sun Sensors “on”
      // GPS, Dagr (internal and boom), INMS and Gyro “off”
      EPS_set_switch(1,  false); // DAGR (12V)
      EPS_set_switch(5,  false); // DAGR (5.0V)

      EPS_set_switch(2,  false); // INMS (12V)
      EPS_set_switch(7,  false); // INMS (5.0V & -> -5.0V)
      EPS_set_switch(8,  false); // INMS (3.3V)

      EPS_set_switch(3,  true);  // WFSS (VBAT -> 3.3V)
      EPS_set_switch(4,  false); // Gyro (VBAT -> 5.0V)
      EPS_set_switch(6,  true);  // Reaction Wheels (5.0V) // TODO: Should be on?
      EPS_set_switch(9,  true);  // Both GFSS (3.3V)
      EPS_set_switch(10, false); // GPS (3.3V & -> 6.0V)
      break;
    case MODE_DAGR:
      // Full ACS suite and Dagr “on”
      // INMS "off"
      EPS_set_switch(1,  true);  // DAGR (12V)
      EPS_set_switch(5,  true);  // DAGR (5.0V)

      EPS_set_switch(2,  false); // INMS (12V)
      EPS_set_switch(7,  false); // INMS (5.0V & -> -5.0V)
      EPS_set_switch(8,  false); // INMS (3.3V)

      EPS_set_switch(3,  true);  // WFSS (VBAT -> 3.3V)
      EPS_set_switch(4,  true);  // Gyro (VBAT -> 5.0V)
      EPS_set_switch(6,  true);  // Reaction Wheels (5.0V)
      EPS_set_switch(9,  true);  // Both GFSS (3.3V)
      EPS_set_switch(10, true);  // GPS (3.3V & -> 6.0V)
      break;
    case MODE_INMS:
      // Full ACS suite and INMS “on”
      // Internal magnetometers “off” (set by DAGR app power mode 3)
      EPS_set_switch(1,  true);  // DAGR (12V)
      EPS_set_switch(5,  true);  // DAGR (5.0V)

      EPS_set_switch(8,  true);  // INMS (3.3V)
      EPS_set_switch(2,  true);  // INMS (12V)
      EPS_set_switch(7,  true);  // INMS (5.0V & -> -5.0V)

      EPS_set_switch(3,  true);  // WFSS (VBAT -> 3.3V)
      EPS_set_switch(4,  true);  // Gyro (VBAT -> 5.0V)
      EPS_set_switch(6,  true);  // Reaction Wheels (5.0V)
      EPS_set_switch(9,  true);  // Both GFSS (3.3V)
      EPS_set_switch(10, true);  // GPS (3.3V & -> 6.0V)
      break;
    default:
      break;
  }

}

void read_ltc_bank(uint8 i2c_address, uint16 *data) {
  uint16 toss;
  SSC_LTC2309_Read(i2c_address, LTC_CH0 | LTC_UNIPOLAR_MODE, &toss);
  // data index will be one behind LTC_CHX because we are reading the last request
  SSC_LTC2309_Read(i2c_address, LTC_CH1 | LTC_UNIPOLAR_MODE, &data[0]);
  SSC_LTC2309_Read(i2c_address, LTC_CH2 | LTC_UNIPOLAR_MODE, &data[1]);
  SSC_LTC2309_Read(i2c_address, LTC_CH3 | LTC_UNIPOLAR_MODE, &data[2]);
  SSC_LTC2309_Read(i2c_address, LTC_CH4 | LTC_UNIPOLAR_MODE, &data[3]);
  SSC_LTC2309_Read(i2c_address, LTC_CH5 | LTC_UNIPOLAR_MODE, &data[4]);
  SSC_LTC2309_Read(i2c_address, LTC_CH6 | LTC_UNIPOLAR_MODE, &data[5]);
  SSC_LTC2309_Read(i2c_address, LTC_CH7 | LTC_UNIPOLAR_MODE, &data[6]);
  // NOT an error: data[7] is recorded in the next line, but was requested previous line
  // Request for LTC_CH0 will be tossed by the next call.
  SSC_LTC2309_Read(i2c_address, LTC_CH0 | LTC_UNIPOLAR_MODE, &data[7]);
}

void read_bcrs(void) {
    uint16 sun_value = 0;

    // BCR 1
    EPS_read_raw_eps(0x00, &SHK_HkTelemetryPkt.eps_sa1_v_y, FAST_TELEM);
    EPS_read_raw_eps(0x01, &SHK_HkTelemetryPkt.eps_sa1_c_py, FAST_TELEM);
    EPS_read_raw_eps(0x02, &SHK_HkTelemetryPkt.eps_sa1_c_ny, FAST_TELEM);

    // BCR 2
    EPS_read_raw_eps(0x07, &SHK_HkTelemetryPkt.eps_sa2_v_y, FAST_TELEM);
    EPS_read_raw_eps(0x08, &SHK_HkTelemetryPkt.eps_sa2_c_py, FAST_TELEM);

    // BCR 3
    EPS_read_raw_eps(0x0E, &SHK_HkTelemetryPkt.eps_sa3_v_z, FAST_TELEM);
    EPS_read_raw_eps(0x0F, &SHK_HkTelemetryPkt.eps_sa3_c_pz, FAST_TELEM);
    EPS_read_raw_eps(0x10, &SHK_HkTelemetryPkt.eps_sa3_c_nz, FAST_TELEM);

    // BCR 4
    EPS_read_raw_eps(0x40, &SHK_HkTelemetryPkt.eps_sa4_v_y, SLOW_TELEM);
    EPS_read_raw_eps(0x41, &SHK_HkTelemetryPkt.eps_sa4_c_py, SLOW_TELEM);

    // BCR 5
    EPS_read_raw_eps(0x47, &SHK_HkTelemetryPkt.eps_sa5_v_y, SLOW_TELEM);
    EPS_read_raw_eps(0x48, &SHK_HkTelemetryPkt.eps_sa5_c_ny, SLOW_TELEM);

    // BCR 6
    EPS_read_raw_eps(0x4E, &SHK_HkTelemetryPkt.eps_sa6_v_x, SLOW_TELEM);
    EPS_read_raw_eps(0x4F, &SHK_HkTelemetryPkt.eps_sa6_c_px, SLOW_TELEM);

    // BCR 7
    EPS_read_raw_eps(0x55, &SHK_HkTelemetryPkt.eps_sa7_v_x, SLOW_TELEM);
    EPS_read_raw_eps(0x56, &SHK_HkTelemetryPkt.eps_sa7_c_nx, SLOW_TELEM);
}

/*
** Collect the data from the Battery/EPS -- This function can be in the app completely
*/
int SHK_read_hk_data(void)
{
    int i, retval;

    retval = EPS_read_eps_status(&(SHK_HkTelemetryPkt.eps_status));
    if (retval != E_NO_ERR) {
        return retval;
    } else {
        // Only read eps data if status call did not fail

        read_bcrs();

        /*
        ** Read EPS power bus for 12V, batt, 5V and 3.3V
        */
        EPS_read_raw_eps(32, &(SHK_HkTelemetryPkt.eps_12v_current), FAST_TELEM);
        EPS_read_raw_eps(33, &(SHK_HkTelemetryPkt.eps_12v_voltage), FAST_TELEM);
        EPS_read_raw_eps(34, &(SHK_HkTelemetryPkt.eps_Batt_Bus_current), FAST_TELEM);
        EPS_read_raw_eps(35, &(SHK_HkTelemetryPkt.eps_Batt_Bus_voltage), FAST_TELEM);
        EPS_read_raw_eps(36, &(SHK_HkTelemetryPkt.eps_5v_current), FAST_TELEM);
        EPS_read_raw_eps(37, &(SHK_HkTelemetryPkt.eps_5v_voltage), FAST_TELEM);
        EPS_read_raw_eps(38, &(SHK_HkTelemetryPkt.eps_3v_current), FAST_TELEM);
        EPS_read_raw_eps(39, &(SHK_HkTelemetryPkt.eps_3v_voltage), FAST_TELEM);

        /*
        ** Read EPS switch voltage and currents
        */
        for (i=0; i < EPS_SWITCH_COUNT; i++)
        {
            EPS_read_raw_eps(40 + (2 * i), &(SHK_HkTelemetryPkt.eps_switch_voltage_array[i]), FAST_TELEM);
            EPS_read_raw_eps(41 + (2 * i), &(SHK_HkTelemetryPkt.eps_switch_current_array[i]), FAST_TELEM);
        }
    }

    // Should send as struct, but don't want shk_msg.h included in dhl.
    retval = EPS_read_battery_pack_status(bat_address[0], &(SHK_HkTelemetryPkt.bat_status_p0));
    if (retval != E_NO_ERR) {
        // TODO: Can't tell if the EPS or one of the batteries failed.
        return retval;
    } else {
        EPS_read_battery_pack_hk(bat_address[0], &SHK_HkTelemetryPkt.bat_voltage_p0,
            &SHK_HkTelemetryPkt.bat_current_p0, &SHK_HkTelemetryPkt.bat_c_direc_p0,
            &SHK_HkTelemetryPkt.bat_5V_volt_p0, &SHK_HkTelemetryPkt.bat_5V_curr_p0,
            &SHK_HkTelemetryPkt.bat_3V_volt_p0, &SHK_HkTelemetryPkt.bat_3V_curr_p0,
            SHK_HkTelemetryPkt.bat_temp_array_p0, &SHK_HkTelemetryPkt.bat_all_heaters_p0);
    }
    retval = EPS_read_battery_pack_status(bat_address[1], &(SHK_HkTelemetryPkt.bat_status_p1));
    if (retval != E_NO_ERR) {
        return retval;
    } else {
        EPS_read_battery_pack_hk(bat_address[1], &SHK_HkTelemetryPkt.bat_voltage_p1,
            &SHK_HkTelemetryPkt.bat_current_p1, &SHK_HkTelemetryPkt.bat_c_direc_p1,
            &SHK_HkTelemetryPkt.bat_5V_volt_p1, &SHK_HkTelemetryPkt.bat_5V_curr_p1,
            &SHK_HkTelemetryPkt.bat_3V_volt_p1, &SHK_HkTelemetryPkt.bat_3V_curr_p1,
            SHK_HkTelemetryPkt.bat_temp_array_p1, &SHK_HkTelemetryPkt.bat_all_heaters_p1);
    }

    return(0);
}

