/*************************************************************************
** File:
**   $Id: wallops_fss.c  $
**
** Purpose:
**   Shared library functions Wallops Fine Sun Sensor device
**
**
*************************************************************************/

/*
** Comment DEBUG_FSS out to disable debug error messages
*/
//#define DEBUG_FSS 1


#include "dhl_lib.h"

#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <dev/spi.h>

extern spi_dev_t spi_dev; // one spi_dev delared in task_init.

spi_chip_t fss_spi_chip;


/*
** Read the data from the Wallops FSS
*/
int WFSS_Initchip(void)
{
   // TODO: Need to check the cs, bits, and stay_act setings.
   fss_spi_chip.spi_dev = &spi_dev; /* A pointer to the physical device SPI0 */
   fss_spi_chip.baudrate = 5000000;
   fss_spi_chip.spi_mode = 2 ;     /* SPI mode */
   fss_spi_chip.bits = 8;          /* Default value for transferring bytes */
   fss_spi_chip.cs = DHL_NM_SPI_CS_WFSS;            /* The chip select register */
   fss_spi_chip.reg = DHL_NM_SPI_CS_WFSS/4;         /* The register bank to use */
   fss_spi_chip.spck_delay = 4;    /* Delay */
   fss_spi_chip.trans_delay = 60;  /* Delay */
   fss_spi_chip.stay_act = 1;
   spi_setup_chip(&fss_spi_chip);

   return(0);
}
int SOL_LibReadWFSS(WFSS_data_t *WFssData)
{
   uint16_t data = 0;
   uint16_t data2 = 0;
   unsigned char chsum=0;
   unsigned char dataspi[25];
   float    az;
   float    el;
   float    ux;
   float    uy;
   float    uz;
   uint32_t azi;
   uint32_t eli;
   uint32_t uxi;
   uint32_t uyi;
   uint32_t uzi;
   uint16_t th;
   uint16_t tl;
   int      i;
   int      ReturnCode = 0;
   int      error=0;
   uint16_t counter=0;

   // The spi init call is made once at start up. It should NOT be called for every read.

   /*
   ** Lock SPI Device
   */
   if(spi_lock_dev(&spi_dev)==0)
   {
      /* wait until the device is ready , with a timeout */
      while(data%256 != 0xf7 && counter<25) // 25 is the size of data
      {
         spi_write(&fss_spi_chip, 0xcc);
         data=spi_read(&fss_spi_chip);
         // printf("first byte: %x\n",data);
         if (data%256 == 0xf7)
         {
            spi_write(&fss_spi_chip, 0xcc);
            data2=spi_read(&fss_spi_chip);
            // printf("second byte: %x\n",data2);
            if (data2%256 != 0xE5)
            {
               data=0;
            }

         }
         //printf("Results: %d\n",data);
         counter++;
      }

      if(counter<25)
      {
//         spi_write(&fss_spi_chip, 0xcc);
//         data=spi_read(&fss_spi_chip);
//         printf("second byte: %x\n",data);
         for (i=0;i<23;i++)
         {
            spi_write(&fss_spi_chip, 0x01);
            data=spi_read(&fss_spi_chip);
            //printf("Results: %d\n",data);
            dataspi[i]=data;
            chsum=chsum+data;
         }
       //printf("Wallops check sum: %d\n",chsum);

         /* Unlock SPI Device*/
         spi_unlock_dev(&spi_dev);

         /*
         ** If the checksum is OK, process the data
         */
         if (chsum == 0)
         {
	    // TODO: Consider making this repeated calculation a function, and using bitshifts 
            azi=dataspi[3]*16777216+dataspi[2]*65536+dataspi[1]*256+dataspi[0];
            if (azi > 2147483648U)
            {
               th=azi/65536;
               th=~th;
               tl=azi%65536;
               tl=~tl;
               azi=th*65536+tl+1;//(2's Complement)
               az=azi;
               az=-1*az;
            }
            else
            {
               az=azi;
            }
            az=az/23000000 ;

            eli=dataspi[7]*16777216+dataspi[6]*65536+dataspi[5]*256+dataspi[4];
            if (eli > 2147483648U)
            {
               th=eli/65536;
               th=~th;
               tl=eli%65536;
               tl=~tl;
               eli=th*65536+tl+1;//(2's Compliment)
               el=eli;
               el=-1*el;
            }
            else
            {
               el=eli;
            }
            el=el/23000000 ;

            //Unit_Vector_X
            uxi=dataspi[11]*16777216+dataspi[10]*65536+dataspi[9]*256+dataspi[8];
            if (uxi > 2147483648U)
            {
               th=uxi/65536;
               th=~th;
               tl=uxi%65536;
               tl=~tl;
               uxi=th*65536+tl+1;//(2's Compliment)
               ux=uxi;
               ux=-1*ux;
            }
            else
            {
               ux=uxi;
            }
            ux=ux/23000000 ;

            //Unit_Vector_Y
            uyi=dataspi[15]*16777216+dataspi[14]*65536+dataspi[13]*256+dataspi[12];
            if (uyi > 2147483648U)
            {
               th=uyi/65536;
               th=~th;
               tl=uyi%65536;
               tl=~tl;
               uyi=th*65536+tl+1;//(2's Compliment)
               uy=uyi;
               uy=-1*uy;
            }
            else
            {
               uy=uyi;
            }
            uy=uy/23000000 ;

            //Unit_Vector_Z
            uzi=dataspi[19]*16777216+dataspi[18]*65536+dataspi[17]*256+dataspi[16];
            if (uzi > 2147483648U)
            {
               th=uzi/65536;
               th=~th;
               tl=uzi%65536;
               tl=~tl;
               uzi=th*65536+tl+1;//(2's Compliment)
               uz=uzi;
               uz=-1*uz;
            }
            else
            {
               uz=uzi;
            }
            uz=uz/23000000 ;

            /*
            ** Return the data to the caller
            */
            WFssData->Azimuth=az;
            WFssData->Elevation=el;
            WFssData->Unit_Vector_X=ux;
            WFssData->Unit_Vector_Y=uy;
            WFssData->Unit_Vector_Z=uz;
            WFssData->FOV=dataspi[20];
            WFssData->InRange=dataspi[21];

         }
         else /* Checksum != 0 */
         {
            #ifdef DEBUG_FSS
            printf("SOL_LibReadWFSS: FSS Checksum is bad\n");
            #endif
            ReturnCode = -1;
         }
      }

      else /* Retry counter is equal to or greater 50 */
      {
         #ifdef DEBUG_FSS
         printf("SOL_LibReadWFSS: FSS did not respond to SPI Request\n");
         #endif
         /* Unlock SPI Device*/
         spi_unlock_dev(&spi_dev);
         ReturnCode = -2; //FSS not responding
      }
   }
   else
   {
      #ifdef DEBUG_FSS
      printf("SOL_LibReadWFSS: Cannot Lock SPI\n");
      #endif
      ReturnCode = -3;
   }
   return(ReturnCode);
}
