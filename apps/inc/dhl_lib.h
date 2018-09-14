/*************************************************************************
** File:
**   $Id: dhl_lib.h  $
**
** Purpose: 
**   Specification for the Dellingr Hardware Library functions.
**
** References:
**
**   $Log: dhl_lib.h  $
**  
*************************************************************************/
#ifndef _dhl_lib_h_
#define _dhl_lib_h_

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

#include "dhl_hw_defs.h"
#include "l3c_lib.h"
#include "ssc_lib.h"
#include "gps_lib.h"
#include "eps_lib.h"
#include "sol_lib.h"
#include "rw_lib.h"
#include "nm_lib.h"
#include "inms_lib.h"
#include "dagr_lib.h"
#include "cam_lib.h"
#include "tlc_lib.h"
#include "sim_lib.h"
#include "mode_lib.h"
#include "gyro_lib.h"
#include "att_lib.h"

/************************************************************************
** Type Definitions
*************************************************************************/



/*************************************************************************
** Exported Functions
*************************************************************************/

int32 DHL_LibInit(void);

/************************************************************************/



#endif /* _dhl_lib_h_ */

/************************/
/*  End of File Comment */
/************************/
