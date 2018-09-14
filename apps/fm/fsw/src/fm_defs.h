/*
** $Id: fm_defs.h 1.4 2010/03/04 15:44:14EST lwalling Exp  $
**
** Title: CFS File Manager (FM) Macro Definitions File
**
** Purpose: Value definitions
**
** Author: Scott Walling (Microtel)
**
** Notes:
**
** References:
**    Flight Software Branch C Coding Standard Version 1.0a
**
** $Log: fm_defs.h  $
** Revision 1.4 2010/03/04 15:44:14EST lwalling 
** Remove include of cfe.h - not needed
** Revision 1.3 2010/02/25 13:31:01EST lwalling 
** Remove local definition of uint64 data type
** Revision 1.2 2009/11/13 16:28:17EST lwalling 
** Modify macro names, move some macros to platform cfg file, delete TableID
** Revision 1.1 2009/11/09 16:47:46EST lwalling 
** Initial revision
** Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/fm/fsw/src/project.pj
*/

#ifndef _fm_defs_h_
#define _fm_defs_h_


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM argument to not calculate CRC during Get File Info command   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define FM_IGNORE_CRC               0


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM directory entry definitions                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define FM_THIS_DIRECTORY           "."
#define FM_PARENT_DIRECTORY         ".."


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM filename status definitions                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define FM_NAME_IS_INVALID          0
#define FM_NAME_IS_NOT_IN_USE       1
#define FM_NAME_IS_FILE_OPEN        2
#define FM_NAME_IS_FILE_CLOSED      3
#define FM_NAME_IS_DIRECTORY        4


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM free space table entry state definitions                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define FM_TABLE_ENTRY_UNUSED       0
#define FM_TABLE_ENTRY_ENABLED      1
#define FM_TABLE_ENTRY_DISABLED     2


#endif /* _fm_defs_h_ */

/************************/
/*  End of File Comment */
/************************/

