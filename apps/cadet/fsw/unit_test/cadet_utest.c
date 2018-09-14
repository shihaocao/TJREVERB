/************************************************************************
** File:
**   $Id: cadet_utest.c $
**
** Purpose: 
**   This is a test driver used to unit test the CADET application.
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

#include "cadet_perfids.h"
#include "cadet_msgids.h"

#include "cadet_msg.h"
#include "cadet_app.h"
#include "cadet_hk.h"
#include "cadet_cds.h"
#include "cadet_fltrtbl.h"
#include "cadet_input.h"
#include "cadet_output.h"
#include "cadet_radio.h"
#include "cadet_vcdu.h"
#include "cadet_events.h"
#include "cadet_version.h"

#include "cfe_es_cds.h"        /* cFE headers         */

#include <stdlib.h>            /* System headers      */

/************************************************************************
** Macro Definitions
*************************************************************************/
#define MESSAGE_FORMAT_IS_CCSDS

#define CADET_CMD_SWPIPE           1
#define CADET_FIFO_SWPIPE          2
#define CADET_RADIO_SWPIPE         3
#define CADET_WAKEUP_SWPIPE        4


/************************************************************************
** CADET global data external to this file
*************************************************************************/
extern void CADET_ProcessMainPipe ( CFE_SB_MsgPtr_t MsgPtr );


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

CADET_NoArgsCmd_t      *UT_HkRequest =      (CADET_NoArgsCmd_t *)       &UT_CommandPkt[0];
CADET_NoArgsCmd_t      *UT_WakeupRequest =  (CADET_NoArgsCmd_t *)       &UT_CommandPkt[0];

CADET_NoArgsCmd_t                        *UT_NoopCmd =                          (CADET_NoArgsCmd_t *)                        &UT_CommandPkt[0];
CADET_NoArgsCmd_t                        *UT_ResetCmd =                         (CADET_NoArgsCmd_t *)                        &UT_CommandPkt[0];
CADET_NoArgsCmd_t                        *UT_SemTakeCmd =                       (CADET_NoArgsCmd_t *)                        &UT_CommandPkt[0];
CADET_NoArgsCmd_t                        *UT_SemGiveCmd =                       (CADET_NoArgsCmd_t *)                        &UT_CommandPkt[0];
CADET_JamFilterCmd_t                     *UT_JamFilterCmd =                     (CADET_JamFilterCmd_t *)                     &UT_CommandPkt[0];
CADET_StartPlaybackCmd_t                 *UT_StartPlaybackCmd =                 (CADET_StartPlaybackCmd_t *)                 &UT_CommandPkt[0];
CADET_NoArgsCmd_t                        *UT_AbortPlaybackCmd =                 (CADET_NoArgsCmd_t *)                        &UT_CommandPkt[0];
CADET_FlushFifoCmd_t                     *UT_FlushFifoCmd =                     (CADET_FlushFifoCmd_t *)                     &UT_CommandPkt[0];
CADET_FlushEntireFifoHiCmd_t             *UT_FlushEntireFifoHiCmd =             (CADET_FlushEntireFifoHiCmd_t *)             &UT_CommandPkt[0];
CADET_PlaybackAndFlushHiFifoCmd_t        *UT_PlaybackAndFlushHiFifoCmd =        (CADET_PlaybackAndFlushHiFifoCmd_t *)        &UT_CommandPkt[0];
CADET_StartLoFifoWithHiFifoPlaybackCmd_t *UT_StartLoFifoWithHiFifoPlaybackCmd = (CADET_StartLoFifoWithHiFifoPlaybackCmd_t *) &UT_CommandPkt[0];
CADET_MaxiumuSizeToAutoFlushCmd_t        *UT_MaxiumuSizeToAutoFlushCmd =        (CADET_MaxiumuSizeToAutoFlushCmd_t *)        &UT_CommandPkt[0];
CADET_NumberPacketsToAutoFlushCmd_t      *UT_NumberPacketsToAutoFlushCmd =      (CADET_NumberPacketsToAutoFlushCmd_t *)      &UT_CommandPkt[0];
CADET_NumberPacketsToAutoPlaybackCmd_t   *UT_NumberPacketsToAutoPlaybackCmd =   (CADET_NumberPacketsToAutoPlaybackCmd_t *)   &UT_CommandPkt[0];


/************************************************************************
** Local function prototypes
*************************************************************************/

