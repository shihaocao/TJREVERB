/*
 * gha.h
 *
 *  Created on: Dec 10, 2015
 *      Author: Salman Sheikh 
 */

#ifndef GHA_H_
#define GHA_H_


#include "vector3.h"
#include "matrix3x3.h"

float GHA (double Time_Sys_UTC, Matrix3x3f *Dcm_EFToGF);

#endif /* GHA_H_ */
