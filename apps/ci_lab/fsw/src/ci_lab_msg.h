/*******************************************************************************
** File:
**   $Id: ci_lab_msg.h 1.3 2010/09/20 13:27:18EDT wmoleski Exp  $
**
** Purpose: 
**  Define CI Lab Messages and info
**
** Notes:
**
** $Log: ci_lab_msg.h  $
** Revision 1.3 2010/09/20 13:27:18EDT wmoleski 
** Modified the CI_LAB, SCH_LAB and TO_LAB applications to use unique message IDs and Pipe Names. The "_LAB"
** was added to all definitions so that a mission can use these "Lab" apps as well as their own mission apps together.
** Revision 1.2 2010/09/02 09:53:56EDT wmoleski 
** Modifications to the CI_Lab code to capture, drop and/or manipulate CFDP PDUs that are uplinked
** from the ground. These changes were needed to test the CF fault detection requirements.
** Revision 1.1 2008/04/30 13:09:17EDT rjmcgraw 
** Initial revision
** Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/ci_lab/fsw/src/project.pj
**
*******************************************************************************/
#ifndef _ci_lab_msg_h_
#define _ci_lab_msg_h_

/*
** CI_Lab command codes
*/
#define CI_NOOP_CC                 0
#define CI_RESET_COUNTERS_CC       1

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
   uint8          CmdHeader[CFE_SB_CMD_HDR_SIZE];

} CI_NoArgsCmd_t;


/*************************************************************************/
/*
** Type definition (CI_Lab housekeeping)...
*/
typedef struct {

    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8              ci_command_error_count;
    uint8              ci_command_count;
    uint8              ci_xsums_enabled;
    uint8              SocketConnected;
    uint8              spare[3];
    uint32             IngestPackets;
    uint32             IngestErrors;

}   OS_PACK ci_hk_tlm_t  ;

#define CI_LAB_HK_TLM_LNGTH           sizeof ( ci_hk_tlm_t )

#endif /* _ci_lab_msg_h_ */

/************************/
/*  End of File Comment */
/************************/