void PrintHKPacket (uint8 source, void *packet);
void Test_ground_cmds ( void );
void Test_app_init ( void );
void Test_app_output ( void );
void Test_app_input ( void );
void Test_app_table ( void );
int32 CFE_SB_CreatePipeHook(CFE_SB_PipeId_t *PipeIdPtr, uint16  Depth, char *PipeName);

void GetInputChar ( void )
{
    int c;

    printf ( "Enter to continue\n" );
    c = getchar();
    while (c != 'A') {
        putchar(c);
        c = getchar();
    }
    printf ( "\n" );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CADET unit test program main                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int main(void)
{
    /*
    ** Set up output results text file           
    */
    UTF_set_output_filename("cadet_utest.out");

    /*
    ** Set up HK packet handler           
    */
    UTF_set_packet_handler(CADET_APP_HK_TLM_MID, (utf_packet_handler)PrintHKPacket);
    
    /*******************************************************************/
    /* Add command pipes and associated script files                   */           
    /*******************************************************************/   
    UTF_add_input_file(CADET_FIFO_SWPIPE, "pipe.in");
    UTF_add_input_file(CADET_RADIO_SWPIPE, "radiopipe.in");
    UTF_add_input_file(CADET_CMD_SWPIPE, "cmdpipe.in");
    UTF_add_input_file(CADET_WAKEUP_SWPIPE, "wakeuppipe.in");

    /*
    ** Initialize time data structures
    */
    UTF_init_sim_time(0.0);

    /*
    ** Initialize ES application data                         
    */
    UTF_ES_InitAppRecords();
    UTF_ES_AddAppRecord("CADET",0);  
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
     * ... cadet/fsw/unit_test            <-- this is the current working directory
     * ... cadet/fsw/unit_test/disk/boot  <-- physical location for virtual disk "/boot"
     */
    UTF_add_volume("/boot", "disk", FS_BASED, FALSE, FALSE, TRUE, "YY", "/boot", 0);

    /*
    ** Required setup prior to calling many CFE API functions
    */
    CFE_ES_RegisterApp();
    CFE_EVS_Register(NULL, 0, 0);

    UTF_SB_set_function_hook(CFE_SB_CREATEPIPE_HOOK, (void *)&CFE_SB_CreatePipeHook);   

    /*
    ** Run CADET ground command unit tests
    */
    UTF_put_text("\n*** CADET -- Testing CADET commands ***\n");
    CFE_ES_CDS_EarlyInit(); /* Must wipe CDS prior to reiniting */
    CADET_AppInit();  /* need to init HK packet as it's used in the test */
    Test_ground_cmds();

    UTF_put_text("\n*** CADET -- Total test count = %d, total test errors = %d\n\n", UT_TotalTestCount, UT_TotalFailCount);

    /*
    ** Run CADET app init unit tests
    */
    UTF_put_text("\n*** CADET -- Testing CADET app init ***\n");
    CFE_ES_CDS_EarlyInit(); /* Must wipe CDS prior to reiniting */
    CADET_AppInit();
//    Test_app_init();

    UTF_put_text("\n*** CADET -- Total test count = %d, total test errors = %d\n\n", UT_TotalTestCount, UT_TotalFailCount);

    /*
    ** Run CADET app output/radio unit tests
    */
    UTF_put_text("\n*** CADET -- Testing CADET app output ***\n");
    CFE_ES_CDS_EarlyInit(); /* Must wipe CDS prior to reiniting */
    CADET_AppInit();
    Test_app_output();

    UTF_put_text("\n*** CADET -- Total test count = %d, total test errors = %d\n\n", UT_TotalTestCount, UT_TotalFailCount);

    /*
    ** Run CADET app input unit tests
    */
    UTF_put_text("\n*** CADET -- Testing CADET app input ***\n");
    CFE_ES_CDS_EarlyInit(); /* Must wipe CDS prior to reiniting */
    CADET_AppInit();
    Test_app_input();

    UTF_put_text("\n*** CADET -- Total test count = %d, total test errors = %d\n\n", UT_TotalTestCount, UT_TotalFailCount);

    /*
    ** Run CADET app table unit tests
    */
    UTF_put_text("\n*** CADET -- Testing CADET app table ***\n");
    CFE_ES_CDS_EarlyInit(); /* Must wipe CDS prior to reiniting */
    CADET_AppInit();
    Test_app_table();

    UTF_put_text("\n*** CADET -- Total test count = %d, total test errors = %d\n\n", UT_TotalTestCount, UT_TotalFailCount);

    /*
    ** Run CADET vcdu unit tests
    */
    UTF_put_text("\n*** CADET -- Testing CADET app vcdu ***\n");
    CFE_ES_CDS_EarlyInit(); /* Must wipe CDS prior to reiniting */
    CADET_AppInit();
//    Test_app_vcdu();

    UTF_put_text("\n*** CADET -- Total test count = %d, total test errors = %d\n\n", UT_TotalTestCount, UT_TotalFailCount);

    /*
    ** Invoke the main loop "success" test now because the program
    **  will end when the last entry in the SB input file is read.
    */
    CFE_ES_CDS_EarlyInit(); /* Must wipe CDS prior to reiniting */
    UTF_CFE_ES_Set_Api_Return_Code(CFE_ES_RUNLOOP_PROC, TRUE);
    CADET_AppMain();

    return 0;
   
} /* End of main() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Hook function for CFE_SB_CreatePipe that will set the correct   */
/* values for each pipe                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 CFE_SB_CreatePipeHook(CFE_SB_PipeId_t *PipeIdPtr, uint16  Depth, char *PipeName)
{
    static uint32 Count = 0;

    Count++;
    
    if (Count == 6)
    {
        Count = 1;
    }
    if (Count == 1)
    {
        *PipeIdPtr = CADET_CMD_SWPIPE;
    }
    if (Count == 2)
    {
        *PipeIdPtr = CADET_FIFO_SWPIPE;
    }
    if (Count == 3)
    {
        *PipeIdPtr = CADET_RADIO_SWPIPE;
    }
    if (Count == 4)
    {
        *PipeIdPtr = CADET_WAKEUP_SWPIPE;
    }
    return(CFE_SUCCESS);
      
}/* end CFE_SB_CreatePipeHook */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Prints out the current values in the CADET housekeeping packet     */
/* data structure                                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void PrintHKPacket (uint8 source, void *packet)
{
    CADET_HkTlmPkt_t *HkPacket = (CADET_HkTlmPkt_t *) packet;
    
    /* Output the CADET housekeeping data */
    UTF_put_text("\nCADET HOUSEKEEPING DATA:");

    UTF_put_text("CC = %d, EC = %d, TIP = %d, TP = %d, [%d] [%d] [%d] [%d] [%d] [%d] [%d] [] \n", HkPacket->command_count, HkPacket->command_error_count );
   
} /* End of PrintHKPacket() */

