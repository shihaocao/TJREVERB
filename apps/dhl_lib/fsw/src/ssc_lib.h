/*************************************************************************
** File:
**   $Id: ssc_lib.h  $
**
** Purpose: 
**   Specification for the Special Services Card functions.
**
**   The SSC card has the following devices:
**      LTC2309 8 channel ADC with SPI(?) interface ( 4 devices )
**      SC16IS760 USART/GPIO extender with I2C interface ( 2 devices ) 
**      MPU6000 IMU with SPI interface ( 4 devices ) 
**
** References:
**
**   $Log: ssc_lib.h  $
**  
*************************************************************************/
#ifndef _ssc_lib_h_
#define _ssc_lib_h_

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include <stdbool.h>

/************************************************************************
** Macros 
*************************************************************************/


/*
** Macros for LTC2309 A2D converter ----------------------------------
*/
#define LTC_I2C_ADDR0 0x18
#define LTC_I2C_ADDR1 0x19
#define LTC_I2C_ADDR2 0x1A
#define LTC_I2C_ADDR3 0x1B

/* Single-Ended Channel Configuration */
#define LTC_CH0                0x80
#define LTC_CH1                0xC0
#define LTC_CH2                0x90
#define LTC_CH3                0xD0
#define LTC_CH4                0xA0
#define LTC_CH5                0xE0
#define LTC_CH6                0xB0
#define LTC_CH7                0xF0

/*
** Modified for Nanomind 1/25/15 (E.L)
** LTC1867 Configuration Bits
*/
#define LTC_SLEEP_MODE         0x04
#define LTC_EXIT_SLEEP_MODE    0x00
#define LTC_UNIPOLAR_MODE      0x08
#define LTC_BIPOLAR_MODE       0x00
#define LTC_SINGLE_ENDED_MODE  0x80
#define LTC_DIFFERENTIAL_MODE  0x00

/*
** Macros for the SC16IS760 IO Extender ( USART and GPIO ) -----------------
*/
#define IOExtender1  0x48 // GPIO IO Extender1 I2C address
#define IOExtender2  0x49 // GPIO IO Extender2 I2C address

#define DLL_ADDR  0x00    // divisor latch lsb r/w
#define DLH_ADDR  0x08    // divisor latch msb r/w

#define RHR_ADDR  0x00 // Receive Holding Register R
#define THR_ADDR  0x00 // Transmit Holding Register W
#define IER_ADDR 0x08  // Interrupt Enable Register R/W
#define FCR_ADDR 0x10  // FIFO Control Register W
#define IIR_ADDR 0x10  // Line Control Register R
#define LCR_ADDR 0x18  // Modem Control Register R/W
#define MCR_ADDR 0x20  // Line Status Register R/W
#define LSR_ADDR 0x28  // Line Status Register R/W
#define MSR_ADDR 0x30  // Line Status Register R
#define SPR_ADDR 0x38  // Line Status Register R/W
#define TCR_ADDR 0x30  // Line Status Register R/W *
#define TLR_ADDR 0x38  // Line Status Register R/W *
#define TXLVL_ADDR 0x40  // Line Status Register R/W *
#define RXLVL_ADDR 0x48  // Line Status Register R/W *
#define IODIR_ADDR 0x50  // GPIO Dir Register 1=output, 0 = input R/W
#define IOSTATE_ADDR 0x58  // GPIO State Register R/W
#define IOINTENA_ADDR 0x60  // GPIO INT Enable Register R/W
#define IOControl 0x70  // GPIO IO Control Register
// Enhanced Feature registers accessible wonly when LCR = 0xBF
#define EFR_ADDR   0x10  // Enhanced features Register
#define XON1_ADDR  0x20  // Xon1 Word
#define XON2_ADDR  0x28  // Xon2 word
#define XOFF1_ADDR 0x30  // Xoff1 word
#define XOFF2_ADDR 0x38  // Xoff2 word

#define OUTPUT 1
#define INPUT 0

#define SC16IS760_FIFO_SIZE	(64)
#define SC16IS760_REG_SHIFT	2

/*
** Macros for MPU 6000 device --------------------------------------
*/
#define MPU_GSCALE 9.80665/8192.0
#define MPU_DEGDIV 131

/*MPU 6000 reg addr:      hex     dec    description    */
#define MPU_ACCEL_XH 	 0x3B    //59   ACCEL_X[15:8]
#define MPU_ACCEL_XL 	 0x3C    //60   ACCEL_X[7:0]
#define MPU_ACCEL_YH 	 0x3D    //61   ACCEL_Y[15:8]
#define MPU_ACCEL_YL 	 0x3E    //62   ACCEL_Y[7:0]
#define MPU_ACCEL_ZH 	 0x3F    //63   ACCEL_Z[15:8]
#define MPU_ACCEL_ZL 	 0x40    //64   ACCEL_Z[7:0]
#define MPU_TEMP_H   	 0x41    //65   TEMP[15:8]
#define MPU_TEMP_L   	 0x42    //66   TEMP[7:0]
#define MPU_GYRO_XH  	 0x43    //67   GYRO_X[15:8]
#define MPU_GYRO_XL  	 0x44    //68   GYRO_X[7:0]
#define MPU_GYRO_YH  	 0x45    //69   GYRO_Y[15:8]
#define MPU_GYRO_YL  	 0x46    //70   GYRO_Y[7:0]
#define MPU_GYRO_ZH  	 0x47    //71   GYRO_Z[15:8]
#define MPU_GYRO_ZL  	 0x48    //72   GYRO_Z[7:0]

#define MPU_DEVICE_0     0
#define MPU_DEVICE_1     1
#define MPU_DEVICE_2     2
#define MPU_DEVICE_3     3

/************************************************************************
** Type Definitions
*************************************************************************/

/*************************************************************************
** Exported Functions
*************************************************************************/
/*
** SSC library init. This is called 
** by ES when the library is loaded.
*/
int32 SSC_LibInit(void);
void SSC_ResetCadetRadio ( void );

/*
** Reads 12-bit code from LTC2309, programs channel and mode for next conversion.
** Returns the state of the acknowledge bit after the I2C address write. 
*/
int8 SSC_LTC2309_Read(uint8   i2c_address,  // I2C address of device
                      uint8   adc_command,  // ADC command / address bits
                      uint16 *adc_data);    // Returns code read from ADC

float SSC_LTC2309_CodeToVoltage(uint16 adc_code);

/*
** MPU6000 functions
*/
int  SSC_MPU6000_Init(void);
/* Used to be this? int32 SSC_MPU6000_Init(int32 MpuDeviceNum); */
uint16 SSC_MPU6000_ReadTemp(uint32 MpuDeviceNum);
uint16 SSC_MPU6000_ReadRate(uint32 MpuDeviceNum, uint8 addrH) ;
uint16 SSC_MPU6000_ReadAcc(uint32 MpuDeviceNum, uint8 addrH) ;


/*
** SC16 IO Extender functions
*/
int   SSC_SC16_Init(void);
int   SSC_SC16_UartInit(char addrdev);
void  SSC_SC16_CheckUart(void);
char  SSC_SC16_UartWriteByte(char addrdev, char address, uint8 data);
uint8 SSC_SC16_UartReadByte(char addrdev, char addr);

int   SSC_SC16_WriteAreg(uint8 AdevAddr, uint8 Areg, uint8  val);
int   SSC_SC16_ReadAreg (uint8 AdevAddr, uint8 Areg, uint8 *val);


#endif /* _ssc_lib_h_ */

/************************/
/*  End of File Comment */
/************************/
