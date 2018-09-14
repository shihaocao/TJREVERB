#include "dhl_lib.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <dev/i2c.h>
#include <dev/arm/at91sam7.h>
#include <util/error.h>
#include <util/csp_buffer.h>

#define GOM_FSS_ADD1      0x3A
#define GOM_FSS_ADD2      0x3B
#define GOM_FSS_ADDTEMP   0x05
#define GOM_FSS_DARKTH    25000
#define GOM_FSS_IDARKTH   3000
#define GOM_FSS_BRIGHTTH  25000

#define NON_FLIGHT_SIMULATOR 0

const uint8 fssaddr[2] = {GOM_FSS_ADD1,GOM_FSS_ADD2};

/* gom_fss_data_t gom_fss_data; */

const float gfss_param[2][4]=
   {  //[h x0 y0 rho]
      {0.648132, 0.050799, 0.043385, 0.0077854}, //uid : 1434021502
      {0.630176, -0.033350, 0.028859, 0.000000} //uid : 1434020430
   };

const float gfss_theta[2][81] = 
   { //[Theta error]
      { 0.0050469,  0.78347,  0.81541,  0.75778,  0.81471,  0.55938,  0.60705,  0.82424,  0.85129,  1.0352,
        1.1628,     1.3075,   1.1101,   0.79426,  1.0163,   0.9568,   1.3571,   0.65722,  0.62662,  0.97074,
        1.0824,     1.0866,   1.1061,   0.94146,  0.67326,  0.56093,  0.76575,  0.78701,  0.49573,  0.25141, 
        0.51195,    0.93774,  0.9678,  -0.74296, -0.60858, -0.20677,  0.073991, 0.12805, -0.061325,-0.23342,
       -0.040467,   0.18026,  0.083934,-0.28558, -0.67012, -0.73876, -0.58133, -0.20161,  0.06523, -0.60825,
       -1.334,     -1.6611,  -1.4078,  -0.8406,   0.053351,-0.088391,-0.27722, -3.5118,  -2.5728,  -1.4967,
       -0.93463,   -1.1362,  -2.0109,  -2.613,   -2.0679,  -1.3601,  -1.248,   -1.7576,  -3.003,   -3.7407,
       -2.7561,    -1.686,   -1.1123,  -1.788,   -2.9114,  -3.7399,  -3.0161,  -1.7872,  -1.3499,  -1.6134, -2.3042}, //uid : 1434021502

      { 0.010499,   0.92818,  0.89595,  0.67524,  0.66552,  0.37767,  0.575,    0.87279,  0.73275,  1.3945,
        1.5116,     1.0769,   0.77845,  0.57859,  0.9547,   0.92158,  1.211,    1.1095,   1.2852,   1.3369,
        0.97076,    0.58411,  0.60824,  0.59397,  0.36794,  0.15872,  0.44122,  0.61206,  0.39186,  0.19361,
        0.42122,    0.89237,  1.2144,  -0.46314, -0.10995,  0.2607,   0.39234,  0.066788,-0.57391, -1.0077,
       -0.76835,   -0.37759, -0.26015, -0.50701, -0.9503,  -1.0818,  -0.95111, -0.50594, -0.18757, -0.67287,
       -1.2392,    -1.484,   -1.3792,  -0.75671,  0.058744, 0.06151, -0.14505, -3.217,   -2.1955,  -1.3992,
       -1.0917,    -1.6916,  -2.9175,  -3.7992,  -3.231,   -2.2465,  -1.7713,  -2.2033,  -2.9001,  -3.4399,
       -2.7311,    -2.0525,  -1.378,   -1.7793,  -2.4736,  -2.901,   -2.5141,  -1.5813,  -1.3432,  -1.4979, -2.3314}}; //uid : 1434020430

