/*************************************************************************
** File:
**   $Id: dellingr_lib.c  $
**
** Purpose: 
**   Shared library functions for Dellingr applications
**
**   $Log: dellingr_lib.c  $
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"
#include "dhl_lib_version.h"

#include <ctype.h>
#include <string.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/


/*************************************************************************
** Private Function Prototypes
*************************************************************************/


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DHL Library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 DHL_LibInit(void)
{

    int32 status;

    /*
    ** Init all hardware subsystems and interfaces
    ** order may be important. 
    */    
   
    status = L3C_LibInit();

    status = SSC_LibInit();
 
    status = GPS_LibInit();
 
    status = EPS_LibInit();

    status = SOL_LibInit();

    status = RW_LibInit();

    status = NM_LibInit();

    status = INMS_LibInit();

    status = DAGR_LibInit();

    status = CAM_LibInit();

    status = TLC_LibInit();

    status = SIM_LibInit();

    status = MODE_LibInit();

    status = GYRO_LibInit();
   
    status = SSC_MPU6000_Init();

    OS_printf ("DHL Lib Initialized.  Version %d.%d.%d.%d",
                DHL_LIB_MAJOR_VERSION,
                DHL_LIB_MINOR_VERSION, 
                DHL_LIB_REVISION, 
                DHL_LIB_MISSION_REV);
                
    return OS_SUCCESS;
 
}/* End DHL_LibInit */

/************************/
/*  End of File Comment */
/************************/
