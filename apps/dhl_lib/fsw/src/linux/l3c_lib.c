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

#undef L3C_ENABLE_SOCKETS
#define L3C_ENABLE_SOCKETS 1 

/*************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "dhl_lib.h"

#include "cadet_radio.h" /* Need to include from the cadet app.. */

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <errno.h>

#include "network_includes.h"

/*
** Macro Definitions
*/


#define L3C_TLM_ADDR "127.0.0.1"
#define L3C_TLM_PORT  1235
#define L3C_CMD_PORT  1234

#define L3C_TASK_STACK_SIZE 4096
#define L3C_TASK_PRIORITY   102

/*
** Typedefs
*/

typedef struct
{
   CadetMessageHeader_t  CadetHeader;
   uint8                 CommandBuffer[256];
} CadetRelayCmd_t; 

typedef struct
{
   CadetMessageHeader_t  CadetHeader;
   CadetStatusPayload_t  CadetData;
} CadetIsStatus_t;

/*
**  Function Prototypes
*/


/*
** External global memory
*/  

/*
** Global memory
*/
int                L3CTlmSocketId;
int                L3CTlmSocketEnabled = 0;

int                L3CCmdSocketId;
int                L3CCmdSocketEnabled = 0;

L3C_callback_t     L3CCallbackFunction = NULL;
uint32             L3CCmdReceiveTaskId;

CadetRelayCmd_t    CadetRelayCmd;

FILE *outfile = NULL;
int outfile_valid = FALSE;

/*
** This is the L3C/Cadet command receive task. 
**  It will wait on a UDP socket, then 
**  call the callback function to deliver the 
**  command uplink data
*/ 
void L3C_CmdReceiveTask(void)
{
    int                SocketConnected = FALSE;
    struct sockaddr_in SocketAddress;
    socklen_t          addr_len;
    int                status;

    OS_TaskRegister();
    OS_printf("L3C Lib: Starting Command receive child task\n");

    bzero((char *) &SocketAddress, sizeof(SocketAddress));
    SocketAddress.sin_family      = AF_INET;
    SocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    SocketAddress.sin_port        = htons(L3C_CMD_PORT);

    if ( (bind(L3CCmdSocketId, (struct sockaddr *) &SocketAddress, sizeof(SocketAddress)) < 0) )
    {
       printf("L3C - Bind call failed for Command input socket\n");
    }
    else
    {
        SocketConnected = TRUE;
    }

    while (1)
    {
       /*
       ** Read the socket
       */
       if ( SocketConnected == TRUE )
       {
          addr_len = sizeof(SocketAddress);
          bzero((char *) &SocketAddress, sizeof(SocketAddress));
          status = recvfrom(L3CCmdSocketId, (char *)&(CadetRelayCmd.CommandBuffer[0]), 256, 0,
                         (struct sockaddr *) &SocketAddress, &addr_len);

          printf("L3C - Command Input - received command over socket\n");

          /*
          ** Process the command 
          ** Need to add the Cadet RELAY command header then
          ** send the data to the callback 
          */
          if ( status > 0 ) 
          {
             CadetRelayCmd.CadetHeader.Prefix = 0x0AAF;
             CadetRelayCmd.CadetHeader.Destination = CADET_PKT_ADDRESS_OBC;
             CadetRelayCmd.CadetHeader.Source = CADET_PKT_ADDRESS_GROUND;
             CadetRelayCmd.CadetHeader.MessageId = CADET_PKT_MSGID_RELAY_DATA;
             CadetRelayCmd.CadetHeader.PayloadLength = status;
             CadetRelayCmd.CadetHeader.HeaderCRC = 0;

             CCSDS_LoadCheckSum ((CCSDS_CmdPkt_t *)&(CadetRelayCmd.CommandBuffer[0]));

             /* Change to if 1 to dump the packet being sent to the Cadet app */
             #if 0  
             {
                int     NumWords;
                uint16 *WordPtr;
                int     i;

                /* Dump packet */
                NumWords = (status + 16 )/2; /* Number of words for Relay command packet */
                WordPtr = (uint16 *)&CadetRelayCmd;

                printf("Dumping Relay + Command Packet\n");
                printf("------------------------------\n");
                for ( i = 0; i < NumWords; i++ )
                {
                   if ( i == 0 ) 
                      printf("CADET Relay Command:\n");
                   else if ( i == 8 ) 
                      printf("CCSDS FSW Command Packet:\n");

                   printf("  -->Word %02d: 0x%04X\n", i, WordPtr[i]);             
                }
                printf("------------------------------\n");
             }
             #endif

             /*
             ** Now call the callback function
             */
             if ( L3CCallbackFunction != NULL )
             {
                L3CCallbackFunction((uint8 *)&(CadetRelayCmd), sizeof(CadetMessageHeader_t) + status, 0);
             }

          }
       }
       else
       {
          /*
          ** Instead of doing this, exit the child task
          */
          OS_TaskDelay(1000);
       }
   } /* End while */
}

