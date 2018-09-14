#include "vector4.h"
#include "vector3.h"
#include "matrix3x3.h"
#include "util.h"

#include <math.h>

// modified from Matrix3x3f_Skew
void CrossMatrixd(Matrix3x3d *x, Vector3d *v) {
  x->Comp[0][0] =    0.0; 
  x->Comp[1][1] =    0.0;
  x->Comp[2][2] =    0.0;
	
  x->Comp[1][0] =   v->Comp[2];
  x->Comp[0][1] = - v->Comp[2];

  x->Comp[2][0] = - v->Comp[1];
  x->Comp[0][2] =   v->Comp[1];

  x->Comp[2][1] =   v->Comp[0];
  x->Comp[1][2] = - v->Comp[0];
}

void CrossMatrixf(Matrix3x3f *x, Vector3f *v) {
  x->Comp[0][0] =    0.0; 
  x->Comp[1][1] =    0.0;
  x->Comp[2][2] =    0.0;
	
  x->Comp[1][0] =   v->Comp[2];
  x->Comp[0][1] = - v->Comp[2];

  x->Comp[2][0] = - v->Comp[1];
  x->Comp[0][2] =   v->Comp[1];

  x->Comp[2][1] =   v->Comp[0];
  x->Comp[1][2] = - v->Comp[0];
}

void DcmToQuat(Vector4f *Qout, Matrix3x3f *CosMat) {
  float mat00 = CosMat->Comp[0][0];
  float mat11 = CosMat->Comp[1][1];
  float mat22 = CosMat->Comp[2][2];
  float trA = mat00 + mat11 + mat22;

  if ((trA >= mat00) && (trA >= mat11) && (trA >= mat22)) {
    Qout->Comp[0] = CosMat->Comp[2][1] - CosMat->Comp[1][2];
    Qout->Comp[1] = CosMat->Comp[0][2] - CosMat->Comp[2][0];
    Qout->Comp[2] = CosMat->Comp[1][0] - CosMat->Comp[0][1];
    Qout->Comp[3] = 1 + trA;
    Vector4f_Normalize(Qout, Qout);
  } else if ((mat00 >= mat11) && (mat00 >= mat22)) {
    Qout->Comp[0] = 1 + mat00 - mat11 - mat22;
    Qout->Comp[1] = CosMat->Comp[1][0] + CosMat->Comp[0][1];
    Qout->Comp[2] = CosMat->Comp[0][2] + CosMat->Comp[2][0];
    Qout->Comp[3] = CosMat->Comp[2][1] - CosMat->Comp[1][2];
  } else if (mat11 >= mat22) {
    Qout->Comp[0] = CosMat->Comp[1][0] + CosMat->Comp[0][1];
    Qout->Comp[1] = 1 - mat00 + mat11 - mat22;
    Qout->Comp[2] = CosMat->Comp[2][1] + CosMat->Comp[1][2];
    Qout->Comp[3] = CosMat->Comp[0][2] - CosMat->Comp[2][0];
  } else {
    Qout->Comp[0] = CosMat->Comp[0][2] + CosMat->Comp[2][0];
    Qout->Comp[1] = CosMat->Comp[2][1] + CosMat->Comp[1][2];
    Qout->Comp[2] = 1 - mat00 - mat11 + mat22;
    Qout->Comp[3] = CosMat->Comp[1][0] - CosMat->Comp[0][1];
  }
}

void Quat_Conj(Vector4f *Qinvert, Vector4f *Qin) {
  unsigned char index;

  for (index = 0; index < 3; index++) {
    Qinvert->Comp[index] = - Qin->Comp[index];
  }
  Qinvert->Comp[3] = Qin->Comp[3];
}

// Modled after Matrix3x3f
typedef struct
{
  float Comp[4][4];
} Matrix4x4f;

// Modled after Matrix3x3f_MultVec
void Matrix4x4f_MultVec (Vector4f         *Result, 
			 const Matrix4x4f *Left, 
			 const Vector4f   *Right) {
  Vector4f    Rslt;

  MatrixMxNf_Mult(&Rslt.Comp[0], &Left->Comp[0][0],4,4,&Right->Comp[0],4,1);
  MatrixMxNf_Copy(&Result->Comp[0],&Rslt.Comp[0],4,1);
}

