/*************************************************************************
** File:
**   $Id: gyro_lib.c  $
**
** Purpose: 
**   Shared library functions for the Gyro used on Dellngr
**   This is the nanomind version
**
**   $Log: gyro_lib.c  $
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"
#include "FreeRTOS.h"
#include "task.h"

#include <util/error.h>
#include <stdio.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/
#define GYRO_DEBUG

#define GYRO_GPIO_ADDR 0x48
#define GYRO_UART_ADDR 0x49

#define GYRO_DATA_LEN   18

typedef struct
{
	uint8  identifier;
	uint8  status;
	uint8  crc;
	float  x_axis_gyro;
	float  y_axis_gyro;
	float  z_axis_gyro;
	uint16 x_axis_temperature;
	uint16 y_axis_temperature;
	uint16 z_axis_temperature;
} gyro210_message_t ;

/*************************************************************************
** Global Data 
*************************************************************************/


/*************************************************************************
** Private Function Prototypes
*************************************************************************/

/*
** calculate crc8
*/
uint8 GYRO_Crc8(uint8 ncrc, uint8 ndata)
{
	uint8_t i;
	unsigned short  data;

	data = ncrc ^ ndata;
	data <<= 8;

	for ( i = 0; i < 8; i++ )
	{
		if (( data & 0x8000 ) != 0 )
		{
			data = data ^ 0x8380;
		}
		data = data << 1;
	}
	return (unsigned char)( data >> 8 );
}

/*
** convert 3 bytes of Gyro data to one float value
*/
float GYRO_Convert(uint8 AR1, uint8 AR2, uint8 AR3)
{
	float  gyrodata;
	float  sgyrodata;
	uint32 tgyrodata;
	uint16 th,tl;

	tgyrodata = ((AR1 << 16)+(AR2 << 8)+AR3);
	if (tgyrodata>8388608)//2^23
	{
		th=tgyrodata >> 16;
		th=~th;
		th=th & 0x00ff;
		tl=tgyrodata % 65536;
		tl=~tl;

		tgyrodata = (th << 16) + tl + 1;
		sgyrodata = tgyrodata;
		sgyrodata = -1 * sgyrodata;
	}
	else
	{
		sgyrodata = tgyrodata;
	}

	gyrodata = sgyrodata / 16384;
	gyrodata = gyrodata * 0.017453289; // convert to radians
	return gyrodata;
}

/*
** read gyro value
*/
int GYRO_Read210(gyro210_message_t *GyroData)
{
   uint8   datagyro[GYRO_DATA_LEN];
   int     status,i;
   int     data_len;
   uint8   ncrc=0xff;
   int     SC_success;
   uint8_t readval = 0;

   /*
   ** Set GPIO pin to 0
   */
   SC_success = SSC_SC16_ReadAreg(GYRO_GPIO_ADDR,  IOSTATE_ADDR, &readval);
   readval = readval & 0b11111110;  /* GPIO pin 0 */
   SC_success = SSC_SC16_WriteAreg(GYRO_GPIO_ADDR, IOSTATE_ADDR, readval);

   vTaskDelay(0.5); // 0.5ms works, no delay fails

   /*
   ** Set GPIO Pin to 1 
   */
   SC_success = SSC_SC16_ReadAreg(GYRO_GPIO_ADDR,  IOSTATE_ADDR, &readval);
   readval = readval | 0x01;  /* GPIO pin 0 */
   SC_success = SSC_SC16_WriteAreg(GYRO_GPIO_ADDR, IOSTATE_ADDR, readval);

   vTaskDelay(2); // 2ms works, 0.5ms fails.
   data_len = SSC_SC16_UartReadByte(GYRO_UART_ADDR,0x48);
   //printf("Imu FIFO bytes  = %d\n", data_len);
   if (data_len > GYRO_DATA_LEN)
   {
      status = SSC_SC16_WriteAreg(GYRO_UART_ADDR, FCR_ADDR, 0x03);//FCR_ADDR reset RX fifo
      status = -2;
   }
   else
   {
      for (i=0;i<data_len;i++)
      {
         datagyro[i] = SSC_SC16_UartReadByte(GYRO_UART_ADDR,0x00) ;
      }

      GyroData->identifier=datagyro[0];
      GyroData->x_axis_gyro=GYRO_Convert(datagyro[1],datagyro[2],datagyro[3]);
      GyroData->y_axis_gyro=GYRO_Convert(datagyro[4],datagyro[5],datagyro[6]);
      GyroData->z_axis_gyro=GYRO_Convert(datagyro[7],datagyro[8],datagyro[9]);
      GyroData->status=datagyro[10];
      GyroData->x_axis_temperature = (datagyro[11] << 8) + datagyro[12];
      GyroData->y_axis_temperature = (datagyro[13] << 8) + datagyro[14];
      GyroData->z_axis_temperature = (datagyro[15] << 8) + datagyro[16];

      for (i=0; i<17; i++)
      {
         ncrc=GYRO_Crc8(ncrc,datagyro[i]);
      }
      if (ncrc==datagyro[17])
      {
         GyroData->crc= 1;
         status = 0;
      }
      else
      {
         printf("ncrc, gyrocrc = %x %x\n", ncrc, datagyro[17]);
         GyroData->crc= 0;
         status = -1;
      }
   }
   return(status);
}

