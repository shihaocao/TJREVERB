/*************************************************************************
** File:
**   $Id: rw_lib.h  $
**
** Purpose:
**   Specification for the Reaction Wheel subsystem functions.
**
** References:
**
**   $Log: rw_lib.h  $
**
*************************************************************************/
#ifndef _rw_lib_h_
#define _rw_lib_h_

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

/************************************************************************
** Type Definitions
*************************************************************************/
#define RW_MODE_IDLE         0x00
#define RW_MODE_CURRENT      0x02
#define RW_MODE_TORQUE       0x16

#define RW_PARAM_CURRENT     0x02
#define RW_PARAM_TEMP        0x03
#define RW_PARAM_SPEED       0x05
#define RW_PARAM_TORQUE      0x1C
#define RW_PARAM_SEU_COUNT   0x29
#define RW_PARAM_SFFT_START  0x80
#define RW_PARAM_SFFT_END    0xE3

/*************************************************************************
** Exported Functions
*************************************************************************/

int32 RW_LibInit(void);

/*
** High level library interface for ACS to hardware and 42
*/

/*
** Command the reaction wheels
*/
int32 RW_LibCommandRW(float x, float y, float z);

/*
** Return the Reaction Wheel speeds
*/
int32 RW_LibGetSpeed(float *X, float *Y, float *Z);

/*
** Low level reaction wheel functions
*/

// turn on wheel
int rwlib_turnon(uint8 dest_index);
// turn off wheel
int rwlib_turnoff(uint8 dest_index);
uint8 rwlib_get_onoff_status(void);


// Ping
int rwlib_ping(uint8 dest_index);

// Low level interaction with reaction wheels
int rwlib_receiveModeTelem(uint8 dest_index, uint8 *mode_type, float *mode_value);
int rwlib_receiveParamTelem(uint8 dest_index, uint8 param_number, float *param_value);
int rwlib_sendModeCommand(uint8 dest_index, uint8 mode_number, float mode_value);
int rwlib_sendParamCommand(uint8 dest_index, uint8 param_number, float param_value);

// Shared data for the reaction wheels
void RWS_SetTach(float wheel_speed_1, float wheel_speed_2, float wheel_speed_3);
void RWS_GetTach(float *wheel_speed_1, float *wheel_speed_2, float *wheel_speed_3);

// Idle the wheels
void RWS_SetAllIdle(void);

/************************************************************************/

#endif /* _rw_lib_h_ */

/************************/
/*  End of File Comment */
/************************/

