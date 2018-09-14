/*************************************************************************
 ** File:
 **   $Id: sc_app.c 1.14 2011/09/26 13:51:13EDT lwalling Exp  $
 **
 ** Purpose: 
 **     This file contains the Stored Command main event loop function. It also
 **     contains the initialization function. The SC app handles the scheduling
 **     of stored commands for the fsw. The main event loop handles the Software
 **     Bus interface.
 **
 ** References:
 **   Flight Software Branch C Coding Standard Version 1.2
 **   CFS Development Standards Document
 **
 ** History:
 **   $Log: sc_app.c  $
 **   Revision 1.14 2011/09/26 13:51:13EDT lwalling 
 **   Remove references to CDS, create common startup functions
 **   Revision 1.13 2011/03/15 17:33:42EDT lwalling 
 **   Delete enable auto-exec RTS from load defaults, add select auto-exec RTS per boot type
 **   Revision 1.12 2010/10/01 13:38:34EDT lwalling 
 **   Add call to SC_RegisterManageCmds() in SC_AppInit()
 **   Revision 1.11 2010/09/28 10:40:41EDT lwalling 
 **   Update list of included header files, add SC_GetTableAddresses(), add SC_RegisterManageCmds()
 **   Revision 1.10 2010/05/12 11:12:37EDT lwalling 
 **   Create info event for RTS load count during non-CDS startup
 **   Revision 1.9 2010/04/05 11:47:06EDT lwalling 
 **   Register Append ATS tables and restore table data from CDS
 **   Revision 1.8 2010/03/11 16:27:55EST lwalling 
 **   Create table file name definition separate from table object name
 **   Revision 1.7 2010/03/09 15:11:28EST lwalling 
 **   Change CDS cfg definition from ifdef or ifndef to if true or if false
 **   Revision 1.6 2009/06/12 14:40:22EDT rmcgraw 
 **   DCR82191:1 Changed OS_Mem function calls to CFE_PSP_Mem
 **   Revision 1.5 2009/02/25 14:56:54EST nyanchik 
 **   Check in of new CDS Recovery method
 **   Revision 1.4 2009/02/19 10:07:10EST nyanchik 
 **   Update SC To work with cFE 5.2 Config parameters
 **   Revision 1.3 2009/01/26 14:44:41EST nyanchik 
 **   Check in of Unit test
 **   Revision 1.2 2009/01/05 08:26:48EST nyanchik 
 **   Check in after code review changes
 *************************************************************************/

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/

#include "cfe.h"
#include "sc_app.h"
#include "sc_rts.h"
#include "sc_cmds.h"
#include "sc_loads.h"
#include "sc_events.h"
#include "sc_msgids.h"
#include "sc_perfids.h"
#include "sc_version.h"


void CFE_PSP_RamDiskFreeSpace(void);

/**************************************************************************
 **
 ** Function prototypes
 **
 **************************************************************************/

/************************************************************************/
/** \brief Main loop for SC
 **  
 **  \par Description
 **       This function is the entry point and main loop for the Stored
 **       Commands (SC) application.
 **       
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 *************************************************************************/
void SC_AppMain (void);

/************************************************************************/
/** \brief Initialize application
 **  
 **  \par Description
 **       This function initializes the SC application. The return value
 **       is either CFE_SUCCESS or the error code from the failed cFE
 **       function call. Note that all errors generate an identifying
 **       event message.
 **       
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_AppInit (void);

/************************************************************************/
/** \brief Initialize application tables
 **  
 **  \par Description
 **       This function initializes the SC application tables. The
 **       return value is either CFE_SUCCESS or the error code from the
 **       failed cFE function call. Note that all errors generate an
 **       identifying event message.
 **       
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_InitTables (void);

/************************************************************************/
/** \brief Register tables with cFE Table Services
 **  
 **  \par Description
 **       This function registers all SC tables with cFE Table Services.
 **       The return value is either CFE_SUCCESS or the error code from
 **       the failed cFE function call. Note that all errors generate an
 **       identifying event message.
 **       
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_RegisterAllTables (void);

/************************************************************************/
/** \brief Get dump only table buffer pointers
 **  
 **  \par Description
 **       This function acquires buffer pointers to the dump only tables.
 **       The return value is either CFE_SUCCESS or the error code from
 **       the failed cFE function call. Note that all errors generate an
 **       identifying event message.
 **       
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_GetDumpTablePointers (void);

/************************************************************************/
/** \brief Get loadable table buffer pointers
 **  
 **  \par Description
 **       This function acquires buffer pointers to the loadable tables.
 **       The return value is either CFE_SUCCESS or the error code from
 **       the failed cFE function call. Note that all errors generate an
 **       identifying event message.
 **       
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
int32 SC_GetLoadTablePointers (void);

/************************************************************************/
/** \brief Load default RTS tables
 **  
 **  \par Description
 **       This function loads the default RTS tables. The return value
 **       is either CFE_SUCCESS or the error code from the failed cFE
 **       function call. Note that all errors generate an identifying
 **       event message.
 **       
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \returns
 **  \retcode #CFE_SUCCESS \retdesc \copydoc CFE_SUCCESS \endcode
 **  \retstmt Return status from CFE initialization function \endcode
 **  \endreturns
 **
 *************************************************************************/
