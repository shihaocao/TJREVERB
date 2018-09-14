/*******************************************************************************
** File:
**   rw_msg.h
**
** Purpose:
**  Define RW App Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _rw_msg_h_
#define _rw_msg_h_

/*
** RW App command codes
*/
#define RW_NOOP_CC            0
#define RW_RESET_COUNTERS_CC  1
#define RW_MODE_CC            2
#define RW_PARAM_CC           3
#define RW_UPDATE_TACH_CC     4

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
	uint8	CmdHeader[CFE_SB_CMD_HDR_SIZE];
} RW_NoArgsCmd_t;

typedef struct
{
	uint8	CmdHeader[CFE_SB_CMD_HDR_SIZE];
	uint8	wheelAddress;
	uint8	type;
	float	value;
} RW_ModeCmd_t;

typedef struct
{
	uint8	CmdHeader[CFE_SB_CMD_HDR_SIZE];
	uint8	wheelAddress;
	uint8	number;
	float	value;
} RW_ParamCmd_t;

/*************************************************************************/
/*
** Type definition (RW App housekeeping)
*/
typedef struct
{
	uint8 TlmHeader[CFE_SB_TLM_HDR_SIZE];
	uint8 rw_command_count;
	uint8 rw_command_error_count;
	uint8 rw_wheel_on_status;
	uint8 spare;
	float rw0_current;
	float rw0_speed;
	float rw0_temp;
	float rw0_seu_count;
	float rw1_current;
	float rw1_speed;
	float rw1_temp;
	float rw1_seu_count;
	float rw2_current;
	float rw2_speed;
	float rw2_temp;
	float rw2_seu_count;
}   OS_PACK rw_hk_tlm_t;

#define RW_HK_TLM_LNGTH		sizeof ( rw_hk_tlm_t )

typedef struct
{
	uint8 TlmHeader[CFE_SB_TLM_HDR_SIZE];
	// RW App command and error counts in rw_hk_tlm_t
	uint8 valid;
	uint8 spare;

	uint16 temp[3]; // 0 - x, 1 - y, 2 - z, gyro frame of reference
	float  rate[3]; // 0 - x, 1 - y, 2 - z, gyro frame of reference
}   OS_PACK gy_hk_tlm_t;

#define GY_HK_TLM_LNGTH		sizeof ( gy_hk_tlm_t )

/*************************************************************************/
/*
** Type definition (RW torque command)
*/
typedef struct
{
	uint8			TlmHeader[CFE_SB_TLM_HDR_SIZE];
	float			torque_cmd[3];
} OS_PACK rw_torque_cmd_t;

#define RW_TORQUE_CMD_LNGTH	sizeof(rw_torque_cmd_t)

/*************************************************************************/
/*
** Type definition (RW current command)
*/
typedef struct
{
	uint8			TlmHeader[CFE_SB_TLM_HDR_SIZE];
	float			current_cmd[3];
} OS_PACK rw_current_cmd_t;

#define RW_CURRENT_CMD_LNGTH	sizeof(rw_current_cmd_t)

#endif /* _rw_msg_h_ */

/************************/
/*  End of File Comment */
/************************/
