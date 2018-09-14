/*************************************************************************
** File:
**   $Id: l3c_lib.h  $
**
** Purpose: 
**   Specification for the L3 Cadet functions.
**
** References:
**
**   $Log: l3c_lib.h  $
**  
*************************************************************************/
#ifndef _l3c_lib_h_
#define _l3c_lib_h_

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

/************************************************************************
** Type Definitions
*************************************************************************/

typedef void (*L3C_callback_t) (uint8 * buf, int len, void * pxTaskWoken);

/*************************************************************************
** Exported Functions
*************************************************************************/
int32   L3C_LibInit(void);

int32   L3C_SendMessageToRadio(uint8 *MsgHdr,  uint32 HdrSize,
                               uint8 *Payload, uint32 PayloadSize);

int32   L3C_InstallCallback (L3C_callback_t callback);

int32   L3C_OpenSerialPort(void);

uint32  L3C_crc32(uint8 *block, unsigned int length);

/************************************************************************/

#endif /* _l3c_lib_h_ */

/************************/
/*  End of File Comment */
/************************/
