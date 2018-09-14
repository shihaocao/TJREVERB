
// For talking to ClydeSpace EPS and batteries over I2C.

#include <stdio.h>
#include <dev/i2c.h>
#include <util/error.h>

#include "FreeRTOS.h"
#include "task.h"

// TODO: consider scope for all methods

/*
 * Commands may have 1 or 2 data parameters. (usually 1, but new battery telem is 2... TODO: TEST)
 * Commands may return either 0 bytes, 2 bytes (a single value for the main board),
 * or 4 bytes (one value for the main board, one for the daughter board).
 * Some telemetry points (EPS BCRs) need longer delays, thus the slow arg.
 * Slow EPS functions: All BCR telemetry, get / set PDM initial state, get system resets, get soft resets.
 *   Reset node takes about 1 second.
 */
int CLY_SendCommand(uint8_t dest_addr, uint8_t cmd, uint16_t cmd_data, uint8_t cmd_data_count, uint8_t value_count, uint16_t *value, uint16_t *value_db, uint8_t slow) 
{
	uint8_t MSB_read;
	uint8_t LSB_read;
	uint8_t tx_data[1 + cmd_data_count];
	uint8_t rx_data[value_count];
	int retval;

	tx_data[0] = cmd;
	if (1 == cmd_data_count) {
		tx_data[1] = cmd_data;
	} else if (2 == cmd_data_count) {
		// Byte order is reversed.
		tx_data[1] = cmd_data >> 8;
		tx_data[2] = cmd_data;
	}

	uint16_t delay = 3; // default for fast transactions.
	if (slow) {
		// 18 ms was found to be too slow
		delay = 20;
	}

	retval = i2c_master_trans_delay(0, dest_addr, tx_data, 1 + cmd_data_count, rx_data, value_count, 1000, delay);
	if (retval != E_NO_ERR) {
		printf("CLY: i2c_master_trans_delay FAILED: %d\n", retval);
		return retval; // send failed
	}

	if (slow) {
		// This delay could be implemented outside of this call: the
		// pause is needed between sequential telemetry requests.
		// 26 ms was tested and found to be too slow
		vTaskDelay(30);
	} // No delay needed for non-slow telemetry requests.

	// Check number of bytes sent back
	if (0 == value_count) {
		// No data expected.
	} else if (2 <= value_count) {
		MSB_read = rx_data[0];
		LSB_read = rx_data[1];
		// Compose into 16 bits
		*value = (MSB_read << 8) | LSB_read;

		if (4 == value_count) {
			// First is MSB, first 6 bits are ignored
			MSB_read = rx_data[2];
			LSB_read = rx_data[3];

			// Compose into 16 bits
			*value_db = (MSB_read << 8) | LSB_read;
		}
	}

	return E_NO_ERR;
}
