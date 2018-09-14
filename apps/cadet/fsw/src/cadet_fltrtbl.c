/*
** File: cadet_fltrtbl.c
**
** Purpose:
**     This file contains the source code for the filter table routines, initialization routine, and 
**     interface functions.
**
*/



/*
**   Includes:
*/
#include "cfe.h"
#include "cadet_app.h"
#include "cadet_hk.h"
#include "cadet_cds.h"
#include "cadet_fltrtbl.h"
#include "cadet_events.h"
#include "mission_msgids.h"



/*
** Defines
*/
#define SB_QOS_LOW_PRIORITY     0
#define SB_QOS_LOW_RELIABILITY  0

#define DELLINGR_TBL_ERR_VALIDATION  -600        /* Value doesn't matter */



/*
** Structures
*/



/*
** Exported Variables
*/
CADET_FilterTable_t      *CADET_FilterTablePtr;
CFE_TBL_Handle_t         CADET_FilterTableHandle;
CADET_FilterStatTable_t  *CADET_FilterStatTablePtr;
CFE_TBL_Handle_t         CADET_FilterStatTableHandle;
char                     CADET_InitialTblFilename[OS_MAX_PATH_LEN]; /* Filename for first table to load */



/*
** Local Variables (could be defined static)
*/



/*
** Exported Functions
*/



/*
** Local Functions (could be defined static)
*/
void    CADET_InitPacketStats ( void );
int32   CADET_GetFilterTableData  ( void );
uint16  CADET_SubscribeToMessageIDs ( void );
int32   CADET_ValidateFilterData ( void *TableData );



/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/



/*
** Function: CADET_SubscribeToPacket
**
** Purpose: Subscribe to the messages that need to be received.
**     NOTE: For messages that are simply dropped, they will not received from software bus.
**
*/
boolean CADET_SubscribeToPacket ( uint16  AppID, uint16 HifilterN, uint16 HifilterX, uint16 LofilterN, uint16 LofilterX )               
{
    boolean  RetStatus    = FALSE;
    uint16   MessageLimit = CADET_DEFAULT_MSG_SUBSCRIBE_BUFFERS;
    int32    Result       = CFE_SUCCESS;
    CFE_SB_PipeId_t       TlmPipe = CADET_INVALID_PIPE;
    CFE_SB_Qos_t  SBMessageConfig;
    

    /*
    ** For the file downlink message IDs, use the file downlink pipe.  
    ** These three will always be the FDL pipe, never the regular telemetry pipe.
    */
    if ( ( AppID == CCSDS_SID_APID(FDL_APP_STARTFILE_MID) ) ||
         ( AppID == CCSDS_SID_APID(FDL_APP_SEGMENT_MID) ) ||
         ( AppID == CCSDS_SID_APID(FDL_APP_COMPLETEFILE_MID) ) )
    {
        TlmPipe = CADET_FdlPipe;
    }
    else
    {
        TlmPipe = CADET_TlmPipe;
    }

    /*
    **    Set defaults for message subscriptions
    */
    SBMessageConfig.Priority    = SB_QOS_LOW_PRIORITY;
    SBMessageConfig.Reliability = SB_QOS_LOW_RELIABILITY;

    if ( ( ( HifilterN == 0 ) && ( LofilterN == 0 ) ) || ( ( HifilterX == 0 ) && ( LofilterX == 0 ) ) ) 
    {
        /* Unsubscibe if we are currently subscribed */
        if ( CADET_FilterStatTablePtr->SubscribeAppList [ AppID ] == TRUE )
        {
            Result =  CFE_SB_Unsubscribe ( DELLINGR_MIN_TLM_MID + AppID, TlmPipe );
            if ( Result == CFE_SUCCESS )
            {
                CADET_HkTelemetryPkt.NumberOfPacketSubscriptions--;
                RetStatus = TRUE;
            }
        }
        else
        {
            /* Nothing wrong, but nothing to do */
            RetStatus = TRUE;
        }
        CADET_FilterStatTablePtr->SubscribeAppList [ AppID ] = FALSE;

    }
    else if ( CADET_FilterStatTablePtr->SubscribeAppList [ AppID ] != TRUE )
    {
        Result = CFE_SB_SubscribeEx ( DELLINGR_MIN_TLM_MID + AppID, TlmPipe, SBMessageConfig, MessageLimit );
        if ( Result == CFE_SUCCESS )
        {
            RetStatus = TRUE;

        } /*  else failed as indicated by RetStatus */

        CADET_FilterStatTablePtr->SubscribeAppList  [ AppID ] = TRUE;
        CADET_HkTelemetryPkt.NumberOfPacketSubscriptions++;

    }
    else /* Already subscribed but check if pipe has changed */
    {
        /* Pipe has not changed so just set RetStatus */
        RetStatus = TRUE;
    }

    return ( RetStatus );
}



