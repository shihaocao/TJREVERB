/*
 * SysMom.h
 *
 *  Created on: Jan 12, 2016
 *  Author: ssheikh
 */

#include "vector3.h"
#include "matrix3x3.h"

#ifndef SYSMOM_H_
#define SYSMOM_H_

int SysMom(int Simu_DataValid, 
           Vector3f *Rate_BF_Simu, 
           Vector3f *RwaMom_BF,
           Vector3f *SysMom_BF_Calc);

#endif /* SYSMOM_H_ */
