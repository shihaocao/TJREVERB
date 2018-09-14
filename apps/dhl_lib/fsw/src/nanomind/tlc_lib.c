/*************************************************************************
** File:
**   $Id: tlc_lib.c  $
**
** Purpose: 
**   Shared library functions for the Thermal Lourvre Control Experiment 
**       on Dellingr
**
**   $Log: tlc_lib.c  $
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"
#include <util/error.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/


/*************************************************************************
** Private Function Prototypes
*************************************************************************/

int32 TLC_LibInit(void) {
	return 0;
}

void tlc_gpio_enable(uint8 addr, uint8 reg, uint8 val) {
	uint8 currentVal, debugVal;

	printf("\tGPIO Enable 0x%02X, 0x%02X, 0x%02X\n", addr, reg, val);
	SSC_SC16_ReadAreg(addr, reg, &currentVal);
	SSC_SC16_WriteAreg(addr, reg, currentVal | val);
	SSC_SC16_ReadAreg(addr, reg, &debugVal);
	printf("\tbefore write 0x%02X\tafter write 0x%02X\n", currentVal, debugVal);
}

void tlc_gpio_disable(uint8 addr, uint8 reg, uint8 val) {
	uint8 currentVal, debugVal;

	printf("\tGPIO Disable 0x%02X, 0x%02X, 0x%02X\n", addr, reg, val);
	SSC_SC16_ReadAreg(addr, reg, &currentVal);
	SSC_SC16_WriteAreg(addr, reg, currentVal & (~val));
	SSC_SC16_ReadAreg(addr, reg, &debugVal);
	printf("\tbefore write 0x%02X\tafter write 0x%02X\n", currentVal, debugVal);
}

void TLCTurnOn(void) {

	// GPIO Extender 1
	tlc_gpio_enable(GPIO_CHIP1_ADDR, IOSTATE_ADDR, TLC_ENABLE_PIN);

	// GPIO Extender 2
	tlc_gpio_enable(GPIO_CHIP2_ADDR, IOSTATE_ADDR, TLC_ENABLE_PIN);

	printf("Heaters powered on\n");
}

void TLCTurnOff(void) {

	// GPIO Extender 1
	tlc_gpio_disable(GPIO_CHIP1_ADDR, IOSTATE_ADDR, TLC_ENABLE_PIN);

	// GPIO Extender 2
	tlc_gpio_disable(GPIO_CHIP2_ADDR, IOSTATE_ADDR, TLC_ENABLE_PIN);

	printf("Heaters powered off\n");
}

/************************/
/*  End of File Comment */
/************************/
