/*
** File: cadet_vcdu.c
**
** Purpose:
**     This file contains the source code for the vcdu routines, initialization routine, and 
**     interface functions.
**
*/



/*
**   Includes:
*/
#include "cfe.h"
#include "cadet_vcdu.h"



/*
** Defines
*/
#define   CDH_BUFFER_SYNC_MARK                     ( 0x1ACFFC1D )
#define   CDH_BUFFER_CCSDS_TLM_VERSION_NUM         ( 1 )
#define   CDH_BUFFER_NO_PACKET_HEADER              ( 0x07FF )  /* First Header pointer value when a packet spans across a frame 0x07FF */
#define   CDH_BUFFER_RT_FRAME_DATA_SIZE            ( 1188 )   /* bytes */
#define   DELLINGR_SPACECRAFT_ID                   ( 0x7A )   // b 011110 10
#define   DELLINGR_SPACECRAFT_ID_UPPER             ( 0x1E )
#define   DELLINGR_SPACECRAFT_ID_LOWER             ( 0x02 )

#define   CDH_BUFFER_HI_BUFFER_COUNT               ( 8 )
#define   CDH_BUFFER_LO_BUFFER_COUNT               ( 24 )



/*
** Structures
*/
/* AOS Frame Type definitions */
__extension__ typedef struct
{
     uint16      SpacecraftID1    :6;
     uint16      VersionNumber    :2;

     uint16      VirtualChannelID :6;  
     uint16      SpacecraftID2    :2;

     uint16      FrameCounterMSW; 
     
     uint16      FrameCounterLSB  :8;  

     uint16      ReplayFlagRT     :1;
     uint16      TimeCorrBit      :1;
     uint16      Reserved         :6;

} CDH_BUFFER_FramePrimaryHeader_t ;

__extension__ typedef struct                      
{
     uint16      FirstHeaderPointer ;

} CDH_BUFFER_FrameDataFieldHeader_t;

/* Real-Time AOS Frame structure */
typedef struct
{
    uint32                             SyncMarker;
    CDH_BUFFER_FramePrimaryHeader_t    Header ;
    CDH_BUFFER_FrameDataFieldHeader_t  PDUHeader ;
    uint8                              PacketData[CDH_BUFFER_RT_FRAME_DATA_SIZE]; 
   
}  CDH_BUFFER_RT_TransferFrameMsg_t;            

typedef struct
{
    uint32 WriteIndex;
    uint32 ReadIndex;
    uint32 WritePtr;
    uint32 NextFrameCounter;

}  CDH_BUFFER_RT_FrameIndexes_t;



/*
** Exported Variables
*/



/*
** Local Variables (could be defined static)
*/
/* NOTE: Not two dimensional array to allow Hi/Lo counts to be different. */
CDH_BUFFER_RT_TransferFrameMsg_t  CDH_BUFFER_HI_VCDU [CDH_BUFFER_HI_BUFFER_COUNT];
CDH_BUFFER_RT_TransferFrameMsg_t  CDH_BUFFER_LO_VCDU [CDH_BUFFER_LO_BUFFER_COUNT];
CDH_BUFFER_RT_FrameIndexes_t      CDH_BUFFER_HI_Indexes;
CDH_BUFFER_RT_FrameIndexes_t      CDH_BUFFER_LO_Indexes;



/*
** Exported Functions
*/



/*
** Local Functions (could be defined static)
*/



/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/



