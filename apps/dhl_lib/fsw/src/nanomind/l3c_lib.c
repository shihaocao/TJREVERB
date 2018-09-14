/*************************************************************************
** File:
**   $Id: l3c_lib.c  $
**
** Purpose: 
**   Shared library functions the L3 Cadet Radio 
**
**   $Log: l3c_lib.c  $
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "dhl_lib.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h>  /* UNIX standard function definitions */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <util/console.h>
#include <util/error.h>
#include <util/test_malloc.h>
#include <util/crc32.h>

#include <io/nanomind.h>
#include <dev/usart.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/


/*************************************************************************
** Private Function Prototypes
*************************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* L3C Library Initialization Routine                              */
/* cFE requires that a library have an initialization routine      */ 
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 L3C_LibInit(void)
{
    return OS_SUCCESS;
 
}/* End L3C_LibInit */

/************************/
/*  End of File Comment */
/************************/

/*
** Prototypes
*/
int  cts_get(void);

/*
** Defines
*/
/* #undef L3C_ENABLE_OUTPUT */
#define L3C_ENABLE_OUTPUT 1

/*
** External global memory
*/  
extern uint32_t          cpu_core_clk;

/*
** Global memory
*/

/*
** CADET_OpenSerialPort 
**
** Returns the file descriptor on success or -1 on error.
*/
int32 L3C_OpenSerialPort(void)
{
   OS_printf("L3C Lib: Open USART port 2\n");
   cts_init();
   usart_init(2, cpu_core_clk, 57600);
   return(0);
}

/*
** L3C_InstallCallback
**  This function should probably have the callback passed to it
*/
int32 L3C_InstallCallback (L3C_callback_t callback)
{
   OS_printf("L3C Lib: Install USART callback for Cadet\n");
   usart_set_callback(2, callback);
   return(0);
}

/*
** L3C_SendMessageToRadio
** 
** This function will send a message to the Cadet Radio over USART2
**
** Error handling??
*/
int32 L3C_SendMessageToRadio(uint8 *MsgHdr,  uint32 HdrSize, 
                             uint8 *Payload, uint32 PayloadSize)
{

#ifdef L3C_ENABLE_OUTPUT 

   /*
   ** Wait for the Cadet to set CTS
   */
   while(cts_get())
      OS_TaskDelay(10);

   /*
   ** Send the Header data
   */
   usart_putstr(2, (char *)MsgHdr, HdrSize);
   OS_TaskDelay ( 40 ); /* Takes 28 ms to send header to CADET, this waits at least 30 ms */

   /*
   ** If there is a payload, send it
   */
   if ( PayloadSize > 0 )
   {
      while(cts_get())
         OS_TaskDelay(10);

      usart_putstr(2, (char *)Payload, PayloadSize);

      /* usart_putstr(2, (char *)Payload, PayloadSize); */
   }
#endif

#if 0 
   /* Original Cadet UART output code */
   /* 
   ** Send the Header data 
   */
   usart_putstr2((char *)MsgHdr, HdrSize); 
   OS_TaskDelay ( 40 ); /* Takes 28 ms to send header to CADET, this waits at least 30 ms */

   /* 
   ** If there is a payload, send it
   */
   if ( PayloadSize > 0 )
   {
      usart_putstr2((char *)Payload, PayloadSize); 

//      printf("Msg sent to cadet\n");

      /* usart_putstr(2, (char *)Payload, PayloadSize); */
   }
#endif

   return(0);
}

/*
** L3C CRC32 functions
*/
uint32 L3C_Crc32_4byte(uint32 Crc, uint32 Data)
{
    int i;

    Crc = Crc ^ Data;

    for (i = 0; i < 32; i++)
    {
        if ((Crc & 0x80000000) == 0x80000000)
        {
            Crc = (Crc << 1);
            Crc = Crc ^ 0x04C11DB7;
        } /* Polynomial used in STM32 */
        else
        { 
            Crc = (Crc << 1); }
    }

    return (Crc);
}
        
uint32 L3C_crc32(uint8_t *block, unsigned int length) 
{
    uint32   crc = 0xFFFFFFFF;
    uint32   data;
    unsigned int i;
                   
    for (i = 0; i < length/4; i++)
    {
        data = (*((uint8_t *)block) << 24) | 
               (*((uint8_t *)block+1) << 16) | 
               (*((uint8_t *)block+2) << 8) | 
               (*((uint8_t *)block+3));
        block += 4;
        crc=L3C_Crc32_4byte(crc,data);
    }
    return (crc ^ 0xFFFFFFFF);
}

