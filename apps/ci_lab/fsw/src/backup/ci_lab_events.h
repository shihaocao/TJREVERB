/************************************************************************
** File:
**   $Id: ci_lab_events.h 1.2 2010/09/02 09:53:56EDT wmoleski Exp  $
**
** Purpose: 
**  Define CI Lab Events IDs
**
** Notes:
**
** $Log: ci_lab_events.h  $
** Revision 1.2 2010/09/02 09:53:56EDT wmoleski 
** Modifications to the CI_Lab code to capture, drop and/or manipulate CFDP PDUs that are uplinked
** from the ground. These changes were needed to test the CF fault detection requirements.
** Revision 1.1 2008/04/30 13:09:04EDT rjmcgraw 
** Initial revision
** Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/ci_lab/fsw/src/project.pj
**
*************************************************************************/
#ifndef _ci_lab_events_h_
#define _ci_lab_events_h_


#define CI_RESERVED_EID              0
#define CI_SOCKETCREATE_ERR_EID      1 
#define CI_SOCKETBIND_ERR_EID        2
#define CI_STARTUP_INF_EID           3 
#define CI_COMMAND_ERR_EID           4
#define CI_COMMANDNOP_INF_EID        5 
#define CI_COMMANDRST_INF_EID        6
#define CI_INGEST_INF_EID            7
#define CI_INGEST_ERR_EID            8
#define CI_INVALID_MSGID_ERR_EID     13
#define CI_NOCAPTURE_ERR_EID         15
#define CI_LEN_ERR_EID               16

#endif /* _ci_lab_events_h_ */

/************************/
/*  End of File Comment */
/************************/
