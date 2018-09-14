/*************************************************************************
** File:
**   $Id: mode_lib.c  $
**
** Purpose: 
**     Mode functions 
**
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "dhl_lib.h"
#include "dhl_lib_version.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/

/*************************************************************************
** Private Function Prototypes
*************************************************************************/

/*************************************************************************
** Global data and functions 
*************************************************************************/

/*
** Global Mode data 
*/

/*
** This structure will have to be reloated to where the memory
**  does not get cleared on a processor reset
*/
SpacecraftMode_t SpacecraftMode;


/*
** Functions
*/

int32  MODE_LibInit(void)
{
    /*
    ** Do we want to have this as part of the init?
    */
    SpacecraftMode.SpacecraftMode = MODE_SAFE;
    SpacecraftMode.ACSConfig = 1;
    return(0);
}

uint32 MODE_GetSpacecraftMode(void)
{
    return(SpacecraftMode.SpacecraftMode);
}

uint32 MODE_GetACSConfig(void)
{ 
    return(SpacecraftMode.ACSConfig);
}

void   MODE_SetSpacecraftMode(uint32 NewMode)
{
    if ( NewMode > MODE_INMS)
    {
       return;
    }
    else
    {
       SpacecraftMode.SpacecraftMode = NewMode;
    }
}

void   MODE_SetACSConfig(uint32 NewConfig)
{
   SpacecraftMode.ACSConfig = NewConfig;
}

