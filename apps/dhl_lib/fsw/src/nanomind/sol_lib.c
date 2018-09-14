/*************************************************************************
** File:
**   $Id: sol_lib.c  $
**
** Purpose: 
**   Shared library functions for the Solar Panel Subsystem used on Dellngr
**
**   $Log: sol_lib.c  $
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"

#include <stdint.h>
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <dev/spi.h>
#include <dev/pwm.h>
#include <dev/adc.h>


/*************************************************************************
** Macro Definitions
*************************************************************************/

//#define DEBUG_FSS 1

/*************************************************************************
** Private Function Prototypes
*************************************************************************/

int WFSS_Initchip(void);

/*******************************************************************/
/* Library init function                                           */
/*******************************************************************/

int32 SOL_LibInit(void)
{
    int reswff;
    reswff=WFSS_Initchip();
    return (reswff);
 
}/* End SOL_LibInit */



/*
** Need to figure out how this works 
** with individual devices(WFSS/GFSS)
** Do they all need to be setup first?
*/
int  SOL_LibSetupFSS(int FssDevice)
{

   

   return(0);
}

/*******************************************************************/
/* Commands going to the subsystem                                 */
/*******************************************************************/
int32 SOL_LibEnablePWM(int32 Channel, int32 Frequency, int32 DutyCycle, uint32 Direction)
{
   unsigned int duty16bit;
   uint8    dir;

   if (Channel > 2)
   {
      return (-1);
   }

   if ( Direction == 0 ) 
   {
      dir = 0;
   }
   else 
   {
      dir = 1;
   }

   if (DutyCycle == 0)
   {
       duty16bit = 0;
   }
   else
   {
       duty16bit = 65535 * (DutyCycle / 100.0);
   }

   pwm_init();
   pwm_disable(Channel);
   pwm_set_freq(Channel, 100, Frequency);
   pwm_set_dir(Channel, dir);
   pwm_enable(Channel);
   pwm_set_duty(Channel, duty16bit);

   return(0);
}


void  SOL_LibDisablePWM(int Channel)
{
   if ( Channel < 3 )
   {
      pwm_disable(Channel);
   }
}

/*
** MT command  
*/
int32 SOL_LibCommandMT(float x, float y, float z)
{
    char CommandString[128];

   /*
   ** Command 42 or the real MT hardware 
   */
   if ( SIM_Data.StateData.ActState == 1 )
   {
      sprintf(CommandString, DHL_MTC_CMD_STRING, x, y, z);
      SIM_SendCommand(CommandString);
      return(0);
   }
   else
   {

      #if 0  
      /* 
      ** PWM code not activated  
      */
      unsigned int dutycycle = 50;
      unsigned int dirx, diry, dirz;

      dirx = (x > 0)?  1 : 0;
      diry = (y > 0)?  1 : 0;
      dirz = (z > 0)?  1 : 0;
      SOL_LibEnablePWM(0, 100, abs(x)*dutycycle, dirx);
      SOL_LibEnablePWM(1, 100, abs(y)*dutycycle, diry);
      SOL_LibEnablePWM(2, 100, abs(z)*dutycycle, dirz);

      #endif

      return(0);
   }
}