/*
** Function: CADET_InitializeVcduBuffer
**
** Purpose: Initialize a single VCDU (including header).
**
*/
void CADET_InitializeVcduBuffer ( CDH_BUFFER_RT_TransferFrameMsg_t *Buffer, uint32 FrameCounter, uint32 VC, uint16 FFP )
{
    uint32 SyncHigh, SyncMidHigh, SyncMidLow, SyncLow;
    uint16 FrameCounterHigh, FrameCounterMed, FrameCounterLow;
    uint16 FFPHigh, FFPLow;

    SyncHigh =    ( CDH_BUFFER_SYNC_MARK >> 24 ) & 0xFF;
    SyncMidHigh = ( CDH_BUFFER_SYNC_MARK >> 16 ) & 0xFF;
    SyncMidLow =  ( CDH_BUFFER_SYNC_MARK >>  8 ) & 0xFF;
    SyncLow =     ( CDH_BUFFER_SYNC_MARK >>  0 ) & 0xFF;

    FrameCounterHigh = ( FrameCounter >> 16 ) & 0xFF;
    FrameCounterMed  = ( FrameCounter >>  8 ) & 0xFF;
    FrameCounterLow  = ( FrameCounter >>  0 ) & 0xFF;

    FFPHigh = ( FFP >> 8 ) & 0xFF;
    FFPLow  = ( FFP >> 0 ) & 0xFF;    

    Buffer->SyncMarker                   = ( SyncLow << 24 ) | ( SyncMidLow << 16 ) | ( SyncMidHigh << 8 ) | ( SyncHigh );

    Buffer->Header.VersionNumber         = 1;
    Buffer->Header.SpacecraftID1         = DELLINGR_SPACECRAFT_ID_UPPER;
    Buffer->Header.SpacecraftID2         = DELLINGR_SPACECRAFT_ID_LOWER;
    Buffer->Header.VirtualChannelID      = VC;
    Buffer->Header.FrameCounterMSW       = ( FrameCounterMed << 8 ) | ( FrameCounterHigh );
    Buffer->Header.FrameCounterLSB       = FrameCounterLow;
    Buffer->Header.ReplayFlagRT          = 0;
    Buffer->Header.TimeCorrBit           = 0;
    Buffer->Header.Reserved              = 0;

    Buffer->PDUHeader.FirstHeaderPointer = ( FFPLow << 8 ) | ( FFPHigh );

}



/*
** Function: CADET_VCDU_InitailizeVcduBufferLibrary
**
** Purpose: Initialize entire VCDU library setting all variables and pointers and indexes.
**
*/
void CADET_VCDU_InitailizeVcduBufferLibrary ( void )
{
    CDH_BUFFER_HI_Indexes.WriteIndex = 0;
    CDH_BUFFER_HI_Indexes.ReadIndex = 0;
    CDH_BUFFER_HI_Indexes.WritePtr = 0;
    CDH_BUFFER_HI_Indexes.NextFrameCounter = 0;
    CADET_InitializeVcduBuffer ( &CDH_BUFFER_HI_VCDU [ CDH_BUFFER_HI_Indexes.WriteIndex ], CDH_BUFFER_HI_Indexes.NextFrameCounter, VCDU_HI_FIFO, (uint16) 0 );
    CDH_BUFFER_HI_Indexes.NextFrameCounter++;

    CDH_BUFFER_LO_Indexes.WriteIndex = 0;
    CDH_BUFFER_LO_Indexes.ReadIndex = 0;
    CDH_BUFFER_LO_Indexes.WritePtr = 0;
    CDH_BUFFER_LO_Indexes.NextFrameCounter = 0;
    CADET_InitializeVcduBuffer ( &CDH_BUFFER_LO_VCDU [ CDH_BUFFER_LO_Indexes.WriteIndex ], CDH_BUFFER_LO_Indexes.NextFrameCounter, VCDU_LO_FIFO, (uint16) 0 );
    CDH_BUFFER_LO_Indexes.NextFrameCounter++;
}



