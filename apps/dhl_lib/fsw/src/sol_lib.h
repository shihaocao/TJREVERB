/*************************************************************************
** File:
**   $Id: sol_lib.h  $
**
** Purpose: 
**   Specification for the Solar subsystem functions.
**
** References:
**
**   $Log: sol_lib.h  $
**  
*************************************************************************/
#ifndef _sol_lib_h_
#define _sol_lib_h_

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

/************************************************************************
** Macro Definitions
*************************************************************************/

#define FSS_NUM_DEVICES   3

#define FSS_DEVICE_0      0
#define FSS_DEVICE_1      1
#define FSS_DEVICE_2      2

//#define FSS_GFSS_DEVICE_0 0
#define FSS_GFSS_DEVICE_0 1
#define FSS_GFSS_DEVICE_1 2

/************************************************************************
** Type Definitions
*************************************************************************/
/*
** This is the struct for
** the wallops fine sun sensor
** It is not used for the generic call.
*/
typedef struct
{
  float   Azimuth;
  float   Elevation;
  float   Unit_Vector_X;
  float   Unit_Vector_Y;
  float   Unit_Vector_Z;
  uint8   FOV;
  uint8   InRange;
} WFSS_data_t;

/*
** This is the struct for the 
** Nanomind Fine Sun Sensor
** It is not used for the generic call
*/
typedef struct
{
  uint16  FSS_A_sen;
  uint16  FSS_B_sen;
  uint16  FSS_C_sen;
  uint16  FSS_D_sen;
  float   Azimuth;
  float   Elevation;
  float   Unit_Vector_X;
  float   Unit_Vector_Y;
  float   Unit_Vector_Z;
  uint8   FSS_Valid;
} GFSS_data_t  ;

/*************************************************************************
** Exported Functions
*************************************************************************/

/*
** Generic functions 
*/
int32 SOL_LibInit(void);

/*
** MT functions
*/
int32 SOL_LibCommandMT(float x, float y, float z);


int32 SOL_LibEnablePWM(int32 Channel, int32 Frequency, int32 DutyCycle, uint32 Direction);
void  SOL_LibDisablePWM(int Channel);


/*
** CSS Functions
*/
int   SOL_LibSetupFSS(int FssDevice);
int   SOL_LibReadFSS(int FssDevice, int *Valid, float *X, float *Y, float *Z);

/*
** Css function
** SOL_GetTemps returns the temperature data from the last call to SOL_LibReadCSS
*/
void  SOL_GetTemps(uint16 *temp_0, uint16 *temp_1);
int   SOL_LibReadCSS(uint16 *XPlus, uint16 *XMinus, uint16 *YPlus, uint16 *YMinus, uint16 *ZPlus, uint16 *ZMinus);

/*
** Wallops Fine sun sensor functions
*/
int      SOL_LibReadWFSS(WFSS_data_t *WFssData);

/*
** Gomspace/Nanomind Fine Sun Sensor functions
*/
int      SOL_LibSetupGFSS(int FssDevice);
int      SOL_LibReadGFSS(int FssDevice, GFSS_data_t *NFssData);
uint16   SOL_LibReadGFSSTemp(int FssDevice);
uint32   SOL_LibGetGFSSVersion(int FssDevice);
uint32   SOL_LibGetGFSSUUID(int FssDevice);

/*
** Command to trigger sampling of sunsensor. The data can be read after a delay of 30 ms with the command
** CMD_SUN_GET_DATA. If another command is called between sample sensor and the get sensor data command,
** the data can be destroyed.
*/
int SOL_LibGFSS_SampleSun(int FssDevice);

/*
** Gets the sampled data from the sun sensor. Wait at least 30 ms after the sample
** sensor command before calling this command.
*/
int SOL_LibGFSS_GetSunRaw(int FssDevice, uint8 nfssbytes[]);

/*
** Gets the sampled data from the sun sensor. Wait at least 30 ms after the sample
** sensor command before calling this command.
*/
int SOL_LibGFSS_GetSun(int FssDevice, GFSS_data_t *NFssData);

/*
** Calculate the vectors based on the data from the FSS
*/
int SOL_LibGFSS_Calculate(int FssDevice, GFSS_data_t *NFssData);


#endif /* _sol_lib_h_ */

/************************/
/*  End of File Comment */
/************************/