void  SC_LoadDefaultTables (void);

/************************************************************************/
/** \brief Register to receive cFE Table Services manage request commands
 **
 **  \par Description
 **       This function provides cFE Table Services with the information
 **       necessary to send a notification command when one of the SC dump
 **       only tables has a dump pending, or when one of the SC loadable
 **       tables has a load pending.  Upon receipt of the command, the
 **       command handler will call the cFE Table Services API function
 **       to manage the table.  This sequence of events ensures that dump
 **       tables are not being updated by SC at the same moment that the
 **       dump occurs, and likewise, that loadable tables are not being
 **       referenced by SC at the moment that the update occurs.
 **
 **  \par Assumptions, External Events, and Notes:
 **        None
 **
 **  \param [in]         (none)
 **
 **  \sa #SC_TableManageCmd
 **
 *************************************************************************/
void SC_RegisterManageCmds(void);


/**************************************************************************
 **
 ** Global variables
 **
 **************************************************************************/

SC_AppData_t  SC_AppData;         /* SC Application Data */
SC_OperData_t SC_OperData;        /* SC Operational Data */


/**************************************************************************
 **
 ** Functions
 **
 **************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  SC main process loop (task entry point)                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_AppMain(void)
{   
    uint32 RunStatus = CFE_ES_APP_RUN;
    int32 Result;

    /* Register application with cFE */
    Result = CFE_ES_RegisterApp();

    /* Performance Log (start time counter) */
    CFE_ES_PerfLogEntry(SC_APPMAIN_PERF_ID);

    /* Startup initialization */
    if (Result == CFE_SUCCESS)
    {
        Result = SC_AppInit();                  
    }

    /* Check for start-up error */
    if (Result != CFE_SUCCESS)
    {
        /* Set request to terminate main loop */
        RunStatus = CFE_ES_APP_ERROR;
    }

    /* Main process loop */
    while (CFE_ES_RunLoop(&RunStatus))
    {
        /* Performance Log (stop time counter) */
        CFE_ES_PerfLogExit(SC_APPMAIN_PERF_ID);

        /* Pend on Software Bus for message */
        Result = CFE_SB_RcvMsg(&SC_OperData.MsgPtr, SC_OperData.CmdPipe, CFE_SB_PEND_FOREVER);

        /* Performance Log (start time counter) */
        CFE_ES_PerfLogEntry(SC_APPMAIN_PERF_ID);

        /* Check for Software Bus error */
        if (Result == CFE_SUCCESS)
        {
            /* Invoke command handlers */
            SC_ProcessRequest(SC_OperData.MsgPtr);   
        }
        else
        {
            /* Exit main process loop */
            RunStatus = CFE_ES_APP_ERROR;
        }
    }
    
    /* Check for "fatal" process error */
    if (Result !=  CFE_SUCCESS)
    {
        /* Send event describing reason for termination */
        CFE_EVS_SendEvent(SC_APP_EXIT_ERR_EID, CFE_EVS_ERROR,
           "App terminating, Result = 0x%08X", Result);

        /* In case cFE Event Services is not working */
        CFE_ES_WriteToSysLog("SC App terminating, Result = 0x%08X\n", Result);
    }

    /* Performance Log (stop time counter) */
    CFE_ES_PerfLogExit(SC_APPMAIN_PERF_ID);
    
    /* Let cFE kill the app */
    CFE_ES_ExitApp(RunStatus);
    
} /* end SC_AppMain() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Application startup initialization                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SC_AppInit(void)
{
    int32 Result;
            
    /* Clear global data structures */
    CFE_PSP_MemSet(&SC_OperData, 0, sizeof(SC_OperData_t));
    CFE_PSP_MemSet(&SC_AppData,  0, sizeof(SC_AppData_t));
       
    /* Number of ATS and RTS commands already executed this second */
    SC_OperData.NumCmdsSec = 0;

    /* Continue ATS execution if ATS command checksum fails */
    SC_AppData.ContinueAtsOnFailureFlag = SC_CONT_ON_FAILURE_START;
    
    /* Make sure nothing is running */
    SC_AppData.NextProcNumber = SC_NONE;
    SC_AppData.NextCmdTime[SC_ATP] = SC_MAX_TIME;
    SC_AppData.NextCmdTime[SC_RTP] = SC_MAX_TIME;

    /* Initialize the SC housekeeping packet */
    CFE_SB_InitMsg(&SC_OperData.HkPacket, SC_HK_TLM_MID, sizeof(SC_HkTlm_t), FALSE);
        
    /* Select auto-exec RTS to start during first HK request */
    if (CFE_ES_GetResetType(NULL) == CFE_ES_POWERON_RESET)
    {
        SC_AppData.AutoStartRTS = RTS_ID_AUTO_POWER_ON;    
    }    
    else
    {
        SC_AppData.AutoStartRTS = RTS_ID_AUTO_PROCESSOR;    
    }    

    /* Must be able to register for events */
    Result = CFE_EVS_Register(NULL,0,CFE_EVS_NO_FILTER);
    if (Result !=  CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Event Services Register returned: 0x%08X\n", Result);
        return(Result);
    }

    /* Must be able to create Software Bus message pipe */
    Result = CFE_SB_CreatePipe(&SC_OperData.CmdPipe, SC_PIPE_DEPTH, SC_CMD_PIPE_NAME);
    if (Result !=  CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_CREATE_ERR_EID, CFE_EVS_ERROR,
                         "Software Bus Create Pipe returned: 0x%08X", Result);
        return(Result);
    }
    
    /* Must be able to subscribe to HK request command */
    Result = CFE_SB_Subscribe(SC_SEND_HK_MID, SC_OperData.CmdPipe);
    if (Result !=  CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_SUBSCRIBE_HK_ERR_EID, CFE_EVS_ERROR,
                         "Software Bus subscribe to housekeeping returned: 0x%08X", Result);
        return(Result);
    }
    
    /* Must be able to subscribe to 1Hz wakeup command */
    Result = CFE_SB_Subscribe(SC_1HZ_WAKEUP_MID, SC_OperData.CmdPipe);
    if (Result !=  CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_SUBSCRIBE_1HZ_ERR_EID, CFE_EVS_ERROR,
                         "Software Bus subscribe to 1 Hz cycle returned: 0x%08X", Result);        
        return(Result);
    }

    /* Must be able to subscribe to SC commands */
    Result = CFE_SB_Subscribe(SC_CMD_MID, SC_OperData.CmdPipe);
    if (Result !=  CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_INIT_SB_SUBSCRIBE_CMD_ERR_EID, CFE_EVS_ERROR,
                         "Software Bus subscribe to command returned: 0x%08X", Result);
        return(Result);
    }

    /* Must be able to create and initialize tables */
    Result = SC_InitTables();
    if (Result !=  CFE_SUCCESS)
    {
        return(Result);
    }

    /* Send application startup event */
    CFE_EVS_SendEvent(SC_INIT_INF_EID, CFE_EVS_INFORMATION, 
                     "SC Initialized. Version %d.%d.%d.%d",
                      SC_MAJOR_VERSION, SC_MINOR_VERSION, SC_REVISION, SC_MISSION_REV);

    return(CFE_SUCCESS);

} /* end SC_AppInit() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Create and initialize loadable and dump-only tables             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SC_InitTables(void)
{
    int32 Result;
    int32 i;
    int32 j;

    /* Must be able to register all tables with cFE Table Services */
    Result = SC_RegisterAllTables();
    if (Result !=  CFE_SUCCESS)
    {
        return(Result);
    }

    /* Must be able to get dump only table pointers */
    Result = SC_GetDumpTablePointers();
    if (Result !=  CFE_SUCCESS)
    {
        return(Result);
    }

    /* ATP control block status table */
    SC_OperData.AtsCtrlBlckAddr -> AtpState =  SC_IDLE;
    SC_OperData.AtsCtrlBlckAddr -> AtsNumber = SC_NO_ATS;
    SC_OperData.AtsCtrlBlckAddr -> CmdNumber = 0xFFFF;
    
    /* RTP control block status table */
    SC_OperData.RtsCtrlBlckAddr -> NumRtsActive = 0;
    SC_OperData.RtsCtrlBlckAddr -> RtsNumber = SC_INVALID_RTS_NUMBER;
    
    /* ATS command status table(s) */
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
	    {
        for (j = 0; j < SC_MAX_ATS_CMDS; j++)
        {
            SC_OperData.AtsCmdStatusTblAddr[i][j] = SC_EMPTY;
        }
    }    

    /* RTS information table */
    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        SC_OperData.RtsInfoTblAddr[i].NextCommandTime = SC_MAX_TIME;
        SC_OperData.RtsInfoTblAddr[i].NextCommandPtr = 0;
        SC_OperData.RtsInfoTblAddr[i].RtsStatus = SC_EMPTY;
        SC_OperData.RtsInfoTblAddr[i].DisabledFlag = TRUE;
    }

    /* Load default RTS tables */  
    SC_LoadDefaultTables();

    /* Must be able to get loadable table pointers */
    Result = SC_GetLoadTablePointers();
    if (Result !=  CFE_SUCCESS)
    {
        return(Result);
    }

    /* Register for table update notification commands */    
    SC_RegisterManageCmds();     

    return(CFE_SUCCESS);   

} /* end SC_InitTables() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Register all SC tables with cFE Table Services                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SC_RegisterAllTables(void)
{
    int i;
    int32 Result;
    int32 TableSize;
    uint16 TableOptions;
    char TableName[CFE_TBL_MAX_NAME_LENGTH];

    /* Set table options for dump only tables */
    TableOptions = (CFE_TBL_OPT_DEFAULT | CFE_TBL_OPT_SNGL_BUFFER | CFE_TBL_OPT_DUMP_ONLY);

    /* Register dump only RTS information table */
    TableSize = sizeof(SC_RtsInfoEntry_t) * SC_NUMBER_OF_RTS;
    Result = CFE_TBL_Register(&SC_OperData.RtsInfoHandle,
                 SC_RTSINFO_TABLE_NAME, TableSize, TableOptions, NULL);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_RTS_INFO_TABLE_ERR_EID, CFE_EVS_ERROR,
           "RTS info table register failed, returned: 0x%08X", Result);
        return(Result);
    }

    /* Register dump only RTP control block table */
    TableSize = sizeof(SC_RtpControlBlock_t);
    Result = CFE_TBL_Register(&SC_OperData.RtsCtrlBlckHandle,
                 SC_RTP_CTRL_TABLE_NAME, TableSize, TableOptions, NULL);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_RTS_CTRL_BLK_TABLE_ERR_EID, CFE_EVS_ERROR,
           "RTS control block table register failed, returned: 0x%08X", Result);
        return(Result);
    }

    /* Register dump only ATS information table */
    TableSize = sizeof(SC_AtsInfoTable_t) * SC_NUMBER_OF_ATS;
    Result = CFE_TBL_Register(&SC_OperData.AtsInfoHandle,
                 SC_ATSINFO_TABLE_NAME, TableSize, TableOptions, NULL);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_ATS_INFO_TABLE_ERR_EID, CFE_EVS_ERROR,
           "ATS Info table register failed, returned: 0x%08X", Result);
        return(Result);
    }
    
    /* Register dump only Append ATS information table */
    TableSize = sizeof(SC_AtsInfoTable_t);
    Result = CFE_TBL_Register(&SC_OperData.AppendInfoHandle,
                 SC_APPENDINFO_TABLE_NAME, TableSize, TableOptions, NULL);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_APPEND_INFO_TABLE_ERR_EID, CFE_EVS_ERROR,
           "Append ATS Info table register failed, returned: 0x%08X", Result);
        return(Result);
    }
    
    /* Register dump only ATP control block table */
    TableSize = sizeof(SC_AtpControlBlock_t);
    Result = CFE_TBL_Register(&SC_OperData.AtsCtrlBlckHandle,
                 SC_ATS_CTRL_TABLE_NAME, TableSize, TableOptions, NULL);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_ATS_CTRL_BLK_TABLE_ERR_EID, CFE_EVS_ERROR,
           "ATS control block table register failed, returned: 0x%08X", Result);
        return(Result);
    }
    
    /* Register dump only ATS command status tables */
    TableSize = SC_MAX_ATS_CMDS * sizeof(uint8);
    for (i  = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        sprintf(TableName, "%s%d", SC_ATS_CMD_STAT_TABLE_NAME, i+1);
        Result = CFE_TBL_Register(&SC_OperData.AtsCmdStatusHandle[i],
                     TableName, TableSize, TableOptions, NULL);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SC_REGISTER_ATS_CMD_STATUS_TABLE_ERR_EID, CFE_EVS_ERROR,
               "ATS command status table register failed for ATS %d, returned: 0x%08X", i+1, Result);
            return(Result);
        }
    }

    /* Register loadable RTS tables */
    TableOptions = CFE_TBL_OPT_DEFAULT | CFE_TBL_OPT_SNGL_BUFFER;
    TableSize = SC_RTS_BUFF_SIZE * SC_BYTES_IN_WORD;
    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        sprintf(TableName, "%s%03d", SC_RTS_TABLE_NAME, i+1);
        Result = CFE_TBL_Register(&SC_OperData.RtsTblHandle[i],
                     TableName, TableSize, TableOptions, SC_ValidateRts);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SC_REGISTER_RTS_TBL_ERR_EID, CFE_EVS_ERROR,
               "RTS Table Registration Failed for RTS %d, returned: 0x%08X", i+1, Result);                      
            return(Result);
        }    
    }

    /* Register loadable ATS tables */
    TableOptions = CFE_TBL_OPT_DBL_BUFFER;
    TableSize = SC_ATS_BUFF_SIZE * SC_BYTES_IN_WORD;
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        sprintf(TableName, "%s%d", SC_ATS_TABLE_NAME, i+1);
        Result = CFE_TBL_Register(&SC_OperData.AtsTblHandle[i],
                     TableName, TableSize, TableOptions, SC_ValidateAts);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SC_REGISTER_ATS_TBL_ERR_EID, CFE_EVS_ERROR,
               "ATS Table Registration Failed for ATS %d, returned: 0x%08X", i+1, Result);
            return(Result);
        }    
    }    

    /* Register loadable Append ATS table */
    TableOptions = CFE_TBL_OPT_DBL_BUFFER;
    TableSize = SC_APPEND_BUFF_SIZE * SC_BYTES_IN_WORD;
    Result = CFE_TBL_Register(&SC_OperData.AppendTblHandle,
                 SC_APPEND_TABLE_NAME, TableSize, TableOptions, SC_ValidateAppend);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_REGISTER_APPEND_TBL_ERR_EID, CFE_EVS_ERROR,
           "Append ATS Table Registration Failed, returned: 0x%08X", Result);
        return(Result);
    }    

    return(CFE_SUCCESS);   

} /* end SC_RegisterAllTables() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get buffer pointers for dump only tables                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SC_GetDumpTablePointers(void)
{
    int i;
    int32 Result;

    /* Get buffer address for dump only RTS information table */
    Result = CFE_TBL_GetAddress((void **) &SC_OperData.RtsInfoTblAddr, SC_OperData.RtsInfoHandle);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_RTS_INFO_ERR_EID, CFE_EVS_ERROR,
           "RTS Info table failed Getting Address, returned: 0x%08X", Result);
        return(Result);
    }

    /* Get buffer address for dump only RTP control block table */
    Result = CFE_TBL_GetAddress((void **) &SC_OperData.RtsCtrlBlckAddr, SC_OperData.RtsCtrlBlckHandle);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_RTS_CTRL_BLCK_ERR_EID, CFE_EVS_ERROR,
           "RTS Ctrl Blck table failed Getting Address, returned: 0x%08X", Result);
        return(Result);
    }

    /* Get buffer address for dump only ATS information table */
    Result = CFE_TBL_GetAddress((void **) &SC_OperData.AtsInfoTblAddr, SC_OperData.AtsInfoHandle);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_ATS_INFO_ERR_EID, CFE_EVS_ERROR,
           "ATS Info table failed Getting Address, returned: 0x%08X", Result);
        return(Result);
    }
    
    /* Get buffer address for dump only Append ATS information table */
    Result = CFE_TBL_GetAddress((void **) &SC_OperData.AppendInfoTblAddr, SC_OperData.AppendInfoHandle);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_APPEND_INFO_ERR_EID, CFE_EVS_ERROR,
           "Append ATS Info table failed Getting Address, returned: 0x%08X", Result);
        return(Result);
    }

    /* Get buffer address for dump only ATP control block table */
    Result = CFE_TBL_GetAddress((void **) &SC_OperData.AtsCtrlBlckAddr, SC_OperData.AtsCtrlBlckHandle);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SC_GET_ADDRESS_ATS_CTRL_BLCK_ERR_EID, CFE_EVS_ERROR,
           "ATS Ctrl Blck table failed Getting Address, returned: 0x%08X", Result);
        return(Result);
    }
    
    /* Get buffer address for dump only ATS command status tables */
    for (i  = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        Result = CFE_TBL_GetAddress((void **) &SC_OperData.AtsCmdStatusTblAddr[i], SC_OperData.AtsCmdStatusHandle[i]);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SC_GET_ADDRESS_ATS_CMD_STAT_ERR_EID, CFE_EVS_ERROR,
               "ATS Cmd Status table for ATS %d failed Getting Address, returned: 0x%08X", i+1, Result);
            return(Result);
        }
    }

    return(CFE_SUCCESS);   

} /* end SC_GetDumpTablePointers() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get buffer pointers for loadable tables                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SC_GetLoadTablePointers(void)
{
    int i;
    int32 Result;

    /* Get buffer address for loadable ATS tables */
    for (i = 0; i < SC_NUMBER_OF_ATS; i++)
    {        
        Result = CFE_TBL_GetAddress((void **) &SC_OperData.AtsTblAddr[i], SC_OperData.AtsTblHandle[i]);
                                        
        if ((Result != CFE_TBL_ERR_NEVER_LOADED) && (Result != CFE_TBL_INFO_UPDATED))
        {
            CFE_EVS_SendEvent(SC_GET_ADDRESS_ATS_ERR_EID, CFE_EVS_ERROR,
               "ATS table %d failed Getting Address, returned: 0x%08X", i+1, Result);
            return(Result);
        }
    }

    /* Get buffer address for loadable ATS Append table */
    Result = CFE_TBL_GetAddress((void **) &SC_OperData.AppendTblAddr, SC_OperData.AppendTblHandle);

    if ((Result !=  CFE_TBL_ERR_NEVER_LOADED) && (Result !=  CFE_TBL_INFO_UPDATED))
    {
        CFE_EVS_SendEvent (SC_GET_ADDRESS_APPEND_ERR_EID, CFE_EVS_ERROR,
           "Append ATS table failed Getting Address, returned: 0x%08X", Result);
        return(Result);
    }

    /* Get buffer address for loadable RTS tables */
    for (i = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        Result = CFE_TBL_GetAddress((void **) &SC_OperData.RtsTblAddr[i], SC_OperData.RtsTblHandle[i]);
        
        if ((Result != CFE_TBL_ERR_NEVER_LOADED) && (Result != CFE_TBL_INFO_UPDATED))
        {
            CFE_EVS_SendEvent(SC_GET_ADDRESS_RTS_ERR_EID, CFE_EVS_ERROR,
               "RTS table %d failed Getting Address, returned: 0x%08X", i+1, Result);
            return(Result);
        }
        
        /* Process new RTS table data */
        if (Result == CFE_TBL_INFO_UPDATED)
        {
            SC_LoadRts(i);
        } 
    }
    
    return(CFE_SUCCESS);   

} /* end SC_GetLoadTablePointers() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Unbundle a file of RTSs.                                        */
/* This is a special function for the Dellingr mission.            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
** Global memory for the Unbundle functions
*/
static char sc_rts_file_buff[420];
static char sc_bundle_load_buffer[128];

