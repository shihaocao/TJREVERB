/*
 * CSS.h
 *
 *  Created on: Dec 8, 2014
 *      Author: ssheikh
 */

#ifndef CSS_H_
#define CSS_H_

#include "vector3.h"

uint32_t CSS ( 
	uint16_t *Cnt_CSS, 
	int      FSS1_Valid,
	int      FSS2_Valid,
	int      FSS3_Valid,
	// Outputs
	uint8_t *CSS_Eclipse,
	uint8_t *CSS_SunPres, 
	float *Sun_CssF_CSS,
	Vector3f *Unit_Sun_BF_CSS);

#endif /* CSS_H_ */
