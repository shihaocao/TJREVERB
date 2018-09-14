/*******************************************************************************
** File:
**   dagr_msg.h
**
** Purpose:
**  Define DAGR App Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _dagr_msg_h_
#define _dagr_msg_h_

/*
** DAGR App command codes
*/
#define DAGR_APP_NOOP_CC                 0
#define DAGR_APP_RESET_COUNTERS_CC       1

/*
** DAGR app power modes
*/
#define DAGR_PWR_CMD_1  1 // All mags enabled.
#define DAGR_PWR_CMD_2  2 // Boom and Todd enabled, Bills disabled.
#define DAGR_PWR_CMD_3  3 // Boom enabled, Todd and Bills disabled.
#define DAGR_PWR_CMD_4  4 // All mags disabled. Default.

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
	uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
} DAGR_NoArgsCmd_t;

/*************************************************************************/

/* Type definition (No args message, MSGID is all that is needed) */
typedef struct
{
	uint8    TLM_Header[CFE_SB_TLM_HDR_SIZE];
} OS_PACK dagr_no_args_msg_t;

/* Type definition (1 arg message to change dagr mode) */
typedef struct
{
	uint8    TLM_Header[CFE_SB_TLM_HDR_SIZE];
	uint8    new_mode;
} OS_PACK dagr_change_mode_t;

/* Type definition (DAGR App housekeeping) */
typedef struct
{
	uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
	uint8              dagr_command_count;
	uint8              dagr_command_error_count;
}   OS_PACK dagr_hk_tlm_t  ;

#define DAGR_APP_HK_TLM_LNGTH   sizeof ( dagr_hk_tlm_t )

/* DAGR telemetry for Science! */
typedef struct dagr_sci_tlm_s
{
	uint8             TlmHeader[CFE_SB_TLM_HDR_SIZE];
	uint8             power_mode;

	// From the instrument
	// Boom Mag data
	uint8 boom_mag_x_pol; // single bit
	uint8 boom_mag_x_msb;
	uint8 boom_mag_x_nmsb;
	uint8 boom_mag_x_lsb;
	uint8 boom_mag_y_pol; // single bit
	uint8 boom_mag_y_msb;
	uint8 boom_mag_y_nmsb;
	uint8 boom_mag_y_lsb;
	uint8 boom_mag_z_pol; // single bit
	uint8 boom_mag_z_msb;
	uint8 boom_mag_z_nmsb;
	uint8 boom_mag_z_lsb;
	// Todd Mag data
	uint8 todd_mag_x_pol; // single bit
	uint8 todd_mag_x_msb;
	uint8 todd_mag_x_nmsb;
	uint8 todd_mag_x_lsb;
	uint8 todd_mag_y_pol; // single bit
	uint8 todd_mag_y_msb;
	uint8 todd_mag_y_nmsb;
	uint8 todd_mag_y_lsb;
	uint8 todd_mag_z_pol; // single bit
	uint8 todd_mag_z_msb;
	uint8 todd_mag_z_nmsb;
	uint8 todd_mag_z_lsb;
	// Billingsley Mag data
	uint8 bill_mag_x_msb; // two bits
	uint8 bill_mag_x_nmsb;
	uint8 bill_mag_x_lsb;
	uint8 bill_mag_y_msb; // two bits
	uint8 bill_mag_y_nmsb;
	uint8 bill_mag_y_lsb;
	uint8 bill_mag_z_msb; // two bits
	uint8 bill_mag_z_nmsb;
	uint8 bill_mag_z_lsb;
	// Boom Mag temp
	uint8 boom_mag_temp_msb; // two bits
	uint8 boom_mag_temp_nmsb;
	uint8 boom_mag_temp_lsb;
} OS_PACK dagr_sci_tlm_t;

#define DAGR_APP_SCI_TLM_LNGTH   sizeof ( dagr_sci_tlm_t )

/* DAGR telemetry, boom only */
typedef struct dagr_sci_boom_tlm_s
{
	uint8             TlmHeader[CFE_SB_TLM_HDR_SIZE];
	uint8             power_mode;

	// From the instrument
	// Boom Mag data
	uint8 boom_mag_x_pol; // single bit
	uint8 boom_mag_x_msb;
	uint8 boom_mag_x_nmsb;
	uint8 boom_mag_x_lsb;
	uint8 boom_mag_y_pol; // single bit
	uint8 boom_mag_y_msb;
	uint8 boom_mag_y_nmsb;
	uint8 boom_mag_y_lsb;
	uint8 boom_mag_z_pol; // single bit
	uint8 boom_mag_z_msb;
	uint8 boom_mag_z_nmsb;
	uint8 boom_mag_z_lsb;
} OS_PACK dagr_sci_boom_tlm_t;

#define DAGR_APP_SCI_BOOM_TLM_LNGTH   sizeof ( dagr_sci_boom_tlm_t )

/* DAGR telemetry for ACS */
typedef struct dagr_acs_tlm_s
{
	uint8             TlmHeader[CFE_SB_TLM_HDR_SIZE];
	// Boom Mag data
	// Pulled from science tlm, but converted to real values.
	float boom_mag_x;
	float boom_mag_y;
	float boom_mag_z;
} OS_PACK dagr_acs_tlm_t;

#define DAGR_APP_ACS_TLM_LNGTH   sizeof ( dagr_acs_tlm_t )

#endif /* _dagr_msg_h_ */

/************************/
/*  End of File Comment */
/************************/
