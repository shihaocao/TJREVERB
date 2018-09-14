/*************************************************************************
** File:
**   $Id: sim_lib.c  $
**
** Purpose: 
**    Sim State functions ( hopefully generic ) 
**
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/

#include "cfe.h"
#include "cfe_time.h"

#include "dhl_lib.h"
#include "dhl_lib_version.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

void NM_SetMET(unsigned long Seconds, unsigned long SubSeconds);

/*************************************************************************
** Macro Definitions
*************************************************************************/

#define SIM_LIB_ACT_ALL 1
#define SIM_LIB_ACT_MT  2
#define SIM_LIB_ACT_RW  4

/*************************************************************************
** Private Function Prototypes
*************************************************************************/

/*************************************************************************
** Global data and functions 
*************************************************************************/

/*
** Global Simulation data
*/
SIM_Data_t  SIM_Data;

void SIM_TrimSpaces (char *NewString, const char *OriginalString)
{
  while (*OriginalString != '\0')
  {
    if(!isspace(*OriginalString))
    {
      *NewString = *OriginalString;
      NewString++;
    }
    OriginalString++;
  }
  *NewString = '\0';
}

/*
** Process input from the simulator
*/
void SIM_ProcessInput(char *InputString)
{
    static char   CleanedString[256];
    static float  X, XV;
    static float  Y, YV;
    static float  Z, ZV;
    static float  Lat;
    static float  Long;
    static float  Alt;
    unsigned int  Week; 
    static float  Seconds;
    uint32        status;
    int           GpsState;
    int           CssState;
    int           FssState;
    int           NmmdState;
    int           ActState;
    int           FssId;
    float         FssX, FssY, FssZ;
    float         Q1, Q2, Q3, Q4;
    int           FssValid;
    int           XPlus, XMinus, YPlus, YMinus, ZPlus, ZMinus;

    CFE_TIME_SysTime_t   StcfTime;
    CFE_TIME_SysTime_t   MetTime;
    unsigned long        TaiSeconds;


    /*
    ** Remove spaces from the input string
    */
    memset(CleanedString, 0, 256);
    SIM_TrimSpaces(CleanedString, InputString);    

    /*
    ** Check for the SWITCH command
    */ 
    status = sscanf(CleanedString,DHL_SWITCH_CMD_STRING, &GpsState, &CssState, &FssState, &NmmdState, &ActState);  
    if ( status == 5 ) 
    {
       if ( GpsState == 0 || GpsState == 1 ) 
           SIM_Data.StateData.GpsState = GpsState;
       else
           OS_printf("DHL - SWITCH - Invalid GPS State: %d\n",GpsState);

       if ( CssState == 0 || CssState == 1 ) 
           SIM_Data.StateData.CssState = CssState;
       else
           OS_printf("DHL - SWITCH - Invalid CSS State: %d\n",CssState);
    
       if ( FssState == 0 || FssState == 1 ) 
           SIM_Data.StateData.FssState = FssState;
       else
           OS_printf("DHL - SWITCH - Invalid FSS State: %d\n",FssState);

       if ( NmmdState == 0 || NmmdState == 1 ) 
           SIM_Data.StateData.NmmdState = NmmdState;
       else
           OS_printf("DHL - SWITCH - Invalid NMMD State: %d\n",NmmdState);

       if ( ActState == 0 || ActState == 1 ) 
           SIM_Data.StateData.ActState = ActState;
       else
           OS_printf("DHL - SWITCH - Invalid ACT State: %d\n",ActState); 

       OS_printf("DHL - SWITCH command processed - GPS = %d, CSS = %d, DHL = %d, ACT = %d\n",
                         SIM_Data.StateData.GpsState, 
                         SIM_Data.StateData.CssState, 
                         SIM_Data.StateData.FssState, 
                         SIM_Data.StateData.ActState);
    }
    else
    {
       /*
       ** Check for the NMMD Sensor message 
       */ 
       status = sscanf(CleanedString, DHL_NMMD_SENSOR_STRING, &X, &Y, &Z);  
       if ( status == 3 ) 
       {
          SIM_Data.NmmdData.X = X;
          SIM_Data.NmmdData.Y = Y;
          SIM_Data.NmmdData.Z = Z;
  
          #if 0
             OS_printf("DHL - NMMD Sensor Data Processed: X:%f, Y:%f, Z:%f\n", X, Y, Z);
          #endif
       }
       else
       {
    
          /*
          ** Check for the GPS POS Sensor message 
          */ 
          status = sscanf(CleanedString, DHL_GPS_POS_SENSOR_STRING, &Lat, &Long, &Alt);  
          if ( status == 3 ) 
          {
             SIM_Data.GpsData.Latitude  = Lat;
             SIM_Data.GpsData.Longitude = Long;
             SIM_Data.GpsData.Altitude  = Alt;
             #if 0
                OS_printf("DHL - GPS Position Sensor Data Processed: Lat:%f, Long:%f, Alt:%f\n", Lat, Long, Alt);
             #endif
          }
          else
          {

             /*
             ** Check for the GPS XYZ Sensor message 
             */ 
             status = sscanf(CleanedString, DHL_GPS_XYZ_SENSOR_STRING, &X, &Y, &Z, &XV, &YV, &ZV, &Week, &Seconds);  
             if ( status == 8 ) 
             {
                SIM_Data.GpsData.XPosition = X;
                SIM_Data.GpsData.YPosition = Y;
                SIM_Data.GpsData.ZPosition = Z;
                SIM_Data.GpsData.XVelocity = XV;
                SIM_Data.GpsData.YVelocity = YV;
                SIM_Data.GpsData.ZVelocity = ZV;
                SIM_Data.GpsData.ZVelocity = ZV;
                SIM_Data.GpsData.Week = Week;
                SIM_Data.GpsData.Seconds = Seconds;

                #if 0
                   OS_printf("DHL - GPS XYZ Sensor Data Processed : XPos:%f, YPos:%f, ZPos:%f\n", X, Y, Z);
                   OS_printf("DHL -                                 XVel:%f, YVel:%f, ZVel:%f\n", XV, YV, ZV);
                   OS_printf("DHL -                                 Week:%u, Seconds:%f\n", Week, Seconds);
                #endif

                /*
                ** Figure out the TAI time based on the GPS time 
                */
                TaiSeconds = (Week * 604800) + Seconds + 19;
                #if 0
                	OS_printf("====> GPS (TAI) time = %d\n",TaiSeconds); 
                #endif
                MetTime  = CFE_TIME_GetMET();
 
                /*
                ** Calculate the new STCF ( seconds ) 
                **   STCFSeconds = TimeTAI - Virtual MET 
                */
                StcfTime.Seconds = TaiSeconds - MetTime.Seconds;
                StcfTime.Subseconds = 0;
                
                /*
                ** Set the SCTF
                */
                CFE_TIME_SetSTCF(StcfTime); 


             }
             else
             {
             
                /*
                ** Check for the FSS Sensor message 
                ** This will probably change with the addition of the Gomspace FSS
                */
                status = sscanf(CleanedString, DHL_FSS_SENSOR_STRING, &FssId, &FssX, &FssY, &FssZ, &FssValid);  
                if ( status == 5 ) 
                {
                   if ( FssId == 1 || FssId == 2 || FssId == 3 )
                   {
                      SIM_Data.FssData[FssId-1].Valid = FssValid;
                      SIM_Data.FssData[FssId-1].X = FssX;
                      SIM_Data.FssData[FssId-1].Y = FssY;
                      SIM_Data.FssData[FssId-1].Z = FssZ;
                      #if 0
                         printf("DHL - FSS Data Processed: ID = %d, X = %f, Y = %f, Z = %f\n", FssId-1, FssX, FssY, FssZ);
                      #endif
                   }
                   else
                   {
                      printf("DHL - Invalid Fss ID: %d\n",FssId);
                   }
                }
                else
                {

                   /*
                   ** Check for the CSS Sensor message 
                   */
                   status = sscanf(CleanedString, DHL_CSS_SENSOR_STRING, &XPlus, &YPlus, &ZPlus, &XMinus, &YMinus, &ZMinus);  
                   if ( status == 6 ) 
                   {
                      #if 0 
                         printf("DHL - CSS Data Processed: X+ = %d, Y+ = %d, Z+ = %d, X- = %d, Y- = %d, Z- = %d\n", 
                              XPlus, YPlus, ZPlus, XMinus, YMinus, ZMinus);
                      #endif

                      SIM_Data.CssData.XPlus =  XPlus; 
                      SIM_Data.CssData.XMinus = XMinus; 
                      SIM_Data.CssData.YPlus =  YPlus; 
                      SIM_Data.CssData.YMinus = YMinus; 
                      SIM_Data.CssData.ZPlus =  ZPlus; 
                      SIM_Data.CssData.ZMinus = ZMinus; 
                   }
                   else
                   {
                      /*
                      ** Check for the Gyro Data
                      */
                      status = sscanf(CleanedString, DHL_GYRO_SENSOR_STRING, &X, &Y, &Z);  
                      
                      if ( status == 3 )
                      {
                         SIM_Data.GyroData.XR = X; 
                         SIM_Data.GyroData.YR = Y; 
                         SIM_Data.GyroData.ZR = Z; 
                         #if 0 
                            printf("DHL - GYRO Data Processed: X = %f, Y = %f, Z = %f\n", 
                              X, Y, Z);
                         #endif
                      }
                      else
                      {
                         /*
                         ** Check for Wheel speed data
                         */
                         status = sscanf(CleanedString, DHL_RWSPD_SENSOR_STRING, &X, &Y, &Z);  

                         if ( status == 3 ) 
                         {
                            SIM_Data.WheelData.X = X; 
                            SIM_Data.WheelData.Y = Y; 
                            SIM_Data.WheelData.Z = Z; 
                            #if 0 
                               printf("DHL - Wheel Data Processed: X = %f, Y = %f, Z = %f\n", 
                              X, Y, Z);
                            #endif
                         }
                         else
                         {
                            /*
                            ** Check for the QBN Sensor message 
                            */ 
                            status = sscanf(CleanedString, DHL_QBN_SENSOR_STRING, &Q1, &Q2, &Q3, &Q4);  
                            if ( status == 4 ) 
                            {
                               SIM_Data.QBNData.Q1 = Q1;
                               SIM_Data.QBNData.Q2 = Q2;
                               SIM_Data.QBNData.Q3 = Q3;
                               SIM_Data.QBNData.Q4 = Q4;
  
                               #if 0
                                  OS_printf("DHL - QBN Sensor Data Processed: Q1:%f, Q2:%f, Q3:%f, Q4:%f\n", 
                                             Q1, Q2, Q3, Q4);
                               #endif
                            }
       								else
		 								{
								       /*
								       ** Check for the DAGR Sensor message 
								       */ 
								       status = sscanf(CleanedString, DHL_DAGR_SENSOR_STRING, &X, &Y, &Z);  
								       if ( status == 3 ) 
										 {
								          SIM_Data.DagrData.X = X;
								          SIM_Data.DagrData.Y = Y;
								          SIM_Data.DagrData.Z = Z;
  
								          #if 0
								             OS_printf("DHL - DAGR Sensor Data Processed: X:%f, Y:%f, Z:%f\n", X, Y, Z);
								          #endif
										 }
                            	else
                            	{
                             	  printf("DHL - Unrecognized 42 Sim string.\n");
                             	  printf("DHL - String is: %s\n", InputString);
                              }
								    }
                         }
                      }
                   }
               }
            }
         }
      }
   }
}

/************************/
/*  End of File Comment */
/************************/
