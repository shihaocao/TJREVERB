/*************************************************************************
** File:
**   $Id: ssc_lib.c  $
**
** Purpose: 
**   Shared library functions for the Special Services card 
**
**   $Log: ssc_lib.c  $
**
** Notes for LTC2309
** Modified for Nanomind 1/25/15 (E.L)
** LTC2309: 8-channel, 12-Bit SAR ADC with I2C interface
** 
** The LTC2309 is a low noise, low power, 8-channel, 12-bit successive 
** approximation ADC with an I2C compatible serial interface. This ADC includes an 
** internal reference and a fully differential sample-and-hold circuit to reduce 
** common mode noise. The LTC2309 operates from an internal clock to achieve a fast 
** 1.3 microsecond conversion time. 
** 
** The LTC2309 operates from a single 5V supply and draws just 300 microamps at a 
** throughput rate of 1ksps. The ADC enters nap mode when not converting, reducing 
** the power dissipation. 
**
**
LTC2309: 8-channel, 12-Bit SAR ADC with I2C interface

@verbatim

The LTC2309 is a low noise, low power, 8-channel, 12-bit successive
approximation ADC with an I2C compatible serial interface. This ADC includes an
internal reference and a fully differential sample-and-hold circuit to reduce
common mode noise. The LTC2309 operates from an internal clock to achieve a fast 
1.3 microsecond conversion time.

The LTC2309 operates from a single 5V supply and draws just 300 microamps at a
throughput rate of 1ksps. The ADC enters nap mode when not converting, reducing
the power dissipation.

I2C DATA FORMAT (MSB First):

       Byte #1                             Byte #2
START  SA6 SA5 SA4 SA3 SA2 SA1 SA0 W SACK  SD OS S1 S0 UNI SLP X X SACK

             Byte #3                             Byte #4                             Byte #5
Repeat Start SA6 SA5 SA4 SA3 SA2 SA1 SA0 R SACK  D11 D10 D9  D8  D7  D6  D5 D4 MACK  D3 D2 D1 D0 X  X  X  X  MNACK  STOP

SACK  : Slave Acknowledge
MACK  : Master Acknowledge
MNACK : Master Not Acknowledge
SD    : Single, Differential# Bit
OS    : ODD, Sign# Bit
Sx    : Address Select Bit
COM   : CH7/COM Configuration Bit
UNI   : Unipolar, Bipolar# Bit
SLP   : Sleep Mode Bit
Dx    : Data Bits
X     : Don't care

Example Code:

Read Channel 0 in Single-Ended Unipolar mode

    adc_command = CH0 | UNIPOLAR_MODE;                  // Build ADC command for channel 0

    ack |= LTC2309_read(I2C_ADDR, adc_command, &adc_code);   // Throws out last reading
    ack |= LTC2309_read(I2C_ADDR, adc_command, &adc_code);   // Obtains the current reading and stores to adc_code variable

    Header for LTC2309: 8-channel, 12-Bit SAR ADC with I2C interface

*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/

#define SSC_NUM_MPU_DEVICES   4
#define SSC_NUM_LTC_DEVICES   4

/*************************************************************************
** Private Function Prototypes
*************************************************************************/


/**********************************************************************
**  Global data
**********************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SSC Library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SSC_LibInit(void)
{
    return OS_SUCCESS;
}/* End SSC_LibInit */


/*
** Reset the cadet radio.
*/
void SSC_ResetCadetRadio ( void )
{
    return;
}


/* 
** Number of LTC2309 and MPU devices present on board 
*/

/**********************************************
** Commands
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
    /* may want to remove I2C address from API.. */
int8 SSC_LTC2309_Read(uint8 i2c_address, uint8 adc_command, uint16 *ptr_adc_code)
{
  int8 ack = 0;

  *ptr_adc_code = 0x032A; // Good of test code as any

  return(ack);
}

/* 
**
** Calculates the LTC2309 input unipolar voltage.
**
*/

float SSC_LTC2309_CodeToVoltage(uint16 adc_code)
{
  float voltage;

      voltage = (float)adc_code;
      voltage = voltage / ((1<<12) - 1)* 4.096;

  return(voltage);

}

#if 0
/*
** SSC MPU6000 functions 
*/
void SSC_mpu6000_spi_setup_cs(spi_dev_t * spi_dev, spi_chip_t * spi_chip, uint8 cs) 
{
   return;
}
#endif

void SSC_MPU_init(void)
{
   return;
}
int SSC_MPU6000_Init(void)
{
   return 0;
}

/*
** MPU Read temperature
*/
uint16 SSC_MPU6000_ReadTemp(uint32 MpuDeviceNum)
{
    uint16 rdata = 0;

    if ( MpuDeviceNum > 3 )
    {
       return(0);
    }
    return rdata;
}

uint16 SSC_MPU6000_ReadRate(uint32 MpuDeviceNum, uint8 addrH)
{
   uint16 rdata = 0;
   if ( MpuDeviceNum > 3 )
   {
      return(0);
   }
   return rdata;
}

uint16 SSC_MPU6000_ReadAcc(uint32 MpuDeviceNum, uint8 addrH)
{
   uint16 rdata = 0;

   if ( MpuDeviceNum > 3 )
   {
      return(0);
   }
   return rdata;
}

/*
** Is this generic for all SPI devices and not just the MPU?
*/
#if 0
void SSC_spi_write_reg(spi_chip_t *spi_chip, uint8 reg_base, uint8 value)
{
     return;
}

uint8 SSC_spi_read_reg(spi_chip_t *spi_chip, uint8_t reg_base)
{
   return 0;
}

#endif

float SSC_mpu6000_read_temp(uint32 MpuDeviceNum ) 
{
    /*
    ** To-Do: Need to convert MPU device number to the spi_chip that is already initialized
    */

    if ( MpuDeviceNum > 3 ) 
    {
       return(0);
    }
    return 0; 
}

float SSC_mpu6000_read_rate(uint32 MpuDeviceNum, uint8 addrH) 
{
   if ( MpuDeviceNum > 3 ) 
   {
      return(0);
   }

   return (0);
}

float SSC_mpu6000_read_angle(uint32 MpuDeviceNum, uint8 addrH) 
{
   if ( MpuDeviceNum > 3 ) 
   {
      return(0);
   }

   return (0);

}

// sc16 related
#define LSR_DR           0x01   // Data ready in RX FIFO

// Communication flags and variables
char incoming_data;

int sc16_write_Areg(uint8 AdevAddr, uint8 Areg, uint8 val)
{
  return 0;
}


int sc16_read_Areg (uint8 AdevAddr, uint8 Areg, uint8 *val)
{
  return 0;
}

/*****************************************************
* uart section
******************************************************/
int I2C_Uart_Init(char addrdev)
{
  printf("starting Uart Init\n");
  return 0;
}

void checkUART(void)
{
  return;
}

uint8 I2C_Uart_ReadByte(char addrdev,char addr)
{
  return 0;
}

// write I2c command to uart
char I2C_Uart_WriteByte(char addrdev,char address, uint8 data)
{
  return 0;
}

/************************/
/*  End of File Comment */
/************************/
