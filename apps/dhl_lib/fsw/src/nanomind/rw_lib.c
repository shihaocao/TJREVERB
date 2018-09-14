/*************************************************************************
** File:
**   $Id: rw_lib.c  $
**
** Purpose:
**   Shared library functions for the Reaction Wheel Subsystem used on Dellngr
**
**   $Log: rw_lib.c  $
**
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"

#include <csp/csp.h>
#include <dev/i2c.h>
#include <util/error.h>
#include <util/log.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/

#define NSP_MODULE_PING 0x00
#define NSP_MODULE_INIT 0x01
#define NSP_MODULE_PEEK 0x02
#define NSP_MODULE_POKE 0x03
#define NSP_MODULE_TELEMETRY 0x04
// No 0x05?
#define NSP_MODULE_CRC 0x06
#define NSP_MODULE_APP_TELEMETRY 0x07
#define NSP_MODULE_APP_COMMAND 0x08

#define NSP_MCF_POLL_MASK 0x80

#define MAX_RW_MSG 16 // Max length of a command to a wheel after SLIP encoding.

// Should functions build messages each time? Seems like the safest
// Do I need to worry about mutual exclusion?
struct NSP_message {
	uint8  dest_addr;
	uint8  src_addr;
	uint8  mcf; // message control field
	uint8* data;
	uint8  data_len;
	uint16 crc;
};

// TODO: Consider scope for all methods

// Adds a byte to data array, encodes in SLIP if neccessary
// returns the new count.
// Special characters in SLIP (Serial Line Internet Protocol)
#define SLIP_FEND  0xC0 // Marks end of message
#define SLIP_FESC  0xDB
#define SLIP_TFEND 0xDC // Natural FENDs are escaped with FESC TFEND
#define SLIP_TFESC 0xDD // Natural FESCs are escaped with FESC TFESC

#define RW_ADDRESS_0              0x33
#define RW_ADDRESS_1              0x35
#define RW_ADDRESS_2              0x36

bool rw_turnon_status[3] = {false};
const uint8 rw_address_array[3] = {RW_ADDRESS_0, RW_ADDRESS_1, RW_ADDRESS_2};

/*************************************************************************
** Private Function Prototypes
*************************************************************************/


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* RW Library Initialization Routine                               */
/* cFE requires that a library have an initialization routine      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 rw_mutex_id;
int32 RW_LibInit(void)
{
	int32 retval;

	retval = OS_MutSemCreate(&rw_mutex_id, "RW_Mutex", 0);
	if (OS_SUCCESS != retval) {
		OS_printf("RW_lib failed to create RW Mutex: %d\n", retval);
		return retval;
	}

	return retval;
}/* End RW_LibInit */

/*
** Global RW Tach data
*/
float ws_1 = 0.0;
float ws_2 = 0.0;
float ws_3 = 0.0;

void RWS_SetTach(float wheel_speed_1, float wheel_speed_2, float wheel_speed_3)
{
	OS_MutSemTake(rw_mutex_id);
	ws_1 = wheel_speed_1;
	ws_2 = wheel_speed_2;
	ws_3 = wheel_speed_3;
	OS_MutSemGive(rw_mutex_id);
}

void RWS_GetTach(float *wheel_speed_1, float *wheel_speed_2, float *wheel_speed_3)
{
	OS_MutSemTake(rw_mutex_id);
	*wheel_speed_1 = ws_1;
	*wheel_speed_2 = ws_2;
	*wheel_speed_3 = ws_3;
	OS_MutSemGive(rw_mutex_id);
	// printf("RWS_GetTach: %f\t %f\t %f\n", ws_1, ws_2, ws_3);
}

void RWS_SetAllIdle(void) {
	if ( SIM_Data.StateData.ActState == 1 )	{
		// Not sure how to handle for simulator
	} else {
		rwlib_sendModeCommand(0, RW_MODE_IDLE, 0);
		rwlib_sendModeCommand(1, RW_MODE_IDLE, 0);
		rwlib_sendModeCommand(2, RW_MODE_IDLE, 0);
	}
	return;
}

