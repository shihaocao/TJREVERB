/*******************************************************************************
** File: inms_instrument.c
**
** Purpose:
**   This file contains the source code for the INMS App instrument interface
**    code.
**
*******************************************************************************/

#include <stdio.h>
#include <stdint.h>

/*
**   Include Files:
*/

#include "inms_app.h"
#include "inms_perfids.h"
#include "inms_msgids.h"

#include "dhl_lib.h"

extern inms_hk_tlm_t            INMS_HkTelemetryPkt;
extern inms_instrument_hk_tlm_t INMS_InstTelemetryPkt;
extern CFE_SB_MsgPtr_t          INMSMsgPtr;
extern uint32                   INMSQueueId;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  INMS_PollInstrument                                                */
/*                                                                            */
/*  Purpose:                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void INMS_PollInstrument(void)
{
    uint32     QueueStatus;
    uint32     SizeCopied;
    uint8      InstrumentCommand[INMS_CMDSIZE] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
    int32      retval;

    /*
    ** When polling the INMS instrument, check for a pending 
    ** command to clock out on the SPI bus
    */
    QueueStatus = OS_QueueGet (INMSQueueId, InstrumentCommand, INMS_CMDSIZE,
                                &SizeCopied, OS_CHECK);

    if (QueueStatus != OS_SUCCESS) {
       CFE_PSP_MemSet(InstrumentCommand, 0, INMS_CMDSIZE);
    }

    retval = INMS_LibTransferINMSData(InstrumentCommand,
                                (uint8 *)&(INMS_InstTelemetryPkt.inms_hk_data),
                                INMS_TELEMETRY_LENGTH);

    /*
    ** Check the status of the spi_dma_transfer
    ** Send data if present
    */ 
    if (1 == retval) {
        /*
        ** Check to see if the packet is present 
        */
        if (INMS_SYNC_BYTE == INMS_InstTelemetryPkt.inms_hk_data.HDR.sync)
        { 
            INMS_HkTelemetryPkt.inms_inst_tlm_packets++;
            CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &INMS_InstTelemetryPkt);
            CFE_SB_SendMsg((CFE_SB_Msg_t *) &INMS_InstTelemetryPkt);

            /*
            ** Invalidate the buffer for the next transfer
            */
            #ifdef INMS_TEST
            INMS_LibPrintINMSHeader(&(INMS_InstTelemetryPkt.inms_hk_data.HDR));
            #endif
            INMS_InstTelemetryPkt.inms_hk_data.HDR.sync = 0;
        }
    }

    return;

} /* End of INMS_PollInstrument() */

uint8 INMS_CalcTlmChecksum(inms_telemetry_t* data) {
    unsigned int i = 0;
    uint8 sum = 0;

    for (i = 0; i < sizeof(inms_telemetry_t) - 1; i++) {
        sum += ((char *)data)[i];
    }
    return ~sum;
}

uint8 INMS_CalcCmdChecksum(INMS_CMDInst_t* cmd) {
    unsigned int i = 0;
    uint8 sum = 0;

    for (i = 0; i < 8; i++) {
        sum += ((char *)&(cmd->opcode))[i];
    }
    return ~sum + 1;
}

// TODO: Testing only
void INMS_LibPrintINMSHeader (inms_msg_header_t *header)
{
    printf("\r\n----------------- HEADER -------------- \r\n");
    printf("Header sync %x\r\n", header->sync);
    printf("HK0 %x\r\n", header->t0);
    printf("HK1 %x\r\n", header->t1);
    printf("HK2 %x\r\n", header->t2);
    printf("HK3 %x\r\n", header->t3);
    printf("S0 %x\r\n",  header->s0);
    printf("S1 %x\r\n",  header->s1);
}