int32 SC_UnbundleRTSs(const char *bundle_path, const char *rts_path, uint32 rts_size, uint32 starting_rts_num)
{
   int    bundle_fd;
   int    rts_fd;
   char   rts_filename[OS_MAX_PATH_LEN];
   int    n;       
   uint32 rts_number = starting_rts_num;
   int32  ReturnCode = CFE_SUCCESS;

   bundle_fd = OS_open( bundle_path, OS_READ_ONLY, 0);
   if (bundle_fd < 0) 
   {
      printf("SC Unbundle -- Cannot open bundle file: %s\n",bundle_path);
      return OS_ERROR;
   }

   /*
   ** Read until there is an error or EOF
   */
   while (1) 
   {
      n = OS_read(bundle_fd, sc_rts_file_buff, rts_size);
      if (n != rts_size) 
      {
         if ( n != 0 ) 
         {
            printf("------> SC -- did not read %d bytes: %d\n", (int)rts_size, n); 
            ReturnCode = OS_ERROR; 
         }
         /* May set error codes, finished should = 0 */
         break;
      }

      /*
      ** Build RTS Name
      */
      sprintf(rts_filename, "%s/rts_%03u.tbl", rts_path, (unsigned int)rts_number);

      /*
      ** Open/create the RTS file
      */
      rts_fd = OS_creat(rts_filename, OS_WRITE_ONLY); 
      if ( rts_fd >= 0 ) 
      {
         /* printf("------> SC -- Wrtiting new RTS: %s\n",rts_filename); */
         (void) OS_write(rts_fd, sc_rts_file_buff, n);
         OS_close(rts_fd);
      }
      else
      {
         printf("SC Unbundle -- could not open new RTS: %s\n",rts_filename);
      }

      rts_number++;
    }
    OS_close(bundle_fd);
      
    return ReturnCode;
}