/*
** Function: CADET_SubscribeToMessageIDs
**
** Purpose: Subscribe to the messages that need to be received.
**     NOTE: For messages that are simply dropped, they will not received from software bus.
**
*/
uint16  CADET_SubscribeToMessageIDs ( void )               
{
    uint16            NumberOfSubscriptions = 0;
    CFE_SB_MsgId_t    AppID = 0;

    for ( AppID = 0;  AppID < CADET_FILTER_TABLE_SIZE ; AppID++ )
    {
        CADET_SubscribeToPacket ( AppID, CADET_FilterTablePtr->AppID[AppID].Fifo[FILTER_HI_FIFO].N, CADET_FilterTablePtr->AppID[AppID].Fifo[FILTER_HI_FIFO].X, 
                                         CADET_FilterTablePtr->AppID[AppID].Fifo[FILTER_LO_FIFO].N, CADET_FilterTablePtr->AppID[AppID].Fifo[FILTER_LO_FIFO].X  );
    }
    
    return ( NumberOfSubscriptions );

}



/*
** Function: CADET_AppTableInit
**
** Purpose: Register the filter table and load (if not recovered from CDS) and register the statistics table.
**
*/
int32   CADET_AppTableInit ( void )
{
    int32 Result = CFE_SUCCESS;

    /*
    ** Initialize CADET table access variables...
    */
    CADET_FilterTablePtr    = (CADET_FilterTable_t *) NULL;                             
    CADET_FilterTableHandle = 0;                                                    
    CADET_FilterStatTablePtr    = (CADET_FilterStatTable_t *) NULL;                             
    CADET_FilterStatTableHandle = 0;                                                    

    /*
    ** Register table...
    */
    Result = CFE_TBL_Register ( &CADET_FilterTableHandle,  CADET_FILTER_TABLE_NAME, sizeof(CADET_FilterTable_t),                                                  
                                (CFE_TBL_OPT_DBL_BUFFER | CFE_TBL_OPT_LOAD_DUMP | CFE_TBL_OPT_CRITICAL), CADET_ValidateFilterData );  

    /*
    ** Load default schedule table data, if we didn't restore a critical table
    */
    if ( Result == CFE_SUCCESS )                                                             
    {
        CFE_TBL_Load ( CADET_FilterTableHandle, CFE_TBL_SRC_FILE, (const void *) &CADET_InitialTblFilename );                 
    }                                                                                      
 
    if ( ( Result == CFE_SUCCESS ) || ( Result == CFE_TBL_INFO_RECOVERED_TBL ) )                                                             
    {                                                                                      
        Result = CFE_TBL_Register ( &CADET_FilterStatTableHandle, CADET_FILTER_STATS_TABLE_NAME,                                     
                                    sizeof(CADET_FilterStatTable_t), (CFE_TBL_OPT_SNGL_BUFFER | CFE_TBL_OPT_DUMP_ONLY), NULL );
    }  
                                                                                    
    /*
    ** Get pointers to table data...
    */
    if ( Result == CFE_SUCCESS )
    {
        CADET_InitPacketStats ( );
        Result = CADET_GetFilterTableData ( );
        if ( Result == CFE_TBL_ERR_NEVER_LOADED )
        {
            CADET_SubscribeToMessageIDs ( );
            Result = CFE_SUCCESS;
        }
    }

    return ( Result );

}



