/*******************************************************************************
** File:
**   $Id: ful_msg.h  $
**
** Purpose: 
**  Define FUL App Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _ful_msg_h_
#define _ful_msg_h_

/*
** FUL App command codes
*/
#define FUL_APP_NOOP_CC                 0
#define FUL_APP_RESET_COUNTERS_CC       1
#define FUL_APP_FILE_START_CC           2
#define FUL_APP_FILE_SEGMENT_CC         3
#define FUL_APP_FILE_FINISH_CC          4
#define FUL_APP_FILE_CANCEL_CC          5

/*
** Defines used in command(s) and app
*/

#define FUL_FILE_SEGMENT_SIZE     48

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

}   OS_PACK FUL_NoArgsCmd_t;

/*****************************************************************************/

typedef struct
{
    uint8              CmdHeader[CFE_SB_CMD_HDR_SIZE];
    char               FileName[OS_MAX_PATH_LEN];  

}   OS_PACK FUL_FileStartCmd_t;

/*****************************************************************************/

typedef struct
{
    uint8              CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint16             SegmentNumber;     /* First segment to have data is 1, 0 is the init case */
    uint16             SegmentDataSize;   /* Number of bytes in the SegmentData */ 
    uint8              SegmentData[FUL_FILE_SEGMENT_SIZE];

}   OS_PACK FUL_FileSegmentCmd_t;

/*****************************************************************************/

typedef struct
{
    uint8              CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint32             FileSize;
    uint16             FileCrc;
    uint16             FileLastSegment;

}   OS_PACK FUL_FileFinishCmd_t;

/*************************************************************************/
/*
** Type definition (FUL App housekeeping)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8              ful_command_count;
    uint8              ful_command_error_count;
    uint16             FileInProgress;
    uint16             LastSegmentAccepted;
    uint16             SegmentsRejected;

    uint32             BytesTransferred;
    uint8              CurrentFileName[OS_MAX_PATH_LEN];
    uint16             CurrentFileCrc;
    int32              CurrentFileFd;
    uint16             SuccessfulFileUploads;

}   OS_PACK ful_hk_tlm_t  ;

#define FUL_APP_HK_TLM_LNGTH   sizeof ( ful_hk_tlm_t )

#endif /* _ful_msg_h_ */

/************************/
/*  End of File Comment */
/************************/
