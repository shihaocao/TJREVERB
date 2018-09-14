/************************************************************************
** File:
**   $Id: ful_utest.c $
**
** Purpose: 
**   This is a test driver used to unit test the File Uplink (FUL)
**   Application.
**
**   $Log: $
** 
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "utf_custom.h"        /* UTF headers         */
#include "utf_types.h"
#include "utf_cfe_sb.h"
#include "utf_osapi.h"
#include "utf_osloader.h"
#include "utf_osfileapi.h"
#include "utf_cfe.h"

#include "ful_perfids.h"
#include "ful_msgids.h"
/* #include "ful_platform_cfg.h"
*/

#include "ful_msg.h"
#include "ful_app.h"
#include "ful_events.h"
#include "ful_version.h"

#include "cfe_es_cds.h"        /* cFE headers         */

#include <stdlib.h>            /* System headers      */

/************************************************************************
** Macro Definitions
*************************************************************************/
#define MESSAGE_FORMAT_IS_CCSDS

/************************************************************************
** FUL global data external to this file
*************************************************************************/
extern  ful_hk_tlm_t            FUL_HkTelemetryPkt;



/************************************************************************
** Local data
*************************************************************************/

/*
** Global variables used by function hooks
*/
uint32   CFE_SB_SubscribeCallCount     = 1;
uint32   CFE_SB_SubscribeFailCount     = 1;
uint32   CFE_TBL_RegisterCallCount     = 1;
uint32   CFE_TBL_RegisterFailCount     = 1;
uint32   CFE_TBL_LoadCallCount         = 1;
uint32   CFE_TBL_LoadFailCount         = 1;
uint32   CFE_TBL_GetAddressCallCount   = 1; 
uint32   CFE_TBL_GetAddressFailCount   = 1;
uint32   CFE_TBL_GetAddressUpdateCount = 1;

uint32   UT_TotalTestCount = 0;
uint32   UT_TotalFailCount = 0;

uint16   UT_CommandPkt[256];

FUL_NoArgsCmd_t      *UT_NoopCmd =        (FUL_NoArgsCmd_t *)       &UT_CommandPkt[0];
FUL_NoArgsCmd_t      *UT_ResetCmd =       (FUL_NoArgsCmd_t *)       &UT_CommandPkt[0];
FUL_FileStartCmd_t   *UT_StartCmd =       (FUL_FileStartCmd_t *)    &UT_CommandPkt[0];
FUL_NoArgsCmd_t      *UT_CancelCmd =      (FUL_NoArgsCmd_t *)       &UT_CommandPkt[0];
FUL_FileFinishCmd_t  *UT_FinishCmd =      (FUL_NoArgsCmd_t *)       &UT_CommandPkt[0];
FUL_FileSegmentCmd_t *UT_SegmentCmd =     (FUL_FileSegmentCmd_t *)  &UT_CommandPkt[0];


FUL_NoArgsCmd_t      *UT_HkRequest =      (FUL_NoArgsCmd_t *)       &UT_CommandPkt[0];


/************************************************************************
** Local function prototypes
*************************************************************************/

void PrintHKPacket (uint8 source, void *packet);
void Test_ground_cmds ( void );
void Test_app_init ( void );