const float gfss_phi[2][81]={ //[Phi error]
      { 0.20695,   -0.68951, -0.31469, -0.13289, -0.43272, -0.074894,-0.2394,   0.82068,  0.16593, -0.58271,
        0.4293,     0.19214, -0.83704, -0.12533,  0.15182,  0.49382, -0.17287, -0.71908,  0.33917,  0.58246,
        0.34356,    0.26127, -0.058758,-0.91589, -0.7663,   0.16685,  0.52411, -0.12104, -0.4471,   0.36394,
        0.57093,   -0.23479, -0.93658, -0.42949,  0.49358,  0.91143,  0.64171,  0.27003, -0.035701, 0.31849,
        0.60784,   -0.13143, -0.82643, -1.3972,  -1.0028,   0.30305,  1.1683,   1.2675,   0.088693,-1.2287,
       -1.0842,     0.29883,  1.4343,   0.92834, -0.26853, -1.0652,  -1.0967,  -0.82393,  2.4343,   2.6601,
        0.85031,   -1.3037,  -1.5331,   0.33354,  2.015,    1.1954,  -1.0831,  -2.7905,  -2.3692,   0.53152,
        3.5605,     2.8665,   0.38164, -2.6284,  -2.6495,   0.097853, 3.1443,   2.8417,   0.080168,-1.9996, -2.832}, //uid : 1434021502

      { 0.38994,    0.052257, 0.18322, -0.39957, -1.1787,   0.77009,  0.65807,  0.95026,  0.58625,  0.045335,
       -0.22171,   -0.19207, -0.94527,  0.50964,  0.35981, -0.017368, 0.31912, -0.032997, 0.20911, -0.34995,
       -0.59428,   -0.17569, -0.35376, -0.83984, -0.62844,  0.41717,  1.0052,   0.18503, -0.52062, -0.0635,
        0.35257,    0.1838,  -0.19878, -0.31368,  0.637,    0.48451, -0.53048, -0.84969, -0.99028, -0.012421,
        0.40405,    0.035493,-0.45506, -1.0991,  -0.92882,  0.33292,  1.4174,   1.6094,   0.3204,  -0.84857,
       -0.97914,   -0.059899, 0.88738,  0.79083, -0.047251,-0.80906, -1.026,   -0.62248,  1.9304,   1.6367, 
       -0.63856,   -2.594,   -2.2062,   0.10412,  1.5254,   1.3563,  -0.31487, -1.8393,  -1.7438,   0.34117,
        2.6291,     2.8429,   0.8925,  -1.5301,  -1.6602,  -0.21149,  1.622,    1.5854,  -0.41376, -2.5028, -2.7603}}; //uid : 1434020430


int SOL_LibSetupGFSS(int FssDevice)
{
   int retval = 0;
   return(retval);
}

int SOL_LibReadGFSS(int FssDevice, GFSS_data_t *GFssData)
{
   int      ReturnCode = 0;
  
   /*
   ** Check Parameters
   */ 
   if ( FssDevice < 0 || FssDevice > 1 )
   {
      return(-1);
   }
   if ( GFssData == 0 )
   {
      return(-2);
   }

   /*
   ** Sample the Sun Sensor
   */
   ReturnCode = SOL_LibGFSS_SampleSun(FssDevice);
   if ( ReturnCode == -1 )
   {
      return(-3);
   }

   /*
   ** Delay for 25 milliseconds
   */
   vTaskDelay(30); 
   
   /*
   ** Get the Sun Sensor Data
   */ 
   ReturnCode = SOL_LibGFSS_GetSun(FssDevice, GFssData);
   if ( ReturnCode == -1 ) 
   {
      return(-5);
   }

   /*
   ** Calculate Vectors
   */
   ReturnCode = SOL_LibGFSS_Calculate(FssDevice, GFssData);

   return(ReturnCode);
}

