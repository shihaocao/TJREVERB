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

bool rw_turnon_status[3] = {false};

// Should functions build messages each time? Seems like the safest
// Do I need to worry about mutual exclusion?
struct NSP_message {
	uint8 dest_addr;
	uint8 src_addr;
	uint8 mcf; // message control field
	uint8* data;
	uint8 data_len;
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

/*************************************************************************
** Private Function Prototypes
*************************************************************************/


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* RW Library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 rw_mutex_id;
int32 RW_LibInit(void)
{
	int32 retval;

	retval = OS_MutSemCreate(&rw_mutex_id, "RW_Mutex", 0);

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
}

void RWS_SetAllIdle(void) {
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
    char CommandString[256];

    sprintf(CommandString, DHL_RWTC_CMD_STRING, x, y, z);
    SIM_SendCommand(CommandString);
    return(0);
}

/*
** High Level Reaction Wheel speed query
*/
int32 RW_LibGetSpeed(float *X, float *Y, float *Z)
{
   if ( X == 0 || Y == 0 || Z == 0 )
   {
      return(-1);
   }

   /*
   ** Return the simulated data
   */
   *X =  SIM_Data.WheelData.X;
   *Y =  SIM_Data.WheelData.Y;
   *Z =  SIM_Data.WheelData.Z;

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
int send_nsp_message(struct NSP_message nsp_mesg) 
{
    return (CFE_SUCCESS);
}

// For messages that require a response
// Buffer should already be allocated.
// ret_nsp_mesg should already be fully allocated
// TODO: CAN ONLY READ 144 CHARACTERS (maybe less with SLIP)
int send_recv_nsp_message(struct NSP_message nsp_mesg, struct NSP_message *ret_nsp_mesg) 
{
	return CFE_SUCCESS;
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
int rwlib_turnon(uint8 dest_index) {
	rw_turnon_status[dest_index] = true;
	return (CFE_SUCCESS);
}

// turn off wheel
int rwlib_turnoff(uint8 dest_index) {
	rw_turnon_status[dest_index] = false;
	return (CFE_SUCCESS);
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
	return (CFE_SUCCESS);
}

int rwlib_receiveParamTelem(uint8 dest_index, uint8 param_number, float *param_value) {
	int retval = CFE_SUCCESS;
	return retval;
}

int rwlib_sendModeCommand(uint8 dest_index, uint8 mode_number, float mode_value) {
	int retval = 0;
	   return retval;
}

int rwlib_sendParamCommand(uint8 dest_index, uint8 param_number, float param_value) {
    int retval = 0;
    return retval;
}

// Ping
// PING DOES NOT WORK: Only seem to be able to read 144 characters.
int rwlib_ping(uint8 dest_index) {
	return (CFE_SUCCESS);
}

/************************/
/*  End of File Comment */
/************************/
