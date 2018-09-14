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
#include <dev/arm/at91sam7.h>
#include <dev/i2c.h>
#include <dev/usart.h>
#include <dev/magnetometer.h>
#include <util/error.h>

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
    uint8  cmd_type = 0;
    uint8  ret_cnt  = 0;
    uint16 cmd_data = 0;
    uint16 value = 0;
    uint16 value_db = 0;
    int retval = 0;

    // Get Bat Voltage
    cmd_type = BAT_COMMANDS[BAT_CMD_ADC_I][0];
    ret_cnt  = BAT_COMMANDS[BAT_CMD_ADC_I][1];
    cmd_data = BAT_TLE_CODE[cmd_index];
    if ( CLY_SendCommand(address, cmd_type, cmd_data, 2, ret_cnt, &value, &value_db, FAST_TELEM) != E_NO_ERR) {
        // TODO ERROR
    } else {
        return value;
    }
    return 0;
}

void EPS_reset_bus(uint8 reset_mask) {
    uint16 cmd_type;
    uint8  ret_cnt;
    int    retval;

    cmd_type = EPS_COMMANDS[EPS_CMD_SET_PCM_RESET_I][0];
    ret_cnt  = EPS_COMMANDS[EPS_CMD_SET_PCM_RESET_I][1];
    retval = CLY_SendCommand(EPS_ADDRESS, cmd_type, reset_mask, 1, ret_cnt, NULL, NULL, FAST_TELEM);

    if ( retval < 0 )
    {
         printf("Read error for EPS_reset_bus\n");
         // TODO: error
    }
}


/*
** Gather housekeeping for a battery pack.
** For each battery pack, reads the voltage, current and direction. Motherboard and each plate have a temperature (1+4 on the 40Whr packs).
*/
void EPS_read_battery_pack_hk(uint8 address, uint16 *voltage, uint16 *current, uint8 *direction, uint16 *volt_5V, uint16 *curr_5V, uint16 *volt_3V, uint16 *curr_3V, uint16 temp_array[], uint8 *all_heaters)
{
    uint8  cmd_type = 0;
    uint8  ret_cnt  = 0;
    uint16 cmd_data = 0;
    uint16 value = 0;
    uint16 value_db = 0;
    int retval = 0;
    uint16 cmd_index = 0;
    int i;

    // Get Bat Voltage
    *voltage = read_analog_bat(address, cmd_index);
    cmd_index++;

    // current
    *current = read_analog_bat(address, cmd_index);
    cmd_index++;

    // Current direction
    value = read_analog_bat(address, cmd_index);
    cmd_index++;
    if (value < 512) {
        *direction = 0; // Charging
    } else {
        *direction = 1; // Discharging
    }

    // Mother board temperature
    temp_array[0] = read_analog_bat(address, cmd_index);
    cmd_index++;

    // Get Bat 5V Current
    *curr_5V = read_analog_bat(address, cmd_index);
    cmd_index++;

    // Get Bat 5V Voltage
    *volt_5V = read_analog_bat(address, cmd_index);
    cmd_index++;

    // Get Bat 3.3V Current
    *curr_3V = read_analog_bat(address, cmd_index);
    cmd_index++;

    // Get Bat 3.3V Voltage
    *volt_3V = read_analog_bat(address, cmd_index);
    cmd_index++;

    uint8 heaters = 0;
    for (i = 0; i < 4; i++) {
        // Plate i temperature
        temp_array[i + 1] = read_analog_bat(address, cmd_index);
        cmd_index++;

        value = read_analog_bat(address, cmd_index);
        cmd_index++;

        if (value < 512) {
            // Heater off, 0 already stored
            heaters = heaters << 1;
        } else {
            heaters = heaters + 1;
            heaters = heaters << 1;
        }
    }
    *all_heaters = heaters;
}

