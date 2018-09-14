/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: acs_top.c                             */
/*                                             */
/* Purpose:                                    */
/*  This file contains the source code for     */
/*  top acs function (nominally run @ 1Hz)     */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define _GPS_DEBUG_
#define  SIM_DAGRBOOM_DATA
#define  SIM_WHEELS
#define _GYRO_PHASE_TEST_
//#define _FSS_PHASE_TEST_
//#define _GFSS_PHASE_TEST_
//#define _CSS_PHASE_TEST_
//#define _MPU_TEST_
#ifdef _NANOMIND_
 #include <freertos/FreeRTOS.h>
 #include <freertos/task.h>
#endif


/*
** include files:
*/
#include "vector3.h"
#include "vector4.h"
#include "vector6.h"
#include "matrix3x3.h"
#include "matrixmxn.h"
#include "monitor.h"
#include "util.h"
#include "CSS.h"
#include "FSS.h"
#include "gha.h"
#include "new_igrf.h"
#include "nanoMag.h"
#include "dagrMag.h"
#include "AttKalmanFilter.h"
#include "solar_ephemeris.h"
#include "spacecraft_ephemeris.h"
#include "scimode.h"
#include "selectFSS.h"
#include "sunpointmode.h"
#include "magnetic_control.h"
#include "calculate_torques.h"
#include "ReactionWheelTach.h"
#include "SysMom.h"
#include "betangle.h"
#include "gyro.h"
#include "acs_top.h"
#include "acs_app.h"
#include "acs_msg.h"
#include "acs_msgids.h"
#include "dhl_lib.h"
#include "sol_lib.h"
#include "cfe.h"
#include "dagr_msg.h"
#include "rw_msg.h"
#include "ssc_lib.h"
#include "att_lib.h"
//#include <dev/magnetometer.h>

#define LEAP_SECONDS   36
const Matrix3x3f ALIGN_BF2RWAF = {{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, -1.0}}};
const float RAD_EARTH_KM = 6378.136;

/**************************************************
** Macro Definitions
*************************************************/

extern dagr_acs_tlm_t DagrBoomPkt;
extern rw_torque_cmd_t RWTorqueCmdPkt;
extern acs_state_t ACS_State;
extern acs_hk_tlm_t ACS_HkTlmPkt;
extern acs_spt_tlm_t ACS_SptTlmPkt;
extern acs_sci_tlm_t ACS_SciTlmPkt;
extern acs_sdp_tlm_t ACS_SdpTlmPkt;
extern acs_ad_tlm_t ACS_AdTlmPkt;
extern acs_mpu_tlm_t ACS_MpuTlmPkt;

