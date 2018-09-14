/*******************************************************************************
** File:
**   shk_msg.h
**
** Purpose:
**  Define SHK App Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _shk_msg_h_
#define _shk_msg_h_

/*
** SHK App command codes
*/
#define SHK_APP_NOOP_CC                 0
#define SHK_APP_RESET_COUNTERS_CC       1
#define SHK_APP_CHANGE_SC_MODE_CC       2
#define SHK_APP_TLC_CONTROL_CC          3
#define SHK_APP_CAM_CAPTURE_CC          4
#define SHK_APP_DEVICE_ON_CC            5
#define SHK_APP_DEVICE_OFF_CC           6
#define SHK_APP_PRIMARY_DEPLOY_CC       7
#define SHK_APP_BUS_RESET_CC            8
#define SHK_APP_SECONDARY_DEPLOY_CC     9


// TODO: Makes more sense to be in EPS3_XUA.h
//#define EPS_BCR_COUNT                   7 // BCRs 8 and 9 are only for ground use. Total BCR count: 9
#define EPS_SWITCH_COUNT               10
#define BATT_PLATE_COUNT                4 // 2 packs, each with 4 10WHr plates
/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
} SHK_NoArgsCmd_t;

// TODO: All three of these could be combined. Also, check sizes (all could be uint8 for the param) and spare.
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
   uint32   DeviceToSwitch;
} SHK_DeviceCmd_t;

typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
   uint8    spare;
   uint8    reset_mask;
} SHK_ResetCmd_t;

typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
   uint32   SpacecraftMode;
} SHK_ChangeSpacecraftModeCmd_t;

/*
** Image size will be 1, 2, or 3
** 1 == 160x120
** 2 == 320x240
** 3 == 640x480
*/
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
   uint32   ImageSize;
   uint8    ImagePath[OS_MAX_PATH_LEN];
} SHK_CameraCaptureCmd_t;


/*
** RTS Request command structure
*/
typedef struct 
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
   uint16   RTSId;

} SHK_RTSRequest_t;


/************************************************************************/

typedef struct
{
   uint8  Voltage_MSB;
   uint8  Voltage_LSB;
   uint8  Thermister1_MSB;
   uint8  Thermister1_LSB;
   uint8  Thermister2_MSB;
   uint8  Thermister2_LSB;
} OS_PACK SHK_TlcData_t;

#define TLC_DATA_CNT   30

typedef struct
{
   uint8         TlmHeader[CFE_SB_TLM_HDR_SIZE];
   SHK_TlcData_t data[TLC_DATA_CNT];
   uint8         CheckSum; // TODO: How is this calulated?
} OS_PACK SHK_TlcTelemetryPkt_t;

// TODO: mpio.h uses int for boolean values.
typedef struct
{
    uint8 TlmHeader[CFE_SB_TLM_HDR_SIZE];
    // TODO: Spacing?
    uint8 gpio_ext_addr;
    uint8 pre_deploy_state_ant;
    uint8 pre_deploy_state_mag;
    // TODO: 12V bus current? GPIO5?
    uint8 post_deploy_state_ant;
    uint8 post_deploy_state_mag;
} OS_PACK shk_deploy_tlm_t;

#define SHK_APP_DEPLOY_TLM_LNGTH   sizeof ( shk_deploy_tlm_t )

