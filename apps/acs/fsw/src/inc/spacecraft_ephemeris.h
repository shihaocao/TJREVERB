/*
 * spacecraft_ephemeris.h
 *
 *  Created on: Dec 18, 2014
 *  Author: ssheikh
 */

#ifndef SPACECRAFT_EPHEMERIS_H_
#define SPACECRAFT_EPHEMERIS_H_

#include "vector3.h"
#include "vector6.h"
#include "matrix3x3.h"

int spacecraft_ephemeris (Vector3f *Pos_GciF_ScWrtEarth,  
                          Vector3f *Vel_GciF_ScWrtEarth, 
                          double Time_Sys_TAI);

#endif /* spacecraft_ephemeris.h */
