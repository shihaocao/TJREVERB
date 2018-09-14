/*
 * Utility functions as defined in section 7.
 */

//#include "common_types.h"
#include "vector3.h"
#include "vector4.h"
#include "matrix3x3.h"
#include "matrixmxn.h"

/*
 *7.1:
 * Create skew cross-product matrix.
 * Compare to Matrix3x3f_Skew, which returns the transpose of this.
 *
 *note:
 * Skew matrix defined as: 0  -V3  V2
 *                         V3  0  -V1
 *                        -V2  V1  0  
 *
 *params:
 * input - Vector3f *v - 3 vector to turn into skew
 * output - Matrix3x3f *x - resulting skew matrix
 */
void CrossMatrixf(Matrix3x3f *x, Vector3f *v);
void CrossMatrixd(Matrix3x3d *x, Vector3d *v);
/*
 *7.2:
 * This routine converts a 3x3 direction cosine matrix into its equivalent quaternion.
 * Indices are as in C, which is 0-indexed, column-first. The quaternion has its 
 * fourth element (i.e. Qout[3]) as the scalar component. 
 *
 *params:
 * input - Matrix3x3f *CosMatrix
 * output - Vector4f *Qout 
 */
void DcmToQuat(Vector4f *Qout, Matrix3x3f *CosMat);

/*
 *7.2.1.1:
 * This routine provides the conjugate of the given quaternion (does not modify
 * given quaternion).
 * Probably numbered wrong.
 *
 *params:
 * input - Vector4f *Qin
 * output - Vector4f *Qinvert - the conjugated quaternion
 */
void QuaternionConjugation(Vector4f *Qinvert, Vector4f *Qin);

/*
 *7.3:
 * Quaternion Multiplication (Supports Vector Rotation)
 * This routine performs quaternion multiplication without enforcing the positive 4th
 * quaternion element convention. This routine supports the ‘RotateVector’ utility.
 * The vector rotation operation must not enforce the positive fourth element, or a
 * sign ambiguity is created.  When the positive sign of the 4th element must be
 * enforced, that will take place after QuaternionMultiply. Also note that, since
 * this supports the rotation of a vector, the output quaternion is not normalized –
 * because the input quaternion is really a vector and we don’t want to change the
 * magnitude of the input vector inadvertently.
 *
 * TODO: Waiting on response about normalization.
 *
 *params:
 * input - Vector4f *Qinit - initial quaternion
 * input - Vector4f *Qrot - quaternion representing the desired rotation
 * output - Vector4f *Qout
 */
void QuaternionMultiplication(Vector4f *Qout, Vector4f *Qinit, Vector4f *Qrot);

/*
 *7.4 Quaternion Normalization
 * TODO: Wating to hear about sign check for 4th element.
 * If no check is needed, then same as Vector4f_Normalize.
 * If check is added, MUST search for all Vector4f_Normalize calls and fix.
 */

/*
 *7.5:
 * Quaternion Scalar Component Positive - 
 * This routine guarantees the scalar (i.e. fourth) component of the input quaternion
 * is positive or zero.
 * 
 * TODO: Change name to QuaternionPositive, or update all others to Q4Whatever
 *
 *params:
 * input - Vector4f *Qin
 * output - Vector4f *Qout
 */
void Q4Positive(Vector4f *Qout, Vector4f *Qin);

/*
 *7.6:
 * Why not use normalization functions for Vector3f and so on?
 * This one prevents / 0 errors, perhaps?
 *
 * TODO: Need to check all calls to the Vector Normalization functions
 * Figure out if checks should be done first. Then decide about this one
 */
void QuatToMatrix(Matrix3x3f *Mat, Vector4f *Qin);

/*
 * Standard signum function: returns 1 if f is positive, -1 if negative, and 0 otherwise
 */
int signum(float f);

void Quat_Conj(Vector4f *Qinvert, Vector4f *Qin);

void Quat_Mult(Vector4f *Qout, Vector4f *Qinit, Vector4f *Qrot);

void Quat_Norm(Vector4f *Qout);
