/*
 * DagrMag.h
 *
 *  Created on: Dec 22, 2014
 *  Author: ssheikh
 */

#include "vector3.h"
#include "matrix3x3.h"

#ifndef DAGRMAG_H_
#define DAGRMAG_H_

int32_t BfieldDagr ( 
     float Bfield_SBM_X,   // B-field data read from Science Boom Mag (nT)
     float Bfield_SBM_Y,   // B-field data read from Science Boom Mag (nT)
     float Bfield_SBM_Z,   // B-field data read from Science Boom Mag (nT)
     uint32_t SBM_Ready,        // flag showing that new data is available 
     uint32_t SBM_Valid,        // SBM data valid
     Vector3f *Bfield_BF_SBM); // magnetic b-field vector in BCS

#endif /* DAGRMAG_H_ */