int SOL_LibReadFSS(int FssDevice, int *Valid, float *X, float *Y, float *Z)
{
   int         ReturnCode = 0;
   WFSS_data_t WFssData;
   GFSS_data_t GFssData;

   /*
   ** if the simulated data is requested, return it
   */
   if ( SIM_Data.StateData.FssState == 1 )
   {
      if ( FssDevice == 0 )
      {
         *Valid = SIM_Data.FssData[0].Valid;
         *X = SIM_Data.FssData[0].X;
         *Y = SIM_Data.FssData[0].Y;
         *Z = SIM_Data.FssData[0].Z;
          ReturnCode = 0;
      }
      else if ( FssDevice == 1 )
      {
         *Valid = SIM_Data.FssData[1].Valid;
         *X = SIM_Data.FssData[1].X;
         *Y = SIM_Data.FssData[1].Y;
         *Z = SIM_Data.FssData[1].Z;
          ReturnCode = 0;
      }
      else if ( FssDevice == 2 )
      {
         *Valid = SIM_Data.FssData[2].Valid;
         *X = SIM_Data.FssData[2].X;
         *Y = SIM_Data.FssData[2].Y;
         *Z = SIM_Data.FssData[2].Z;
          ReturnCode = 0;
      }
      else
      {
         ReturnCode = -1;
      }

   }
   else
   {

      /* 
      ** Get the data from the WFSS or GFSS 
      */
      if ( FssDevice == 0 )
      {
         ReturnCode = SOL_LibReadWFSS(&WFssData);
         if ( ReturnCode == 0 )
         {
            *X = WFssData.Unit_Vector_X;
            *Y = WFssData.Unit_Vector_Y;
            *Z = WFssData.Unit_Vector_Z;
            *Valid = WFssData.FOV && WFssData.InRange; 
             //printf("UnitVec, Val = %f %f %f %d\n", WFssData.Unit_Vector_X,  WFssData.Unit_Vector_Y,  WFssData.Unit_Vector_Z, *Valid);
         } 
         else
         {
            //#ifdef DEBUG_FSS
            printf("SOL_LibReadFSS: SOL_LibReadWFSS returned an error: %d\n",ReturnCode);
            //#endif
         }
      }
      else if ( FssDevice == 1 || FssDevice == 2 ) 
      {
         ReturnCode = SOL_LibReadGFSS((FssDevice - 1), &GFssData);
         if ( ReturnCode == 0 )
         {
            *X = GFssData.Unit_Vector_X;
            *Y = GFssData.Unit_Vector_Y;
            *Z = GFssData.Unit_Vector_Z;
            *Valid = GFssData.FSS_Valid; 
         } 
         else
         {
            #ifdef DEBUG_FSS
            printf("SOL_LibReadFSS: SOL_LibReadGFSS returned an error: %d\n",ReturnCode);
            #endif
         }

      }
      else
      {
         #ifdef DEBUG_FSS
         printf("SOL_LibReadFSS: Invalid FSS Device:  %d\n",FssDevice);
         #endif
         ReturnCode = -1;
      }

   }

   return(ReturnCode);

}

uint16 NM_Temp0 = 0;
uint16 NM_Temp1 = 0;

void SOL_GetTemps(uint16 *temp_0, uint16 *temp_1) {
   *temp_0 = NM_Temp0;
   *temp_1 = NM_Temp1;

   return;
}

int SOL_LibReadCSS(uint16 *XPlus, uint16 *YPlus, uint16 *ZPlus, uint16 *XMinus, uint16 *YMinus, uint16 *ZMinus)
{

   uint16_t * adc_buffer;

   /*
   ** Check for null params
   */
   if ( XPlus == NULL || YPlus == NULL || ZPlus == NULL || XMinus == NULL || YMinus == NULL || ZMinus == NULL )
   {
      return (-1);
   }
   
   /*
   ** if the simulated data is requested, return it
   */
   if ( SIM_Data.StateData.FssState == 1 )
   {
      *XPlus  = SIM_Data.CssData.XPlus;
      *XMinus = SIM_Data.CssData.XMinus;

      *YPlus  = SIM_Data.CssData.YPlus;
      *YMinus = SIM_Data.CssData.YMinus;

      *ZPlus  = SIM_Data.CssData.ZPlus;
      *ZMinus = SIM_Data.CssData.ZMinus;

      return(0);
   }
   else
   {
      adc_buffer = adc_start_blocking(1);
      if (adc_buffer == NULL) {
         printf("ERROR: adc_start_blocking timed out\n");
      } else {
         /* 
	 ** Save the temperature data for SHK
         ** This way SHK won't interfer with ACS, but ACS must always run.
         */
         NM_Temp0 = adc_buffer[0];
         NM_Temp1 = adc_buffer[1];

         /*
         ** Read the real CSS data
         */
         *XPlus  = adc_buffer[2];
         *ZPlus  = adc_buffer[3];
         *YPlus  = adc_buffer[4];
         *XMinus = adc_buffer[5];
         *ZMinus = adc_buffer[6];
         *YMinus = adc_buffer[7];

      }

      return(0);
   }
}

/************************/
/*  End of File Comment */
/************************/
