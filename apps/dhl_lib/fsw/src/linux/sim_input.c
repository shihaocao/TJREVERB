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

/*
** Socket related includes
*/
#include <errno.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/

#define UART_TASK_ID         1
#define UART_TASK_STACK_SIZE 32768 
#define UART_TASK_PRIORITY   101

/* Define for UART/stdio input from 42 */
#undef FORTYTWO_CONNECTION_UART 

/*************************************************************************
** Private Function Prototypes
*************************************************************************/

/*************************************************************************
** Global data and functions 
*************************************************************************/
uint32             SIM_UartTaskId;
int                FswSocket = -1;
FILE              *FswStream;

extern SIM_Data_t  SIM_Data;


/*
** Init simulation data
**  This is now separate between the Linux and Nanomind versions
*/
void SIM_InitData(void)
{
   /*
   ** Defaulting to Simulator on
   */
   SIM_Data.StateData.GpsState = 1;
   SIM_Data.StateData.CssState = 1;
   SIM_Data.StateData.FssState = 1;
   SIM_Data.StateData.NmmdState = 1;
   SIM_Data.StateData.GyroState = 1;
   SIM_Data.StateData.ActState = 1;

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


#ifdef FORTYTWO_CONNECTION_UART 
void SIM_UartTask(void)
{
    uint32 status;
    char   input_string[256];

    OS_TaskRegister();

    OS_printf("DHL - Starting UART receive child task\n");

    while(1)
    {
       memset(input_string, 0, 256);
       fgets(input_string,256,stdin);
       if ( strlen(input_string) > 2 )
       {
          #if 0
          OS_printf("SIM - Input received: %s\n",input_string);
          #endif
          SIM_ProcessInput(input_string);
       }
    }
}
#else
int InitSocketClient(const char *hostname, int Port, int AllowBlocking)
{
      int sockfd,flags;
      struct sockaddr_in Server;
      struct hostent *Host;

      sockfd = socket(AF_INET,SOCK_STREAM,0);
      if (sockfd < 0) {
         printf("Error opening client socket.\n");
         return(-1);
      }
      Host = gethostbyname(hostname);

      if (Host == NULL) {
         printf("Server not found by client socket.\n");
         return(-1);
      }
      memset((char *) &Server,0,sizeof(Server));
      Server.sin_family = AF_INET;
      memcpy((char *)&Server.sin_addr.s_addr,(char *)Host->h_addr,
         Host->h_length);
      Server.sin_port = htons(Port);

      printf("Client connecting to Server on Port %i\n",Port);
      if (connect(sockfd,(struct sockaddr *) &Server,sizeof(Server)) < 0) {
         printf("Error connecting client socket: %s.\n",strerror(errno));
         return(-1);
      }
      printf("DHL - Client side of socket established.\n");

      /* Keep read() from waiting for message to come */
      if (!AllowBlocking) {
         flags = fcntl(sockfd, F_GETFL, 0);
         fcntl(sockfd,F_SETFL, flags|O_NONBLOCK);
      }

      return(sockfd);
}

void SIM_UartTask(void)
{
    uint32 status;
    char   input_string[256];
    char   ack_string[32];

    OS_TaskRegister();

    OS_printf("DHL - Starting Socket receive child task\n");
    FswSocket = InitSocketClient("localhost", 42420, 1);
    
    if ( FswSocket == -1 ) 
    {
       for(;;)
          sleep(100);
    }
    FswStream = fdopen(FswSocket,"r+");

    while(1)
    {
       memset(input_string, 0, 256);
       fgets(input_string,256,FswStream);

       /*
       ** Need to ignore the Ack message from 42
       */
       if ( input_string[0] == '4' && input_string[1] == '2' )
       {
          /* Need to send an Ack to 42 */
          strncpy(ack_string,"Ack", 32);
          status = send(FswSocket,ack_string,32,0);
          #if 0
          printf("DHL - Input received from Socket: %s\n",input_string);
          #endif
          SIM_ProcessInput(input_string);
       }
    }
}

#endif

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

    OS_printf ("DHL - Setting up Serial input handler\n");
    status = OS_TaskCreate( &SIM_UartTaskId, "UART_RCV", SIM_UartTask, 0, UART_TASK_STACK_SIZE, UART_TASK_PRIORITY, 0);
    if ( status != OS_SUCCESS )
    {
        OS_printf("DHL - Error creating UART Receive Task\n");
    }

    return(status);

}/* End SIM_LibInit */


/*
** Send outout to the simulator
*/
void SIM_SendCommand(char *CommandString)
{
    int Status;
#ifdef FORTYTWO_CONNECTION_UART 
    /*
    ** for now, send it to the serial port.
    ** This could end up being a network packet on some platforms
    */
    OS_printf("%s", CommandString);
#else

    if ( FswSocket != -1 ) 
    {
       #if 0
          printf("DHL - Sending to 42 Socket: %s",CommandString);
       #endif
       Status = send(FswSocket,CommandString,strlen(CommandString),0);
    }
    else
    {
       OS_printf("DHL - Send to 42 - but Socket not ready\n");
    }

#endif

}

/************************/
/*  End of File Comment */
/************************/
