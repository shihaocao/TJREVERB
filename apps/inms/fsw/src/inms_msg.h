/*******************************************************************************
** File:
**   inms_msg.h 
**
** Purpose: 
**  Define INMS App Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _inms_msg_h_
#define _inms_msg_h_

/*
** INMS App command codes
*/
#define INMS_APP_NOOP_CC                 0
#define INMS_APP_RESET_COUNTERS_CC       1
#define INMS_APP_INSTRUMENT_COMMAND_CC   2

/*************************************************************************/

/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
} INMS_NoArgsCmd_t;

/* INMS request command */
typedef struct {
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint8 opcode;
    uint8 param1;
    uint8 param2;
    uint8 param3;
    uint8 param4;
    uint8 param5;
    uint8 param6;
    uint8 chksum;
} INMS_CMDInst_t;

/* INMS message header */
typedef struct inms_msg_header_s
{
    uint8 sync; // 0xA5
    uint8 t0;   // utc sec // TODO: Change to uint32?
    uint8 t1;   // utc sec
    uint8 t2;   // utc sec
    uint8 t3;   // utc sec
    uint8 s0;   // utc sub sec // TODO: Change to uint16?
    uint8 s1;   // utc sub sec
} OS_PACK inms_msg_header_t;

/* INMS Housekeeping log response */
typedef struct inms_housekeeping_s
{
    uint8  opcode;          // Last Command
    uint8  counter;         // Command counter
    uint8  tof1;
    uint8  tof2;
    uint8  tof3;
    uint8  max_tof;
    uint8  start_pulse;     // four bits
    uint8  delay_start;     // four bits
    uint8  control_reg;     // control register
    uint8  packet_cnt;
    uint8  tof_dac0_msb;
    uint8  tof_dac0_1_lsb;  // bits 0-3 are DAC0LSB, 4-7 DAC1LSB
    uint8  tof_dac1_msb;
    uint8  dac0_msb;
    uint8  dac0_1_lsb;      // bits 0-3 are DAC0LSB, 4-7 DAC1LSB
    uint8  dac1_msb;
    uint8  dac2_msb;
    uint8  dac2_3_lsb;      // bits 0-3 are DAC0LSB, 4-7 DAC1LSB
    uint8  dac3_msb;
    uint8  temp1;           // HK Telem # 19
    uint8  temp2;
    uint8  hk_reg;
    uint8  reserved;
    uint8  ion_float_msb;   // TODO: Make these two byte values uint16?
    uint8  ion_float_lsb;
    uint8  neutral_float_msb;
    uint8  neutral_float_lsb;
    uint8  hv_msb;
    uint8  hv_lsb;
    uint8  neutral_esa_msb;
    uint8  neutral_esa_lsb;
    uint8  ion_esa_msb;
    uint8  ion_esa_lsb;
    uint8  anode_i_msb;
    uint8  anode_i_lsb;
    uint8  filament_i_msb;
    uint8  filament_i_lsb;  // HK Telem #36
    uint8  D1_TCNT1;
    uint8  D1_TCNT2;
    uint8  D1_TCNT3;
    uint8  D2_TCNT1;
    uint8  D2_TCNT2;
    uint8  D2_TCNT3;
    uint16 D1_Data[400];
    uint16 D2_Data[400];
} OS_PACK inms_housekeeping_t;

typedef struct inms_telemetry_s
{
    inms_msg_header_t HDR;
    inms_housekeeping_t HK; // And data
    uint8 CHKSUM;
} OS_PACK inms_telemetry_t;

/*
** Type definition (INMS Instrument housekeeping Packet)
**  This is the cFE SB packet that is sent for data storage
**  and/or downlink. 
*/
typedef struct 
{
    uint8            TlmHeader[CFE_SB_TLM_HDR_SIZE];
    inms_telemetry_t inms_hk_data;
}   OS_PACK inms_instrument_hk_tlm_t  ;

#define INMS_APP_INST_TLM_LNGTH   sizeof ( inms_instrument_hk_tlm_t )

/*
** Type definition (INMS App housekeeping)
**  This is the periodic housekeeping packet for the app
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8              inms_command_count;
    uint8              inms_command_error_count;
    uint16             spare;
    uint32             inms_inst_tlm_packets;
}   OS_PACK inms_hk_tlm_t  ;

#define INMS_APP_HK_TLM_LNGTH   sizeof ( inms_hk_tlm_t )

#endif /* _inms_msg_h_ */

/************************/
/*  End of File Comment */
/************************/
