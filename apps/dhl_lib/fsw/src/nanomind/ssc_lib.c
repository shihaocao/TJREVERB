/*************************************************************************
** File:
**   $Id: ssc_lib.c  $
**
** Purpose: 
**   Shared library functions for the Special Services card
**
**   The SSC card has the following devices:
**      LTC2309 8 channel ADC ( 4 devices )
**      SC16IS760 USART/GPIO extender with I2C interface ( 2 devices ) 
**      MPU6000 IMU with SPI interface ( 4 devices ) 
**
**
*/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

#include <FreeRTOS.h>
#include <freertos/task.h>

#include <dev/i2c.h>
#include <dev/spi.h>
#include <dev/usart.h>

#include <util/error.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/

#define SSC_NUM_MPU_DEVICES   4

/*
**  SC16 related
*/
#define LSR_DR           0x01   // Data ready in RX FIFO

/*************************************************************************
** Private Function Prototypes
*************************************************************************/

uint8 SSC_MPU6000_InitInternal(spi_chip_t *spi_chip);
void  SSC_MPU6000_SpiSetupCs(spi_dev_t * spi_dev, spi_chip_t * spi_chip, uint8 cs);

void  SSC_SpiWriteReg(spi_chip_t *spi_chip, uint8 reg_base, uint8 value);
uint8 SSC_SpiReadReg(spi_chip_t *spi_chip, uint8 reg_base);

/**********************************************************************
**  Global data
**********************************************************************/
/*
** spi_dev is in nanomind/src/task_init.c
** It is only supposed to be initialized once
*/
extern spi_dev_t  spi_dev;

/*
** The spi_chip structure is for each
** SPI device
*/
static spi_chip_t mpu_spi_chip[SSC_NUM_MPU_DEVICES];

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SSC Library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SSC_LibInit(void)
{

/*
#define DHL_SSC_GPIO_0_0_GYRO_ENABLE     0
#define DHL_SSC_GPIO_0_1                 1
#define DHL_SSC_GPIO_0_2                 2
#define DHL_SSC_GPIO_0_3                 3
#define DHL_SSC_GPIO_0_4_PRI_BUCK_ENABLE 4
#define DHL_SSC_GPIO_0_5_PRI_FET_ENABLE  5
#define DHL_SSC_GPIO_0_6_TLC_ENABLE_1    6
#define DHL_SSC_GPIO_0_7_CAM_ENABLE_1    7


#define DHL_SSC_GPIO_1_0_CADET_MODEM_RESET  0
#define DHL_SSC_GPIO_1_1_CADET_HOST_RESET   1
#define DHL_SSC_GPIO_1_2_MAG_RESET          2
#define DHL_SSC_GPIO_1_3                    3
#define DHL_SSC_GPIO_1_4_BACKUP_BUCK_ENABLE 4
#define DHL_SSC_GPIO_1_5_BACKUP_FET_ENABLE  5
#define DHL_SSC_GPIO_1_6_TLC_ENABLE_2       6
#define DHL_SSC_GPIO_1_7_CAM_ENABLE_2       7
*/

 
   /*
   ** Program the GPIO Direction and value for each GPIO extender chip  
   **  A '1' in the GPIO register is output, a '0' bit is input.
   **
   ** For the IOSTATE ADDR, Initialize all pins to inactive. 
   **  All pins are are active high except for the gyro which is active low
   */
   SSC_SC16_WriteAreg(0x48, IODIR_ADDR,   0b11110001 ); 
   vTaskDelay(configTICK_RATE_HZ*0.001 );
   SSC_SC16_WriteAreg(0x48, IOSTATE_ADDR, 0b00000001 ); 

   SSC_SC16_WriteAreg(0x49, IODIR_ADDR,   0b11110101 ); 
   vTaskDelay(configTICK_RATE_HZ*0.001 );
   SSC_SC16_WriteAreg(0x49, IOSTATE_ADDR, 0b00000001 ); 

   return OS_SUCCESS;

}/* End SSC_LibInit */


