 /*************************************************************************
 ** File:
 **   $Id: sc_msgids.h 1.1 2009/01/26 14:46:33EST nyanchik Exp  $
 **
 ** Purpose: 
 **   This file contains the message ID's used by Stored Command 
 **
 ** References:
 **   Flight Software Branch C Coding Standard Version 1.2
 **   CFS Development Standards Document
 ** Notes:
 **
 **   $Log: sc_msgids.h  $ 
 **   Revision 1.1 2009/01/26 14:46:33EST nyanchik  
 **   Initial revision 
 **   Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/sc/fsw/platform_inc/project.pj 
 *************************************************************************/
#ifndef _sc_msgids_
#define _sc_msgids_

/*************************************************************************
 ** Macro Definitions
 *************************************************************************/

/**
 ** \name SC Command Message Numbers */ 
/** \{ */
#define SC_CMD_MID                          (0x18A9)    /**< \brief Msg ID for cmds to SC   */
#define SC_SEND_HK_MID                      (0x18AA)    /**< \brief Msg ID to request SC HK */
#define SC_1HZ_WAKEUP_MID                   (0x18AB)    /**< \brief Msg ID to recieve the 1Hz */ 
/** \} */

/**
 ** \name SC Telemetery Message Number */ 
/** \{ */
#define SC_HK_TLM_MID                       (0x08AA)    /**< \brief Msg ID to send telemtry down on */
/** \} */

#endif /*_sc_msgids_*/

/************************/
/*  End of File Comment */
/************************/
