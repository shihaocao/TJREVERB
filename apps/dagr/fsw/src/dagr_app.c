/*******************************************************************************
** File: dagr_app.c
**
** Purpose:
**   This file contains the source code for the Dagr App.
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "dagr_app.h"
#include "dagr_perfids.h"
#include "dagr_msgids.h"
#include "dagr_lib.h"
#include "mode_lib.h"
#include "shk_msgids.h"

#include <math.h>

/*
** global data
*/

dagr_hk_tlm_t       DAGR_HkTelemetryPkt;
dagr_sci_tlm_t      DAGR_SciTelemetryPkt;
dagr_sci_boom_tlm_t DAGR_SciBoomTelemetryPkt;
dagr_acs_tlm_t      DAGR_ACSTelemetryPkt;
CFE_SB_PipeId_t     DAGR_CommandPipe;
CFE_SB_MsgPtr_t     DAGRMsgPtr;
uint8               DAGR_PowerMode = DAGR_PWR_CMD_4;

static CFE_EVS_BinFilter_t  DAGR_EventFilters[] =
       {  /* Event ID    mask */
          {DAGR_STARTUP_INF_EID,       0x0000},
          {DAGR_COMMAND_ERR_EID,       0x0000},
          {DAGR_COMMANDNOP_INF_EID,    0x0000},
          {DAGR_COMMANDRST_INF_EID,    0x0000},
       };

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* DAGR_AppMain() -- Application entry point and main process loop          */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void DAGR_AppMain( void )
{
    int32  Status    = CFE_SUCCESS;
    uint32 RunStatus = CFE_ES_APP_RUN;

    CFE_ES_PerfLogEntry(DAGR_APP_PERF_ID);

    Status = DAGR_AppInit();
    if (Status != CFE_SUCCESS)
    {
        RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** DAGR Runloop
    */
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        CFE_ES_PerfLogExit(DAGR_APP_PERF_ID);

        Status = CFE_SB_RcvMsg(&DAGRMsgPtr, DAGR_CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(DAGR_APP_PERF_ID);

        if (Status == CFE_SUCCESS)
        {
            DAGR_ProcessCommandPacket();
        } else if (Status == CFE_SB_TIME_OUT) {
            // Timeouts are okay.
        } else {
            RunStatus = CFE_ES_APP_ERROR;
        }
    }

    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(DAGR_TASK_EXIT_EID, CFE_EVS_CRITICAL,
                          "Task terminating, err = 0x%08X", Status);
        CFE_ES_WriteToSysLog("DAGR task terminating, err = 0x%08X\n", Status);
    }

    CFE_ES_PerfLogExit(DAGR_APP_PERF_ID);
    CFE_ES_ExitApp(RunStatus);

} /* End of DAGR_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/* DAGR_AppInit() --  initialization                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 DAGR_AppInit(void)
{
    int32 Status;

    // Init DAGR
    DAGR_InitMagDevice();

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
    Status = CFE_EVS_Register(DAGR_EventFilters,
                     sizeof(DAGR_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                     CFE_EVS_BINARY_FILTER);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    /*
    ** Create the Software Bus command pipe and subscribe to housekeeping
    **  messages
    */
    Status = CFE_SB_CreatePipe(&DAGR_CommandPipe, DAGR_PIPE_DEPTH, "DAGR_CMD_PIPE");
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(DAGR_APP_CMD_MID, DAGR_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(DAGR_APP_SEND_HK_MID, DAGR_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(DAGR_APP_1HZ_WAKEUP_MID, DAGR_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    DAGR_ResetCounters();

    CFE_SB_InitMsg(&DAGR_HkTelemetryPkt,
                   DAGR_APP_HK_TLM_MID,
                   DAGR_APP_HK_TLM_LNGTH, TRUE);

    CFE_SB_InitMsg(&DAGR_SciTelemetryPkt,
                   DAGR_APP_SCI_TLM_MID,
                   DAGR_APP_SCI_TLM_LNGTH, TRUE);

    CFE_SB_InitMsg(&DAGR_SciBoomTelemetryPkt,
                    DAGR_APP_SCI_BOOM_TLM_MID,
                    DAGR_APP_SCI_BOOM_TLM_LNGTH, TRUE);

    CFE_SB_InitMsg(&DAGR_ACSTelemetryPkt,
                   DAGR_APP_ACS_TLM_MID,
                   DAGR_APP_ACS_TLM_LNGTH, TRUE);

    CFE_EVS_SendEvent(DAGR_STARTUP_INF_EID, CFE_EVS_INFORMATION,
               "DAGR App Initialized. Version %d.%d.%d.%d",
                DAGR_APP_MAJOR_VERSION,
                DAGR_APP_MINOR_VERSION,
                DAGR_APP_REVISION,
                DAGR_APP_MISSION_REV);

    return CFE_SUCCESS;
} /* End of DAGR_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  DAGR_ProcessCommandPacket                                          */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the DAGR      */
/*     command pipe.                                                          */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void DAGR_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId(DAGRMsgPtr);

    switch (MsgId)
    {
        case DAGR_APP_CMD_MID:
            DAGR_ProcessGroundCommand();
            break;

        case DAGR_APP_SEND_HK_MID:
            DAGR_ReportHousekeeping();
            break;

        case DAGR_APP_1HZ_WAKEUP_MID:
            DAGR_ReportTelemetry();
            break;

        default:
            DAGR_HkTelemetryPkt.dagr_command_error_count++;
            CFE_EVS_SendEvent(DAGR_COMMAND_ERR_EID,CFE_EVS_ERROR,
            "DAGR: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End DAGR_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* DAGR_ProcessGroundCommand() -- DAGR ground commands                        */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void DAGR_ProcessGroundCommand(void)
{
    uint16 CommandCode;

    CommandCode = CFE_SB_GetCmdCode(DAGRMsgPtr);

    /* Process "known" DAGR app ground commands */
    switch (CommandCode)
    {
        case DAGR_APP_NOOP_CC:
            DAGR_HkTelemetryPkt.dagr_command_count++;
            CFE_EVS_SendEvent(DAGR_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION,
            "DAGR: NOOP command. Version %d.%d.%d.%d",
                DAGR_APP_MAJOR_VERSION,
                DAGR_APP_MINOR_VERSION,
                DAGR_APP_REVISION,
                DAGR_APP_MISSION_REV);
            break;

        case DAGR_APP_RESET_COUNTERS_CC:
            DAGR_ResetCounters();
            break;

        default:
            DAGR_HkTelemetryPkt.dagr_command_error_count++;
            CFE_EVS_SendEvent(DAGR_COMMAND_ERR_EID, CFE_EVS_ERROR,
                             "DAGR invalid command code: %d", CommandCode);
            break;
    }
    return;
} /* End of DAGR_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  DAGR_ReportHousekeeping                                            */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void DAGR_ReportHousekeeping(void)
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &DAGR_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &DAGR_HkTelemetryPkt);

    return;
} /* End of DAGR_ReportHousekeeping() */

// Changes DAGR to the specified power mode
void checkSetPowerMode(int intended_mode) {
    int ret_val;

    switch (intended_mode) {
        case MODE_SAFE: ;
            // EPS will have switched off DAGR. Upon power on, default mode will be 4 (all off)
            DAGR_PowerMode = DAGR_PWR_CMD_4;
            break;
        case MODE_CHARGING: ;
            // intentional fall-through to MODE_INMS
        case MODE_INMS: ;
            if (DAGR_PWR_CMD_3 == DAGR_PowerMode) {
                // DAGR is already in correct mode
            } else {
                DAGR_PowerMode = DAGR_PWR_CMD_3;
                #ifdef DAGR_TEST
                printf("DAGR switching mode to: sc %u - dp %u\n", intended_mode, DAGR_PowerMode);
                #endif /* DAGR_TEST */
                // Set command to change mode (same as tlm request)
                // TODO: Error checking?
                ret_val = DAGR_ReadDagrPacket(DAGR_PowerMode, &(DAGR_SciTelemetryPkt.boom_mag_x_pol));
            }
            break;
        case MODE_DAGR: ;
            if (DAGR_PWR_CMD_1 == DAGR_PowerMode) {
                // DAGR is already in correct mode
            } else {
                DAGR_PowerMode = DAGR_PWR_CMD_1;
                #ifdef DAGR_TEST
                printf("DAGR switching mode to: sc %u - dp %u\n", intended_mode, DAGR_PowerMode);
                #endif /* DAGR_TEST */
                // Set command to change mode (same as tlm request)
                // TODO: Error checking?
                ret_val = DAGR_ReadDagrPacket(DAGR_PowerMode, &(DAGR_SciTelemetryPkt.boom_mag_x_pol));
            }
            break;
    } // close switch
}

void DAGR_ReportTelemetry(void) {
    int ret_val;
    uint32 sc_mode;

    // Check spacecraft mode
    sc_mode = MODE_GetSpacecraftMode();

    checkSetPowerMode(sc_mode); // Make sure DAGR power mode is correct for given spacecraft mode

    // set DAGR power mode mode if needed
    if (MODE_SAFE == sc_mode) {
        // Do nothing. Not even ACS.
        #ifdef DAGR_TEST
        printf("DAGR in safe mode: sc %d - dp %d\n", sc_mode, DAGR_PowerMode);
        #endif /* DAGR_TEST */
    } else  {
        #ifdef DAGR_TEST
        printf("DAGR sending telem, mode: sc %d - dp %d\n", sc_mode, DAGR_PowerMode);
        #endif /* DAGR_TEST */
        // Read telemetry data
        ret_val = DAGR_ReadDagrPacket(DAGR_PowerMode, &(DAGR_SciTelemetryPkt.boom_mag_x_pol));

        // Test code
        #ifdef DAGR_TEST
            DAGR_TEST_PrintTlm(&DAGR_SciTelemetryPkt);
        #endif

        // Create and send ACS
        DAGR_ExtractAcs(&DAGR_SciTelemetryPkt, &DAGR_ACSTelemetryPkt);
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &DAGR_ACSTelemetryPkt);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &DAGR_ACSTelemetryPkt);

        // Send either just boom or all sci tlm
        if (MODE_DAGR == sc_mode) {
            CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &DAGR_SciTelemetryPkt);
            CFE_SB_SendMsg((CFE_SB_Msg_t *) &DAGR_SciTelemetryPkt);
        } else {
            // Extract boom telemetry
            memcpy(&(DAGR_SciBoomTelemetryPkt.power_mode), &(DAGR_SciTelemetryPkt.power_mode), 13);
            CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &DAGR_SciBoomTelemetryPkt);
            CFE_SB_SendMsg((CFE_SB_Msg_t *) &DAGR_SciBoomTelemetryPkt);
        }
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  DAGR_ResetCounters                                                 */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void DAGR_ResetCounters(void)
{
    /* Status of commands processed by the DAGR App */
    DAGR_HkTelemetryPkt.dagr_command_count       = 0;
    DAGR_HkTelemetryPkt.dagr_command_error_count = 0;

    CFE_EVS_SendEvent(DAGR_COMMANDRST_INF_EID, CFE_EVS_DEBUG,
                      "DAGR: RESET command");
    return;

} /* End of DAGR_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* DAGR_VerifyCmdLength() -- Verify command packet length                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
boolean DAGR_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
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

        CFE_EVS_SendEvent(DAGR_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        DAGR_HkTelemetryPkt.dagr_command_error_count++;
    }

    return(result);

} /* End of DAGR_VerifyCmdLength() */

