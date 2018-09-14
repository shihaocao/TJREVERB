/*
** File: cadet_cds.c
**
** Purpose:
**     This file contains the source code for the CADET critical data storage routines, initialization routine, and 
**     processing functions.
**
*/



/*
**   Includes:
*/
#include "cadet_cds.h"
#include "cadet_fltrtbl.h"
#include "cadet_events.h"



/*
** Defines
*/



/*
** Structures
*/



/*
** Exported Variables
*/
CADET_CDSDataType_t      CADET_CritDS;          /* Critical Data Store interface area */
CFE_ES_CDSHandle_t       CADET_CDSMemoryHandle; /* Handle to CDS memory block */
boolean                  CADET_CDSMemoryValid;  /* Flag set to TRUE if CDS is usable (created) */



/*
** Local Variables (could be defined static)
*/



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
** Function: CADET_CDS_UpdateCriticalDataStore
**
** Purpose: Update the contents of the critical data store.
**
*/
void CADET_CDS_UpdateCriticalDataStore ( void )
{
    CFE_TBL_Info_t TblInfo;
    int32          RetStatus;
    
    if ( CADET_CDSMemoryValid == TRUE )
    {

        RetStatus = CFE_TBL_GetInfo ( &TblInfo, CADET_APP_FILTER_TABLE_NAME );
        
        if ( RetStatus ==  CFE_SUCCESS )
        {
            if ( TblInfo.LastFileLoaded[0] == '\0' ) /*  Get Info returned nothing - assign current working filename */
            {
                CFE_PSP_MemCpy ( &CADET_CritDS.LastTblFileLoaded, &CADET_InitialTblFilename, OS_MAX_PATH_LEN );
            }
            else  /*  Have a new filter table name - save this name */
            {
                CFE_PSP_MemCpy ( &CADET_CritDS.LastTblFileLoaded, &TblInfo.LastFileLoaded, OS_MAX_PATH_LEN );
            }
        }
        else  /*  Error getting filename, so replace with current working filename */
        {
             CFE_PSP_MemCpy ( &CADET_CritDS.LastTblFileLoaded, &CADET_InitialTblFilename, OS_MAX_PATH_LEN );
        }
    
        CFE_ES_CopyToCDS ( CADET_CDSMemoryHandle, &CADET_CritDS );
    }
    /*  else CDS not valid so do nothing */
}



/*
** Function: CADET_CDS_AccessCriticalDataStore
**
** Purpose: Initialize the CDS by registering and if already exists then retrieve information.
**
*/
boolean CADET_CDS_AccessCriticalDataStore ( void )
{
    int32    Status        = CFE_SUCCESS;
    boolean  SetToDefaults = FALSE;
    char     DefaultTblFilename[OS_MAX_PATH_LEN] =  CADET_DEF_FILTER_TABLE_FILE_NAME;
        
    /* Create the Critical Data Store */
    Status = CFE_ES_RegisterCDS ( &CADET_CDSMemoryHandle, sizeof(CADET_CDSDataType_t), CADET_CDS_NAME );

    if ( Status ==  CFE_ES_CDS_ALREADY_EXISTS )
    {
        CADET_CDSMemoryValid = TRUE;
        /*  
        **   Data is available, retrieve it
        */
        Status = CFE_ES_RestoreFromCDS ( &CADET_CritDS, CADET_CDSMemoryHandle );
        
        if ( Status ==  CFE_SUCCESS )
        {
            /* Retrieve table filename last used */
            CFE_PSP_MemCpy ( &CADET_InitialTblFilename, &CADET_CritDS.LastTblFileLoaded, OS_MAX_PATH_LEN );
        }
        else  /* Could not restore the data, just init to default values */
        {
            SetToDefaults = TRUE;
            CFE_EVS_SendEvent ( CADET_CRITICAL_DATA_STORE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Failed to Restore CDS Data, Defaults Assigned [Err=0x%08x]", __LINE__, Status );
        }
    }
    else if ( Status ==  CFE_SUCCESS )
    {
        /*
        **  No data available, init to default values
        */
        CADET_CDSMemoryValid = TRUE;
        SetToDefaults  = TRUE;
    }
    else  /* Error creating the Critical Data Storage, init to default values */
    {
        CADET_CDSMemoryValid = FALSE;
        SetToDefaults  = TRUE;
        CFE_EVS_SendEvent ( CADET_CRITICAL_DATA_STORE_ERR_EID, CFE_EVS_ERROR, "L%d CADET: Failed to create CDS [Err=0x%08x]", __LINE__, Status );
    }
    /*
    **    Assign the default values if the CDS is not available
    */   
    if ( SetToDefaults == TRUE  )
    {
        /* Assign default table filename  */
        CFE_PSP_MemCpy ( &CADET_InitialTblFilename, &DefaultTblFilename, OS_MAX_PATH_LEN );
    }
    if ( ( CADET_CDSMemoryValid == TRUE ) && ( SetToDefaults == TRUE ) )
    {
        /*
        **   Assign values to CDS data structure and update the CDS
        */
        CADET_CDS_UpdateCriticalDataStore ( );
    }
                                
    return ( !SetToDefaults );
    
}



/************************/
/*  End of File Comment */
/************************/




