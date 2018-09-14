/*
 * betangle.h
 *
 *  Created on: Mar 2, 2016
 *      Author: Salman Sheikh 
 */

#ifndef BETANGLE_H_
#define BETANGLE_H_


#include "vector3.h"

float betangle (
	Vector3f *Unit_Pos_EF, // pos vect from ECIF to s/c
	Vector3f *Unit_Vel_EF, // vel vect from ECIF to s/c
	Vector3f *Unit_Sun_GF
);


#endif /* BETANGLE_H_ */
