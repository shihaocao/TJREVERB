/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: NavDynamic     .c                     */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*   the Greenwich Hour Angle and NavDynamic   */
/*   calculations                              */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/*
** include files:
*/
#include "spacecraft_ephemeris.h"
#include "gha.h"
#include "vector6.h"
#include "matrix3x3.h"
#include "monitor.h"

#define AEFF 4.0e-8                 // Km^2     Effective cross sectional area of spacecraft (Km2)
#define ANG22   -0.5211056236946071 //  rad     ATAN2(S22,C22), (rad)
#define CS22          1.815378203447825e-006 // Sectorial harmonic SQRT(C22^2 + S22^2) (C22 = 1.57442176e-6; S22 = -9.03766642e-7;) 
#define Density      3.3e-12       // Kg/Km^33     Atmospheric density (Kg/Km3)
#define DragCoefficient  2.0       // Drag coefficient
#define DragFudgeFactor  1.0       // Drag fudge factor
#define EARTHRATE    7.292115854791750e-005 //rad/sec %USED P.M. 11/28/05: Earth Rate  
#define EQEARTHRADIUS        6378.1363e3       //   km     Radius of the earth at the equator (m)
#define MSC    12.0 //   Kg     Spacecraft mass _Kg Full Tank  
#define R_EARTH       6378140.0 //    M     Equatorial radius of the Earth _IAU 1976 value, m  
#define SEC_PER_CENTURY  3155760000.0 //  sec     Number of seconds in a Julian century  
#define SecHarm_0     2.0 // Coefficients for computing sectoral harmonics (5 elements)  
#define SecHarm_1     12.0 // Coefficients for computing sectoral harmonics (5 elements)  
#define SecHarm_2     12.0 // Coefficients for computing sectoral harmonics (5 elements)  
#define SecHarm_3      2.5 // Coefficients for computing sectoral harmonics (5 elements)  
#define SecHarm_4      2.0 // Coefficients for computing sectoral harmonics (5 elements)  
#define ZonHarm_xy2_0   3.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_xy2_1   5.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_xy2_2  -1.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_xy3_0   5.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_xy3_1   7.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_xy3_2  -3.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_xy4_0   3.75 // Coefficients for computing zonal harmonics (4 elements)  
#define ZonHarm_xy4_1   21.0 // Coefficients for computing zonal harmonics (4 elements)  
#define ZonHarm_xy4_2  -14.0 // Coefficients for computing zonal harmonics (4 elements)  
#define ZonHarm_xy4_3   1.0 // Coefficients for computing zonal harmonics (4 elements)  
#define ZonHarm_z2_0    3.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_z2_1    5.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_z2_2   -3.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_z3_0   35.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_z3_1  -30.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_z3_2    3.0 // Coefficients for computing zonal harmonics (3 elements)  
#define ZonHarm_z4_0    1.25 // Coefficients for computing zonal harmonics (4 elements)  
#define ZonHarm_z4_1    63.0 // Coefficients for computing zonal harmonics (4 elements)  
#define ZonHarm_z4_2   -70.0 // Coefficients for computing zonal harmonics (4 elements)  
#define ZonHarm_z4_3    15.0 // Coefficients for computing zonal harmonics (4 elements)  
#define J2                     1.08262645e-3 // Zonal harmonic coefficient J2  
#define J3                    -2.5323078e-6  // Zonal harmonic coefficient J3  
#define J4                    -1.62043e-6 // Zonal harmonic coefficient J4  
#define K3                     8.107673775286226e+027 //Km5/sec2     Gravity parameter, Km5/sec2 _= mu_earth*EqEarthRadius^2/2
#define MU_EARTH               398600.4415e9 //_km^3/sec^2     Earth gravitational constant (km^3/sec^2)  