void GetInputChar ( void )
{
    int c;

    printf ( "Enter to continue\n" );
    c = getchar();
    while (c != 'A') {
        putchar(c);
        c = getchar();
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FDL unit test program main                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int main(void)
{
    /*
    ** Set up output results text file           
    */
    UTF_set_output_filename("ful_utest.out");

    /*
    ** Set up HK packet handler           
    */
    UTF_set_packet_handler(FUL_APP_HK_TLM_MID, (utf_packet_handler)PrintHKPacket);
    
    /*
    ** Initialize time data structures
    */
    UTF_init_sim_time(0.0);

    /*
    ** Initialize ES application data                         
    */
    UTF_ES_InitAppRecords();
    UTF_ES_AddAppRecord("FUL_APP",0);  
    CFE_ES_RegisterApp();

    /*
    ** Initialize CDS and table services data structures
    */
    CFE_ES_CDS_EarlyInit();
    CFE_TBL_EarlyInit();

    /*
     * Setup the virtual/physical file system mapping...
     *
     * The following local machine directory structure is required:
     *
     * ... ful/fsw/unit_test          <-- this is the current working directory
     * ... ful/fsw/unit_test/disk/TT  <-- physical location for virtual disk "/tt"
     */
    UTF_add_volume("/TT", "disk", FS_BASED, FALSE, FALSE, TRUE, "TT", "/tt", 0);

    /*
    ** Delete files created during previous tests
    */
    OS_remove("/tt/app00002000.tlm");
    OS_remove("/tt/app1980001000000.hk");
    OS_remove("/tt/b_00000000.x");
    OS_remove("/tt/b_99999999.x");
    OS_remove("/tt/testfile");
    OS_remove("/tt/test");

    /*
    ** Required setup prior to calling many CFE API functions
    */
    CFE_ES_RegisterApp();
    CFE_EVS_Register(NULL, 0, 0);

    /*
    ** Run FDL ground command unit tests
    */
    UTF_put_text("\n*** FUL -- Testing FUL commands ***\n");
    FUL_AppInit();  /* need to init HK packet as it's used in the test */
    Test_ground_cmds();

    UTF_put_text("\n*** FUL -- Total test count = %d, total test errors = %d\n\n", UT_TotalTestCount, UT_TotalFailCount);

    /*
    ** Run FDL app init unit tests
    */
    UTF_put_text("\n*** FUL -- Testing FDL app init ***\n");
    FUL_AppInit();
    Test_app_init();

    UTF_put_text("\n*** FUL -- Total test count = %d, total test errors = %d\n\n", UT_TotalTestCount, UT_TotalFailCount);

    /*
    ** Invoke the main loop "success" test now because the program
    **  will end when the last entry in the SB input file is read.
    */
    UTF_CFE_ES_Set_Api_Return_Code(CFE_ES_RUNLOOP_PROC, TRUE);
    FUL_AppMain();

    return 0;
   
} /* End of main() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Prints out the current values in the FDL housekeeping packet     */
/* data structure                                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void PrintHKPacket (uint8 source, void *packet)
{
    ful_hk_tlm_t *HkPacket = (ful_hk_tlm_t *) packet;
    
    /* Output the FDL housekeeping data */
    UTF_put_text("\nFDL HOUSEKEEPING DATA:");

    /* TODO Add HK telemetry here */
    UTF_put_text("CC = %d, EC = %d FIP = %d, [%d] [%d] [%d] [%08lX] [%s]\n", HkPacket->ful_command_count, HkPacket->ful_command_error_count, HkPacket->FileInProgress, HkPacket->LastSegmentAccepted, HkPacket->SegmentsRejected, HkPacket->BytesTransferred, HkPacket->CurrentFileCrc, HkPacket->CurrentFileName );
   
} /* End of PrintHKPacket() */

void HkRequest ( void )
{
    CFE_SB_InitMsg(UT_HkRequest, FUL_APP_SEND_HK_MID, sizeof(FUL_NoArgsCmd_t), TRUE);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_HkRequest);
}

void CancelCommand ( void )
{
    /* 
    ** Send a valid cancel command in the process of testing somethinge else. 
    */
    CFE_SB_InitMsg(UT_CancelCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_CancelCmd), FUL_APP_FILE_CANCEL_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_CancelCmd);
    if ( FUL_HkTelemetryPkt.FileInProgress != FALSE )
    {
        exit(0);
    }
}

void StartCommand ( void )
{
    /* 
    ** Send a valid start file playback command in the process of testing somethinge else. 
    */
    CFE_SB_InitMsg(UT_StartCmd, FUL_APP_CMD_MID, sizeof(FUL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FUL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), "/tt/testfile\0", OS_MAX_PATH_LEN );
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    if ( FUL_HkTelemetryPkt.FileInProgress != TRUE )
    {
        exit(0);
    }
}

void FinishCommand ( void )
{
    CFE_SB_InitMsg(UT_FinishCmd, FUL_APP_CMD_MID, sizeof(FUL_FileFinishCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_FinishCmd), FUL_APP_FILE_FINISH_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_FinishCmd);
}

