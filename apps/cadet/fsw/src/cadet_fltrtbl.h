/*
** File: cadet_fltrtbl.h
**
** Purpose:
**   Define CADET filter table Routine messages, exported variables, and interface routines.
**
*/
#ifndef _cadet_fltrtbl_h_
#define _cadet_fltrtbl_h_



/*
**   Includes:
*/
#include "cfe.h"



/*
** Defines
*/
#define   CADET_FILTER_TABLE_SIZE      ( DELLINGR_MAX_TLM_MID - DELLINGR_MIN_TLM_MID + 1 )

#define   CADET_SEQUENCE_BASED_FILTER_TYPE  ( 0 )
#define   CADET_TIME_BASED_FILTER_TYPE      ( 1 )

/*
**  Table entry codes representing Software Bus telemetry pipe assignments and priority
*/
#define   FILTER_HI_FIFO             ( 0 )    /* Must be 0 */
#define   FILTER_LO_FIFO             ( 1 )    /* Must be 1 */
#define   FILTER_BOTH_FIFO           ( 2 )
#define   NUMBER_FILTER_FIFOS        ( 2 )

#define   CADET_INVALID_PIPE        ( 0xFF ) /* equal to private define CFE_SB_INVALID_PIPE */

/*
** Software Bus pipe subscription parameters
*/
#define  CADET_MAX_MSG_SUBSCRIBE_BUFFERS      ( 100 )
#define  CADET_DEFAULT_MSG_SUBSCRIBE_BUFFERS  (  50 )

/*
**  Table services definitions for the filter table
*/
#define CADET_FILTER_TABLE_NAME            "FILTER_TBL"
#define CADET_APP_FILTER_TABLE_NAME        "CADET."CADET_FILTER_TABLE_NAME
#define CADET_FILTER_TABLE_TITLE           "CADET Filter Table"
#define CADET_DEF_FILTER_TABLE_FILE_NAME   "/boot/cdt_ft01.tbl"

#define CADET_FILTER_STATS_TABLE_NAME      "FLTR_STAT_TBL"



/*
** Structures
*/
/* Filter Table - 2 dimensional array Number APPIDs by Number FIFOs */
typedef struct
{
    uint8  FilterMethod; /*  Time = 1 | Sequence = 0 */
    uint16 N;            /*  'N' (Remainder Limit) */
    uint16 X;            /*  'X' (Modulo Factor) */
    uint16 O;            /*  'O' (Offset Factor) */

} CADET_FilterTableEntry_t;

typedef struct
{
    CADET_FilterTableEntry_t Fifo [NUMBER_FILTER_FIFOS];
 
} CADET_FilterTableLine_t;

typedef struct
{
    CADET_FilterTableLine_t AppID [CADET_FILTER_TABLE_SIZE];

} CADET_FilterTable_t;

/* Statistics Table - 1 dimensional array Number APPIDs */
typedef struct
{
    boolean               SubscribeAppList  [ CADET_FILTER_TABLE_SIZE ];
    uint16                PacketRecvCnt     [ CADET_FILTER_TABLE_SIZE ];

} CADET_FilterStatTable_t;



/*
** Exported Variables
*/
extern CADET_FilterTable_t*     CADET_FilterTablePtr;
extern CFE_TBL_Handle_t         CADET_FilterTableHandle;
extern CADET_FilterStatTable_t* CADET_FilterStatTablePtr;
extern CFE_TBL_Handle_t         CADET_FilterStatTableHandle;
extern char                     CADET_InitialTblFilename [OS_MAX_PATH_LEN]; /* Filename for first table to load */



/*
** Exported Functions
*/
int32   CADET_AppTableInit ( void );
int32   CADET_GetFilterTableData  ( void );
boolean CADET_SubscribeToPacket ( uint16  AppID, uint16 HifilterN, uint16 HifilterX, uint16 LofilterN, uint16 LofilterX );



#endif /* _CADET_filter_ */



/************************/
/*  End of File Comment */
/************************/

