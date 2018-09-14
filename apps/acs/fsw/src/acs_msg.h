/*******************************************************************************
** File:
**   acs_msg.h 
**
** Purpose: 
**  Define ACS App Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _acs_msg_h_
#define _acs_msg_h_

#include "cfe_time.h"
#include "vector3.h"

#define CFG_RW_IDLE  0
#define CFG_RW_SUNPT 1
#define CFG_RW_LVLH  2

#define CFG_MTC_IDLE    0
#define CFG_MTC_BDOT    1
#define CFG_MTC_MOMMNG  2 // All latitudes
#define CFG_MTC_LATMNG  3 // Low latitudes

/*
** ACS App command codes
*/
#define ACS_NOOP_CC                 0
#define ACS_RESET_COUNTERS_CC       1
#define ACS_SPT_CC                  2
#define ACS_SCI_CC                  3
#define ACS_SDP_CC                  4
#define ACS_AD_CC                   5
#define ACS_MODE_CC                 6
#define ACS_SCIGAIN_CC              7
#define ACS_RATETARG_CC             8
#define ACS_PKTFLAG_CC              9
#define ACS_MPU_CC                 10
#define ACS_SUNGAIN_CC             11



/*************************************************************************/
/*
** Typedef for ACS diagnostic telemetry flags
** Used for both the global data and command packet structure
*/
typedef struct 
{
    uint8              packet1;
    uint8              packet2;
    uint8              packet3;
    uint8              packet4;
}   OS_PACK acs_diag_flags_t;

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

} ACS_NoArgsCmd_t;