/*
** Read the Battery Pack Status
*/
int EPS_read_battery_pack_status(uint8 address, uint16 *status)
{
    uint8 cmd_type = BAT_COMMANDS[BAT_CMD_STATUS_I][0];
    uint8 ret_cnt  = BAT_COMMANDS[BAT_CMD_STATUS_I][1];
    uint8 cmd_data = 0;
    uint16 value_db = 0;

    return CLY_SendCommand(address, cmd_type, cmd_data, 1, ret_cnt, status, &value_db, FAST_TELEM);
}

/*
** Read the EPS Status
** TODO: The docs have 16 bits for the daughter board status as well, but those are ignored here.
*/
int EPS_read_eps_status(uint16 *status)
{
    uint8   cmd_type = EPS_COMMANDS[EPS_CMD_GET_STATUS_I][0];
    uint8   ret_cnt  = EPS_COMMANDS[EPS_CMD_GET_STATUS_I][1];
    uint8   cmd_data = 0;
    uint16  value_db = 0;

    return CLY_SendCommand(EPS_ADDRESS, cmd_type, cmd_data, 1, ret_cnt, status, &value_db, FAST_TELEM);
}

void EPS_read_raw_eps(uint16 cmd_data, uint16 * data_output, uint8 slow)
{
    uint8  cmd_type;
    uint8  ret_cnt;

    cmd_type = EPS_COMMANDS[EPS_CMD_GET_ANALOG_TELEM_I][0];
    ret_cnt  = EPS_COMMANDS[EPS_CMD_GET_ANALOG_TELEM_I][1];
    if (CLY_SendCommand(EPS_ADDRESS, cmd_type, cmd_data, 1, ret_cnt, data_output, NULL, slow) < 0) {
        // TODO: error 
    }
}

/*
 * Turns an EPS switch on or off. Duplicated in nanomind/src/eps_test.c:_set_eps_switch
 * Check eps switch status to validate command worked (switches have current limits, will turn off if exceeded)
 *
 * state: 0 for off, 1 for on
 * switch_num: 1 - 10
 */
void EPS_set_switch(uint8 switch_num, boolean state) {
    uint8_t  cmd_type = 0;
    uint8_t  ret_cnt  = 0;
    uint16_t cmd_data = 0;
    uint16_t value = 0;
    uint16_t value_db = 0;

    if (state) {
        cmd_type = EPS_COMMANDS[EPS_CMD_SET_PDM_ON_I][0];
        ret_cnt  = EPS_COMMANDS[EPS_CMD_SET_PDM_ON_I][1];
    } else {
        cmd_type = EPS_COMMANDS[EPS_CMD_SET_PDM_OFF_I][0];
        ret_cnt  = EPS_COMMANDS[EPS_CMD_SET_PDM_OFF_I][1];
    }

    cmd_data = switch_num;
    value    = 0;
    value_db = 0;
    if ( CLY_SendCommand(EPS_ADDRESS, cmd_type, cmd_data, 1, ret_cnt, &value, &value_db, FAST_TELEM) < 0) {
        // TODO: ERROR
    }
}

boolean EPS_get_switch(uint8 switch_num) {
    uint8_t  cmd_type = 0;
    uint8_t  ret_cnt  = 0;
    uint16_t cmd_data = 0;
    uint16_t value = 0;
    uint16_t value_db = 0;
    int retval = 0;

    cmd_type = EPS_COMMANDS[EPS_CMD_GET_ACTUAL_PDM_STATUS_I][0];
    ret_cnt  = EPS_COMMANDS[EPS_CMD_GET_ACTUAL_PDM_STATUS_I][1];
    cmd_data = switch_num;
    value    = 0;
    value_db = 0;
    retval = CLY_SendCommand(EPS_ADDRESS, cmd_type, cmd_data, 1, ret_cnt, &value, &value_db, FAST_TELEM);
    if ( retval >= 0 ) {
        return (boolean)value;
    } else {
        return false;
    }
}

/************************/
/*  End of File Comment */
/************************/
