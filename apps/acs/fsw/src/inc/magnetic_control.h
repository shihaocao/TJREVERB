/*
 * magnetic_control.h
 *
 *  Created on: Jun 09, 2015
 *      Author: James Marshall
 */

#ifndef MAGNETIC_CONTROL_H_
#define MAGNETIC_CONTROL_H_


#include "vector3.h"
#include "matrix3x3.h"

int MagneticControl(
		    int      Config_MTC,
		    float    Latitude,
          Vector3f *Bfield_NMM,
		    //Vector3f *Pos_EF_ScWrtEarth,
		    Vector3f *SysMom_BF_Calc,
		    Vector3f *Moment_MTC_Cmd);

#endif /* MAGNETIC_CONTROL_H_ */
