/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: nanoMag.c                             */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    the NanoMind mag dat processing.         */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/*
** include files:
*/

#include "nanoMag.h"
#include "vector3.h"
#include "matrix3x3.h"
#include "monitor.h"

#define LIM_NMM_LOWER_X -50000.00
#define LIM_NMM_LOWER_Y -50000.00
#define LIM_NMM_LOWER_Z -50000.00
#define LIM_NMM_UPPER_X  50000.00
#define LIM_NMM_UPPER_Y  50000.00
#define LIM_NMM_UPPER_Z  50000.00
/*
#define LIM_NMM_LOWER_X -1575.39
#define LIM_NMM_LOWER_Y -1575.39
#define LIM_NMM_LOWER_Z -1575.39
#define LIM_NMM_UPPER_X 1574.61
#define LIM_NMM_UPPER_Y 1574.61
#define LIM_NMM_UPPER_Z 1574.61
*/
#define NMM_BIAS_X 0.0
#define NMM_BIAS_Y 0.0
#define NMM_BIAS_Z 0.0
#define NMM_SCALE_X 1
#define NMM_SCALE_Y 1
#define NMM_SCALE_Z 1

const Matrix3x3f ALIGN_NMM2BCS = {{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}}; 

int32_t BfieldNMM ( float Bfield_X, float Bfield_Y, float Bfield_Z,
     uint32_t Ready, uint32_t Valid, Vector3f *Bfield_BF_NMM)
{
    Vector3f Bfield_Adj;


	/* The NanoMind processor board includes a 3-axis 
      magnetometer that should always be powered. 
      The NMM Sensor Data Processing should be run at all times. 
      If the science magnetometer data is preferable, other logic 
      will select that output for further AD and ACS calculations 
	*/
		
	if (Ready == 1) 
	{
		//Check measured data verus limits
		if((LIM_NMM_LOWER_X > Bfield_X) || 
		   (LIM_NMM_UPPER_X < Bfield_X) || 
		   (LIM_NMM_LOWER_Y > Bfield_Y) || 
		   (LIM_NMM_UPPER_Y < Bfield_Y) || 
		   (LIM_NMM_LOWER_Z > Bfield_Z) || 
		   (LIM_NMM_UPPER_Z < Bfield_Z) ) 

		    {
         //printf("\n\n\noutside limits\n");
			Bfield_BF_NMM->Comp[0] = 0.0;
			Bfield_BF_NMM->Comp[1] = 0.0;
			Bfield_BF_NMM->Comp[2] = 0.0;
			Valid = 0;
		    }
		else
		{
		   Bfield_Adj.Comp[0] = Bfield_X*NMM_SCALE_X - NMM_BIAS_X;
		   Bfield_Adj.Comp[1] = Bfield_Y*NMM_SCALE_Y - NMM_BIAS_Y;
		   Bfield_Adj.Comp[2] = Bfield_Z*NMM_SCALE_Z - NMM_BIAS_Z;
		   //printf("Bfield, xyz: %f, %f, %f\n", Bfield_Adj.Comp[0],Bfield_Adj.Comp[1],Bfield_Adj.Comp[2]);
		   Matrix3x3f_MultVec(Bfield_BF_NMM, &ALIGN_NMM2BCS, &Bfield_Adj);
			//print_Vector3f("Bfield_BF_NMM", Bfield_BF_NMM);
	  	   Valid = 1;
		}
	}
	else
	{ 	Valid = 0;
		Bfield_BF_NMM->Comp[0] = 0.0;
		Bfield_BF_NMM->Comp[1] = 0.0;
		Bfield_BF_NMM->Comp[2] = 0.0;
	   //printf("\n\n\nBoom Mag Data Not Avail\n");
	}
	
	return 0;
}