/*
** Function: CADET_InitPacketStats
**
** Purpose: Initialize the statistics table.
**
*/
void    CADET_InitPacketStats ( void )
{
    CFE_SB_MsgId_t    AppID = 0;
    int32             Result;

    CFE_TBL_ReleaseAddress ( CADET_FilterStatTableHandle );

    /*
    ** Let cFE manage the tables...
    */
    CFE_TBL_Manage ( CADET_FilterStatTableHandle );
 
    /*
    ** Get a pointer to the filter table...
    */
    Result = CFE_TBL_GetAddress ( (void *) &CADET_FilterStatTablePtr, CADET_FilterStatTableHandle );
    if ( Result == CFE_SUCCESS )
    {
        for ( AppID = 0;  AppID < CADET_FILTER_TABLE_SIZE ; AppID++ )
        {
            CADET_FilterStatTablePtr->SubscribeAppList  [ AppID ] = FALSE; 
            CADET_FilterStatTablePtr->PacketRecvCnt     [ AppID ] = 0; 
        }
    }
    
}



/*
** Function: CADET_ValidateFilterData
**
** Purpose: Validate a new filter table (prior to be activated).
**     NOTE: CFE table services basically requires a validation routine.
**
*/
int32   CADET_ValidateFilterData ( void *TableData )
{
    CADET_FilterTable_t  *TableArray = (CADET_FilterTable_t *) TableData;
    int32 EntryResult = CFE_SUCCESS;
    int32 TableResult = CFE_SUCCESS;
    int32 TableIndex;

    int32 GoodCount   = 0;
    int32 BadCount    = 0;
    
    /*
    ** Verify each entry in pending table...
    */
    for ( TableIndex = 0; TableIndex < CADET_FILTER_TABLE_SIZE; TableIndex++ )
    {
        /*
        ** Check Filter Method 
        */
        if (  ( TableArray->AppID[TableIndex].Fifo[FILTER_HI_FIFO].FilterMethod != CADET_TIME_BASED_FILTER_TYPE ) && 
              ( TableArray->AppID[TableIndex].Fifo[FILTER_HI_FIFO].FilterMethod != CADET_SEQUENCE_BASED_FILTER_TYPE ) )
        {
            EntryResult = DELLINGR_TBL_ERR_VALIDATION; 
            BadCount++;
        }
        else if (  ( TableArray->AppID[TableIndex].Fifo[FILTER_LO_FIFO].FilterMethod != CADET_TIME_BASED_FILTER_TYPE ) && 
                   ( TableArray->AppID[TableIndex].Fifo[FILTER_LO_FIFO].FilterMethod != CADET_SEQUENCE_BASED_FILTER_TYPE ) )
        {
            EntryResult = DELLINGR_TBL_ERR_VALIDATION; 
            BadCount++;
        }
        else
        {
            GoodCount++;
        }

        /*
        ** Check filter params 
        */
        if (  ( TableArray->AppID[TableIndex].Fifo[FILTER_HI_FIFO].N  > TableArray->AppID[TableIndex].Fifo[FILTER_HI_FIFO].X ) || 
              ( TableArray->AppID[TableIndex].Fifo[FILTER_HI_FIFO].O  > TableArray->AppID[TableIndex].Fifo[FILTER_HI_FIFO].X ) )
        {
            EntryResult = DELLINGR_TBL_ERR_VALIDATION;
            BadCount++;
        }
        else if (  ( TableArray->AppID[TableIndex].Fifo[FILTER_LO_FIFO].N  > TableArray->AppID[TableIndex].Fifo[FILTER_LO_FIFO].X ) || 
                   ( TableArray->AppID[TableIndex].Fifo[FILTER_LO_FIFO].O  > TableArray->AppID[TableIndex].Fifo[FILTER_LO_FIFO].X ) )
        {
            EntryResult = DELLINGR_TBL_ERR_VALIDATION;
            BadCount++;
        }
        else
        {
            GoodCount++;
        }

        /*
        ** Send event for "first" error found...
        */
        if ( ( EntryResult == DELLINGR_TBL_ERR_VALIDATION ) && ( TableResult == CFE_SUCCESS ) )
        {
            TableResult = DELLINGR_TBL_ERR_VALIDATION;

            CFE_EVS_SendEvent ( CADET_FILTER_TBL_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Filter Table verify error -- AppID [x%04x] HI Method [%d] N [%d] X [%d] O [%d] LO Method [%d] N [%d] X [%d] O [%d] ",
                                __LINE__, TableIndex,
                                TableArray->AppID[TableIndex].Fifo[FILTER_HI_FIFO].FilterMethod,
                                TableArray->AppID[TableIndex].Fifo[FILTER_HI_FIFO].N, TableArray->AppID[TableIndex].Fifo[FILTER_HI_FIFO].X, TableArray->AppID[TableIndex].Fifo[FILTER_HI_FIFO].O,
                                TableArray->AppID[TableIndex].Fifo[FILTER_LO_FIFO].FilterMethod,
                                TableArray->AppID[TableIndex].Fifo[FILTER_LO_FIFO].N, TableArray->AppID[TableIndex].Fifo[FILTER_LO_FIFO].X, TableArray->AppID[TableIndex].Fifo[FILTER_LO_FIFO].O  );
        }
    }/* end For */

    /*
    ** Send event describing results...
    */
    CFE_EVS_SendEvent ( CADET_FILTER_TABLE_DBG_EID, CFE_EVS_DEBUG, "L%d CADET: Filter table verify results -- good[%d] bad[%d] ", __LINE__, GoodCount, BadCount );

    /*
    ** Maintain table verification statistics...
    */
    if ( TableResult == CFE_SUCCESS )
    {
        CADET_HkTelemetryPkt.TableVerifySuccessCount++;
    }
    else
    {
        CADET_HkTelemetryPkt.TableVerifyFailureCount++;
    }

    return ( TableResult );

}



