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
#include "dagr_lib.h"

#include <string.h>
#include <dev/spi.h>
#include "dhl_hw_defs.h"


/*************************************************************************
** Macro Definitions
*************************************************************************/

/*************************************************************************
** Global variables
*************************************************************************/
extern spi_dev_t spi_dev; // one spi_dev delared in task_init.
spi_chip_t       spi_magnetometer_chip;


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
  // TODO: Need to check the cs, bits, and stay_act setings.
  spi_magnetometer_chip.spi_dev = &spi_dev;
  spi_magnetometer_chip.baudrate = 625000;
  spi_magnetometer_chip.spi_mode = 0;
  spi_magnetometer_chip.bits = 8;
  spi_magnetometer_chip.cs = DHL_NM_SPI_CS_DAGR;
  spi_magnetometer_chip.reg = 0;
  spi_magnetometer_chip.spck_delay = 0;
  spi_magnetometer_chip.trans_delay = 10;
  spi_magnetometer_chip.stay_act = 1;
  spi_setup_chip(&spi_magnetometer_chip);

  return 0;
}

/*
** Read a packet from the DAGR instrument
**
** OPERATION:
** send a power command 1,2,3, or 4 (first byte of 36 bytes to be sent)-
** use power mode 1 for default.
** then receive and send 36 bytes (full duplex) making sure that the last
** byte sent is an EOF (99 (0x63)).
*/
int32 DAGR_ReadDagrPacket(uint8 CommandCode, uint8 *DagrData)
{
  uint8       Acmd[36]; // 36 bytes from DAGR Instrument.
  uint16      i;

  memset(Acmd,0x00,sizeof(Acmd));

  /*
  ** Select power command: 0x01,0x02,0x03 or 0x04
  */
  Acmd[0] = CommandCode;

  /*
  ** stuff payload with selected command. Instrument will ignore payload.
  */
  for (i = 1; i < sizeof(Acmd); i++)
  {
       Acmd[i] = CommandCode;
  }

  /*
  ** instrument will look for this byte
  */
  Acmd[sizeof(Acmd) - 1] = 0x63;

  // Lock device
  // TODO: Is locking once better than locking for each byte? Are these calls going to block / is that okay?
  if (spi_lock_dev(spi_magnetometer_chip.spi_dev) != 0) { // 0 if okay, -1 if error
    printf("ERROR: DAGR Failed SPI Lock call.\n");
    return -1;
  }

  uint16 dataval;
  for (i = 0; i<sizeof(Acmd); i++)
  {
    dataval = Acmd[i] | 0x0000;

    // These calls busy wait on the SPI device, but that might not be a problem.
    spi_write(&spi_magnetometer_chip, dataval); // void return, so no error information.
    DagrData[i] = (uint8) spi_read(&spi_magnetometer_chip);
  }

  spi_unlock_dev(spi_magnetometer_chip.spi_dev);

  return 0;
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
   *X = 0;// SIM_Data.DagrData.X;
   *Y = 0;// SIM_Data.DagrData.Y;
   *Z = 0;// SIM_Data.DagrData.Z;

   return(0);
}

/************************/
/*  End of File Comment */
/************************/
