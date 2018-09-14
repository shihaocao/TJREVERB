/*************************************************************************
** File:
**   $Id: gps_lib.h  $
**
** Purpose: 
**   Specification for the GPS functions.
**
** References:
**
**   $Log: gps_lib.h  $
**  
*************************************************************************/

#ifndef _gps_lib_h_
#define _gps_lib_h_

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

/************************************************************************
** Defines 
*************************************************************************/
#define GPS_BEST_POS 1
#define GPS_BEST_XYZ 2
#define GPS_TIME     4

/************************************************************************
** Type Definitions
*************************************************************************/
/*
** Local storage for values coming from the GPS
*/
typedef struct
{
   float               Latitude;
   float               Longitude;
   float               Altitude;
   float               XPosition;
   float               YPosition;
   float               ZPosition;
   float               XVelocity;
   float               YVelocity;
   float               ZVelocity;
   uint32              Week;
   double              Seconds;
   uint32              NumSats;
   uint32              GpsStatus;
   CFE_TIME_SysTime_t  LastGpsTime;

} GPS_GpsData_t;

extern GPS_GpsData_t GPS_Data;

/*************************************************************************
** Exported Functions
*************************************************************************/

/*
** Called when the Library Module is loaded
*/
int32 GPS_LibInit(void);

/*
** Initialize the GPS UART
*/
void GPS_UartInit(void);

/*
** The main API functions
*/
int   GPS_LibGetTime(unsigned int *week, double *sec);

int   GPS_LibGetXyz(float *px, float *py, float *pz, float *vx, float *vy, float *vz);

int   GPS_LibGetPosition(float *lat, float *lgt, float* hgt);

int   GPS_LibGetNumSats(void);

/*
** Update the Time and Position data from the GPS 
*/
int GPS_SampleGPSData(void);


#endif /* _gps_lib_h_ */

/************************/
/*  End of File Comment */
/************************/
