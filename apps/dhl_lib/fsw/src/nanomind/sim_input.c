/*************************************************************************
** File:
**   $Id: sim_input.c  $
**
** Purpose: 
**   Serial callback and setup 
**
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"
#include "dhl_lib_version.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <util/console.h>
#include <util/error.h>
#include <util/test_malloc.h>
#include <util/crc32.h>

#include <io/nanomind.h>
#include <dev/usart.h>


/*
** States
*/ 
#define SEARCHING 0
#define HEADER    1
#define BODY      2
#define FOOTER    3

/*
** Buffers
*/
char HeaderBuffer[9] = "42START[";
char FooterBuffer[7] = "]42END";
char CmdBuffer[256];

/*
** Buffer index vars
*/
int HeaderIndex = 0;
int FooterIndex = 0;
int CmdIndex = 0;

/*
** State
*/
int State = SEARCHING;

/*
** Global Simulation data
*/
extern SIM_Data_t  SIM_Data;


/*
** Private Function Prototypes
*/
void SIM_UsartCallback (uint8 * buf, int len, void * pxTaskWoken);

/*
** Init simulation data
**  This is now separate between the Linux and Nanomind versions
*/
void SIM_InitData(void)
{
   /*
   ** Defaulting to Simulator off 
   */
   SIM_Data.StateData.GpsState = 0;
   SIM_Data.StateData.CssState = 0;
   SIM_Data.StateData.FssState = 0;
   SIM_Data.StateData.NmmdState = 0;
   SIM_Data.StateData.GyroState = 0;
   SIM_Data.StateData.ActState = 0;

   SIM_Data.GpsData.Latitude = 0.0;
   SIM_Data.GpsData.Longitude = 0.0;
   SIM_Data.GpsData.Altitude = 0.0;
   SIM_Data.GpsData.XPosition = -1815954.3505;
   SIM_Data.GpsData.YPosition = -5900884.6173;
   SIM_Data.GpsData.ZPosition = 2797341.6258;
   SIM_Data.GpsData.XVelocity = 6887.3644;
   SIM_Data.GpsData.YVelocity = -1253.3371;
   SIM_Data.GpsData.ZVelocity = 1827.2140;
   SIM_Data.GpsData.Week = 1836;
   SIM_Data.GpsData.Seconds = 572400;
   SIM_Data.GpsData.NumSats = 8;

   SIM_Data.FssData[0].X = 1.0;
   SIM_Data.FssData[0].Y = 0.0;
   SIM_Data.FssData[0].Z = 0.0;
   SIM_Data.FssData[0].Valid = 0;
   SIM_Data.FssData[1].X = 0.0;
   SIM_Data.FssData[1].Y = 0.0;
   SIM_Data.FssData[1].Z = 0.0;
   SIM_Data.FssData[1].Valid = 0;
   SIM_Data.FssData[2].X = 0.0;
   SIM_Data.FssData[2].Y = 0.0;
   SIM_Data.FssData[2].Z = 0.0;
   SIM_Data.FssData[2].Valid = 0;

   SIM_Data.CssData.XPlus = 0;
   SIM_Data.CssData.XMinus = 0;
   SIM_Data.CssData.YPlus = 0;
   SIM_Data.CssData.YPlus = 0;
   SIM_Data.CssData.ZMinus = 0;
   SIM_Data.CssData.ZMinus = 0;

   SIM_Data.NmmdData.X = 0.0;
   SIM_Data.NmmdData.Y = 0.0;
   SIM_Data.NmmdData.Z = 0.0;

   SIM_Data.GyroData.XR = 1.0;
   SIM_Data.GyroData.YR = 2.0;
   SIM_Data.GyroData.ZR = 3.0;

   SIM_Data.WheelData.X = 0.0;
   SIM_Data.WheelData.Y = 0.0;
   SIM_Data.WheelData.Z = 0.0;

   SIM_Data.QBNData.Q1 = 0.0;
   SIM_Data.QBNData.Q2 = 0.0;
   SIM_Data.QBNData.Q3 = 0.0;
   SIM_Data.QBNData.Q4 = 0.0;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SIM Library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SIM_LibInit(void)
{
    int32 status = 0;

    /*
    ** Initialize the global data strucutres
    */
    SIM_InitData();

    /*
    ** Setup the USART callback function
    */
    #if 1
    OS_printf ("DHL - Setting up Serial input handler\n");
    usart_set_callback(0, SIM_UsartCallback);
    #endif

    return(status);

}/* End SIM_LibInit */

/*
** Send outout to the simulator
*/
void SIM_SendCommand(char *CommandString)
{
    /*
    ** Send the packet to the console. 
    ** Do not use OS_printf, we don't want to turn this off. 
    */
    printf("%s", CommandString);
}

void ResetStateMachine(void)
{
   HeaderIndex = 0;
   FooterIndex = 0;
   CmdIndex = 0;
   State = SEARCHING;
   memset(CmdBuffer,0,128);
}

/*
** USART callback function
** Provides a state machine to process the response from the CADET radio.
*/
void SIM_UsartCallback ( uint8 *buf, int len, void *pxTaskWoken)
{
   int i;

   for ( i = 0; i < len; i++ ) 
   {
      if (buf[i] != '\0' ) 
      {
         if ( State == SEARCHING )
         {
            /* Look for start of header */
            if ( buf[i] == HeaderBuffer[HeaderIndex])
            {
               #if 0
                  printf("Transition to HEADER State!\n");
               #endif
               State = HEADER;
               CmdBuffer[CmdIndex] = buf[i];
               CmdIndex++;
               HeaderIndex++;
            }
         }
         else if ( State == HEADER )
         {
            #if 0
               printf("Debug: buf[i] = %c, HeaderBuffer[HeaderIndex] = %c\n",
                       buf[i], HeaderBuffer[HeaderIndex]);
            #endif
            if ( buf[i] == HeaderBuffer[HeaderIndex] )
            {
               CmdBuffer[CmdIndex] = buf[i];
               CmdIndex++;
               if ( HeaderBuffer[HeaderIndex] == '[' )
               {
                  #if 0
                     printf("Transition to BODY State!\n");
                  #endif
                  State = BODY;
                  HeaderIndex = 0;
               }
               else
               {
                  HeaderIndex++;
               } 
            }
            else
            {
               ResetStateMachine();
               #if 0
                  printf("Resetting State machine in Header search\n");
               #endif
            }
         }
         else if ( State == BODY )
         {
            if ( buf[i] == FooterBuffer[FooterIndex] )
            {  
               #if 0
                  printf("Found Start of Footer!\n");
               #endif
               State = FOOTER;
               FooterIndex++;
            }
            CmdBuffer[CmdIndex] = buf[i];
            CmdIndex++;
         }
         else if ( State == FOOTER )
         {
            #if 0
               printf("Debug: buf[i] = %c, FooterBuffer[FooterIndex] = %c\n",
                       buf[i], FooterBuffer[FooterIndex]);
            #endif
            if ( buf[i] == FooterBuffer[FooterIndex] )
            {  
               if ( FooterBuffer[FooterIndex] == 'D' )
               { 
                  /* Process packet */
                  CmdBuffer[CmdIndex] = buf[i];
                  CmdIndex++;
                  CmdBuffer[CmdIndex] = '\n';
                  CmdIndex++;
                  CmdBuffer[CmdIndex] = '\0';
                  SIM_ProcessInput((char *)CmdBuffer);
                  ResetStateMachine();
               }
               else
               {
                  CmdBuffer[CmdIndex] = buf[i];
                  CmdIndex++;
                  FooterIndex++;
               }
            }
            else
            {
               ResetStateMachine();
               #if 0
                  printf("Resetting State machine in Footer search\n");
               #endif
            }
         }
         else
         {
            #if 0
               printf("Illegal state detected!\n");
            #endif
            ResetStateMachine();
         } 
      } /* end if not null */
   } /* end for */     
   #if 0
      printf("Processed all characters in input buffer\n");
   #endif
} 
