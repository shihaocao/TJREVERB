/*************************************************************************
** File:
**   $Id: inms_lib.c  $
**
** Purpose: 
**   Shared library functions for the INMS functions used on Dellingr
**
**   $Log: inms_lib.c  $
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
** Global and external data 
*************************************************************************/
/* 
** This variable is in task_init.c 
*/

/*************************************************************************
** Private Function Prototypes
*************************************************************************/


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* INMS Library Initialization Routine                             */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 INMS_LibInit(void)
{
	return OS_SUCCESS;
}/* End INMS_LibInit */



/*
** Init SPI for the INMS instrument
** This code will have to be moved into the task_init.c file.
** Does calling spi_init_dev a second time invalidate the setup for the SD card?
*/
FILE* data_file;
void INMS_LibInitDevice(void)
{
	data_file = fopen("sim_data/data_1436990073.exo", "r");
	if (data_file == NULL) {
		perror("Failed to open test data for INMS");
	}

	OS_printf("  INMS SPI Init called\n");
}

// Reads packets from a local file.
int32 INMS_LibTransferINMSData(uint8 *CommandBuffer, uint8 *TelemetryBuffer, uint32 TelemetryLength)
{
	int isReady = 1;

	int read_ret;

	printf("INMS Command sent: %2X %2X %2X %2X %2X %2X %2X %2X\r\n", CommandBuffer[0],
		CommandBuffer[1], CommandBuffer[2], CommandBuffer[3], CommandBuffer[4], 
		CommandBuffer[5], CommandBuffer[6], CommandBuffer[7]);
	// size_t fread(void *ptr, size_t size, size_t nmembFILE *" stream );
	read_ret = fread(TelemetryBuffer, sizeof(unsigned char), TelemetryLength, data_file);

	if (read_ret != TelemetryLength) { // Only works because sizeof(unsigned char) == 1
		perror("INMS test data read returned unexpected data amount");
	}

	return (isReady); 
}

/************************/
/*  End of File Comment */
/************************/
