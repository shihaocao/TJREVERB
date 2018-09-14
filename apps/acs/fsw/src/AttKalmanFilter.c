/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: AttKalmanFilter.c                     */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    the ACS Kalman Filter functions.         */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/*
**   Include Files:
*/
#include "AttKalmanFilter.h"
#include "vector3.h"
#include "vector4.h"
#include "vector6.h"
#include "matrix3x3.h"
#include "matrix6x6.h"
#include "monitor.h"
#include "util.h"
#include "acs_app.h"
#include "acs_msg.h"

/*
**   exported data
*/
extern acs_ad_tlm_t ACS_AdTlmPkt;

#define TOL_FSSRES0 1.0
#define TOL_FSSRES1 1.0
#define TOL_FSSRES2 1.0
#define TOL_SBMRES0 1.0
#define TOL_SBMRES1 1.0
#define TOL_SBMRES2 1.0
#define LIM_KF_RATEMAG 0.001
#define DELTATIME_AKF 1.0
#define TRIG_COUNT_TRIAD 5

/*
**   constants
*/
Vector3f COVFSSMEASNOIS  = {{0.017, 0.017, 0.017}};
Vector3f COVSBMMEASNOIS  = {{0.017, 0.017, 0.017}};
const Matrix6x6f STNOISCOV  = 
	{{
   {3.3E-9,    0.0,    0.0, -3E-15,    0.0,    0.0},
	{0.0,    3.3E-9,    0.0,    0.0, -3E-15,    0.0},
	{0.0,       0.0, 3.3E-9,    0.0,    0.0, -3E-15},
	{-3E-15,    0.0,    0.0,  6E-15,    0.0,    0.0},
	{   0.0, -3E-15,    0.0,    0.0,  6E-15,    0.0},
	{   0.0,    0.0, -3E-15,    0.0,    0.0,  6E-15}, 
   }};

/* * * * * * * *  * * * * * * * */
/*                              */ 
/* Triad Solution               */
/*                              */ 
/* * * * * * * *  * * * * * * * */
/* The AKF must be initialized with an approximation of the current attitude */
/* quaternion. THis is obtained via the Triad method. The triad quaternion  */
/* is calculated at all times that the FSS Sun vector and Dagr (SBM) B-field */
/* vector are both available. When the AKF is enabled, a sufficient number   */
/* of consistent Triad solutions must be obtained to guarantee that data     */
/* dropouts will not affect the AKF; when this persistence trigger is met,   */
/* the Triad solution is used to initialize the AKF and from that point, the */
/* AKF provides the attitude solution and rate bias estimates.               */
/* When the s/c is in umbra (i.e., eclipse), the AKF can continue to         */
/* estimate updates, but the lack of Sun vector information will allow the   */
/* solution to drift.  If this is severe enough, the AKF will diverge when   */
/* the Sun vector is once again introduced. To prevent this, the AKF is      */
/* reinitialized with Triad every time the perisistence trigger is reached   */
/* by the Triad counter */

