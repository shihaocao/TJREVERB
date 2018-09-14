/*
 * scimode.h
 *
 *  Created on: Dec 10, 2014
 *      Author: ssheikh
 */
#ifndef CALCULATE_TORQUES_H_
#define CALCULATE_TORQUES_H_

#include <stdint.h>
#include "vector3.h"

int32_t calculate_torques (
	Vector3f *AttErr,	// input, Attitude error entering the controller, rad
	Vector3f *RateErr,	//Rate error entering the controller, rad/sec  
	uint32_t Science_FirstCycle, //	True is first cycle running Science.
	uint32_t UseStructFilter,  // Flag allowing use of structural filter
   Vector3f *Gain_att_sci,
   Vector3f *Gain_int_sci,
   Vector3f *Gain_rate_sci,
	uint8_t  *ACS_Settling,    //Flag that defines the mode the integral term is operating in.
        // outputs
	Vector3f *AttErr_Lim,	// Limited attitude error, rad 
	Vector3f *IntErr,	    // Limited integral error, rad*sec
	Vector3f *IntErr_Lim,	// Limited integral error, rad*sec 
	Vector3f *RateErr_Lim,	// Limited rate error, rad/sec
	Vector3f *Tor_RWA_Cmd,
	Vector3f *Tor_BF_Ctrl   // Control torques to be applied to the actuators in the BcsF, Nm
);


#endif /* CALCUALATE_TORQUES_H_ */