/*
**---------------------------------------------------------------------------------------
** Name: SC_ParseFileEntry
**
**   Purpose: This function parses the bundle file line. The line data comes
**            from the "bundles.txt" file which describes the bundle file, 
**            destination RTS path, RTS size, and starting RTS Number for the 
**            bundle. 
**---------------------------------------------------------------------------------------
*/
int32 SC_ParseFileEntry(char *FileEntry)
{
   char   BundleName[OS_MAX_PATH_LEN];
   char   DestPath[OS_MAX_API_NAME];
   uint32 RtsSize;
   uint32 StartingRtsNum; 
   int    ScanfStatus;
   int32  UnbundleStatus;

   /*
   ** Use sscanf to parse the string.
   ** When this routine is called, the buffer is already verified to be less than the 
   ** maximum size, and the parameters have been separated by a single space each.
   ** The white space has also been stripped out ( except for the spaces between each field ) 
   ** If the sscanf fails, then it is a malformed file entry.  
   */
   ScanfStatus = sscanf(FileEntry,"%s %s %u %u",
                         BundleName, 
                         DestPath, 
                         (unsigned int *)&RtsSize, 
                         (unsigned int *)&StartingRtsNum);

   /*
   ** Check to see if the correct number of items were parsed
   */
   if ( ScanfStatus != 4 )
   {
      printf("SC: Invalid bundle file entry: %d\n",ScanfStatus);
      return (ScanfStatus);
   }

   #if 0 
       printf("----> SC About to unbundle\n");
       printf("    ---> BundleName = %s\n",BundleName);
       printf("    ---> DestPath   = %s\n",DestPath);
       printf("    ---> RtsSize    = %d\n",(int)RtsSize);
       printf("    ---> StartingRtsNum = %d\n",(int)StartingRtsNum);
   #endif
 
   /*
   ** Now unbundle the RTSs into RAM
   */
   UnbundleStatus = SC_UnbundleRTSs(BundleName, DestPath, RtsSize, StartingRtsNum);

   if ( UnbundleStatus == CFE_SUCCESS )
   {
       SC_AppData.CmdCtr++;
       CFE_EVS_SendEvent(SC_UNBUNDLE_RTS_INF_EID, CFE_EVS_ERROR,
                         "Unbundle RTS OK: %s, Dest: %s, RTS Size: %d, RTS Num: %d",
                          BundleName,
                          DestPath,
                          RtsSize,
                          StartingRtsNum);
   }
   else
   {
      SC_AppData.CmdErrCtr++;
      CFE_EVS_SendEvent(SC_UNBUNDLE_RTS_ERR_EID, CFE_EVS_ERROR,
                        "Unbundle RTS Failed. Status = %d",
                         UnbundleStatus);
   }

   return (CFE_SUCCESS);
}