// DAGR Calibration
int level_shift_pos[3] = {-212, -242, -225};
int level_shift_neg[3] = {-235, -241, -249};
int cal_24[3] = {258.11, 258.11, 258.11};

// Used for Boom, and Todd
float convertMag24(uint8 polarity, uint8 *value, int index) {
    int intval = 0;
    intval = (value[0] << 16) | (value[1] << 8) | value[2];
    if (1 != polarity) {
        intval = -intval;
        intval = intval + level_shift_neg[index];
    } else {
        intval = intval + level_shift_pos[index];
    }

    return intval / cal_24[index];
}

// 18 bit, 2's complement
// TODO: May only be for testing... but data converted for ACS may need it.
float convertMagTemp(uint8 *value) {
    int intval = 0;
    intval = (value[0] << 16) | (value[1] << 8) | value[2];
    if (intval > 131071) { // strip 2's complement
        intval -= 262144;
    }
    return (625.0 * intval) / 131071;
}

// 18 bit, 2's complement
// TODO: May only be for testing
#ifdef DAGR_TEST
float convertMag18(uint8 *value) {
    int intval = 0;
    intval = (value[0] << 16) | (value[1] << 8) | value[2];
    if (intval > 131071) { // strip 2's complement
        intval -= 262144;
    }
    return 10000 * ((10.24 * intval) / 131071);
}
#endif /* DAGR_TEST */

