#ifndef _IGRF_H
#define _IGRF_H

#include "vector3.h"

//Extrapolate model
int extrapsh(float date);

//Calculates field components from models
int new_igrf(float flat,float flon,float elev,int nmax, Vector3f *dest);

#endif

