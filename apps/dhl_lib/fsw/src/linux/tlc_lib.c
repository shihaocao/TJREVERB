/*************************************************************************
** File:
**   $Id: tlc_lib.c  $
**
** Purpose: 
**   Shared library functions for the Thermal Louvre Control Experiment 
**       on Dellingr
**
**   $Log: tlc_lib.c  $
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"


/*************************************************************************
** Macro Definitions
*************************************************************************/


/*************************************************************************
** Private Function Prototypes
*************************************************************************/


int32 TLC_LibInit(void) {
	printf("Thermal Louvre fake init.\n");
}

void TLCTurnOn(void) {
	printf("Thermal Louvre fake turn on.\n");
}

void TLCTurnOff(void) {
	printf("Thermal Louvre fake turn off.\n");
}

void TLCReadData(uint8 *data) {
	uint16 testData[] = {0xbeef, 0x1234, 0x5678};
	memcpy(data, testData, sizeof(testData));
	printf("Thermal Louvre test data copied.\n");
}


/************************/
/*  End of File Comment */
/************************/
