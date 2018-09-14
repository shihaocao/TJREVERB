/************************************************************************
** File:
**   $Id: sch_cmds.h 1.1 2008/10/16 15:08:35EDT dkobe Exp  $
**
** Purpose: 
**  The CFS Scheduler (SCH) Application header file
**
** Notes:
**
** $Log: sch_cmds.h  $
** Revision 1.1 2008/10/16 15:08:35EDT dkobe 
** Initial revision
** Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/sch/fsw/src/project.pj
**
*************************************************************************/
#ifndef _sch_cmds_
#define _sch_cmds_

/*************************************************************************
**
** Include section
**
**************************************************************************/

#include "cfe.h"

/*************************************************************************
**
** Macro definitions
**
**************************************************************************/

/*************************************************************************
**
** Type definitions
**
**************************************************************************/

/*************************************************************************
**
** Exported data
**
**************************************************************************/

/*************************************************************************
**
** Exported functions
**
**************************************************************************/
/************************************************************************/
/** \brief Process a command pipe message
**  
**  \par Description
**       Processes a single software bus command pipe message. Checks
**       the message and command IDs and calls the appropriate routine
**       to handle the command.
**       
**  \par Assumptions, External Events, and Notes:
**       None
**       
**  \param [in]  MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                            references the software bus message 
**
**  \returns
**  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
**  \retstmt Return codes from #CFE_EVS_Register         \endcode
**  \retstmt Return codes from #CFE_SB_CreatePipe        \endcode
**  \retstmt Return codes from #CFE_SB_Subscribe         \endcode
**  \endreturns
**
**  \sa #CFE_SB_RcvMsg
**
*************************************************************************/
int32 SCH_AppPipe(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Manages Scheduler's Schedule and Message Definition Tables
**  
**  \par Description
**       This function manages the contents of the Schedule and Message
**       Definition Tables.
**
**  \par Assumptions, External Events, and Notes:
**       None
**       
**  \returns
**  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
**  \retstmt Return codes from #CFE_EVS_Register         \endcode
**  \retstmt Return codes from #CFE_SB_CreatePipe        \endcode
**  \retstmt Return codes from #CFE_SB_Subscribe         \endcode
**  \endreturns
**
*************************************************************************/
int32 SCH_AcquirePointers(void);

#endif /* _sch_cmds_ */

/************************/
/*  End of File Comment */
/************************/
