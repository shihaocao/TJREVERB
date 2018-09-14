/*************************************************************************
** File:
**   $Id: cam_lib.c  $
**
** Purpose: 
**   Shared library functions for the Serial Camera on Dellingr
**
**   $Log: cam_lib.c  $
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
#include <util/driver_debug.h>
#include <util/hexdump.h>
#include <util/byteorder.h>
#include <FreeRTOS.h>
#include <task.h>
#include <dev/usart.h>
#include <util/timestamp.h>
#include <dev/arm/at91sam7.h>
#include <dev/arm/cpu_pm.h>
#include <dev/i2c.h>
#include <util/error.h>


/*************************************************************************
** Macro Definitions
*************************************************************************/

#define CAMERA_DEBUG

#define GPIO_CHIP1_ADDR 0x48
#define GPIO_CHIP2_ADDR 0x49
#define CAM_ENABLE_PIN  0x80


static char h, l;
static int  z = 0x0000;      

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CAM Library Initialization Routine                             */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 CAM_LibInit(void)
{
    return OS_SUCCESS;
 
}/* End CAM_LibInit */

void CAM_gpio_enable(uint8 addr, uint8 reg, uint8 val)
{
   uint8_t currentVal;

   SSC_SC16_ReadAreg(addr, reg, &currentVal);
   SSC_SC16_WriteAreg(addr, reg, (currentVal | val));
}

void CAM_gpio_disable(uint8_t addr, uint8_t reg, uint8_t val)
{
   uint8_t currentVal;

   SSC_SC16_ReadAreg(addr, reg, &currentVal);
   SSC_SC16_WriteAreg(addr, reg, (currentVal & (~val)));
}

/************************************************************************/
/* Higher level camera functions */

/*
** Power On Camera
*/
void CAM_PowerOnCamera(void)
{
   /* 
   ** GPIO Extender 1 
   */
   CAM_gpio_enable(GPIO_CHIP1_ADDR, IODIR_ADDR, CAM_ENABLE_PIN);
   CAM_gpio_enable(GPIO_CHIP1_ADDR, IOSTATE_ADDR, CAM_ENABLE_PIN);

   // GPIO Extender 2
   CAM_gpio_enable(GPIO_CHIP2_ADDR, IODIR_ADDR, CAM_ENABLE_PIN);
   CAM_gpio_enable(GPIO_CHIP2_ADDR, IOSTATE_ADDR, CAM_ENABLE_PIN);

}

/*
** Power Off Camera
*/
void CAM_PowerOffCamera(void)
{
   CAM_gpio_disable(GPIO_CHIP1_ADDR, IOSTATE_ADDR, CAM_ENABLE_PIN);
   CAM_gpio_disable(GPIO_CHIP2_ADDR, IOSTATE_ADDR, CAM_ENABLE_PIN);
}


/*
** Init Camera
** Status: Working
*/
int CAM_InitCamera(void)
{
   if(SSC_SC16_UartInit(0x48) == 0)
   {
      printf("Camera Init OK\n");
   }
   return 0;
}

/*
** CAM_EmptyFifo - Empty the fifo
*/
void CAM_EmptyFifo(void)
{
   uint8_t  fiforead[64];
   char     fifo_len;
   int      i;

   fifo_len=SSC_SC16_UartReadByte(0x48, 0x48);
   printf("CAM_EmptyFifo: Fifo Size  = %d\n", fifo_len);

   for (i=0;i<fifo_len;i++)
   {
      fiforead[i]=SSC_SC16_UartReadByte(0x48, 0x00) ;
   }

   for (i=0;i<fifo_len;i++)
   {
      printf(" CAM_EmptyFifo: Fifo[%d] = %02X\n", i,fiforead[i]);
   }
}

/*
** Reset the I2C/Uart Fifo
*/
void CAM_ResetFifo(void)
{
   char fifo_size;
   /*
   ** read the number of bytes in the fifo
   */
   fifo_size=SSC_SC16_UartReadByte(0x48, 0x48);
   if (fifo_size>0)
   {
      /*
      ** reset the transmit fifo
      */
      SSC_SC16_WriteAreg(0x48, FCR_ADDR , 0x02);
   }
}