int32_t TriadSolution ( 
	uint32_t AKF_Enable,       
	Vector3f *Bfield_BF_SBM,			
	Vector3f *Bfield_GF,			
   uint32_t UseFSS_AKF,
   uint32_t UseSBM_AKF,
   Vector3f *Unit_Sun_BF_FSS,
   Vector3f *Unit_Sun_GF,
   // outputs
   uint32_t *AKF_FirstCycle, 
   uint32_t *Count_Triad,       // Count for persistence of Triad Solution   
   Vector4f *GFToBF_Est_Triad, // Current estimate GF to BF Quaternion,
   Vector4f *GFToBF_AKF_Init,  // intial estimate for AKF 
   uint32_t *Run_AKF           // flag indication the AKF is running
   )
{
   
   
	Vector3f BF2, BF3 = {{0.0, 0.0, 0.0}};
	Vector3f GF1, GF2, GF3 = {{0.0, 0.0, 0.0}};
   Vector3f Unit_BF2 = {{0.0, 0.0, 0.0}};
   Vector3f Unit_GF2 = {{0.0, 0.0, 0.0}};
   Matrix3x3f Dcm_GFToBF_Triad;
   float TempSum[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
   float GF1T[3], GF2T[3], GF3T[3];
   Vector3f Unit_Bfield_BF; 

   //*Count_Triad = 0;
   //Vector4f_InitZero(GFToBF_AKF_Init);
	Matrix3x3f_InitZero(&Dcm_GFToBF_Triad);

  	#ifdef _ACS_DEBUG_
	printf("AKF Use: SBM %u, FSS %u , AKF_Enable %u\n", UseSBM_AKF, UseFSS_AKF, AKF_Enable);
	#endif
   
   if (UseSBM_AKF && UseFSS_AKF && AKF_Enable)
	{ 
     //printf("UseSBMAF & UseFSSAF && AKEnable\n");
     *Count_Triad = *Count_Triad + 1;
     printf("CountTriad = %d\n", *Count_Triad);
     //print_Vector3f("Bfield_GF", Bfield_GF);
     //print_Vector3f("Bfield_BF_SBM", Bfield_BF_SBM);
     //print_Vector3f("Unit_Sun_BF_FSS", Unit_Sun_BF_FSS);
     //print_Vector3f("Unit_Sun_GF", Unit_Sun_GF);
	  Vector3f_Normalize(&Unit_Bfield_BF, Bfield_BF_SBM); 
     Vector3f_Cross(&BF2, &Unit_Bfield_BF, Unit_Sun_BF_FSS); 
	  Vector3f_Normalize(&Unit_BF2, &BF2); 
     Vector3f_Cross(&BF3, &Unit_Bfield_BF, &Unit_BF2);   
     Vector3f_Normalize(&GF1, Bfield_GF);
     Vector3f_Cross(&GF2, &GF1, Unit_Sun_GF);  
	  Vector3f_Normalize(&Unit_GF2, &GF2); 
     Vector3f_Cross(&GF3, &GF1, &Unit_GF2);  
	  MatrixMxNf_Transpose(GF1T, (float *)&GF1, 3, 1);
	  MatrixMxNf_Transpose(GF2T, (float *)&GF2, 3, 1);
	  MatrixMxNf_Transpose(GF3T, (float *)&GF3, 3, 1);
	  MatrixMxNf_Mult(TempSum, (float *)&Unit_Bfield_BF, 3, 1, GF1T, 1, 3); 
	  Matrix3x3f_Add(&Dcm_GFToBF_Triad, &Dcm_GFToBF_Triad, (Matrix3x3f *)TempSum);
	  MatrixMxNf_Mult(TempSum, (float *)&BF2, 3, 1, GF2T, 1, 3); 
	  Matrix3x3f_Add(&Dcm_GFToBF_Triad, &Dcm_GFToBF_Triad, (Matrix3x3f *)TempSum);
	  MatrixMxNf_Mult(TempSum, (float *)&BF3, 3, 1, GF3T, 1, 3); 
	  Matrix3x3f_Add(&Dcm_GFToBF_Triad, &Dcm_GFToBF_Triad, (Matrix3x3f *)TempSum);
     DcmToQuat(GFToBF_Est_Triad, &Dcm_GFToBF_Triad); 
	
	} else  {
     *Count_Triad = 0;
     printf("CountTriad Reset = %d\n", *Count_Triad);
	}
   if (*Count_Triad == TRIG_COUNT_TRIAD) 
   { 
      Vector4f_Copy(GFToBF_AKF_Init, GFToBF_Est_Triad); 
   	if (*Run_AKF == 0) 
   	{
     	 *Run_AKF = 1;
     	 *AKF_FirstCycle = 1;
     	 printf("Run_AKF in Triad = %d, AKF_FirstCycle= %d\n", *Run_AKF, *AKF_FirstCycle);
   	}
   	else {
     	 *Run_AKF = 1;
     	 *AKF_FirstCycle = 1;
     	 printf("Run_AKF in Triad = %d, AKF_FirstCycle= %d\n", *Run_AKF, *AKF_FirstCycle);
     	 }
   }
    
   return 0;
}

/* * * * * * * * * * * * * * * * * */
/*                                 */ 
/* Attitude Kalman Filter Solution */
/*                                 */ 
/* * * * * * * * * * * * * * * * * */
int32_t AttSolution ( 
	uint32_t *AKF_FirstCycle,       
	uint32_t Run_AKF,			
   Vector3f *Rate_BF_Gyro,
	uint32_t Use_FSS_AKF,			     
	uint32_t Use_SBM_AKF,			     
	Vector3f *Bfield_BF_SBM,	     
	Vector3f *Bfield_GF,       
	Vector3f *Unit_Sun_BF_FSS,      
	Vector3f *Unit_Sun_GF, 
   Vector4f *GFToBF_AKF_Init,
	// outputs
   Vector4f *GFToBF_Est,
   Vector3f *Rate_BF_Est)
{
	
   static Vector3f Bias_AKF_Est = {{0.0, 0.0, 0.0}};
	Vector4f GFToBF_Est_AKF; 
	Vector4f GFToBF_Est_Rate = {{0.0, 0.0, 0.0, 0.0}};
	static Vector6f XF;
	static Matrix6x6f P; 
	Vector3f Rate_BF_AKF;
	uint32_t FSS_FilterDataGood;
	uint32_t SBM_FilterDataGood;
	static Vector6f HxFSS; 			
	static Vector6f HyFSS; 			
	static Vector6f HzFSS; 			
	Vector6f HxSBM; 			
	Vector6f HySBM; 			
	Vector6f HzSBM; 			
	static Vector3f Res_FSS;
	static Vector3f Res_SBM;		

    //	Matrix6x6f_InitZero(&P);
   if (*AKF_FirstCycle == 1) {
		Matrix6x6f_InitZero(&P);
	   Vector3f_InitZero(&Rate_BF_AKF);
   	#ifdef _ACS_DEBUG_
      	//print_Vector4f("GFToBF_Est first", GFToBF_Est);
		#endif
      Vector4f_Copy(GFToBF_Est, GFToBF_AKF_Init);
		//*AKF_FirstCycle = 0;
	}
   /*else 
	{
    GFToBF_Est->Comp[0] = 1;
    GFToBF_Est->Comp[1] = 0;
    GFToBF_Est->Comp[2] = 0;
    GFToBF_Est->Comp[3] = 0;
	}
   */

   //print_Vector4f("Prior: GFToBF_Est", GFToBF_Est);
    
	Vector3f_Add(&Rate_BF_AKF, Rate_BF_Gyro, &Bias_AKF_Est);
	QuatRateProp(GFToBF_Est, &Rate_BF_AKF, &GFToBF_Est_Rate );

   #ifdef _ACS_DEBUG_
   print_Vector4f("AS: GFToBF_Est", GFToBF_Est);
   //print_Vector3f("AS: Rate_BF_AKF", &Rate_BF_AKF);
   print_Vector4f("AS: GFToBF_Est_Rate", &GFToBF_Est_Rate);
	#endif

	// setup for KF by computing measurement residuals
		
	if  (Run_AKF == 1)
	{
		// Call AKFilterProcess
		AKF_PreProcess(
   	Use_FSS_AKF, Use_SBM_AKF, &GFToBF_Est_Rate, 
   	Bfield_BF_SBM, Bfield_GF, Unit_Sun_BF_FSS,      
   	Unit_Sun_GF, &HxFSS, &HyFSS, &HzFSS, &HxSBM, &HySBM, &HzSBM,        
   	&Res_FSS, &Res_SBM, &FSS_FilterDataGood, &SBM_FilterDataGood);   

      #ifdef _ACS_DEBUG_ 
      	print_Vector3f("AKF_PreProc: Res_FSS", &Res_FSS);
      	print_Vector3f("AKF_PreProc: Res_SBM", &Res_SBM);
      #endif
	
		ACS_AdTlmPkt.res_fss[0] = Res_FSS.Comp[0];
		ACS_AdTlmPkt.res_fss[1] = Res_FSS.Comp[1];
		ACS_AdTlmPkt.res_fss[2] = Res_FSS.Comp[2];

		ACS_AdTlmPkt.res_dagr[0] = Res_SBM.Comp[0];
		ACS_AdTlmPkt.res_dagr[1] = Res_SBM.Comp[1];
		ACS_AdTlmPkt.res_dagr[2] = Res_SBM.Comp[2];

		// select AD source; AD source is rate propagation for thruster modes
     //printf("AKF_FirstCycle before AKF_AttDet %d\n", *AKF_FirstCycle);

		AKF_AttDet( AKF_FirstCycle, &Rate_BF_AKF, 
         &P, &XF, FSS_FilterDataGood, SBM_FilterDataGood, 
         &HxFSS, &HyFSS, &HzFSS, &HxSBM, &HySBM, &HzSBM, 
         &Res_FSS, &Res_SBM, 
         &GFToBF_Est_Rate, &GFToBF_Est_AKF, &Bias_AKF_Est);
		Vector4f_Copy(GFToBF_Est, &GFToBF_Est_AKF);
		Vector3f_Copy(Rate_BF_Est, &Rate_BF_AKF);
      
      #ifdef _ACS_DEBUG_ 
      	//print_Vector4f("AKF: GFToBF_Est after AttDet", GFToBF_Est);
      	//print_Vector3f("AKF: Rate_BF_Est after AttDet", Rate_BF_Est);
      #endif
	}
	 else
	{
		Vector4f_Copy(GFToBF_Est, &GFToBF_Est_Rate);
		Vector3f_Copy(Rate_BF_Est, Rate_BF_Gyro);
      #ifdef _ACS_DEBUG_ 
      	print_Vector4f("AKF: GFToBF_Est no Run_AKF", GFToBF_Est);
      	print_Vector3f("AKF: Rate_BF_Est no Run_AKF", Rate_BF_Est);
      #endif
	}

	return 0;
}

/* * * * * * * *  * * * * * * * */
/*                              */ 
/* Quaternion Rate Propagation  */
/*                              */ 
/* * * * * * * *  * * * * * * * */
int32_t QuatRateProp(Vector4f *GFToBF_Est,   
			 Vector3f *Rate_BF_AKF, Vector4f *GFToBF_Est_Rate)
	{

	float Ang;			
	float Cos_Term;	
	float Sin_Term;
	Vector4f Quat_Exp; 
	float RateMag; 		

   //print_Vector3f("QRP: Rate_BF_AKF in QRP", Rate_BF_AKF);
   //print_Vector4f("QRP: GFToBF_Est in QRP", GFToBF_Est);
	//RateMag = D2R * Vector3f_Magnitude(Rate_BF_AKF);
	RateMag = Vector3f_Magnitude(Rate_BF_AKF);
   //printf("QRP: RateMag %f\n", RateMag);
	Ang = RateMag * DELTATIME_AKF;
	Cos_Term = cos(Ang/2.0);
	Sin_Term = sin(Ang/2.0)/RateMag;
	{	
	Quat_Exp.Comp[0] = Cos_Term*GFToBF_Est->Comp[0] + 
				Sin_Term*GFToBF_Est->Comp[1]*Rate_BF_AKF->Comp[2] - 
				Sin_Term*GFToBF_Est->Comp[2]*Rate_BF_AKF->Comp[1] + 
				Sin_Term*GFToBF_Est->Comp[3]*Rate_BF_AKF->Comp[0] ;

	Quat_Exp.Comp[1] = -Sin_Term*GFToBF_Est->Comp[0]*Rate_BF_AKF->Comp[2] + 
					Cos_Term*GFToBF_Est->Comp[1] + 
					Sin_Term*GFToBF_Est->Comp[2]*Rate_BF_AKF->Comp[0] + 
					Sin_Term*GFToBF_Est->Comp[3]*Rate_BF_AKF->Comp[1];  
   
	Quat_Exp.Comp[2] = Sin_Term*GFToBF_Est->Comp[0]*Rate_BF_AKF->Comp[1] - 
					Sin_Term*GFToBF_Est->Comp[1]*Rate_BF_AKF->Comp[0] + 
					Cos_Term*GFToBF_Est->Comp[2] + 
					Sin_Term*GFToBF_Est->Comp[3]*Rate_BF_AKF->Comp[2];  
	
	Quat_Exp.Comp[3] = -Sin_Term*GFToBF_Est->Comp[0]*Rate_BF_AKF->Comp[0] - 
				Sin_Term*GFToBF_Est->Comp[1]*Rate_BF_AKF->Comp[1] - 
				Sin_Term*GFToBF_Est->Comp[2]*Rate_BF_AKF->Comp[2] + 
				Cos_Term*GFToBF_Est->Comp[3];

	
	}

	//print_Vector4f("QRP: Quat_Exp", &Quat_Exp);
	Quat_Norm(&Quat_Exp); 
	//print_Vector4f("QRP: Quat_Exp", &Quat_Exp);
	Vector4f_Copy(GFToBF_Est_Rate, &Quat_Exp); 
   //print_Vector4f("QRP: GFToBF_Est_Rate", GFToBF_Est_Rate);
   
	return 0;
}

/* * * * * * * *  * * * * * * * * * * */
/*                                    */ 
/* Attitude Kalman Filter PreProcess  */
/*                                    */ 
/* * * * * * * *  * * * * * * * * * * */
int32_t AKF_PreProcess(
	// inputs
	uint32_t Use_FSS_AKF,			     // Use the FSS in the AKF
	uint32_t Use_SBM_AKF,			     // Use the SBM in the AKF
	Vector4f *GFToBF_Est_Rate, // current estimate of GF to BF
	Vector3f *Bfield_BF_SBM,	     //Bfield in BcSF measured by SBM
	Vector3f *Bfield_GF,       //Bfield in GF from model
	Vector3f *Unit_Sun_BF_FSS,      // unit sun vec from FSS
	Vector3f *Unit_Sun_GF,     // unit sun vec in GF
	// outputs
	Vector6f *HxFSS, 			// X-axis meas matrix for FSS  
	Vector6f *HyFSS, 			// Y-axis meas matrix for FSS  
	Vector6f *HzFSS, 			// Z-axis meas matrix for FSS  
	Vector6f *HxSBM, 			// X-axis meas matrix for SBM  
	Vector6f *HySBM, 			// Y-axis meas matrix for SBM  
	Vector6f *HzSBM, 			// Z-axis meas matrix for SBM  
	Vector3f *Res_FSS,		// difference between FSS and expected FSS	
	Vector3f *Res_SBM,		// difference between SBM and expected SBM
	uint32_t *FSS_FilterDataGood, // FSS Data good for KF
	uint32_t *SBM_FilterDataGood  // SBM Data good for KF
	)
	{
	

	Matrix3x3f Dcm_GFToBF; // rotation mat GF to BF
	Vector3f Unit_Bfield_BF_Exp; // expected unit Bfield in BF	
	Vector3f Unit_Bfield_BF_SBM; // measured SBM in Bfield
	Vector3f Unit_Bfield_GF;  // unit b-field in GF
	Vector3f Unit_Sun_BF_Exp;  // expected sun vector in the BF

	// begin AKFilterProcess

	// Calculated SBM residuals 
	// further down determine whether to use the measuements
	if (Use_SBM_AKF) 
	{
      #ifdef _ACS_DEBUG_ 
		//printf("AKF: Using SBM_AKF\n"); 
	   #endif
		// computed expected Bfield unit vect in BF
		Vector3f_Normalize(&Unit_Bfield_BF_SBM, Bfield_BF_SBM); 
		Vector3f_Normalize(&Unit_Bfield_GF, Bfield_GF); 
		QuatToMatrix(&Dcm_GFToBF, GFToBF_Est_Rate);
		Matrix3x3f_MultVec(&Unit_Bfield_BF_Exp, &Dcm_GFToBF, &Unit_Bfield_GF);
	
		// calculate FSS residuals and H matrix
		Vector3f_Sub (Res_SBM, &Unit_Bfield_BF_SBM,  &Unit_Bfield_BF_Exp);
		//print_Vector3f("Res_SBM", Res_SBM); 
	
		HxSBM->Comp[0] = 0;
		HxSBM->Comp[1] = -Unit_Bfield_BF_Exp.Comp[2];
		HxSBM->Comp[2] = Unit_Bfield_BF_Exp.Comp[1];
		HxSBM->Comp[3] = 0;
		HxSBM->Comp[4] = 0;
		HxSBM->Comp[5] = 0;
	
		HySBM->Comp[0] = Unit_Bfield_BF_Exp.Comp[2];
		HySBM->Comp[1] = 0;
		HySBM->Comp[2] = -Unit_Bfield_BF_Exp.Comp[0];
		HySBM->Comp[3] = 0;
		HySBM->Comp[4] = 0;
		HySBM->Comp[5] = 0;
	
		HzSBM->Comp[0] = -Unit_Bfield_BF_Exp.Comp[1];
		HzSBM->Comp[1] = Unit_Bfield_BF_Exp.Comp[0];
		HzSBM->Comp[2] = 0; 
		HzSBM->Comp[3] = 0;
		HzSBM->Comp[4] = 0;
		HzSBM->Comp[5] = 0;


	// check residuals are good for use in AKF 
	// by checking they are below tolerance
		if ((fabs(Res_SBM->Comp[0]) <= TOL_SBMRES0) && 
			(fabs(Res_SBM->Comp[1]) <= TOL_SBMRES1) && 
			(fabs(Res_SBM->Comp[2]) <= TOL_SBMRES2)) 

			*SBM_FilterDataGood = 1;
		else
			*SBM_FilterDataGood = 0;
	}
	else
			*SBM_FilterDataGood = 0;
	// retain previous SBM residuals and H matrix

	// calculated FSS residuals

	if (Use_FSS_AKF)
	{
		// compute expected sun unit vec in BF
		QuatToMatrix(&Dcm_GFToBF, GFToBF_Est_Rate);
		Matrix3x3f_MultVec(&Unit_Sun_BF_Exp, &Dcm_GFToBF, Unit_Sun_GF);
		
		// calculate FSS residuals and H matrix
		Vector3f_Sub(Res_FSS, Unit_Sun_BF_FSS, &Unit_Sun_BF_Exp);
      #ifdef _ACS_DEBUG_ 
		//printf("AKF: Using FSS_AKF\n"); 
      //print_Vector3f("AKF: Unit_Sun_BF_Exp", &Unit_Sun_BF_Exp);
      //print_Vector3f("AKF: Res_FSS", Res_FSS);
	   #endif

	   // check residuals are good for use in AKF by checking vs tolerance	
		if ((fabs(Res_FSS->Comp[0]) <= TOL_FSSRES0) && 
			(fabs(Res_FSS->Comp[1]) <= TOL_FSSRES1) && 
			(fabs(Res_FSS->Comp[2]) <= TOL_FSSRES2)) 
			*FSS_FilterDataGood = 1;
		else
			*FSS_FilterDataGood = 0;
		
		HxFSS->Comp[0] = 0;
		HxFSS->Comp[1] = -Unit_Sun_BF_Exp.Comp[2];
		HxFSS->Comp[2] = Unit_Sun_BF_Exp.Comp[1];
		HxFSS->Comp[3] = 0;
		HxFSS->Comp[4] = 0;
		HxFSS->Comp[5] = 0;
      //print_Vector6f("HxFSS", HxFSS);
	
		HyFSS->Comp[0] = Unit_Sun_BF_Exp.Comp[2];
		HyFSS->Comp[1] = 0;
		HyFSS->Comp[2] = -Unit_Sun_BF_Exp.Comp[0];
		HyFSS->Comp[3] = 0;
		HyFSS->Comp[4] = 0;
		HyFSS->Comp[5] = 0;
      //print_Vector6f("HyFSS", HyFSS);
	
		HzFSS->Comp[0] = -Unit_Sun_BF_Exp.Comp[1];
		HzFSS->Comp[1] = Unit_Sun_BF_Exp.Comp[0];
		HzFSS->Comp[2] = 0;
		HzFSS->Comp[3] = 0;
		HzFSS->Comp[4] = 0;
		HzFSS->Comp[5] = 0;
      //print_Vector6f("HzFSS", HzFSS);

	}
	else
	{
		// retain previous SBM residuals and H matrix
		*FSS_FilterDataGood = 0;
	
	}


	// end AKFilterProcess
return 0;

}

/* * * * * * * *  * * * * * * * * * * * * */
/*                                        */ 
/* Attitude Kalman Attitude Determination */
/*                                        */ 
/* * * * * * * *  * * * * * * * * * * * * */
int32_t AKF_AttDet( 
		uint32_t *AKF_FirstCycle, Vector3f *Rate_BF_AKF, 
		Matrix6x6f *P, Vector6f *XF, 
      uint32_t FSS_FilterDataGood, uint32_t SBM_FilterDataGood, 
      Vector6f *HxFSS, Vector6f *HyFSS, Vector6f *HzFSS, 
      Vector6f *HxSBM, Vector6f *HySBM, Vector6f *HzSBM,
		Vector3f *Res_FSS, Vector3f *Res_SBM, 
      Vector4f *GFToBF_Est_Rate, 
		Vector4f *GFToBF_Est_AKF, 
      Vector3f *Bias_AKF_Est)
		
	{
   //printf("AKF_FirstCycle inside AKF_AttDet %d\n", *AKF_FirstCycle);

	// propagate error variance, P, to update time..
	AKF_Cov_Propagate(AKF_FirstCycle, Rate_BF_AKF, P, XF);
	//print_Matrix6x6f("P after AKF_CP", P);
	AKF_Main( FSS_FilterDataGood, SBM_FilterDataGood,
			HxFSS, HyFSS, HzFSS, HxSBM, HySBM, HzSBM,
			Res_FSS, Res_SBM, P, XF);
	//print_Matrix6x6f("P after AKF_Main", P);
   //print_Vector6f("XF after AKF_Main", XF);
    
	AttUpdate(GFToBF_Est_Rate, XF, GFToBF_Est_AKF, Bias_AKF_Est);
   #ifdef _ACS_DEBUG_ 
	//print_Matrix6x6f("P after Propagate", P);
   //print_Vector6f("XF after propagate", XF);
	//print_Vector4f("AttUp: GFToBF_Est_AKF", GFToBF_Est_AKF);
   print_Vector3f("AttUp: Bias_AKF_Est", Bias_AKF_Est);
	#endif

	return 0;
   }

// *********************************************
int32_t AKF_Cov_Propagate(
		uint32_t *AKF_FirstCycle, 
		Vector3f *Rate_BF_AKF, // s/c measured rate rad/sec
		Matrix6x6f *P,			// Error cov matrix [rad^2 rad^2/sec ^2 
		Vector6f *XF)			// Optimal estimate of KF state
	{

		Matrix3x3f A; // skew matrix of s/c rate
		Matrix3x3f A2; // A^2
		float C0, C1, C2, C3; // used to compute STM
		float RateMag;
		Matrix6x6f STM, STMTR; // state trans matrix
		Matrix3x3f STM1, STM2, Temp;
	
      #ifdef _ACS_DEBUG_ 
		//printf("AKF_FirstCycle %d\n", (int) AKF_FirstCycle); 
		#endif
		Matrix6x6f_InitIdentity(&STM);
		Matrix3x3f_InitIdentity(&STM2);
	
		// begin AKfilter_Cov_Prop
	
		// initialize P when AKF first started	
		if (*AKF_FirstCycle == 1 )
		{
		 Matrix6x6f_InitIdentity(P);
		//print_Matrix6x6f("P init in Cov_Propagate ", P);
       *AKF_FirstCycle = 0;
		}
		
		// create the STM by computing mag of s/c rate
		RateMag = Vector3f_Magnitude(Rate_BF_AKF);
      //printf("RateMag = %f\n", RateMag);
		//RateMag = RateMag * D2R;
	
		// if RateMag less the specified limit, for stm from zero rate
		if (RateMag <= LIM_KF_RATEMAG)
		{ 
			STM.Comp[0][3] = -DELTATIME_AKF;
			STM.Comp[1][4] = -DELTATIME_AKF;
			STM.Comp[2][5] = -DELTATIME_AKF;
		}
		else
		{
			A.Comp[0][0] = 0;
			A.Comp[0][1] = Rate_BF_AKF->Comp[2];
			A.Comp[0][2] = -Rate_BF_AKF->Comp[1];
			A.Comp[1][0] = -Rate_BF_AKF->Comp[2];
			A.Comp[1][1] = 0;
			A.Comp[1][2] = Rate_BF_AKF->Comp[0];
			A.Comp[2][0] = Rate_BF_AKF->Comp[1];
			A.Comp[2][1] = -Rate_BF_AKF->Comp[0];
			A.Comp[2][2] = 0;

		   //print_Matrix3x3f("A in CovProp", &A);
		
			Matrix3x3f_Mult(&A2, &A, &A);
			C0 = RateMag*DELTATIME_AKF;
			C1 = sin(C0)/RateMag;
			C2 = (1-cos(C0))/(RateMag*RateMag);
			C3 = (DELTATIME_AKF - C1)/(RateMag*RateMag);
			Matrix3x3f_MultScalar(&STM1, &A, C1);
			//Matrix3x3f_MultScalar(&STM1, &A2, C2);
			Matrix3x3f_MultScalar(&Temp, &A2, C2);
			Matrix3x3f_Add(&STM1, &STM1, &Temp);
			STM1.Comp[0][0] = STM1.Comp[0][0] + 1.0;
			STM1.Comp[1][1] = STM1.Comp[1][1] + 1.0;
			STM1.Comp[2][2] = STM1.Comp[2][2] + 1.0;
		   //print_Matrix3x3f("STM1", &STM1);
	
			Matrix3x3f_MultScalar(&STM2, &STM2, DELTATIME_AKF) ;
			Matrix3x3f_MultScalar(&Temp, &A, C2); 
			Matrix3x3f_Add(&STM2, &STM2, &Temp);
			Matrix3x3f_MultScalar(&Temp, &A2, C3);
			Matrix3x3f_Add(&STM2, &STM2, &Temp);
			Matrix3x3f_MultScalar(&STM2, &STM2, -1.0) ;
		   //print_Matrix3x3f("STM2", &STM2);
		
			// initialize STM
			STM.Comp[0][0] = STM1.Comp[0][0];
			STM.Comp[0][1] = STM1.Comp[0][1];
			STM.Comp[0][2] = STM1.Comp[0][2];
			STM.Comp[0][3] = STM2.Comp[0][0];
			STM.Comp[0][4] = STM2.Comp[0][1];
			STM.Comp[0][5] = STM2.Comp[0][2];
			STM.Comp[1][0] = STM1.Comp[1][0];
			STM.Comp[1][1] = STM1.Comp[1][1];
			STM.Comp[1][2] = STM1.Comp[1][2];
			STM.Comp[1][3] = STM2.Comp[1][0];
			STM.Comp[1][4] = STM2.Comp[1][1];
			STM.Comp[1][5] = STM2.Comp[1][2];
			STM.Comp[2][0] = STM1.Comp[2][0];
			STM.Comp[2][1] = STM1.Comp[2][1];
			STM.Comp[2][2] = STM1.Comp[2][2];
			STM.Comp[2][3] = STM2.Comp[2][0];
			STM.Comp[2][4] = STM2.Comp[2][1];
			STM.Comp[2][5] = STM2.Comp[2][2];
		   //print_Matrix6x6f("STM", &STM);
		}
		// propagate est error cov matrix
		Matrix6x6f_Transpose (&STMTR, &STM);
		//print_Matrix6x6f("STMTR", &STMTR);
		Matrix6x6f_Mult(P, P, &STMTR);
		Matrix6x6f_Mult(P, &STM, P);
		Matrix6x6f_Add(P, P, &STNOISCOV);
		//print_Matrix6x6f("P in Cov_Propagate ", P);
		// initialize XF with 0's to begin update
		XF->Comp[0] = 0;
		XF->Comp[1] = 0;
		XF->Comp[2] = 0;
		XF->Comp[3] = 0;
		XF->Comp[4] = 0;
		XF->Comp[5] = 0;
	
	// end AKfilter_Cov_Prop
	return 0;
	}	

// *********************************************
int32_t AKF_Main(
	uint32_t FSS_FilterDataGood, 
	uint32_t SBM_FilterDataGood, 
	Vector6f *HxFSS,
	Vector6f *HyFSS, 			// Y-axis meas matrix for FSS  
	Vector6f *HzFSS, 			// Z-axis meas matrix for FSS  
	Vector6f *HxSBM, 			// X-axis meas matrix for SBM  
	Vector6f *HySBM, 			// Y-axis meas matrix for SBM  
	Vector6f *HzSBM, 			// Z-axis meas matrix for SBM  
	Vector3f *Res_FSS,		// difference between FSS and expected FSS	
	Vector3f *Res_SBM,		// difference between SBM and expected SBM
	Matrix6x6f *P, 		  // Error cov matrix 
	Vector6f *XF  			  // Optimal esimate of KF state [att err; drift rate]
	)			
	{
		float DDX=0, DDY=0, DDZ=0;
		float ZMx, ZMy, ZMz;


		// Begin AKFMain
		// store data in KF format, call KF and store res in Res_Stl_Adj
		if (FSS_FilterDataGood) 
		{
			ZMx = Res_FSS->Comp[0];
			ZMy = Res_FSS->Comp[1];
			ZMz = Res_FSS->Comp[2];
			//Call 
			KalmanFilter(HxFSS, HyFSS, HzFSS, ZMx, ZMy, ZMz, &COVFSSMEASNOIS,
         P, XF, DDX, DDY, DDZ);			
			//print_Matrix6x6f("P after KF FSS_Good", P);
      	//print_Vector6f("XF after KF FSS_Good", XF);
         
		}
		// don't used FSS in KF update

		// store data in KF format, call KF and store res in Res_SBM_Adj
		if (SBM_FilterDataGood) 
			{
			ZMx = Res_SBM->Comp[0];
			ZMy = Res_SBM->Comp[1];
			ZMz = Res_SBM->Comp[2];
			// Call 
			KalmanFilter(HxSBM, HySBM, HzSBM, ZMx, ZMy, ZMz, &COVSBMMEASNOIS, 
            P, XF, DDX, DDY, DDZ);
			//print_Matrix6x6f("P after KF SBM_Good", P);
      	//print_Vector6f("XF after KF SBM_Good", XF);
			}
		// don't used SBM in KF update

   ACS_AdTlmPkt.state_update[0] = XF->Comp[0];
   ACS_AdTlmPkt.state_update[1] = XF->Comp[1];
   ACS_AdTlmPkt.state_update[2] = XF->Comp[2];
   ACS_AdTlmPkt.state_update[3] = XF->Comp[3];
   ACS_AdTlmPkt.state_update[4] = XF->Comp[4];
   ACS_AdTlmPkt.state_update[5] = XF->Comp[5];

   ACS_AdTlmPkt.cov_diag[0] = P->Comp[0][0]; 
   ACS_AdTlmPkt.cov_diag[1] = P->Comp[0][1]; 
   ACS_AdTlmPkt.cov_diag[2] = P->Comp[0][2]; 
   ACS_AdTlmPkt.cov_diag[3] = P->Comp[0][3]; 
   ACS_AdTlmPkt.cov_diag[4] = P->Comp[0][4]; 
   ACS_AdTlmPkt.cov_diag[5] = P->Comp[0][5]; 
   ACS_AdTlmPkt.cov_diag[6] = P->Comp[1][1]; 
   ACS_AdTlmPkt.cov_diag[7] = P->Comp[1][2]; 
   ACS_AdTlmPkt.cov_diag[8] = P->Comp[1][3]; 
   ACS_AdTlmPkt.cov_diag[9] = P->Comp[1][4]; 
   ACS_AdTlmPkt.cov_diag[10] = P->Comp[1][5]; 
   ACS_AdTlmPkt.cov_diag[11] = P->Comp[2][2]; 
   ACS_AdTlmPkt.cov_diag[12] = P->Comp[2][3]; 
   ACS_AdTlmPkt.cov_diag[13] = P->Comp[2][4]; 
   ACS_AdTlmPkt.cov_diag[14] = P->Comp[2][5]; 
   ACS_AdTlmPkt.cov_diag[15] = P->Comp[3][3]; 
   ACS_AdTlmPkt.cov_diag[16] = P->Comp[3][4]; 
   ACS_AdTlmPkt.cov_diag[17] = P->Comp[3][5]; 
   ACS_AdTlmPkt.cov_diag[18] = P->Comp[4][4]; 
   ACS_AdTlmPkt.cov_diag[19] = P->Comp[4][5]; 
   ACS_AdTlmPkt.cov_diag[20] = P->Comp[5][5]; 

	return 0;
	}

// *********************************************
int32_t AttUpdate(Vector4f *GFToBF_Est_Rate, 
				Vector6f *XF, 
				Vector4f *GFToBF_Est_AKF, 
				Vector3f *Bias_AKF_Est)
	{
		Vector4f QS;
		QS.Comp[0] = XF->Comp[0] * 0.5;
		QS.Comp[1] = XF->Comp[1] * 0.5;
		QS.Comp[2] = XF->Comp[2] * 0.5;
		QS.Comp[3] = 1.0;
	
   
		Quat_Mult(GFToBF_Est_AKF, GFToBF_Est_Rate, &QS); 
		Quat_Norm(GFToBF_Est_AKF); 
		// udpate estimated bias rate in body frame, Bias_AKF_est should be initialized on startup 
	
		Bias_AKF_Est->Comp[0] = Bias_AKF_Est->Comp[0] + XF->Comp[3]; 
		Bias_AKF_Est->Comp[1] = Bias_AKF_Est->Comp[1] + XF->Comp[4]; 
		Bias_AKF_Est->Comp[2] = Bias_AKF_Est->Comp[2] + XF->Comp[5]; 
      #ifdef _ACS_DEBUG_ 
      //print_Vector4f("QS in AttUpdate", &QS);
      //print_Vector3f("Bias_AKF_Est in AttUpdate", Bias_AKF_Est);
		#endif
	 

   ACS_AdTlmPkt.bias_est[0] = Bias_AKF_Est->Comp[0];
   ACS_AdTlmPkt.bias_est[1] = Bias_AKF_Est->Comp[1];
   ACS_AdTlmPkt.bias_est[2] = Bias_AKF_Est->Comp[2];

		return 0;
	}

// *********************************************
int32_t KalmanFilter(
				Vector6f *Hx,  // x-axis measurement matr for input sensor
				Vector6f *Hy,  // y-axis " 
				Vector6f *Hz,  // z-axis "
				float ZMx, 		// X-Comp SBM or FSS data residual 
				float ZMy, 		// Y-comp "
				float ZMz, 		// Z-comp "
				Vector3f *CovMeasNoise,
				// outputs
				Matrix6x6f *P, // I/O Error cov matrix 
				Vector6f *XF, 	// I/O Optimal esimate of KF state [att err; drift rate]
				float DDX, 		// SBM or FSS X-comp data residual adj for reduced certainty
				float DDY, 		// SBM or FSS Y-comp " 
				float DDZ)		// SBM or FSS Z-comp "
	{
		Matrix6x6f P_Temp;
		Vector6f XF_Temp;
		float DX, DY, DZ;
		Vector6f PHx, PHy, PHz;
		// store cov mat P and att err est vector XF
		Vector6f_InitZero(&XF_Temp);
		Matrix6x6f_Copy (&P_Temp, P);
		//print_Matrix6x6f("P_Temp in KalmanFilter", &P_Temp);
		Vector6f_Copy (&XF_Temp, XF);
		//print_Vector6f("XF_Temp in KalmanFilter", &XF_Temp);
		// computer PHx, HXPHXt, a scalar and compute adjusted residual DDX
		// Call KF to update local P_Temp and XF_Temp based on x-axis data
		uint32_t i; 
		for (i=0; i<=5; i++)
		PHx.Comp[i] = P_Temp.Comp[i][0]*Hx->Comp[0] + P_Temp.Comp[i][1]*Hx->Comp[1] 
                    + P_Temp.Comp[i][2]*Hx->Comp[2];
      //print_Vector6f("PHx", &PHx);
		DX = CovMeasNoise->Comp[0] + Hx->Comp[0]*PHx.Comp[0] + Hx->Comp[1]*PHx.Comp[1] 
           + Hx->Comp[2]*PHx.Comp[2];
		DDX = ZMx - (Hx->Comp[0]*XF_Temp.Comp[0] + Hx->Comp[1]*XF_Temp.Comp[1] 
            + Hx->Comp[2]*XF_Temp.Comp[2]);
		Kalman(DDX, &PHx, DX, &P_Temp, &XF_Temp);
      #ifdef _ACS_DEBUG_
		//print_Matrix6x6f("P_Temp after Kalman1", &P_Temp);
		#endif
		
		// Used updated P_Temp, XF_Temp 
		// computer PHy, HYPHYt, a scalar and computed adjusted residual DDY
		// Call KF to update local P_Temp and XF_Temp based on Y-axis data
		for (i=0; i<=5; i++)
		{
			PHy.Comp[i] = P_Temp.Comp[i][0]*Hy->Comp[0] + P_Temp.Comp[i][1]*Hy->Comp[1]  
            + P_Temp.Comp[i][2]*Hy->Comp[2];
		}
  		//print_Vector6f("PHy", &PHy);
		DY = CovMeasNoise->Comp[1] + Hy->Comp[0]*PHy.Comp[0] + Hy->Comp[1]*PHy.Comp[1] 
           + Hy->Comp[2]*PHy.Comp[2];
		DDY = ZMy -  (Hy->Comp[0]*XF_Temp.Comp[0] + Hy->Comp[1]*XF_Temp.Comp[1] 
            + Hy->Comp[2]*XF_Temp.Comp[2]);
		Kalman(DDY, &PHy, DY, &P_Temp, &XF_Temp);
      #ifdef _ACS_DEBUG_ 
		//print_Matrix6x6f("P_Temp after Kalman2", &P_Temp);
		#endif
		// Used updated P_Temp, XF_Temp 
		// computer PHz, HZPHZt, a scalar and computed adjusted residual DDZ
		// Call KF to update local P_Temp and XF_Temp based on Z-axis data
		for (i=0; i<=5; i++)
		{
			PHz.Comp[i] = P_Temp.Comp[i][0]*Hz->Comp[0] + P_Temp.Comp[i][1]*Hz->Comp[1]  
             + P_Temp.Comp[i][2]*Hz->Comp[2];
		}
		//print_Vector6f("PHz", &PHz);
		DZ = CovMeasNoise->Comp[1] + Hz->Comp[0]*PHz.Comp[0] + Hz->Comp[1]*PHz.Comp[1] 
           + Hz->Comp[2]*PHz.Comp[2];
		DDZ = ZMz -  (Hz->Comp[0]*XF_Temp.Comp[0] + Hz->Comp[1]*XF_Temp.Comp[1] 
            + Hz->Comp[2]*XF_Temp.Comp[2]);
		Kalman(DDZ, &PHz, DZ, &P_Temp, &XF_Temp);
      #ifdef _ACS_DEBUG_ 
		//print_Matrix6x6f("P_Temp after Kalman3", &P_Temp);
		#endif
	   //printf("DD XYZ : %f %f %f\n", DDX, DDY, DDZ);
		// Update P and XF with P_Temp and XF_Temp
		Matrix6x6f_Copy (P, &P_Temp);
		Vector6f_Copy (XF, &XF_Temp);
     
	return 0;
	}

// *********************************************
// Kalman Function compute KF gain, update cov and update filter states
int32_t Kalman( float DD,  // SBM of FSS X, Y, Z comp data
				Vector6f *PH,	// Cov matrix * SBM or FSS measurement matrix
				float VAR,	
				Matrix6x6f *P_Temp, // I/O SBM or FSS measurement noise cov + H(*)PH(*)
				Vector6f *XF_Temp)	// I/O Optimate estimate of state)
	{
	float D; 	
	Vector6f K, Y;
	float E[36];
	float PHT[6];

	Matrix6x6f_InitZero(P_Temp);
	//print_Matrix6x6f("P_Temp in Kalman", P_Temp);
	// Compute gain matrix K
	D=1/VAR;
	Vector6f_MultScalar(&K, PH, D);
	//print_Vector6f("K in Kalman", &K);

	// update post-measurement estimation error cov matrix, P_Temp
	MatrixMxNf_Transpose(PHT, (float *)PH, 6, 1);
	MatrixMxNf_Mult(E, (float *)&K, 6, 1, PHT, 1, 6); 
	Matrix6x6f_Sub(P_Temp, P_Temp, (Matrix6x6f *)E);
   #ifdef _ACS_DEBUG_ 
	//print_Matrix6x6f("P_Temp in Kalman", P_Temp);
	#endif

	// Update optimal estimate of state, XF_Temp
	Vector6f_MultScalar(&Y,  &K, DD);
	Vector6f_Add(XF_Temp, XF_Temp, &Y);
	//print_Vector6f("XF_Temp in Kalman", XF_Temp);

	return 0;
	}