/*
** Function: CADET_GetFilterTableData
**
** Purpose: Manage the tables with CFE table services.
**     NOTE: THis must be done periodically, most applications do it on the housekeeping cycle.
**
*/
int32   CADET_GetFilterTableData  ( void )
{
    int32  Result = CFE_SUCCESS;
 
    /*
    ** It is not necessary to release dump only table pointers before
    **  calling cFE Table Services to manage the table
    */
    CFE_TBL_ReleaseAddress ( CADET_FilterTableHandle );

    /*
    ** Let cFE manage the tables...
    */
    CFE_TBL_Manage ( CADET_FilterTableHandle );
    CFE_TBL_Manage ( CADET_FilterStatTableHandle );
 
    /*
    ** Get a pointer to the filter table table, never released address of dump only stat table.
    */
    Result = CFE_TBL_GetAddress ( (void *) &CADET_FilterTablePtr, CADET_FilterTableHandle );

    /*
    ** If table has changed, get the bit rate value, calculate the interval again and re-subscribe packets
    */    
    if ( Result == CFE_TBL_INFO_UPDATED )
    {
        CADET_SubscribeToMessageIDs ( );
        Result = CFE_SUCCESS;
    }
    /* else no new table data so do nothing */
 
    if ( Result == CFE_SUCCESS )
    {
        /*
        **  Save the new table load to the CDS 
        */
        CADET_CDS_UpdateCriticalDataStore ( ); 
    }
    /*  else, do nothing */

    return ( Result );

}



/************************/
/*  End of File Comment */
/************************/