/*******************************************************************/
/* Commands going to the subsystem                                 */
/*******************************************************************/

/*
** High Level Reaction wheel command
*/
int32 RW_LibCommandRW(float x, float y, float z)
{
	char CommandString[128];

	/*
	** If the RW interface is being simulated, return the sim data
	** Currently there is not a separate flag for the Reaction Wheels
	** Need to use the actuator state for now.
	*/
	if ( SIM_Data.StateData.ActState == 1 )
	{
		sprintf(CommandString, DHL_RWTC_CMD_STRING, x, y, z);
		SIM_SendCommand(CommandString);
	}
	else
	{
/* Disable for thermal vac
		rwlib_sendModeCommand(0, RW_MODE_TORQUE, x);
		rwlib_sendModeCommand(1, RW_MODE_TORQUE, y);
		rwlib_sendModeCommand(2, RW_MODE_TORQUE, z);
*/
//		OS_printf("Disabled rw commands\n");
	}
	return(0);
}

/*
** High Level Reaction Wheel speed query
*/
int32 RW_LibGetSpeed(float *X, float *Y, float *Z)
{
	float local_x, local_y, local_z;

	if ( X == 0 || Y == 0 || Z == 0 )
	{
		printf("ERROR: RW_LibGetSpeed passed null pointers\n");
		return(-1);
	}

	/*
	** If the RW interface is being simulated, return the sim data
	** Currently there is not a separate flag for the Reaction Wheels
	** Need to use the actuator state for now.
	*/
	if ( SIM_Data.StateData.ActState == 1 )
	{
		*X =  SIM_Data.WheelData.X;
		*Y =  SIM_Data.WheelData.Y;
		*Z =  SIM_Data.WheelData.Z;
	}
	else
	{
		rwlib_receiveParamTelem(0, RW_PARAM_SPEED, &(local_x));
		rwlib_receiveParamTelem(1, RW_PARAM_SPEED, &(local_y));
		rwlib_receiveParamTelem(2, RW_PARAM_SPEED, &(local_z));

		*X = local_x;
		*Y = local_y;
		*Z = local_z;
	}

	return(0);
}

/*******************************************************************/
/* lower level functions for the library                           */
/*******************************************************************/

int add_slip_byte(int count, uint8 data_array[], uint8 byte) {
	if (byte == SLIP_FEND) {
		data_array[count] = SLIP_FESC;
		count++;
		data_array[count] = SLIP_TFEND;
		count++;
	} else if (byte == SLIP_FESC) {
		data_array[count] = SLIP_FESC;
		count++;
		data_array[count] = SLIP_TFESC;
		count++;
	} else {
		// no SLIP needed.
		data_array[count] = byte;
		count++;
	}

	return count;
}

int read_slip_byte(uint8 array[], uint8* byte) {
	if (array[0] == SLIP_FESC) {
		// FESC, followed either by a TFEND or a TFESC
		if (array[1] == SLIP_TFEND) {
			*byte = SLIP_FEND;
			return 2;
		} else if (array[1] == SLIP_TFESC) {
			*byte = SLIP_FESC;
			return 2;
		} else {
			// TODO: Handle Error
			printf("Error in read slip byte, FESC not followed by TFEND or TFEST\n");
			*byte = array[1];
			return 2;
		}
	} else {
		*byte = array[0];
		return 1;
	}
}

