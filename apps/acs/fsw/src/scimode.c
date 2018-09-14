/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: scimode.c                             */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    the science mode controller    .         */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

/*
** include files
*/
#include "vector3.h"
#include "vector4.h"
#include "matrix3x3.h"
#include "scimode.h"
#include "util.h"
#include "monitor.h"
#include "acs_app.h"
#include "acs_msg.h"

/* external data */
extern acs_sci_tlm_t ACS_SciTlmPkt;

int32_t scimode (
	Vector4f *GFToBF_Est,       //  current best est to gci-to-bcs quat
	Vector4f *LFToTarF_Cmd,     //  delta quat rotating from lvlhF to TarF
	Vector3f *Unit_Pos_GF,      // pos vect from ECIF to s/c
	Vector3f *Unit_Vel_GF,      // vel vect from ECIF to s/c
   Vector3f *Rate_Cmd_Science, // Command adjustment to rate for INMS controller (rad/s)
	Vector3f *Rate_BF_Est,      // Current best esimate of s/c body ang rates (rad/s) 
	Vector3f *AttErr,	         // Attitude error from the controller, rad
	Vector3f *RateErr  	      // Rate error from the controller, rad/sec  
)
{
	Matrix3x3f Dcm_LFToGF;  // direction cos matrix to transform lvlh to gci fram
	//Vector4f Delta_LFwrtGF; // delta quat for lvlh in gci frame
	Vector3f E1, E2, E3;  		  // basis vectors for LVLH in gci frame
	static Vector4f LFToGF_Prev = {{0.0,0.0,0.0,1.0}};   // quat for reexpressing lvlh in gci frame
	Vector4f LFToGF = {{0.0, 0.0, 0.0, 1.0}};        // quat for reexpressing lvlh in gci frame
	Vector4f TarFToGF;	// represents current inertial target quat 
	//Vector3f Rate_LFwrtBF_Est = {{0.0, 0.0, 0.0}};  // estimated rate of lvlh fram relative to bcs
	//Vector4f Rate_LFwrtGF;  // estimated rate of lvlh fram relative to inert ref
	Vector4f TarFToBF;
	Vector4f TarFToLF;

	// determine base vectors defining lvlh frma to construct dcm_lvlhftoGcif
	
	//print_Vector4f("Init: LFToGF_Prev", &LFToGF_Prev);
	Vector3f_MultScalar(&E3, Unit_Pos_GF, -1.0);
	//print_Vector3f("E3", &E3);
	Vector3f_Cross(&E2, &E3, Unit_Vel_GF);
	//print_Vector3f("E2", &E2);
	Vector3f_Normalize (&E2, &E2);
	//print_Vector3f("E2 normal", &E2);
	Vector3f_Cross(&E1, &E2, &E3);
	//print_Vector3f("E1", &E1);
	Matrix3x3f_ColVecToMatrix (&Dcm_LFToGF, &E1, &E2, &E3); 
	print_Matrix3x3f("Dcm_LFToGF", &Dcm_LFToGF);

	// Find the error quat, TarFToBF
	DcmToQuat(&LFToGF, &Dcm_LFToGF);
	//print_Vector4f("LFToGF", &LFToGF);
	Quat_Conj(&TarFToLF, LFToTarF_Cmd);
	//print_Vector4f("TarFToLF", &TarFToLF);
	//Quat_Mult(&TarFToGF, &LFToGF, &TarFToLF);
	Quat_Mult(&TarFToGF, &TarFToLF, &LFToGF);
	//print_Vector4f("TarFToGF", &TarFToGF);
	//Quat_Mult(&TarFToBF, GFToBF_Est, &TarFToGF);
	Quat_Mult(&TarFToBF, &TarFToGF, GFToBF_Est);
	//print_Vector4f("TarFToBF", &TarFToBF);
	Q4Positive(&TarFToBF, &TarFToBF);	
	//print_Vector4f("TarFToBF", &TarFToBF);
		
	//print_Vector4f("LFToGF_Prev before conj", &LFToGF_Prev);
	Quat_Conj(&LFToGF_Prev, &LFToGF_Prev); // renaming overwriting variable
	//print_Vector4f("LFToGF_Prev after conj", &LFToGF_Prev);
	//Quat_Mult(&Delta_LFwrtGF,  &LFToGF_Prev, &LFToGF);
	//Quat_Mult(&Delta_LFwrtGF,  &LFToGF, &LFToGF_Prev);
	//print_Vector4f("Delta_LFwrtGF", &Delta_LFwrtGF);
	
	AttErr->Comp[0] = TarFToBF.Comp[0] * 2.0;
	AttErr->Comp[1] = TarFToBF.Comp[1] * 2.0;
	AttErr->Comp[2] = TarFToBF.Comp[2] * 2.0;

	//Rate_LFwrtGF.Comp[0] = Delta_LFwrtGF.Comp[0] * 2.0/(float)Time_FSW;
	//Rate_LFwrtGF.Comp[1] = Delta_LFwrtGF.Comp[1] * 2.0/(float)Time_FSW;
	//Rate_LFwrtGF.Comp[2] = Delta_LFwrtGF.Comp[2] * 2.0/(float)Time_FSW;
	//Rate_LFwrtGF.Comp[0] = 0;
	//Rate_LFwrtGF.Comp[1] = 0;
	//Rate_LFwrtGF.Comp[2] = 0;
	//Rate_LFwrtGF.Comp[3] = 0.0;
   //print_Vector4f("CT: Rate_LFwrtGF", &Rate_LFwrtGF);

	//Vector4f RLFBFE;
	//Quat_Mult(&RLFBFE, &Rate_LFwrtGF, GFToBF_Est);
	//Quat_Mult(&RLFBFE, GFToBF_Est, &Rate_LFwrtGF);
   //print_Vector4f("RLBFE", &RLFBFE);


	//Rate_LFwrtBF_Est.Comp[0] = RLFBFE.Comp[0];
	//Rate_LFwrtBF_Est.Comp[1] = RLFBFE.Comp[1];
	//Rate_LFwrtBF_Est.Comp[2] = RLFBFE.Comp[2];
	//Rate_LFwrtBF_Est.Comp[0] = 0;
	//Rate_LFwrtBF_Est.Comp[1] = 0;
	//Rate_LFwrtBF_Est.Comp[2] = 0;

	Vector3f_Sub(RateErr, Rate_BF_Est, Rate_Cmd_Science);
	//print_Vector3f("Rate_BF_Est", Rate_BF_Est); 
	//Vector3f_Sub(RateErr, RateErr, &Rate_LFwrtBF_Est);
	#ifdef _ACS_DEBUG_
	print_Vector3f("AttErr", AttErr); 
	//print_Vector3f("Rate_LFwrtBF_Est", &Rate_LFwrtBF_Est);
	print_Vector3f("RateErr", RateErr); 
	#endif 
	Vector4f_Copy (&LFToGF_Prev, &LFToGF);

   ACS_SciTlmPkt.quat_est[0] = GFToBF_Est->Comp[0];
   ACS_SciTlmPkt.quat_est[1] = GFToBF_Est->Comp[1];
   ACS_SciTlmPkt.quat_est[2] = GFToBF_Est->Comp[2];
   ACS_SciTlmPkt.quat_est[3] = GFToBF_Est->Comp[3];
   ACS_SciTlmPkt.rate_est[0] = Rate_BF_Est->Comp[0];
   ACS_SciTlmPkt.rate_est[1] = Rate_BF_Est->Comp[1];
   ACS_SciTlmPkt.rate_est[2] = Rate_BF_Est->Comp[2];
   ACS_SciTlmPkt.sci_raterr[0] = RateErr->Comp[0];
   ACS_SciTlmPkt.sci_raterr[1] = RateErr->Comp[1];
   ACS_SciTlmPkt.sci_raterr[2] = RateErr->Comp[2];
   ACS_SciTlmPkt.sci_atterr[0] = AttErr->Comp[0];
   ACS_SciTlmPkt.sci_atterr[1] = AttErr->Comp[1];
   ACS_SciTlmPkt.sci_atterr[2] = AttErr->Comp[2];

return 0;
}