/*
**---------------------------------------------------------------------------------------
** Name: SC_ProcessBundleFile
**
**   Purpose: This function opens the file "bundles.txt" and processes 
**            each line for parsing. 
**            The code is based on the ES code that processes the cfe_es_startup.scr file.
**---------------------------------------------------------------------------------------
*/
int32 SC_ProcessBundleFile(const char *file_name)
{
   int32  BundleFd;
   int32  ReadStatus;
   uint32 BuffLen;
   uint32 LineTooLong; 
   char   c;
  
   BundleFd = OS_open( file_name, OS_READ_ONLY, 0);
   if (BundleFd < 0) 
   {
      printf("-----> SC -- Cannot open bundle description file: %s\n",file_name);
      return (OS_ERROR);
   }

   CFE_PSP_MemSet(sc_bundle_load_buffer, 0, 128);
   BuffLen = 0;      
   LineTooLong = FALSE;
      
   /*
   ** Parse the lines from the file. If it has an error
   ** or reaches EOF, then abort the loop.
   */
   while(1)
   {
      ReadStatus = OS_read(BundleFd, &c, 1);
      if ( ReadStatus == OS_FS_ERROR )
      {
         printf("SC: Error Reading bundle description file. EC = 0x%08X\n",(unsigned int)ReadStatus);
         break;
      }
      else if ( ReadStatus == 0 )
      {
         /*
         ** EOF Reached
         */
         break;
      }
      else if(c != '!')
      {
          if ( c <= ' ')
          {
             /*
             ** Skip all white space in the file
             */
             ;
          }
          else if ( c == ',' )
          {
             /*
             ** replace the field delimiter with a space
             ** This is used for the sscanf string parsing
             */
             if ( BuffLen < 128 )
             {
                sc_bundle_load_buffer[BuffLen] = ' ';
             }
             else
             {
                LineTooLong = TRUE;
             } 
             BuffLen++;
          }
          else if ( c != ';' )
          {
             /*
             ** Regular data gets copied in
             */
             if ( BuffLen < 128 )
             {
                sc_bundle_load_buffer[BuffLen] = c;
             }
             else
             {
                LineTooLong = TRUE;
             }
             BuffLen++;
          }
          else
          {
             if ( LineTooLong == TRUE )
             {   
                /*
                ** The was too big for the buffer
                */
                printf("SC: bundle description file line too long: %d bytes.\n",(int)BuffLen);
                LineTooLong = FALSE;
             }
             else
             {
                /*
                ** Send the line to the file parser
                */
                SC_ParseFileEntry(sc_bundle_load_buffer); 
             }
             CFE_PSP_MemSet(sc_bundle_load_buffer, 0, 128);
             BuffLen = 0;
          }
      }
      else
      {
        /*
        ** break when EOF character '!' is reached
        */
        break;
      }
   }
   /*
   ** close the file
   */
   OS_close(BundleFd);
       
   return(CFE_SUCCESS);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Load default startup tables from non-volatile memory            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SC_LoadDefaultTables(void)
{
    char    TableName[OS_MAX_PATH_LEN];
    int32   FileDesc;
    int32   RtsIndex;
    int32   RtsCount = 0;
    int32   BundleStatus;

    int i,n;
    char rts_filename[OS_MAX_PATH_LEN];
    int32 rts_fd;
 
    /*
    ** Currently, only RTS tables are loaded during initialization.
    **
    ** ATS and ATS Append tables must be loaded by command.
    */ 
    #if 0       
       printf("SC ---> Report RAM Disk Free Space before unpacking bundles\n");
       CFE_PSP_RamDiskFreeSpace();
    #endif
   
    BundleStatus = SC_ProcessBundleFile(SC_RTS_BUNDLE_DESC_FILE);
    if ( BundleStatus != 0 ) 
    {
       printf("SC Unbundle - Process Bundle File failed\n");
    }

    #if 0
       printf("SC ---> Report RAM Disk Free Space After unpacking bundles\n");
       CFE_PSP_RamDiskFreeSpace();
    #endif

    /*
    ** load the first 3 RTSs from flash ( Dellingr specific ) 
    */
    FileDesc = OS_open(SC_BOOT_RTS_1, OS_READ_ONLY, 0);
    if (FileDesc >= 0)
    {
        OS_close(FileDesc);
        CFE_TBL_Load(SC_OperData.RtsTblHandle[0], CFE_TBL_SRC_FILE, SC_BOOT_RTS_1);
        /* printf("SC: Loaded RTS 1 from boot drive\n"); */
        RtsCount++;
    } 
    FileDesc = OS_open(SC_BOOT_RTS_2, OS_READ_ONLY, 0);
    if (FileDesc >= 0)
    {
        OS_close(FileDesc);
        CFE_TBL_Load(SC_OperData.RtsTblHandle[1], CFE_TBL_SRC_FILE, SC_BOOT_RTS_2);
        /* printf("SC: Loaded RTS 2 from boot drive\n"); */
        RtsCount++;
    } 
    FileDesc = OS_open(SC_BOOT_RTS_3, OS_READ_ONLY, 0);
    if (FileDesc >= 0)
    {
        OS_close(FileDesc);
        CFE_TBL_Load(SC_OperData.RtsTblHandle[2], CFE_TBL_SRC_FILE, SC_BOOT_RTS_3);
        /* printf("SC: Loaded RTS 3 from boot drive\n"); */
        RtsCount++;
    } 

    /*
    ** Load the rest of the RTSs from RAM. 
    ** These files were unpacked from the boot drive at startup. 
    */
    for (RtsIndex = 3; RtsIndex < SC_NUMBER_OF_RTS; RtsIndex++)
    {
        /* Example filename: /cf/apps/sc_rts001.tbl */
        sprintf(TableName, "%s%03d.tbl", SC_RTS_FILE_NAME, (int)(RtsIndex + 1));
        FileDesc = OS_open(TableName, OS_READ_ONLY, 0);
                   
        if (FileDesc >= 0)
        {
            OS_close(FileDesc);
            
            /* Only try to load table files that can be opened, ignore others */
            CFE_TBL_Load(SC_OperData.RtsTblHandle[RtsIndex], CFE_TBL_SRC_FILE, TableName);
            RtsCount++;
        }
    }

    /* Display startup RTS load count */
    CFE_EVS_SendEvent(SC_RTS_LOAD_COUNT_INFO_EID, CFE_EVS_INFORMATION,
       "RTS table file load count = %ld", RtsCount);

    return;    

} /* end SC_LoadDefaultTables() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Register to receive cFE table manage request commands           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SC_RegisterManageCmds(void)
{
    int32 i;

    /* Register for RTS info table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.RtsInfoHandle,
        SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_RTS_INFO);

    /* Register for RTS control block table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.RtsCtrlBlckHandle,
        SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_RTP_CTRL);

    /* Register for ATS info table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.AtsInfoHandle,
        SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_ATS_INFO);

    /* Register for ATS Append info table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.AppendInfoHandle,
        SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_APP_INFO);

    /* Register for ATS control block table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.AtsCtrlBlckHandle,
        SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_ATP_CTRL);

    /* Register for ATS Append table manage request commands */
    CFE_TBL_NotifyByMessage(SC_OperData.AppendTblHandle,
        SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_APPEND);

    for (i  = 0; i < SC_NUMBER_OF_ATS; i++)
    {
        /* Register for ATS cmd status table manage request commands */
        CFE_TBL_NotifyByMessage(SC_OperData.AtsCmdStatusHandle[i],
            SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_ATS_CMD_0 + i);

        /* Register for ATS table manage request commands */
        CFE_TBL_NotifyByMessage(SC_OperData.AtsTblHandle[i],
            SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_ATS_0 + i);
    }

    for (i  = 0; i < SC_NUMBER_OF_RTS; i++)
    {
        /* Register for RTS table manage request commands */
        CFE_TBL_NotifyByMessage(SC_OperData.RtsTblHandle[i],
            SC_CMD_MID, SC_MANAGE_TABLE_CC, SC_TBL_ID_RTS_0 + i);
    }

    return;    

} /* End SC_RegisterManageCmds() */    


/************************/
/*  End of File Comment */
/************************/
