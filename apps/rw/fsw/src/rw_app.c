/*******************************************************************************
** File: rw_app.c
**
** Purpose:
**   Controls the reaction wheels.
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "rw_app.h"
#include "rw_lib.h"
#include "rw_perfids.h"
#include "rw_msgids.h"
#include "rw_msg.h"

#include "dhl_lib.h"

/*
** Device specific includes
*/
#include <math.h>

#define WINDOW_LIMIT 10
#define WARM_TIME    20
// These limits are just to detect super wrong responses (the occasional speed reading of 1.0e32)
#define MAX_RW_SPEED_READING  800
#define MIN_RW_SPEED_READING -800

/*
** global data
*/
rw_hk_tlm_t        RW_HkTelemetryPkt;
gy_hk_tlm_t        GY_HkTelemetryPkt;
CFE_SB_PipeId_t    RW_CommandPipe;
CFE_SB_MsgPtr_t    RWMsgPtr;
float              speed_values[WINDOW_LIMIT][3];
float              gyro_data[WINDOW_LIMIT][3];
int                warm_countdown = WARM_TIME;

static CFE_EVS_BinFilter_t  RW_EventFilters[] =
       {  /* Event ID    mask */
          {RW_STARTUP_INF_EID,       0x0000},
          {RW_COMMAND_ERR_EID,       0x0000},
          {RW_COMMANDNOP_INF_EID,    0x0000},
          {RW_COMMANDRST_INF_EID,    0x0000},
       };

