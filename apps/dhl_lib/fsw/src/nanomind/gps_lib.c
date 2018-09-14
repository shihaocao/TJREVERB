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
#include "gps_lib.h"

#include "cfe.h"
#include "cfe_time_utils.h"


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include <dev/usart.h>
#include <dev/novatel_615.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/
#define GPS_DEBUG

#define GPS_UART  1

/*************************************************************************
** Global Data 
*************************************************************************/
extern uint32_t cpu_core_clk;

GPS_GpsData_t GPS_Data;

/*************************************************************************
** Private Function Prototypes
*************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* GPS UART Init                                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GPS_UartInit(void)
{
   /* 
   **  Setup the UART communication with the GPS 
   */

   /*
   ** First set the GPS UART to 9600 baud
   */
   usart_init(GPS_UART, cpu_core_clk, 9600);


#if 0
   /*
   ** Delay to allow the GPS to initialize
   */
   vTaskDelay(10000);

   /*
   ** Next, set the baudrte on the GPS to 115k
   */
   set_serial_baudrate(115200, 0, 8, 1);


   /*
   ** Finally, set the GPS UART baudrate to match the GPS
   */
   usart_init(1, cpu_core_clk, 115200);
#endif

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* GPS Library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 GPS_LibInit(void)
{
   /*
   ** Initialize the local data
   */
   GPS_Data.Latitude = 0;
   GPS_Data.Longitude = 0;
   GPS_Data.Altitude = 0;
   GPS_Data.XPosition = 0;
   GPS_Data.YPosition = 0;
   GPS_Data.ZPosition = 0;
   GPS_Data.XVelocity = 0;
   GPS_Data.YVelocity = 0;
   GPS_Data.ZVelocity = 0;
   GPS_Data.Week = 0;
   GPS_Data.Seconds = 0;
   GPS_Data.NumSats = 0;

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

   /*
   ** If the GPS interface is being simulated, return the sim data
   */ 
   if ( SIM_Data.StateData.GpsState == 1 ) 
   {
     *sec =  SIM_Data.GpsData.Seconds;
     *week = SIM_Data.GpsData.Week;
   }
   else
   {
      *sec =  GPS_Data.Seconds;
      *week = GPS_Data.Week;
   }
   return 0;
}

int GPS_LibGetXyz(float  *posx, float *posy, float *posz, float *velx, float *vely, float *velz)
{

   if ( posx == 0 || posy == 0 || posz == 0 || velx == 0 || vely == 0 || velz == 0)
   {
      return(-1);
   }

   /*
   ** If the GPS interface is being simulated, return the sim data
   */ 
   if ( SIM_Data.StateData.GpsState == 1 ) 
   {
      /*
      ** Return the data from the simulator
      */
      *posx = SIM_Data.GpsData.XPosition;
      *posy = SIM_Data.GpsData.YPosition;
      *posz = SIM_Data.GpsData.ZPosition;
      *velx = SIM_Data.GpsData.XVelocity;
      *vely = SIM_Data.GpsData.YVelocity;
      *velz = SIM_Data.GpsData.ZVelocity;
   }
   else
   {
      /*
      ** Return the data from the GPS  
      */
      *posx = GPS_Data.XPosition;
      *posy = GPS_Data.YPosition;
      *posz = GPS_Data.ZPosition;
      *velx = GPS_Data.XVelocity;
      *vely = GPS_Data.YVelocity;
      *velz = GPS_Data.ZVelocity;
   }
   return 0;
}

int GPS_LibGetPosition(float *lat, float *lgt, float* hgt)
{
   if ( lat == 0 || lgt == 0 || hgt == 0 )
   {
      return(-1);
   }

   /*
   ** If the GPS interface is being simulated, return the sim data
   */ 
   if ( SIM_Data.StateData.GpsState == 1 ) 
   {
      /*
      ** Return the data from the simulator
      */
      *lat = SIM_Data.GpsData.Latitude;
      *lgt = SIM_Data.GpsData.Longitude;
      *hgt = SIM_Data.GpsData.Altitude;
   }
   else
   {
      /*
      ** Return the data from the GPS  
      */
      *lat = GPS_Data.Latitude;
      *lgt = GPS_Data.Longitude;
      *hgt = GPS_Data.Altitude;
   }
   return 0;
}

int   GPS_LibGetNumSats(void)
{
   /*
   ** If the GPS interface is being simulated, return the sim data
   */ 
   if ( SIM_Data.StateData.GpsState == 1 ) 
   {
      /*
      ** Return the data from the simulator
      */
      return(SIM_Data.GpsData.NumSats);
   }
   else
   {
      /*
      ** Return the data from the GPS  
      */
      return(GPS_Data.NumSats);
   }
}

