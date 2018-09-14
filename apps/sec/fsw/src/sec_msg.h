/*******************************************************************************
** File:
**   sec_msg.h 
**
** Purpose: 
**  Define SEC App  Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _sec_msg_h_
#define _sec_msg_h_

/*
** RW App command codes
*/
#define SEC_NOOP_CC			0
#define SEC_RESET_COUNTERS_CC		1
#define SEC_CONFIG_CC			2

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
	uint8	CmdHeader[CFE_SB_CMD_HDR_SIZE];
} SEC_NoArgsCmd_t;


/*************************************************************************/
/*
** Type definition (SEC App housekeeping)
*/
typedef struct 
{
	uint8		    TlmHeader[CFE_SB_TLM_HDR_SIZE];
	uint8		    sec_command_count;
	uint8		    sec_command_error_count;
	uint8               sec_gps_valid;
	uint8               spare;
        uint32              gps_status;
        CFE_TIME_SysTime_t  last_gps_time;
        uint32              num_sats;

}   OS_PACK sec_hk_tlm_t  ;

#define SEC_HK_TLM_LNGTH		sizeof ( sec_hk_tlm_t )

/*************************************************************************/
/*
** Type definition (SEC Config command)
*/

typedef struct
{
	uint8	CmdHeader[CFE_SB_CMD_HDR_SIZE];
        uint32  SecConfig;
} sec_config_cmd_t;

#define SEC_CONFIG_CMD_LNGTH	sizeof(sec_config_cmd_t)

#endif /* _sec_msg_h_ */

/************************/
/*  End of File Comment */
/************************/
