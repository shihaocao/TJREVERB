/*
** File: acs_table.c
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
#include "acs_app.h"
#include "acs_table.h"
#include "acs_events.h"

/*
** Defines
*/
#define DELLINGR_TBL_ERR_VALIDATION  -600        /* Value doesn't matter */

/*
** Structures
*/


/*
** Exported Variables
*/
ACS_ConstantTable_t   *ACS_ConstantTablePtr;
CFE_TBL_Handle_t       ACS_ConstantTableHandle;

/*
** This is the data structure in the app where the constants are stored
** Should probably go in the acs_app.c with the rest of the global data
** An alternative to this would be to init/validate/read the ACS constants table, then populate the 
**  individual global variables that represent the constants in the app. 
*/
ACS_ConstantTable_t    ACS_Constants;
uint32                 ACS_TableVerifyCount;      /* should be in ACS Housekeeping */
uint32                 ACS_TableVerifyErrorCount; /* should be in ACS housekeeping */ 

/*
** Local Variables (could be defined static)
*/


/*
** Exported Functions
*/


/*
** Local Functions (could be defined static)
*/
int32   ACS_ValidateConstantData ( void *TableData );

/**********************************************************************************************************************/
/*
** Function: ACS_ConstantTableInit
**
** Purpose: Register the Constant table and load (if not recovered from CDS) 
**
*/
int32   ACS_ConstantTableInit ( void )
{
    int32 Result = CFE_SUCCESS;

    /*
    ** Initialize ACS table access variables
    */
    ACS_ConstantTablePtr    = (ACS_ConstantTable_t *) NULL;                             
    ACS_ConstantTableHandle = 0;                                                    

    /* 
    ** Register the Constants table 
    */
    Result = CFE_TBL_Register ( &ACS_ConstantTableHandle,  ACS_CONSTANT_TABLE_NAME, 
                                sizeof(ACS_ConstantTable_t), CFE_TBL_OPT_DEFAULT, ACS_ValidateConstantData);

    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ACS_CONST_TBL_REG_ERR_EID, CFE_EVS_ERROR,
            "Error Registering ACS Constants Table,RC=0x%08X",Result);
        return(Result);
    }

    /* 
    ** Load the ACS Constants Table 
    */
    Result = CFE_TBL_Load (ACS_ConstantTableHandle,
                           CFE_TBL_SRC_FILE,
                           (const void *) ACS_CONSTANT_TABLE_FILE_NAME);
    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ACS_CONST_TBL_LOAD_ERR_EID, CFE_EVS_ERROR,
            "Error Loading Constant Table,RC=0x%08X",Result);
        /* Return here?? What do do in Load error */
    }

    /*
    ** Get pointers to table data.
    */
    if ( Result == CFE_SUCCESS )
    {
        Result = ACS_UpdateConstantTableData();

        /* Why??  CDS related?? */
        if ( Result == CFE_TBL_ERR_NEVER_LOADED )
        {
            Result = CFE_SUCCESS;
        }
    }

    return ( Result );
}

/*
** Function: ACS_ValidateConstantData
**
** Purpose: Validate a new Constants table (prior to be activated).
**     NOTE: CFE table services requires a validation routine.
**
*/
int32   ACS_ValidateConstantData ( void *TableData )
{
    ACS_ConstantTable_t  *ConstantTableData = (ACS_ConstantTable_t *) TableData;
    int32 TableResult = CFE_SUCCESS;

    /*
    ** Verify each item in pending table
    ** With the constants, each item has to be checked, since it is not an array of data structures
    */
    /* Check constants here!  - use the ConstantTableData pointer to access elements of structure */
    printf("ACS Constant Table check: \n");
    printf("    rate_cmd_sun = %f\n",ConstantTableData->rate_cmd_sun);
    printf("    rate_cmd_sci = %f, %f, %f\n",ConstantTableData->rate_cmd_sci[0], ConstantTableData->rate_cmd_sci[1], ConstantTableData->rate_cmd_sci[2]);
    printf("    targ_cmd_sci = %f, %f, %f, %f\n",ConstantTableData->targ_cmd_sci[0], ConstantTableData->targ_cmd_sci[1], ConstantTableData->targ_cmd_sci[2], ConstantTableData->targ_cmd_sci[3]);
    printf("    gain_att_sci = %f, %f, %f\n",ConstantTableData->gain_att_sci[0], ConstantTableData->gain_att_sci[1], ConstantTableData->gain_att_sci[2]);
    printf("    gain_int_sci = %f, %f, %f\n",ConstantTableData->gain_int_sci[0], ConstantTableData->gain_int_sci[1], ConstantTableData->gain_int_sci[2]);
    printf("    gain_rate_sci = %f, %f, %f\n",ConstantTableData->gain_rate_sci[0], ConstantTableData->gain_rate_sci[1], ConstantTableData->gain_rate_sci[2]);
    printf("    gain_rate_sun = %f, %f, %f\n",ConstantTableData->gain_rate_sun[0], ConstantTableData->gain_rate_sun[1], ConstantTableData->gain_rate_sun[2]);
    printf("    gain_att_sun = %f, %f, %f\n",ConstantTableData->gain_att_sun[0], ConstantTableData->gain_att_sun[1], ConstantTableData->gain_att_sun[2]);

    if ( TableResult == CFE_SUCCESS )
    {
       CFE_EVS_SendEvent ( ACS_CONST_TBL_DBG_EID, CFE_EVS_DEBUG, "ACS: Constant Table Verified");
       /* TODO - This should be a counter in ACS housekeeping */
       ACS_TableVerifyCount++;
    }
    else
    {
       CFE_EVS_SendEvent ( ACS_CONST_TBL_ERR_EID, CFE_EVS_ERROR, "ACS: Constant Table verify error");
       /* TODO - This should be a counter in ACS housekeeping */
       ACS_TableVerifyErrorCount++;

    }
    return ( TableResult );
}

