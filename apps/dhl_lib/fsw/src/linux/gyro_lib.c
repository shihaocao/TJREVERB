/*************************************************************************
** File:
**   $Id: gyro_lib.c  $
**
** Purpose: 
**   Shared library functions for the Gyro used on Dellngr
**   This is the nanomind version
**
**   $Log: gyro_lib.c  $
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
#define GYRO_DEBUG

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
/* Gyro library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 gy_mutex_id;
int32 GYRO_LibInit(void)
{
	int32 retval;

	retval = OS_MutSemCreate(&gy_mutex_id, "GY_Mutex", 0);

	return retval;

}/* End GYRO_LibInit */

/*
** Global Gyro Tach data
*/
float gy_1 = 0.0;
float gy_2 = 0.0;
float gy_3 = 0.0;

void GYS_SetTach(float x_value, float y_value, float z_value)
{
	OS_MutSemTake(gy_mutex_id);
	gy_1 = x_value;
	gy_2 = y_value;
	gy_3 = z_value;
	OS_MutSemGive(gy_mutex_id);
}

void GYS_GetTach(float *x_value, float *y_value, float *z_value)
{
	/*
	** If the Gyro interface is being simulated, return the sim data
	*/
	if ( SIM_Data.StateData.GyroState == 1 )
	{
		*x_value =  SIM_Data.GyroData.XR;
		*y_value =  SIM_Data.GyroData.YR;
		*z_value =  SIM_Data.GyroData.ZR;
	}
	else
	{
	OS_MutSemTake(gy_mutex_id);
	*x_value = gy_1;
	*y_value = gy_2;
	*z_value = gy_3;
	OS_MutSemGive(gy_mutex_id);
	}
}

int GYRO_LibGetXyz(float *X, float *Y, float *Z)
{

	if ( X == 0 || Y == 0 || Z == 0 )
	{
		return(-1);
	}

	/*
	** If the Gyro interface is being simulated, return the sim data
	*/
	if ( SIM_Data.StateData.GyroState == 1 )
	{
		*X =  SIM_Data.GyroData.XR;
		*Y =  SIM_Data.GyroData.YR;
		*Z =  SIM_Data.GyroData.ZR;
	}
	else
	{
		*X = 0.0;
		*Y = 0.0;
		*Z = 0.0;
	}

	return 0;
}

/*
** Get X, Y, Z and temperature data
*/
int GYRO_LibGetData(GYRO_data_t *GyroData)
{
	int     ReturnCode = 0;

	if ( GyroData == 0 )
	{
		return(-1);
	}

	GyroData->x_axis_gyro =  SIM_Data.GyroData.XR;
	GyroData->y_axis_gyro =  SIM_Data.GyroData.YR;
	GyroData->z_axis_gyro =  SIM_Data.GyroData.ZR;

	GyroData->x_axis_temperature = 10;
	GyroData->y_axis_temperature = 11;
	GyroData->z_axis_temperature = 12;

	return (ReturnCode);
}

/************************/
/*  End of File Comment */
/************************/