uint16   SOL_LibReadGFSSTemp(int FssDevice)
{
   int16 fss_temp = 0;
   uint8 TEMP_H = 0;
   uint8 TEMP_L = 0;

   uint8 addr;
   uint8 datatosend[1];
   uint8 datatorec[2];
   int8  ack = 0;


   if ( FssDevice < 0 || FssDevice > 1 )
   {
      return(fss_temp);
   }

   /* Convert device address ID to I2C address */
   addr = fssaddr[FssDevice];
   /*
   ** Step 1: Sample the temp
   */
   datatosend[0]=0x04;
   ack = i2c_master_transaction(0, addr, datatosend, 1, NULL, 0, 1000);
   if (ack != E_NO_ERR) {
      printf("GFSS SOL_LibReadGFSSTemp transaction 1 failed: %d\n", ack);
   }
   /*
   ** Delay for 25 milliseconds
   */
   vTaskDelay(25); 

   /*
   ** Step 2: Read the temp ( Delay needed between? ) 
   */
   datatosend[0]=0x05;
   ack = i2c_master_transaction(0, addr, datatosend, 1, datatorec, 2, 1000) ;
   if (ack != E_NO_ERR) {
      printf("GFSS SOL_LibReadGFSSTemp transaction 2 failed: %d\n", ack);
   }
  
   TEMP_H=datatorec[0];
   TEMP_L=datatorec[1];

   fss_temp=(((int16_t)((TEMP_H << 8) + TEMP_L)) >> 2 ) * 0.03125;

   return (fss_temp);
}

uint32   SOL_LibGetGFSSVersion(int FssDevice)
{
   uint32     fssver=0;
   int        retval = 0;
   uint8      addr;
   uint8      datatosend[1];
   uint8      datatorec[4];

   if ( FssDevice < 0 || FssDevice > 1 )
   {
      return(fssver);
   }
   
   /* Convert device address ID to I2C address */
   addr = fssaddr[FssDevice];
   datatosend[0]=0x08; // CMD_GET_VERSION command
   retval = i2c_master_transaction(0, addr, datatosend, 1, datatorec, 4, 1000);

   if (retval != E_NO_ERR)
   {
      printf("GOM FSS: SOL_LibGetGFSSVersion transaction Failed %d\n", retval);
   }
   else
   {
     // TODO: change to bit shifts
     fssver=(datatorec[0]*16777216)+(datatorec[1]*65536)+(datatorec[2]*256)+datatorec[3];
   }
   return fssver;
}

uint32   SOL_LibGetGFSSUUID(int FssDevice)
{
   uint32       fssid=0;
   int          retval = 0;
   uint8        addr;
   uint8        datatosend[1];
   uint8        datatorec[4];

   if ( FssDevice < 0 || FssDevice > 1 )
   {
      return(fssid);
   }
   
   /* Convert device address ID to I2C address */
   addr = fssaddr[FssDevice];
   datatosend[0]=0x0D;
   retval= i2c_master_transaction(0, addr, datatosend, 1, datatorec, 4, 1000);

   if (retval != E_NO_ERR)
   {
      printf("GFSS FSS: SOL_LibGetGFSSUUID transaction Failed %d\n", retval);
   }
   else
   {
      // TODO: Bit shifts
      fssid=(datatorec[0]*16777216)+(datatorec[1]*65536)+(datatorec[2]*256)+datatorec[3];
   }
   return fssid;
}


/*
** Command to trigger sampling of sunsensor. The data can be read after a delay of 30 ms with the command
** CMD_SUN_GET_DATA. If another command is called between sample sensor and the get sensor data command,
** the data can be destroyed.
*/
int SOL_LibGFSS_SampleSun(int FssDevice)
{
   uint8        addr;
   int          retval = 0;
   uint8 datatosend[1];

   if ( FssDevice < 0 || FssDevice > 1 )
   {
      printf("Wrong Device in SampleSun\n");
      return(-1);
   }
   
   /* Convert device address ID to I2C address */
   addr = fssaddr[FssDevice];
   datatosend[0] = 0x00;
   retval = i2c_master_transaction(0, addr, datatosend, 1, NULL, 0, 1000);
   if (retval != E_NO_ERR)
   {
      printf("Gomspace FSS: SOL_LibGFSS_SampleSun Failed %d\n", retval);
   }
   else
   {
      retval = 0;
   }

   return(retval);
}