/*
** Function: CADET_VcduFreeSpace
**
** Purpose: Determine free space in VCDU buffers.
**
*/
uint32 CADET_VcduFreeSpace ( CDH_BUFFER_RT_FrameIndexes_t *Index, uint32 TotalBufferCount )
{
    uint32 TotalFreeSpace;
    uint32 FreeSpaceCurrentVcdu;
    uint32 FreeVcdus;
    uint32 UsedVcdus;

    // Determine free space in current working VCDU
    // Determine free space in unused VCDUs
    //     Write index >= read index then no rollover
    //     Write index < read index then rollover
    // Add together

    FreeSpaceCurrentVcdu = CDH_BUFFER_RT_FRAME_DATA_SIZE - Index->WritePtr;

    if ( Index->WriteIndex >= Index->ReadIndex )
    {
        UsedVcdus = Index->WriteIndex - Index->ReadIndex + 1;
    }
    else
    {
        UsedVcdus = TotalBufferCount + Index->WriteIndex - Index->ReadIndex + 1;
    }

    FreeVcdus = TotalBufferCount - UsedVcdus;
    TotalFreeSpace = ( CDH_BUFFER_RT_FRAME_DATA_SIZE * FreeVcdus ) + FreeSpaceCurrentVcdu;

    return ( TotalFreeSpace );
}



/*
** Function: CADET_VcduTotalFreeSpace
**
** Purpose: Determine free space in VCDU buffers.
**
*/
uint32 CADET_VcduTotalFreeSpace ( uint32 FIFO )
{
    if ( FIFO == VCDU_HI_FIFO )
    {
        return ( CADET_VcduFreeSpace ( &CDH_BUFFER_HI_Indexes, CDH_BUFFER_HI_BUFFER_COUNT ) );
    }
    else
    {
        return ( CADET_VcduFreeSpace ( &CDH_BUFFER_LO_Indexes, CDH_BUFFER_LO_BUFFER_COUNT ) );
    }
}



/*
** Function: CADET_VcduTotalSize
**
** Purpose: Determine size of VCDU buffers.
**
*/
uint32 CADET_VcduTotalSize ( uint32 FIFO )
{
    if ( FIFO == VCDU_HI_FIFO )
    {
        return ( (uint32) ( CDH_BUFFER_RT_FRAME_DATA_SIZE * CDH_BUFFER_HI_BUFFER_COUNT ) );
    }
    else
    {
        return ( (uint32) ( CDH_BUFFER_RT_FRAME_DATA_SIZE * CDH_BUFFER_LO_BUFFER_COUNT ) );
    }
}



/*
** Function: CADET_AdvanceAndInitializeBuffer
**
** Purpose: Current VCDU is full, advance to next one.
**
*/
/*
** I was unable to pass the TransferFrameMsg_t as a pointer to a parameter.
*/
CDH_BUFFER_RT_TransferFrameMsg_t* CADET_AdvanceAndInitializeBuffer ( uint32 FIFO, uint16 FFP )
{
    CDH_BUFFER_RT_FrameIndexes_t *Index;
    uint32 BufferCount;
    CDH_BUFFER_RT_TransferFrameMsg_t *NextBuffer;

    // Advance write index
    // Set pointer to buffer
    // Update index items.

    if ( FIFO == VCDU_HI_FIFO )
    {
        Index = &CDH_BUFFER_HI_Indexes;
        BufferCount = CDH_BUFFER_HI_BUFFER_COUNT;
        Index->WriteIndex = ( Index->WriteIndex + 1 ) % BufferCount;
        NextBuffer = &CDH_BUFFER_HI_VCDU [ Index->WriteIndex ];
    }
    else
    {
        Index = &CDH_BUFFER_LO_Indexes;
        BufferCount = CDH_BUFFER_LO_BUFFER_COUNT;
        Index->WriteIndex = ( Index->WriteIndex + 1 ) % BufferCount;
        NextBuffer = &CDH_BUFFER_LO_VCDU [ Index->WriteIndex ];
    }

    if ( Index->WriteIndex == Index->ReadIndex )
    {
        printf ( "\n\n\n\n ERROR: Write index advanced to match read index\n" );
    }

    CADET_InitializeVcduBuffer ( NextBuffer, Index->NextFrameCounter, FIFO, FFP );
    Index->NextFrameCounter++;
    Index->WritePtr = 0;

    return ( NextBuffer );

}



