/*
 * scimode.h
 *
 *  Created on: Jan 11, 2015
 *      Author: ssheikh
 */
#ifndef SCIMODE_H_
#define SCIMODE_H_

#include "vector3.h"
#include "vector4.h"

int32_t scimode (
	Vector4f *GFToBF_Est,    //  current best eest to gci-to-bcs quat
	Vector4f *LFToTarF_Cmd,  //  delta quat rotating from lvlhF to TarF
	Vector3f *Unit_Pos_EF,   // pos vect from ECIF to s/c
	Vector3f *Unit_Vel_GF,   // vel vect from ECIF to s/c
   Vector3f *Rate_Cmd_Science, // Command adjustment to rate for INMS controller (rad/s)
	Vector3f *Rate_BF_Est,   // Current best esimate of s/c body ang rates (rad/s)
	// outputs
	Vector3f *AttErr,	       // Attitude error from the controller, rad
	Vector3f *RateErr 	    // Rate error from the controller, rad/sec  
	);


#endif /* SCIMODE_H_ */