int32_t acs_top(
	Vector4f *Targ_Cmd_Sci,
	float    Rate_Cmd_Sun,
	Vector3f *Rate_Cmd_Sci,
	uint32_t AKF_Enable,
	//uint32_t Run_AKF,
	uint8_t  RW_Config,
	uint8_t  MTC_Config, 
   uint8_t  sc_mode,
   Vector3f *Gain_Att_Sci,
   Vector3f *Gain_Int_Sci,
   Vector3f *Gain_Rate_Sci,
   Vector3f *Gain_Rate_Sun,
   Vector3f *Gain_Att_Sun)
{
	Vector3f ScaledTor_RW_Cmd = {{0.0, 0.0, 0.0}}; 
	Vector3f Moment_MTC_Cmd = {{0.0, 0.0, 0.0}};
	int      Use_FSS_AKF = 0;
	uint32_t Use_SBM_AKF = 0; 
	uint16_t Cnt_CSS[6]={0,0,0,0,0,0};
	uint8_t  CSS_Eclipse;
	uint32_t NMM_Ready = 0, NMM_Valid = 0;
	uint32_t SBM_Valid = 0;
	uint8_t CSS_Pres = 0;
	float   CssF_CSS[6] = {0,0,0,0,0,0};
	int     Valid_wfssY = 0, Valid_gfssY = 0, Valid_gfssZ = 0;
	Vector3f Unit_wfssY = {{0.0, 0.0, 0.0}}; // unit sun vector Wallops FPSS on +Y
	Vector3f Unit_gfssY = {{0.0, 0.0, 0.0}}; // unit sun vector Gomspace FSS on +Y
	Vector3f Unit_gfssZ = {{0.0, 0.0, 0.0}}; // unit sun vector Gomspace FSS on -Z
	Vector3f Unit_Sun_BF = {{0.0, 0.0, 0.0}};
	Vector3f Unit_BF_CSS = {{0.0, 0.0, 0.0}};
	uint8_t ACS_Settling = 0;
	//static uint8_t GPS_Available = 0;
	uint8_t GoodGps = 0;
	//unsigned int  GpsWeek=0; 
	//double GpsSec;
	//uint32_t TimeGPS;
	uint32_t SBM_Ready = 0;
	float latitude=0, longitude=0, altitude=0;
	uint32_t TimeTAI;
	//float TimeTAI;
	static Vector3f BfNMM = {{1.0, 0.0, 0.0}};
	static Vector3f Bfield_BF_NMM = {{0.0, 1.0, 0.0}};
	Vector3f Bfield_BF_SBM = {{1.0, 0.0, 0.0}};	 
	Vector3f Att_Err_Sat, Rate_BF_Sun = {{0.0, 0.0, 0.0}};
	Vector3f Gyro_Rate = {{0.0, 0.0, 0.0}}; 	
	Vector3f Rate_BF_Gyro = {{0.0, 0.0, 0.0}}; 	
	Vector3f AttErr = {{0.0, 0.0, 0.0}};
        Vector3f RateErr = {{0.0, 0.0, 0.0}};
        Vector3f IntErr = {{0.0, 0.0, 0.0}};
	Vector3f AttErr_Lim = {{0.0, 0.0, 0.0}};
        Vector3f IntErr_Lim = {{0.0, 0.0, 0.0}};
        Vector3f RateErr_Lim = {{0.0, 0.0, 0.0}}; 
	Vector3f Tor_RW_Cmd = {{0.0, 0.0, 0.0}}; 
	Vector3f Sci_Tor_RW_Cmd = {{0.0, 0.0, 0.0}};
        Vector3f Sun_Tor_RW_Cmd = {{0.0, 0.0, 0.0}};
	Vector3f Tor_BF_Ctrl = {{0.0, 0.0, 0.0}};
        Vector3f Sun_Tor_BF_Ctrl = {{0.0, 0.0, 0.0}};
        Vector3f Sci_Tor_BF_Ctrl = {{0.0, 0.0, 0.0}};
	Vector3f RwaMom_BF = {{0.0, 0.0, 0.0}}; 	
	Vector3f Speed_RW = {{0.0, 0.0, 0.0}}; 	
	Vector3f Rate_BF_Est = {{0.0, 0.0, 0.0}}; 
	static Vector4f GFToBF_Est = {{1.0, 0.0, 0.0, 1.0}};
	Vector4f LibGF2BF_Est = {{1.0, 0.0, 0.0, 1.0}};
	Vector4f GFToBF_Est_Triad = {{0.0, 0.0, 0.0, 1.0}};
	static Vector4f GFToBF_AKF_Init = {{0.0, 0.0, 0.0, 0.0}};
	static Vector3f Unit_Sun_GF= {{0.0, 0.0, 0.0}};
	static Vector3f VelGpsEF = {{0.0, 0.0, 0.0}};
	static Vector3f PosGpsEF = {{0.0, 0.0, 0.0}};
	static Vector3f VelGpsGF = {{0.0, 0.0, 0.0}};
	static Vector3f PosGpsGF = {{0.0, 0.0, 0.0}};
	Vector3f Bfield_GF = {{0.0, 0.0, 0.0}};
	Matrix3x3f Dcm_EFToGF = {{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}};
	Vector3f SysMom_BF_Calc = {{0.0, 0.0, 0.0}};
	uint8_t  Gyro_Valid = 0;
	int      UseStructFilter = 0;
	Vector3f	Unit_Pos_GF = {{0.0, 0.0, 0.0}}; 	// unit vector of s/c position GF
	Vector3f	Unit_Vel_GF = {{0.0, 0.0, 0.0}}; 	// unit vector of s/c velocity GF
	Vector3f	Unit_Pos_EF = {{0.0, 0.0, 0.0}}; 	// unit vector of s/c position EF
	Vector3f	Unit_Vel_EF = {{0.0, 0.0, 0.0}}; 	// unit vector of s/c velocity EF
	static uint32_t Count_Triad = 0;
	static uint32_t Run_AKF = 0;
	uint32_t AKF_FirstCycle = 0;
   // uint8_t MagCycle = 0;
	float DagrX, DagrY, DagrZ;

	#ifdef _NANOMIND_
	//portTickType time_start = xTaskGetTickCount();
	#endif


// Read in 3 FSS if available
	//if (sc_mode != MODE_SAFE)
	//{
	   SOL_LibReadFSS(0, &Valid_wfssY, &Unit_wfssY.Comp[0], &Unit_wfssY.Comp[1], &Unit_wfssY.Comp[2]);  
	   SOL_LibReadFSS(1, &Valid_gfssY, &Unit_gfssY.Comp[0], &Unit_gfssY.Comp[1], &Unit_gfssY.Comp[2]);  
	   SOL_LibReadFSS(2, &Valid_gfssZ, &Unit_gfssZ.Comp[0], &Unit_gfssZ.Comp[1], &Unit_gfssZ.Comp[2]); 
	
	   #if defined( _ACS_DEBUG_) && defined(_FSS_PHASE_TEST_)
	     printf("WfssY: %d %0.3f %0.3f %0.3f\n", 
	        Valid_wfssY, Unit_wfssY.Comp[0], Unit_wfssY.Comp[1], Unit_wfssY.Comp[2]);  
	   #endif
	   #if defined( _ACS_DEBUG_) && defined(_GFSS_PHASE_TEST_)
	     printf("GfssY: %d %0.3f %0.3f %0.3f\n", 
	        Valid_gfssY, Unit_gfssY.Comp[0], Unit_gfssY.Comp[1], Unit_gfssY.Comp[2]);  
	     printf("GfssZ: %d %0.3f %0.3f %0.3f\n", 
	        Valid_gfssZ, Unit_gfssZ.Comp[0], Unit_gfssZ.Comp[1], Unit_gfssZ.Comp[2]);  
	   #endif
	

// Read in 6 CSS 
	SOL_LibReadCSS(&Cnt_CSS[0], &Cnt_CSS[1], &Cnt_CSS[2],  
        &Cnt_CSS[3], &Cnt_CSS[4], &Cnt_CSS[5]);
	   #if defined(_ACS_DEBUG_) && defined(_CSS_PHASE_TEST_)
      	//printf("Cnt_CSS: %d %d %d %d %d %d\n", 
			//Cnt_CSS[0], Cnt_CSS[1], Cnt_CSS[2], 
			//Cnt_CSS[3], Cnt_CSS[4], Cnt_CSS[5]);
	   #endif 
	
// Compute Unit_BF_CSS and Eclipse conditions
		CSS( Cnt_CSS, Valid_wfssY, Valid_gfssY, Valid_gfssZ,
         &CSS_Eclipse, &CSS_Pres, CssF_CSS, &Unit_BF_CSS);
	
	   #if defined(_ACS_DEBUG_) && defined(_CSS_PHASE_TEST_)
         //printf("CSS_Eclipse: %d\n", CSS_Eclipse);
		   //print_Vector3f("ACS: Unit_BF_CSS", &Unit_BF_CSS);
	   #endif 

// choose best FSS, computer Unit_Sun_BF 
	selectFSS (Valid_wfssY, Unit_wfssY, Valid_gfssY, Unit_gfssY, Valid_gfssZ, Unit_gfssZ, 
	           &Unit_BF_CSS, &Use_FSS_AKF, &Unit_Sun_BF);
	
	#if defined(_ACS_DEBUG_) 
	  print_Vector3f("ACS: Unit_Sun_BF", &Unit_Sun_BF);
	#endif
	//}
	
//   if (MagCycle % 2 ==  0) 
//	{
	
      //printf("MagCycle = %u\n", MagCycle);
		if( NM_LibReadMagnetometer(&BfNMM.Comp[0], &BfNMM.Comp[1], &BfNMM.Comp[2]) == 0)  
		{
			NMM_Ready = 1;
	  	 #ifdef _ACS_DEBUG_ 
	  	    //printf("ACS: Bfield_NMM : %.3f %.3f %.3f nT\n", BfNMM.Comp[0],  BfNMM.Comp[1],  BfNMM.Comp[2]);  
	  	 #endif 
		}
		else
		{
			NMM_Ready = 0;
	  	 #ifdef _ACS_DEBUG_ 
	  	   //printf("ACS: not ready Bfield_NMM : %0.3f, %0.3f, %0.3f\n", BfNMM.Comp[0],  BfNMM.Comp[1],  BfNMM.Comp[2]);  
	  	 #endif 
		}
//	}
	
	// compute Bfield BcsF NMM
	BfieldNMM (BfNMM.Comp[0], BfNMM.Comp[1], BfNMM.Comp[2], NMM_Ready, NMM_Valid, &Bfield_BF_NMM);

	CFE_TIME_SysTime_t Time_Tai;
	Time_Tai =CFE_TIME_GetTAI();
	TimeTAI = Time_Tai.Seconds;
	#ifdef _ACS_DEBUG_ 
   	printf("TimeTAI %u\n", TimeTAI);
	#endif
	
	if (sc_mode != MODE_SAFE)
	{

     #ifdef SIM_DAGRBOOM_DATA
		SBM_Ready = 1;
	   Use_SBM_AKF = 1; 
      //Vector3f_Copy(&Bfield_BF_SBM, &Bfield_BF_NMM);
      DAGR_LibReadBoom(&DagrX, &DagrY, &DagrZ);
		//printf("Dagr Sim Data: %f, %f, %f\n", DagrX, DagrY, DagrZ);
	   BfieldDagr ( DagrX, DagrY, DagrZ, SBM_Ready, SBM_Valid, &Bfield_BF_SBM);
	   //print_Vector3f("ACS Bfield BF SBM ", &Bfield_BF_SBM);  
     #else
		SBM_Ready = 1;
	   Use_SBM_AKF = 1; 
	   BfieldDagr ( DagrBoomPkt.boom_mag_x,   DagrBoomPkt.boom_mag_y,   
	    DagrBoomPkt.boom_mag_z,  SBM_Ready, SBM_Valid, &Bfield_BF_SBM);
      #ifdef _ACS_DEBUG_
		   printf("ACS Dagr: %0.3f, %0.3f, %0.3f\n", DagrBoomPkt.boom_mag_x,  DagrBoomPkt.boom_mag_y,  DagrBoomPkt.boom_mag_z);  
		#endif
     #endif
	   
	}
	else
	{
		SBM_Ready = 0;
	   Use_SBM_AKF = 0; 
	}
	
	if (sc_mode != MODE_SAFE)
	{
          // Get GPS Num of Sats,	
	      GoodGps = (GPS_LibGetNumSats() > 4) ? 1 : 0; // If the number of sats > 4, gps is good
         // Get GPS Pos & Vel
	      GPS_LibGetXyz(&PosGpsEF.Comp[0], &PosGpsEF.Comp[1],  &PosGpsEF.Comp[2], &VelGpsEF.Comp[0], &VelGpsEF.Comp[1],  &VelGpsEF.Comp[2]);
	      #if defined(_ACS_DEBUG_) && defined(_GPS_DEBUG_) 
	      	//print_Vector3f("ACS PosGpsEF", &PosGpsEF);
	      	//print_Vector3f("ACS VelGpsEF", &VelGpsEF);
	      #endif


         // Get Lat, Long, Alt and computer igrf
	      if (GPS_LibGetPosition(&latitude, &longitude, &altitude) == 0)
	      {
	         extrapsh(2016);
	         new_igrf(latitude, longitude, altitude/1000.0 + RAD_EARTH_KM, 13, &Bfield_GF);
	         #if defined(_ACS_DEBUG_) && defined(_GPS_DEBUG_)
	         //printf("lat, long, alt = %f %f %f\n", latitude, longitude, altitude);
            //print_Vector3f("Bfield_GF", &Bfield_GF);
	         #endif
	      }
         // Compute DCM using GHA and translate Pos/Vel from EF frame to GCiF
	      GHA(TimeTAI-LEAP_SECONDS, &Dcm_EFToGF);
			//print_Matrix3x3f("Dcm_EFToGF", &Dcm_EFToGF);
	      Matrix3x3f_MultVec(&PosGpsGF, &Dcm_EFToGF, &PosGpsEF);
	      Matrix3x3f_MultVec(&VelGpsGF, &Dcm_EFToGF, &VelGpsEF);
	      #if defined(_ACS_DEBUG_) && defined(_GPS_DEBUG_)
	      //print_Vector3f("ACS PosGpsGF", &PosGpsGF);
	      //print_Vector3f("ACS VelGpsGF", &VelGpsGF);
	      #endif
	      Vector3f_Normalize (&Unit_Pos_GF, &PosGpsGF);
	      Vector3f_Normalize (&Unit_Vel_GF, &VelGpsGF);
	}


	// Get RW Speed and Calculate RwaMom_BF
//	if (sc_mode != MODE_SAFE)
	//{
		#ifdef SIM_WHEELS  
			RW_LibGetSpeed(&Speed_RW.Comp[0], &Speed_RW.Comp[1], &Speed_RW.Comp[2]);
		#else
	   	RWS_GetTach(&Speed_RW.Comp[0], &Speed_RW.Comp[1], &Speed_RW.Comp[2]);
		#endif
	   #ifdef _ACS_DEBUG_
	      print_Vector3f("ACS RWSPD", &Speed_RW);
	   #endif
	   ReactionCalcMomentum(&Speed_RW, &RwaMom_BF);
	   //print_Vector3f("RwaMom_BF", &RwaMom_BF);
//	}
	
	//if ( (RW_Config != CFG_RW_IDLE) && ( MTC_Config != CFG_MTC_IDLE) ) 
   // Get Gyro Rate and calculate Rate_BF_Gyro
	if (sc_mode != MODE_SAFE)
	{
		GYS_GetTach(&Gyro_Rate.Comp[0], &Gyro_Rate.Comp[1], &Gyro_Rate.Comp[2]);
		#if defined(_ACS_DEBUG_) && defined(_GYRO_PHASE_TEST_)
			//print_Vector3f("ACS Gyro_Rate (rad)", &Gyro_Rate);
		#endif
	}

	gyro(&Rate_BF_Gyro, &Gyro_Rate);
	#if defined(_ACS_DEBUG_) && defined(_GYRO_PHASE_TEST_)
		print_Vector3f("ACS RBFGYRO", &Rate_BF_Gyro);
	#endif

   // Calculate SysmMom_BF_Calc 
	SysMom(Gyro_Valid, &Rate_BF_Gyro, &RwaMom_BF, &SysMom_BF_Calc);
	#ifdef _ACS_DEBUG_ 
        //print_Vector3f("SymMom_BF_Calc", &SysMom_BF_Calc);
   #endif

   // calculate solar ephemeris
	solar_ephemeris (TimeTAI, &Unit_Sun_GF);
	#ifdef _ACS_DEBUG_ 
		print_Vector3f("ACS: Unit_Sun_GF", &Unit_Sun_GF);
	   Vector3f_Normalize (&Unit_Pos_EF, &PosGpsEF);
	   Vector3f_Normalize (&Unit_Vel_EF, &VelGpsEF);
      betangle (&Unit_Pos_EF, &Unit_Vel_EF, &Unit_Sun_GF);
	#endif
   
   // if Science modes then call, Triad and Att Solution 
	//if ((sc_mode == MODE_INMS ) || (sc_mode == MODE_DAGR)) 
	if (AKF_Enable == 1)
	{
	   #ifdef _ACS_DEBUG_ 
		//		print_Vector3f("ACS Unit_Sun_BF", &Unit_Sun_BF);
		//		print_Vector3f("ACS Unit_Sun_GF", &Unit_Sun_GF);
		#endif
   
   	TriadSolution ( AKF_Enable, &Bfield_BF_SBM, &Bfield_GF, Use_FSS_AKF, 
     		Use_SBM_AKF, &Unit_Sun_BF, &Unit_Sun_GF,
   	// outputs
   	&AKF_FirstCycle, &Count_Triad, &GFToBF_Est_Triad, &GFToBF_AKF_Init, &Run_AKF);
   	#ifdef _ACS_DEBUG_ 
     	 //printf("ACS: Count Triad = %d\n", (int)Count_Triad);
     	 //printf("ACS: AKF_Enable = %d\n", (int) AKF_Enable);
     	 printf("ACS: Run_AKF = %d\n", (int) Run_AKF);
     	 print_Vector4f("ACS GF2BF_Est_Triad", &GFToBF_Est_Triad);
     	 print_Vector4f("ACS GF2BF_AKF_Init", &GFToBF_AKF_Init);
   	#endif

		AttSolution ( &AKF_FirstCycle, Run_AKF, &Rate_BF_Gyro, 
			Use_FSS_AKF, Use_SBM_AKF, &Bfield_BF_SBM, &Bfield_GF, 
			&Unit_Sun_BF,  &Unit_Sun_GF, &GFToBF_AKF_Init,
			// outputs
      	&GFToBF_Est, &Rate_BF_Est);
		#ifdef _ACS_DEBUG_ 
			print_Vector4f("ACS GFToBF_Est", &GFToBF_Est);
			print_Vector3f("ACS Rate_BF_Est", &Rate_BF_Est);
	   #endif
     	ACS_State.AKF_FirstCycle = 0;
	}

   // True Quaternion from 42 Simulator
	ATT_LibGetEstimate( (ATT_estimate_data_t *)&LibGF2BF_Est);
   #ifdef _ACS_DEBUG_
   print_Vector4f("ACS LibGetEst GFToBF_Est", &LibGF2BF_Est);
   #endif

	if (RW_Config == CFG_RW_SUNPT) 
	{ 
     // Call SunPoint Mode Controller
		SunPointMode (ACS_State.SunPt_FirstCycle, CSS_Eclipse, &Bfield_BF_NMM,
		Rate_Cmd_Sun, &RwaMom_BF, &Unit_Sun_BF, 
		Gain_Rate_Sun, Gain_Att_Sun, &Att_Err_Sat, 
		&Rate_BF_Sun, &Sun_Tor_BF_Ctrl, &Sun_Tor_RW_Cmd);

		Vector3f_Copy(&Tor_RW_Cmd, &Sun_Tor_RW_Cmd);
		Vector3f_Copy(&Tor_BF_Ctrl, &Sun_Tor_BF_Ctrl);

		ACS_State.SunPt_FirstCycle = 0;
	}
	else if (RW_Config == CFG_RW_LVLH) 
	{ 

     // Call Science Mode Controller
		scimode ( &GFToBF_Est,  Targ_Cmd_Sci,  &Unit_Pos_GF, &Unit_Vel_GF, 
		//scimode ( &LibGF2BF_Est,  (Vector4f *)LF_Targ_Cmd,  &Unit_Pos_GF, &Unit_Vel_GF, 
		Rate_Cmd_Sci, &Rate_BF_Est, &AttErr, &RateErr);

		calculate_torques ( &AttErr, &RateErr,	ACS_State.Science_FirstCycle, 
     	 UseStructFilter, Gain_Att_Sci, Gain_Int_Sci, Gain_Rate_Sci,
     	 &ACS_Settling, &AttErr_Lim, &IntErr, &IntErr_Lim, &RateErr_Lim,	
     	 &Sci_Tor_RW_Cmd, &Sci_Tor_BF_Ctrl);

    	ACS_State.Science_FirstCycle  = 0;
    	ACS_SciTlmPkt.sci_status = ACS_Settling; 

		#ifdef _ACS_DEBUG_
		//    print_Vector3f("AttErr_Lim", &AttErr_Lim);
		//    print_Vector3f("IntErr_Lim", &IntErr_Lim);
		//    print_Vector3f("RateErr_Lim", &RateErr_Lim);
		#endif

  	  Vector3f_Copy(&Tor_RW_Cmd, &Sci_Tor_RW_Cmd);
  	  Vector3f_Copy(&Tor_BF_Ctrl, &Sci_Tor_BF_Ctrl);
	}

	else if (RW_Config == CFG_RW_IDLE) 
	{ 
		Vector3f_InitZero(&Tor_RW_Cmd);
		Vector3f_InitZero(&Tor_BF_Ctrl);
		RWS_SetAllIdle();
	}

	CalculateWheelCommands(&Tor_RW_Cmd, &ScaledTor_RW_Cmd);
	#ifdef _ACS_DEBUG_
		print_Vector3f("ACS TORWCMD", &Tor_RW_Cmd);
		//print_Vector3f("ACS Scaled Tor RW CMD", &ScaledTor_RW_Cmd);
	#endif


	if (RW_Config != CFG_RW_IDLE)
	{
  		CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &RWTorqueCmdPkt);
  		RWTorqueCmdPkt.torque_cmd[0] = ScaledTor_RW_Cmd.Comp[0];
  		RWTorqueCmdPkt.torque_cmd[1] = ScaledTor_RW_Cmd.Comp[1];
  		RWTorqueCmdPkt.torque_cmd[2] = ScaledTor_RW_Cmd.Comp[2];
		CFE_SB_SendMsg((CFE_SB_Msg_t *) &RWTorqueCmdPkt);
      RW_LibCommandRW(ScaledTor_RW_Cmd.Comp[0], ScaledTor_RW_Cmd.Comp[1], 
         ScaledTor_RW_Cmd.Comp[2]);
	}

	MagneticControl(MTC_Config, latitude, &Bfield_BF_NMM, &SysMom_BF_Calc, &Moment_MTC_Cmd);
	#ifdef _ACS_DEBUG_
	   print_Vector3f("ACS Moment_MTC_Cmd", &Moment_MTC_Cmd);
	#endif

	// if (MagCycle % 2 == 1) 
	// {
