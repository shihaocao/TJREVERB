/*************************************************************************
** File:
**   $Id: eps_lib.h  $
**
** Purpose:
**   Specification for the EPS functions.
**
** References:
**
**   $Log: eps_lib.h  $
**
*************************************************************************/
#ifndef _eps_lib_h_
#define _eps_lib_h_

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

// For 3rd generation Clyde Space batteries and EPS
#define EPS_CMD_COUNT 20

#define EPS_SWITCH_COUNT 10

// Some telemetry requests need a delay.
#define FAST_TELEM        0
#define SLOW_TELEM        1

// EPS Commands
#define EPS_CMD_GET_STATUS_I				0
#define EPS_CMD_SET_PCM_RESET_I				1
#define EPS_CMD_GET_VERSION_I				2
#define EPS_CMD_SET_WATCHDOG_I				3
#define EPS_CMD_RESET_WATCHDOG_I			4
#define EPS_CMD_GET_SYSTEM_RESET_COUNT_I	5
#define EPS_CMD_PDM_INITIAL_STATE_OFF_I		6
#define EPS_CMD_PDM_INITIAL_STATE_ON_I		7
#define EPS_CMD_SET_ALL_PDM_ON_I			8
#define EPS_CMD_SET_ALL_PDM_OFF_I			9
#define EPS_CMD_GET_ACTUAL_PDM_STATUS_I		10
#define EPS_CMD_GET_INITIAL_PDM_STATE_I		11
#define EPS_CMD_GET_ANALOG_TELEM_I			12
#define EPS_CMD_GET_WATCHDOG_TIMEOUT_I		13
#define EPS_CMD_SET_PDM_ON_I				14
#define EPS_CMD_SET_PDM_OFF_I				15
#define EPS_CMD_GET_SOFT_RESET_COUNT_I		16
#define EPS_CMD_GET_EXPECTED_PDM_STATE_I	17
#define EPS_CMD_GET_TEMP_I					18
#define EPS_CMD_RESET_NODE_I				19

static const uint8 EPS_COMMANDS[EPS_CMD_COUNT][2] = {
	{0x01, 4},	// EPS_CMD_GET_STATUS_I
	{0x02, 0},	// EPS_CMD_SET_PCM_RESET_I
	{0x04, 4},	// EPS_CMD_GET_VERSION_I
	{0x06, 0},	// EPS_CMD_SET_WATCHDOG_I
	{0x07, 0},	// EPS_CMD_RESET_WATCHDOG_I
	{0x09, 4},	// EPS_CMD_GET_SYSTEM_RESET_COUNT_I
	{0x0A, 0},	// EPS_CMD_PDM_INITIAL_STATE_OFF_I
	{0x0B, 0},	// EPS_CMD_PDM_INITIAL_STATE_ON_I
	{0x0C, 0},	// EPS_CMD_SET_ALL_PDM_ON_I
	{0x0D, 0},	// EPS_CMD_SET_ALL_PDM_OFF_I
	{0x0E, 2},	// EPS_CMD_GET_ACTUAL_PDM_STATUS_I
	{0x0F, 2},	// EPS_CMD_GET_INITIAL_PDM_STATE_I
	{0x10, 2},	// EPS_CMD_GET_ANALOG_TELEM_I
	{0x11, 4},	// EPS_CMD_GET_WATCHDOG_TIMEOUT_I
	{0x12, 0},	// EPS_CMD_SET_PDM_ON_I
	{0x13, 0},	// EPS_CMD_SET_PDM_OFF_I
	{0x14, 4},	// EPS_CMD_GET_SOFT_RESET_COUNT_I
	{0x16, 2},	// EPS_CMD_GET_EXPECTED_PDM_STATE_I
	{0x17, 4},	// EPS_CMD_GET_TEMP_I
	{0x80, 0}};	// EPS_CMD_RESET_NODE_I


// Battery related
// TODO: Need to test (waiting on hardware)
// TODO: Error handling
#define BAT_CMD_COUNT 11