/*
** Function: CADET_AddPacketToBuffer
**
** Purpose: Add one packet to the hi OR lo VCDU (calling function determined space permits).
**
*/
void CADET_AddPacketToBuffer ( CFE_SB_Msg_t *PktPtr, uint32 FIFO )
{
    uint16 totalremainingsize;
    uint16 vcduremainingsize;
    CDH_BUFFER_RT_FrameIndexes_t *Index;
    CDH_BUFFER_RT_TransferFrameMsg_t *Buffer;
    uint8 *CurrentPktPtr;

    // ASSUMPTION: PACKET WILL FIT IN BUFFER, BUT MAY OVERFLOW INTO NEXT BUFFER OR BUFFERS.
    // ASSUMPTION: PACKET WILL NOT COMPLETELY FILL UP ALL BUFFERS.
    // DO NOT ASSUME PACKET WILL FINISH IN THE NEXT VCDU.

    // Adding packet will NOT fill current VCDU (copy into)
    // Adding packet will exactly fill current VCDU (copy into, initialize next VCDU, update indexes and set first packet pointer to 0)
    // Adding packet will overflow into next VCDU (copy partial, initialize next VCDU, update indexes, copy partial, set first packet pointer)

    if ( FIFO == VCDU_HI_FIFO )
    {
        Index = &CDH_BUFFER_HI_Indexes;
        Buffer = &CDH_BUFFER_HI_VCDU [ Index->WriteIndex ];
    }
    else
    {
        Index = &CDH_BUFFER_LO_Indexes;
        Buffer = &CDH_BUFFER_LO_VCDU [ Index->WriteIndex ];
    }

    totalremainingsize = CFE_SB_GetTotalMsgLength ( PktPtr );
    CurrentPktPtr = (uint8 *) PktPtr;
    while ( totalremainingsize > 0 )
    {
        vcduremainingsize = CDH_BUFFER_RT_FRAME_DATA_SIZE - Index->WritePtr;
        if ( vcduremainingsize <= totalremainingsize )
        {
            /* Might have a copy after this one, but might finish on even vcdu */
            CFE_PSP_MemCpy ( &Buffer->PacketData [ Index->WritePtr ], CurrentPktPtr, vcduremainingsize );
            CurrentPktPtr += vcduremainingsize;
            totalremainingsize -= vcduremainingsize; /* Could end up zero */

            if ( totalremainingsize >= CDH_BUFFER_RT_FRAME_DATA_SIZE )
            {
                Buffer = CADET_AdvanceAndInitializeBuffer ( FIFO, CDH_BUFFER_NO_PACKET_HEADER );  /* First Packet Pointer, no packet header in vcdu */
            }
            else
            {
                Buffer = CADET_AdvanceAndInitializeBuffer ( FIFO, totalremainingsize ); /* First Packet Pointer, could end up zero */
            }        
        }
        else
        {
            /* Last copy, will not fill vcdu */
            CFE_PSP_MemCpy ( &Buffer->PacketData [ Index->WritePtr ], CurrentPktPtr, totalremainingsize );
            Index->WritePtr += (uint32) totalremainingsize; /* Should not fill final VCDU */
            totalremainingsize = 0;
        }
    }
}



