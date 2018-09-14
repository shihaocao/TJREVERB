/*************************************************************************
** File:
**   $Id: tlc_lib.h  $
**
** Purpose: 
**   Specification for the TLC ( Thermal Louvre Control ) subsystem functions.
**
** References:
**
**   $Log: tlc_lib.h  $
**  
*************************************************************************/
#ifndef _tlc_lib_h_
#define _tlc_lib_h_

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

/************************************************************************
** Type Definitions
*************************************************************************/
#define GPIO_CHIP1_ADDR 0x48
#define GPIO_CHIP2_ADDR 0x49

#define TLC_ENABLE_PIN  0x40

typedef struct
{
	uint8              Voltage_MSB;
	uint8              Voltage_LSB;
	uint8              Thermister1_MSB;
	uint8              Thermister1_LSB;
	uint8              Thermister2_MSB;
	uint8              Thermister2_LSB;
} OS_PACK tlc_data_reading_t;

/*************************************************************************
** Exported Functions
*************************************************************************/

int32 TLC_LibInit(void);
void TLCTurnOn(void);
void TLCTurnOff(void);
void TLCReadData(uint8 *data);

/************************************************************************/

#endif /* _tlc_lib_h_ */

/************************/
/*  End of File Comment */
/************************/
