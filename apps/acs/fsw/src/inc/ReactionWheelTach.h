
/*
 * ReactionWheelTach.h
 *
 *  Created on: Dec 8, 2014
 *      Author: ssheikh
 */

#include "vector3.h"

#ifndef REACTIONWHEELTACH_H_
#define REACTIONWHEELTACH_H_

/*
// For ReactionWheelTach
#define LIM_WINDOW 10
#define MOI_RW     .000028 // moment of inertia for each reaction wheel
// CalculateWheelCommands
#define GAIN_RW    1
#define BIAS_RW    1
*/

void ReactionCalcMomentum(Vector3f *Spd_rw_latest, Vector3f *Mom_BF);
void CalculateWheelCommands(Vector3f *Tor_Rw_Cmd, Vector3f *ScaledTor_Rw_Cmd);
// X, Y, and Z
//uint32_t Rw_Addresses[3] = {0x33, 0x35, 0x36};

#endif /* REACTIONWHEELTACH_H_ */
