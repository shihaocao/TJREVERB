/************************************************************************
** File:
**   $Id: fdl_events.h $
**
** Purpose: 
**  Define File Downlink App Event IDs
**
** Notes:
**
**
*************************************************************************/
#ifndef _fdl_events_h_
#define _fdl_events_h_


#define FDL_STARTUP_INF_EID           1 
#define FDL_COMMAND_ERR_EID           2
#define FDL_COMMANDNOP_INF_EID        3 
#define FDL_COMMANDRST_DBG_EID        4
#define FDL_INVALID_MSGID_ERR_EID     5 
#define FDL_LEN_ERR_EID               6 
#define FDL_STARTXFER_INF_EID         7
#define FDL_CANCELXFER_INF_EID        8
#define FDL_PAUSEXFER_INF_EID         9
#define FDL_PAUSEXFER_ERR_EID        10
#define FDL_RESUMEXFER_INF_EID       11
#define FDL_RESUMEXFER_ERR_EID       12
#define FDL_XFERCOMPLETE_INF_EID     13
#define FDL_STARTXFER_ERR_EID        14
#define FDL_EXIT_ERR_EID             15
#define FDL_INIT_ERR_EID             16
#define FDL_START_FILE_XFER_ERR_EID  17
#define FDL_XFERCOMPLETE_ERR_EID     18


#endif /* _fdl_events_h_ */

/************************/
/*  End of File Comment */
/************************/