// frame's data is set to the maximum count
// TODO: Make sure the max isn't exceeded (I2C_MTU)
int send_nsp_message(struct NSP_message nsp_mesg) {
	int retval = 0, count = 0, index = 0;
	uint8_t tx_data[MAX_RW_MSG];

	count = add_slip_byte(count, tx_data, nsp_mesg.src_addr);
	count = add_slip_byte(count, tx_data, nsp_mesg.mcf);

	for (index = 0; index < nsp_mesg.data_len; index++) {
		count = add_slip_byte(count, tx_data, nsp_mesg.data[index]);
	}

	// Send CRC, 2 bytes
	count = add_slip_byte(count, tx_data, ((uint8 *)(&nsp_mesg.crc))[0]);
	count = add_slip_byte(count, tx_data, ((uint8 *)(&nsp_mesg.crc))[1]);

	tx_data[count] = SLIP_FEND;
	count++;

	retval = i2c_master_transaction(0, nsp_mesg.dest_addr, tx_data, count, NULL, 0, 1000);

	if (retval != E_NO_ERR) {
		printf("RW DHL failed send_nsp_message: %d\n", retval);
		return retval;
	}

	return E_NO_ERR;
}

// For messages that require a response
// Buffer should already be allocated.
// ret_nsp_mesg should already be fully allocated
int send_recv_nsp_message(struct NSP_message nsp_mesg, struct NSP_message *ret_nsp_mesg) {
	int retval = 0, count = 0, index = 0;
	uint8_t tx_data[MAX_RW_MSG];
	int rx_length = ret_nsp_mesg->data_len * 2 + 3;

	rx_length = rx_length & 0xFF;

	uint8_t rx_data[rx_length];

	count = add_slip_byte(count, tx_data, nsp_mesg.src_addr);
	count = add_slip_byte(count, tx_data, nsp_mesg.mcf);

	for (index = 0; index < nsp_mesg.data_len; index++) {
		count = add_slip_byte(count, tx_data, nsp_mesg.data[index]);
	}

	// Send CRC, 2 bytes
	count = add_slip_byte(count, tx_data, ((uint8 *)(&nsp_mesg.crc))[0]);
	count = add_slip_byte(count, tx_data, ((uint8 *)(&nsp_mesg.crc))[1]);

	tx_data[count] = SLIP_FEND;
	count++;

	retval = i2c_master_transaction(0, nsp_mesg.dest_addr, tx_data, count, rx_data, rx_length, 1000);

	if (retval != E_NO_ERR) {
		printf("RW DHL failed send_recv_nsp_message: %d\n", retval);
		return retval;
	}

	ret_nsp_mesg->dest_addr = nsp_mesg.src_addr;
	ret_nsp_mesg->src_addr = nsp_mesg.dest_addr;

	int frame_index = 0; // needed to un-SLIP.
	uint8 next_byte;
	frame_index += read_slip_byte(&(rx_data[frame_index]), &next_byte);
	ret_nsp_mesg->mcf = next_byte;

	// Copy from recv_frame to the buffer
	index = 0;
	while (frame_index < rx_length - 3) {
		if (index >= ret_nsp_mesg->data_len) {
			break;
		} else {
			frame_index += read_slip_byte(&(rx_data[frame_index]), &next_byte);
			ret_nsp_mesg->data[index] = next_byte;
			index++;
		}
	}

	frame_index += read_slip_byte(&(rx_data[frame_index]), &next_byte);
	((uint8 *)&(ret_nsp_mesg->crc))[0] = next_byte;
	frame_index += read_slip_byte(&(rx_data[frame_index]), &next_byte);
	((uint8 *)&(ret_nsp_mesg->crc))[1] = next_byte;

	return E_NO_ERR;
}

// Message length is the header length plus the data length
#define NSP_HEADER_BYTES 3 // One for each destination address, source address, and message control field
#define POLY 0x8408 // x^16 + x^12 + x^5 + 1, reversed for LSB-first