/*
** CAM_SetPictureSize:
** Status:  Works, sometimes has to be repeated to take effect
**
** Sizes
** <1> 0x22 : 160*120
** <2> 0x11 : 320*240
** <3> 0x00 : 640*480
** <4> 0x1D : 800*600
** <5> 0x1C : 1024*768
** <6> 0x1B : 1280*960
** <7> 0x21 : 1600*1200
*/
void CAM_SetPictureSize(uint8 Size)
{
   SSC_SC16_WriteAreg(0x48,0x00,0x56);
   SSC_SC16_WriteAreg(0x48,0x00,0x00);
   SSC_SC16_WriteAreg(0x48,0x00,0x54);
   SSC_SC16_WriteAreg(0x48,0x00,0x01);
   SSC_SC16_WriteAreg(0x48,0x00,Size);

   CAM_EmptyFifo();
}

/* 
** Get Picture Size - Return the size of the picture that was captured
** Status: Works after the picture capture command is called
*/
uint16 CAM_GetPictureSize(void)
{
   uint8 dataread[9];
   uint16 picsize=0;
   int data_len,i;

   SSC_SC16_WriteAreg(0x48,0x00,0x56);
   SSC_SC16_WriteAreg(0x48,0x00,00);
   SSC_SC16_WriteAreg(0x48,0x00,0x34);
   SSC_SC16_WriteAreg(0x48,0x00,01);
   SSC_SC16_WriteAreg(0x48,0x00,00);
   vTaskDelay(configTICK_RATE_HZ );

   data_len=SSC_SC16_UartReadByte(0x48, 0x48);
   printf("GetPictureSize: Fifo Size  = %d\n", data_len);

   for (i=0;i<data_len;i++)
   {
      dataread[i]=SSC_SC16_UartReadByte(0x48, 0x00) ;
   }
   for (i=0;i<data_len;i++)
   {
      printf("Fifo[%d] = %02X\n", i, dataread[i]);
   }
   if (data_len>0)
   {
      picsize=(dataread[7]*256)+dataread[8];
   }
   return picsize;
}


/* 
** Send Reset command
*/
int CAM_ResetCamera(void)
{
   int status = 0;

   status = SSC_SC16_WriteAreg(0x48,0x00, 0x56);
   status = SSC_SC16_WriteAreg(0x48,0x00, 0x00);
   status = SSC_SC16_WriteAreg(0x48,0x00, 0x26);
   status = SSC_SC16_WriteAreg(0x48,0x00, 0x00);
   vTaskDelay(configTICK_RATE_HZ * 1);

   CAM_EmptyFifo();

   return status;
}
 
/*
** Send take picture command
**
*/
int CAM_SnapImage(void)
{

   int return_code = 0;

   if (((SSC_SC16_WriteAreg(0x48,0x00,0x56)) == E_NO_ERR)  &&
       ((SSC_SC16_WriteAreg(0x48,0x00,0x00)) == E_NO_ERR)  &&
       ((SSC_SC16_WriteAreg(0x48,0x00,0x36)) == E_NO_ERR)  &&
       ((SSC_SC16_WriteAreg(0x48,0x00,0x01)) == E_NO_ERR)  &&
       ((SSC_SC16_WriteAreg(0x48,0x00,0x00)) == E_NO_ERR)  )
   {
      return_code = 0 ;
      CAM_EmptyFifo();

   }
   else
   {
      return_code = 1;
   }

   return(return_code);
}

