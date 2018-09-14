/************************************************************************
** File:
**   $Id: fdl_utest.c $
**
** Purpose: 
**   This is a test driver used to unit test the File Downlink (FDL)
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

#include "fdl_perfids.h"
#include "fdl_msgids.h"

#include "fdl_msg.h"
#include "fdl_app.h"
#include "fdl_events.h"
#include "fdl_version.h"

#include "cfe_es_cds.h"        /* cFE headers         */

#include <stdlib.h>            /* System headers      */

/************************************************************************
** Macro Definitions
*************************************************************************/
#define MESSAGE_FORMAT_IS_CCSDS

/************************************************************************
** FDL global data external to this file
*************************************************************************/
extern  fdl_hk_tlm_t            FDL_HkTelemetryPkt;



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

FDL_NoArgsCmd_t      *UT_NoopCmd =        (FDL_NoArgsCmd_t *)       &UT_CommandPkt[0];
FDL_NoArgsCmd_t      *UT_ResetCmd =       (FDL_NoArgsCmd_t *)       &UT_CommandPkt[0];
FDL_FileStartCmd_t   *UT_StartCmd =       (FDL_FileStartCmd_t *)    &UT_CommandPkt[0];
FDL_NoArgsCmd_t      *UT_CancelCmd =      (FDL_NoArgsCmd_t *)       &UT_CommandPkt[0];
FDL_NoArgsCmd_t      *UT_PauseCmd =       (FDL_NoArgsCmd_t *)       &UT_CommandPkt[0];
FDL_NoArgsCmd_t      *UT_ResumeCmd =      (FDL_NoArgsCmd_t *)       &UT_CommandPkt[0];


FDL_NoArgsCmd_t      *UT_HkRequest =      (FDL_NoArgsCmd_t *)       &UT_CommandPkt[0];
FDL_NoArgsCmd_t      *UT_WakeupRequest =  (FDL_NoArgsCmd_t *)       &UT_CommandPkt[0];


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
    UTF_set_output_filename("fdl_utest.out");

    /*
    ** Set up HK packet handler           
    */
    UTF_set_packet_handler(FDL_APP_HK_TLM_MID, (utf_packet_handler)PrintHKPacket);
    
    /*
    ** Initialize time data structures
    */
    UTF_init_sim_time(0.0);

    /*
    ** Initialize ES application data                         
    */
    UTF_ES_InitAppRecords();
    UTF_ES_AddAppRecord("FDL_APP",0);  
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
     * ... fdl/fsw/unit_test          <-- this is the current working directory
     * ... fdl/fsw/unit_test/disk/TT  <-- physical location for virtual disk "/tt"
     */
    UTF_add_volume("/TT", "disk", FS_BASED, FALSE, FALSE, TRUE, "TT", "/tt", 0);

    /*
    ** Delete files created during previous tests
    */
    OS_remove("/tt/app00002000.tlm");
    OS_remove("/tt/app1980001000000.hk");
    OS_remove("/tt/b_00000000.x");
    OS_remove("/tt/b_99999999.x");

    /*
    ** Required setup prior to calling many CFE API functions
    */
    CFE_ES_RegisterApp();
    CFE_EVS_Register(NULL, 0, 0);

    /*
    ** Run FDL ground command unit tests
    */
    UTF_put_text("\n*** FDL -- Testing FDL commands ***\n");
    FDL_AppInit();  /* need to init HK packet as it's used in the test */
    Test_ground_cmds();

    UTF_put_text("\n*** FDL -- Total test count = %d, total test errors = %d\n\n", UT_TotalTestCount, UT_TotalFailCount);

    /*
    ** Run FDL app init unit tests
    */
    UTF_put_text("\n*** FDL -- Testing FDL app init ***\n");
    FDL_AppInit();
    Test_app_init();

    UTF_put_text("\n*** FDL -- Total test count = %d, total test errors = %d\n\n", UT_TotalTestCount, UT_TotalFailCount);

    /*
    ** Invoke the main loop "success" test now because the program
    **  will end when the last entry in the SB input file is read.
    */
    UTF_CFE_ES_Set_Api_Return_Code(CFE_ES_RUNLOOP_PROC, TRUE);
    FDL_AppMain();

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
    fdl_hk_tlm_t *HkPacket = (fdl_hk_tlm_t *) packet;
    
    /* Output the FDL housekeeping data */
    UTF_put_text("\nFDL HOUSEKEEPING DATA:");

    UTF_put_text("CC = %d, EC = %d, TIP = %d, TP = %d, [%d] [%d] [%d] [%d] [%d] [%d] [%d] [] \n", HkPacket->fdl_command_count, HkPacket->fdl_command_error_count, HkPacket->TransferInProgress, HkPacket->TransferPaused, HkPacket->LastSegmentSent, HkPacket->SegmentSize, HkPacket->BytesTransferred, HkPacket->Offset, HkPacket->CurrentTransferSize, HkPacket->CurrentFileSize, HkPacket->CurrentFileCrc );
   
} /* End of PrintHKPacket() */

