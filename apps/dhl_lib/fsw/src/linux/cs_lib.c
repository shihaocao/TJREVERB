// For talking to ClydeSpace EPS and batteries over I2C.

#include <inttypes.h>

/*
 * Commands may return either 0 bytes, 2 bytes (a single value for the main board),
 * or 4 bytes (one value for the main board, one for the daughter board).
 */ 
int CLY_SendCommand(uint8_t dest_addr, uint8_t cmd, uint16_t cmd_data, uint8_t cmd_data_count, uint8_t value_count, uint16_t *value, uint16_t *value_db) 
{

	return 0;
}