/*
** Reset the cadet radio.
*/
void SSC_ResetCadetRadio ( void )
{
    uint8_t readval = 0;
    int     Status;

    /*
    ** Set GPIO Pin to 1 
    */
    Status = SSC_SC16_ReadAreg(0x49,  IOSTATE_ADDR, &readval);
    if ( E_NO_ERR != Status )
    {
        printf ( "ERROR: ResetCadetRadio first read error\n" );
        return;
    }
    readval = readval & 0b11111110;  /* GPIO pin 0 */
    Status = SSC_SC16_WriteAreg(0x49, IOSTATE_ADDR, readval);
    if ( E_NO_ERR != Status )
    {
        printf ( "ERROR: ResetCadetRadio first write error\n" );
        return;
    }

    vTaskDelay(configTICK_RATE_HZ*0.100);  /* TODO:Is 100 ms wait needed or if needed long enough or to long? */

    /*
    ** Set GPIO Pin to 0 
    */
    Status = SSC_SC16_ReadAreg(0x49,  IOSTATE_ADDR, &readval);
    if ( E_NO_ERR != Status )
    {
        printf ( "ERROR: ResetCadetRadio second read error\n" );
        return;
    }
    readval = readval | 0b00000001;  /* GPIO pin 0 */
    Status = SSC_SC16_WriteAreg(0x49, IOSTATE_ADDR, readval);
    if ( E_NO_ERR != Status )
    {
        printf ( "ERROR: ResetCadetRadio second write error\n" );
        return;
    }

    vTaskDelay(configTICK_RATE_HZ*0.100);  /* TODO:Is 100 ms wait needed or if needed long enough or to long? */

}


/*
** Generic SSC functions ( for SPI )
*/
void SSC_SpiWriteReg(spi_chip_t *spi_chip, uint8 reg_base, uint8 value)
{
   if (spi_lock_dev(spi_chip->spi_dev) < 0)
     return;
   spi_write(spi_chip, (reg_base << 8) | value);
   spi_read(spi_chip);
   spi_unlock_dev(spi_chip->spi_dev);
}

uint8 SSC_SpiReadReg(spi_chip_t *spi_chip, uint8_t reg_base)
{
   uint16_t data;

   if (spi_lock_dev(spi_chip->spi_dev) < 0)
	   return 0;
   spi_write(spi_chip, (reg_base << 8) | 0x8000);
   data = spi_read(spi_chip);
   spi_unlock_dev(spi_chip->spi_dev);
   return data & 0xFF;
}

/*
** LTC2309 A2D Functions -----------------------------------
*/

/*
** SSC_LTC2309_Read
**
**
** Construct a channel / uni/bipolar by bitwise ORing one choice from the channel configuration
** and one choice from the command.
**
** Example - read channel 3 single-ended
** adc_command = LTC2309_CH3 | LTC2309_UNIPOLAR_MODE;
** 
** Example - read voltage between channels 5 and 4 with 4 as positive polarity and in bipolar mode.
** adc_command = LTC2309_P4_N5 | LTC2309_BIPOLAR_MODE;
**
** Reads 12 bits in binary format
*/
int8 SSC_LTC2309_Read(uint8 i2c_address, uint8 adc_command, uint16 *ptr_adc_code)
{
  int8                  ack = 0;
  uint8                 txdata[1];
  uint16                data;
  uint8                 byte;

  txdata[0] = adc_command;  // for i2c_master_transaction

  ack = i2c_master_transaction(0, i2c_address, txdata, 1, &data, 2, 1000);
  if (ack != E_NO_ERR) {
    printf("SSC_LTC2309_Read failed i2c transaction: %d\n", ack);
    return ack;
  }

  // Correct byte order and shift. For example, turn 0xA032 into 0x32A0 into 0x032A
  byte = data >> 8;   // byte is now 0xA0
  data = data << 8;   // data is now 0x3200
  data = data | byte; // data is now 0x32A0
  data = data >> 4;   // data is now 0x032A
  *ptr_adc_code = data;

  return(ack);
}

/*
** SSC MPU6000 functions -----------------------------------------
*/