int NavDynamic ( // inputs
	uint32_t DragOn,
	double   Time_UTC,          // Current system universal time (UTC)
	Vector6f *X,              // Position and Velocity State
	// outputs 
	Matrix3x3f *Dcm_EFToGF,   // direct cosine rotation matrix 
	Vector6f *DX)            // state derivative velocity [m/s] and accel [m/s^2]
             
		{  

     float vRelx, vRely, vRelz, vRel;
     float Rmag, Rmag2, Rmag3;
     float zr, zr2, zr3, zr4;
     float k0, k1, k2, EqERoverRmag; 
     float fxy2, fxy3, fxy4;
     float fz2, fz3, fz4;
     float gha, angle, C22X12, S22X12, f22;
     float agx, agy, agz;
     float adx, ady, adz;

     /*  Compute velocity relative to rotation.  
        Update magnitudes of position vector and relative velocity vector.  
       If DragOn flag is FALSE, do not compute relative velocity and its magnitude. 
       (Based on TRMM)
    */


    //int i;
    #ifdef _ACS_DEBUG_ 
    //printf("Input from X[]: "); 
    #endif
    #ifdef _ACS_DEBUG_ 
    //for (i=0; i<=5; i++)
    //printf(" %f ", X->Comp[i]);
    //printf("\n");
    #endif

	 if (DragOn) 
    {
        vRelx = X->Comp[3] + EARTHRATE * X->Comp[1];
        vRely = X->Comp[4] + EARTHRATE * X->Comp[0];
        vRelz = X->Comp[5];
        vRel  = sqrt(vRelx*vRelx + vRely*vRely + vRelz * vRelz);
        //printf("vRel %f\n", vRel);
    } 
             
    Rmag = sqrt(X->Comp[0]*X->Comp[0] + X->Comp[1]*X->Comp[1]+ X->Comp[2]*X->Comp[2]);
    Rmag2 = Rmag*Rmag;
    Rmag3 = Rmag*Rmag*Rmag;
    EqERoverRmag = EQEARTHRADIUS/Rmag;

    // Update the following parameters (note: X[2] = third  element of X vector) (Based on TRMM)

    k0 = -MU_EARTH / Rmag3;
    k1 = - K3/(Rmag3* Rmag2);
    zr = X->Comp[2]/Rmag;
    zr2 = zr * zr;
    zr3 = zr2 * zr;
    zr4 = zr3 * zr;

    //  Update zonal harmonic terms (Based on TRMM)

    fxy2 = ZonHarm_xy2_0 * J2 * (ZonHarm_xy2_1* zr2 + ZonHarm_xy2_2);
    fxy3 = ZonHarm_xy3_0 * J3 * (EqERoverRmag) * (ZonHarm_xy3_1* zr3 * ZonHarm_xy3_2* zr);
    fxy4 = ZonHarm_xy4_0 * J4 * (EqERoverRmag*EqERoverRmag) * (ZonHarm_xy4_1* zr4 + ZonHarm_xy3_2 * zr2 + ZonHarm_xy4_3);
        
    fz2 = ZonHarm_z2_0 * J2 * (ZonHarm_z2_1* zr2 * ZonHarm_z2_2);
    fz3 = J3 * (EqERoverRmag )* ( X->Comp[2] *(ZonHarm_z3_0 * zr3 + ZonHarm_z3_1* zr) + ZonHarm_z3_2*Rmag);
    fz4 = ZonHarm_z4_0 * J4 * (EqERoverRmag*EqERoverRmag) * (ZonHarm_z4_1 * zr4 + ZonHarm_z4_2 * zr2 + ZonHarm_z4_3);
     
    gha = GHA (Time_UTC, Dcm_EFToGF);
    //print_Matrix3x3f("Dcm", Dcm_EFToGF);
    //printf("gha is %f\n", gha);	
    // Compute sectorial harmonic terms (Based on TRMM)

    angle  = SecHarm_0*gha*ANG22;
    //printf("angle is %f\n", angle);	
    C22X12 = SecHarm_1*CS22 * cos(angle);
    S22X12 = SecHarm_2*CS22 * sin(angle);
    //printf("C22, S22 %f %f\n", C22X12, S22X12);
    f22    = SecHarm_3*(SecHarm_4*S22X12*X->Comp[0] * X->Comp[1] * 
              C22X12 * (X->Comp[0]*X->Comp[0] - X->Comp[1]*X->Comp[1]))/Rmag2;

    //printf("f22 is %f\n", f22);
    
   // agx, agy, agz are acceleration due to earth gravitation gradient: (Based on TRMM)

   agx = k1* ( X->Comp[0] * (fxy2 + fxy3 + fxy4 - f22 + C22X12) + X->Comp[1] * S22X12);
   agy = k1* ( X->Comp[1] * (fxy2 + fxy3 + fxy4 - f22 - C22X12) + X->Comp[0] * S22X12);
   agz = k1* ( X->Comp[2] * (fz2 + fz4  - f22) + fz3);

   //printf("agx,y,z is %f %f %f\n", agx, agy, agz);

   // Compute atmospheric drag effects:  adx, ady, adz are acceleration due to atmospheric drag.  
   // When DragOn is false, set adx, ady, adz to zero.
   // I want to make a Aeff a function of Control Mode (Staring 10/21/14)
	if (DragOn) 
   {
      k2 = -Density * DragCoefficient* DragFudgeFactor * AEFF/(2* MSC);
      adx = k2 * vRel * vRelx;
      ady = k2 * vRel * vRely;
      adz = k2 * vRel * vRelz;
   } 
   else
   {
      k2 = 0;
      adx = 0.0;
      ady = 0.0;
      adz = 0.0;
   } 

   //printf("adx,y,z is %f %f %f\n", adx, ady, adz);
   // Define derivative vector

   DX->Comp[0] = X->Comp[3];
   DX->Comp[1] = X->Comp[4];
   DX->Comp[2] = X->Comp[5];
   DX->Comp[3] = k0 * X->Comp[0] + agx + adx;
   DX->Comp[4] = k0 * X->Comp[1] + agy + ady;
   DX->Comp[5] = k0 * X->Comp[2] + agz + adz;

   #ifdef _ACS_DEBUG_ 
   //printf("DX[] : ");
   #endif
    
    #ifdef _ACS_DEBUG_ 
    //for (i=0; i<=5; i++)
    // printf(" %f ", DX->Comp[i]);
    // printf("\n");
    #endif

return 0;
}
