/*************************************************************************
** File:
**   $Id: att_lib.c  $
**
** Purpose: 
**   Shared library functions for Attitude functions on Dellngr
**
**   $Log: att_lib.c  $
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"
#include <stdio.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/

/*************************************************************************
** Global Data 
*************************************************************************/


/*************************************************************************
** Private Function Prototypes
*************************************************************************/


/**************************************************************************
** API commands
***************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Attitude library Initialization Routine                         */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 ATT_LibInit(void)
{
	int32 retval = 0;

	return retval;

}/* End ATT_LibInit */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Attitude library Quaternion estimate funciton                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int   ATT_LibGetEstimate(ATT_estimate_data_t *att_estimate)
{
   if ( att_estimate == 0 ) 
   {
      return(-1);
   }
   
   att_estimate->Q1 = SIM_Data.QBNData.Q1;
   att_estimate->Q2 = SIM_Data.QBNData.Q2;
   att_estimate->Q3 = SIM_Data.QBNData.Q3;
   att_estimate->Q4 = SIM_Data.QBNData.Q4;

   return(0);
}


/************************/
/*  End of File Comment */
/************************/