/*
** Gets the Raw sampled data from the sun sensor. Wait at least 30 ms after the sample
** sensor command before calling this command.
*/
// TODO: Not used. Remove?
int SOL_LibGFSS_GetSunRaw(int FssDevice, uint8 gfssbytes[])
{
   int          i;
   int          retval = 0;
   uint8        addr;
   uint8        datatosend[1];
   uint8        datatorec[8];

   if ( FssDevice < 0 || FssDevice > 1 )
   {
      return(-1);
   }
   
   /* Convert device address ID to I2C address */
   addr = fssaddr[FssDevice];
   datatosend[0]=0x01;

   retval = i2c_master_transaction(0, addr, datatosend, 1, datatorec, 8, 1000);
   if (retval != E_NO_ERR)
   {
      printf("GOM FSS: SOL_LibGFSS_GetSunRaw transaction Failed %d\n", retval);
      return(retval);
   }
   for (i=0;i<8;i++)
   {
      // TODO: Why not memcpy or call master_transaction with gfssbytes?
      gfssbytes[i]=datatorec[i];
   }

   return(0);
}

/*
** Gets the sampled data from the sun sensor. Wait at least 30 ms after the sample
** sensor command before calling this command.
*/
int SOL_LibGFSS_GetSun(int FssDevice, GFSS_data_t *GFssData)
{
   uint8        addr;
   int          retval = 0;
   uint8        datatosend[1];
   uint8        datatorec[8];

   if ( FssDevice < 0 || FssDevice > 1 )
   {
      return(-1);
   }

   if ( GFssData == 0 )
   {
      return(-1);
   }
   
   /* Convert device address ID to I2C address */
   addr = fssaddr[FssDevice];
   datatosend[0]=0x01;
   retval = i2c_master_transaction(0, addr, datatosend, 1, datatorec, 8, 1000);
 
   if (retval != E_NO_ERR)
   {
      printf("GOM FSS: SOL_LibGFSS_GetSun Failed %d\n", retval);
      return(retval);
   }

   // TODO: bitshifts
   GFssData->FSS_A_sen=(datatorec[0]*256)+datatorec[1];
   GFssData->FSS_B_sen=(datatorec[2]*256)+datatorec[3];
   GFssData->FSS_C_sen=(datatorec[4]*256)+datatorec[5];
   GFssData->FSS_D_sen=(datatorec[6]*256)+datatorec[7];

   return(0);
}

