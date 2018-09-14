/*
 * sunpointmode.h
 *
 *  Created on: Jan 11, 2015
 *      Author: ssheikh
 */
#ifndef SUNPOINTMODE_H_
#define SUNPOINTMODE_H_

#include <stdint.h>
#include "vector3.h"
#include "matrix3x3.h"

int32_t SunPointMode (
	uint32_t SP_FirstCycle, 
	uint32_t CSS_Eclipse,  // Flag indicateing CSS dark, s/c in eclipse
	Vector3f *Bfield_BF_NMM, 
	float    Rate_SunPoint_Cmd,
	Vector3f *RwaMom_BF,
	Vector3f *Unit_Sun_BF, 
	Vector3f *Gain_Rate_Sun, 
	Vector3f *Gain_Att_Sun, 
	Vector3f *Att_Err_Sat, 
	Vector3f *Rate_BF_Sun, 
	Vector3f *Tor_BF_Ctrl, 
	Vector3f *Tor_RWA_Cmd);


#endif /* SUNPOINTMODE_H_ */