/*************************************************************************/
/*
** Type definition (SHK App housekeeping)
*/
typedef struct
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8              shk_command_count;
    uint8              shk_command_error_count;

    // Spacecraft Mode
    uint8              spacecraft_mode;
    uint8              spare;

    // EPS and Battery
    uint16             eps_status;
    uint16             bat_status_p0;
    uint16             bat_status_p1;

    // BCR 1
    uint16             eps_sa1_v_y;
    uint16             eps_sa1_c_py;
    uint16             eps_sa1_c_ny;

    // BCR 2
    uint16             eps_sa2_v_y;
    uint16             eps_sa2_c_py;

    // BCR 3
    uint16             eps_sa3_v_z;
    uint16             eps_sa3_c_pz;
    uint16             eps_sa3_c_nz;

    // BCR 4
    uint16             eps_sa4_v_y;
    uint16             eps_sa4_c_py;

    // BCR 5
    uint16             eps_sa5_v_y;
    uint16             eps_sa5_c_ny;

    // BCR 6
    uint16             eps_sa6_v_x;
    uint16             eps_sa6_c_px;

    // BCR 7
    uint16             eps_sa7_v_x;
    uint16             eps_sa7_c_nx;

    uint16             eps_12v_current;
    uint16             eps_12v_voltage;
    uint16             eps_Batt_Bus_current;
    uint16             eps_Batt_Bus_voltage;
    uint16             eps_5v_current;
    uint16             eps_5v_voltage;
    uint16             eps_3v_current;
    uint16             eps_3v_voltage;

    uint16             eps_switch_voltage_array[EPS_SWITCH_COUNT];
    uint16             eps_switch_current_array[EPS_SWITCH_COUNT];

    // Pack 0
    uint16             bat_voltage_p0;
    uint16             bat_current_p0;
    uint16             bat_5V_curr_p0;
    uint16             bat_5V_volt_p0;
    uint16             bat_3V_curr_p0;
    uint16             bat_3V_volt_p0;
    uint8              bat_c_direc_p0;
    uint8              bat_all_heaters_p0;
    uint16             bat_temp_array_p0[BATT_PLATE_COUNT + 1]; // Plus one for motherboard temp

    // Pack 1
    uint16             bat_voltage_p1;
    uint16             bat_current_p1;
    uint16             bat_5V_curr_p1;
    uint16             bat_5V_volt_p1;
    uint16             bat_3V_curr_p1;
    uint16             bat_3V_volt_p1;
    uint8              bat_c_direc_p1;
    uint8              bat_all_heaters_p1;
    uint16             bat_temp_array_p1[BATT_PLATE_COUNT + 1]; // Plus one for motherboard temp

    // SSC (including thermistors)
    // Thermistors, ADCX_Y, X is 0-3 for the four ADC chips, Y is 0-7 for the 8 lines each chip provides
    uint16             THM_BP1;          // ADC1_0 - Baseplate 1 (between RW hsg and 3-axis mag)
    uint16             THM_BP2;          // ADC1_1 - Baseplate 2 (by the SSC)
    uint16             THM_SPMZ;         // ADC1_2 - Solar Panel -Z (6U cell covered panel)
    uint16             THM_SPPX;         // ADC1_3 - Solar Panel +X
    uint16             THM_SPMX;         // ADC1_4 - Solar Panel -X
    uint16             THM_SPPY;         // ADC1_5 - Solar Panel +Y
    uint16             THM_SPMY;         // ADC1_6 - Solar Panel -Y
    uint16             THM_RW_Hsg_Top;   // ADC1_7 - RW housing - Top
    uint16             THM_RW_Hsg_Side;  // ADC2_0 - RW housing - Wall where L3 radio mounts
    uint16             THM_L3;           // ADC2_1 - L3 Cadet radio
    uint16             THM_Bill_Board;   // ADC2_2 - Billingsley - board
    uint16             THM_Bill_Brkt;    // ADC2_3 - Billingsley - AL bracket
    uint16             THM_INMS_Back;    // ADC2_4 - INMS structure - backplane
    uint16             THM_INMS_Side;    // ADC2_5 - INMS structure - side wall (facing 3 gyro)
    // uint16             THM_INMS_IF;      // ADC2_6 - INMS Interface (NOT USED)
    uint16             THM_GYRO;         // ADC2_7 - MEMs Gyro
    uint16             THM_Todd_Mag;     // ADC3_0 - Todd Magnetometer
    uint16             THM_Ecard_Hsg_PZ; // ADC3_1 - Card stack plate - facing INMS
    uint16             THM_Ecard_Hsg_MZ; // ADC3_2 - Card stack plate - facing RW
    uint16             THM_Mag_Card;     // ADC3_3 - Mag Card
    uint16             THM_GPS_Card;     // ADC3_4 - GPS Card
    uint16             THM_SSC_Card;     // ADC3_5 - SSC Card
    uint16             THM_SPPZ;         // ADC3_6 - Solar Panel +Z
    uint16             THM_Batt_Top;     // ADC3_7 - Battery Top Cover

    // Other ADC values
    uint16             TL_VOUT;          // ADC4_0 Thermal Louver Voltage Mon
    uint16             TL_TEMP_1;        // ADC4_1 Thermal Louvre Temp1
    uint16             TL_TEMP_2;        // ADC4_2 Thermal Louvre Temp2
    // uint16             ADC_UNASSIGNED_3; // ADC4_3 Not assigned.
    // uint16             INMS_P5V_I;       // ADC4_4 +5V INMS Imon
    // uint16             INMS_P5V_V;       // ADC4_5 +5V INMS Vmon
    uint16             INMS_M5V_I;       // ADC4_6 -5V INMS Vmon
    // uint16             INMS_M5V_V;       // ADC4_7 -5V INMS Imon

    // NM Tlm
    uint16             NM_TEMP_1;
    uint16             NM_TEMP_2;
    // TODO: What about the magnetometer?
    /*
    Reset Status        1
    WatchDog Status     1
    */

}   OS_PACK shk_hk_tlm_t  ;

#define SHK_APP_HK_TLM_LNGTH   sizeof ( shk_hk_tlm_t )

#endif /* _shk_msg_h_ */

/************************/
/*  End of File Comment */
/************************/
