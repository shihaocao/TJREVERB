/*
** File: cadet_msg.h
**
** Purpose:
**   Define CADET App Main Routine command messages.
**
*/
#ifndef _cadet_msg_h_
#define _cadet_msg_h_



/*
**   Includes:
*/
#include "cfe.h"
#include "cadet_radio.h"



/*
** Defines
*/
#define CADET_APP_NOOP_CC                 0
#define CADET_APP_RESET_COUNTERS_CC       1
#define CADET_APP_TAKE_DOWNLINK_SEM_CC    2       /* Take one off the downlike semaphore pipe */
#define CADET_APP_GIVE_DOWNLINK_SEM_CC    3       /* Give one to the downlink semaphore pipe */
#define CADET_APP_JAM_FILTER_FACTOR_CC    5       /* Jam Filter Factor */
#define CADET_APP_START_PLAYBACK_CC       6       /* Start a playback of the cadet fifo */
#define CADET_APP_ABORT_PLAYBACK_FLUSH_CC 7       /* Abort a playback or flush of the cadet fifo */
#define CADET_APP_FLUSH_FIFO_CC           8       /* Flush a FIFO */
#define CADET_APP_FLUSH_HI_FIFO_CC        9       /* Flush hi FIFO */
#define CADET_APP_PB_FLUSH_HI_FIFO_CC     10      /* Playback then flush hi FIFO */
#define CADET_APP_PB_LO_HI_FIFO_CC        11      /* Playback low fifo with some hi fifo */
#define CADET_MAX_SIZE_TO_AUTO_FLUSH_CC   12      /* Set maximum HI fifo size prior to auto flush */
#define CADET_PACKETS_TO_AUTO_FLUSH_CC    13      /* Number of packets to auto flush */
#define CADET_PACKETS_TO_AUTO_PLAYBACK_CC 14      /* Number of packets to auto playback */
#define CADET_APP_JAM_FILTER_RANGE_CC     15      /* Jam Filter Factor Range */
#define CADET_APP_INIT_CADET_UART_CC      16      /* Reset Cadet Uart */
#define CADET_APP_RESET_CADET_CC          17      /* Reset Cadet */



/*
** Structures
*/

/* Type definition (generic "no arguments" command) */
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

} OS_PACK  CADET_NoArgsCmd_t;

/* This is the format for the Jam Filter Factor command. */
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

    uint16   MessageID;  /** packet's message ID to change **/
    uint8    Pipe;       /** HI = 0 | LO = 1 | BOTH = 2 **/
    uint8    Method;     /** Sequence or Time filtering (Time =0 | Sequence =1) **/
    uint16   ValueN;     /** Number of packets to pass **/
    uint16   ValueX;     /** Number of packets in filter group **/
    uint16   ValueO;     /** Offset value within group **/

} OS_PACK  CADET_JamFilterCmd_t;

/* This is the format for the Jam Filter range command. */
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

    uint16   StartMID;   /** Start packet's message ID to change **/
    uint16   EndMID;     /** End packet's message ID to change **/
    uint8    Pipe;       /** HI = 0 | LO = 1 | BOTH = 2 **/
    uint8    Method;     /** Sequence or Time filtering (Time =0 | Sequence =1) **/
    uint16   ValueN;     /** Number of packets to pass **/
    uint16   ValueX;     /** Number of packets in filter group **/
    uint16   ValueO;     /** Offset value within group **/

} OS_PACK  CADET_JamFilterRangeCmd_t;

/* This is the format for the start playback command. */
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

    uint8    PlaybackFifo;
    uint32   PlaybackPktsPerRequest;
    uint32   PlaybackTotalPacketsToSend;

} OS_PACK  CADET_StartPlaybackCmd_t;

/* This is the format for the flush FIFO command. */
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

    uint8    FlushFifo;
    uint32   FlushPktsPerRequest;
    uint32   FlushTotalPacketsToClear;

} OS_PACK  CADET_FlushFifoCmd_t;

/* This is the format for the flush entire hi FIFO command. */
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

    uint32   FlushPktsPerRequest;

} OS_PACK  CADET_FlushEntireFifoHiCmd_t;

/* This is the format for the playback and flush entire hi FIFO command. */
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

    uint32   PktsPerRequest;

} OS_PACK  CADET_PlaybackAndFlushHiFifoCmd_t;

/* This is the format for the start Lo FIFO playback interleaved with hi FIFO playback command. */
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

    uint32   PlaybackPktsPerRequest;
    uint32   PlaybackTotalPacketsToSend;

} OS_PACK  CADET_StartLoFifoWithHiFifoPlaybackCmd_t;

/* This is the format for the maximum high fifo size prior to auto flush command. */
typedef struct
{
    uint8              CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint16             MaxSize;

}  OS_PACK  CADET_MaxiumuSizeToAutoFlushCmd_t;

/* This is the format for the number of high fifo packets to auto flush command. */
typedef struct
{
    uint8              CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint16             NumPackets;

}  OS_PACK  CADET_NumberPacketsToAutoFlushCmd_t;

/* This is the format for the number of packets to auto playback upon command. */
typedef struct
{
    uint8              CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint16             NumPackets;

}  OS_PACK  CADET_NumberPacketsToAutoPlaybackCmd_t;



/*
** Exported Variables
*/



/*
** Exported Functions
*/



#endif /* _cadet_msg_h_ */



/************************/
/*  End of File Comment */
/************************/