/*
** L3C_ForwardTelemetry
**  
** This is a helper function that forwards telemetry
** to a UDP socket to the ground system.
*/
void L3C_ForwardTelemetry(uint8 *Payload, uint32 PayloadSize)
{
    static struct sockaddr_in s_addr;
    int                       status;

    bzero((char *) &s_addr, sizeof(s_addr));
    s_addr.sin_family      = AF_INET;
    s_addr.sin_addr.s_addr = inet_addr(L3C_TLM_ADDR);
    s_addr.sin_port        = htons(L3C_TLM_PORT);
    status = 0;

    if (L3CTlmSocketEnabled == 1)
    {
        status = sendto(L3CTlmSocketId, (char *)Payload, PayloadSize, 0,
                                   (struct sockaddr *) &s_addr,
                                   sizeof(s_addr) );
    }
    if (status < 0)
    {
       L3CTlmSocketEnabled = 0;
    }

} 

/********************************************************************************
** Public API
*/

/*                                                                 
** L3C Library Initialization Routine                              
** cFE requires that a library have an initialization routine       
*/                                                                 
int32 L3C_LibInit(void)
{
    return OS_SUCCESS;
 
}/* End L3C_LibInit */

/*
** L3C_InstallCallback
**  This function should probably have the callback passed to it
*/
int32 L3C_InstallCallback (L3C_callback_t callback)
{
   OS_printf("L3C Lib: Install USART callback for Cadet uplink\n");
   L3CCallbackFunction = callback;
   return(0);
}