/*
** Read the latest data from the GPS
*/
int  GPS_SampleGPSData(void)
{
    int                  returncode;
    novatel_bestpos_t    bestpos;
    novatel_bestxyz_t    bestxyz;
    novatel_msg_header_t msgheader;

    uint32_t             start_time;
    uint32_t             end_time;
   
    CFE_TIME_SysTime_t   StcfTime;
    CFE_TIME_SysTime_t   MetTime;
    unsigned long        TaiSeconds;
    CFE_TIME_SysTime_t   GpsTime;

    int                  gps_status = 0;

    start_time = xTaskGetTickCount();

    /*
    ** Sample the data using the novatel GPS driver
    */
    #if 0
       OS_printf("----------- SEC:Child:Read GPS data---------------\n");
    #endif

    returncode = get_bestpos(&bestpos);
    if ( returncode == 1 )
    {
       /* 
       ** copy Position data to DHL GPS structures
       */
       GPS_Data.Latitude  = bestpos.lat;
       GPS_Data.Longitude = bestpos.lon;
       GPS_Data.Altitude  = bestpos.hgt;
       GPS_Data.NumSats   = bestpos.SVs; 

       gps_status = gps_status + GPS_BEST_POS;
    }
    else
      printf("------- GPSLIB: problem reading get_bestpos from GPS RC = %d -----\n",returncode);

    returncode = get_bestxyz(&bestxyz);
    if ( returncode == 1 )
    {
       /* 
       ** copy XYZ data to DHL GPS structures
       */
       GPS_Data.XPosition = bestxyz.p_x;
       GPS_Data.YPosition = bestxyz.p_y;
       GPS_Data.ZPosition = bestxyz.p_z;
       GPS_Data.XVelocity = bestxyz.v_x;
       GPS_Data.YVelocity = bestxyz.v_y;
       GPS_Data.ZVelocity = bestxyz.v_z;
       GPS_Data.NumSats   = bestxyz.SVs; 

       gps_status = gps_status + GPS_BEST_XYZ;
    }
    else
      printf("------- GPSLIB: problem reading get_bestxyz from GPS RC = %d -----\n",returncode);

    returncode = get_time_ws(&msgheader);
    if ( returncode == 1 )
    {
       /* 
       ** copy Time data to DHL GPS structures
       */
       GPS_Data.Week = msgheader.week;
       GPS_Data.Seconds = msgheader.ms / 1000;

       #if 0
          OS_printf("GPS Week = %d\n",GPS_Data.Week);
       #endif
       /*
       ** After receiving the GPS Week and Seconds,
       ** Compute the new STCF and store it in the cFE.
       ** This keeps cFE and GPS time syncronized
       */

       /*
       ** Figure out the TAI time based on the GPS time
       */
       TaiSeconds = (GPS_Data.Week * 604800) + GPS_Data.Seconds + 19;
       MetTime  = CFE_TIME_GetMET();
       #if 0 
          OS_printf("====> GPS (TAI) time = %d\n",TaiSeconds);
       #endif

       /*
       ** Calculate the new STCF ( seconds )
       **   STCFSeconds = TimeTAI - Virtual MET
       */
       StcfTime.Seconds = TaiSeconds - MetTime.Seconds;
       StcfTime.Subseconds = 0;

       /*
       ** Set the SCTF
       */
       CFE_TIME_SetSTCF(StcfTime);
       #if 0 
          OS_printf("====> GPS (TAI) time = %d\n",TaiSeconds);
          OS_printf("====> GPS Set new SCTF based on GPS\n");
       #endif

       gps_status = gps_status + GPS_TIME;

       /*
       ** Update the time and status 
       */
       GpsTime = CFE_TIME_GetTime();
       GPS_Data.LastGpsTime.Seconds = GpsTime.Seconds;       
       GPS_Data.LastGpsTime.Subseconds = GpsTime.Subseconds;
       GPS_Data.GpsStatus = gps_status;

    }
    #if 1
    else
      printf("------- GPSLIB: problem reading get_time_ws from GPS RC = %d -----\n",returncode);
    #endif

    end_time = xTaskGetTickCount() - start_time;
    #if 1
    //printf("--- Time to read GPS data time: %d milliseconds\n",(int)end_time);
    #endif
    
    return(gps_status);
}

/************************/
/*  End of File Comment */
/************************/