/*
** Function: CADET_VCDU_AddPacketToVcdu
**
** Purpose: Add one packet to the hi OR lo VCDU if space permits.
**
*/
int32 CADET_VCDU_AddPacketToVcdu ( CFE_SB_Msg_t *PktPtr, uint32 FIFO )
{
    int32 PacketDropped;
    uint16 size;
    uint32 BufferCount;
    CDH_BUFFER_RT_FrameIndexes_t *Index;
    uint32 TotalFreeSpace;

    // Determine total packet length
    // Determine total free space in buffer library (for high or low)
    // If packet will fit, add to buffer (overflowing into the next buffer if necessary)

    size = CFE_SB_GetTotalMsgLength(PktPtr);

    if ( FIFO == VCDU_HI_FIFO )
    {
        BufferCount = CDH_BUFFER_HI_BUFFER_COUNT;
        Index = &CDH_BUFFER_HI_Indexes;
    }
    else
    {
        BufferCount = CDH_BUFFER_LO_BUFFER_COUNT;
        Index = &CDH_BUFFER_LO_Indexes;
    }

    PacketDropped = TRUE;
    /* 
    ** Size must be strictly less than, do not want to exactly fill up last VCDU buffer.
    ** Write pointer can't advance to meet the read pointer.
    ** When current VCDU full then write pointer is advanced.
    ** Hence, if we're on the last VCDU we can't advance write pointer, so we can't fill VCDU.
    */
    TotalFreeSpace = CADET_VcduFreeSpace ( Index, BufferCount );
    if ( size < TotalFreeSpace )
    {
        PacketDropped = FALSE;
        CADET_AddPacketToBuffer ( PktPtr, FIFO );
    }

    return ( PacketDropped );
}



/*
** Function: CADET_VCDU_GetNextBufferToOutput
**
** Purpose: Determine if there is a full VCDU on the FIFO and return information to output.
**
*/
int32 CADET_VCDU_GetNextBufferToOutput ( uint32 FIFO, uint32* ToBeOutputIndex, uint8** DataPtr, uint32* DataSize )
{
    CDH_BUFFER_RT_FrameIndexes_t *Index;
    CDH_BUFFER_RT_TransferFrameMsg_t* Buffer;
    int32 Status = !CDH_BUFFER_VCDU_READY_TO_OUTPUT; /* Assume no buffer ready */

    (void) DataPtr; /* Had to do this to get rid of a warning */

    if ( FIFO == VCDU_HI_FIFO )
    {
        Index = &CDH_BUFFER_HI_Indexes;
        Buffer = &CDH_BUFFER_HI_VCDU [ Index->ReadIndex ];
    }
    else
    {
        Index = &CDH_BUFFER_LO_Indexes;
        Buffer = &CDH_BUFFER_LO_VCDU [ Index->ReadIndex ];
    }

    /*
    ** If the read/write indexes don't match then there is at least one VCDU that is full.
    */
    if ( Index->WriteIndex != Index->ReadIndex )
    {
        /*
        ** There is a buffer ready to go.
        */
        Status = CDH_BUFFER_VCDU_READY_TO_OUTPUT;
        *ToBeOutputIndex = Index->ReadIndex;
        *DataPtr = (uint8 *) Buffer;
        *DataSize = sizeof(CDH_BUFFER_RT_TransferFrameMsg_t); /* We always output full VCDUs */
    }

    return ( Status );
}



/*
** Function: CADET_VCDU_FreeReadIndex
**
** Purpose: Free the first read index for the specified FIFO.
**
*/
void CADET_VCDU_FreeReadIndex ( uint32 FIFO, uint32 ToBeFreedIndex )
{
    CDH_BUFFER_RT_FrameIndexes_t *Index;
    uint32 BufferCount;

    if ( FIFO == VCDU_HI_FIFO )
    {
        Index = &CDH_BUFFER_HI_Indexes;
        BufferCount = CDH_BUFFER_HI_BUFFER_COUNT;
    }
    else
    {
        Index = &CDH_BUFFER_LO_Indexes;
        BufferCount = CDH_BUFFER_LO_BUFFER_COUNT;
    }

    if ( Index->ReadIndex == ToBeFreedIndex )
    {
        Index->ReadIndex = ( Index->ReadIndex + 1 ) % BufferCount;
    }
    else
    {
            printf ( "\n\n\n\n\n\n\n\n\n\n\nERROR, Trying to free an read index that is not correct\n" );
    }
}



/************************/
/*  End of File Comment */
/************************/
