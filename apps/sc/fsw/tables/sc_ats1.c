
/************************************************************************
**
** $Id: sc_ats1.c 1.3 2010/03/30 11:52:14EDT lwalling Exp  $
**
** CFS Stored Command (SC) sample ATS table #1
**
** Note 1: The following source code demonstrates how to create a sample
**         Stored Command ATS table.  The preferred method for creating
**         flight versions of ATS tables is to use custom ground system
**         tools that output the binary table files, skipping this step
**         altogether.
**         
** Note 2: This source file creates a sample ATS table that contains the
**         following commands that are scheduled as follows:
**
**         SC NOOP command, execution time = SC_TEST_TIME + 30
**         SC Enable RTS #1 command, execution time = SC_TEST_TIME + 35
**         SC Start RTS #1 command, execution time = SC_TEST_TIME + 40
**         SC Reset Counters command, execution time = SC_TEST_TIME + 100
**
** Note 3: Before starting the sample ATS, set time = SC_TEST_TIME.  The
**         user will then have 30 seconds to start the ATS before the
**         first command in the sample ATS is scheduled to execute.
**
** Note 4: The byte following the command code in each command packet
**         secondary header must contain an 8 bit checksum.  Refer to
**         the SC Users Guide for information on how to calculate this
**         checksum.
**
** $Log: sc_ats1.c  $
** Revision 1.3 2010/03/30 11:52:14EDT lwalling 
** Calculate correct command checksum values
** Revision 1.2 2010/03/26 18:04:19EDT lwalling 
** Remove pad from ATS and RTS structures, change 32 bit ATS time to two 16 bit values
** Revision 1.1 2010/03/16 15:43:07EDT lwalling 
** Initial revision
** Member added to project c:/MKSDATA/MKS-REPOSITORY/CFS-REPOSITORY/sc/fsw/tables/project.pj
**
*************************************************************************/

#include "cfe.h"
#include "cfe_tbl_filedef.h"

#include "sc_platform_cfg.h"    /* defines table buffer size */
#include "sc_msgdefs.h"         /* defines SC command code values */
#include "sc_msgids.h"          /* defines SC packet msg ID's */


/*
** Arbitrary spacecraft time for start of sample ATS
*/
#define TEST_TIME     1000000


/*
** Execution time for each sample command
*/
#define CMD1_TIME     (TEST_TIME + 30)
#define CMD2_TIME     (TEST_TIME + 35)
#define CMD3_TIME     (TEST_TIME + 40)
#define CMD4_TIME     (TEST_TIME + 100)


/*
** Create execution time as two 16 bit values
*/
#define CMD1_TIME_A   ((uint16) ((uint32) CMD1_TIME >> 16))
#define CMD2_TIME_A   ((uint16) ((uint32) CMD2_TIME >> 16))
#define CMD3_TIME_A   ((uint16) ((uint32) CMD3_TIME >> 16))
#define CMD4_TIME_A   ((uint16) ((uint32) CMD4_TIME >> 16))

#define CMD1_TIME_B   ((uint16) ((uint32) CMD1_TIME))
#define CMD2_TIME_B   ((uint16) ((uint32) CMD2_TIME))
#define CMD3_TIME_B   ((uint16) ((uint32) CMD3_TIME))
#define CMD4_TIME_B   ((uint16) ((uint32) CMD4_TIME))


/*
** Calculate checksum for each sample command
*/
#define CMD1_XSUM     0x008F
#define CMD2_XSUM     0x008B
#define CMD3_XSUM     0x0088
#define CMD4_XSUM     0x008E


/*
** Optional command data values
*/
#define CMD2_ARG      1
#define CMD3_ARG      1


/*
** Command packet segment flags and sequence counter
** - 2 bits of segment flags (0xC000 = start and end of packet)
** - 14 bits of sequence count (unused for command packets)
*/
#define PKT_FLAGS     0xC000


/*
** Length of cmd pkt data (in bytes minus one) that follows primary header (thus, 0xFFFF = 64k)
*/
#define CMD1_LENGTH   1
#define CMD2_LENGTH   3
#define CMD3_LENGTH   3
#define CMD4_LENGTH   1


/*
** Sample ATS_TBL1 Table Header
*/
static CFE_TBL_FileDef_t CFE_TBL_FileDef =
{
    "ATS_Table1", "SC.ATS_TBL1", "SC Sample ATS_TBL1",
    "sc_ats1.tbl", (SC_ATS_BUFF_SIZE * sizeof(uint16))
};


/*
** Sample ATS_TBL1 Table Data
*/
uint16 ATS_Table1[SC_ATS_BUFF_SIZE] =
{
  /* cmd num, <---- cmd exe time ---->   <---- cmd pkt primary header ---->  <----- cmd pkt 2nd header ---->   <-- opt data ---> */
           1, CMD1_TIME_A, CMD1_TIME_B,  SC_CMD_MID, PKT_FLAGS, CMD1_LENGTH, ((SC_NOOP_CC << 8) | CMD1_XSUM),
           2, CMD2_TIME_A, CMD2_TIME_B,  SC_CMD_MID, PKT_FLAGS, CMD2_LENGTH, ((SC_ENABLE_RTS_CC << 8) | CMD2_XSUM), CMD2_ARG,
           3, CMD3_TIME_A, CMD3_TIME_B,  SC_CMD_MID, PKT_FLAGS, CMD3_LENGTH, ((SC_START_RTS_CC << 8) | CMD3_XSUM), CMD3_ARG,
           4, CMD4_TIME_A, CMD4_TIME_B,  SC_CMD_MID, PKT_FLAGS, CMD4_LENGTH, ((SC_RESET_COUNTERS_CC << 8) | CMD4_XSUM)
};

/************************/
/*  End of File Comment */
/************************/