void HkRequest ( void )
{
    CFE_SB_InitMsg(UT_HkRequest, FDL_APP_SEND_HK_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_HkRequest);
}

void WakupRequest ( void )
{
    /* 
    ** Send a valid wakeup in the process of testing somethinge else. 
    */
    CFE_SB_InitMsg(UT_HkRequest, FDL_APP_WAKEUP_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_HkRequest);
}

void PauseCommand ( void )
{
    /* 
    ** Send a valid pause in the process of testing somethinge else. 
    */
    CFE_SB_InitMsg(UT_PauseCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_PauseCmd), FDL_APP_FILE_PAUSE_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_PauseCmd);
    if ( FDL_HkTelemetryPkt.TransferPaused != TRUE )
    {
        exit(0);
    }
}

void ResumeCommand ( void )
{
    /* 
    ** Send a valid resume command in the process of testing somethinge else. 
    */
    CFE_SB_InitMsg(UT_ResumeCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResumeCmd), FDL_APP_FILE_RESUME_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResumeCmd);
    if ( FDL_HkTelemetryPkt.TransferPaused != FALSE )
    {
        exit(0);
    }
}

void CancelCommand ( void )
{
    /* 
    ** Send a valid cancel command in the process of testing somethinge else. 
    */
    CFE_SB_InitMsg(UT_CancelCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_CancelCmd), FDL_APP_FILE_CANCEL_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_CancelCmd);
    if ( FDL_HkTelemetryPkt.TransferInProgress != FALSE )
    {
        exit(0);
    }
}

void StartCommand ( void )
{
    /* 
    ** Send a valid start file playback command in the process of testing somethinge else. 
    */
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), "/tt/testfile\0", OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 100;
    UT_StartCmd->Offset = 0;
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    if ( FDL_HkTelemetryPkt.TransferInProgress != TRUE )
    {
        exit(0);
    }
}

void StartCommandNoErrorCheck ( void )
{
    /* 
    ** Send a valid start file playback command in the process of testing somethinge else. 
    */
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), "/tt/testfile\0", OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 100;
    UT_StartCmd->Offset = 0;
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
}

