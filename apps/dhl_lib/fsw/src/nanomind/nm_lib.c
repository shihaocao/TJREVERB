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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <dev/arm/at91sam7.h>
#include <dev/i2c.h>
#include <dev/usart.h>
#include <dev/magnetometer.h>
#include <util/error.h>

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
   int           status;

   status = mag_init();
   if (status != E_NO_ERR)
   {
      printf("NM_LibInit failed: %d\n", status);
      return(-1);
   }

   return OS_SUCCESS;
}/* End NM_LibInit */

/*
** Read data from the Nanomind Magnetometer
*/
int NM_LibReadMagnetometer(float *X, float *Y, float *Z) 
{
   int        retval;
   mag_data_t mag_data;

   /*
   ** Check parameters
   */
   if ( X == 0 || Y == 0 || Z == 0 )
   {
      printf("NM_LibReadMagnetometer passed null pointer.\n");
      return(-1);
   }

   /*
   ** Check to see if we are reading from the simulator or hardware
   */
   if ( SIM_Data.StateData.NmmdState != 1 )
   {

      /*
      ** Read data 
      */
      retval = mag_read_single(&mag_data);
      if (retval == E_NO_ERR)
      {
         *X = mag_data.x*100.0;
         *Y = mag_data.y*100.0;
         *Z = mag_data.z*100.0;
          return(0);
      }
      else
      {
         printf("NM_LibReadMagnetometer failed mag_read_single %d\n", retval);
         return(-1);
      }
   }
   else /* return data from the simulator */
   {
     /*
     **  Return the data from the simulator
     */
     *X = SIM_Data.NmmdData.X;
     *Y = SIM_Data.NmmdData.Y;
     *Z = SIM_Data.NmmdData.Z;

     return(0);

   }
}

/************************/
/*  End of File Comment */
/************************/