void Quat_Mult(Vector4f *Qout, Vector4f *Qinit, Vector4f *Qrot) {
  Matrix4x4f mat;
  mat.Comp[0][0] =   Qrot->Comp[3];
  mat.Comp[0][1] =   Qrot->Comp[2];
  mat.Comp[0][2] = - Qrot->Comp[1];
  mat.Comp[0][3] =   Qrot->Comp[0];
  mat.Comp[1][0] = - Qrot->Comp[2];
  mat.Comp[1][1] =   Qrot->Comp[3];
  mat.Comp[1][2] =   Qrot->Comp[0];
  mat.Comp[1][3] =   Qrot->Comp[1];
  mat.Comp[2][0] =   Qrot->Comp[1];
  mat.Comp[2][1] = - Qrot->Comp[0];
  mat.Comp[2][2] =   Qrot->Comp[3];
  mat.Comp[2][3] =   Qrot->Comp[2];
  mat.Comp[3][0] = - Qrot->Comp[0];
  mat.Comp[3][1] = - Qrot->Comp[1];
  mat.Comp[3][2] = - Qrot->Comp[2];
  mat.Comp[3][3] =   Qrot->Comp[3];

  Matrix4x4f_MultVec(Qout, &mat, Qinit);

  // TODO: Comments say not to normalize, yet code has a call to normalize
  Vector4f_Normalize(Qout, Qout);
}

// This routine normalizes a given quaternion (modifies the given quaternion)
void Quat_Norm(Vector4f *Qout) {
  float temp = 0.0, sum = 0.0;
  int i = 0;

  for (i = 0; i < 4; i++) { // sum the ^2 of each component
    sum += Qout->Comp[i] * Qout->Comp[i];
  }

  temp = sqrt(sum);

  for (i = 0; i < 4; i++) {
    Qout->Comp[i] = Qout->Comp[i] / temp;
  }
}

void Q4Positive(Vector4f *Qout, Vector4f *Qin) {
  if (Qin->Comp[3] < 0) {
    Vector4f_MultScalar(Qout, Qin, -1.0);
  } else {
    Vector4f_MultScalar(Qout, Qin, 1.0);
  }
}


void QuatToMatrix(Matrix3x3f *Mat, Vector4f *Qin) {
  Mat->Comp[0][0] = 1 - 2*Qin->Comp[1]*Qin->Comp[1] - 2*Qin->Comp[2]*Qin->Comp[2];
  Mat->Comp[0][1] = 2*Qin->Comp[0]*Qin->Comp[1] - 2*Qin->Comp[2]*Qin->Comp[3];
  Mat->Comp[0][2] = 2*Qin->Comp[0]*Qin->Comp[2] + 2*Qin->Comp[1]*Qin->Comp[3];
  Mat->Comp[1][0] = 2*Qin->Comp[0]*Qin->Comp[1] + 2*Qin->Comp[2]*Qin->Comp[3];
  Mat->Comp[1][1] = 1 - 2*Qin->Comp[0]*Qin->Comp[0] - 2*Qin->Comp[2]*Qin->Comp[2];
  Mat->Comp[1][2] = 2*Qin->Comp[1]*Qin->Comp[2] - 2*Qin->Comp[0]*Qin->Comp[3];
  Mat->Comp[2][0] = 2*Qin->Comp[0]*Qin->Comp[2] - 2*Qin->Comp[1]*Qin->Comp[3];
  Mat->Comp[2][1] = 2*Qin->Comp[1]*Qin->Comp[2] + 2*Qin->Comp[0]*Qin->Comp[3];
  Mat->Comp[2][2] =  1 - 2*Qin->Comp[0]*Qin->Comp[0] - 2*Qin->Comp[1]*Qin->Comp[1];
}

int signum(float f) {
  if (f < 0) {
    return -1;
  } else if (f > 0) {
    return 1;
  } else {
    return 0;
  }
}



