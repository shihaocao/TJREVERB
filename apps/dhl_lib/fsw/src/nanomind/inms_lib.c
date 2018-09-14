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

#include <stdio.h>
#include <dev/spi.h>
#include <csp/csp.h>

extern spi_dev_t spi_dev;
spi_chip_t spi_inms_chip;

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

void INMS_LibInitDevice(void)
{
	spi_inms_chip.spi_dev = &spi_dev;  // A pointer to the physical device SPI0
	spi_inms_chip.baudrate = 1000000; // This is only the initial baud rate, it will be increased by the driver
	spi_inms_chip.spi_mode = 0 ;      // SPI mode
	// TODO why not 16? NM supports 16 (from docs... not tested)
	spi_inms_chip.bits = 8;           // Default value for transferring bytes
	spi_inms_chip.cs = DHL_NM_SPI_CS_INMS;             // The chip select register
	spi_inms_chip.reg = 0;            // The register bank to use
	spi_inms_chip.spck_delay = 0;
	spi_inms_chip.trans_delay = 10;
	// TODO: Should stay_act be 0?
	spi_inms_chip.stay_act = 1;  // set this chip to stay active after last transfer
	spi_setup_chip(&spi_inms_chip);
}

/*
 * Sends the first 8 bytes of the CommandBuffer, and reads 1651 bytes back.
 * Telemetry may NOT be valid. Most of the time it will not be.
 * If valid, the first byte returned will be "A5"
 */
int32 INMS_LibTransferINMSData(uint8 *CommandBuffer, uint8 *TelemetryBuffer, uint32 TelemetryLength)
{
	uint32 retval;
	uint8 outbuf[TelemetryLength];

	if (spi_lock_dev(spi_inms_chip.spi_dev) < 0) {
		return 0;
	}

	memset(outbuf, 0x00, TelemetryLength);
	memcpy(outbuf, CommandBuffer, INMS_CMDSIZE);
	memset(CommandBuffer, 0x00, INMS_CMDSIZE);

	retval = spi_dma_transfer(&spi_inms_chip, outbuf, TelemetryBuffer, TelemetryLength);

	spi_unlock_dev(spi_inms_chip.spi_dev);

	return retval;
}

/************************/
/*  End of File Comment */
/************************/
