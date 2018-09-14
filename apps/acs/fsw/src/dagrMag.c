/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: dagrMag.c                             */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*   the boom magnetometer data processing     */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/*
** include files
*/
#include "dagrMag.h"
#include "vector3.h"
#include "matrix3x3.h"

/*
** constants and defines
*/
#define LIM_SBM_LOWER_X -50000.0
#define LIM_SBM_LOWER_Y -50000.0
#define LIM_SBM_LOWER_Z -50000.0
#define LIM_SBM_UPPER_X  50000.0
#define LIM_SBM_UPPER_Y  50000.0
#define LIM_SBM_UPPER_Z  50000.0
#define SBM_BIAS_X 0.0
#define SBM_BIAS_Y 0.0
#define SBM_BIAS_Z 0.0
#define SBM_SCALE_X 1.0
#define SBM_SCALE_Y 1.0
#define SBM_SCALE_Z 1.0
const Matrix3x3f ALIGN_SBM2BCS = { { {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}}; 

int32_t BfieldDagr ( 
     float Bfield_SBM_X,   // B-field data read from Science Boom Mag (nT)
     float Bfield_SBM_Y,   // B-field data read from Science Boom Mag (nT)
     float Bfield_SBM_Z,   // B-field data read from Science Boom Mag (nT)
     uint32_t SBM_Ready,        // flag showing that new data is available 
     uint32_t SBM_Valid,        // SBM data valid
     Vector3f *Bfield_BF_SBM) // magnetic b-field vector in BCS
{
    Vector3f Bfield_SBM_Adj;
		
	if (SBM_Ready == 1) 
	{
		//Check measured data verus limits
		if((LIM_SBM_LOWER_X > Bfield_SBM_X) || 
	        (LIM_SBM_UPPER_X < Bfield_SBM_X) || 
	        (LIM_SBM_LOWER_Y > Bfield_SBM_Y) || 
	        (LIM_SBM_UPPER_Y < Bfield_SBM_Y) || 
	        (LIM_SBM_LOWER_Z > Bfield_SBM_Z) || 
	        (LIM_SBM_UPPER_Z < Bfield_SBM_Z) )
		    {
			Bfield_BF_SBM->Comp[0] = 0.0;
			Bfield_BF_SBM->Comp[1] = 0.0;
			Bfield_BF_SBM->Comp[2] = 0.0;
			SBM_Valid = 0;
			}
		else
		{
		   Bfield_SBM_Adj.Comp[0] = Bfield_SBM_X*SBM_SCALE_X - SBM_BIAS_X;
		   Bfield_SBM_Adj.Comp[1] = Bfield_SBM_Y*SBM_SCALE_Y - SBM_BIAS_Y;
		   Bfield_SBM_Adj.Comp[2] = Bfield_SBM_Z*SBM_SCALE_Z - SBM_BIAS_Z;
		   Matrix3x3f_MultVec(Bfield_BF_SBM, &ALIGN_SBM2BCS, &Bfield_SBM_Adj);
	  	   SBM_Valid = 1;
		}
	}
	else
	{ 	SBM_Valid = 0;
		Bfield_BF_SBM->Comp[0] = 0;
		Bfield_BF_SBM->Comp[0] = 0;
		Bfield_BF_SBM->Comp[0] = 0;
	}
		
	return 0;
}
