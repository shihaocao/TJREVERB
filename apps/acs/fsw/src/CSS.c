/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: CSS.c                                 */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    Course Sun Sensor data processing.       */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/*
** include files:
*/
#include "CSS.h"
#include "vector3.h"
#include "matrix3x3.h"
#include "monitor.h"


const Matrix3x3f ALIGN_CSS2BF = { { {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};
#define LIM_CSS_LOWER 0
#define LIM_CSS_UPPER 1023
#define THRS_SUNPRES 100.0
#define THRS_NUM_CSS_ECLIPSE  6
#define CSSMAX 1023
#define BIAS_XP 0
#define BIAS_XN 0
#define BIAS_YP 0
#define BIAS_YN 0
#define BIAS_ZP 0
#define BIAS_ZN 0
#define SCALE_XP 1.0
#define SCALE_XN 1.0
#define SCALE_YP 1.0
#define SCALE_YN 1.0
#define SCALE_ZP 1.0
#define SCALE_ZN 1.0

uint32_t CSS ( 
	uint16_t *Cnt_CSS, 
	int      FSS1_Valid,
	int      FSS2_Valid,
	int      FSS3_Valid,
   // outputs
	uint8_t *CSS_Eclipse,
	uint8_t *CSS_SunPres, 
	float *Sun_CssF_CSS,
	Vector3f *Unit_Sun_BF_CSS)
{
   // CSS1 = +X, CSS2 = +Y, CSS3 = +Z
   // CSS4 = -X, CSS5 = -Y, CSS6 = -Z
   static Vector3f Unit_Sun_BF_CSS_Prev;
	Vector3f Sun_CssF_CSS_Diff;
	static Vector3f Sun_CssF_CSS_Diff_Prev;
	int i, j;
	int8_t Sum_CSS_SunPres = 0;
	uint16_t Thresh_SunPres_CSS[6] = {THRS_SUNPRES, THRS_SUNPRES, THRS_SUNPRES, THRS_SUNPRES, THRS_SUNPRES, THRS_SUNPRES};
	float Sun_CssF_CSS_Max[6] = {CSSMAX, CSSMAX, CSSMAX, CSSMAX, CSSMAX, CSSMAX};
	float Mag_Sun_BF_CSS = 0;
	float Bias_CSS[6]  = {BIAS_XP, BIAS_ZP, BIAS_YP, BIAS_XN, BIAS_ZN, BIAS_YN};
	float Scale_CSS[6] = {SCALE_XP, SCALE_ZP, SCALE_YP, SCALE_XN, SCALE_ZN, SCALE_YN};
	Vector3f Sun_BF_CSS;
   uint8_t SunPress = 0;


	/* Solar energy produces a current on the solar cell. Convert raw CSS data to currents (microamperes). */

		//Determine the sun measurement and set Sun Presence flags.
	for (i=0; i<6; i++){

		//Convert the CSS measurement to engineering units.  		 		
		Sun_CssF_CSS[i] = Cnt_CSS[i] * Scale_CSS[i] - Bias_CSS[i];
		//printf("Sun_CssF_CSS %d is %f\r\n", i, Sun_CssF_CSS[i]);


		//Check if data is outside of database specified range or if analog-to-digital data conversions 
        	// completed for each CSS; set CSS_DataADComplete accordingly.  

		if ( (Sun_CssF_CSS[i] > LIM_CSS_UPPER) || (Sun_CssF_CSS[i] < LIM_CSS_LOWER) ){ 
			Sun_CssF_CSS[i] = 0;
		}

		//Determine sun presence and normalize measurement for all CSS sensors	
		if (Sun_CssF_CSS[i] >= Thresh_SunPres_CSS[i]) 
         SunPress = SunPress | (1 << i);

			Sun_CssF_CSS[i] = Sun_CssF_CSS[i]/ Sun_CssF_CSS_Max[i];
			//printf("cssf_css[%d] is %f\n", i, Sun_CssF_CSS[i]);
	}
   //printf("SunPress: 0x%x\n", SunPress);
   *CSS_SunPres = SunPress;
   //printf("SunCssF : %f %f %f %f %f %f\n", Sun_CssF_CSS[0], Sun_CssF_CSS[1], Sun_CssF_CSS[2], Sun_CssF_CSS[3], Sun_CssF_CSS[4], Sun_CssF_CSS[5]); 
        
    	for (j=0; j<6; j++)
        Sum_CSS_SunPres = Sum_CSS_SunPres +  ((SunPress >> j) & 0x01);

	   //printf("Sum_CSS_SunPres = %d\n", Sum_CSS_SunPres);


	//If data does not indicate sun presence,  				
   if (FSS1_Valid || FSS2_Valid || FSS3_Valid) 
   {
	*CSS_Eclipse = 0;
   }
	else if(6 - Sum_CSS_SunPres >= THRS_NUM_CSS_ECLIPSE) {

		*CSS_Eclipse = 1;
       Vector3f_Copy(Unit_Sun_BF_CSS, &Unit_Sun_BF_CSS_Prev);
       Vector3f_Copy(&Sun_CssF_CSS_Diff, &Sun_CssF_CSS_Diff_Prev);
	}
	else {

	*CSS_Eclipse = 0;

	//Determine sun presence directionally.  
	Sun_CssF_CSS_Diff.Comp[0] = Sun_CssF_CSS[0] - Sun_CssF_CSS[3];
	Sun_CssF_CSS_Diff.Comp[1] = Sun_CssF_CSS[1] - Sun_CssF_CSS[4];
	Sun_CssF_CSS_Diff.Comp[2] = Sun_CssF_CSS[2] - Sun_CssF_CSS[5];


	//Rotate to the body frame using full alignment matrix.

   Matrix3x3f_MultVec(&Sun_BF_CSS, &ALIGN_CSS2BF, &Sun_CssF_CSS_Diff);
   //print_Vector3f("Sun_BF_CSS", &Sun_BF_CSS);

	// Compute Vector Mag of Sun_BcF_CSS
	Mag_Sun_BF_CSS = Vector3f_Magnitude(&Sun_BF_CSS);
	//printf("Mag_Sun_BF_CSS = %f\n", Mag_Sun_BF_CSS);

	//Avoid divide by zero.

	if(Mag_Sun_BF_CSS == 0){
			Unit_Sun_BF_CSS->Comp[0] = 0.0;
			Unit_Sun_BF_CSS->Comp[1] = 1.0;
			Unit_Sun_BF_CSS->Comp[2] = 0.0;
	}
	else{
			Unit_Sun_BF_CSS->Comp[0] = Sun_BF_CSS.Comp[0]/Mag_Sun_BF_CSS;
			Unit_Sun_BF_CSS->Comp[1] = Sun_BF_CSS.Comp[1]/Mag_Sun_BF_CSS;
			Unit_Sun_BF_CSS->Comp[2] = Sun_BF_CSS.Comp[2]/Mag_Sun_BF_CSS;
	}

	//for (k=0; k<=2; k++)
	//	printf("Sun_CssF_CSS_Diff_Prev[%d]= %f\n", k,  Sun_CssF_CSS_Diff_Prev.Comp[k]);
   Vector3f_Copy(&Unit_Sun_BF_CSS_Prev, Unit_Sun_BF_CSS);
   Vector3f_Copy(&Sun_CssF_CSS_Diff_Prev, &Sun_CssF_CSS_Diff);
	}

	return 0;
}
