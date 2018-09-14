/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: spacecraft_ephemeris.c                */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    the s/c ephemeris when no gps available  */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/*
** include files:
*/
#include "NavDynamic.h"
#include "spacecraft_ephemeris.h"
#include "vector6.h"
#include "vector3.h"
#include "monitor.h"

#define EPHEM_STEPLIMIT 6.0  //  sec %Time limit on the difference between 
 //the current and previous times (seconds)
#define LEAP_SECONDS 36

int spacecraft_ephemeris (
        Vector3f *Pos_GF_ScWrtEarth,  
        Vector3f *Vel_GF_ScWrtEarth, 
		  double Time_Sys_TAI)
{

	int DragOn = 1;
	double StepDelta;
	static double Time_EphemPrev_TAI = 0.0;
	Vector6f X; // state vector
	Vector6f XT; // state vector
	Vector6f DX; // Time Derives of X
	Vector6f k1, k2, k3, k4;
	Matrix3x3f Dcm_EFToGF;
	int i; 
	
	 StepDelta = Time_Sys_TAI - Time_EphemPrev_TAI;
	
	#ifdef _ACS_DEBUG_
	 //printf("StepDelta = %f\n", StepDelta);
	#endif 

 	if ( StepDelta > 0  && StepDelta <= EPHEM_STEPLIMIT)
	{

	X.Comp[0] = Pos_GF_ScWrtEarth->Comp[0];
	X.Comp[1] = Pos_GF_ScWrtEarth->Comp[1];
	X.Comp[2] = Pos_GF_ScWrtEarth->Comp[2];
	X.Comp[3] = Vel_GF_ScWrtEarth->Comp[0];
	X.Comp[4] = Vel_GF_ScWrtEarth->Comp[1];
	X.Comp[5] = Vel_GF_ScWrtEarth->Comp[2];

	NavDynamic (DragOn, Time_Sys_TAI - LEAP_SECONDS, &X, &Dcm_EFToGF, &DX);

	for(i=0; i<6; i++) 
	{
		k1.Comp[i] = StepDelta * DX.Comp[i];
		XT.Comp[i] = X.Comp[i] + k1.Comp[i]/2;
	}

	NavDynamic (DragOn, Time_Sys_TAI - LEAP_SECONDS, &XT, &Dcm_EFToGF, &DX);
	for(i=0; i<6; i++) 
	{
		k2.Comp[i] = StepDelta * DX.Comp[i];
		XT.Comp[i] = X.Comp[i] + k2.Comp[i]/2;
 	}

	NavDynamic (DragOn,  Time_Sys_TAI - LEAP_SECONDS, &XT, &Dcm_EFToGF, &DX);
	for(i=0; i<6; i++) 
	{
		k3.Comp[i] = StepDelta * DX.Comp[i];
		XT.Comp[i] = X.Comp[i] + k3.Comp[i];
	}
	NavDynamic (DragOn,  Time_Sys_TAI - LEAP_SECONDS, &XT, &Dcm_EFToGF, &DX);

	for(i=0; i<6; i++) 
	{
		k4.Comp[i] = StepDelta * DX.Comp[i];
		X.Comp[i] = X.Comp[i] + (k1.Comp[i] + 2*k2.Comp[i] + 2*k3.Comp[i] + k4.Comp[i])/(float)6.0;
	}

 	// Store Time_Sys_TAI
	// Time_Ephem is used as the previous Time_SYS_TAI to compute time step
	// EphemDelta, in the Runga-Kutta Routine

   //printf("Time_EphemPrev_TAI: %f\n",Time_EphemPrev_TAI);

   Pos_GF_ScWrtEarth->Comp[0] = X.Comp[0];
   Pos_GF_ScWrtEarth->Comp[1] = X.Comp[1];
   Pos_GF_ScWrtEarth->Comp[2] = X.Comp[2];
   Vel_GF_ScWrtEarth->Comp[0] = X.Comp[3];
   Vel_GF_ScWrtEarth->Comp[1] = X.Comp[4];
   Vel_GF_ScWrtEarth->Comp[2] = X.Comp[5];
	}
	else
   {
    #if _ACS_DEBUG_ 
    printf("StepDelta Over Steplimit/negative\n");
    #endif 
    }

   Time_EphemPrev_TAI = Time_Sys_TAI;

return 0;

}
