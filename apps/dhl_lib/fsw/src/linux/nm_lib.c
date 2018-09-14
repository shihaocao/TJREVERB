/*************************************************************************
** File:
**   $Id: nm_lib.c  $
**
** Purpose: 
**   Shared library functions for Misc Nanomind CPU functions used on Dellngr
**
**   $Log: nm_lib.c  $
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/


/*************************************************************************
** Private Function Prototypes
*************************************************************************/


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* NM Library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 NM_LibInit(void)
{
    return OS_SUCCESS;
 
}/* End NM_LibInit */


int NM_LibReadMagnetometer(float *X, float *Y, float *Z) 
{
        /*
        **  Return the data from the simulator
        */ 
        *X = SIM_Data.NmmdData.X;
        *Y = SIM_Data.NmmdData.Y;
        *Z = SIM_Data.NmmdData.Z;

        return(0);
}

/************************/
/*  End of File Comment */
/************************/
