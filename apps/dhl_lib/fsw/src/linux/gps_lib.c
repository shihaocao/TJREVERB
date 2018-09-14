/*************************************************************************
** File:
**   $Id: gps_lib.c  $
**
** Purpose: 
**   Shared library functions for the GPS used on Dellngr
**   This is the Linux/simulator version
**
**   $Log: gps_lib.c  $
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/
#define GPS_DEBUG

/*************************************************************************
** Global Data 
*************************************************************************/

GPS_GpsData_t GPS_Data;

/*************************************************************************
** Private Function Prototypes
*************************************************************************/

void GPS_UartInit(void)
{
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* GPS Library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 GPS_LibInit(void)
{
   return OS_SUCCESS;
 
}/* End GPS_LibInit */

/**************************************************************************
** API commands
***************************************************************************/

int GPS_LibGetTime(unsigned int *week, double *sec)
{
   if ( sec == 0 || week == 0 )
   {
      return(-1);
   }
   *sec = SIM_Data.GpsData.Seconds;
   *week = SIM_Data.GpsData.Week;

   return 0;
}

int GPS_LibGetXyz(float  *posx, float *posy, float *posz, float *velx, float *vely, float *velz)
{

   if ( posx == 0 || posy == 0 || posz == 0 || velx == 0 || vely == 0 || velz == 0)
   {
      return(-1);
   }

   /*
   **  Return the data from the simulator
   */
   *posx = SIM_Data.GpsData.XPosition;
   *posy = SIM_Data.GpsData.YPosition;
   *posz = SIM_Data.GpsData.ZPosition;
   *velx = SIM_Data.GpsData.XVelocity;
   *vely = SIM_Data.GpsData.YVelocity;
   *velz = SIM_Data.GpsData.ZVelocity;

   return 0;
}

int GPS_LibGetPosition(float *lat, float *lgt, float* hgt)
{
   if ( lat == 0 || lgt == 0 || hgt == 0 )
   {
      return(-1);
   }

   *lat = SIM_Data.GpsData.Latitude;
   *lgt = SIM_Data.GpsData.Longitude;
   *hgt = SIM_Data.GpsData.Altitude;

   return 0;
}

int   GPS_LibGetNumSats(void)
{
  return(SIM_Data.GpsData.NumSats);
}


/*
** Read the latest data from the GPS 
*/
int  GPS_SampleGPSData(void)
{
    /*
    ** Right now, this is not needed on Linux
    ** The GPS data comes from the 42 simulator socket connection
    ** It could be used to read data from a GPS device on a Raspberry Pi
    ** or other Linux system. 
    */ 

    #if 0
       OS_printf("-----------Read GPS data---------------\n");
    #endif
      
    return(0); 
} 


/************************/
/*  End of File Comment */
/************************/