void HkRequest ( void )
{
    CFE_SB_InitMsg(UT_HkRequest, CADET_APP_SEND_HK_MID, sizeof(CADET_NoArgsCmd_t), TRUE);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_HkRequest);
}

void WakupRequest ( void )
{
    /* 
    ** Send a valid wakeup in the process of testing somethinge else. 
    */
    CFE_SB_InitMsg(UT_HkRequest, CADET_APP_WAKEUP_MID, sizeof(CADET_NoArgsCmd_t), TRUE);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_HkRequest);
}

void Test_ground_cmds ( void )
{
    uint16 CmdAcceptedCounter;
    uint16 CmdRejectedCounter;

    uint32 TestCount = 0;
    uint32 FailCount = 0;
 
    /*
    ** Tests for function CADET Noop ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nCADET_CmdNoop() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), CADET_APP_NOOP_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_CmdNoop() length to short -- test passed (1/3)\n");
    }
    else
    {
        UTF_put_text("CADET_CmdNoop() length to short -- test failed (1/3)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), CADET_APP_NOOP_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_CmdNoop() length to long -- test passed (2/3)\n");
    }
    else
    {
        UTF_put_text("CADET_CmdNoop() length to long -- test failed (2/3)\n");
        FailCount++;
    }

    /* (3) Valid packet length */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), CADET_APP_NOOP_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_CmdNoop() valid command -- test passed (3/3)\n");
    }
    else
    {
        UTF_put_text("CADET_CmdNoop() valid command -- test failed (3/3)\n");
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function CADET Reset Counters ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nCADET_Reset Counters() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_ResetCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResetCmd), CADET_APP_RESET_COUNTERS_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_ResetCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_ResetCmd() length to short -- test passed (1/3)\n");
    }
    else
    {
        UTF_put_text("CADET_ResetCmd() length to short -- test failed (1/3)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_ResetCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResetCmd), CADET_APP_RESET_COUNTERS_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_ResetCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_ResetCmd() length to long -- test passesd (2/3)\n");
    }
    else
    {
        UTF_put_text("CADET_ResetCmd() length to long -- test failed (2/3)\n");
        FailCount++;
    }

    /* (3) Valid packet length */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_ResetCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_ResetCmd), CADET_APP_RESET_COUNTERS_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_ResetCmd);
    TestCount++;
    if ( ( CADET_HkTelemetryPkt.command_count == 0 ) && ( CADET_HkTelemetryPkt.command_error_count == 0 ) )
    {
        UTF_put_text("CADET_ResetCmd() valid command -- test passed (3/3)\n");
    }
    else
    {
        UTF_put_text("CADET_ResetCmd() valid command -- test failed (3/3)\n");
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function CADET invalid command headers ()...
    **
    **   (1)  invalid message ID
    **   (2)  invalid command code
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nCADET_ Invalid command headers() tests\n");

    /* (1) invalid message ID */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, CADET_APP_CMD_MID-10, sizeof(CADET_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), CADET_APP_NOOP_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET invalid message ID -- test passed (1/2)\n");
    }
    else
    {
        UTF_put_text("CADET invalid message ID -- test failed (1/2)\n");
        FailCount++;
    }

    /* (2) invalid command code */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_NoopCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NoopCmd), CADET_APP_NOOP_CC+20);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_NoopCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET invalid command code -- test passed (2/2)\n");
    }
    else
    {
        UTF_put_text("CADET invalid command code -- test failed (2/2)\n");
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function CADET HK request ()...
    **
    **   (1)  good HK request packet (only validation is message ID and if wrong then not a HK request.
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nCADET_HK() tests\n");

    /* (1) Valid HK request */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_HkRequest, CADET_APP_SEND_HK_MID, sizeof(CADET_NoArgsCmd_t), TRUE);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_HkRequest);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_HK Request() -- test passed (1/1)\n");
    }
    else
    {
        UTF_put_text("CADET_HK Request() -- test failed (1/1)\n");
        FailCount++;
    }

    /*
    ** Tests for function CADET Semaphore Take ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nCADET_TakeSemaphore() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_SemTakeCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SemTakeCmd), CADET_APP_TAKE_DOWNLINK_SEM_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_SemTakeCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_SemTakeCmd() length to short -- test passed (1/3)\n");
    }
    else
    {
        UTF_put_text("CADET_SemTakeCmd() ERROR: length to short -- test failed (1/3)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_SemTakeCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SemTakeCmd), CADET_APP_TAKE_DOWNLINK_SEM_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_SemTakeCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_SemTakeCmd() length to long -- test passed (2/3)\n");
    }
    else
    {
        UTF_put_text("CADET_SemTakeCmd() ERROR: length to long -- test failed (2/3)\n");
        FailCount++;
    }

    /* (3) Valid packet length */
    HkRequest();
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
        UTF_put_text("CADET_SemTakeCmd() ERROR: valid command -- test failed (3/3) %d, %d, %d, %d, %d, %d\n",CmdAcceptedCounter + 1, 
                     CADET_HkTelemetryPkt.command_count, CmdRejectedCounter, CADET_HkTelemetryPkt.command_error_count, 
                     CADET_HkTelemetryPkt.FDLSempahoresInUse, CADET_FDL_HANDSHAKE_SEM_VALUE - 1);
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_SemTakeCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SemTakeCmd), CADET_APP_TAKE_DOWNLINK_SEM_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_SemTakeCmd);
    TestCount++;
    HkRequest();
    if ( ( CmdAcceptedCounter + 1 == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter == CADET_HkTelemetryPkt.command_error_count ) &&
         ( CADET_HkTelemetryPkt.FDLSempahoresInUse == ( CADET_FDL_HANDSHAKE_SEM_VALUE - 1 ) ) ) /* Decrements by 1 */
    {
        UTF_put_text("CADET_SemTakeCmd() valid command -- test passed (3/3)\n");
    }
    else
    {
        UTF_put_text("CADET_SemTakeCmd() ERROR: valid command -- test failed (3/3) %d, %d, %d, %d, %d, %d\n",CmdAcceptedCounter + 1, 
                     CADET_HkTelemetryPkt.command_count, CmdRejectedCounter, CADET_HkTelemetryPkt.command_error_count, 
                     CADET_HkTelemetryPkt.FDLSempahoresInUse, CADET_FDL_HANDSHAKE_SEM_VALUE - 1);
        FailCount++;
    }
    HkRequest();

    /*
    ** Tests for function CADET Semaphore Give ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  good command packet
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nCADET_GiveSemaphore() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_SemGiveCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SemGiveCmd), CADET_APP_GIVE_DOWNLINK_SEM_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_SemGiveCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_SemGiveCmd() length to short -- test passed (1/3)\n");
    }
    else
    {
        UTF_put_text("CADET_SemGiveCmd() ERROR: length to short -- test failed (1/3)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_SemGiveCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SemGiveCmd), CADET_APP_GIVE_DOWNLINK_SEM_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_SemGiveCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_SemGiveCmd() length to long -- test passed (2/3)\n");
    }
    else
    {
        UTF_put_text("CADET_SemGiveCmd() ERROR: length to long -- test failed (2/3)\n");
        FailCount++;
    }

    /* (3) Valid packet length */
    HkRequest();
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
        UTF_put_text("CADET_SemGiveCmd() ERROR: valid command -- test failed (3/3) %d, %d, %d, %d, %d, %d\n",CmdAcceptedCounter + 1, 
                     CADET_HkTelemetryPkt.command_count, CmdRejectedCounter, CADET_HkTelemetryPkt.command_error_count, 
                     CADET_HkTelemetryPkt.FDLSempahoresInUse, CADET_FDL_HANDSHAKE_SEM_VALUE);
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_SemGiveCmd, CADET_APP_CMD_MID, sizeof(CADET_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_SemGiveCmd), CADET_APP_GIVE_DOWNLINK_SEM_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_SemGiveCmd);
    TestCount++;
    HkRequest();
    if ( ( CmdAcceptedCounter + 1 == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter == CADET_HkTelemetryPkt.command_error_count ) &&
         ( CADET_HkTelemetryPkt.FDLSempahoresInUse == ( CADET_FDL_HANDSHAKE_SEM_VALUE ) ) )  /* Back up to original value */
    {
        UTF_put_text("CADET_SemGiveCmd() valid command -- test passed (3/3)\n");
    }
    else
    {
        UTF_put_text("CADET_SemGiveCmd() ERROR: valid command -- test failed (3/3) %d, %d, %d, %d, %d, %d\n",CmdAcceptedCounter + 1, 
                     CADET_HkTelemetryPkt.command_count, CmdRejectedCounter, CADET_HkTelemetryPkt.command_error_count, 
                     CADET_HkTelemetryPkt.FDLSempahoresInUse, CADET_FDL_HANDSHAKE_SEM_VALUE);
        FailCount++;
    }
    HkRequest();