/*
** Calculate the vectors based on the data from the FSS
*/
int SOL_LibGFSS_Calculate(int FssDevice, GFSS_data_t *GFssData)
{
   float   sun[4];
   float   x,y,xc,yc,phi,theta;
   uint8   thetastep = 11;
   uint8   thetapoints = 6;
   uint8   phipoints[6] = {1,8,8,16,24,24};
   uint8   j,iphi,isum,index,itheta,i;

   if ( FssDevice < 0 || FssDevice > 1 )
   {
      return(-1);
   }

   if ( GFssData == 0 )
   {
      return(-1);
   }

   //Perform sun angle correction based on look-up table C
   if (GFssData->FSS_A_sen > 45000) GFssData->FSS_A_sen = 0; 
   if (GFssData->FSS_B_sen > 45000) GFssData->FSS_B_sen = 0; 
   if (GFssData->FSS_C_sen > 45000) GFssData->FSS_C_sen = 0; 
   if (GFssData->FSS_D_sen > 45000) GFssData->FSS_D_sen = 0; 

   sun[0] = (float) GFssData->FSS_A_sen;
   sun[1] = (float) GFssData->FSS_B_sen;
   sun[2] = (float) GFssData->FSS_C_sen;
   sun[3] = (float) GFssData->FSS_D_sen; 

if (NON_FLIGHT_SIMULATOR == 1)
  { 
   x = ((sun[0] + sun[1]) - (sun[2] + sun[3])) / (sun[0] + sun[1] + sun[2] + sun[3]); 
   y = ((sun[0] + sun[3]) - (sun[1] + sun[2])) / (sun[0] + sun[1] + sun[2] + sun[3]); 
   GFssData->FSS_Valid=1;
  } else { // FOR FLIGHT 
     if ((sun[0] + sun[1] + sun[2] + sun[3]) > GOM_FSS_DARKTH)
     {
      x = ((sun[0] + sun[1]) - (sun[2] + sun[3])) / (sun[0] + sun[1] + sun[2] + sun[3]); 
      y = ((sun[0] + sun[3]) - (sun[1] + sun[2])) / (sun[0] + sun[1] + sun[2] + sun[3]); 
      GFssData->FSS_Valid=1;
      } else {
      GFssData->FSS_Valid=0;
     }


   // Filter out if any sensor element is too low
   if ((sun[0] < GOM_FSS_IDARKTH) || (sun[1] < GOM_FSS_IDARKTH) || (sun[2] < GOM_FSS_IDARKTH) || (sun[3] < GOM_FSS_IDARKTH)) 
   {
      GFssData->FSS_Valid=0;
   }

   // Filter out if any sensor element is too high
   if ((sun[0] > GOM_FSS_BRIGHTTH) || (sun[1] > GOM_FSS_BRIGHTTH) || (sun[2] > GOM_FSS_BRIGHTTH) || (sun[3] > GOM_FSS_BRIGHTTH)) 
   {
      GFssData->FSS_Valid=0;
   }

   }
   /* 
   ** Parametric calibration [h, x0, y0, t]
   */
   /* 
   ** x,y offset
   */
   x = x + gfss_param[FssDevice][1];
   y = y + gfss_param[FssDevice][2];

   /* 
   ** Rotation
   */
   xc = cos(gfss_param[FssDevice][3])*x + sin(gfss_param[FssDevice][3])*y;
   yc = -sin(gfss_param[FssDevice][3])*x + cos(gfss_param[FssDevice][3])*y;

   /* 
   ** map from x and y to phi,theta
   */
   phi = atan2(xc,yc);
   if (phi<0) 
      phi += 2*M_PI;
   theta = atan2( sqrt(xc*xc + yc*yc), gfss_param[FssDevice][0]);

   /* 
   ** Lookup table calibration 
   */

   /* find theta index: */
   itheta = round( (theta*180/M_PI) / thetastep); 
   if ((itheta+1) > thetapoints) 
      itheta = thetapoints-1; 

   /* find phi index:  */
   iphi = round( (phi*180/M_PI) / (360/phipoints[itheta]) ); 
   if ((iphi+1) > phipoints[itheta]) 
      iphi = iphi - phipoints[itheta]; 

   /* index of correction point:  */
   isum = 0; 
   for (j=0;j<itheta;j++) 
      isum += phipoints[j]; 
   index = isum  + iphi  ; 
   theta = theta - (gfss_theta[FssDevice][index]*M_PI)/180; 
   phi = phi - (gfss_phi[FssDevice][index]*M_PI)/180; 

   GFssData->Azimuth=phi;
   GFssData->Elevation=theta;

   GFssData->Unit_Vector_X = cos(theta); // x
   GFssData->Unit_Vector_Y = sin(theta)*cos(phi); // y
   GFssData->Unit_Vector_Z = sin(theta)*sin(phi); // z

#if 0
   printf("Azimuth: %f   ***   %f \n",nano_fss_data.Azimuth,(nano_fss_data.Azimuth*180/M_PI));
   printf("Elevation: %f   ***   %f \n",nano_fss_data.Elevation,(nano_fss_data.Elevation*180/M_PI));
   printf("Unit_Vector_X: %f \n",nano_fss_data.Unit_Vector_X);
   printf("Unit_Vector_Y: %f \n",nano_fss_data.Unit_Vector_Y);
   printf("Unit_Vector_Z: %f \n",nano_fss_data.Unit_Vector_Z);
#endif
   return(0);
}