uint16 generateCRC(int message_length, uint8* header, uint8* data) {
	uint16 crc = 0xFFFF;
	int index, jndex;

	// TODO: assert for message length
	for (index = 0; index < NSP_HEADER_BYTES; index++) {
		uint8 current_char = header[index];
		for (jndex = 0; jndex < 8; jndex++) {
			crc = (crc >> 1) ^ (((current_char ^ crc) & 0x01) ? POLY : 0);
			current_char >>= 1;
		}
	}

	if (message_length > NSP_HEADER_BYTES) {
		for (index = 0; index < message_length - NSP_HEADER_BYTES; index++) {
			uint8 current_char = data[index];
			for (jndex = 0; jndex < 8; jndex++) {
				crc = (crc >> 1) ^ (((current_char ^ crc) & 0x01) ? POLY : 0);
				current_char >>= 1;
			}
		}
	}

	return crc;
}

int verifyCRC(struct NSP_message nsp_mesg) {
	uint16 actual_crc;
	actual_crc = generateCRC(3 + nsp_mesg.data_len, &(nsp_mesg.dest_addr), nsp_mesg.data);

	if ((((uint8 *)(&actual_crc))[0] == 
		 ((uint8 *)(&nsp_mesg.crc))[0]) &&
		(((uint8 *)(&actual_crc))[1] == 
		 ((uint8 *)(&nsp_mesg.crc))[1])) {
		return 0;
	} else {
		printf("RW: CRC of message: %x\n", nsp_mesg.crc);
		printf("RW: CRC calculated: %x\n", actual_crc);
		return -1;
	}
}

// turn on wheel
// Address should be 0, 1, or 2
int rwlib_turnon(uint8 dest_index) {
	// TODO: Set array status
	int retval;

	rw_turnon_status[dest_index] = true;

	// Data - 0x00100000 for starting the wheel
	uint8 init_start_data[4];
	init_start_data[0] = 0x00;
	init_start_data[1] = 0x10;
	init_start_data[2] = 0x00;
	init_start_data[3] = 0x00;

	struct NSP_message nsp_mesg;
	nsp_mesg.dest_addr = rw_address_array[dest_index];
	nsp_mesg.src_addr = 0x11;
	nsp_mesg.mcf = NSP_MODULE_INIT;
	nsp_mesg.data_len = 4;
	nsp_mesg.data = init_start_data;
	nsp_mesg.crc = generateCRC(3 + 4, &(nsp_mesg.dest_addr), nsp_mesg.data);

	retval = send_nsp_message(nsp_mesg);

	if (retval == E_NO_ERR) {
		return (CFE_SUCCESS);
	} else {
		rw_turnon_status[dest_index] = false;
		return retval;
	}
}

// turn off wheel
// Address should be 0, 1, or 2
int rwlib_turnoff(uint8 dest_index) {
	int retval;

	rw_turnon_status[dest_index] = false;

	struct NSP_message nsp_mesg;
	nsp_mesg.dest_addr = rw_address_array[dest_index];
	nsp_mesg.src_addr = 0x11;
	nsp_mesg.mcf = NSP_MODULE_INIT;
	nsp_mesg.data_len = 0;
	nsp_mesg.crc = generateCRC(3, &(nsp_mesg.dest_addr), NULL);

	retval = send_nsp_message(nsp_mesg);

	if (retval == E_NO_ERR) {
		return (CFE_SUCCESS);
	} else {
		return retval;
	}
}

// xxxxx[rw 0 status][rw 1 status][rw 2 status]
uint8 rwlib_get_onoff_status(void) {
	uint8 retval = 0x00;

	if (rw_turnon_status[0]) {
		retval = retval | 0x04;
	}
	if (rw_turnon_status[0]) {
		retval = retval | 0x02;
	}
	if (rw_turnon_status[0]) {
		retval = retval | 0x01;
	}

	return retval;
}