void DAGR_ExtractAcs(dagr_sci_tlm_t *sci_tlm, dagr_acs_tlm_t *acs_tlm) {
    acs_tlm->boom_mag_x = convertMag24(sci_tlm->boom_mag_x_pol, &(sci_tlm->boom_mag_x_msb), 0);
    acs_tlm->boom_mag_y = convertMag24(sci_tlm->boom_mag_y_pol, &(sci_tlm->boom_mag_y_msb), 1);
    acs_tlm->boom_mag_z = convertMag24(sci_tlm->boom_mag_z_pol, &(sci_tlm->boom_mag_z_msb), 2);
}

#ifdef DAGR_TEST
void DAGR_TEST_PrintTlm(dagr_sci_tlm_t *sci_tlm) {
    printf("\tpower_mode: 0x%02X\n", sci_tlm->power_mode);

    // Boom Mag data
    printf("\t Boom X 0x%02X 0x%06X\tvalue: %fnT\n", sci_tlm->boom_mag_x_pol, (sci_tlm->boom_mag_x_msb << 16) | (sci_tlm->boom_mag_x_nmsb << 8) | sci_tlm->boom_mag_x_lsb, convertMag24(sci_tlm->boom_mag_x_pol, &(sci_tlm->boom_mag_x_msb), 0));
    printf("\t Boom Y 0x%02X 0x%06X\tvalue: %fnT\n", sci_tlm->boom_mag_y_pol, (sci_tlm->boom_mag_y_msb << 16) | (sci_tlm->boom_mag_y_nmsb << 8) | sci_tlm->boom_mag_y_lsb, convertMag24(sci_tlm->boom_mag_y_pol, &(sci_tlm->boom_mag_y_msb), 1));
    printf("\t Boom Z 0x%02X 0x%06X\tvalue: %fnT\n", sci_tlm->boom_mag_z_pol, (sci_tlm->boom_mag_z_msb << 16) | (sci_tlm->boom_mag_z_nmsb << 8) | sci_tlm->boom_mag_z_lsb, convertMag24(sci_tlm->boom_mag_z_pol, &(sci_tlm->boom_mag_z_msb), 2));
    // Todd Mag data
    printf("\t Todd X 0x%02X 0x%06X\tvalue: %fnT\n", sci_tlm->todd_mag_x_pol, (sci_tlm->todd_mag_x_msb << 16) | (sci_tlm->todd_mag_x_nmsb << 8) | sci_tlm->todd_mag_x_lsb, convertMag24(sci_tlm->todd_mag_x_pol, &(sci_tlm->todd_mag_x_msb), 0));
    printf("\t Todd Y 0x%02X 0x%06X\tvalue: %fnT\n", sci_tlm->todd_mag_y_pol, (sci_tlm->todd_mag_y_msb << 16) | (sci_tlm->todd_mag_y_nmsb << 8) | sci_tlm->todd_mag_y_lsb, convertMag24(sci_tlm->todd_mag_y_pol, &(sci_tlm->todd_mag_y_msb), 1));
    printf("\t Todd Z 0x%02X 0x%06X\tvalue: %fnT\n", sci_tlm->todd_mag_z_pol, (sci_tlm->todd_mag_z_msb << 16) | (sci_tlm->todd_mag_z_nmsb << 8) | sci_tlm->todd_mag_z_lsb, convertMag24(sci_tlm->todd_mag_z_pol, &(sci_tlm->todd_mag_z_msb), 2));
    // Billingsley Mag data // TODO: signed?
    printf("\t Bill X 0x%06X\tvalue: %fnT\n", (sci_tlm->bill_mag_x_msb << 16) | (sci_tlm->bill_mag_x_nmsb << 8) | sci_tlm->bill_mag_x_lsb, convertMag18(&(sci_tlm->bill_mag_x_msb)));
    printf("\t Bill Y 0x%06X\tvalue: %fnT\n", (sci_tlm->bill_mag_y_msb << 16) | (sci_tlm->bill_mag_y_nmsb << 8) | sci_tlm->bill_mag_y_lsb, convertMag18(&(sci_tlm->bill_mag_y_msb)));
    printf("\t Bill Z 0x%06X\tvalue: %fnT\n", (sci_tlm->bill_mag_z_msb << 16) | (sci_tlm->bill_mag_z_nmsb << 8) | sci_tlm->bill_mag_z_lsb, convertMag18(&(sci_tlm->bill_mag_z_msb)));
    // Boom Mag temp // TODO: signed?
    printf("\t Boom Temp 0x%06X\tvalue: %fC\n", (sci_tlm->boom_mag_temp_msb << 16) | (sci_tlm->boom_mag_temp_nmsb << 8) | sci_tlm->boom_mag_temp_lsb, convertMagTemp(&(sci_tlm->boom_mag_temp_msb)));
    // Todd Mag temp
    printf("\t Todd Temp 0x%06X\tvalue: %fC\n", (sci_tlm->todd_mag_temp_msb << 16) | (sci_tlm->todd_mag_temp_nmsb << 8) | sci_tlm->todd_mag_temp_lsb, convertMagTemp(&(sci_tlm->todd_mag_temp_msb)));  
}
#endif /* DAGR_TEST */
