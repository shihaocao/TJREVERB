/*
** File: cadet_vcdu.h
**
** Purpose:
**   Define CADET vcdu messages, exported variables, and interface routines.
**
*/
#ifndef _cadet_vcdu_h_
#define _cadet_vcdu_h_



/*
**   Includes:
*/
#include "cfe.h"



/*
** Defines
*/
#define   CDH_BUFFER_VCDU_READY_TO_OUTPUT          ( 1 )

#define   VCDU_HI_FIFO                             ( 0 )    /* Must be 0 */
#define   VCDU_LO_FIFO                             ( 1 )    /* Must be 1 */
#define   NUMBER_VCDU_FIFOS                        ( 2 )



/*
** Structures
*/



/*
** Exported Variables
*/



/*
** Exported Functions
*/
extern void CADET_VCDU_InitailizeVcduBufferLibrary ( void );
extern int32 CADET_VCDU_AddPacketToVcdu ( CFE_SB_Msg_t *PktPtr, uint32 FIFO );
extern int32 CADET_VCDU_GetNextBufferToOutput ( uint32 FIFO, uint32* ToBeOutputIndex, uint8** DataPtr, uint32* DataSize );
extern void CADET_VCDU_FreeReadIndex ( uint32 FIFO, uint32 ToBeFreedIndex );
extern uint32 CADET_VcduTotalFreeSpace ( uint32 FIFO );
extern uint32 CADET_VcduTotalSize ( uint32 FIFO );



#endif /* _cadet_vcdu_h_ */



/************************/
/*  End of File Comment */
/************************/