void Test_ground_cmds ( void )
{
    uint16 CmdAcceptedCounter;
    uint16 CmdRejectedCounter;

    uint32 TestCount = 0;
    uint32 FailCount = 0;
 
    char Testfile1 [] = "/tt/test\0";


    /*
    ** Tests for function FUL Noop ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFUL_CmdNoop() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), FUL_APP_NOOP_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_CmdNoop() length to short -- test passed (1/3)\n");
    }
    else
    {
        UTF_put_text("FUL_CmdNoop() length to short -- test failed (1/3)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), FUL_APP_NOOP_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_CmdNoop() length to long -- test passed (2/3)\n");
    }
    else
    {
        UTF_put_text("FUL_CmdNoop() length to long -- test failed (2/3)\n");
        FailCount++;
    }

    /* (3) Valid packet length */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), FUL_APP_NOOP_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_CmdNoop() valid command -- test passed (3/3)\n");
    }
    else
    {
        UTF_put_text("FUL_CmdNoop() valid command -- test failed (3/3)\n");
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function FDL Reset Counters ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFUL_Reset Counters() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_ResetCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResetCmd), FUL_APP_RESET_COUNTERS_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResetCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_ResetCmd() length to short -- test passed (1/3)\n");
    }
    else
    {
        UTF_put_text("FUL_ResetCmd() length to short -- test failed (1/3)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_ResetCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResetCmd), FUL_APP_RESET_COUNTERS_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResetCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_ResetCmd() length to long -- test passesd (2/3)\n");
    }
    else
    {
        UTF_put_text("FUL_ResetCmd() length to long -- test failed (2/3)\n");
        FailCount++;
    }

    /* (3) Valid packet length */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_ResetCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResetCmd), FUL_APP_RESET_COUNTERS_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResetCmd);
    TestCount++;
    if ( ( FUL_HkTelemetryPkt.ful_command_count == 0 ) && ( FUL_HkTelemetryPkt.ful_command_error_count == 0 ) )
    {
        UTF_put_text("FUL_ResetCmd() valid command -- test passed (3/3)\n");
    }
    else
    {
        UTF_put_text("FUL_ResetCmd() valid command -- test failed (3/3)\n");
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function FDL invalid command headers ()...
    **
    **   (1)  invalid message ID
    **   (2)  invalid command code
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFUL_ Invalid command headers() tests\n");

    /* (1) invalid message ID */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, FUL_APP_CMD_MID-10, sizeof(FUL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), FUL_APP_NOOP_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FDL invalid message ID -- test passed (1/2)\n");
    }
    else
    {
        UTF_put_text("FDL invalid message ID -- test failed (1/2)\n");
        FailCount++;
    }

    /* (2) invalid command code */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), FUL_APP_NOOP_CC+20);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FDL invalid command code -- test passed (2/2)\n");
    }
    else
    {
        UTF_put_text("FDL invalid command code -- test failed (2/2)\n");
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function FUL cancel ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  no file transfer in progress  NOTE: No errors here.
    **   (4)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFUL_Cancel() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_CancelCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_CancelCmd), FUL_APP_FILE_CANCEL_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_CancelCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_Cancel() length to short -- test passed (1/4)\n");
    }
    else
    {
        UTF_put_text("FUL_Cancel() length to short -- test failed (1/4)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_CancelCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_CancelCmd), FUL_APP_FILE_CANCEL_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_CancelCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_Cancel() length to long -- test passed (2/4)\n");
    }
    else
    {
        UTF_put_text("FUL_Cancel() length to long -- test failed (2/4)\n");
        FailCount++;
    }

    /* (3) no file transfer in progress  NOTE: No errors here. */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_CancelCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_CancelCmd), FUL_APP_FILE_CANCEL_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_CancelCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter == FUL_HkTelemetryPkt.ful_command_error_count ) && ( FUL_HkTelemetryPkt.FileInProgress== FALSE ) )
    {
        UTF_put_text("FUL_Cancel() no file transfer in progress -- test passed (3/4)\n");
    }
    else
    {
        UTF_put_text("FUL_Cancel() no file transfer in progress -- test failed (3/4)\n");
        FailCount++;
    }

    /* (4) Valid packet length */
    StartCommand();
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_CancelCmd, FUL_APP_CMD_MID, sizeof(FUL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_CancelCmd), FUL_APP_FILE_CANCEL_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_CancelCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter == FUL_HkTelemetryPkt.ful_command_error_count ) && ( FUL_HkTelemetryPkt.FileInProgress== FALSE ) )
    {
        UTF_put_text("FUL_Cancel() file transfer in progress -- test passed (4/4)\n");
    }
    else
    {
        UTF_put_text("FUL_Cancel() file transfer in progress -- test failed (4/4)\n");
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function FUL HK request ()...
    **
    **   (1)  good HK request packet (only validation is message ID and if wrong then not a HK request.
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFUL_HK() tests\n");

    /* (1) Valid HK request */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_HkRequest, FUL_APP_SEND_HK_MID, sizeof(FUL_NoArgsCmd_t), TRUE);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_HkRequest);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_HK Request() -- test passed (1/1)\n");
    }
    else
    {
        UTF_put_text("FUL_HK Request() -- test failed (1/1)\n");
        FailCount++;
    }

    /*
    ** Tests for function FUL start ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  file transfer in progress
    **   (4)  open failures (unknown file)
    **   (5)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFUL_Start() tests\n");
    HkRequest();

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FUL_APP_CMD_MID, sizeof(FUL_FileStartCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FUL_APP_FILE_START_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_CmdNoop() length to short -- test passed (1/5)\n");
    }
    else
    {
        UTF_put_text("FUL_CmdNoop() length to short -- test failed (1/5)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FUL_APP_CMD_MID, sizeof(FUL_FileStartCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FUL_APP_FILE_START_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_CmdNoop() length to long -- test passed (2/5)\n");
    }
    else
    {
        UTF_put_text("FUL_CmdNoop() length to long -- test failed (2/5)\n");
        FailCount++;
    }

    /* (3) file transfer in progress */
    HkRequest();
    StartCommand();
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FUL_APP_CMD_MID, sizeof(FUL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FUL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), "/tt/testfile.out", OS_MAX_PATH_LEN );
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_CmdNoop() valid command -- test passed (3/5)\n");
    }
    else
    {
        UTF_put_text("FUL_CmdNoop() valid command -- test failed (3/5)\n");
        FailCount++;
    }
    HkRequest();
    CancelCommand();

    /* (4) open failures (unknown file). */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FUL_APP_CMD_MID, sizeof(FUL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FUL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), "/nofile", OS_MAX_PATH_LEN );
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_CmdNoop() valid command -- test passed (4/5)\n");
    }
    else
    {
        UTF_put_text("FUL_CmdNoop() valid command -- test failed (4/5)\n");
        FailCount++;
    }
    HkRequest();

    /* (5) Good command paket */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FUL_APP_CMD_MID, sizeof(FUL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FUL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), &(Testfile1[0]), OS_MAX_PATH_LEN );
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_CmdNoop() valid command -- test passed (5/5)\n");
    }
    else
    {
        UTF_put_text("FUL_CmdNoop() valid command -- test failed (5/5)\n");
        FailCount++;
    }
    HkRequest();


    /*
    ** Tests for function FUL finish ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  file transfer NOT in progress (this also covers valid case)
    **   (4)  invalid CRC
    **   (5)  invalid size
    **   (6)  invalid last segment
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFUL_Finish() tests\n");
    HkRequest();

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_FinishCmd, FUL_APP_CMD_MID, sizeof(FUL_FileFinishCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_FinishCmd), FUL_APP_FILE_FINISH_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_FinishCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_FinishCmd() length to short -- test passed (1/6)\n");
    }
    else
    {
        UTF_put_text("FUL_FinishCmd() length to short -- test failed (1/6)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_FinishCmd, FUL_APP_CMD_MID, sizeof(FUL_FileFinishCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_FinishCmd), FUL_APP_FILE_FINISH_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_FinishCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_FinishCmd() length to long -- test passed (2/6)\n");
    }
    else
    {
        UTF_put_text("FUL_FinishCmd() length to long -- test failed (2/6)\n");
        FailCount++;
    }

    /* (3) file transfer NOT in progress */
    HkRequest();
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_FinishCmd, FUL_APP_CMD_MID, sizeof(FUL_FileFinishCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_FinishCmd), FUL_APP_FILE_FINISH_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_FinishCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_FinishCmd() valid command -- test passed (3/6)\n");
    }
    else
    {
        UTF_put_text("FUL_FinishCmd() valid command -- test failed (3/6)\n");
        FailCount++;
    }
    HkRequest();

    /* (4) invalid CRC */
    HkRequest();
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_FinishCmd, FUL_APP_CMD_MID, sizeof(FUL_FileFinishCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_FinishCmd), FUL_APP_FILE_FINISH_CC);
    UT_FinishCmd->FileCrc=0x12345678;
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_FinishCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_FinishCmd() invalid CRC command -- test passed (4/6)\n");
    }
    else
    {
        UTF_put_text("FUL_FinishCmd() invalid CRC command -- test failed (4/6)\n");
        FailCount++;
    }
    HkRequest();

    /* (4) invalid Size */
    HkRequest();
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_FinishCmd, FUL_APP_CMD_MID, sizeof(FUL_FileFinishCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_FinishCmd), FUL_APP_FILE_FINISH_CC);
    UT_FinishCmd->FileSize=0x12345678;
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_FinishCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_FinishCmd() invalid Size command -- test passed (5/6)\n");
    }
    else
    {
        UTF_put_text("FUL_FinishCmd() invalid Size command -- test failed (5/6)\n");
        FailCount++;
    }
    HkRequest();

    /* (5) invalid last segment */
    HkRequest();
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_FinishCmd, FUL_APP_CMD_MID, sizeof(FUL_FileFinishCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_FinishCmd), FUL_APP_FILE_FINISH_CC);
    UT_FinishCmd->FileLastSegment=0x1234;
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_FinishCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_FinishCmd() invalid last segment command -- test passed (6/6)\n");
    }
    else
    {
        UTF_put_text("FUL_FinishCmd() invalid last segment command -- test failed (6/6)\n");
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function FUL segment ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  file transfer NOT in progress
    **   (4)  invalid segment size
    **   (5)  invalid segment number
    **   (6)  invalid OS write error
    **   (7)  valid command
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFUL_Segment() tests\n");
    HkRequest();

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_SegmentCmd, FUL_APP_CMD_MID, sizeof(FUL_FileSegmentCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SegmentCmd), FUL_APP_FILE_SEGMENT_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_SegmentCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_SegmentCmd() length to short -- test passed (1/6)\n");
    }
    else
    {
        UTF_put_text("FUL_SegmentCmd() length to short -- test failed (1/6)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_SegmentCmd, FUL_APP_CMD_MID, sizeof(FUL_FileSegmentCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SegmentCmd), FUL_APP_FILE_SEGMENT_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_SegmentCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_SegmentCmd() length to long -- test passed (2/6)\n");
    }
    else
    {
        UTF_put_text("FUL_SegmentCmd() length to long -- test failed (2/6)\n");
        FailCount++;
    }

    /* (3)  file transfer NOT in progress */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_SegmentCmd, FUL_APP_CMD_MID, sizeof(FUL_FileSegmentCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SegmentCmd), FUL_APP_FILE_SEGMENT_CC);
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_SegmentCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_SegmentCmd() file transfer NOT in progress -- test passed (3/6)\n");
    }
    else
    {
        UTF_put_text("FUL_SegmentCmd() file transfer NOT in progress -- test failed (3/6)\n");
        FailCount++;
    }

    StartCommand(); /* for 4 and 5 and 6 and 7 */

    /* (4)  invalid segment size */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_SegmentCmd, FUL_APP_CMD_MID, sizeof(FUL_FileSegmentCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SegmentCmd), FUL_APP_FILE_SEGMENT_CC);
    UT_SegmentCmd->SegmentDataSize=FUL_FILE_SEGMENT_SIZE + 1;
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_SegmentCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_SegmentCmd() invalid segment size -- test passed (5/6)\n");
    }
    else
    {
        UTF_put_text("FUL_SegmentCmd() invalid segment size -- test failed (5/6)\n");
        FailCount++;
    }

    /* (5)  invalid segment number */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_SegmentCmd, FUL_APP_CMD_MID, sizeof(FUL_FileSegmentCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SegmentCmd), FUL_APP_FILE_SEGMENT_CC);
    UT_SegmentCmd->SegmentNumber = 80;
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_SegmentCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_SegmentCmd() invalid segment number -- test passed (5/6)\n");
    }
    else
    {
        UTF_put_text("FUL_SegmentCmd() invalid segment number -- test failed (5/6)\n");
        FailCount++;
    }

    /* (6)  OS file write error (this stops the file uplink) */
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_SegmentCmd, FUL_APP_CMD_MID, sizeof(FUL_FileSegmentCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SegmentCmd), FUL_APP_FILE_SEGMENT_CC);
    UT_SegmentCmd->SegmentNumber = 1;
    UTF_OSFILEAPI_Set_Api_Return_Code ( OS_WRITE_PROC, -1 );
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_SegmentCmd);
    UTF_OSFILEAPI_Use_Default_Api_Return_Code ( OS_WRITE_PROC );
    TestCount++;
    if ( ( CmdAcceptedCounter == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter + 1 == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_SegmentCmd() OS file write error -- test passed (6/6)\n");
    }
    else
    {
        UTF_put_text("FUL_SegmentCmd() OS file write error -- test failed (6/6)\n");
        FailCount++;
    }

    /* (7)  valid file segement */
    StartCommand(); 
    CmdAcceptedCounter = FUL_HkTelemetryPkt.ful_command_count;
    CmdRejectedCounter = FUL_HkTelemetryPkt.ful_command_error_count;
    CFE_SB_InitMsg(UT_SegmentCmd, FUL_APP_CMD_MID, sizeof(FUL_FileSegmentCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SegmentCmd), FUL_APP_FILE_SEGMENT_CC);
    UT_SegmentCmd->SegmentNumber = 1;
    UT_SegmentCmd->SegmentDataSize = 3;
    UT_SegmentCmd->SegmentData [0] = 'D';
    UT_SegmentCmd->SegmentData [1] = 'a';
    UT_SegmentCmd->SegmentData [2] = 'n';
    UTF_put_text("XXXXXXXXX\n");
    //HkRequest();
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_SegmentCmd);
    //HkRequest();
    UTF_put_text("XXXXXXXXX\n");
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FUL_HkTelemetryPkt.ful_command_count ) && ( CmdRejectedCounter == FUL_HkTelemetryPkt.ful_command_error_count ) )
    {
        UTF_put_text("FUL_SegmentCmd() Valid File Segement Command -- test passed (7/6)\n");
    }
    else
    {
        UTF_put_text("FUL_SegmentCmd() Valid File Segement Command -- test failed (7/6)\n");
        FailCount++;
    }
    // FinishCommand();  Need to set last segment number.
    CFE_SB_InitMsg(UT_FinishCmd, FUL_APP_CMD_MID, sizeof(FUL_FileFinishCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_FinishCmd), FUL_APP_FILE_FINISH_CC);
    UT_FinishCmd->FileSize = 3;
    UT_FinishCmd->FileLastSegment = 2;
    UT_FinishCmd->FileCrc = 0x000069E8;
    FUL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_FinishCmd);
    HkRequest();

}

