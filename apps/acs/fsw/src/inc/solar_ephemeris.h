/*
 * solar_ephemeris.h
 *
 *  Created on: Dec 18, 2014
 *  Author: ssheikh
 */


#ifndef SOLAR_EPHEMERIS_H_
#define SOLAR_EPHEMERIS_H_


#include "vector3.h"

int32_t solar_ephemeris (double TimeTAI, Vector3f * Unit_Sun_GciF);

#endif /* SOLAR_EPHEMERIS_H_ */
