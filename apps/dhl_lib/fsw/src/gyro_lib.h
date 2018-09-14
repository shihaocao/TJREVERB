/*************************************************************************
** File:
**   $Id: gyro_lib.h  $
**
** Purpose: 
**   Specification for the Gyro functions.
**
** References:
**
**   $Log: gyro_lib.h  $
**  
*************************************************************************/
#ifndef _gyro_lib_h_
#define _gyro_lib_h_

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

/************************************************************************
** Type Definitions
*************************************************************************/

typedef struct
{
   float  x_axis_gyro;
   float  y_axis_gyro;
   float  z_axis_gyro;
   uint16 x_axis_temperature;
   uint16 y_axis_temperature;
   uint16 z_axis_temperature;

} GYRO_data_t;

/*************************************************************************
** Data 
*************************************************************************/


/*************************************************************************
** Exported Functions
*************************************************************************/

int32 GYRO_LibInit(void);

int   GYRO_LibGetXyz(float *X, float *Y, float *Z);

int   GYRO_LibGetData(GYRO_data_t *GyroData);

// Shared data for the gyro
void GYS_SetTach(float x_value, float y_value, float z_value);
void GYS_GetTach(float *x_value, float *y_value, float *z_value);

#endif /* _gyro_lib_h_ */

/************************/
/*  End of File Comment */
/************************/