/*
** SSC_MPU6000_SpiSetupCs is internal to this module
*/
void SSC_MPU6000_SpiSetupCs(spi_dev_t * spi_dev, spi_chip_t * spi_chip, uint8 cs) 
{
   spi_chip->spi_dev = spi_dev;  // A pointer to the physical device SPI0
   spi_chip->baudrate = 1000000; 
   spi_chip->spi_mode = 0 ;      // SPI mode
   spi_chip->bits = 16;          // Default value for transferring bytes
   spi_chip->cs = cs;            // The chip select register
   spi_chip->reg = cs/4;         // The register bank to use
   spi_chip->spck_delay = 4;     // Delay
   spi_chip->trans_delay = 60;   // Delay
   spi_setup_chip(spi_chip);
}

/*
** SSC_MPU6000_InitInternal is internal to this module
*/
uint8 SSC_MPU6000_InitInternal(spi_chip_t *spi_chip)
{
   // TODO: Are these delays correct? 1 second, 3 seconds...
   vTaskDelay(100);
   SSC_SpiWriteReg(spi_chip, 0x6B, 0x80); // Chip reset
   vTaskDelay(100);
   SSC_SpiWriteReg(spi_chip, 0x6B, 0x03); // Wake up device and select GyroZ clock (better performance)
   SSC_SpiWriteReg(spi_chip, 0x6A, 0x10); // Disable I2C bus (recommended on datasheet)
   // TODO: Why is every other arg in hex, but not 19?
   SSC_SpiWriteReg(spi_chip, 0x19, 19);   // Sample rate = 50Hz    Fsample= 1Khz/(19+1) = 50Hz
   SSC_SpiWriteReg(spi_chip, 0x1A, 0x04); //  DLPF = 20Hz (low pass filter)
   SSC_SpiWriteReg(spi_chip, 0x1B, 0x18); // Gyro scale
   SSC_SpiWriteReg(spi_chip, 0x1C, 0x08); // Accel scale 4g (16384 LSB/g)
   SSC_SpiWriteReg(spi_chip, 0x6B, 0x03); // Oscillator set

   return 0;
}

/*
** SSC_MPU6000_Init - This is the public Init function for the MPU6000s
*/
int SSC_MPU6000_Init(void)
{
   int     i;
   uint8   fs_sel;
   uint8   afs_sel;
   uint8   dlpf_cfg;
   uint8   smplrt_div;

   SSC_MPU6000_SpiSetupCs(&spi_dev, &mpu_spi_chip[0], DHL_NM_SPI_CS_IMU1); 
   SSC_MPU6000_SpiSetupCs(&spi_dev, &mpu_spi_chip[1], DHL_NM_SPI_CS_IMU2); 
   SSC_MPU6000_SpiSetupCs(&spi_dev, &mpu_spi_chip[2], DHL_NM_SPI_CS_IMU3); 
   SSC_MPU6000_SpiSetupCs(&spi_dev, &mpu_spi_chip[3], DHL_NM_SPI_CS_IMU4); 

   // wake up mpu device
   SSC_MPU6000_InitInternal(&mpu_spi_chip[0]);
   SSC_MPU6000_InitInternal(&mpu_spi_chip[1]);
   SSC_MPU6000_InitInternal(&mpu_spi_chip[2]);
   SSC_MPU6000_InitInternal(&mpu_spi_chip[3]);

   return OS_SUCCESS;
}


/*
** MPU Read temperature
*/
uint16 SSC_MPU6000_ReadTemp(uint32 MpuDeviceNum) 
{

    if ( MpuDeviceNum > 3 ) 
    {
       return(0);
    }

    uint16 rdata;
    uint8 Hbyte, Lbyte; 
    Hbyte=SSC_SpiReadReg(&mpu_spi_chip[MpuDeviceNum], MPU_TEMP_H);  //H
    Lbyte=SSC_SpiReadReg(&mpu_spi_chip[MpuDeviceNum], MPU_TEMP_L);  //L
    rdata = (Hbyte << 8) | Lbyte;
    //return (short)(rdata)/340.0 + 36.53; 
    return rdata;
}

uint16 SSC_MPU6000_ReadRate(uint32 MpuDeviceNum, uint8 addrH) 
{
   uint16 rdata;
   uint8  Hbyte, Lbyte; 

   if ( MpuDeviceNum > 3 ) 
   {
      return(0);
   }

   Hbyte=SSC_SpiReadReg(&mpu_spi_chip[MpuDeviceNum],addrH);    //H
   Lbyte=SSC_SpiReadReg(&mpu_spi_chip[MpuDeviceNum],addrH+1);  //L
   rdata = Hbyte << 8 | Lbyte;
   //return ((short)rdata*MPU_GSCALE);
   return rdata;
}