/*
** Read data
*/
int CAM_GetCapturedImage(int addr)
{
   uint8_t MH, ML, MM;
   int     tempm;

   MH = addr / 0x10000;
   tempm = addr % 0x10000;
   MM = tempm / 0x100;
   ML = tempm % 0x100;

   #if 0
   printf(" ----> ReadCameraData: address = %d\n",addr);
   printf(" ----> ReadCameraData: address (hex) = %X\n",addr);
   printf(" ----> ReadCameraData: MH: %02X ,MM: %02X, Ml: %02X  \n",MH,MM,ML);
   #endif

   SSC_SC16_WriteAreg(0x48,0x00,0x56);
   SSC_SC16_WriteAreg(0x48,0x00,0x00);
   SSC_SC16_WriteAreg(0x48,0x00,0x32);
   SSC_SC16_WriteAreg(0x48,0x00,0x0C);
   SSC_SC16_WriteAreg(0x48,0x00,0x00);
   SSC_SC16_WriteAreg(0x48,0x00,0x0A);
   SSC_SC16_WriteAreg(0x48,0x00,0x00);
   SSC_SC16_WriteAreg(0x48,0x00,MH);
   SSC_SC16_WriteAreg(0x48,0x00,MM);
   SSC_SC16_WriteAreg(0x48,0x00,ML);
   SSC_SC16_WriteAreg(0x48,0x00,0x00);
   SSC_SC16_WriteAreg(0x48,0x00,0x00);
   SSC_SC16_WriteAreg(0x48,0x00,0x00);
   /* Just ask for 32 bytes at a time */
   /* RX fifo is 64, may want to up this? */
   SSC_SC16_WriteAreg(0x48,0x00,0x20);
   SSC_SC16_WriteAreg(0x48,0x00,0x00);
   SSC_SC16_WriteAreg(0x48,0x00,0x0A);

   addr += 0x20;
   #if 0
   printf("ReadCameraData: Current Addr: %d\n",addr);
   #endif

   return addr;

}

int CAM_EndCaptureImage(void)
{
    SSC_SC16_WriteAreg(0x48,0x00,0x56);
    SSC_SC16_WriteAreg(0x48,0x00,0x00);
    SSC_SC16_WriteAreg(0x48,0x00,0x36);
    SSC_SC16_WriteAreg(0x48,0x00,0x01);
    SSC_SC16_WriteAreg(0x48,0x00,0x03);
    vTaskDelay(configTICK_RATE_HZ );
    CAM_EmptyFifo();
    return 0;
}
 

/*
** Get Image Data
** Status :  working
*/
int CAM_GetImage(char *FileName)
{
   uint8_t picByte;
   int EndFlag=0;
   int j = 0;
   int k = 0;
   int count = 0;
   int zerodatacount = 0;
   int fifodatacount;
   int receivecount=0 ;
   int byteswritten;
   uint8_t b[32];

   int ImageFile;

   ImageFile = OS_open( FileName, OS_READ_WRITE, 0);
   if ( ImageFile < 0 )
   {
       /* Error */
       printf("Error: Cannot open file!\n");
       return(-1);
   }

   /* 
   ** empties any additional characters from the Fifo before sending image command 
   */
   while ((SSC_SC16_UartReadByte(0x48, 0x28) & 0x01))
   {
      picByte = SSC_SC16_UartReadByte(0x48, 0x00);
   }

   while(EndFlag != 1 )
   {
      j = 0;
      k = 0;
      count = 0;
      
      receivecount = CAM_GetCapturedImage(receivecount);

      vTaskDelay(100 / portTICK_RATE_MS); // may increase if needed

      fifodatacount = SSC_SC16_UartReadByte(0x48, 0x48);
      
      #if 0
        printf("CAM_GetImage, after readdata called, there are %d bytes in fifo\n",
                           fifodatacount);
      #endif

      /*
      ** If 10 consecutive reads return zero data, abort
      */
      if ( fifodatacount == 0 )
      { 
         zerodatacount++;
         if ( zerodatacount > 10 ) 
         {
            printf("CAM_GetImage: Looks like no data is coming back from the camera, aborting\n");
            break;
         }
      }
      else
      {
         zerodatacount = 0;
      }

      /*
      ** While there is data in the Fifo
      */
      while ((SSC_SC16_UartReadByte(0x48, 0x28) & 0x01))
      {	
         picByte = SSC_SC16_UartReadByte(0x48, 0x00);
         k++;

         if((k>5)&&(j<32)&&(EndFlag != 1)) 
         {
            b[j]=picByte;
			if (j>0)
			{
				if((b[j-1]==0xFF)&&(b[j]==0xD9))
				{
					EndFlag=1;
					printf("CAM_GetImage: end of JPEG file detected\n");
				}
			}
			else
			{
				if((b[31]==0xFF)&&(b[j]==0xD9))
				{
					EndFlag=1;
					printf("CAM_GetImage: inner whileloop: end of JPEG file detected\n");
				}
			}
            j++;
            count++;
         }
      } /* end while */

      #if 0
      printf("CAM_GetImage: Finished inner whileloop, data count = %d\n",count);
      printf("CAM_GetImage : Data follows\n");
      #endif

      /*
      ** Write data to a file
      */
      byteswritten = OS_write(ImageFile, b, count );
      if ( byteswritten != count )
      {
         printf("CAM_GetImage Error: byteswritten : %d , count = %d\n",byteswritten, count);
      }

      #if 0
      printf("File Data: Addr = %X, bytes = %d, bytes written = %d\n", receivecount, count, byteswritten);
      #endif
   }
   vTaskDelay(3000 / portTICK_RATE_MS);
   int rtn = 0;

   printf("\n CAM_GetImage: Sending Stop Capture command\n");
   CAM_EndCaptureImage();
   printf(" File Closed\n");
   OS_close(ImageFile);

   return rtn;
}


