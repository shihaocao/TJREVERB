/*
 * ACS Constants 
 *
 *  Created on: August 18, 2015
 *  Author: Salman Sheikh
 *  NASA-GSFC
 *  Code 564    
 *  Greenbelt, MD 20771   
 */

#ifndef ACS_CONSTANTS_H
#define ACS_CONSTANTS_H

#include <stdint.h>
#include "vector3.h"
#include "vector4.h"
#include "util.h"
#include "cfe.h"

int32_t acs_top(
	Vector4f *Targ_Cmd_Sci,
	float    Rate_Cmd_Sun,
	Vector3f *Rate_Cmd_Sci,
	uint32_t AKF_Enable,
	uint8_t  RW_Config, 
	uint8_t  MTC_Config, 
	uint8_t  sc_mode, 
	Vector3f *Gain_Att_Sci,
	Vector3f *Gain_Int_Sci,
	Vector3f *Gain_Rate_Sci,
	Vector3f *Gain_Rate_Sun,
	Vector3f *Gain_Att_Sun);

#endif 
