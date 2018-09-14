/*************************************************************************
** File:
**   $Id: sim_lib.h  $
**
** Purpose: 
**   Specification for the Simulator Functions.
**
** References:
**
**   $Log: sim_lib.h  $
**  
*************************************************************************/
#ifndef _sim_lib_h_
#define _sim_lib_h_

#include "cfe.h"

/*
** Defines
*/

#define DHL_SWITCH_CMD_STRING     "42START[SWITCH:GPS=%d,CSS=%d,FSS=%d,NMMD=%d,ACT=%d]42END\n"
#define DHL_FSS_SENSOR_STRING     "42START[FSSD:ID=%d,X=%f,Y=%f,Z=%f,Sun=%d]42END\n"
#define DHL_CSS_SENSOR_STRING     "42START[CSSD:CSS_XPLUS=%d,CSS_YPLUS=%d,CSS_ZPLUS=%d,CSS_XMINUS=%d,CSS_YMINUS=%d,CSS_ZMINUS=%d]42END\n"
#define DHL_NMMD_SENSOR_STRING    "42START[NMMD:X=%f,Y=%f,Z=%f]42END\n"
#define DHL_DAGR_SENSOR_STRING    "42START[DAGR:X=%f,Y=%f,Z=%f]42END\n"
#define DHL_GPS_POS_SENSOR_STRING "42START[GPSPOS:LAT=%f,LONG=%f,ALT=%f]42END\n"
#define DHL_GPS_XYZ_SENSOR_STRING "42START[GPSXYZ:POSX=%f,POSY=%f,POSZ=%f,VELX=%f,VELY=%f,VELZ=%f,WEEK=%d,SEC=%f]42END\n"

/* Need to incorporate the number of Sats into the GPS messages */
#define DHL_GYRO_SENSOR_STRING    "42START[GYROD:XR=%f,YR=%f,ZR=%f]42END\n"
#define DHL_RWSPD_SENSOR_STRING   "42START[RWSPD:X=%f,Y=%f,Z=%f]42END\n"
#define DHL_QBN_SENSOR_STRING     "42START[QBN: %f, %f, %f, %f]42END\n"

/*
** Actuator commands going from the FSW to 42
*/
#define DHL_RWTC_CMD_STRING "RWTC: X=%f, Y=%f, Z=%f\n"
#define DHL_MTC_CMD_STRING  "MTC: X=%f, Y=%f, Z=%f\n"

/*
** Data
*/
typedef struct 
{
   int GpsState;  /* GPS */
   int CssState;  /* Coarse sun sensors */
   int FssState;  /* Fine sun sensors */
   int NmmdState; /* Nanomind Magnetometers */ 
   int GyroState; /* Gyro Data */
   int ActState;  /* Actuators */

} SIM_StateData_t;

typedef struct
{
   float  Latitude;
   float  Longitude;
   float  Altitude;
   float  XPosition;
   float  YPosition;
   float  ZPosition;
   float  XVelocity;
   float  YVelocity;
   float  ZVelocity;
   uint32 Week;    
   double Seconds;  
   uint32 NumSats;

} SIM_GpsData_t ;

/*
** FSS data 
*/
typedef struct
{
   float X;
   float Y;
   float Z;
   uint8 Valid;
} SIM_FssData_t; 

/*
** CSS data
*/
typedef struct
{
   uint16  XPlus;
   uint16  XMinus;
   uint16  YPlus;
   uint16  YMinus;
   uint16  ZPlus;
   uint16  ZMinus;
} SIM_CssData_t;

typedef struct
{
   float X; 
   float Y;
   float Z;
} SIM_NmmdData_t;

typedef struct
{
   float X; 
   float Y;
   float Z;
} SIM_DagrData_t;

typedef struct 
{
   float XR;
   float YR;
   float ZR;
} SIM_GyroData_t;

typedef struct 
{
   float X;
   float Y;
   float Z;
} SIM_WheelData_t;

typedef struct
{
   float Q1;
   float Q2;
   float Q3;
   float Q4;
} SIM_QBNData_t;

typedef struct
{
   SIM_StateData_t StateData;
   
   SIM_GpsData_t   GpsData;
  
   SIM_FssData_t   FssData[3];

   SIM_CssData_t   CssData;
 
   SIM_NmmdData_t  NmmdData;

   SIM_DagrData_t  DagrData;

   SIM_GyroData_t  GyroData;

   SIM_WheelData_t WheelData;

   SIM_QBNData_t   QBNData;

} SIM_Data_t; 

extern SIM_Data_t  SIM_Data;

typedef void (*SIM_callback_t) (uint8 * buf, int len, void * pxTaskWoken);


/*
** Code 
*/
int32 SIM_LibInit(void);
void  SIM_InitData(void);
void  SIM_SendCommand(char *CommandString);
void  SIM_ProcessInput(char *InputString);

#endif /* _sim_lib_h_ */