/**************************************************************************
** API functions
***************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Gyro library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 gy_mutex_id;
// TODO: May have to split into a init_on_sc_startup and init_on_gyro_turnon
int32 GYRO_LibInit(void)
{
   uint8_t readval = 0;
   int     SC_success;

   SC_success = OS_MutSemCreate(&gy_mutex_id, "GY_Mutex", 0);
   if (OS_SUCCESS != SC_success) 
   {
      OS_printf("gyro_lib failed to create GY Mutex: %d\n", SC_success);
      return OS_ERROR;
   }

   /*
   ** GPIO Enable 
   */
   SC_success = SSC_SC16_ReadAreg(GYRO_GPIO_ADDR,  IOSTATE_ADDR, &readval);
   readval = readval | 0x01;  /* GPIO pin 0 */
   SC_success = SSC_SC16_WriteAreg(GYRO_GPIO_ADDR, IOSTATE_ADDR, readval);
   if (E_NO_ERR != SC_success) 
   {
      OS_printf("gyro_lib unable to write to gpio address: %d\n", SC_success);
      return OS_ERROR;
   }

   // TODO: SSC_SC16_UartInit always returns 0
   if(SSC_SC16_UartInit(GYRO_UART_ADDR) == 0)
   {
      printf("Gyro UART initialized successfully!\n");
   }
   return OS_SUCCESS;

}/* End GYRO_LibInit */

/*
** Global Gyro Tach data
*/
float gy_1 = 0.0;
float gy_2 = 0.0;
float gy_3 = 0.0;

void GYS_SetTach(float x_value, float y_value, float z_value)
{
	OS_MutSemTake(gy_mutex_id);
	gy_1 = x_value;
	gy_2 = y_value;
	gy_3 = z_value;
	OS_MutSemGive(gy_mutex_id);
}

void GYS_GetTach(float *x_value, float *y_value, float *z_value)
{
	/*
	** If the Gyro interface is being simulated, return the sim data
	*/
	if ( SIM_Data.StateData.GyroState == 1 )
	{
		*x_value =  SIM_Data.GyroData.XR;
		*y_value =  SIM_Data.GyroData.YR;
		*z_value =  SIM_Data.GyroData.ZR;
	}
	else
	{
	OS_MutSemTake(gy_mutex_id);
	*x_value = gy_1;
	*y_value = gy_2;
	*z_value = gy_3;
	OS_MutSemGive(gy_mutex_id);
	//printf("GYS_GetTach: %f\t %f\t %f\n", gy_1, gy_2, gy_3);
	}
}

int GYRO_LibGetXyz(float *X, float *Y, float *Z)
{
	int                ReturnCode = 0;
	gyro210_message_t  GyroData;

	if ( X == 0 || Y == 0 || Z == 0 )
	{
		return(-1);
	}

	/*
	** If the Gyro interface is being simulated, return the sim data
	*/
	if ( SIM_Data.StateData.GyroState == 1 )
	{
		*X =  SIM_Data.GyroData.XR;
		*Y =  SIM_Data.GyroData.YR;
		*Z =  SIM_Data.GyroData.ZR;
	}
	else
	{
		ReturnCode = GYRO_Read210(&GyroData);
		if ( ReturnCode == 0 )
		{
			*X = GyroData.x_axis_gyro;
			*Y = GyroData.y_axis_gyro;
			*Z = GyroData.z_axis_gyro;
		}
		else if ( ReturnCode == -1 )
		{
			printf("GYRO_LibGetXyz: Gyro CRC failed\n");
		}
		else if ( ReturnCode == -2 )
		{
			printf("GYRO_LibGetXyz: Gyro UART fifo failed\n");
		}
	}
	return (ReturnCode);
}

int GYRO_LibGetData(GYRO_data_t *GyroData)
{
	int                ReturnCode = 0;
	gyro210_message_t  InternalGyroData;

	if ( GyroData == 0 )
	{
		return(-1);
	}

	/*
	** If the GPS interface is being simulated, return the sim data
	*/
	if ( SIM_Data.StateData.GyroState == 1 )
	{
		GyroData->x_axis_gyro =  SIM_Data.GyroData.XR;
		GyroData->y_axis_gyro =  SIM_Data.GyroData.YR;
		GyroData->z_axis_gyro =  SIM_Data.GyroData.ZR;

		GyroData->x_axis_temperature = 10;
		GyroData->y_axis_temperature = 11;
		GyroData->z_axis_temperature = 12;

	}
	else
	{
		ReturnCode = GYRO_Read210(&InternalGyroData);
		if ( ReturnCode == 0 )
		{
			GyroData->x_axis_gyro = InternalGyroData.x_axis_gyro;
			GyroData->y_axis_gyro = InternalGyroData.y_axis_gyro;
			GyroData->z_axis_gyro = InternalGyroData.z_axis_gyro;

			GyroData->x_axis_temperature = InternalGyroData.x_axis_temperature;
			GyroData->y_axis_temperature = InternalGyroData.y_axis_temperature;
			GyroData->z_axis_temperature = InternalGyroData.z_axis_temperature;

		}
		else if ( ReturnCode == -1 )
		{
			printf("GYRO_LibGetData: Gyro CRC failed\n");
		}
		else if ( ReturnCode == -2 )
		{
			printf("GYRO_LibGetData: Gyro UART fifo failed\n");
		}
	}
	return (ReturnCode);
}

/************************/
/*  End of File Comment */
/************************/