/*
** Get Next Image Chunk
** This function will read the next section of the image file from the camera memory. 
** It will write the data to the open file descriptor OsalFileFd.
** 
** This function must be available, because the calling application can only process a portion of the data
** per cycle. 
*/
int32 CAM_GetNextImageChunk(int32 OsalFileFd, uint32 ImageOffset)
{
   uint8_t b[32];
   uint8_t picByte;
   int     EndFileFlag=0;
   int32   ReceiveCount = ImageOffset ;
   int     j = 0;
   int     k = 0;
   int     count = 0;
   int     BytesWritten;
   int     ErrFlag = 0;

   /* 
   ** empties any additional characters from the Fifo before sending image command 
   */
   while ((SSC_SC16_UartReadByte(0x48, 0x28) & 0x01))
   {
      picByte = SSC_SC16_UartReadByte(0x48, 0x00);
   }

   /*
   ** Send the command to the camera to read the next chunk 
   ** of data
   */
   ReceiveCount = CAM_GetCapturedImage(ReceiveCount);
   vTaskDelay(300 / portTICK_RATE_MS); // may increase if needed

#if 0
   /* Note: this is not used, delete it? */
   int32   FifoDataCount;
   /*
   ** After the delay, read how many bytes are in the 
   ** Rx FIFO
   */
   FifoDataCount = SSC_SC16_UartReadByte(0x48, 0x48);
#endif
      
   /*
   ** While there is data in the Fifo
   */
   while ((SSC_SC16_UartReadByte(0x48, 0x28) & 0x01))
   {	
      picByte = SSC_SC16_UartReadByte(0x48, 0x00);
      k++;

      if((k>5)&&(j<32)&&(EndFileFlag != 1)) 
      {
         b[j]=picByte;
         if (j>0)
         {
            if((b[j-1]==0xFF)&&(b[j]==0xD9))
            {
               EndFileFlag=1;
               printf("CAM_GetNextImageChunk: end of JPEG file detected\n");
            }
         }
         else
         {
            if((b[31]==0xFF)&&(b[j]==0xD9))
            {
               EndFileFlag=1;
               printf("CAM_GetNextImageChunk: end of JPEG file detected\n");
            }
         }
         j++;
         count++;
      }
   } /* end while */

   #if 0
   printf("CAM_GetNextImageChunk: Finished inner whileloop, data count = %d\n",count);
   #endif

   /*
   ** Write data to file
   */
   BytesWritten = OS_write(OsalFileFd, b, count );
   if ( BytesWritten != count )
   {
      printf("CAM_GetImage Error: BytesWritten : %d , count = %d\n",BytesWritten, count);
      /*
      ** In this case, errors will be ignored. 
      ** I think it is better to save as much data as possible, rather than 
      ** aborting the file transfer
      */
   }
   
   if ( EndFileFlag == 1 )
   {
      return(CAM_END_OF_FILE);
   }
   else
   {
      return (ReceiveCount);
   } 
}

/************************/
/*  End of File Comment */
/************************/
