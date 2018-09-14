/************************************************************************
** File:
**   $Id: mm_app.h 1.4 2008/09/05 13:24:51GMT-05:00 dahardison Exp  $
**
** Purpose: 
**   Unit specification for the Core Flight System (CFS) 
**   Memory Manger (MM) Application.  
**
** References:
**   Flight Software Branch C Coding Standard Version 1.2
**   CFS MM Heritage Analysis Document
**   CFS MM CDR Package
**
** Notes:
**
**   $Log: mm_app.h  $
**   Revision 1.4 2008/09/05 13:24:51GMT-05:00 dahardison 
**   Updated declaration of local HK variables
**   Revision 1.3 2008/05/19 15:22:56EDT dahardison 
**   Version after completion of unit testing
** 
*************************************************************************/
#ifndef _mm_app_
#define _mm_app_

/************************************************************************
** Includes
*************************************************************************/
#include "mm_msg.h"
#include "cfe.h"

/************************************************************************
** Macro Definitions
*************************************************************************/
/**
** \name MM Command Pipe Parameters */ 
/** \{ */
#define MM_CMD_PIPE_DEPTH   12
#define MM_HK_LIMIT          2
#define MM_CMD_LIMIT         4    
/** \} */

/************************************************************************
** Type Definitions
*************************************************************************/
/** 
**  \brief MM global data structure
*/
typedef struct          
{ 
   MM_HkPacket_t        HkPacket;        /**< \brief Housekeeping telemetry packet  */
   
   CFE_SB_MsgPtr_t      MsgPtr;          /**< \brief Pointer to command message     */
   CFE_SB_PipeId_t      CmdPipe;         /**< \brief Command pipe ID                */
   
   uint32               RunStatus;       /**< \brief Application run status         */
   
   char                 PipeName[16];    /**< \brief Command pipe name              */
   uint16               PipeDepth;       /**< \brief Command pipe message depth     */
   
   uint8                LimitHK;         /**< \brief Houskeeping messages limit     */
   uint8                LimitCmd;        /**< \brief Command messages limit         */

   uint8                CmdCounter;      /**< \brief MM Application Command Counter       */
   uint8                ErrCounter;      /**< \brief MM Application Command Error Counter */
   uint8                LastAction;      /**< \brief Last command action executed         */
   uint8                MemType;         /**< \brief Memory type for last command         */
   uint32               Address;         /**< \brief Fully resolved address used for last 
                                                     command                              */
   uint32               FillPattern;     /**< \brief Fill pattern used if memory fill 
                                                     command was issued                   */    
   uint32               BytesProcessed;  /**< \brief Bytes processed for last command     */
   
   char                 FileName[OS_MAX_PATH_LEN];   /**< \brief Name of the data file 
                                                                 used for last command, 
                                                                 where applicable         */
   
} MM_AppData_t;           

/************************************************************************
** Exported Functions
*************************************************************************/
/************************************************************************/
/** \brief CFS Memory Manager (MM) application entry point
**  
**  \par Description
**       Memory Manager application entry point and main process loop.
**
**  \par Assumptions, External Events, and Notes:
**       None
**       
*************************************************************************/
void MM_AppMain(void);

#endif /* _mm_app_ */

/************************/
/*  End of File Comment */
/************************/
