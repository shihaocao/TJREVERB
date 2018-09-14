#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "gha.h"

/*
**   Constants :
*/

#define EARTHRATE    7.292115854791750e-005 //rad/sec %USED P.M. 11/28/05: Earth Rate 
#define GHAZer           1.745881021656302 //  rad     JulianDate (1/1/2008, 0 hr)  Greenwich hour angle (rad)
#define JulTime1958_UTC  2436204.5       //hour UTC    Julian Date -> Jan. 1, 1958, 0 hour UTC  
#define JulTimeInit_UTC  2454466.5       //UTC  seconds Julian Date -> Jan, 1, 2008, 0 hr 
#define SEC_PER_DAY  86400.0 //  sec     Number of seconds in a day

// Greenwich Hour Angle 
// Computed Dcm_EFToGF conversion matrix

float GHA (double Time_Sys_UTC, Matrix3x3f *Dcm_EFToGF)
{
	double HA;
	HA = GHAZer + EARTHRATE * (Time_Sys_UTC - (JulTimeInit_UTC - JulTime1958_UTC) * SEC_PER_DAY);
   //printf("H1, HA1 is %f %f\n", HA, HA1);
	//printf("Greenwich Hour Angle= %f\n", HA1);
	Dcm_EFToGF->Comp[0][0] =  cos(HA);
	Dcm_EFToGF->Comp[1][0] =  sin(HA);
	Dcm_EFToGF->Comp[2][0] =  0;
	Dcm_EFToGF->Comp[0][1] =  -sin(HA);
	Dcm_EFToGF->Comp[1][1] =  cos(HA);
	Dcm_EFToGF->Comp[2][1] =  0;
	Dcm_EFToGF->Comp[0][2] =  0;
	Dcm_EFToGF->Comp[1][2] =  0;
	Dcm_EFToGF->Comp[2][2] =  1;
	return (float)HA;
}
