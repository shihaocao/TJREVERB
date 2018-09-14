/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: solar_ephemeris.c                     */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    the solar ephemeris calculations.        */
/*                                             */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <math.h>
#include <stdio.h>
#include <stdint.h>

/*
** include files
*/
#include "vector3.h"
#include "solar_ephemeris.h"

//#define  DATE_EPOCH 		2451545.00  // Jan 01, 2000
#define  DATE_EPOCH 		2436204.5  // Jan 01, 1958 JD
#define  DATE_JD_GPS    2444239.5  // Jan 01, 1980 JD
#define  COEFF_L1   		280.46
#define  COEFF_L2   		0.98565
#define  COEFF_G1   		357.53
#define  COEFF_G2   		0.98560
#define  COEFF_LONG1   	1.915
#define  COEFF_LONG2   	0.02
#define  COS_OBLIQ_ECLP	0.91748
#define  SIN_OBLIQ_ECLP	0.39779

int32_t solar_ephemeris (double TimeTAI, Vector3f * Unit_Sun_GciF)
{
	float Date_Julian, Date_Solar; 
	float g, g_rad;
	float Long_Ecliptic;
	float Sin_Long_Ecliptic;

   //Date_Julian = DATE_JD_GPS + (TimeTAI + 32.184)/86400.0;
   //Date_Julian = DATE_JD_GPS + TimeTAI/86400.0;
	//Date_Solar = Date_Julian - DATE_EPOCH;
   Date_Solar = TimeTAI/86400 + DATE_JD_GPS - DATE_EPOCH;
   
	//printf("Date_Solar %lf\n", Date_Solar);

	g = COEFF_G1 + COEFF_G2 * (float)Date_Solar;
	g_rad = g*M_PI/180.0;
   //printf("g_rad is %f\n", g_rad);

	Long_Ecliptic = COEFF_L1 + COEFF_L2 * Date_Solar + 
		COEFF_LONG1*sin(g_rad) + COEFF_LONG2*sin(2*g_rad);
	//printf("LongEcliptic is %f\n", Long_Ecliptic);
	Sin_Long_Ecliptic = sin(Long_Ecliptic*M_PI/180.0);
	Unit_Sun_GciF->Comp[0] = cos(Long_Ecliptic*M_PI/180.0);
	Unit_Sun_GciF->Comp[1] = COS_OBLIQ_ECLP * Sin_Long_Ecliptic;
	Unit_Sun_GciF->Comp[2] = SIN_OBLIQ_ECLP * Sin_Long_Ecliptic;

	return 0;
}
