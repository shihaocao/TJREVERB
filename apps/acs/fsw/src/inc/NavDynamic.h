/*
 * NavDynamic.h
 *
 *  Created on: Jul 19, 2016
 *  Author: ssheikh
 */

#ifndef NAVDYNAMIC_H
#define NAVDYNAMIC_H_

#include "vector6.h"
#include "matrix3x3.h"

int NavDynamic ( // inputs
	uint32_t DragOn,
	double   Time_UTC,          // Current system universal time (UTC)
	Vector6f *X,              // Position and Velocity State
	// outputs 
	Matrix3x3f *Dcm_EFToGF,   // direct cosine rotation matrix 
	Vector6f *DX);            // state derivative velocity [m/s] and accel [m/s^2]

#endif /* NavDynamic.h */
