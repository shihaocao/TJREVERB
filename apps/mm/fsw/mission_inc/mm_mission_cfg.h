/*************************************************************************
** File:
**   $Id: mm_mission_cfg.h 1.1 2008/09/05 12:11:52GMT-05:00 dahardison Exp  $
**
** Purpose: 
**   Specification for the CFS Memory Manager constants that can
**   be configured from one mission to another
**
** References:
**   Flight Software Branch C Coding Standard Version 1.2
**   CFS Development Standards Document
**   CFS MM Heritage Analysis Document
**   CFS MM CDR Package
**
** Notes:
**
**   $Log: mm_mission_cfg.h  $
**   Revision 1.1 2008/09/05 12:11:52GMT-05:00 dahardison 
**   Initial revision
**   Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/mm/fsw/mission_inc/project.pj
** 
*************************************************************************/
#ifndef _mm_mission_cfg_
#define _mm_mission_cfg_

/** \mmcfg CRC type for interrupts disabled loads
**  
**  \par Description:
**       CFE CRC type to use when processing the "memory load with 
**       interrupts disabled" (#MM_LOAD_MEM_WID_CC) command.
**
**  \par Limits:
**       This must be one of the CRC types supported by the 
**       #CFE_ES_CalculateCRC function.
*/
#define MM_LOAD_WID_CRC_TYPE     CFE_ES_DEFAULT_CRC

/** \mmcfg CRC type for load files
**  
**  \par Description:
**       CFE CRC type to use when processing memory loads
**       from a file.
**
**  \par Limits:
**       This must be one of the CRC types supported by the 
**       #CFE_ES_CalculateCRC function.
*/
#define MM_LOAD_FILE_CRC_TYPE    CFE_ES_DEFAULT_CRC

/** \mmcfg CRC type for dump files
**  
**  \par Description:
**       CFE CRC type to use when processing memory dumps
**       to a file.
**
**  \par Limits:
**       This must be one of the CRC types supported by the 
**       #CFE_ES_CalculateCRC function.
*/
#define MM_DUMP_FILE_CRC_TYPE    CFE_ES_DEFAULT_CRC

#endif /*_mm_mission_cfg_*/

/************************/
/*  End of File Comment */
/************************/
