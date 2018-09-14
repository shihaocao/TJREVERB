/*
 * FSS.h
 *
 *  Created on: Dec 18, 2014
 *  Author: ssheikh
 */

#ifndef FSS_H_
#define FSS_H_

#include "vector3.h"

int32_t FSS ( 
	float Azimuth,    	// Sampled FSS data in azimuth, 
	float Elevation,    	// then elevation 
	uint32_t FSS_SunPres, 		// FSS Sun Presence Flag (1 = Sun seen by FSS)
	uint32_t *FSS_SunValid,   // Flag showing the output Sun vector is valid
	Vector3f *Unit_Sun_FssF);

#endif /* FSS_H_ */
