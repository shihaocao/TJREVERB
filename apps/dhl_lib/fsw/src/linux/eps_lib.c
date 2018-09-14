/*************************************************************************
** File:
**   $Id: eps_lib.c  $
**
** Purpose:
**   Shared library functions for the EPS used on Dellngr
**
**   $Log: eps_lib.c  $
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

/*************************************************************************
** Macro Definitions
*************************************************************************/
#define EPS_ADDRESS       0x2B

/*************************************************************************
** Data
*************************************************************************/

/*************************************************************************
** Private Function Prototypes
*************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EPS Library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 EPS_LibInit(void)
{
    return OS_SUCCESS;
}/* End EPS_LibInit */


uint16 read_analog_bat(uint8 address, uint16 cmd_index) {
	return;
}

void EPS_reset_bus(uint8_t reset_mask) {
    return;
}

/*
** Gather housekeeping for a battery pack.
*/
void EPS_read_battery_pack_hk(uint8 address, uint16 *voltage, uint16 *current, uint8 *direction, uint16 *volt_5V, uint16 *curr_5V, uint16 *volt_3V, uint16 *curr_3V, uint16 temp_array[], uint8 *all_heaters)
{
    uint8 cmd_type = 0;
    uint8 ret_cnt  = 0;
    uint8 cmd_data = 0;
    uint16 value = 0;
    uint16 value_db = 0;
    int retval = 0;
    float float_value = 0.0;

    /*
    ** Get Bat Voltage 0 
    */

    /*
    ** Get Bat Current 0 
    */

    /*
    ** Get Bat Temp 0 
    */

    /*
    ** Get Bat Voltage 1 
    */

    /*
    ** Get Bat Current 1 
    */

    /*
    ** Get Bat Temp 1 
    */

    /*
    ** Get Bat Voltage 2
    */

    /*
    ** Get Bat Current 2 
    */

    /*
    ** Get Bat Temp 2
    */
    return;
}

/*
** Read the Battery Pack Status
*/
int EPS_read_battery_pack_status(uint8 address, uint16 *status)
{
    uint16 value = 0;
    return value;
}

/*
** Read the EPS Status 
*/
int EPS_read_eps_status(uint16 *status)
{
   uint16  value = 0;
   return value;
}


/*
** Read the Raw EPS status
*/
void EPS_read_raw_eps(uint16 cmd_data, uint16 *data_output, uint8 slow)
{
   return;
}

/*
 * Turns an EPS switch on or off. Duplicated in nanomind/src/eps_test.c:_set_eps_switch
 * Check eps switch status to validate command worked (switches have current limits, will turn off if exceeded)
 *
 * state: 0 for off, 1 for on
 * switch_num: 1 - 10
 */
void EPS_set_switch(uint8 switch_num, boolean state) {
   return;
}

boolean EPS_get_switch(uint8 switch_num) {
    return true; // Everything is on! But doesn't exist.
}


/************************/
/*  End of File Comment */
/************************/
