/*******************************************************************************
** File: sec_app.c
**
** Purpose:
**   Spacecraft Ephemeric Calculation App
**    Computes the Ephemeris and manages the GPS interface
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "sec_app.h"
#include "sec_perfids.h"
#include "sec_msgids.h"
#include "sec_msg.h"

#include "dhl_lib.h"

// #define _SEC_DEBUG_

/*
** Device specific includes
*/
#include <math.h>


/*
** Defines
*/
#define GPS_OFF           0
#define GPS_ON            1
#define GPS_STARTUP       3

#define GPS_STARTUP_DELAY 20


/*
** global data
*/
sec_hk_tlm_t       SEC_HkTelemetryPkt;
CFE_SB_PipeId_t    SEC_CommandPipe;
CFE_SB_MsgPtr_t    SECMsgPtr;
int                SECCurrentMode = -1;
uint32             SEC_GPSChildTaskID;     /* GPS child task ID */
uint32             SEC_GPSChildSemaphore;  /* GPS child task Semaphore */


static CFE_EVS_BinFilter_t  SEC_EventFilters[] =
       {  /* Event ID    mask */
          {SEC_STARTUP_INF_EID,       0x0000},
          {SEC_COMMAND_ERR_EID,       0x0000},
          {SEC_COMMANDNOP_INF_EID,    0x0000},
          {SEC_COMMANDRST_INF_EID,    0x0000},
       };

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * */
/* SEC_AppMain() -- Application entry point and main process loop        */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * **/
void SEC_AppMain( void )
{
    int32  Status    = CFE_SUCCESS;
    uint32 RunStatus = CFE_ES_APP_RUN;
    int    NewMode;

    CFE_ES_PerfLogEntry(SEC_PERF_ID);

    Status = SEC_AppInit();
    if (Status != CFE_SUCCESS)
    {
        RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** SEC Runloop
    */
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        CFE_ES_PerfLogExit(SEC_PERF_ID);

        /* Pend on receipt of command packet */
        Status = CFE_SB_RcvMsg(&SECMsgPtr, SEC_CommandPipe, CFE_SB_PEND_FOREVER);
        
        CFE_ES_PerfLogEntry(SEC_PERF_ID);

        /*
        ** Check SC Mode. 
        */
        NewMode = MODE_GetSpacecraftMode();
        if (SECCurrentMode != NewMode) 
        {
            #ifdef _SEC_DEBUG_
            OS_printf("SEC : Mode changed. mode = %d\n",NewMode);
            #endif
            SECCurrentMode = NewMode;
        }

        if (Status == CFE_SUCCESS)
        {
            SEC_ProcessCommandPacket();
        } 
        else 
        {
            RunStatus = CFE_ES_APP_ERROR;
        }
    }

    if (Status != CFE_SUCCESS) 
    {
        CFE_EVS_SendEvent(SEC_TASK_EXIT_EID, CFE_EVS_CRITICAL,
                          "Task terminating, err = 0x%08X", Status);
        CFE_ES_WriteToSysLog("SEC task terminating, err = 0x%08X\n", Status);
    }

    CFE_ES_PerfLogExit(SEC_PERF_ID);
    CFE_ES_ExitApp(RunStatus);

} /* End of SEC_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* SEC_AppInit() --  initialization                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 SEC_AppInit(void)
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
    Status = CFE_EVS_Register(SEC_EventFilters,
                     sizeof(SEC_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                     CFE_EVS_BINARY_FILTER);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }    

    /*
    ** Create the Software Bus command pipe and subscribe to housekeeping
    **  messages
    */
    Status = CFE_SB_CreatePipe(&SEC_CommandPipe, SEC_PIPE_DEPTH, "SEC_CMD_PIPE");
    if (Status != CFE_SUCCESS)
    {
        OS_printf("SEC: CFE_SB_CreatePipe returned an error!\n");
        return Status;
    }

    Status = CFE_SB_Subscribe(SEC_CMD_MID, SEC_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(SEC_SEND_HK_MID, SEC_CommandPipe);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_SB_Subscribe(SEC_APP_GPS_POLL_MID, SEC_CommandPipe);
    if (Status != CFE_SUCCESS) 
    {
        return Status;
    }

    /*
    ** Create Binary Semaphore for child task wakeup
    */
    Status = OS_BinSemCreate(&SEC_GPSChildSemaphore,
                             SEC_CHILD_SEM_NAME, 0, 0);
    if (Status != CFE_SUCCESS)
    {
       OS_printf("Error creating SEC GPS child task binary semaphore\n");
       return Status;
    }

    /*
    ** Create the child task that manages the GPS interface
    */
    Status = CFE_ES_CreateChildTask(&SEC_GPSChildTaskID,
                                     SEC_CHILD_TASK_NAME,
                                     (void *) SEC_GPSChildTask, 0,
                                     SEC_CHILD_TASK_STACK_SIZE,
                                     SEC_CHILD_TASK_PRIORITY, 0);
    if (Status != CFE_SUCCESS)
    {
        OS_printf("Error creating SEC GPS child task\n");
        return Status;
    }


    SEC_ResetCounters();
    
    CFE_SB_InitMsg(&SEC_HkTelemetryPkt, SEC_HK_TLM_MID,
                   SEC_HK_TLM_LNGTH, TRUE);

    CFE_EVS_SendEvent (SEC_STARTUP_INF_EID, CFE_EVS_INFORMATION,
               "SEC Initialized. Version %d.%d.%d.%d",
                SEC_MAJOR_VERSION,
                SEC_MINOR_VERSION, 
                SEC_REVISION, 
                SEC_MISSION_REV);

    return CFE_SUCCESS;               
} /* End of SEC_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SEC_ProcessCommandPacket                                            */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the RW        */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SEC_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId(SECMsgPtr);

    switch (MsgId)
    {
        case SEC_CMD_MID:
            SEC_ProcessGroundCommand();
            break;

        case SEC_SEND_HK_MID:
            SEC_ReportHousekeeping();
            break;

        case SEC_APP_GPS_POLL_MID:
            SEC_ProcessGPS();
            break;

        default:
            SEC_HkTelemetryPkt.sec_command_error_count++;
            CFE_EVS_SendEvent(SEC_COMMAND_ERR_EID,CFE_EVS_ERROR,
            "SEC: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End SEC_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SEC_ProcessGPS() -- SEC ground commands                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void SEC_ProcessGPS(void)
{
   OS_BinSemGive(SEC_GPSChildSemaphore);
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SEC_ProcessGroundCommand() -- SEC ground commands                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void SEC_ProcessGroundCommand(void)
{
    uint16 CommandCode;

    CommandCode = CFE_SB_GetCmdCode(SECMsgPtr);

    /* Process SEC app ground commands */
    switch (CommandCode)
    {
        case SEC_NOOP_CC:
            SEC_HkTelemetryPkt.sec_command_count++;
            CFE_EVS_SendEvent(SEC_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION,
            "SEC: NOOP command");
            break;

        case SEC_CONFIG_CC:
             SEC_ConfigCommand();
             break;

        case SEC_RESET_COUNTERS_CC:
            SEC_ResetCounters();
            break;

        /* default case already found during FC vs length test */
        default:
            break;
    }
    return;

} /* End of SEC_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SEC_ReportHousekeeping                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */

void SEC_ReportHousekeeping(void)
{
   CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SEC_HkTelemetryPkt);
   CFE_SB_SendMsg((CFE_SB_Msg_t *) &SEC_HkTelemetryPkt);
   return;

} /* End of SEC_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SEC_ConfigCommand                                                 */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This is a sample command for the SEC app                           */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SEC_ConfigCommand(void) 
{
    sec_config_cmd_t * configCmd = (sec_config_cmd_t *)SECMsgPtr;
    OS_printf("SEC Config Command received, parameter = %d\n", (int)configCmd->SecConfig);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SEC_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SEC_ResetCounters(void)
{
    /* Status of commands processed by the SEC App */
    SEC_HkTelemetryPkt.sec_command_count       = 0;
    SEC_HkTelemetryPkt.sec_command_error_count = 0;

    CFE_EVS_SendEvent(SEC_COMMANDRST_INF_EID, CFE_EVS_INFORMATION,
		"SEC: RESET command");
    return;

} /* End of SEC_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SEC_VerifyCmdLength() -- Verify command packet length                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
// TODO: Used?    
boolean SEC_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
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

        CFE_EVS_SendEvent(SEC_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        SEC_HkTelemetryPkt.sec_command_error_count++;
    }

    return(result);
} /* End of SEC_VerifyCmdLength() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* SEC_GPSChildTask() -- GPS Child Task main loop                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void SEC_GPSChildTask(void)
{
    int32    Result;
    int      ReturnCode;
    int      GpsStatus = 0;
    int32    CurrentMode = 0;
    int32    GpsState = GPS_OFF;         
    int32    GpsStartupDelay = 0;

    /*
    ** The child task runs until the parent dies (normal end) or
    **  until it encounters a fatal error (semaphore error, etc.)...
    */
    Result = CFE_ES_RegisterChildTask();

    if (Result != CFE_SUCCESS)
    {
        OS_printf("SEC_GPSChildTask: RegisterChildTask error!\n");
#if 0
        /* Should be an event message */
        CFE_EVS_SendEvent(FM_CHILD_INIT_ERR_EID, CFE_EVS_ERROR,
           "%s initialization error: register child failed: result = %d",
            TaskText, Result);
#endif
    }
    else
    {
        while ( Result == CFE_SUCCESS )
        {
            /* Pend on the "handshake" semaphore */
            Result = OS_BinSemTake(SEC_GPSChildSemaphore);

            CFE_ES_PerfLogEntry(SEC_APP_GPS_PERF_ID);

            /*
            ** Get the current mode 
            */
            CurrentMode = MODE_GetSpacecraftMode();

            /* 
            ** Check to see if the GPS should be Initialized 
            */
            if ( CurrentMode != MODE_SAFE && GpsState == GPS_OFF )
            {
               GpsState = GPS_STARTUP;
               #ifdef _SEC_DEBUG_
               OS_printf("--- SEC:GPS GPS Init ----\n");
               #endif
               GPS_UartInit(); 
            }
            else if ( CurrentMode == MODE_SAFE ) 
            {
               GpsState = GPS_OFF;
            }

            if ( GpsState == GPS_STARTUP )
            {
               GpsStartupDelay++;
               if ( GpsStartupDelay == GPS_STARTUP_DELAY )
               {
                  GpsStartupDelay = 0;
                  #ifdef _SEC_DEBUG_
                  OS_printf("--- SEC:GPS Finished with GPS init delay ----\n");
                  #endif
                  GpsState = GPS_ON;
               }
            }

            /*
            ** Read the GPS if it is on
            */
            if ( GpsState == GPS_ON ) 
            { 
               GpsStatus = GPS_SampleGPSData();
               if ( GpsStatus & GPS_TIME  )
               {
                  SEC_HkTelemetryPkt.last_gps_time.Seconds = GPS_Data.LastGpsTime.Seconds;
                  SEC_HkTelemetryPkt.last_gps_time.Subseconds = GPS_Data.LastGpsTime.Subseconds;
               } 
               SEC_HkTelemetryPkt.gps_status = GpsStatus;
               SEC_HkTelemetryPkt.num_sats = GPS_Data.NumSats;
               #ifdef _SEC_DEBUG_
               OS_printf("Polled GPS. Status = %d, NumSats = %d\n",GpsStatus, GPS_Data.NumSats);
               #endif
            }
            CFE_ES_PerfLogExit(SEC_APP_GPS_PERF_ID);
        }
    }
    OS_printf("SEC GPS Child task exiting\n");

    /* Stop the parent from invoking the child task */
    SEC_GPSChildSemaphore = 0;

    /* This call allows cFE to clean-up system resources */
    CFE_ES_DeleteChildTask(SEC_GPSChildTaskID);

    return;

} /* End of SEC_GPSChildTask() */


