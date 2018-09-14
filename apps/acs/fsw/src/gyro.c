/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: gyro.c                                */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    translating the gyro coordinates         */
/*    to the spacecraft coord system           */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdint.h>
#include <math.h>

/*
** include files:
*/
//#include "vector3.h"
#include "matrix3x3.h"
#include "monitor.h"

const Matrix3x3f ALIGN_SIMU2BF = { { {-1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 1.0, 0.0}}};
//const Vector3f BIAS_RATE = {{0.0070515258, -0.0281202645, -0.0143432548}};
// Monday result	
const Vector3f BIAS_RATE = {{0.0, 0.0, 0.0}};
//const Vector3f BIAS_RATE = {{0.00013388, -0.000517591, -0.000251002}};

/* 
 *params:
 * input - Vector3f *Rate_Simu -  Body rate measurement from Simu (3x1) deg/sec
 * output - Vector3f *Rate_BF_Simu - DCM from SIMU Coordinate to BF
 */

int gyro(Vector3f *Rate_BF_Simu, Vector3f *Rate_Simu) {

Vector3f UnBiased_Rate = {{0.0, 0.0, 0.0}};

  Vector3f_Sub(&UnBiased_Rate, Rate_Simu, &BIAS_RATE); 
//  print_Vector3f("Unbiased Gyro Rate", Rate_BF_Simu);
  Matrix3x3f_MultVec(Rate_BF_Simu, &ALIGN_SIMU2BF, &UnBiased_Rate);
  return 0;
}
