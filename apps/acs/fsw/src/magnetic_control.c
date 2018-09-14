/*
 ============================================================================
 Name        : magnetic_control.c
 Author      : James Marshall
 Version     :
 Copyright   : Copyright 2015 NASA-GFSC Code 564 Greenbelt, MD 20771
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "magnetic_control.h"
#include "monitor.h"
#include "util.h"
#include "cfe_time.h"
#include "cfe_sb.h"
#include "acs_msg.h"


#define GAIN_MOM_MANAGE    1000.0 // The gain on the commanded moment for MomManage mode.
// LIM_MTC_MOM_MANAGE Should be equal to the smallest value of Lim_MTC_Cmd.
#define LIM_MTC_MOM_MANAGE 1 // The moment limit used for scaling commands in MomManage Mode. [A-m^2]
#define LIM_LATITUDE 20.00

// Alignment matrix from BCS to MTC frame. 
const const Matrix3x3f ALIGN_BCS2MTCF = {{
	{1.0, 0.0, 0.0},
	{0.0, 0.0, 1.0},
	{0.0, 1.0, 0.0}}};
// The absolute command limits on the MTC commanded moments for each axis. [A-m^2]
const const Vector3d Lim_MTC_Cmd = {{1.0, 1.0, 1.0}};

// TODO: Test
float max_abs(Vector3f *vec) {
  float max = 0.0;
  int i;

  for(i = 0; i < 3; i++)	{
    if (fabs(vec->Comp[i]) > max) {
      max = fabs(vec->Comp[i]);
    }
  }
  return max;
}

/*
 * Calculate moments to command the Magnetic Control Torquers (MCTs). 
 * There are three modes: Standby (0), MomManage (1), and Bdot (2), Latitude Manage(3)
 *
 * Input:
 *   Bfield_NMM: as measured by the NMM
 *   Config_MTC: Commanded MTC configuration.
 *   Latitude :  Latitude from GPS
 *   Pos_EF_ScWrtEarth: Current position of spacecraft. Only needed in Science Mode
 *   SysMom_BF_Calc: The calculated system angular momentum. The sign dictates the direction of the wheel.
 * Output:
 *   Moment_MTC_Cmd: Commanded MTC moments. [A-m^2]
 */
int MagneticControl(
          int Config_MTC,
          float Latitude, 
          Vector3f *Bfield_NMM,
		    //Vector3f *Pos_EF_ScWrtEarth,
		    Vector3f *SysMom_BF_Calc,
		    Vector3f *Moment_MTC_Cmd)
{
  // persistant variables
  // NMM-measured B-field from the previous cycle.
  static Vector3f Bfield_NMM_Prev = {{0.0, 0.0, 0.0}};

  // local variables
  //  Control moments in BCS frame. [A-m^2](3)
  Vector3f Moment_BF_Ctrl;
  int i;
  float max_abs_MTC_Cmd;

  if (Config_MTC == CFG_MTC_IDLE) {
    // In IDLE Mode; zero MTC commands.
    Vector3f_MultScalar(Moment_MTC_Cmd, Moment_MTC_Cmd, 0.0);
  } else if (Config_MTC == CFG_MTC_BDOT) {
    // In Bdot Mode; calculate commands to maximize rate damping effect.
    for (i = 0; i < 3; i++) {
      Moment_BF_Ctrl.Comp[i] = -(Lim_MTC_Cmd.Comp[i] * signum(Bfield_NMM->Comp[i] - Bfield_NMM_Prev.Comp[i]));
    }
    Matrix3x3f_MultVec(Moment_MTC_Cmd, &ALIGN_BCS2MTCF, &Moment_BF_Ctrl);

  } else if ( Config_MTC == CFG_MTC_MOMMNG ) {
    // In MomManage Mode (used for Charging Mode), calculate commands based on system momentum
    //print_Vector3f("SysMom_Calc", SysMom_BF_Calc);
    //print_Vector3f("BfieldNMM", Bfield_NMM);
    Vector3f_Cross(&Moment_BF_Ctrl, SysMom_BF_Calc, Bfield_NMM);
    //print_Vector3f("MC: PreGain: Moment_BF_Ctrl", &Moment_BF_Ctrl);
    Vector3f_MultScalar(&Moment_BF_Ctrl, &Moment_BF_Ctrl, GAIN_MOM_MANAGE);
    //print_Vector3f("MC: PostGain: Moment_BF_Ctrl", &Moment_BF_Ctrl);
    Matrix3x3f_MultVec(Moment_MTC_Cmd, &ALIGN_BCS2MTCF, &Moment_BF_Ctrl);
    //print_Vector3f("Moment_MTC_Cmd", Moment_MTC_Cmd);
    max_abs_MTC_Cmd =  max_abs(Moment_MTC_Cmd);
    //printf("max_abs_MTC_Cmd %f\n", max_abs_MTC_Cmd);
    if ( max_abs_MTC_Cmd >= LIM_MTC_MOM_MANAGE ) {
      Vector3f_MultScalar(Moment_MTC_Cmd, Moment_MTC_Cmd, LIM_MTC_MOM_MANAGE / max_abs_MTC_Cmd);
    }
  } else  if ( Config_MTC == CFG_MTC_LATMNG ) {
    // In LatManage configuration (i.e. nommode);  calculate commands based on 
    // system momentum. If above a specified latitude, zero the commands.
    if (abs(Latitude) >  LIM_LATITUDE )  
       Vector3f_MultScalar(Moment_MTC_Cmd, Moment_MTC_Cmd, 0.0);
    else
	 { 
    Vector3f_Cross(&Moment_BF_Ctrl, SysMom_BF_Calc, Bfield_NMM);
    Vector3f_MultScalar(&Moment_BF_Ctrl, &Moment_BF_Ctrl, GAIN_MOM_MANAGE);
    Matrix3x3f_MultVec(Moment_MTC_Cmd, &ALIGN_BCS2MTCF, &Moment_BF_Ctrl);
    max_abs_MTC_Cmd =  max_abs(Moment_MTC_Cmd);
    if ( max_abs_MTC_Cmd >= LIM_MTC_MOM_MANAGE ) 
      Vector3f_MultScalar(Moment_MTC_Cmd, Moment_MTC_Cmd, LIM_MTC_MOM_MANAGE / max_abs_MTC_Cmd);
    }
  }
   #ifdef _ACS_DEBUG_
	  //print_Vector3f("Moment MTC Cmd in mag control", Moment_MTC_Cmd); 
	#endif
    Vector3f_Copy(&Bfield_NMM_Prev, Bfield_NMM);

  return 0;
}
