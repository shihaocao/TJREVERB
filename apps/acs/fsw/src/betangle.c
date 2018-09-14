/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: betangle.c                             */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    the science mode controller    .         */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

/*
** include files
*/
#include "vector3.h"
#include "util.h"
#include "monitor.h"


float betangle (
	Vector3f *Unit_Pos_EF, // unit pos vect from ECIF 
	Vector3f *Unit_Vel_EF, // uint vel vect from ECIF
	Vector3f *Unit_Sun_GF
)
{
   Vector3f Unit_PxV;      // unit vector of Pos Cross Vel
   Vector3f Ang;
   double Beta_Angle;

	Vector3f_Cross(&Unit_PxV, Unit_Pos_EF, Unit_Vel_EF);
	Vector3f_Cross(&Ang, Unit_Sun_GF, &Unit_PxV);
   Beta_Angle = acos(Ang.Comp[0]);
   //*Beta_Angle = (float) DBeta_Angle;
   #ifdef _ACS_DEBUG_ 
	printf("Beta Angle: %f\n", Beta_Angle); 
	#endif

return (float)Beta_Angle;
}
