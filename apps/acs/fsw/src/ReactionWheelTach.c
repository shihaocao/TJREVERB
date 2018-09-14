/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: ReactionWheelTach.c                   */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    the Reaction WheelTachometer &           */
/*    Calculates Wheel Commands                */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdint.h>
#include <math.h>

/*
** include files:
*/
#include "ReactionWheelTach.h"
#include "vector3.h"
#include "matrix3x3.h"

#define LIM_WINDOW 10
#define MOI_RW     0.000028 // moment of inertia for each reaction wheel
#define GAIN_RW    1
#define BIAS_RW    0

// This matches the mechanical drawings. The wheels are aligned, except for Z which faces "backwards"
const const Matrix3x3f ALIGN_RWA2BCS = {{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, -1.0}}};

/*
 *2.6:
 * Converts the reaction wheel speeds to stored momentum in the body frame.
 *
 *params:
 * input - Vector3f *Spd_rw_latest - The latest rw tach reading
 * output - Vector3f *RwaMom_BF - Can send when requested, or publish every iteration
 */
void ReactionCalcMomentum(Vector3f *Spd_rw_latest, Vector3f *RwaMom_BF) {
  Vector3f AngMom_rw_avg = {{0}};

  Vector3f_MultScalar(&AngMom_rw_avg, Spd_rw_latest, MOI_RW);

  // matrix!
  Matrix3x3f_MultVec(RwaMom_BF, &ALIGN_RWA2BCS, &AngMom_rw_avg);

  return;
}

/*
 *
 * Translate between torque commands and current commands for the reaction wheels
 * Note:
 *
 *params:
 * input - Vector3f *Tor_Rw_Cmd - Torque commands for the reaction wheels (frame?)
 * output -  Vector3f *ScaledTor_Rw_Cmd) - Scaled Torque command to send to each wheel 
 */
void CalculateWheelCommands(Vector3f *Tor_Rw_Cmd, Vector3f *ScaledTor_Rw_Cmd) {
  uint32_t index;
  for (index = 0; index < 3; index++) {
    // TODO: Check meaning of Bias, make sure it matches use
    if ((Tor_Rw_Cmd->Comp[index] * GAIN_RW) < BIAS_RW) {
      ScaledTor_Rw_Cmd->Comp[index] = Tor_Rw_Cmd->Comp[index] * GAIN_RW;
    } else {
      ScaledTor_Rw_Cmd->Comp[index] = (Tor_Rw_Cmd->Comp[index] * GAIN_RW) - BIAS_RW;
    }
  }
}
