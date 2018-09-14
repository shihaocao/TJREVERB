/*
 * AttKalmanFilter.h
 *
 *  Created on: Feb 5, 2015
 *  Author: Salman Sheikh
 *  NASA-GSFC
 *  Code 564    
 *  Greenbelt, MD 20771   
 */

#include "vector3.h"
#include "vector4.h"
#include "vector6.h"
#include "matrix6x6.h"

#ifndef ATTKALMANFILTER_H_
#define ATTKALMANFILTER_H_

/*
#define D2R M_PI/180.0
#define LIM_HALFTHETA 10.0
#define TOL_FSSRES0 1.0
#define TOL_FSSRES1 1.0
#define TOL_FSSRES2 1.0
#define TOL_SBMRES0 1.0
#define TOL_SBMRES1 1.0
#define TOL_SBMRES2 1.0
#define LIM_KF_RATEMAG 0.001
#define DELTATIME_AKF 1.0

static const Vector3f COVFSSMEASNOIS  = {{1.0, 1.0, 1.0}};
static Vector3f COVSBMMEASNOIS  = {{1.0, 1.0, 1.0}};

static const Matrix6x6f STNOISCOV  = 
	{{
	{0.1, 0.1, 0.1, 0.1, 0.1},
	{0.1, 0.1, 0.1, 0.1, 0.1},
	{0.1, 0.1, 0.1, 0.1, 0.1},
	{0.1, 0.1, 0.1, 0.1, 0.1},
	{0.1, 0.1, 0.1, 0.1, 0.1},
	{0.1, 0.1, 0.1, 0.1, 0.1},
	}};

*/

int32_t AttUpdate(Vector4f *GFToBF_Est_Rate, Vector6f *XF, 
		Vector4f *GFToBF_Est_AKF, Vector3f *Bias_AKF_Est);

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
   Vector4f *Quat_AKF_Init,    // intial attitude estimate for AKF 
   uint32_t *Run_AKF           // flag indication the AKF is running
   );

int32_t AttSolution ( 
	uint32_t *AKF_FirstCycle,
	uint32_t Run_AKF,			// Flag to enable/disable AKF 
   Vector3f *Rate_BF_Gyro,
   uint32_t UseFSS_AKF,             // Use the FSS in the KFF
   uint32_t UseSBM_AKF,             // Use the SBM in the KFF
   Vector3f *Bfield_BF_SBM,        //Bfield in BcSF measured by SBM
   Vector3f *Bfield_GF,       //Bfield in GF from model
   Vector3f *Unit_Sun_BF_FSS,      // unit sun vec from FSS
   Vector3f *Unit_Sun_GF,     // unit sun vec in GF
   Vector4f *GFToBF_AKF_Init,
   // outputs
	Vector4f *GFToBF_Est,    //Current estimate GF to BF Quat  
	Vector3f *Rate_BF_Est  // Current best estimate of s/c body ang rates (rad/s)
  	 );

	int32_t QuatRateProp(
		Vector4f *GFToBF_Est,   
		Vector3f *Rate_BK_AKF, 
		Vector4f *GFToBF_Est_Rate);

	int32_t AKF_PreProcess(
		uint32_t UseFSS_AKF,			     // Use the FSS in the KFF
		uint32_t UseSBM_AKF,			     // Use the SBM in the KFF
		Vector4f *GFToBF_Est_Rate, // current estimate of GF to BF
		Vector3f *Bfield_BF_SBM,	     // Bfield in BcSF measured by SBM
		Vector3f *Bfield_GF,       // Bfield in GF from model
		Vector3f *Unit_Sun_BF_FSS,      // unit sun vec from FSS
		Vector3f *Unit_Sun_GF,     // unit sun vec in GF
	
		Vector6f *HxFSS, 			        // X-axis meas matrix for FSS  
		Vector6f *HyFSS, 			        // Y-axis meas matrix for FSS  
		Vector6f *HzFSS, 			        // Z-axis meas matrix for FSS  
		Vector6f *HxSBM, 			        // X-axis meas matrix for SBM  
		Vector6f *HySBM, 			        // Y-axis meas matrix for SBM  
		Vector6f *HzSBM, 			        // Z-axis meas matrix for SBM  
		Vector3f *Res_FSS,		        // difference between FSS and expected FSS	
		Vector3f *Res_SBM,		        // difference between SBM and expected SBM
		uint32_t *FSS_FilterDataGood,    // FSS Data good for KF
		uint32_t *SBM_FilterDataGood     // SBM Data good for KF
		);

	int32_t AKF_Cov_Propagate(
		uint32_t *AKF_FirstCycle, 
		Vector3f *Rate_BF_AKF, // s/c measured rate rad/sec
		Matrix6x6f *P,			// Error cov matrix [rad^2 rad^2/sec ^2 
		Vector6f *XF);			// Optimal estimate of KF state

	int32_t Kalman( float DD,  // SBM of FSS X, Y, Z comp data
					Vector6f *PH,	// Cov matrix * SBM or FSS measurement matrix
					float VAR,	// error covariance matrix, temp copy 
					Matrix6x6f *P_Temp, // SBM of FSS measurement noise cov plus H(XYZ)PH(XYZ)
					Vector6f *XF_Temp);	// Optimate estimate of state)

	int32_t KalmanFilter(
				Vector6f *Hx,  // x-axis measurement matr for input sensor
				Vector6f *Hy,  // y-axis measurement matr for input sensor
				Vector6f *Hz,  // z-axis measurement matr for input sensor
				float ZMx, 		// SBM or FSS X-comp data residual 
				float ZMy, 		// SBM or FSS Y-comp data residual 
				float ZMz, 		// SBM or FSS Z-comp data residual 
				Vector3f *CovMeasNoise, 
				Matrix6x6f *P, // Error cov matrix 
				Vector6f *XF, 	// Optimal esimate of KF state [att err; drift rate]
				float DDX, 		// SBM or FSS X-comp data residual adj for reduced certainty
				float DDY, 		// SBM or FSS X-comp data residual adj for reduced certainty
				float DDZ);		// SBM or FSS X-comp data residual adj for reduced certainty

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
		//Vector3f *Res_FSS_Adj,	// adjusted residual for FSS
		//Vector3f *Res_SBM_Adj,	// adjusted residual for SBM
		Matrix6x6f *P, 		  // Error cov matrix 
		Vector6f *XF);  			  // Optimal esimate of KF state [att err; drift rate]

	int32_t AKF_AttDet( 
		uint32_t *AKF_FirstCycle, Vector3f *Rate_BF_AKF, 
		Matrix6x6f *P, Vector6f *XF, uint32_t FSS_FilterDataGood, 
		uint32_t SBM_FilterDataGood, Vector6f *HxFSS, Vector6f *HyFSS, 
		Vector6f *HzFSS, Vector6f *HxSBM, Vector6f *HySBM, Vector6f *HzSBM,
		Vector3f *Res_FSS, Vector3f *Res_SBM, //Vector3f *Res_FSS_Adj, Vector3f *Res_SBM_Adj, 
      Vector4f *GFToBF_Est_Rate, 
		Vector4f *GFToBF_Est_AKF, Vector3f *Bias_AKF_Est );

#endif /* ATTKALMANFILTER_H_ */
