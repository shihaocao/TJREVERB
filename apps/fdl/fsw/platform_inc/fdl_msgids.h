/************************************************************************
** File:
**   $Id: fdl_msgids.h  $
**
** Purpose: 
**  Define FDL App Message IDs
**
** Notes:
**
**
*************************************************************************/
#ifndef _fdl_app_msgids_h_
#define _fdl_app_msgids_h_

#include "mission_msgids.h"
#include "platform_base_msgids.h"

#define FDL_APP_CMD_MID         CFE_CMD_MID_BASE + FDL_APP_CMD_MID_OFFSET
#define FDL_APP_SEND_HK_MID     CFE_CMD_MID_BASE + FDL_APP_SEND_HK_MID_OFFSET
#define FDL_APP_HK_TLM_MID      CFE_TLM_MID_BASE + FDL_APP_HK_TLM_MID_OFFSET
#define FDL_APP_SEGMENT_MID     CFE_TLM_MID_BASE + FDL_APP_SEGMENT_MID_OFFSET


#endif /* _fdl_app_msgids_h_ */

/************************/
/*  End of File Comment */
/************************/