// Battery Commands
#define BAT_CMD_STATUS_I        0   // Request status bytes
#define BAT_CMD_LAST_ERROR_I    1
#define BAT_CMD_VERSION_I       2   // Request Firmware version
#define BAT_CMD_CHECKSUM_I      3   // Request Checksum of memory
#define BAT_CMD_ADC_I           4   // Read an ADC Channel (0-31)
#define BAT_CMD_BROWN_RESET_I   5   // Times board reset due to brown-out
#define BAT_CMD_AUTO_RESET_I    6   // Times board reset due ???
#define BAT_CMD_MANUAL_RESET_I  7   // Times board reset due manual command
#define BAT_CMD_HEATER_GETS_I   8   // Get heater controller status
#define BAT_CMD_HEATER_SETS_I   9   // Set heater controller status
#define BAT_CMD_WATCHDOG_I     10   // Soft reset

// {Command code, number of bytes returned}
static const uint8 BAT_COMMANDS[BAT_CMD_COUNT][2] = {
	{0x01, 2},  // BAT_CMD_STATUS_I
	{0x03, 2},  // BAT_CMD_LAST_ERROR_I
	{0x04, 2},  // BAT_CMD_VERSION_I
	{0x05, 2},  // BAT_CMD_CHECKSUM_I
	{0x10, 2},  // BAT_CMD_ADC_I
	{0x31, 2},  // BAT_CMD_BROWN_RESET_I
	{0x32, 2},  // BAT_CMD_AUTO_RESET_I
	{0x33, 2},  // BAT_CMD_MANUAL_RESET_I
	{0x90, 2},  // BAT_CMD_HEATER_GETS_I
	{0x91, 2},  // BAT_CMD_HEATER_SETS_I
	{0x80, 0}}; // BAT_CMD_WATCHDOG_I

static const uint16 BAT_TLE_CODE[16] = {
	0xE280, // Output Voltage
	0xE284, // Current Magnitude
	0xE28E, // Current Direction
	0xE308, // Motherboard Temperature
	0xE214, // Current draw of 5.0V bus
	0xE210, // Output voltage of 5.0V bus
	0xE204, // Current draw of 3.3V bus
	0xE200, // Output voltage of 3.3V bus
	0xE398,  // Plate 1 Temperature
	0xE39F,  // Plate 1 Heater Status
	0xE3A8,  // Plate 2 Temperature
	0xE3AF,  // Plate 2 Heater Status
	0xE3B8,  // Plate 3 Temperature
	0xE3BF,  // Plate 3 Heater Status
	0xE3C8,  // Plate 4 Temperature
	0xE3CF}; // Plate 4 Heater Status


#define EPS_ADDRESS       0x2B

// Updated based off of 8/10/15 power switching diagram.
#define EPS_SW01  1  // 12V  - DAGR
#define EPS_SW02  2  // 12V  - INMS
#define EPS_SW03  3  // BatV - FPSS #1
#define EPS_SW04  4  // BatV - Spare
#define EPS_SW05  5  // 5V   - DAGR
#define EPS_SW06  6  // 5V   - Reaction Wheels
#define EPS_SW07  7  // 5V   - INMS +/-5V (via charge pump)
#define EPS_SW08  8  // 3.3V - INMS
#define EPS_SW09  9  // 3.3V - Dual GomSpace FSS
#define EPS_SW10 10  // 3.3V - GPS

/************************************************************************
** Type Definitions
*************************************************************************/

/*************************************************************************
** Data
*************************************************************************/

/* Need to export or move these? */
static const uint8 bat_address[] = {0x2A, 0x2D};
static const uint8 bcr_address[] = {0, 7, 14, 64, 71, 78, 85, 92, 99};  // BCR 8 and 9 are not used

/*************************************************************************
** Exported Functions
*************************************************************************/
int CLY_SendCommand(uint8 dest_addr, uint8 cmd, uint16 cmd_data, uint8 cmd_data_count, uint8 value_count, uint16 *value, uint16 *value_db, uint8 slow);

int32   EPS_LibInit(void);

void    EPS_read_battery_pack_hk(uint8 address, uint16 *voltage, uint16 *current, uint8 *direction, uint16 *volt_5V, uint16 *curr_5V, uint16 *volt_3V, uint16 *curr_3V, uint16 temp_array[], uint8 *all_heaters);

int     EPS_read_battery_pack_status(uint8 address, uint16 *status);

int     EPS_read_eps_status(uint16 *status);

void    EPS_read_raw_eps(uint16 cmd_data, uint16 * data_output, uint8 slow);

// True means on, False means off
void EPS_set_switch(uint8 switch_num, boolean state);

boolean EPS_get_switch(uint8 switch_num);

void EPS_reset_bus(uint8 reset_mask);

#endif /* _eps_lib_h_ */

/************************/
/*  End of File Comment */
/************************/
