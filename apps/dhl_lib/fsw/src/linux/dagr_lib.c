/*************************************************************************
** File:
**   $Id: dagr_lib.c  $
**
** Purpose: 
**   Shared library functions for the DAGR Magnetometer functions on Dellingr
**
**   $Log: dagr_lib.c  $
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>


/*************************************************************************
** Macro Definitions
*************************************************************************/

/*************************************************************************
** Global variables 
*************************************************************************/


/*************************************************************************
** Private Function Prototypes
*************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DAGR Library Initialization Routine                             */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 DAGR_LibInit(void)
{

    return OS_SUCCESS;
 
}/* End DAGR_LibInit */



int32 DAGR_InitMagDevice(void)
{
  /* 
  ** Setup the SPI1 hardware for magnetometer 
  */

  return(0);
}

/*
** Read a packet from the DAGR instrument
**
** OPERATION:
** send a power command 1,2,3, or 4 (first byte of 39 bytes to be sent)- 
** use power mode 1 for default.
** then receive and send 39 bytes (full duplex) making sure that the 37th
** byte sent is followed by a EOF (99).
**
** TODO: Report failed attempt?
*/
// Example data from Todd
uint8 exampleData[] = {0x01, 0x01, 0x02, 0x03, 0x02, 0x04, 0x05, 0x06, 0x01, 0x07, 0x08, 0x09,
                       0x02, 0x65, 0x66, 0x67, 0x01, 0x68, 0x69, 0x6A, 0x02, 0x6B, 0x6C, 0x6D,
                       //0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, // From test board
                       0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, // From Todd
                       0xFA, 0xFB, 0xFC};
int32 DAGR_ReadDagrPacket(uint8 CommandCode, uint8 *DagrData)
{
  uint8       Acmd[36]; // 36 bytes from DAGR Instrument
  int         i; 

  memset(Acmd,0x00,sizeof(Acmd));

  for (i = 0; i<sizeof(Acmd); i++)
  {
    DagrData[i] = exampleData[i];
  }

  return(0);
}

/*
** Read a packet from the Boom Data from the 42 simulator 
**
*/
int32 DAGR_LibReadBoom(float *X, float *Y, float *Z)
{

	/* 
   **Return data from the simulator 
	*/
	*X = SIM_Data.DagrData.X;
	*Y = SIM_Data.DagrData.Y;
	*Z = SIM_Data.DagrData.Z;

   return(0);
}
   
	

/************************/
/*  End of File Comment */
/************************/
