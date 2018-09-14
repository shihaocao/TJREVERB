/* * * * * * * * * * * * * * * * * * * * * * * */
/* File: SysMom.c                   */
/*                                             */
/* Purpose:                                    */
/*   This file contains the source code for    */
/*    calculating the system momentum          */
/*    used for momentum management             */
/*    If SIMU rate data is invalid, it is      */
/*    excluded from the calculation and        */
/*    SysMom is set equal to RwaMom            */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <stdint.h>
#include <math.h>

/*
** include files:
*/
#include "vector3.h"
#include "matrix3x3.h"

const Matrix3x3f MOI_SC = {{
      { 0.121, -0.001, -0.004}, 
      {-0.001,  0.149, -0.003}, 
      {-0.004, -0.003,  0.045}}};

/* 
 *params:
 * input - Vector3f *Rate_BF_Simu - i Body rate measurement from Simu (3x1) rad/sec
 * input - Vector3f *RwaMom_BF - Momentum in the reaction wheels estimate from
 *                               RWA speed dataa (3x1) Nms
 * input  - int Simu_DataValid  - flag indicating SIM rate data is valid
 * output - Vector3f *SysMom_BF_Calc - director cosine matrix of tranf from GF to BF
 */

int SysMom(int Simu_DataValid, Vector3f *Rate_BF_Simu, Vector3f *RwaMom_BF,
      Vector3f *SysMom_BF_Calc) {

  Vector3f BodyMom_BF = {{0.0, 0.0, 0.0}};

  Matrix3x3f_MultVec(&BodyMom_BF, &MOI_SC, Rate_BF_Simu);

  if (Simu_DataValid == 1) 
    Vector3f_Add(SysMom_BF_Calc, &BodyMom_BF, RwaMom_BF);
  else
    Vector3f_Copy(SysMom_BF_Calc, RwaMom_BF);

  
  return 0;
}
