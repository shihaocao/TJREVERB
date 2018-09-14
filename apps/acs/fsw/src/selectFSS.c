#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "vector3.h"
#include "monitor.h"
#include "matrixmxn.h"
#include "matrix3x3.h"
#include "common_types.h"
#include "selectFSS.h"

/*
   FSS1 = +Y, FSS2 = +Y, FSS3 = -Z
*/

float ALIGN_FSS1FToBF[9] = {0.0, -1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
float ALIGN_FSS2FToBF[9] = {0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0};
float ALIGN_FSS3FToBF[9] = {0.0, 1.0, 0.0, 0.0, 0.0, -1.0, -1.0, 0.0, 0.0};

int selectFSS (
   int       FSS1_SunValid,    // FSS1 Data Valid Flag 
   Vector3f  Unit_Sun_FSS1F,   // Unit Sun Vect from FSS1 in FSS1 Frame, Wallops
   int       FSS2_SunValid,    // FSS2 Data Valid Flag
   Vector3f  Unit_Sun_FSS2F,  // Unit Sun Vect from FSS2 in FSS2 Frame, +Y NFSS
   int       FSS3_SunValid,   // FSS2 Data Valid Flag
   Vector3f  Unit_Sun_FSS3F,  // Unit Sun Vect from FSS3 in FSS2 Frame, -Z NFSS
   Vector3f  *Unit_Sun_BF_CSS, // Unit Sun Vect in BF calculated from CSSs
   int       *Use_FSS_AKF,   // Flag for Enabling use of AKF
   Vector3f  *Unit_Sun_BF    // Unit Sun Vect in BCS Frame
  )
{
   int FSS_Pref[3] = {0, 1, 2};
   int  FSS_SunValid[3];

   FSS_SunValid[0] = FSS1_SunValid;
   FSS_SunValid[1] = FSS2_SunValid;
   FSS_SunValid[2] = FSS3_SunValid;
   

	if (FSS_SunValid[FSS_Pref[0]])
		{
		   MatrixMxNf_Mult(Unit_Sun_BF->Comp, ALIGN_FSS1FToBF, 3,3, Unit_Sun_FSS1F.Comp, 3, 1);
         //printf("0 valid\n");
		   *Use_FSS_AKF = 1;
		}
		else if (FSS_SunValid[FSS_Pref[1]])
		{
		   MatrixMxNf_Mult(Unit_Sun_BF->Comp, ALIGN_FSS2FToBF, 3,3, Unit_Sun_FSS2F.Comp, 3, 1);
         //printf("1 valid\n");
		   *Use_FSS_AKF = 1;
		}
		else if (FSS_SunValid[FSS_Pref[2]])
		{
		   MatrixMxNf_Mult(Unit_Sun_BF->Comp, ALIGN_FSS3FToBF, 3,3, Unit_Sun_FSS3F.Comp, 3, 1);
         //printf("2 valid\n");
		   *Use_FSS_AKF = 1;
		}
      else
		{
        //print_Vector3f("sel: Unit_Sun_BF_CSS", Unit_Sun_BF_CSS);
        Vector3f_Copy(Unit_Sun_BF, Unit_Sun_BF_CSS);
		  *Use_FSS_AKF = 0;
		}
        //print_Vector3f("sFSS: Unit_Sun_BF", Unit_Sun_BF); 

return 0;

}