/*
** Function: ACS_UpdateConstantTableData
**
** Purpose: Manage the tables with CFE table services.
**     NOTE: THis must be done periodically, most applications do it on the housekeeping cycle.
**
*/
int32  ACS_UpdateConstantTableData (void)
{
    int32  Result = CFE_SUCCESS;
 
    /*
    ** It is not necessary to release dump only table pointers before
    **  calling cFE Table Services to manage the table
    */
    CFE_TBL_ReleaseAddress ( ACS_ConstantTableHandle );

    /*
    ** Let cFE manage the tables.
    */
    CFE_TBL_Manage ( ACS_ConstantTableHandle );
 
    /*
    ** Get a pointer to the Constants table
    */
    Result = CFE_TBL_GetAddress ( (void *) &ACS_ConstantTablePtr, ACS_ConstantTableHandle );

    /*
    ** If table has changed copy the latest constant data over
    */    
    if ( Result == CFE_TBL_INFO_UPDATED )
    {
        Result = CFE_SUCCESS;

        /*
        ** Copy the data from the table to the ACS constants
        ** global structure
        */
        /* Copy in one call */
        //CFE_PSP_MemCpy((void *)&(ACS_Constants), (void *)ACS_ConstantTablePtr, sizeof(ACS_ConstantTable_t));

        /* Or if the variables are scattered, copy individually ( but dont do both ) */
        ACS_Constants.rate_cmd_sun  = ACS_ConstantTablePtr->rate_cmd_sun; 
        ACS_Constants.rate_cmd_sci[0] = ACS_ConstantTablePtr->rate_cmd_sci[0]; 
        ACS_Constants.rate_cmd_sci[1] = ACS_ConstantTablePtr->rate_cmd_sci[1]; 
        ACS_Constants.rate_cmd_sci[2] = ACS_ConstantTablePtr->rate_cmd_sci[2]; 
        ACS_Constants.targ_cmd_sci[0] = ACS_ConstantTablePtr->targ_cmd_sci[0]; 
        ACS_Constants.targ_cmd_sci[1] = ACS_ConstantTablePtr->targ_cmd_sci[1]; 
        ACS_Constants.targ_cmd_sci[2] = ACS_ConstantTablePtr->targ_cmd_sci[2]; 
        ACS_Constants.targ_cmd_sci[3] = ACS_ConstantTablePtr->targ_cmd_sci[3]; 
        ACS_Constants.gain_att_sci[0] = ACS_ConstantTablePtr->gain_att_sci[0]; 
        ACS_Constants.gain_att_sci[1] = ACS_ConstantTablePtr->gain_att_sci[1]; 
        ACS_Constants.gain_att_sci[2] = ACS_ConstantTablePtr->gain_att_sci[2]; 
        ACS_Constants.gain_int_sci[0] = ACS_ConstantTablePtr->gain_int_sci[0]; 
        ACS_Constants.gain_int_sci[1] = ACS_ConstantTablePtr->gain_int_sci[1]; 
        ACS_Constants.gain_int_sci[2] = ACS_ConstantTablePtr->gain_int_sci[2]; 
        ACS_Constants.gain_rate_sci[0] = ACS_ConstantTablePtr->gain_rate_sci[0]; 
        ACS_Constants.gain_rate_sci[1] = ACS_ConstantTablePtr->gain_rate_sci[1]; 
        ACS_Constants.gain_rate_sci[2] = ACS_ConstantTablePtr->gain_rate_sci[2]; 
        ACS_Constants.gain_rate_sun[0] = ACS_ConstantTablePtr->gain_rate_sun[0]; 
        ACS_Constants.gain_rate_sun[1] = ACS_ConstantTablePtr->gain_rate_sun[1]; 
        ACS_Constants.gain_rate_sun[2] = ACS_ConstantTablePtr->gain_rate_sun[2]; 
        ACS_Constants.gain_att_sun[0] = ACS_ConstantTablePtr->gain_att_sun[0]; 
        ACS_Constants.gain_att_sun[1] = ACS_ConstantTablePtr->gain_att_sun[1]; 
        ACS_Constants.gain_att_sun[2] = ACS_ConstantTablePtr->gain_att_sun[2]; 

    }
    /*  else, do nothing */

    return ( Result );
}

/************************/
/*  End of File Comment */
/************************/