// jam filter
// start playback
// abort playback
// flush fifo
// flush hi fifo
// playback and flush hi fifo
// playback lo and hi fifo
// max size auto flush short, long, 0, valid
// max pkts to auto flush short, long, 0, valid
// pkts to auto playback short, long, 0, valid
    /*
    ** Tests for function CADET Set Packets To Auto Playback ()...
    **
    **   (1)  invalid command packet length (too short)
    **   (2)  invalid command packet length (too long)
    **   (3)  good command packet
    **   (4)  invalid parameter (0)
    */
    UTF_put_text("\n\n\n\n\n\n\n\n\n\nCADET_SetPacketsToAutoPlayback() tests\n");

    /* (1) invalid packet length (too short) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_NumberPacketsToAutoPlaybackCmd, CADET_APP_CMD_MID, sizeof(CADET_NumberPacketsToAutoPlaybackCmd_t) - 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NumberPacketsToAutoPlaybackCmd), CADET_PACKETS_TO_AUTO_PLAYBACK_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_NumberPacketsToAutoPlaybackCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_NumberPacketsToAutoPlaybackCmd() length to short -- test passed (1/3)\n");
    }
    else
    {
        UTF_put_text("CADET_NumberPacketsToAutoPlaybackCmd() ERROR: length to short -- test failed (1/3)\n");
        FailCount++;
    }

    /* (2) invalid packet length (too long) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_NumberPacketsToAutoPlaybackCmd, CADET_APP_CMD_MID, sizeof(CADET_NumberPacketsToAutoPlaybackCmd_t) + 1, TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NumberPacketsToAutoPlaybackCmd), CADET_PACKETS_TO_AUTO_PLAYBACK_CC);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_NumberPacketsToAutoPlaybackCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        UTF_put_text("CADET_NumberPacketsToAutoPlaybackCmd() length to long -- test passed (2/3)\n");
    }
    else
    {
        UTF_put_text("CADET_NumberPacketsToAutoPlaybackCmd() ERROR: length to long -- test failed (2/3)\n");
        FailCount++;
    }

    /* (3) Valid packet length */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_NumberPacketsToAutoPlaybackCmd, CADET_APP_CMD_MID, sizeof(CADET_NumberPacketsToAutoPlaybackCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NumberPacketsToAutoPlaybackCmd), CADET_PACKETS_TO_AUTO_PLAYBACK_CC);
    UT_NumberPacketsToAutoPlaybackCmd->NumPackets = 250;
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_NumberPacketsToAutoPlaybackCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter == CADET_HkTelemetryPkt.command_error_count ) &&
         ( CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback == 250 ) )
    {
        UTF_put_text("CADET_NumberPacketsToAutoPlaybackCmd() valid command -- test passed (3/3)\n");
    }
    else
    {
        UTF_put_text("CADET_NumberPacketsToAutoPlaybackCmd() ERROR: valid command -- test failed (3/3)\n");
        FailCount++;
    }

    /* (4) invalid parameter (0) */
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    CFE_SB_InitMsg(UT_NumberPacketsToAutoPlaybackCmd, CADET_APP_CMD_MID, sizeof(CADET_NumberPacketsToAutoPlaybackCmd_t), TRUE);
    CFE_SB_SetCmdCode(((CFE_SB_MsgPtr_t) UT_NumberPacketsToAutoPlaybackCmd), CADET_PACKETS_TO_AUTO_PLAYBACK_CC);
    UT_NumberPacketsToAutoPlaybackCmd->NumPackets = 0;
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_NumberPacketsToAutoPlaybackCmd);
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) &&
         ( CADET_HkTelemetryPkt.HighFifoPacketsToAutoPlayback == 250 ) ) /* No change to packets to auto playback */
    {
        UTF_put_text("CADET_NumberPacketsToAutoPlaybackCmd() valid command -- test passed (3/3)\n");
    }
    else
    {
        UTF_put_text("CADET_NumberPacketsToAutoPlaybackCmd() ERROR: valid command -- test failed (3/3)\n");
        FailCount++;
    }
    HkRequest();


}