void RW_UpdateMode(void) {
    static int current_mode = -1;
    int new_mode;

    new_mode = MODE_GetSpacecraftMode();
    if (current_mode != new_mode) {
	warm_countdown = WARM_TIME;

        if (MODE_SAFE != new_mode) {
            // re-init gyro if needed
        }

        current_mode = new_mode;
    }
}

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* RW_AppMain() -- Application entry point and main process loop          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void RW_AppMain( void )
{
    int32  Status    = CFE_SUCCESS;
    uint32 RunStatus = CFE_ES_APP_RUN;

    CFE_ES_PerfLogEntry(RW_PERF_ID);

    Status = RW_AppInit();
    if (Status != CFE_SUCCESS)
    {
        RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** RW Runloop
    */
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        CFE_ES_PerfLogExit(RW_PERF_ID);

        /* Pend on receipt of command packet -- timeout set to 3 second */
        Status = CFE_SB_RcvMsg(&RWMsgPtr, RW_CommandPipe, 3000);

        CFE_ES_PerfLogEntry(RW_PERF_ID);

        RW_UpdateMode();

        if (Status == CFE_SUCCESS)
        {
            RW_ProcessCommandPacket();
        } else if (Status == CFE_SB_TIME_OUT) {
            // Timeouts are okay.
        } else {
            RunStatus = CFE_ES_APP_ERROR;
        }
    }

    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(RW_TASK_EXIT_EID, CFE_EVS_CRITICAL,
                          "Task terminating, err = 0x%08X", Status);
        CFE_ES_WriteToSysLog("RW task terminating, err = 0x%08X\n", Status);
    }

    CFE_ES_PerfLogExit(RW_PERF_ID);
    CFE_ES_ExitApp(RunStatus);
} /* End of RW_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* RW_AppInit() --  initialization                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 RW_AppInit(void)
{
    int32 Status;
    int index, jndex;

    for (index = 0; index < WINDOW_LIMIT; index++) {
        for (jndex = 0; jndex < 3; jndex++) {
            speed_values[index][jndex] = 0.0;
            gyro_data[index][jndex] = 0.0;
        }
    }

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
    Status = CFE_EVS_Register(RW_EventFilters,
                     sizeof(RW_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                     CFE_EVS_BINARY_FILTER);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    /*
    ** Create the Software Bus command pipe and subscribe to housekeeping
    **  messages
    */
    Status = CFE_SB_CreatePipe(&RW_CommandPipe, RW_PIPE_DEPTH, "RW_CMD_PIPE");
    if (Status != CFE_SUCCESS)
    {
        OS_printf("RW: CFE_SB_CreatePipe returned an error!\n");
        return Status;
    }

    Status = CFE_SB_Subscribe(RW_CMD_MID, RW_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(RW_SEND_HK_MID, RW_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(RW_TORQUE_CMD_MID, RW_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(RW_CURRENT_CMD_MID, RW_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(RW_10HZ_WAKEUP_MID, RW_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    RW_ResetCounters();

    CFE_SB_InitMsg(&RW_HkTelemetryPkt, RW_HK_TLM_MID,
                   RW_HK_TLM_LNGTH, TRUE);

    CFE_SB_InitMsg(&GY_HkTelemetryPkt, GY_HK_TLM_MID,
                   GY_HK_TLM_LNGTH, TRUE);

    // TODO: Init the torque command packet also? recieves torque cmd.

    CFE_EVS_SendEvent (RW_STARTUP_INF_EID, CFE_EVS_INFORMATION,
               "RW Initialized. Version %d.%d.%d.%d",
                RW_MAJOR_VERSION,
                RW_MINOR_VERSION,
                RW_REVISION,
                RW_MISSION_REV);

    return CFE_SUCCESS;
} /* End of RW_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  RW_ProcessCommandPacket                                            */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the RW        */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void RW_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId(RWMsgPtr);

    switch (MsgId)
    {
        case RW_CMD_MID:
            RW_ProcessGroundCommand();
            break;

        case RW_SEND_HK_MID:
            RW_ReportHousekeeping();
            break;

        case RW_TORQUE_CMD_MID:
            RW_TorqueCommand();
        break;

        //case RW_CURRENT_CMD_MID:
        //    RW_CurrentCommand();
        //break;

        case RW_10HZ_WAKEUP_MID:
            RW_UpdateTach();
        break;

        default:
            RW_HkTelemetryPkt.rw_command_error_count++;
            CFE_EVS_SendEvent(RW_COMMAND_ERR_EID,CFE_EVS_ERROR,
            "RW: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End RW_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* RW_ProcessGroundCommand() -- RW ground commands                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void RW_ProcessGroundCommand(void)
{
    uint16 CommandCode;

    CommandCode = CFE_SB_GetCmdCode(RWMsgPtr);

    /* Process "known" RW app ground commands */
    switch (CommandCode)
    {
        case RW_NOOP_CC:
            RW_HkTelemetryPkt.rw_command_count++;
            CFE_EVS_SendEvent(RW_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION,
            "RW: NOOP command");
            break;

        case RW_RESET_COUNTERS_CC:
            RW_ResetCounters();
            break;

        case RW_MODE_CC: ;
            RW_ModeCmd_t * m_cmd = (RW_ModeCmd_t *)RWMsgPtr;
            rwlib_sendModeCommand(m_cmd->wheelAddress, m_cmd->type, m_cmd->value);
            break;

        case RW_PARAM_CC: ;
            RW_ParamCmd_t * p_cmd = (RW_ParamCmd_t *)RWMsgPtr;
            rwlib_sendParamCommand(p_cmd->wheelAddress, p_cmd->number, p_cmd->value);
            break;

        case RW_UPDATE_TACH_CC:
            RW_UpdateTach();
            break;

        /* default case already found during FC vs length test */
        default:
            break;
    }
    return;

} /* End of RW_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  RW_ReportHousekeeping                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
uint8 rw_window_index = 0;
uint8 gy_window_index = 0;
void RW_ReportHousekeeping(void)
{
    int retval;
    int mode;
    GYRO_data_t gyrodata;

    memset(&gyrodata, 0, sizeof(GYRO_data_t));

    // No reads unless warmed up
    if (warm_countdown > 0) {
        warm_countdown--;
        if (warm_countdown == 0) {
            // Turn on the wheels on mode transition // TODO: neccessary? Hold over from wheels being off in survival
            rwlib_turnon(0);
            rwlib_turnon(1);
            rwlib_turnon(2);
        }
        return;
    }

    mode = MODE_GetSpacecraftMode();

    // RW Telem Packet
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &RW_HkTelemetryPkt);

    RW_HkTelemetryPkt.rw_wheel_on_status = rwlib_get_onoff_status();

    // RW 0
    retval = rwlib_receiveParamTelem(0, RW_PARAM_CURRENT, &(RW_HkTelemetryPkt.rw0_current));
    if (retval != CFE_SUCCESS) {
        OS_printf("RW ERROR: failed rwlib_receiveParamTelem rw 0 current: %d\n", retval);
    }
    // rwlib_receiveParamTelem(0, RW_PARAM_SPEED, &(RW_HkTelemetryPkt.rw0_speed));
    RW_HkTelemetryPkt.rw0_speed = speed_values[rw_window_index][0]; // Copy from tach data
    retval = rwlib_receiveParamTelem(0, RW_PARAM_TEMP, &(RW_HkTelemetryPkt.rw0_temp));
    if (retval != CFE_SUCCESS) {
        OS_printf("RW ERROR: failed rwlib_receiveParamTelem rw 0 temp: %d\n", retval);
    }
    retval = rwlib_receiveParamTelem(0, RW_PARAM_SEU_COUNT, &(RW_HkTelemetryPkt.rw0_seu_count));
    if (retval != CFE_SUCCESS) {
        OS_printf("RW ERROR: failed rwlib_receiveParamTelem rw 0 seu: %d\n", retval);
    }

    // RW 1
    retval = rwlib_receiveParamTelem(1, RW_PARAM_CURRENT, &(RW_HkTelemetryPkt.rw1_current));
    if (retval != CFE_SUCCESS) {
        OS_printf("RW ERROR: failed rwlib_receiveParamTelem rw 1 current: %d\n", retval);
    }
    // rwlib_receiveParamTelem(1, RW_PARAM_SPEED, &(RW_HkTelemetryPkt.rw1_speed));
    RW_HkTelemetryPkt.rw1_speed = speed_values[rw_window_index][1]; // Copy from tach data
    retval = rwlib_receiveParamTelem(1, RW_PARAM_TEMP, &(RW_HkTelemetryPkt.rw1_temp));
    if (retval != CFE_SUCCESS) {
        OS_printf("RW ERROR: failed rwlib_receiveParamTelem rw 1 temp: %d\n", retval);
    }
    retval = rwlib_receiveParamTelem(1, RW_PARAM_SEU_COUNT, &(RW_HkTelemetryPkt.rw1_seu_count));
    if (retval != CFE_SUCCESS) {
        OS_printf("RW ERROR: failed rwlib_receiveParamTelem rw 1 seu: %d\n", retval);
    }

    // RW 2
    retval = rwlib_receiveParamTelem(2, RW_PARAM_CURRENT, &(RW_HkTelemetryPkt.rw2_current));
    if (retval != CFE_SUCCESS) {
        OS_printf("RW ERROR: failed rwlib_receiveParamTelem rw 2 current: %d\n", retval);
    }
    // rwlib_receiveParamTelem(2, RW_PARAM_SPEED, &(RW_HkTelemetryPkt.rw2_speed));
    RW_HkTelemetryPkt.rw2_speed = speed_values[rw_window_index][2]; // Copy from tach data
    retval = rwlib_receiveParamTelem(2, RW_PARAM_TEMP, &(RW_HkTelemetryPkt.rw2_temp));
    if (retval != CFE_SUCCESS) {
        OS_printf("RW ERROR: failed rwlib_receiveParamTelem rw 2 temp: %d\n", retval);
    }
    retval = rwlib_receiveParamTelem(2, RW_PARAM_SEU_COUNT, &(RW_HkTelemetryPkt.rw2_seu_count));
    if (retval != CFE_SUCCESS) {
        OS_printf("RW ERROR: failed rwlib_receiveParamTelem rw 2 seu: %d\n", retval);
    }

    CFE_SB_SendMsg((CFE_SB_Msg_t *) &RW_HkTelemetryPkt);

    // Gyro Telem
    if (!MODE_SAFE == mode) {
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &GY_HkTelemetryPkt);

        // Get data
        // TODO: Could save one transaction by storing the temp from the last GYRO_LibGetXXX
        if (GYRO_LibGetData(&gyrodata) == 0) {
            GY_HkTelemetryPkt.valid = 1;
        } else {
            GY_HkTelemetryPkt.valid = 0;
        }
        GY_HkTelemetryPkt.temp[0] = gyrodata.x_axis_temperature;
        GY_HkTelemetryPkt.temp[1] = gyrodata.y_axis_temperature;
        GY_HkTelemetryPkt.temp[2] = gyrodata.z_axis_temperature;
        GY_HkTelemetryPkt.rate[0] = gyrodata.x_axis_gyro;
        GY_HkTelemetryPkt.rate[1] = gyrodata.y_axis_gyro;
        GY_HkTelemetryPkt.rate[2] = gyrodata.z_axis_gyro;

        CFE_SB_SendMsg((CFE_SB_Msg_t *) &GY_HkTelemetryPkt);
    }

    return;
} /* End of RW_ReportHousekeeping() */

void RW_UpdateTach(void) {
    int index, retval, mode;
    float val_1 = 0.0, val_2 = 0.0, val_3 = 0.0; // used for rw, then gyro

    RW_UpdateMode(); // Will cause a re-init for the gyro

    // No reads unless warmed up
    if (warm_countdown > 0) {
        return;
    }

    mode = MODE_GetSpacecraftMode();

    rw_window_index = (rw_window_index + 1) % WINDOW_LIMIT;
    // RWs should be powered on in all modes
    for (index = 0; index < 3; index++) {
        retval = rwlib_receiveParamTelem(index, RW_PARAM_SPEED, &(speed_values[rw_window_index][index]));
        if (retval != CFE_SUCCESS) {
            OS_printf("RW ERROR: failed rwlib_receiveParamTelem rw %d speed: %d\n", index, retval);
        }

        // Check for obviously bad data
        if (speed_values[rw_window_index][index] < MIN_RW_SPEED_READING ||
            speed_values[rw_window_index][index] > MAX_RW_SPEED_READING) {
            speed_values[rw_window_index][index] = 0.0;
        }
    }

    for (index = 0; index < WINDOW_LIMIT; index++) {
        val_1 += speed_values[index][0];
        val_2 += speed_values[index][1];
        val_3 += speed_values[index][2];
    }
    val_1 /= WINDOW_LIMIT;
    val_2 /= WINDOW_LIMIT;
    val_3 /= WINDOW_LIMIT;

    RWS_SetTach(val_1, val_2, val_3);

    // Now gather tach data for the gyro, only when not in safe mode
    if (MODE_SAFE != mode) {
        gy_window_index = (gy_window_index + 1) % WINDOW_LIMIT;
        val_1 = 0.0; val_2 = 0.0; val_3 = 0.0;

        // Read gyro
        // GetData and GetXyz both call the same base function, and get temperature data.
        if (GYRO_LibGetXyz(&gyro_data[gy_window_index][0],
                           &gyro_data[gy_window_index][1],
                           &gyro_data[gy_window_index][2]) == 0) {
            // Valid data written
        } else {
            // Invalid data, set 0 (LibGetXyz should change it, but just to be sure.)
            gyro_data[gy_window_index][0] = 0.0;
            gyro_data[gy_window_index][1] = 0.0;
            gyro_data[gy_window_index][2] = 0.0;
        }

        // update readings
        for (index = 0; index < WINDOW_LIMIT; index++) {
          val_1 += gyro_data[index][0];
          val_2 += gyro_data[index][1];
          val_3 += gyro_data[index][2];
        }
        val_1 /= WINDOW_LIMIT;
        val_2 /= WINDOW_LIMIT;
        val_3 /= WINDOW_LIMIT;

        // set tach
        GYS_SetTach(val_1, val_2, val_3);
    }

    return;
}

void RW_TorqueCommand(void) {
    int mode = MODE_GetSpacecraftMode();
    if (warm_countdown > 0) {
        return;
    }

    // TODO: Error checking
    rw_torque_cmd_t * temp_torque = (rw_torque_cmd_t *)RWMsgPtr;
    
    RW_LibCommandRW(temp_torque->torque_cmd[0], temp_torque->torque_cmd[1], temp_torque->torque_cmd[2]);

    // printf("RW Torque Command: %f\t %f\t %f\n", temp_torque->torque_cmd[0], temp_torque->torque_cmd[1], temp_torque->torque_cmd[2]);
}

//void RW_CurrentCommand(void) {
//    rw_current_cmd_t * temp_current = (rw_current_cmd_t *)RWMsgPtr;
//    rwlib_sendModeCommand(0, RW_MODE_CURRENT, temp_current->current_cmd[0]);
//    rwlib_sendModeCommand(1, RW_MODE_CURRENT, temp_current->current_cmd[1]);
//    rwlib_sendModeCommand(2, RW_MODE_CURRENT, temp_current->current_cmd[2]);
//}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  RW_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void RW_ResetCounters(void)
{
    /* Status of commands processed by the SAMPLE App */
    RW_HkTelemetryPkt.rw_command_count       = 0;
    RW_HkTelemetryPkt.rw_command_error_count = 0;

    CFE_EVS_SendEvent(RW_COMMANDRST_INF_EID, CFE_EVS_INFORMATION,
		"RW: RESET command");
    return;

} /* End of RW_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* RW_VerifyCmdLength() -- Verify command packet length                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
// TODO: Used?
boolean RW_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
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

        CFE_EVS_SendEvent(RW_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        RW_HkTelemetryPkt.rw_command_error_count++;
    }

    return(result);
} /* End of RW_VerifyCmdLength() */
