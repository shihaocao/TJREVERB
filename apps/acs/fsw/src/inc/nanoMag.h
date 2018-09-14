/*
 * nanoMag.h
 *
 *  Created on: Mar 10, 2015
 *  Author: ssheikh
 */
#include "vector3.h"
#include "matrix3x3.h"

#ifndef NANOMAG_H_
#define NANOMAG_H_

int32_t BfieldNMM ( float Bfield_X, 
                float Bfield_Y, 
                float Bfield_Z,
                uint32_t Ready, 
                uint32_t Valid, 
                Vector3f *Bfield_BcsF_NMM);

#endif /* NANOMAG_H_ */