int rwlib_receiveModeTelem(uint8 dest_index, uint8 *mode_type, float *mode_value) {
	int retval = 0;
	uint8 command = 0;
	struct NSP_message nsp_mesg;

	if (!rw_turnon_status[dest_index]) {
		return E_NO_DEVICE;
	}

	nsp_mesg.dest_addr = rw_address_array[dest_index];
	nsp_mesg.src_addr = 0x11;
	nsp_mesg.mcf = NSP_MCF_POLL_MASK | NSP_MODULE_APP_TELEMETRY;
	nsp_mesg.data_len = 1;
	nsp_mesg.data = &command;
	nsp_mesg.crc = generateCRC(3 + 1, &(nsp_mesg.dest_addr), nsp_mesg.data);

	uint8 recv_bytes[6];
	struct NSP_message nsp_recv_mesg;
	nsp_recv_mesg.data = recv_bytes;
	nsp_recv_mesg.data_len = 6;

	retval = send_recv_nsp_message(nsp_mesg, &nsp_recv_mesg);

	if (retval != E_NO_ERR) {
		rwlib_turnoff(dest_index);
		return retval; // Something went wrong.
	}

	if (verifyCRC(nsp_recv_mesg) < 0) {
		printf("RW: CRC check failed.\n");
	}

	*mode_type = nsp_recv_mesg.data[1];
	uint8 *floating_address = (uint8 *) mode_value;
	floating_address[0] =  nsp_recv_mesg.data[2];
	floating_address[1] =  nsp_recv_mesg.data[3];
	floating_address[2] =  nsp_recv_mesg.data[4];
	floating_address[3] =  nsp_recv_mesg.data[5];

	if (retval == E_NO_ERR) {
		return (CFE_SUCCESS);
	} else {
		rwlib_turnoff(dest_index);
		return retval;
	}
}

int rwlib_receiveParamTelem(uint8 dest_index, uint8 param_number, float *param_value) {
	int retval = CFE_SUCCESS;
	struct NSP_message nsp_mesg;

	if (!rw_turnon_status[dest_index]) {
		printf("RW Not turned on %d\n", dest_index);
		return E_NO_DEVICE;
	}

	nsp_mesg.dest_addr = rw_address_array[dest_index];
	nsp_mesg.src_addr = 0x11; // TODO: define
	nsp_mesg.mcf = NSP_MCF_POLL_MASK | NSP_MODULE_APP_TELEMETRY;
	nsp_mesg.data_len = 1;
	nsp_mesg.data = &param_number;
	nsp_mesg.crc = generateCRC(3 + 1, &(nsp_mesg.dest_addr), nsp_mesg.data);

	uint8 recv_bytes[5];
	struct NSP_message nsp_recv_mesg;
	nsp_recv_mesg.data = recv_bytes;
	nsp_recv_mesg.data_len  = 5;

	retval = send_recv_nsp_message(nsp_mesg, &nsp_recv_mesg);

	if (retval != E_NO_ERR) {
                rwlib_turnoff(dest_index);
		printf("RW send_recv failed %d - %d\n", dest_index, retval);
		return retval; // Something went wrong.
	}

	if (verifyCRC(nsp_recv_mesg) < 0) {
		printf("RW: CRC check failed.\n");
	}

	//if (param_number != nsp_recv_mesg.data[0]) { error }
	uint8 *floating_address = (uint8 *) param_value;
	floating_address[0] =  nsp_recv_mesg.data[1];
	floating_address[1] =  nsp_recv_mesg.data[2];
	floating_address[2] =  nsp_recv_mesg.data[3];
	floating_address[3] =  nsp_recv_mesg.data[4];

	if (retval == E_NO_ERR) {
		return (CFE_SUCCESS);
	} else {
		rwlib_turnoff(dest_index);
		return retval;
	}
}

