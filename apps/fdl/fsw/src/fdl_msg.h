/*******************************************************************************
** File:
**   $Id: fdl_msg.h $
**
** Purpose: 
**  Define FDL App Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _fdl_msg_h_
#define _fdl_msg_h_

#include "common_types.h"

/*
** FDL App command codes
*/
#define FDL_APP_NOOP_CC                 0
#define FDL_APP_RESET_COUNTERS_CC       1
#define FDL_APP_FILE_START_CC           2
#define FDL_APP_FILE_CANCEL_CC          3
#define FDL_APP_FILE_PAUSE_CC           4
#define FDL_APP_FILE_RESUME_CC          5


/*
** Defines used in command(s) and app
*/
#define FDL_MAX_SEGMENT_SIZE      1024
#define FDL_DEFAULT_SEGMENT_SIZE  100
#define FDL_MIN_SEGMENT_SIZE      8

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

}  OS_PACK FDL_NoArgsCmd_t;

/*****************************************************************************/

typedef struct
{
    uint8              CmdHeader[CFE_SB_CMD_HDR_SIZE];
    char               FileName[OS_MAX_PATH_LEN];
    uint32             SegmentSize;
    uint32             Offset;

}  OS_PACK FDL_FileStartCmd_t;

/*****************************************************************************/

typedef struct
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint32             CurrentTransferSize;
    char               FileName[OS_MAX_PATH_LEN];

}  OS_PACK FDL_FileStart_t;

/*****************************************************************************/

typedef struct
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint16             SegmentNumber;    /* Starting with 1, 0 is the initialization case */ 
    uint16             SegmentDataSize;  /* Number bytes in SegmentData */
    uint8              SegmentData[FDL_MAX_SEGMENT_SIZE];

}  OS_PACK FDL_FileSegment_t;

/*****************************************************************************/

typedef struct
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint32             CurrentTransferSize;
    uint16             FileCrc;          /* Over entire file, even if only partial file sent */
    uint16             LastSegmentSent;  /* Last SegmentNumber sent in data packet */

}  OS_PACK FDL_FileComplete_t;

/*************************************************************************/

/*
** Type definition (FDL App housekeeping)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];

    uint8              fdl_command_count;
    uint8              fdl_command_error_count;
    uint8              TransferInProgress;
    uint8              TransferPaused;

    uint16             LastSegmentSent;
    uint16             SegmentSize;
    uint32             BytesTransferred;
    uint32             Offset;
    uint32             CurrentTransferSize;
    uint8              CurrentFileName[OS_MAX_PATH_LEN];
    uint32             CurrentFileSize;
    uint16             CurrentFileCrc;
    int32              CurrentFileFd;

}   OS_PACK fdl_hk_tlm_t  ;

#endif /* _fdl_msg_h_ */

/************************/
/*  End of File Comment */
/************************/