/*
** L3C_OpenSerialPort 
**
** This function will create the network sockets necessary for 
** simulating the Cadet I/O. It will also spawn the CADET receive
** task. 
*/
int32 L3C_OpenSerialPort(void)
{
   int status = 0;

#ifdef L3C_ENABLE_SOCKETS
   OS_printf("L3C Lib: Open Socket to telemetry link\n");
   if ( (L3CTlmSocketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
   {
      printf("Error creating socket for telemetry: errno = %d\n",errno);
      L3CTlmSocketEnabled = 0;
   }
   else
   {
      L3CTlmSocketEnabled = 1;
   }

   OS_printf("L3C Lib: Open Socket to command link\n");
   if ( (L3CCmdSocketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
   {
      printf("Error creating socket for Commands: errno = %d\n",errno);
      L3CCmdSocketEnabled = 0;
   }
   else
   {
      L3CCmdSocketEnabled = 1;
      OS_printf ("L3C Lib: Starting Command Receive Task\n");
      status = OS_TaskCreate( &L3CCmdReceiveTaskId, "CMD_RCV", L3C_CmdReceiveTask, 0, L3C_TASK_STACK_SIZE, L3C_TASK_PRIORITY, 0);
      if ( status != OS_SUCCESS )
      {
         OS_printf("L3C Lib - Error creating L3C Command Receive Task\n");
      }
   }

#ifdef _CADET_DEBUG_
   /* Wipe file open truncate/create and then close */
   if ( ( outfile = fopen ( "logging.out", "wb" ) ) != NULL )
   {
      fclose ( outfile );
   }
#endif
#endif

   return(status);
}

/*
** L3C_SendMessageToRadio
** 
** This function will send a message to the Cadet Radio over the socket if 
** properly set up. 
**
*/
int32 L3C_SendMessageToRadio(uint8 *MsgHdr,  uint32 HdrSize, 
                             uint8 *Payload, uint32 PayloadSize)
{
   CadetMessageHeader_t *CadetHeader;
   int i;
   int SendIsStatus = FALSE;
   CadetIsStatus_t CadetIsStatus;
   int FileType;

#ifdef L3C_ENABLE_SOCKETS
   /*
   ** Need to expand this.
   **
   **  Need to : check for different message types before just sending the payload.
   **   
   **   If it is a FIFO HIGH, send it
   **   FIFO LOW, send it
   **   When the packet is sent, we need to send feedback back to the cadet app. So we need to 
   **   keep the state of all HW even if it is fake.
   ** 
   **   May want to make this a standalone simulator.. This way it could be a PyQT program that 
   **   could keep stats, fifos etc. Could even use Zero MQ to keep fifos
   ** 
   ** PyQT / ZeroMQ Cadet Simulator! 
   */

   CadetHeader = (CadetMessageHeader_t *)MsgHdr;

   if ( ( CadetHeader->MessageId == CADET_PKT_MSGID_FIFO_ADD_HIGH ) || ( CadetHeader->MessageId == ( CADET_PKT_MSGID_FIFO_ADD_HIGH | 0x8000 ) ) )
   {
      #if 0
      printf("CADET-->FIFO_ADD_HIGH message received, forwarding packet\n"); 
      #endif

      if ( PayloadSize > 0 )
      {
         #if 0
         printf("L3C->CADET: Forwarding telemetry packet to network socket\n");
         #endif 
         L3C_ForwardTelemetry(Payload, PayloadSize); 
      }

      FileType = 0;
      if ( ( CadetHeader->MessageId & 0x8000 ) == 0x8000 )
      {
          SendIsStatus = FALSE;
      }
      else
      {
          SendIsStatus = TRUE;
      }

      /*
      ** Send feedback back to OBC/Cadet app
      */
   }
   else if ( ( CadetHeader->MessageId == CADET_PKT_MSGID_FIFO_ADD_LOW ) || ( CadetHeader->MessageId == ( CADET_PKT_MSGID_FIFO_ADD_LOW | 0x8000 ) ) )
   {
      #if 0
      printf("CADET-->FIFO_ADD_LOW message received, forwarding packet\n"); 
      #endif

      if ( PayloadSize > 0 )
      {
         #if 0
         printf("L3C->CADET: Forwarding telemetry packet to network socket\n");
         #endif
         L3C_ForwardTelemetry(Payload, PayloadSize); 
      }

      FileType = 1;
      if ( ( CadetHeader->MessageId & 0x8000 ) == 0x8000 )
      {
          SendIsStatus = FALSE;
      }
      else
      {
          SendIsStatus = TRUE;
      }

      /*
      ** Send feedback back to OBC/Cadet app
      */
   }
   else
   {
      printf("CADET--> MESSAGE Received, ID = %d\n",CadetHeader->MessageId);
      FileType = 2;
   }

#ifdef _CADET_DEBUG_
   /* Log data for post run analysis.  Must do append as system won't flush data to file until properly closed, so close every cycle */
   if ( ( outfile = fopen ( "logging.out", "a" ) ) != NULL )
   {
      if ( FileType == 0 )  fprintf ( outfile, "HI :" );
      if ( FileType == 1 )  fprintf ( outfile, "LOW:" );
      if ( FileType == 2 )  fprintf ( outfile, "OTH:" );
      for ( i = 0; i < HdrSize; i++ ) fprintf ( outfile, "%02X", MsgHdr [i] );
      fprintf ( outfile, " " );
      for ( i = 0; i < PayloadSize; i++ ) fprintf ( outfile, "%02X", Payload [i] );
      fprintf ( outfile, "\n" );
      fclose ( outfile );
   }
#endif

   if ( SendIsStatus == TRUE )
   {
      CadetIsStatus.CadetHeader.Prefix = 0x0AAF;
      CadetIsStatus.CadetHeader.Destination = CADET_PKT_ADDRESS_OBC;
      CadetIsStatus.CadetHeader.Source = CADET_PKT_ADDRESS_CADET;
      CadetIsStatus.CadetHeader.MessageId = CADET_PKT_MSGID_IS_STATUS;
      CadetIsStatus.CadetHeader.PayloadLength = sizeof ( CadetStatusPayload_t );
      CadetIsStatus.CadetHeader.HeaderCRC = 0;
      L3CCallbackFunction ( (uint8 *) &(CadetIsStatus), sizeof(CadetIsStatus_t), 0);
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

/************************/
/*  End of File Comment */
/************************/