uint16 SSC_MPU6000_ReadAcc(uint32 MpuDeviceNum, uint8 addrH) 
{
   uint16 rdata;
   uint8 Hbyte, Lbyte; 

   if ( MpuDeviceNum > 3 ) 
   {
      return(0);
   }

   Hbyte=SSC_SpiReadReg(&mpu_spi_chip[MpuDeviceNum],addrH);    //H
   Lbyte=SSC_SpiReadReg(&mpu_spi_chip[MpuDeviceNum],addrH+1);  //L
   rdata = Hbyte << 8 | Lbyte;
   //return ((short)rdata/MPU_DEGDIV);
   return rdata;

}


/*
** Functions for SC16 IO Extender ( UART and GPIO ) ------------- 
*/


/* 
** Global data: 
** Communication flags and variables
*/
/* Perhaps rename? */
char incoming_data;

/*
** 
*/
int SSC_SC16_Init(void)
{
   return 0;
}


int SSC_SC16_WriteAreg(uint8 AdevAddr, uint8 Areg, uint8 val)
{
  uint8 txdata[2];
  txdata[0] = Areg;
  txdata[1] = val;

  return (i2c_master_transaction(0, AdevAddr, txdata, 2, NULL, 0, 1000));
}


int SSC_SC16_ReadAreg (uint8 AdevAddr, uint8 Areg, uint8 *val)
{
  uint8 txdata[1];
  txdata[0] = Areg;
  if ( val == 0 )
  {
    printf ( "\n\n\n\n\nERROR: Calling SSC_SC16_ReadAreg with null pointer\n\n\n\n\n" );
  }

  return (i2c_master_transaction(0, AdevAddr, txdata, 1, val, 1, 1000));
}

int SSC_SC16_UartInit(char addrdev)
{
  printf("starting Uart Init\n");
  SSC_SC16_UartWriteByte(addrdev, LCR_ADDR, 0x80); // 0x80 to program baudrate
  SSC_SC16_UartWriteByte(addrdev, DLL_ADDR, 0x05); // 0x05=115.2K, with crystal X1=9.216 MHz
  SSC_SC16_UartWriteByte(addrdev, DLH_ADDR, 0x00);

  SSC_SC16_UartWriteByte(addrdev, LCR_ADDR, 0xBF); // access EFR register
  SSC_SC16_UartWriteByte(addrdev, EFR_ADDR, 0x10); // enable enhanced registers
  SSC_SC16_UartWriteByte(addrdev, LCR_ADDR, 0x03); // 8 data bit, 1 stop bit, no parity
  SSC_SC16_UartWriteByte(addrdev, FCR_ADDR, 0x06); // reset TXFIFO, reset RXFIFO, non FIFO mode
  SSC_SC16_UartWriteByte(addrdev, FCR_ADDR, 0x01); // enable FIFO mode

  return 0;

}

void SSC_SC16_CheckUart(void)
{
  while ((SSC_SC16_UartReadByte(IOExtender1, LSR_ADDR) & 0x01))
  {
    incoming_data = SSC_SC16_UartReadByte(IOExtender1, RHR_ADDR);
    /* printf("incoming data: %x\r\n", incoming_data); */
  }
}

uint8 SSC_SC16_UartReadByte(char addrdev,char addr)
{
  int8  retval;
  uint8 val= 0;

  retval = SSC_SC16_ReadAreg(addrdev, addr, &val);
  if (retval != E_NO_ERR) {
    printf("SSC_SC16_UartReadByte Failed %d\n", retval);
  }
  return val;
}

// write I2c command to uart
char SSC_SC16_UartWriteByte(char addrdev,char address, uint8 data)
{
  int8 retval;

  retval = SSC_SC16_WriteAreg(addrdev, address, data);  // address should be THR_ADDR
  if (retval != E_NO_ERR) {
    printf("SSC_SC16_UartWriteByte Failed %d\n", retval);
  }

  // TODO: Why return this arg?
  return data;
}

/************************/
/*  End of File Comment */
/************************/
