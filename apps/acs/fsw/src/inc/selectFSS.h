#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "vector3.h"
//#include "matrixmxn.h"
#include "common_types.h"

int selectFSS (
   int      FSS1_SunValid,  // FSS1 Data Valid Flag 
   Vector3f Unit_Sun_FSS1F, // Unit Sun Vect from FSS1 in FSS1 Frame, Wallops
   int      FSS2_SunValid,  // FSS2 Data Valid Flag
   Vector3f Unit_Sun_FSS2F,  // Unit Sun Vect from FSS2 in FSS2 Frame, +Y NFSS
   int      FSS3_SunValid,  // FSS2 Data Valid Flag
   Vector3f Unit_Sun_FSS3F,  // Unit Sun Vect from FSS3 in FSS2 Frame, -Z NFSS
   Vector3f *Unit_Sun_BF_CSS, // Unit Sun Vect in BF calculated from CSSs
   int      *Use_FSS_AKF,   // Flag for Enabling use of AKF
   Vector3f *Unit_Sun_BF    // Unit Sun Vect in BCS Frame
);

