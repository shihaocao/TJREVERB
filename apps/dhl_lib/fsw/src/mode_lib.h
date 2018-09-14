/*************************************************************************
** File:
**   $Id: mode_lib.h  $
**
** Purpose: 
**   Specification for the Mode functions.
**
** References:
**
**   $Log: mode_lib.h  $
**  
*************************************************************************/
#ifndef _mode_lib_h_
#define _mode_lib_h_

#include "cfe.h"

#define MODE_CHARGING 0
#define MODE_SAFE     1
#define MODE_DAGR     2
#define MODE_INMS     3

/*
** Data
*/
typedef struct
{
   uint32 SpacecraftMode;
   uint32 ACSConfig;
} SpacecraftMode_t;

extern SpacecraftMode_t SpacecraftMode;

/*
** Exported Functions 
*/
int32  MODE_LibInit(void);

uint32 MODE_GetSpacecraftMode(void);
uint32 MODE_GetACSConfig(void);
void   MODE_SetSpacecraftMode(uint32 NewMode);
void   MODE_SetACSConfig(uint32 NewConfig);

#endif /* _mode_lib_h_ */
