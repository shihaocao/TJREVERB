/*************************************************************************
** File:
**   $Id: hs_custom_freertos.c 1.2 2011/10/17 16:45:21EDT aschoeni Exp  $
**
**   Purpose: Health and Safety (HS) application custom component for running on FreeRTOS.
**   This custom code is designed to work without modification; however the
**   nature of specific platforms may make it desirable to modify this code
**   to work in a more efficient way, or to provide greater functionality.
**
**   $Log: hs_custom.c  $
**
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "cfe_psp.h"
#include "osapi.h"
#include "hs_app.h"
#include "hs_cmds.h"
#include "hs_msg.h"
#include "hs_custom.h"
#include "hs_events.h"
#include "hs_monitors.h"
#include "hs_perfids.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
** Command Codes
** Custom codes must not conflict with those in hs_msgdefs.h
*/
/* No commands for FreeRTOS */

/*
** Event IDs
** Custom Event IDs must not conflict with those in hs_events.h
*/


/** \brief <tt> 'Error Registering Sync Callback for CPU Utilization Monitoring,RC=0x\%08X' </tt>
**  \event <tt> 'Error Registering Sync Callback for CPU Utilization Monitoring,RC=0x\%08X' </tt>
**
**  \par Type: ERROR
**
**  \par Cause:
**
**  This event message is issued when CFS Health and Safety
**  is unable to create its sync callback via the #CFE_TIME_RegisterSynchCallback
**  API
**
**  The \c RC field contains the return status from the
**  #CFE_TIME_RegisterSynchCallback call that generated the error.
*/
#define HS_CR_SYNC_CALLBACK_ERR_EID 101

/*
** Custom Global Data Structure
*/
typedef struct
{

   unsigned long UtilCycleCounter;
   unsigned long TotalRunTime;
   unsigned long IdleTaskRunTime;
   unsigned long Utilization;

} HS_CustomData_t;

HS_CustomData_t HS_CustomData;

/************************************************************************
** Local function prototypes
*************************************************************************/

/************************************************************************/
/** \brief Mark the CPU Utilization Tracker Counter
**
**  \par Description
**       Utility function that marks the CPU Utilization tracking
**       counter while saving the previous value to a variable for use in
**       calculating CPU Utilization and hogging.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \sa #HS_UtilizationIncrement
**
*************************************************************************/
void HS_UtilizationMark(void);

/************************************************************************/
/** \brief Mark Idle Time Callback from Time App
**
**  \par Description
**       This function marks the idle time for the current interval.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void HS_MarkIdleCallback(void);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize                                         */
/*                                                                 */
/* NOTE: For complete prolog information, see 'hs_custom.h'        */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 HS_CustomInit(void)
{
    int32 Status = CFE_SUCCESS;

    /*
    ** Connect to CFE TIME's time reference marker (typically 1 Hz) for Idle Task Marking
    */
    Status = CFE_TIME_RegisterSynchCallback((CFE_TIME_SynchCallbackPtr_t)&HS_MarkIdleCallback);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_CR_SYNC_CALLBACK_ERR_EID, CFE_EVS_ERROR,
                          "Error Registering Sync Callback for CPU Utilization Monitoring,RC=0x%08X",
                          Status);
    }

    HS_CustomData.UtilCycleCounter = 0;
    HS_CustomData.Utilization = 0;
    HS_CustomData.TotalRunTime = 0;
    HS_CustomData.IdleTaskRunTime = 0;

    return(Status);

} /* end HS_CustomInit */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Clean up                                                        */
/*                                                                 */
/* NOTE: For complete prolog information, see 'hs_custom.h'        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_CustomCleanup(void)
{
    int32 Status;    

    /*
    ** Unregister the Sync Callback for CPU Monitoring
    */
    Status = CFE_TIME_UnregisterSynchCallback((CFE_TIME_SynchCallbackPtr_t)&HS_MarkIdleCallback);

} /* end HS_CustomCleanup */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Mark the Utilization Counter                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_UtilizationMark(void)
{
    static uint32 CycleCount = 0;

    unsigned      int percent_idle;
    unsigned      int utilization;
    unsigned      int IdleTime;
    unsigned      int TotalTime;

    CycleCount++;

    if(CycleCount >= HS_UTIL_CALLS_PER_MARK)
    {
       percent_idle = NM_GetIdlePercent(&IdleTime, &TotalTime);
       utilization = (100 - percent_idle);
#if 0
       printf("HS CUSTOM Utilization ----> %u\n",utilization); 
#endif
       HS_CustomData.Utilization = utilization * 100; 
       HS_CustomData.TotalRunTime = TotalTime;
       HS_CustomData.IdleTaskRunTime = IdleTime;

       CycleCount = 0;
    }
   
    return;

} /* end HS_UtilizationMark */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Callback function that marks the Idle time                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_MarkIdleCallback(void)
{
    /*
    ** Capture the CPU Utilization (at a consistant time)
    */
    HS_UtilizationMark();

    return;

} /* End of HS_MarkIdleCallback() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Performs Utilization Monitoring and reporting                   */
/*                                                                 */
/* NOTE: For complete prolog information, see 'hs_custom.h'        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_CustomMonitorUtilization(void)
{

    HS_CustomData.UtilCycleCounter++;
    if (HS_CustomData.UtilCycleCounter >= HS_UTIL_CYCLES_PER_INTERVAL)
    {
        HS_MonitorUtilization();
        HS_CustomData.UtilCycleCounter = 0;
    }

} /* End of HS_CustomMonitorUtilization() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Processes any additional commands                               */
/*                                                                 */
/* NOTE: For complete prolog information, see 'hs_custom.h'        */
/*                                                                 */
/* This function MUST return CFE_SUCCESS if any command is found   */
/* and must NOT return CFE_SUCCESS if no command was found         */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 HS_CustomCommands(CFE_SB_MsgPtr_t MessagePtr)
{
    int32 Status = CFE_SUCCESS;

    uint16          CommandCode = 0;

    CommandCode = CFE_SB_GetCmdCode(MessagePtr);
    switch (CommandCode)
    {
        default:
            Status = !CFE_SUCCESS;
            break;

    } /* end CommandCode switch */

    return Status;

} /* End of HS_CustomCommands() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Report Utilization                                              */
/*                                                                 */
/* NOTE: For complete prolog information, see 'hs_custom.h'        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 HS_CustomGetUtil(void)
{
   int32 CurrentUtil;

   /*
   ** For FreeRTOS, simply report the utilization that is stored in the global data
   */
   CurrentUtil = HS_CustomData.Utilization;

#ifdef HS_DEBUG
   OS_printf("HS CurrentUtil = %d, Percentage = %d\n",CurrentUtil, CurrentUtil/100);
#endif

   return CurrentUtil;
}

/************************/
/*  End of File Comment */
/************************/
