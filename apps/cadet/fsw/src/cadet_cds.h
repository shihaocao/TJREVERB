/*
** File: cadet_cds.h
**
** Purpose:
**   Define CADET App Critical Data Store messages, exported variables, and interface routines.
**
*/
#ifndef _cadet_cds_h_
#define _cadet_cds_h_



/*
**   Includes:
*/
#include "cfe.h"



/*
** Defines
*/
#define CADET_CDS_NAME                  "CADET_CDS"



/*
** Structures
*/
typedef struct
{
    /*
    **  Filter Table File Name
    */
    char       LastTblFileLoaded[OS_MAX_PATH_LEN]; 
    
}  CADET_CDSDataType_t;



/*
** Exported Variables
*/
extern CADET_CDSDataType_t      CADET_CritDS;          /* Critical Data Store interface area */



/*
** Exported Functions
*/
extern void CADET_CDS_UpdateCriticalDataStore ( void );
extern boolean CADET_CDS_AccessCriticalDataStore ( void );



#endif /* _cadet_cds_h_ */



/************************/
/*  End of File Comment */
/************************/
