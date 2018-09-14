/************************************************************************
** File:
**   $Id: hs_mat_tbl.c 1.2 2011/08/15 15:42:45EDT aschoeni Exp  $
**
** Purpose:
**  The CFS Health and Safety (HS) Message Actions Table Definition
**
** Notes:
**
** $Log: hs_mat_tbl.c  $
** Revision 1.2 2011/08/15 15:42:45EDT aschoeni 
** Updated so application name is configurable
** Revision 1.1 2009/05/04 11:50:10EDT aschoeni 
** Initial revision
** Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/hs/fsw/project.pj
**
*************************************************************************/


/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "hs_tbl.h"
#include "hs_tbldefs.h"
#include "cfe_tbl_filedef.h"


static CFE_TBL_FileDef_t CFE_TBL_FileDef =
{
    "HS_Default_MsgActs_Tbl", HS_APP_NAME ".MsgActs_Tbl", "HS MsgActs Table",
    "hs_mat.tbl", (sizeof(HS_MATEntry_t) * HS_MAX_MSG_ACT_TYPES)
};



HS_MATEntry_t      HS_Default_MsgActs_Tbl[HS_MAX_MSG_ACT_TYPES] =
{
/*          EnableState               Cooldown       Message */

/*   0 */ { HS_MAT_STATE_DISABLED,    10,            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
/*   1 */ { HS_MAT_STATE_DISABLED,    10,            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
/*   2 */ { HS_MAT_STATE_DISABLED,    10,            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
/*   3 */ { HS_MAT_STATE_DISABLED,    10,            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
/*   4 */ { HS_MAT_STATE_DISABLED,    10,            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
/*   5 */ { HS_MAT_STATE_DISABLED,    10,            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
/*   6 */ { HS_MAT_STATE_DISABLED,    10,            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
/*   7 */ { HS_MAT_STATE_DISABLED,    10,            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },

};

/************************/
/*  End of File Comment */
/************************/
