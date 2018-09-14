/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: calculate_torques.c                   */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    the torque calculation for science mode  */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
**   Include Files:
*/
#include "vector3.h"
#include "matrix3x3.h"
#include "monitor.h"
#include "calculate_torques.h"
#include "sunpointmode.h"


/*
**   Constants :
*/

#define LIM_RWATOR      0.001 // Nm/Rad
#define TIME_CONTROLLER 1.0      //Controller timestep, sec.
#define LIM_ATTERR_SCI  0.17       // rad attitude limit for sci mode
#define LIM_INTERR_SCI  0.01    // Integral limit for sci mode, rad*sec.
#define LIM_RATEERR_SCI 10        // rad/sec
#define LIM_ATTERR_ZEROINT  0.01   // rad, attitude limit for turning off integral term of controller for sci mode
#define LIM_RATEERR_ZEROINT  0.04 // rad/sec rate limit for turning off integratl term of controller for sci mode

/*
** exported data 
*/
extern Matrix3x3f ALIGN_BF2RWAF;

const Vector3f NFILTX = {{ 1.0, 0.0, 0.0}}; // X-axis filter numerator coeff 
const Vector3f NFILTY = {{ 1.0, 0.0, 0.0}}; // Y-axis filter numerator coeff 
const Vector3f NFILTZ = {{ 1.0, 0.0, 0.0}}; // Z-axis filter numerator coeff 
const Vector3f DFILTX = {{ 1.0, 0.0, 0.0}}; // X-axis filter denominator coeff 
const Vector3f DFILTY = {{ 1.0, 0.0, 0.0}}; // Y-axis filter denominator coeff 
const Vector3f DFILTZ = {{ 1.0, 0.0, 0.0}}; // Z-axis filter denominator coeff 

