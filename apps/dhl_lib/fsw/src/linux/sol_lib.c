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


/*
** Is this the only SPI dev?
*/

/*************************************************************************
** Macro Definitions
*************************************************************************/


/*************************************************************************
** Private Function Prototypes
*************************************************************************/

/*******************************************************************/
/* Library init function                                           */
/*******************************************************************/

int32 SOL_LibInit(void)
{
    return OS_SUCCESS;
 
}/* End SOL_LibInit */

/*******************************************************************/
/* Commands going to the subsystem                                 */
/*******************************************************************/

int32 SOL_LibEnablePWM(int32 Channel, int32 Frequency, int32 DutyCycle, uint32 Direction)
{
   if (Channel > 2)
   {
      return (-1);
   }
   return(0);
}
void  SOL_LibDisablePWM(int Channel)
{
   return;
}



/*
** MT command
*/
int32 SOL_LibCommandMT(float x, float y, float z)
{
    char CommandString[128];

    sprintf(CommandString, DHL_MTC_CMD_STRING, x, y, z);
    SIM_SendCommand(CommandString);
    return(0);
}

int SOL_LibSetupFSS(int FssDevice) 
{

   return 0;
}

int SOL_LibReadFSS(int FssDevice, int *Valid, float *X, float *Y, float *Z)
{
   int      ReturnCode = 0;

   /*
   ** Add a check for null params
   */


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

   return(ReturnCode);
}


void SOL_GetTemps(uint16 *temp_0, uint16 *temp_1) {
   // Not simulated.
   *temp_0 = 42;
   *temp_1 = 42;
}

int SOL_LibReadCSS(uint16 *XPlus, uint16 *YPlus, uint16 *ZPlus, uint16 *XMinus, uint16 *YMinus, uint16 *ZMinus)
{
   /*
   ** Add a check for null params
   */
   
   *XPlus  = SIM_Data.CssData.XPlus;
   *XMinus = SIM_Data.CssData.XMinus;

   *YPlus  = SIM_Data.CssData.YPlus;
   *YMinus = SIM_Data.CssData.YMinus;

   *ZPlus  = SIM_Data.CssData.ZPlus;
   *ZMinus = SIM_Data.CssData.ZMinus;

   return(0);
} 

/************************/
/*  End of File Comment */
/************************/
