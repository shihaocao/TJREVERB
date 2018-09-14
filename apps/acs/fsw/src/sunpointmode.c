/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: SunPointMode.c                        */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    the sunpointing mode controller.         */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>

/* 
** include files
*/
#include "vector3.h"
#include "matrix3x3.h"
#include "monitor.h"
#include "sunpointmode.h"
#include "acs_app.h"
#include "acs_msg.h"

#define LIM_RWATOR 0.001 // N-m/Rad
#define GAIN_RATE_PERP  1.0      
#define GAIN_RATE_SUNLINE  0.1   //Rate gain about the Sun line in SunPoint [Nm/(rad/sec)]
#define LIM_ATTERR  100
#define LIM_FLIP  -0.9             //Limit for 180 degree flip logic
#define THRESH_LOWXZMOMSQ  0.000001
#define THRESH_POINTINGCLOSE  0.93
#define TIME_SUNPOINT  1.0           //Expected approximate time between data (CSS) measurements [sec] Cannot = 0

const const Vector3f BIAS_FLIP = {{0.0, 0.0, 1.0}};
const const Vector3f UNITSUNBFTARG = {{0.0, 1.0, 0.0}};  
//const const Vector3f GAIN_ATT = {{0.000, 0.000, 0.000}};

extern Matrix3x3f ALIGN_BF2RWAF;
extern acs_spt_tlm_t ACS_SptTlmPkt;

