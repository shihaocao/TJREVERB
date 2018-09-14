/*
 * igrf.h
 *
 *  Created on: Mar 10, 2015
 *  Author: ssheikh
 */

#ifndef IGRF_H_
#define IGRF_H_

#include "vector3.h"

int32_t igrf( float latitude, float longitude, float altitude, Vector3f *Bfield);

#endif /* IGRF_H_ */