//      printf("MagCycle = %u\n", MagCycle);
	   /*
		if( NM_LibReadMagnetometer(&BfNMM.Comp[0], &BfNMM.Comp[1], &BfNMM.Comp[2]) == 0)  
		{
			NMM_Ready = 1;
	  	 #ifdef _ACS_DEBUG_ 
	  	    //printf("ACS: Bfield_NMM : %.3f %.3f %.3f nT\n", BfNMM.Comp[0],  BfNMM.Comp[1],  BfNMM.Comp[2]);  
	  	 #endif 
		}
		else
		{
			NMM_Ready = 0;
	  	 #ifdef _ACS_DEBUG_ 
	  	   //printf("ACS: not ready Bfield_NMM : %0.3f, %0.3f, %0.3f\n", BfNMM.Comp[0],  BfNMM.Comp[1],  BfNMM.Comp[2]);  
	  	 #endif 
		}
   
	}   */
   //else
	//{
		SOL_LibCommandMT(Moment_MTC_Cmd.Comp[0], Moment_MTC_Cmd.Comp[1], Moment_MTC_Cmd.Comp[1]);
	//} //else
	//{
	//	SOL_LibCommandMT(0.0, 0.0, 0.0);
	//}
//      print_Vector3f("Moment_MTC_Cmd", &Moment_MTC_Cmd); 
      //printf("MagCycle = %u\t", MagCycle);
		//MagCycle = 1;
      //printf("Issued MTC Command, Now MagCycle = %u\n", MagCycle);
	//}
   // MagCycle++;

	#ifdef _NANOMIND_
	 //portTickType time_total = xTaskGetTickCount() - time_start;
	 //printf("TIME Pre_assignments = %d\r\n", (int)time_total);
	#endif

   #ifdef _MPU_TEST_
     mpu6000_read();
	#endif

	// ACS Science Controller diag pkt
	ACS_SciTlmPkt.unit_sun_ad[0] = Unit_Sun_BF.Comp[0];
	ACS_SciTlmPkt.unit_sun_ad[1] = Unit_Sun_BF.Comp[1];
	ACS_SciTlmPkt.unit_sun_ad[2] = Unit_Sun_BF.Comp[2];
	ACS_SciTlmPkt.unit_sun_gf[0] = Unit_Sun_GF.Comp[0];
	ACS_SciTlmPkt.unit_sun_gf[1] = Unit_Sun_GF.Comp[1];
	ACS_SciTlmPkt.unit_sun_gf[2] = Unit_Sun_GF.Comp[2];
	ACS_SciTlmPkt.time_tai.Seconds=Time_Tai.Seconds;
	ACS_SciTlmPkt.time_tai.Subseconds=Time_Tai.Subseconds;
	ACS_SciTlmPkt.pos_ef_gps[0]=PosGpsEF.Comp[0];
	ACS_SciTlmPkt.pos_ef_gps[1]=PosGpsEF.Comp[1];
	ACS_SciTlmPkt.pos_ef_gps[2]=PosGpsEF.Comp[2];
	ACS_SciTlmPkt.vel_ef_gps[0]=VelGpsEF.Comp[0];
	ACS_SciTlmPkt.vel_ef_gps[1]=VelGpsEF.Comp[1];
	ACS_SciTlmPkt.vel_ef_gps[2]=VelGpsEF.Comp[2];
	ACS_SciTlmPkt.lf_target[0]=Targ_Cmd_Sci->Comp[0];
	ACS_SciTlmPkt.lf_target[1]=Targ_Cmd_Sci->Comp[1];
	ACS_SciTlmPkt.lf_target[2]=Targ_Cmd_Sci->Comp[2];
	ACS_SciTlmPkt.lf_target[3]=Targ_Cmd_Sci->Comp[3];
	ACS_SciTlmPkt.tor_bf_ctrl[0] = Sci_Tor_BF_Ctrl.Comp[0];
	ACS_SciTlmPkt.tor_bf_ctrl[1] = Sci_Tor_BF_Ctrl.Comp[1];
	ACS_SciTlmPkt.tor_bf_ctrl[2] = Sci_Tor_BF_Ctrl.Comp[2];
	ACS_SciTlmPkt.tor_rwa_cmd[0] = Sci_Tor_RW_Cmd.Comp[0];
	ACS_SciTlmPkt.tor_rwa_cmd[1] = Sci_Tor_RW_Cmd.Comp[1];
	ACS_SciTlmPkt.tor_rwa_cmd[2] = Sci_Tor_RW_Cmd.Comp[2];
	ACS_SciTlmPkt.speed_rwa[0] = Speed_RW.Comp[0]; 
	ACS_SciTlmPkt.speed_rwa[1] = Speed_RW.Comp[1]; 
	ACS_SciTlmPkt.speed_rwa[2] = Speed_RW.Comp[2]; 
	
	 //ACS Sunpointing Controller diag pkt 
	ACS_SptTlmPkt.unit_sun_wfssY[0] = Unit_wfssY.Comp[0];
	ACS_SptTlmPkt.unit_sun_wfssY[1] = Unit_wfssY.Comp[1];
	ACS_SptTlmPkt.unit_sun_wfssY[2] = Unit_wfssY.Comp[2];
	ACS_SptTlmPkt.unit_sun_gfssY[0] = Unit_gfssY.Comp[0];
	ACS_SptTlmPkt.unit_sun_gfssY[1] = Unit_gfssY.Comp[1];
	ACS_SptTlmPkt.unit_sun_gfssY[2] = Unit_gfssY.Comp[2];
	ACS_SptTlmPkt.unit_sun_gfssZ[0] = Unit_gfssZ.Comp[0];
	ACS_SptTlmPkt.unit_sun_gfssZ[1] = Unit_gfssZ.Comp[1];
	ACS_SptTlmPkt.unit_sun_gfssZ[2] = Unit_gfssZ.Comp[2];
	ACS_SptTlmPkt.unit_sun_ad[0] = Unit_Sun_BF.Comp[0];
	ACS_SptTlmPkt.unit_sun_ad[1] = Unit_Sun_BF.Comp[1];
	ACS_SptTlmPkt.unit_sun_ad[2] = Unit_Sun_BF.Comp[2];
	ACS_SptTlmPkt.speed_rwa[0] = Speed_RW.Comp[0]; 
	ACS_SptTlmPkt.speed_rwa[1] = Speed_RW.Comp[1]; 
	ACS_SptTlmPkt.speed_rwa[2] = Speed_RW.Comp[2]; 


	// ACS Housekeeping Tlm 
	ACS_HkTlmPkt.spare = 0x000;
	ACS_HkTlmPkt.acs_status = (((CSS_Pres & 0x3F) <<  16) | ((ACS_Settling & 0x01) << 15) |  
	     ((Run_AKF & 0x01) << 14) | ((Use_SBM_AKF & 0x01) << 13) | ((Use_FSS_AKF & 0x01) << 12) | 
	     ((GoodGps & 0x01) << 11) | ((Gyro_Valid & 0x01) << 10 ) | ((SBM_Valid & 0x01) << 9) | 
	     ((NMM_Valid & 0x01) << 8) | ((Valid_wfssY & 0x01) << 7) | ((Valid_gfssY & 0x01) << 6) | 
	    ((Valid_gfssZ & 0x01) << 5) | ((CSS_Eclipse & 0x01) << 4) | ((RW_Config & 0x03) << 2) | (MTC_Config & 0x03));
	//printf("acs_status: %x\n", (unsigned int)ACS_HkTlmPkt.acs_status);
	ACS_HkTlmPkt.time_tai.Seconds=Time_Tai.Seconds;
	ACS_HkTlmPkt.time_tai.Subseconds=Time_Tai.Subseconds;
	ACS_HkTlmPkt.pos_ef_gps[0]=PosGpsEF.Comp[0];
	ACS_HkTlmPkt.pos_ef_gps[1]=PosGpsEF.Comp[1];
	ACS_HkTlmPkt.pos_ef_gps[2]=PosGpsEF.Comp[2];
	ACS_HkTlmPkt.vel_ef_gps[0]=VelGpsEF.Comp[0];
	ACS_HkTlmPkt.vel_ef_gps[1]=VelGpsEF.Comp[1];
	ACS_HkTlmPkt.vel_ef_gps[2]=VelGpsEF.Comp[2];
	ACS_HkTlmPkt.quat_est[0] = GFToBF_Est.Comp[0];
	ACS_HkTlmPkt.quat_est[1] = GFToBF_Est.Comp[1];
	ACS_HkTlmPkt.quat_est[2] = GFToBF_Est.Comp[2];
	ACS_HkTlmPkt.quat_est[3] = GFToBF_Est.Comp[3];
	ACS_HkTlmPkt.rate_est[0] = Rate_BF_Est.Comp[0];
	ACS_HkTlmPkt.rate_est[1] = Rate_BF_Est.Comp[1];
	ACS_HkTlmPkt.rate_est[2] = Rate_BF_Est.Comp[2];
	ACS_HkTlmPkt.mom_mtc_cmd[0] = Moment_MTC_Cmd.Comp[0]; 
	ACS_HkTlmPkt.mom_mtc_cmd[1] = Moment_MTC_Cmd.Comp[1]; 
	ACS_HkTlmPkt.mom_mtc_cmd[2] = Moment_MTC_Cmd.Comp[2]; 
	ACS_HkTlmPkt.unit_sun_ad[0] = Unit_Sun_BF.Comp[0]; 
	ACS_HkTlmPkt.unit_sun_ad[1] = Unit_Sun_BF.Comp[1]; 
	ACS_HkTlmPkt.unit_sun_ad[2] = Unit_Sun_BF.Comp[2]; 
	ACS_HkTlmPkt.speed_rwa[0] = Speed_RW.Comp[0]; 
	ACS_HkTlmPkt.speed_rwa[1] = Speed_RW.Comp[1]; 
	ACS_HkTlmPkt.speed_rwa[2] = Speed_RW.Comp[2]; 
	
	// Added because no other packets working
	ACS_HkTlmPkt.cur_rwa_cmd[0] = ScaledTor_RW_Cmd.Comp[0]; 
	ACS_HkTlmPkt.cur_rwa_cmd[1] = ScaledTor_RW_Cmd.Comp[1]; 
	ACS_HkTlmPkt.cur_rwa_cmd[2] = ScaledTor_RW_Cmd.Comp[2]; 
	
	ACS_HkTlmPkt.tor_rwa_cmd[0] = Tor_RW_Cmd.Comp[0];
	ACS_HkTlmPkt.tor_rwa_cmd[1] = Tor_RW_Cmd.Comp[1];
	ACS_HkTlmPkt.tor_rwa_cmd[2] = Tor_RW_Cmd.Comp[2];
	
	ACS_HkTlmPkt.rate_simu[0] = Gyro_Rate.Comp[0];
	ACS_HkTlmPkt.rate_simu[1] = Gyro_Rate.Comp[1];
	ACS_HkTlmPkt.rate_simu[2] = Gyro_Rate.Comp[2];

	// ACS Sensor Data Tlm
	ACS_SdpTlmPkt.speed_rwa[0] = Speed_RW.Comp[0]; 
	ACS_SdpTlmPkt.speed_rwa[1] = Speed_RW.Comp[1]; 
	ACS_SdpTlmPkt.speed_rwa[2] = Speed_RW.Comp[2]; 
	ACS_SdpTlmPkt.cur_rwa_cmd[0] = ScaledTor_RW_Cmd.Comp[0]; 
	ACS_SdpTlmPkt.cur_rwa_cmd[1] = ScaledTor_RW_Cmd.Comp[1]; 
	ACS_SdpTlmPkt.cur_rwa_cmd[2] = ScaledTor_RW_Cmd.Comp[2]; 
	ACS_SdpTlmPkt.tor_rwa_cmd[0] = Tor_RW_Cmd.Comp[0];
	ACS_SdpTlmPkt.tor_rwa_cmd[1] = Tor_RW_Cmd.Comp[1];
	ACS_SdpTlmPkt.tor_rwa_cmd[2] = Tor_RW_Cmd.Comp[2];
	ACS_SdpTlmPkt.rate_simu[0] = Gyro_Rate.Comp[0];
	ACS_SdpTlmPkt.rate_simu[1] = Gyro_Rate.Comp[1];
	ACS_SdpTlmPkt.rate_simu[2] = Gyro_Rate.Comp[2];
	ACS_SdpTlmPkt.bfield_nmm[0] = Bfield_BF_NMM.Comp[0];
	ACS_SdpTlmPkt.bfield_nmm[1] = Bfield_BF_NMM.Comp[1];
	ACS_SdpTlmPkt.bfield_nmm[2] = Bfield_BF_NMM.Comp[2];
	ACS_SdpTlmPkt.time_tai.Seconds=Time_Tai.Seconds;
	ACS_SdpTlmPkt.time_tai.Subseconds=Time_Tai.Subseconds;
	//printf("Sdp gps time %d, %d\n", ACS_SdpTlmPkt.time_tai.Seconds, ACS_SdpTlmPkt.time_tai.Subseconds);
	ACS_SdpTlmPkt.pos_ef_gps[0]=PosGpsEF.Comp[0];
	ACS_SdpTlmPkt.pos_ef_gps[1]=PosGpsEF.Comp[1];
	ACS_SdpTlmPkt.pos_ef_gps[2]=PosGpsEF.Comp[2];
	ACS_SdpTlmPkt.vel_ef_gps[0]=VelGpsEF.Comp[0];
	ACS_SdpTlmPkt.vel_ef_gps[1]=VelGpsEF.Comp[1];
	ACS_SdpTlmPkt.vel_ef_gps[2]=VelGpsEF.Comp[2];
	
	// ACS Attitude Determination Tlm
	ACS_AdTlmPkt.quat_est[0] = GFToBF_Est.Comp[0];
	ACS_AdTlmPkt.quat_est[1] = GFToBF_Est.Comp[1];
	ACS_AdTlmPkt.quat_est[2] = GFToBF_Est.Comp[2];
	ACS_AdTlmPkt.quat_est[3] = GFToBF_Est.Comp[3];
	ACS_AdTlmPkt.rate_est[0] = Rate_BF_Est.Comp[0];
	ACS_AdTlmPkt.rate_est[1] = Rate_BF_Est.Comp[1];
	ACS_AdTlmPkt.rate_est[2] = Rate_BF_Est.Comp[2];
	ACS_AdTlmPkt.unit_sun_ad[0] = Unit_Sun_BF.Comp[0];
	ACS_AdTlmPkt.unit_sun_ad[1] = Unit_Sun_BF.Comp[1];
	ACS_AdTlmPkt.unit_sun_ad[2] = Unit_Sun_BF.Comp[2];
	ACS_AdTlmPkt.unit_sun_gf[0] = Unit_Sun_GF.Comp[0];
	ACS_AdTlmPkt.unit_sun_gf[1] = Unit_Sun_GF.Comp[1];
	ACS_AdTlmPkt.unit_sun_gf[2] = Unit_Sun_GF.Comp[2];
	ACS_AdTlmPkt.bfield_dagr[0] = Bfield_BF_SBM.Comp[0];
	ACS_AdTlmPkt.bfield_dagr[1] = Bfield_BF_SBM.Comp[1];
	ACS_AdTlmPkt.bfield_dagr[2] = Bfield_BF_SBM.Comp[2];
	ACS_AdTlmPkt.bfield_gf[0] = Bfield_GF.Comp[0]; 
	ACS_AdTlmPkt.bfield_gf[1] = Bfield_GF.Comp[1]; 
	ACS_AdTlmPkt.bfield_gf[2] = Bfield_GF.Comp[2]; 
	ACS_AdTlmPkt.rate_simu[0] = Gyro_Rate.Comp[0];
	ACS_AdTlmPkt.rate_simu[1] = Gyro_Rate.Comp[1];
	ACS_AdTlmPkt.rate_simu[2] = Gyro_Rate.Comp[2];
	ACS_AdTlmPkt.time_tai.Seconds=Time_Tai.Seconds;
	ACS_AdTlmPkt.time_tai.Subseconds=Time_Tai.Subseconds;
	//printf("Ad gps time %d, %d\n", ACS_SdpTlmPkt.time_tai.Seconds, ACS_SdpTlmPkt.time_tai.Subseconds);
	ACS_AdTlmPkt.pos_ef_gps[0]=PosGpsEF.Comp[0];
	ACS_AdTlmPkt.pos_ef_gps[1]=PosGpsEF.Comp[1];
	ACS_AdTlmPkt.pos_ef_gps[2]=PosGpsEF.Comp[2];
	ACS_AdTlmPkt.vel_ef_gps[0]=VelGpsEF.Comp[0];
	ACS_AdTlmPkt.vel_ef_gps[1]=VelGpsEF.Comp[1];
	ACS_AdTlmPkt.vel_ef_gps[2]=VelGpsEF.Comp[2];
	
	#ifdef _NANOMIND_
	// time_total = xTaskGetTickCount() - time_start;
	// printf("TIME TOTAL = %d\r\n", (int)time_total);
	#endif
	
	return 0;

}