int32_t SunPointMode (
	uint32_t SP_FirstCycle, 
	uint32_t CSS_Eclipse,
	Vector3f *Bfield_BF_NMM,
	float     Rate_Cmd_SunPoint,
	Vector3f *RwaMom_BF,
	Vector3f *Unit_Sun_BF, 
	Vector3f *Gain_Rate_Sun,
	Vector3f *Gain_Att_Sun,
	// outputs
	Vector3f *Att_Err_Sat, 
	Vector3f *Rate_BF_Sun, 
	Vector3f *Tor_BF_Ctrl, 
	Vector3f *Tor_RWA_Cmd)
{

static Vector3f Unit_Sun_BF_Prev;  // UnitSun vector from previous cycle
static Vector3f Tor_RWA_Cmd_Prev;  //Previous value of Tor_RWA_Cmd for Torque-Tach compare [Nm]
static Vector3f RwaMom_BF_Prev;    //Inter	Previous measured reaction wheel momentum vector, body frame [Nms]
static Vector3f  Unit_Bfield_BF_Prev;

//print_Vector3f("SP: Unit_Sun_BF_Prev", &Unit_Sun_BF_Prev);
//print_Vector3f("SP: Bfield_BF_NMM", Bfield_BF_NMM);
//print_Vector3f("SP: RwaMom_BF", RwaMom_BF);
//print_Vector3f("SP: Unit_Sun_BF", Unit_Sun_BF);


Vector3f Rate_Err_Perp; 
float Rate_Err_Sunline; 

Vector3f Att_Err;		//Attitude error before limiting [N/A]
Vector3f Att_Err_Flip;		//Attitude error  to cause spacecraft flip [N/A]
Vector3f Rate_Err;		//Rate error before limiting [rad/sec]

float XZMomSq;	//Square of angular momentum in XZ plane [Nms]
float maxTor_RWA_Cmd;
Vector3f TempResult;
Vector3f  Unit_Bfield_BF = {{0.0, 0.0, 0.0}};
Vector3f  Unit_Bfield_BF_Diff = {{0.0, 0.0, 0.0}};

    // hold on prev torque
    Vector3f_Copy(&Tor_RWA_Cmd_Prev, Tor_RWA_Cmd);
    // Determine safehold pointing error
    Vector3f_Cross (&Att_Err , Unit_Sun_BF, &UNITSUNBFTARG);

	if (Unit_Sun_BF->Comp[1] > LIM_FLIP)
		Vector3f_InitZero(&Att_Err_Flip);
	else
		Vector3f_Copy(&Att_Err_Flip, &BIAS_FLIP);

	//Derive rate errors normal to and aligned with the measured sunline

	if  (SP_FirstCycle)
	{
		Vector3f_InitZero(Rate_BF_Sun);
		Vector3f_InitZero(&Rate_Err_Perp);
		Rate_Err_Sunline = 0;
      //print_Vector3f("Unit_Sun_BF_Prev first", &Unit_Sun_BF_Prev);
	}
	else
	{
      //print_Vector3f("SP: Unit_Sun_BF_Prev", &Unit_Sun_BF_Prev);
		Vector3f_Sub(Rate_BF_Sun, Unit_Sun_BF, &Unit_Sun_BF_Prev);
		Vector3f_DivScalar(Rate_BF_Sun, Rate_BF_Sun,TIME_SUNPOINT);
		Vector3f_Cross(&Rate_Err_Perp, Rate_BF_Sun, Unit_Sun_BF);
		XZMomSq = RwaMom_BF->Comp[0] * RwaMom_BF->Comp[0] + RwaMom_BF->Comp[2] * RwaMom_BF->Comp[2];
		#if _ACS_DEBUG_
      //print_Vector3f("SP: Unit_Sun_BF", Unit_Sun_BF);
      //print_Vector3f("SP: Rate_BF_Sun", Rate_BF_Sun);
		//print_Vector3f("SP: Rate_Err_Perp", &Rate_Err_Perp);
		//printf("SP: XZMomSq is %f\n", XZMomSq);
	   #endif

	/* Perform check on wheel momentum perpendicular to Sun line, to 
   confirm momentum level high enough to guarantee good functioning of the mode.
	*/
	if (XZMomSq >= THRESH_LOWXZMOMSQ)
		Rate_Err_Sunline = ((RwaMom_BF->Comp[2] * RwaMom_BF_Prev.Comp[0]) - 
			(RwaMom_BF->Comp[0] * RwaMom_BF_Prev.Comp[2]))/(XZMomSq * TIME_SUNPOINT);
	else
		Rate_Err_Sunline = 0;
	}

   //print_Vector3f("RwaMom_BF", RwaMom_BF);
   //print_Vector3f("Rate_BF_RKF", Rate_BF_RKF);
	//Vector3f_Copy(&Unit_Sun_BF_Prev , Unit_Sun_BF);
	Vector3f_Copy(&RwaMom_BF_Prev, RwaMom_BF);
	//print_Vector3f("RwaMom_BF_Prev", &RwaMom_BF_Prev);
	//print_Vector3f("Bfield_BF_NMM sunpt", Bfield_BF_NMM);
   Vector3f_Normalize (&Unit_Bfield_BF, Bfield_BF_NMM); 
	//print_Vector3f("SP: Unit_Bfield_BF", &Unit_Bfield_BF);
	// Determine whether to use in-Sun controller or eclipse controller
	if (CSS_Eclipse == 1 )
    {
		Vector3f_Sub(&Unit_Bfield_BF_Diff, &Unit_Bfield_BF, &Unit_Bfield_BF_Prev);
		Vector3f_DivScalar(&Rate_Err, &Unit_Bfield_BF_Diff, TIME_SUNPOINT);
	   //print_Vector3f("SP: Rate Err in Eclipse", &Rate_Err);
      // NOT FOR FLIGHT (remove these lines for Flight)
//		Vector3f_InitZero(&Rate_Err);
//	   print_Vector3f("SP: Rate Err in Eclipse (Nonflight)", &Rate_Err);
	 }
	else
		// Perform check on Sun angle (Y-component of Sun vector vs. THRESH_POINTINGCLOSE)
		// to determine whether to use the CSS-based or RWA-based Y-axis rate estimate.
	{
		if (Unit_Sun_BF->Comp[1] <= THRESH_POINTINGCLOSE)
		{
			Vector3f_MultScalar(&Rate_Err, &Rate_Err_Perp, GAIN_RATE_PERP);
	      //print_Vector3f("SP: Rate Err in SP (THRESH)", &Rate_Err);
		}
		else
		{
			Rate_Err.Comp[0] = Rate_Err_Perp.Comp[0] * GAIN_RATE_PERP;
			Rate_Err.Comp[1] = Rate_Err_Sunline * GAIN_RATE_SUNLINE;
			Rate_Err.Comp[2] = Rate_Err_Perp.Comp[2] * GAIN_RATE_PERP;
	      //printf("Sunline\n");
		}
	}

     #ifdef _ACS_DEBUG_
	   //print_Vector3f("SP: Rate Err", &Rate_Err);
	  #endif
     // Subtract commanded Y rate bias from the Y rate error.
     Rate_Err.Comp[1] = Rate_Err.Comp[1] - Rate_Cmd_SunPoint;

     ACS_SptTlmPkt.spt_atterr[0] = Att_Err.Comp[0];
     ACS_SptTlmPkt.spt_atterr[1] = Att_Err.Comp[1];
     ACS_SptTlmPkt.spt_atterr[2] = Att_Err.Comp[2];
     ACS_SptTlmPkt.spt_raterr[0] = Rate_Err.Comp[0];
     ACS_SptTlmPkt.spt_raterr[1] = Rate_Err.Comp[1];
     ACS_SptTlmPkt.spt_raterr[2] = Rate_Err.Comp[2];

     int i;

     for (i=0; i<=2; i++)
     {
     	if ((Att_Err.Comp[i] + Att_Err_Flip.Comp[i]) >= LIM_ATTERR)
		{
     		Att_Err_Sat->Comp[i] =  LIM_ATTERR;
			//printf("%d Att_Err_Above LIM_ATTERR\n", i);
		}
    	else if ((Att_Err.Comp[i] + Att_Err_Flip.Comp[i]) <= -LIM_ATTERR)
		{
     		Att_Err_Sat->Comp[i] =  -LIM_ATTERR;
			//printf("%d Att_Err_below LIM_ATTERR\n", i);
		}
     	else
		{
     		Att_Err_Sat->Comp[i] =  Att_Err.Comp[i] + Att_Err_Flip.Comp[i];
			//printf("%d Att_Err_in range LIM_ATTERR\n", i);
		}

	}
	#ifdef  _ACS_DEBUG_
   //print_Vector3f("SP: Att_Err", Att_Err_Sat);
  #endif
	// Calculate Torque

	Tor_BF_Ctrl->Comp[0] = Gain_Att_Sun->Comp[0]* Att_Err_Sat->Comp[0] + Gain_Rate_Sun->Comp[0] * Rate_Err.Comp[0];
	Tor_BF_Ctrl->Comp[1] = Gain_Att_Sun->Comp[1]* Att_Err_Sat->Comp[1] + Gain_Rate_Sun->Comp[1] * Rate_Err.Comp[1];
	Tor_BF_Ctrl->Comp[2] = Gain_Att_Sun->Comp[2]* Att_Err_Sat->Comp[2] + Gain_Rate_Sun->Comp[2] * Rate_Err.Comp[2];

   //print_Vector3f("SP: Tor_BF_Ctrl", Tor_BF_Ctrl);
	// Distribute control torque to wheels; limit torques
	Matrix3x3f_MultVec ( Tor_RWA_Cmd, &ALIGN_BF2RWAF, Tor_BF_Ctrl);
	Vector3f_Abs(&TempResult, Tor_RWA_Cmd);
	maxTor_RWA_Cmd = Vector3f_Max(&TempResult);
	//printf("SP: maxTor_RWA_Cmd = %f\n", maxTor_RWA_Cmd);

	if (maxTor_RWA_Cmd >= LIM_RWATOR)
		//Vector3f_Abs(Tor_RWA_Cmd, Tor_RWA_Cmd);
   {
		Vector3f_MultScalar (&TempResult, Tor_RWA_Cmd, LIM_RWATOR);
		Vector3f_DivScalar(Tor_RWA_Cmd, &TempResult, maxTor_RWA_Cmd);
   } 

    Vector3f_Copy(&Unit_Sun_BF_Prev, Unit_Sun_BF);
    Vector3f_Copy(&Unit_Bfield_BF_Prev, &Unit_Bfield_BF);
    Vector3f_Copy(&Tor_RWA_Cmd_Prev, Tor_RWA_Cmd);

ACS_SptTlmPkt.tor_bf_ctrl[0] = Tor_BF_Ctrl->Comp[0];
ACS_SptTlmPkt.tor_bf_ctrl[1] = Tor_BF_Ctrl->Comp[1];
ACS_SptTlmPkt.tor_bf_ctrl[2] = Tor_BF_Ctrl->Comp[2];
ACS_SptTlmPkt.tor_rwa_cmd[0] = Tor_RWA_Cmd->Comp[0];
ACS_SptTlmPkt.tor_rwa_cmd[1] = Tor_RWA_Cmd->Comp[1];
ACS_SptTlmPkt.tor_rwa_cmd[2] = Tor_RWA_Cmd->Comp[2];

return 0;
}
