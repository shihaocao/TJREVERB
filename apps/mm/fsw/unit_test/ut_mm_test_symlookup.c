/************************************************************************
** File:
**   $Id: ut_mm_test_symlookup.c 1.1 2011/11/30 16:07:20GMT-05:00 jmdagost Exp  $
**
** Purpose: 
**   This is a test driver subroutine to test CFS Memory Manager (MM) symbol
**   lookups
**
**   $Log: ut_mm_test_symlookup.c  $
**   Revision 1.1 2011/11/30 16:07:20GMT-05:00 jmdagost 
**   Initial revision
**   Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/mm/fsw/unit_test/project.pj
** 
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"            /* Application headers */
#include "mm_filedefs.h"
#include "mm_load.h"
#include "mm_dump.h"
#include "mm_msgids.h"

#include "cfe_es_cds.h"        /* cFE headers         */

#include "utf_custom.h"        /* UTF headers         */
#include "utf_types.h"
#include "utf_cfe_sb.h"
#include "utf_osapi.h"
#include "utf_cfe.h"
#include "utf_osloader.h"
#include "utf_osfileapi.h"

#include <sys/fcntl.h>         /* System headers      */
#include <unistd.h>
#include <stdlib.h>

/************************************************************************
** Local function prototypes
*************************************************************************/
void    Test_SymLookup(void);

extern void    PrintLocalHKVars(void);

/* These are private functions in the files that contain them
** but we need access to them in this test driver
*/
void    MM_LookupSymbolCmd(CFE_SB_MsgPtr_t msg);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Test symbol lookup command                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Test_SymLookup(void)
{
   MM_LookupSymCmd_t        LookupSymCmdMsg;

   /* Setup the test message header */ 
   CFE_SB_InitMsg(&LookupSymCmdMsg, MM_CMD_MID, sizeof(MM_LookupSymCmd_t), TRUE);

   UTF_put_text("***********************\n");
   UTF_put_text("* Symbol Lookup Tests *\n");
   UTF_put_text("***********************\n");
   UTF_put_text("\n");

   /*
   ** Test bad symbol name 
   */
   UTF_put_text("Test Lookup With Bad Symbol Name \n");
   UTF_put_text("---------------------------------\n");
   strcpy(LookupSymCmdMsg.SymName, "BadSymName");

   MM_LookupSymbolCmd((CFE_SB_MsgPtr_t)&LookupSymCmdMsg);
   PrintLocalHKVars();   
   UTF_put_text("\n");      

   /*
   ** Test empty symbol name 
   */
   UTF_put_text("Test Lookup With empty Symbol Name \n");
   UTF_put_text("-----------------------------------\n");
   LookupSymCmdMsg.SymName[0] = '\0';

   MM_LookupSymbolCmd((CFE_SB_MsgPtr_t)&LookupSymCmdMsg);
   PrintLocalHKVars();   
   UTF_put_text("\n");      
   
   /*
   ** Test good symbol name 
   */
   UTF_put_text("Test Lookup With Good Symbol Name \n");
   UTF_put_text("----------------------------------\n");
   strcpy(LookupSymCmdMsg.SymName, "GoodSymName");

   MM_LookupSymbolCmd((CFE_SB_MsgPtr_t)&LookupSymCmdMsg);
   PrintLocalHKVars();   
   UTF_put_text("\n");      

} /* end Test_SymLookup */
