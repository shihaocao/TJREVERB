/*************************************************************************
** File:
**   $Id: md_cmds.h 1.2 2009/04/18 14:08:15GMT-05:00 dkobe Exp  $
**
** Purpose: 
**   Specification for the CFS Memory Dwell ground commands.
**
**
** Notes:
**
**   $Log: md_cmds.h  $
**   Revision 1.2 2009/04/18 14:08:15GMT-05:00 dkobe 
**   Corrected comment for function parameter
**   Revision 1.1 2008/07/02 13:47:14EDT nsschweiss 
**   Initial revision
**   Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/md/fsw/src/project.pj
** 
*************************************************************************/
/*
** Ensure that header is included only once...
*/
#ifndef _md_cmds_h_
#define _md_cmds_h_

/*************************************************************************
** Includes
*************************************************************************/

/* md_msg needs to be included for MD_SymAddr_t definition */ 
#include "md_msg.h"

/*****************************************************************************/
/**
** \brief Process Memory Dwell Start Command
**
** \par Description
**          Extract command arguments, take appropriate actions,
**          issue event, and increment the command counter or 
**          error counter as appropriate.
** 
** \par Assumptions, External Events, and Notes:
**          Correct message length has been verified.
**
** \param[in] MessagePtr a pointer to the message received from the command pipe
**                                      
** \retval None
******************************************************************************/
void MD_ProcessStartCmd(CFE_SB_MsgPtr_t MessagePtr);


/*****************************************************************************/
/**
** \brief Stop dwell table.
**
** \par Description
**          Stop specified table.
** 
** \par Assumptions, External Events, and Notes:
**          Correct message length has been verified.
**
** \param[in] TableId identifier.  (1.. MD_NUM_DWELL_TABLES)
**                                      
** \retval None
******************************************************************************/
void MD_StopTable(int16 TableId);


/*****************************************************************************/
/**
** \brief Process Memory Dwell Stop Command
**
** \par Description
**          Extract command arguments, take appropriate actions,
**          issue event, and increment the command counter or 
**          error counter as appropriate.
** 
** \par Assumptions, External Events, and Notes:
**          Correct message length has been verified.
**
** \param[in] MessagePtr a pointer to the message received from the command pipe
**                                      
** \retval None
******************************************************************************/
void MD_ProcessStopCmd(CFE_SB_MsgPtr_t MessagePtr);


/*****************************************************************************/
/**
** \brief Process Memory Dwell Jam Command
**
** \par Description
**          Extract command arguments, take appropriate actions,
**          issue event, and increment the command counter or 
**          error counter as appropriate.
** 
** \par Assumptions, External Events, and Notes:
**          Correct message length has been verified.
**
** \param[in] MessagePtr a pointer to the message received from the command pipe
**                                      
** \retval None
******************************************************************************/
void MD_ProcessJamCmd(CFE_SB_MsgPtr_t MessagePtr);

/*****************************************************************************/
/**
** \brief Process Set Signature Command
**
** \par Description
**          Extract command arguments, take appropriate actions,
**          issue event, and increment the command counter or 
**          error counter as appropriate.
** 
** \par Assumptions, External Events, and Notes:
**          Correct message length has been verified.
**
** \param[in] MessagePtr a pointer to the message received from the command pipe
**                                      
** \retval None
******************************************************************************/
void MD_ProcessSignatureCmd(CFE_SB_MsgPtr_t MessagePtr);


#endif /* _md_cmds_ */
/************************/
/*  End of File Comment */
/************************/
