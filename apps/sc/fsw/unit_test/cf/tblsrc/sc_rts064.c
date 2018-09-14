 /*************************************************************************
 ** File:
 **   $Id: sc_rts064.c 1.1 2009/02/19 10:08:07EST nyanchik Exp  $
 **
 ** Purpose: 
 **   This file contains a sample of creating an RTS table
 **
 ** References:
 **   Flight Software Branch C Coding Standard Version 1.2
 **   CFS Development Standards Document
 ** Notes:
 **
 **   $Log: sc_rts064.c  $ 
 **   Revision 1.1 2009/02/19 10:08:07EST nyanchik  
 **   Initial revision 
 **   Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/sc/fsw/unit_test/cf/tblsrc/project.pj 
 **   Revision 1.1 2009/01/26 14:42:17EST nyanchik  
 **   Initial revision 
 **   Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/sc/fsw/unit_test/cf/tblsrc/project.pj 
 **   Revision 1.2 2009/01/05 08:26:56EST nyanchik  
 **   Check in after code review changes 
 *************************************************************************/

#include "sc_app.h"
#include "cfe_tbl_filedef.h"


/************************************************************************
The structure of a cFE raw command using CCSDS. 
Note the command shown has no parameters. If more parameters are needed, the are added after the checksum.

0x1803,0xC000,0x0001,0x0000
   ^       ^      ^     ^ ^ 
   |       |      |     | the last byte is the checksum for the command
   |       |      |     The first byte is the command code
   |       |      2 bytes for the length of command after these two bytes
   |       2 bytes for sequence counter always 0xC000
   The application to recieve and processs the command in app_msgids.h
   
note that in the case of SC, there is information before the cFE command structure,
like the time tag and the command number (for ATS's)
************************************************************************/

uint16     SC_Rts064[SC_RTS_BUFF_SIZE] = {
/* This is a valid table that does not go to the end of the buffer  
 I need a high numbner RTS to est the debug messages for 
 last RTS to send out start/stop event messages   
 */
/* 8F below is the checksum for the command */
0x0000, 0x0001, 0x18A9, 0xC000, 0x0001, 0x008F,
0x0000, 0x0000, 0x18A9, 0xC000, 0x0001, 0x008F,
0x0000, 0x0000, 0x18A9, 0xC000, 0x0001, 0x008F,
0x0000, 0x0000, 0x18A9, 0xC000, 0x0001, 0x008F,
0x0000, 0x0000, 0x18A9, 0xC000, 0x0001, 0x008F,
0x0000, 0x0000, 0x18A9, 0xC000, 0x0001, 0x008F,
0x0000, 0x0000, 0x18A9, 0xC000, 0x0001, 0x008F,
0x0000, 0x0000, 0x18A9, 0xC000, 0x0001, 0x008F,
0x0000, 0x0000, 0x18A9, 0xC000, 0x0001, 0x008F,
0x0000, 0x0000, 0x18A9, 0xC000, 0x0001, 0x008F
};

CFE_TBL_FILEDEF(SC_Rts064, SC_APP.RTS_TBL064, SC RTS table 064,RTS_TBL064.tbl)


/************************/
/*  End of File Comment */
/************************/