void Test_ground_cmds ( void )
{
    uint16 CmdAcceptedCounter;
    uint16 CmdRejectedCounter;

    uint32 TestCount = 0;
    uint32 FailCount = 0;
 
    char Testfile1 [] = "/tt/testfile\0";


    /*
    ** Tests for function FDL Noop ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFDL_CmdNoop() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), FDL_APP_NOOP_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() length to short -- test passed (1/3)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() length to short -- test failed (1/3)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), FDL_APP_NOOP_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() length to long -- test passed (2/3)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() length to long -- test failed (2/3)\n");
        FailCount++;
    }

    /* (3) Valid packet length */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), FDL_APP_NOOP_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test passed (3/3)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test failed (3/3)\n");
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
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFDL_Reset Counters() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_ResetCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResetCmd), FDL_APP_RESET_COUNTERS_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResetCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_ResetCmd() length to short -- test passed (1/3)\n");
    }
    else
    {
        UTF_put_text("FDL_ResetCmd() length to short -- test failed (1/3)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_ResetCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResetCmd), FDL_APP_RESET_COUNTERS_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResetCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_ResetCmd() length to long -- test passesd (2/3)\n");
    }
    else
    {
        UTF_put_text("FDL_ResetCmd() length to long -- test failed (2/3)\n");
        FailCount++;
    }

    /* (3) Valid packet length */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_ResetCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResetCmd), FDL_APP_RESET_COUNTERS_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResetCmd);
    TestCount++;
    if ( ( FDL_HkTelemetryPkt.fdl_command_count == 0 ) && ( FDL_HkTelemetryPkt.fdl_command_error_count == 0 ) )
    {
        UTF_put_text("FDL_ResetCmd() valid command -- test passed (3/3)\n");
    }
    else
    {
        UTF_put_text("FDL_ResetCmd() valid command -- test failed (3/3)\n");
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function FDL invalid command headers ()...
    **
    **   (1)  invalid message ID
    **   (2)  invalid command code
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFDL_ Invalid command headers() tests\n");

    /* (1) invalid message ID */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, FDL_APP_CMD_MID-10, sizeof(FDL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), FDL_APP_NOOP_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL invalid message ID -- test passed (1/2)\n");
    }
    else
    {
        UTF_put_text("FDL invalid message ID -- test failed (1/2)\n");
        FailCount++;
    }

    /* (2) invalid command code */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), FDL_APP_NOOP_CC+20);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
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
    ** Tests for function FDL cancel ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  no file transfer in progress  NOTE: No errors here.
    **   (4)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFDL_Cancel() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_CancelCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_CancelCmd), FDL_APP_FILE_CANCEL_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_CancelCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_Cancel() length to short -- test passed (1/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Cancel() length to short -- test failed (1/4)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_CancelCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_CancelCmd), FDL_APP_FILE_CANCEL_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_CancelCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_Cancel() length to long -- test passed (2/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Cancel() length to long -- test failed (2/4)\n");
        FailCount++;
    }

    /* (3) no file transfer in progress  NOTE: No errors here. */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_CancelCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_CancelCmd), FDL_APP_FILE_CANCEL_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_CancelCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) && ( FDL_HkTelemetryPkt.TransferInProgress == FALSE ) )
    {
        UTF_put_text("FDL_Cancel() no file transfer in progress -- test passed (3/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Cancel() no file transfer in progress -- test failed (3/4)\n");
        FailCount++;
    }

    /* (4) Valid packet length */
    StartCommand();
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_CancelCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_CancelCmd), FDL_APP_FILE_CANCEL_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_CancelCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) && ( FDL_HkTelemetryPkt.TransferInProgress == FALSE ) )
    {
        UTF_put_text("FDL_Cancel() file transfer in progress -- test passed (4/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Cancel() file transfer in progress -- test failed (4/4)\n");
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function FDL pause ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  no file transfer in progress
    **   (4)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFDL_Pause() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_PauseCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_PauseCmd), FDL_APP_FILE_PAUSE_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_PauseCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_Pause() length to short -- test passed (1/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Pause() length to short -- test failed (1/4)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_PauseCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_PauseCmd), FDL_APP_FILE_PAUSE_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_PauseCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_Pause() length to long -- test passed (2/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Pause() length to long -- test failed (2/4)\n");
        FailCount++;
    }

    /* (3) no file transfer in progress */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_PauseCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_PauseCmd), FDL_APP_FILE_PAUSE_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_PauseCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_Pause() no file transfer in progress -- test passed (3/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Pause() no file transfer in progress -- test failed (3/4)\n");
        FailCount++;
    }

    /* (4) Valid packet length */
    StartCommand();
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_PauseCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_PauseCmd), FDL_APP_FILE_PAUSE_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_PauseCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) && ( FDL_HkTelemetryPkt.TransferPaused == TRUE ) )
    {
        UTF_put_text("FDL_Pause() file transfer in progress -- test passed (4/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Pause() file transfer in progress -- test failed (4/4)\n");
        FailCount++;
    }
    CancelCommand();
    HkRequest();

    /*
    ** Tests for function FDL resume ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  no file transfer in progress
    **   (4)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFDL_Resume() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_ResumeCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResumeCmd), FDL_APP_FILE_RESUME_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResumeCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_Resume() length to short -- test passed (1/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Resume() length to short -- test failed (1/4)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_ResumeCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResumeCmd), FDL_APP_FILE_RESUME_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResumeCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_Resume() length to long -- test passed (2/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Resume() length to long -- test failed (2/4)\n");
        FailCount++;
    }

    /* (3) no file transfer in progress */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_ResumeCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResumeCmd), FDL_APP_FILE_RESUME_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResumeCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_Resume() no file transfer in progress -- test passed (3/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Resume() no file transfer in progress -- test failed (3/4)\n");
        FailCount++;
    }

    /* (4) Valid packet length */
    StartCommand();
    PauseCommand();
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_ResumeCmd, FDL_APP_CMD_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResumeCmd), FDL_APP_FILE_RESUME_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_ResumeCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) && ( FDL_HkTelemetryPkt.TransferPaused == FALSE ) )
    {
        UTF_put_text("FDL_Resume() file transfer in progress -- test passed (4/4)\n");
    }
    else
    {
        UTF_put_text("FDL_Resume() file transfer in progress -- test failed (4/4)\n");
        FailCount++;
    }
    CancelCommand();
    HkRequest();

    /*
    ** Tests for function FDL HK request ()...
    **
    **   (1)  good HK request packet (only validation is message ID and if wrong then not a HK request.
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFDL_HK() tests\n");

    /* (1) Valid HK request */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_HkRequest, FDL_APP_SEND_HK_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_HkRequest);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_HK Request() -- test passed (1/1)\n");
    }
    else
    {
        UTF_put_text("FDL_HK Request() -- test failed (1/1)\n");
        FailCount++;
    }

    /*
    ** Tests for function FDL wakeup request ()...
    **
    **   (1)  good wakeup request packet no file in progress (only validation is message ID and if wrong then not a wakeup request)
    **   (2)  good wakeup request packet file in progress.
    **   (3)  good wakeup request packet file in progress but paused.
    **   (4)  good wakeup request packet with semaphore timeout error.
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFDL_wakeup() tests\n");

    /* (1) Valid wakeup request packet no file in progress */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_HkRequest, FDL_APP_WAKEUP_MID, sizeof(FDL_NoArgsCmd_t), TRUE);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_HkRequest);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_wakeup Request() -- test passed (1/1)\n");
    }
    else
    {
        UTF_put_text("FDL_wakeup Request() -- test failed (1/1)\n");
        FailCount++;
    }

    /* TODO (3) good wakeup request packet file in progress but paused. */

    /* TODO (4) good wakeup request packet with semaphore timeout error. */
    HkRequest();

    /*
    ** Tests for function FDL start ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  file transfer in progress
    **   (4)  open failures (unknown file)
    **   (7)  invalid segment size.
    **   (8)  invalid file offset.
    **   (9)  bytes to skip greater than segement size.
    **   (10) good command packet
    ** 
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nFDL_Start() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() length to short -- test passed (1/10)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() length to short -- test failed (1/10)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() length to long -- test passed (2/10)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() length to long -- test failed (2/10)\n");
        FailCount++;
    }

    /* (3) file transfer in progress */
    StartCommand();
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), "/tt/small.out", OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 100;
    UT_StartCmd->Offset = 0;
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test passed (3/10)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test failed (3/10)\n");
        FailCount++;
    }
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();

    /* (4) open failures (unknown file). */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), "/tt/nofile", OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 1100;
    UT_StartCmd->Offset = 0;
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test passed (4/10)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test failed (4/10)\n");
        FailCount++;
    }
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();

    /* (7A) invalid segment size. */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), &(Testfile1[0]), OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 1100;
    UT_StartCmd->Offset = 0;
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test passed (7A/10)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test failed (7A/10)\n");
        FailCount++;
    }
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();

    /* (7B) invalid segment size. */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), &(Testfile1[0]), OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 6;
    UT_StartCmd->Offset = 0;
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test passed (7B/10)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test failed (7B/10)\n");
        FailCount++;
    }
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();

    /* (8) invalid file offset. */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), "/tt/small.out", OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 100;
    UT_StartCmd->Offset = 1250;
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test passed (8/10)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test failed (8/10)\n");
        FailCount++;
    }
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();

    /* (9) bytes to skip greater than segement size. */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), &(Testfile1[0]), OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 100;
    UT_StartCmd->Offset = 1250;
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test passed (9/10)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test failed (9/10)\n");
        FailCount++;
    }
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();

    /* (10) Good command paket */
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), &(Testfile1[0]), OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 80;
    UT_StartCmd->Offset = 0;
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test passed (10/10)\n");
    }
    else
    {
        UTF_put_text("FDL_CmdNoop() valid command -- test failed (10/10)\n");
        FailCount++;
    }
    WakupRequest();  /* send wakeup to finish file. */
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
    uint16 CmdAcceptedCounter;
    uint16 CmdRejectedCounter;


    /* app init with invalid EVS register, invalid create pipe, invalid subscribe to HK, invalid subscribe to cmds, invalid subscribe to wakeup, invalid get semaphore by name, */
    StartCommand();
    HkRequest();
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();

    UTF_CFE_ES_Set_Api_Return_Code(CFE_ES_REGISTERAPP_PROC, -1);
    FDL_AppMain();
    UTF_CFE_ES_Use_Default_Api_Return_Code(CFE_ES_REGISTERAPP_PROC);

    /* (1) CFE_EVS_Register error */
    UTF_CFE_EVS_Set_Api_Return_Code(CFE_EVS_REGISTER_PROC, -1);
    iResult = FDL_AppInit();
    UTF_CFE_EVS_Use_Default_Api_Return_Code(CFE_EVS_REGISTER_PROC);
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("FDL_AppInit() -- EVS Register Error test failed (1)\n");
        FailCount++;
    }

    /* (2) CFE_SB_CreatePipe error */
    UTF_CFE_SB_Set_Api_Return_Code(CFE_SB_CREATEPIPE_PROC, -1);
    iResult = FDL_AppInit();
    UTF_CFE_SB_Use_Default_Api_Return_Code(CFE_SB_CREATEPIPE_PROC);
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("FDL_AppInit() -- Create Pipe Error test failed (2)\n");
        FailCount++;
    }

    UTF_SB_set_function_hook(CFE_SB_SUBSCRIBE_HOOK, (void *)&CFE_SB_SubscribeHook);

    /* (3) CFE_SB_Subscribe error - HK request commands */
    iResult = FDL_AppInit();
    TestCount++;
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("FDL_AppInit() -- Subscribe HK Request Error test failed (3)\n");
        FailCount++;
    }

    /* (4) CFE_SB_Subscribe error - FDL ground commands */
    iResult = FDL_AppInit();
    TestCount++;
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("FDL_AppInit() -- Subscribe Ground Command Error test failed (4)\n");
        FailCount++;
    }

    /* (5) CFE_SB_Subscribe error - FDL wakeup commands */
    iResult = FDL_AppInit();
    TestCount++;
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("FDL_AppInit() -- Subscribe Wakeup Request Error test failed (5)\n");
        FailCount++;
    }

    UTF_CFE_OSAPI_Set_Api_Return_Code ( CFE_OSAPI_COUNTSEMGETIDBYNAME_PROC, -1 );
    iResult = FDL_AppInit();
    UTF_CFE_OSAPI_Use_Default_Api_Return_Code ( CFE_OSAPI_COUNTSEMGETIDBYNAME_PROC );
    TestCount++;
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("FDL_AppInit() -- Subscribe Wakeup Request Error test failed (6)\n");
        FailCount++;
    }

    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    UTF_OSFILEAPI_Set_Api_Return_Code ( OS_STAT_PROC, -1 );
    FDL_AppInit();
    HkRequest();
    StartCommandNoErrorCheck ();
    HkRequest();
    UTF_OSFILEAPI_Use_Default_Api_Return_Code ( OS_STAT_PROC );
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        ;
    }
    else
    {
        UTF_put_text("FDL State Error  -- test failed (7)\n");
        FailCount++;
    }

    FDL_AppInit();
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    UTF_CFE_OSAPI_Set_Api_Return_Code ( CFE_OSAPI_COUNTSEMTIMEDWAIT_PROC, -1 );
    HkRequest();
    StartCommandNoErrorCheck ();
    HkRequest();
    WakupRequest();  /* send wakeup to finish file. */
    UTF_CFE_OSAPI_Use_Default_Api_Return_Code ( CFE_OSAPI_COUNTSEMTIMEDWAIT_PROC );
    HkRequest();
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        ;
    }
    else
    {
        UTF_put_text("FDL Stat Error  -- test failed (7)\n" );
        FailCount++;
    }

    UTF_put_text("\n\n\n\n\n\n\n\n\n\n\n  Starting offset read error test\n");

    FDL_AppInit();
    CmdAcceptedCounter = FDL_HkTelemetryPkt.fdl_command_count;
    CmdRejectedCounter = FDL_HkTelemetryPkt.fdl_command_error_count;
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), "/tt/testfile\0", OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 100;
    UT_StartCmd->Offset = 36;
    UTF_OSFILEAPI_Set_Api_Return_Code ( OS_READ_PROC, -1 );
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    UTF_OSFILEAPI_Use_Default_Api_Return_Code ( OS_READ_PROC );
    HkRequest();
    TestCount++;
    if ( ( CmdAcceptedCounter == FDL_HkTelemetryPkt.fdl_command_count ) && ( CmdRejectedCounter + 1 == FDL_HkTelemetryPkt.fdl_command_error_count ) )
    {
        ;
    }
    else
    {
        UTF_put_text("FDL offset read error -- test failed (8)\n");
        FailCount++;
    }
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();

    UTF_put_text("\n\n\n\n\n\n\n\n\n\n\n Starting wakeup read error test\n");

    FDL_AppInit();
    CFE_SB_InitMsg(UT_StartCmd, FDL_APP_CMD_MID, sizeof(FDL_FileStartCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_StartCmd), FDL_APP_FILE_START_CC);
    strncpy ( &(UT_StartCmd->FileName[0]), "/tt/testfile\0", OS_MAX_PATH_LEN );
    UT_StartCmd->SegmentSize = 100;
    UT_StartCmd->Offset = 0;
    FDL_ProcessCommandPacket((CFE_SB_MsgPtr_t) UT_StartCmd);
    if ( ( FDL_HkTelemetryPkt.TransferInProgress == TRUE ) )
    {
        ;
    }
    else
    {
        UTF_put_text("FDL offset read error -- test failed (8)\n");
        FailCount++;
    }
    UTF_OSFILEAPI_Set_Api_Return_Code ( OS_READ_PROC, -1 );
    WakupRequest();  /* send wakeup to finish file. */
    UTF_OSFILEAPI_Use_Default_Api_Return_Code ( OS_READ_PROC );
    HkRequest();
    TestCount++;
    if ( ( FDL_HkTelemetryPkt.TransferInProgress == FALSE ) )
    {
        ;
    }
    else
    {
        UTF_put_text("FDL offset read error -- test failed (8)\n");
        FailCount++;
    }

    UTF_put_text("\n\n\n\n\n\n\n\n\n\n\n finished read error tests\n");


}