int32 SubscribeHook = 0;

int32 CFE_SB_SubscribeHook(CFE_SB_MsgId_t  MsgId, CFE_SB_PipeId_t PipeId)
{
    SubscribeHook++;

    /*
    ** 1 is first call (of 1 call) in first loop.
    ** 3 is second call (of 2 calls) in second loop.
    ** 6 is third call (of 3 calls) in third loop.
    */
    if ((SubscribeHook == 1) || (SubscribeHook == 3) || (SubscribeHook == 6))
        return (-1);
    else
        return (CFE_SUCCESS);
}

void Test_app_init ( void )
{
    int32  iResult;
    uint32 TestCount = 0;
    uint32 FailCount = 0;


    /* app init with invalid EVS register, invalid create pipe, invalid subscribe to HK, invalid subscribe to cmds, invalid subscribe to wakeup, invalid get semaphore by name, */

    UTF_CFE_ES_Set_Api_Return_Code(CFE_ES_REGISTERAPP_PROC, -1);
    FUL_AppMain();
    UTF_CFE_ES_Use_Default_Api_Return_Code(CFE_ES_REGISTERAPP_PROC);

    /* (1) CFE_EVS_Register error */
    UTF_CFE_EVS_Set_Api_Return_Code(CFE_EVS_REGISTER_PROC, -1);
    iResult = FUL_AppInit();
    UTF_CFE_EVS_Use_Default_Api_Return_Code(CFE_EVS_REGISTER_PROC);
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("FUL_AppInit() -- EVS Register Error test failed (1)\n");
        FailCount++;
    }

    /* (2) CFE_SB_CreatePipe error */
    UTF_CFE_SB_Set_Api_Return_Code(CFE_SB_CREATEPIPE_PROC, -1);
    iResult = FUL_AppInit();
    UTF_CFE_SB_Use_Default_Api_Return_Code(CFE_SB_CREATEPIPE_PROC);
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("FUL_AppInit() -- Create Pipe Error test failed (2)\n");
        FailCount++;
    }

    UTF_SB_set_function_hook(CFE_SB_SUBSCRIBE_HOOK, (void *)&CFE_SB_SubscribeHook);

    /* (3) CFE_SB_Subscribe error - HK request commands */
    iResult = FUL_AppInit();
    TestCount++;
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("FUL_AppInit() -- Subscribe HK Request Error test failed (3)\n");
        FailCount++;
    }

    /* (4) CFE_SB_Subscribe error - FUL ground commands */
    iResult = FUL_AppInit();
    TestCount++;
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("FUL_AppInit() -- Subscribe Ground Command Error test failed (4)\n");
        FailCount++;
    }

}


