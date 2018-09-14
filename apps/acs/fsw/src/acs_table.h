/*
** File: acs_table.h
**
** Purpose:
**   Define the ACS constants table, routine messages, exported variables, and interface routines.
**
*/
#ifndef _acs_table_h_
#define _acs_table_h_

/*
**   Includes:
*/
#include "cfe.h"

/*
** Defines
*/

/*
**  Table services definitions for the constant table
*/
#define ACS_CONSTANT_TABLE_NAME            "CONST_TBL"
#define ACS_APP_CONSTANT_TABLE_NAME        "ACS.CONST_TBL"
#define ACS_CONSTANT_TABLE_TITLE           "ACS Constants Table"

/*
** This should go in acs_platform_cfg.h
*/
#define ACS_CONSTANT_TABLE_FILE_NAME       "/boot/acs_cons.tbl"

/*
** Structures
*/

/* 
** ACS Constant Table 
*/
typedef struct
{
    float    rate_cmd_sun; 
    float    rate_cmd_sci[3];
    float    targ_cmd_sci[4];
    float    gain_att_sci[3];
    float    gain_int_sci[3];
    float    gain_rate_sci[3];
    float    gain_rate_sun[3];
    float    gain_att_sun[3];
} ACS_ConstantTable_t;

/*
** Exported Variables
*/
extern ACS_ConstantTable_t*    ACS_ConstantTablePtr;
extern CFE_TBL_Handle_t        ACS_ConstantTableHandle;

/* 
** May be relocated to rest of ACS globals 
*/
extern ACS_ConstantTable_t     ACS_Constants;

/*
** Exported Functions
*/
int32   ACS_ConstantTableInit ( void );
int32   ACS_UpdateConstantTableData  ( void );

#endif /* _acs_table_ */

/************************/
/*  End of File Comment */
/************************/