int32_t calculate_torques (
	Vector3f *AttErr,	// input, Attitude error entering the controller, rad
	Vector3f *RateErr,	//Rate error entering the controller, rad/sec  
	uint32_t Science_FirstCycle, //	True is first cycle running Science.
   uint32_t UseStructFilter,  // Flag allowing use of structural filter
   Vector3f *Gain_att_sci,  // gain att science
   Vector3f *Gain_int_sci,  // gain int science
   Vector3f *Gain_rate_sci, // gain rate science
        // outputs
	uint8_t  *ACS_Settling,    //Flag that defines the mode the integral term is operating in.
	Vector3f *AttErr_Lim,	// Limited attitude error, rad 
	Vector3f *IntErr,	    // Limited integral error, rad*sec
	Vector3f *IntErr_Lim,	// Limited integral error, rad*sec 
	Vector3f *RateErr_Lim,	// Limited rate error, rad/sec
	Vector3f *Tor_RWA_Cmd,  // Commanded torques in RWA frame
	Vector3f *Tor_BF_Ctrl  // Control torques to be applied to the actuators in the BF, Nm
)
{
	int i;	
	static float Ux[3], Uy[3], Uz[3], Yx[3], Yy[3], Yz[3];
	float maxTor_RwaF_Cmd, ScaleTC;
	static Vector3f IntErr_Prev= {{0, 0, 0}};	// Integral error of previous timestep
	static Vector3f AttErr_Prev= {{0, 0, 0}};	// Attitude error of previous timestep

	Vector3f Abs_AttErr, Abs_RateErr; //, Abs_IntErr;
	Vector3f Abs_Tor_RwaF_Cmd, Tor_RwaF_Cmd, TorRwaF_Prod;
	//Vector3f Inv_Tor_RWA_Cmd;
	Vector3f Scaled_AttErr;
	Vector3f Tor_BF_Ctrl_PreFilter = {{0, 0, 0}}; // Control torque in the BF prior to being filtered, Nm
	Vector3f Sum_AttErr;
	// initial science mode
	if (Science_FirstCycle) 
	{
	Vector3f_InitZero(&AttErr_Prev);
	Vector3f_InitZero(&IntErr_Prev);
	Ux[0] = 0.0; Ux[1] = 0.0; Ux[2] = 0.0;
	Yx[0] = 0; Yx[1] = 0; Yx[2] = 0;
	Uy[0] = 0; Uy[1] = 0; Uy[2] = 0;
	Yy[0] = 0; Yy[1] = 0; Yy[2] = 0;
	Uz[0] = 0; Uz[1] = 0; Uz[2] = 0;
	Yz[0] = 0; Yz[1] = 0; Yz[2] = 0;
	}

	// check attitude and rate errors against settling limits (i.e. ZeroInit limits)
	Vector3f_Abs(&Abs_AttErr,  AttErr);
	Vector3f_Abs(&Abs_RateErr, RateErr);
	//Vector3f_Abs(&Abs_IntErr, IntErr);
	
   float Max_AbsAttErr, Max_AbsRateErr; //, Max_AbsIntErr; 
	Max_AbsAttErr = Vector3f_Max(&Abs_AttErr);
	Max_AbsRateErr = Vector3f_Max(&Abs_RateErr);
	//Max_AbsIntErr = Vector3f_Max(&Abs_IntErr);
   if ( (Max_AbsAttErr > LIM_ATTERR_ZEROINT ) ||  ( Max_AbsRateErr > LIM_RATEERR_ZEROINT ))
	{
		*ACS_Settling = 0;
		Vector3f_InitZero(IntErr);
	}
	else
	{
		*ACS_Settling = 1;
		#ifdef _ACS_DEBUG_ 
        printf("CT: ACS Settled\n"); 
      #endif
		ScaleTC = 0.5 * TIME_CONTROLLER;
		Vector3f_Add(&Sum_AttErr, AttErr, &AttErr_Prev);
		//Vector3f_MultScalar(&Scaled_AttErr, AttErr, ScaleTC);
		Vector3f_MultScalar(&Scaled_AttErr, &Sum_AttErr, ScaleTC);
		Vector3f_Add(IntErr, &IntErr_Prev, &Scaled_AttErr);
	}
	
	// Limit Errors, vector directions are NOT maintained
	//Vector3f_DivScalar(AttErr_Lim, AttErr, Max_AbsAttErr);
	//Vector3f_MultScalar(AttErr_Lim, AttErr_Lim, LIM_ATTERR_SCI);

   for (i=0; i<=2; i++)
   {
	if (AttErr->Comp[i] > LIM_ATTERR_SCI) 
		AttErr_Lim->Comp[i] = LIM_ATTERR_SCI;
	else if (AttErr->Comp[i] < -LIM_ATTERR_SCI)
		AttErr_Lim->Comp[i] = -LIM_ATTERR_SCI;
	else
		AttErr_Lim->Comp[i] = AttErr->Comp[i];
	}

	//print_Vector3f("CT: AttErr_limit\t", AttErr_Lim); 
	//Vector3f_MultScalar(IntErr_Lim, IntErr, Max_AbsIntErr);
	//Vector3f_DivScalar(IntErr_Lim, IntErr_Lim, LIM_INTERR_SCI);

   for (i=0; i<=2; i++)
   {
	if (IntErr->Comp[i] > LIM_INTERR_SCI) 
		IntErr_Lim->Comp[i] = LIM_INTERR_SCI;
	else if (IntErr->Comp[i] < -LIM_INTERR_SCI)
		IntErr_Lim->Comp[i] = -LIM_INTERR_SCI;
	else
		IntErr_Lim->Comp[i] = IntErr->Comp[i];
	}
	//print_Vector3f("CT: IntErr_limit\t", IntErr_Lim);

	//Vector3f_MultScalar(RateErr_Lim, RateErr, Max_AbsRateErr);
	//Vector3f_DivScalar(RateErr_Lim, RateErr_Lim, LIM_RATEERR_SCI);

   for (i=0; i<=2; i++)
   {
	if (RateErr->Comp[i] > LIM_RATEERR_SCI) 
		RateErr_Lim->Comp[i] = LIM_RATEERR_SCI;
	else if (RateErr->Comp[i] < -LIM_RATEERR_SCI)
		RateErr_Lim->Comp[i] = -LIM_RATEERR_SCI;
	else
		RateErr_Lim->Comp[i] = RateErr->Comp[i];
	}

	//print_Vector3f("CT: RateErr_limit\t", RateErr_Lim);

	// Calculate pre-filter torques
	for(i=0; i<3; i++)
	{
	Tor_BF_Ctrl_PreFilter.Comp[i] =
		  AttErr_Lim->Comp[i]  * Gain_att_sci->Comp[i]
		+ IntErr_Lim->Comp[i]  * Gain_int_sci->Comp[i]
		+ RateErr_Lim->Comp[i] * Gain_rate_sci->Comp[i];
	}

	//print_Vector3f("CT: Tor_BF_Ctrl_PreFilter", &Tor_BF_Ctrl_PreFilter);
	// Structural filter applied to control torques
   if (UseStructFilter)
   {
	  Ux[0] = Tor_BF_Ctrl_PreFilter.Comp[0];
	  Yx[0] = 1/DFILTX.Comp[0]*(NFILTX.Comp[0]*Ux[0] + NFILTX.Comp[1]*Ux[1] + NFILTX.Comp[2]*Ux[2]
	          - DFILTX.Comp[1]*Yx[1] + DFILTX.Comp[2]*Yx[2]);
	  Tor_BF_Ctrl->Comp[0] = Yx[0];
	
	  Uy[0] = Tor_BF_Ctrl_PreFilter.Comp[1];
	  Yy[0] = 1/DFILTY.Comp[0]*(NFILTY.Comp[0]*Uy[0] + NFILTY.Comp[1]*Uy[1] + NFILTY.Comp[2]*Uy[2]
              - DFILTY.Comp[1]*Yy[1] + DFILTY.Comp[2]*Yy[2]);
	  Tor_BF_Ctrl->Comp[1] = Yy[0];
		
	  Uz[0] = Tor_BF_Ctrl_PreFilter.Comp[2];
	  Yz[0] = 1/DFILTZ.Comp[0]*(NFILTZ.Comp[0]*Uz[0] + NFILTZ.Comp[1]*Uz[1] + NFILTZ.Comp[2]*Uz[2]
              - DFILTZ.Comp[1]*Yz[1] + DFILTZ.Comp[2]*Yz[2]);
	  Tor_BF_Ctrl->Comp[2] = Yz[0];

    	for (i = 1; i<=2; i++)
	   {
	   	Ux[i]=Ux[i-1];
	 	 Uy[i]=Uy[i-1];
		 Uz[i]=Uz[i-1];
		 Yx[i]=Yx[i-1];
		 Yy[i]=Yy[i-1];
		 Yz[i]=Yz[i-1];
	   }

  }
  else
		Vector3f_Copy(Tor_BF_Ctrl, &Tor_BF_Ctrl_PreFilter);
  

   #ifdef _ACS_DEBUG_
     print_Vector3f("CT: Tor_BF_Ctrl", Tor_BF_Ctrl); 
   #endif

	// Distribute control torques to wheels; limit torques
	Matrix3x3f_MultVec(&Tor_RwaF_Cmd, &ALIGN_BF2RWAF, Tor_BF_Ctrl);
   Vector3f_Abs(&Abs_Tor_RwaF_Cmd, &Tor_RwaF_Cmd);
   maxTor_RwaF_Cmd = Vector3f_Max(&Abs_Tor_RwaF_Cmd);
   //printf("CT: maxTor_RwaF_Cmd : %f\n", maxTor_RwaF_Cmd); 
	if (maxTor_RwaF_Cmd >= LIM_RWATOR )
	{
		/*
		Vector3f_MultScalar(&TorRwaF_Prod, &Tor_RwaF_Cmd, maxTor_RwaF_Cmd);
		Vector3f_DivScalar(&Inv_Tor_RWA_Cmd, &TorRwaF_Prod, LIM_RWATOR);
		Tor_RWA_Cmd->Comp[0] = 1/Inv_Tor_RWA_Cmd.Comp[0];
		Tor_RWA_Cmd->Comp[1] = 1/Inv_Tor_RWA_Cmd.Comp[1];
		Tor_RWA_Cmd->Comp[2] = 1/Inv_Tor_RWA_Cmd.Comp[2];
		*/
		Vector3f_MultScalar(&TorRwaF_Prod, &Tor_RwaF_Cmd, LIM_RWATOR);
      //print_Vector3f("CT: TorRwAF_Prod", &TorRwaF_Prod); 
		Vector3f_DivScalar(Tor_RWA_Cmd, &TorRwaF_Prod, maxTor_RwaF_Cmd);
	}
   else
	{
		Vector3f_Copy(Tor_RWA_Cmd, &Tor_RwaF_Cmd);
	}
      Vector3f_Copy(&AttErr_Prev, AttErr);
		Vector3f_Copy(&IntErr_Prev, IntErr_Lim);

		#ifdef _ACS_DEBUG_ 
      	print_Vector3f("CT: Tor_RWA_Cmd", Tor_RWA_Cmd); 
		#endif
return 0;
}