int rwlib_sendModeCommand(uint8 dest_index, uint8 mode_number, float mode_value) {
	int retval = 0;
	struct NSP_message nsp_mesg;

	if (!rw_turnon_status[dest_index]) {
		return E_NO_DEVICE;
	}

	nsp_mesg.dest_addr = rw_address_array[dest_index];
	nsp_mesg.src_addr = 0x11;
	nsp_mesg.mcf = NSP_MODULE_APP_COMMAND;

	uint8 data_buffer[6];
	data_buffer[0] = 0; // indicates a mode command
	data_buffer[1] = mode_number;
   //printf("Mode value is %f\n", mode_value);
	uint8* floating_address = (uint8 *) &mode_value;
	data_buffer[2] = floating_address[0];
	data_buffer[3] = floating_address[1];
	data_buffer[4] = floating_address[2];
	data_buffer[5] = floating_address[3];

   //printf("data mode is %d\n", data_buffer[1]);
   //printf("data_buffer vals: %x %x %x %x\n", data_buffer[2], data_buffer[3], data_buffer[4], data_buffer[5]);

	nsp_mesg.data_len = 6;
	nsp_mesg.data = data_buffer;

	nsp_mesg.crc = generateCRC(3 + 6, &(nsp_mesg.dest_addr), nsp_mesg.data);

	retval = send_nsp_message(nsp_mesg);

	if (retval == E_NO_ERR) {
		return (CFE_SUCCESS);
	} else {
		rwlib_turnoff(dest_index);
		return retval;
	}
}

int rwlib_sendParamCommand(uint8 dest_index, uint8 param_number, float param_value) {
	int retval = 0;
	struct NSP_message nsp_mesg;

	if (!rw_turnon_status[dest_index]) {
		return E_NO_DEVICE;
	}

	nsp_mesg.dest_addr = rw_address_array[dest_index];
	nsp_mesg.src_addr = 0x11;
	nsp_mesg.mcf = NSP_MODULE_APP_COMMAND;

	uint8 data_buffer[5];
	data_buffer[0] = param_number;
	uint8* floating_address = (uint8 *) &param_value;
	data_buffer[1] = floating_address[0];
	data_buffer[2] = floating_address[1];
	data_buffer[3] = floating_address[2];
	data_buffer[4] = floating_address[3];

	nsp_mesg.data_len = 5;
	nsp_mesg.data = data_buffer;

	nsp_mesg.crc = generateCRC(3 + 5, &(nsp_mesg.dest_addr), nsp_mesg.data);

	retval = send_nsp_message(nsp_mesg);

	if (retval == E_NO_ERR) {
		return (CFE_SUCCESS);
	} else {
		rwlib_turnoff(dest_index);
		return retval;
	}
}

// Ping
#define PING_RET_LENGTH 97
int rwlib_ping(uint8 dest_index) {
	int retval;
	struct NSP_message nsp_mesg;

	if (!rw_turnon_status[dest_index]) {
		// RW should respond to ping if powered, but turned off.
		// return E_NO_DEVICE;
	}

	nsp_mesg.dest_addr = rw_address_array[dest_index];
	nsp_mesg.src_addr = 0x11; // Should be a define somewhere
	nsp_mesg.mcf = NSP_MCF_POLL_MASK | NSP_MODULE_PING;
	nsp_mesg.data_len = 0;
	// TODO: remove the magic 3
	nsp_mesg.crc = generateCRC(3, &(nsp_mesg.dest_addr), NULL);

	uint8 recv_bytes[PING_RET_LENGTH];
	struct NSP_message nsp_recv_mesg;
	nsp_recv_mesg.data = recv_bytes;
	nsp_recv_mesg.data_len = PING_RET_LENGTH;

	retval = send_recv_nsp_message(nsp_mesg, &nsp_recv_mesg);

	if (retval != E_NO_ERR) {
		return retval; // Something went wrong.
	}

	if (verifyCRC(nsp_recv_mesg) < 0) {
		printf("RW: CRC check failed.\n");
	}

	nsp_recv_mesg.data[PING_RET_LENGTH] = 0;

	printf("RW: Ping? %s\n", nsp_recv_mesg.data);

	if (retval == E_NO_ERR) {
		return (CFE_SUCCESS);
	} else {
		return retval;
	}
}

/************************/
/*  End of File Comment */
/************************/