// TODO: RW speed and Gyro rates here are redundant: they are reported by the RW app
/*************************************************************************/
/*
** Type definition (ACS App housekeeping)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8              acs_command_count;
    uint8              acs_command_error_count;
    uint16             spare;
    uint32             acs_status;
    CFE_TIME_SysTime_t time_tai;
    float              pos_ef_gps[3];
    float              vel_ef_gps[3];
    float              quat_est[4];
    float              rate_est[3];
    float              mom_mtc_cmd[3];
    float              unit_sun_ad[3];
    float              speed_rwa[3];
    float              cur_rwa_cmd[3];
    float              tor_rwa_cmd[3];
    float              rate_simu[3];
}   OS_PACK acs_hk_tlm_t  ;


/*************************************************************************/
/*
** Type definition (ACS App SunPointing Controller Diagnostic packet)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint16             spt_status;
    float              spt_atterr[3];
    float              spt_raterr[3];
    float              rate_cmd_sun;
    float              targ_cmd_sci[4];
    float              unit_sun_wfssY[3];
    float              unit_sun_gfssY[3];
    float              unit_sun_gfssZ[3];
    float              unit_sun_ad[3];
    float              tor_bf_ctrl[3];
    float              tor_rwa_cmd[3];
    float              speed_rwa[3];
}   OS_PACK acs_spt_tlm_t  ;
	
/*************************************************************************/
/*************************************************************************/
/*
** Type definition (ACS App Science INMS/DAGR Diagnostic packet)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint16             sci_status; //both
    float              gain_rate_science[3];
    float              quat_est[4];  // both instruments
    float              rate_est[3];  // both instruments 
    float              unit_sun_ad[3]; // both
    float              unit_sun_gf[3]; // both
    CFE_TIME_SysTime_t time_tai;
    float              pos_ef_gps[3]; // both
    float              vel_ef_gps[3]; // both
    float              lf_target[4]; // inms only
    float              sci_atterr[3];
    float              sci_raterr[3];
    float              tor_bf_ctrl[3];
    float              tor_rwa_cmd[3];
    float              speed_rwa[3];
}   OS_PACK acs_sci_tlm_t  ;

/*************************************************************************/
/*
** Type definition (ACS App Sensor Data Processing Diagnostic packet)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    float              speed_rwa[3];
    float              cur_rwa_cmd[3];
    float              tor_rwa_cmd[3];
    // uint16             temp_simu[3]; // TODO: Remove from ITOS packets
    float              rate_simu[3];
    float              bfield_nmm[3];
    uint8              per_mtc_cmd[3];
    CFE_TIME_SysTime_t time_tai;
    float              pos_ef_gps[3];
    float              vel_ef_gps[3];
}   OS_PACK acs_sdp_tlm_t  ;

/*************************************************************************/
/*
** Type definition (ACS App Attitude Determination Diagnostic packet)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    float              quat_est[4];
    float              rate_est[3];
    float              bias_est[3];
    float              unit_sun_ad[3];
    float              unit_sun_gf[3];
    float              res_fss[3];
    float              bfield_dagr[3];
    float              bfield_gf[3];
    float              res_dagr[3];
    double             cov_diag[21];
    double             state_update[6];
    float              angle_gha;
    float              rate_simu[3];
    CFE_TIME_SysTime_t time_tai;
    float              pos_ef_gps[3];
    float              vel_ef_gps[3];
}   OS_PACK acs_ad_tlm_t  ;

/*************************************************************************/
/*
** Type definition (ACS App IMU (MPU600) Telemetry packet)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
	 uint16             mpu1_temp;
	 uint16             mpu1_rate[3];
	 uint16             mpu1_acc[3];
	 uint16             mpu2_temp;
	 uint16             mpu2_rate[3];
	 uint16             mpu2_acc[3];
	 uint16             mpu3_temp;
	 uint16             mpu3_rate[3];
	 uint16             mpu3_acc[3];
	 uint16             mpu4_temp;
	 uint16             mpu4_rate[3];
	 uint16             mpu4_acc[3];
 }  OS_PACK acs_mpu_tlm_t;

#define ACS_HK_TLM_LNGTH    sizeof ( acs_hk_tlm_t )
#define ACS_SPT_TLM_LNGTH   sizeof ( acs_spt_tlm_t )
#define ACS_SCI_TLM_LNGTH   sizeof ( acs_sci_tlm_t )
#define ACS_SDP_TLM_LNGTH   sizeof ( acs_sdp_tlm_t )
#define ACS_AD_TLM_LNGTH    sizeof ( acs_ad_tlm_t )
#define ACS_MPU_TLM_LNGTH   sizeof ( acs_mpu_tlm_t )

/*************************************************************************/
/*
** Type definition (ACS mode command)
*/
typedef struct 
{
    uint8     CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint8     akf_enable;
    uint8     rw_config;
    uint8     mtc_config;
} OS_PACK acs_ModeCmd_t;

/*************************************************************************/
/*
** Type definition (ACS Sun Gains command)
*/
typedef struct 
{
    uint8     CmdHeader[CFE_SB_CMD_HDR_SIZE];
    float     gain_att_sci[3];
    float     gain_int_sci[3];
    float     gain_rate_sci[3];
} OS_PACK acs_SciGainCmd_t;

/*************************************************************************/
/*
** Type definition (ACS Sun Gains command)
*/
typedef struct 
{
    uint8     CmdHeader[CFE_SB_CMD_HDR_SIZE];
    float     gain_rate_sun[3];
    float     gain_att_sun[3];
} OS_PACK acs_SunGainCmd_t;

/*************************************************************************/
/*
** Type definition (ACS targ & rates command)
*/
typedef struct 
{
    uint8     CmdHeader[CFE_SB_CMD_HDR_SIZE];
    float     targ_cmd_sci[4];
    float     rate_cmd_sun;
    float     rate_cmd_sci[3];
} OS_PACK acs_RateTargCmd_t;

/*************************************************************************/
/*
** Type definition (ACS Diagnostic Telemetry flags)
*/
typedef struct 
{
    uint8              CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint8              packet1;
    uint8              packet2;
    uint8              packet3;
    uint8              packet4;
}   OS_PACK acs_flag_cmd_t;

#endif /* _acs_msg_h_ */

/************************/
/*  End of File Comment */
/************************/
