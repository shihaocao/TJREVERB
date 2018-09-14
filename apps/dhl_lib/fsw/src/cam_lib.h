/*************************************************************************
** File:
**   $Id: cam_lib.h  $
**
** Purpose: 
**   Specification for the CAM ( Serial Camera ) subsystem functions.
**
** References:
**
**   $Log: cam_lib.h  $
**  
*************************************************************************/
#ifndef _cam_lib_h_
#define _cam_lib_h_

#include "cfe.h"

#define CAM_END_OF_FILE -2

int32 CAM_LibInit(void);

/************************************************************************/
/* Higher level camera functions */

void  CAM_PowerOnCamera(void);

void  CAM_PowerOffCamera(void);

int   CAM_InitCamera(void);

int   CAM_ResetCamera(void);

void  CAM_SetPictureSize(uint8 Size);

uint16 CAM_GetPictureSize(void);

int   CAM_SnapImage(void);

int   CAM_GetCapturedImage(int addr);

int   CAM_EndCaptureImage(void);

int32 CAM_GetNextImageChunk(int32 OsalFileFd, uint32 ImageOffset);

int   CAM_GetImage(char *FileName);

#endif /* _cam_lib_h_ */