//    /*
//    ** Tests for function CADET wakeup request ()...
//    **
//    **   (1)  good wakeup request packet 
//    */
//    UTF_put_text("\n\n\n\n\n\n\n\n\n\nCADET_wakeup() tests\n");
//
//    /* (1) Valid wakeup request packet no file in progress */
//    UTF_CFE_SB_Set_Api_Return_Code ( CFE_SB_RCVMSG_PROC, -1 );
//    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
//    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
//    CFE_SB_InitMsg(UT_HkRequest, CADET_APP_WAKEUP_MID, sizeof(CADET_NoArgsCmd_t), TRUE);
//    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_HkRequest);
//    TestCount++;
//    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter == CADET_HkTelemetryPkt.command_error_count ) )
//    {
//        UTF_put_text("CADET_wakeup Request() -- test passed (1/1)\n");
//    }
//    else
//    {
//        UTF_put_text("CADET_wakeup Request() -- test failed (1/1)\n");
//        FailCount++;
//    }
//    UTF_CFE_SB_Set_Api_Return_Code ( CFE_SB_RCVMSG_PROC, UTF_CFE_USE_DEFAULT_RETURN_CODE );


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
    HkRequest();
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();

    UTF_CFE_ES_Set_Api_Return_Code(CFE_ES_REGISTERAPP_PROC, -1);
    CADET_AppMain();
    UTF_CFE_ES_Use_Default_Api_Return_Code(CFE_ES_REGISTERAPP_PROC);

    /* (1) CFE_EVS_Register error */
    UTF_CFE_EVS_Set_Api_Return_Code(CFE_EVS_REGISTER_PROC, -1);
    iResult = CADET_AppInit();
    UTF_CFE_EVS_Use_Default_Api_Return_Code(CFE_EVS_REGISTER_PROC);
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("CADET_AppInit() -- EVS Register Error test failed (1)\n");
        FailCount++;
    }

    /* (2) CFE_SB_CreatePipe error */
    UTF_CFE_SB_Set_Api_Return_Code(CFE_SB_CREATEPIPE_PROC, -1);
    iResult = CADET_AppInit();
    UTF_CFE_SB_Use_Default_Api_Return_Code(CFE_SB_CREATEPIPE_PROC);
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("CADET_AppInit() -- Create Pipe Error test failed (2)\n");
        FailCount++;
    }

    UTF_SB_set_function_hook(CFE_SB_SUBSCRIBE_HOOK, (void *)&CFE_SB_SubscribeHook);

    /* (3) CFE_SB_Subscribe error - HK request commands */
    iResult = CADET_AppInit();
    TestCount++;
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("CADET_AppInit() -- Subscribe HK Request Error test failed (3)\n");
        FailCount++;
    }

    /* (4) CFE_SB_Subscribe error - CADET ground commands */
    iResult = CADET_AppInit();
    TestCount++;
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("CADET_AppInit() -- Subscribe Ground Command Error test failed (4)\n");
        FailCount++;
    }

    /* (5) CFE_SB_Subscribe error - CADET wakeup commands */
    iResult = CADET_AppInit();
    TestCount++;
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("CADET_AppInit() -- Subscribe Wakeup Request Error test failed (5)\n");
        FailCount++;
    }

    UTF_CFE_OSAPI_Set_Api_Return_Code ( CFE_OSAPI_COUNTSEMGETIDBYNAME_PROC, -1 );
    iResult = CADET_AppInit();
    UTF_CFE_OSAPI_Use_Default_Api_Return_Code ( CFE_OSAPI_COUNTSEMGETIDBYNAME_PROC );
    TestCount++;
    if (iResult == CFE_SUCCESS)
    {
        UTF_put_text("CADET_AppInit() -- Subscribe Wakeup Request Error test failed (6)\n");
        FailCount++;
    }

    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    UTF_OSFILEAPI_Set_Api_Return_Code ( OS_STAT_PROC, -1 );
    CADET_AppInit();
    HkRequest();
    HkRequest();
    UTF_OSFILEAPI_Use_Default_Api_Return_Code ( OS_STAT_PROC );
    TestCount++;
    if ( ( CmdAcceptedCounter == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter + 1 == CADET_HkTelemetryPkt.command_error_count ) )
    {
        ;
    }
    else
    {
        UTF_put_text("CADET State Error  -- test failed (7)\n");
        FailCount++;
    }

    CADET_AppInit();
    CmdAcceptedCounter = CADET_HkTelemetryPkt.command_count;
    CmdRejectedCounter = CADET_HkTelemetryPkt.command_error_count;
    UTF_CFE_OSAPI_Set_Api_Return_Code ( CFE_OSAPI_COUNTSEMTIMEDWAIT_PROC, -1 );
    HkRequest();
    HkRequest();
    WakupRequest();  /* send wakeup to finish file. */
    UTF_CFE_OSAPI_Use_Default_Api_Return_Code ( CFE_OSAPI_COUNTSEMTIMEDWAIT_PROC );
    HkRequest();
    WakupRequest();  /* send wakeup to finish file. */
    HkRequest();
    TestCount++;
    if ( ( CmdAcceptedCounter + 1 == CADET_HkTelemetryPkt.command_count ) && ( CmdRejectedCounter == CADET_HkTelemetryPkt.command_error_count ) )
    {
        ;
    }
    else
    {
        UTF_put_text("CADET Stat Error  -- test failed (7)\n" );
        FailCount++;
    }


}

void Test_app_output ( void )
{
//    int32  iResult;
//    uint32 TestCount = 0;
//    uint32 FailCount = 0;
//    uint16 CmdAcceptedCounter;
//    uint16 CmdRejectedCounter;

    CFE_SB_InitMsg(UT_HkRequest, CADET_APP_WAKEUP_MID, sizeof(CADET_NoArgsCmd_t), TRUE);
    CADET_ProcessMainPipe((CFE_SB_MsgPtr_t) UT_HkRequest);

}

extern void CADET_UsartCallback ( uint8 *buf, int len, void *pxTaskWoken);
uint8 RawBuffer [128];
int   Length;

void Test_app_input ( void )
{
}


void Test_app_table ( void )
{
}
