/** \file
**
** \brief Define 3x3 matrices of type float and functions to operate on them
**
** $Id: matrix3x3f.h 1.2 2008/09/10 10:00:33EDT gwelter Exp  $
**
** \note
**   -# The matrix is represented as a 3x3 matrix of floats with
**      subcripts beginning at 0.
**   -# Defining type Matrix3x3f allows compile-time type checking
**   -# Row and Column Subscripts must range from 0 to 2.
**   -# Adequate stack space is a precondition.
**   -# The called functions do not return any error indicators
**   -# All of the functions allow for the result to be one of the
**      input variables.
**
** References:
**   -# Math Library Specification
**
**  $Date: 2008/09/10 10:00:33EDT $
**
**  $Revision: 1.2 $
**
**  $Log: matrix3x3f.h  $
**  Revision 1.2 2008/09/10 10:00:33EDT gwelter 
**  pre-gpm updates
**  Revision 1.1  2004/01/23 13:22:59  daviddawsonuser
**  directory reorg
**  Revision 1.1  2004/10/04 18:26:02  ddawson
**  added mathlib files to SDO repository
**  Revision 1.2  2004/02/04 20:32:44  rbjork
**  Updates resulting from vector code walkthrough
**  Revision 1.2 2005/03/01 12:32:05EST ddawson 
**  Temporarily added DCM from vector function.  Function wil be remove when
**  new functions added to math library.
**  Revision 1.2 2005/11/29 07:49:13EST dcmccomas 
**  New SDO delivery and added doxygen markup
**  Revision 1.3  2004/02/24 16:23:28  rbjork
**  Updated Prototypes to have one argument per line
**  Revision 1.3 2005/04/01 18:18:34EST ddawson 
**  Added Matrix3x3f_RowVecToMatrix and Matrix3x3f_ColVecToMatrix
**  Revision 1.4  2004/03/23 15:20:13  ddawson
**  removed DOS line endings
**  Revision 1.4.4.3.2.1  2005/11/17 03:41:20  ddawson
**  Imported SDO changes into main trunk and then merged with doxygen branch
**  Revision 1.5  2005/11/16 20:41:59  ddawson
**  Update from SDO, added new RowVecToMatrix and ColVecToMatrix functions
**
**  Revision 2.0 2008/05/21 15:00:21EDT dcmccomas 
**  Revision for new MKS
**  Revision 2.0  2008/06/23 07:00:00  gwelter
**  add Matrix3x3f_Skew, Matrix3x3f_Invert, & Matrix3x3f_Triad
**
**
*/

/** 
** @addtogroup mathlib_matrix
** @{
*/


#ifndef     _matrix3x3_h
#define     _matrix3x3_h

/****************************   Include Files ****************************/

#include <math.h>
#include "vector3.h"

/**************************** Type Definintions  **************************/

typedef struct 
{
  float      Comp[3][3];
} Matrix3x3f;

/******************** Exported  Function Prototypes ***********************/

/**
** \brief Add two 3x3  matrices of type float.
**        Result[3][3] = Left[3][3] + Right[3][3]
**
** \param[out]  Result   Pointer to matrix containing the addition result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   Right    Pointer to right matrix operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_Add (Matrix3x3f       *Result,
                     const Matrix3x3f *Left,
                     const Matrix3x3f *Right);


/**
** \brief Create a 3x3 matrix out of three 3x1 column vectors.
**
** \par Description
** \verbatim
**          Result[3][3] = First[0]  Second[0]  Third[0]
**                         First[1]  Second[1]  Third[1]
**                         First[2]  Second[2]  Third[2]
** \endverbatim
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   First    Pointer to first column 3x1 vector
** \param[in]   Second   Pointer to second column 3x1 vector
** \param[in]   Third    Pointer to third column 3x1 vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_ColVecToMatrix (Matrix3x3f     *Result,
                                const Vector3f *First,
                                const Vector3f *Second,
                                const Vector3f *Third);


/**
** \brief Copy a 3x3 matrix of type float to another 3x3 matrix.
**        Result[3][3] = Operand[3][3]
**
** \param[out]  Result   Pointer to matrix containing a copy of \c Operand
** \param[in]   Operand  Pointer to source matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_Copy (Matrix3x3f       *Result,
                      const Matrix3x3f *Operand);


/**
** \brief Divide each element of a 3x3 Matrix of type float by a scalar.
**        Result[3][3] = Left[3][3] / Scalar
**
** \warning
**    -# Caller must ensure that Scalar is non-zero
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to the matrix whose elements will be divided by \c Scalar
** \param[in]   Scalar   Scalar value used for the division
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_DivScalar (Matrix3x3f       *Result,
                           const Matrix3x3f *Left,
                           double            Scalar);


/**
** \brief Set a 3x3 matrix of type float to identity matrix.
**        Result[3][3] = Identity
**
** \param[out]  Result   Pointer to matrix containing an identity matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_InitIdentity (Matrix3x3f *Result);


/**
** \brief Sets a 3x3 matrix of type float zeros.
**
** \param[out]  Result   Pointer to matrix containing a zero matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_InitZero (Matrix3x3f *Result);

/**
** \brief   Take inverse of 3x3 matrix
**
** \note
**  the input matrix is considered singular if (a) the size of the
**  determinant is less than 1.0e-15 of the largest product of 
**  matrix elements uesd to construct the determinant (i.e., there
**  was extreme cancellation in the determination of the determinant)
**  or (b) the determinant is actually zero
**
** \param[out]  MatrixInverse  pointer to inverse of input matrix m
** \param[in]   Matrix         pointer to 3x3 matrix to be inverted
**
** \returns (int)
**           inverseExists    1 if inverse exists, otherwise 0
*/
int Matrix3x3f_Invert( Matrix3x3f        *MatrixInverse,
                       const Matrix3x3f  *Matrix         );


/**
** \brief Multiply two 3x3 matrices of type float.
**        Result[3][3] = Left[3][3] * Right[3][3]
**
** \param[out]  Result  Pointer to matrix containing the multiplication result
** \param[in]   Left    Pointer to left matrix operand
** \param[in]   Right   Pointer to right matrix operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_Mult (Matrix3x3f       *Result,
                      const Matrix3x3f *Left,
                      const Matrix3x3f *Right);


/**
** \brief Multiply each element of a 3x3 matrix of type float by a scalar.
**        Result[3][3] = Left[3][3] * Scalar
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to matrix whose elements will be multiplied by \c Scalar
** \param[in]   Scalar   Scalar value used for the multiplication
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_MultScalar (Matrix3x3f       *Result,
                            const Matrix3x3f *Left,
                            double            Scalar);


/**
** \brief Multiply a 3x3 matrix of type float by a 3x1 vector of type float.
**        Result[3][1] = Left[3][3] * Right[3][1]
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to left operand matrix
** \param[in]   Right    Pointer to left operand vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_MultVec (Vector3f         *Result,
                         const Matrix3x3f *Left,
                         const Vector3f   *Right);


/**
** \brief Create a 3x3 matrix out of 3 1x3 row vectors.
** 
** \par Decsription
** \verbatim
**          Result[3][3] = First[0]   First[1]   First[2]
**                         Second[0]  Second[1]  Second[2]
**                         Third[0]   Third[1]   Third[2]
** \endverbatim
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   First    Pointer to first row 3x1 vector
** \param[in]   Second   Pointer to second row 3x1 vector
** \param[in]   Third    Pointer to third row 3x1 vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_RowVecToMatrix (Matrix3x3f     *Result,
                                const Vector3f *First,
                                const Vector3f *Second,
                                const Vector3f *Third);

/**
** \brief  Form skew matrix from vector
**
** \param[out] Skew    pointer to 3x3 skew matrix
** \param[in]  Vector  pointer to input 3-vector
**
** \note
**  given a V, the Skew matrix is defined as
**              0   V3 -V2
**             -V3  0   V1
**              V2 -V1  0
*/
void Matrix3x3f_Skew( Matrix3x3f     *Skew,
                      const Vector3f *Vector );


/**
** \brief Subtract two 3x3 matrices of type float.
**        Result[3][3] = Left[3][3] - Right[3][3]
**
** \param[out]  Result   Pointer to matrix containing the subtraction result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   Right    Pointer to right matrix operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_Sub (Matrix3x3f       *Result,
                     const Matrix3x3f *Left,
                     const Matrix3x3f *Right);


/**
** \brief Computes the trace of 3x3 matrix of type float.
**        Result[3][3] = Trace of Operand[3][3]
**
** \param[out]  Operand  Pointer to matrix whose trace is computed
**
** \returns
** \retcode float \retdesc Trace of 3x3 Matrix \endcode
** \endreturns
*/
float Matrix3x3f_Trace (const Matrix3x3f *Operand);


/**
** \brief Perform a matrix transposition on a 3x3 matrix of type float.
**       Result[3][3] = Transpose of Operand[3][3]
**
** \param[out]  Result   Pointer to matrix containing the transpose of \c Operand
** \param[in]   Operand  Pointer to matrix to be transposed
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3f_Transpose (Matrix3x3f       *Result,
                           const Matrix3x3f *Operand);




/*
** \brief  apply TRIAD method to estimate attitude
**
** Note: Triad is a.k.a. the algebraic method for attitude estimation;
**       see Wertz section 12.2.2
**
** \param[out] attDcm   pointer to attitude direction cosine matrix (GCI-to-body)
**
** \param[in]  Ur       pointer to normalized reference vector 1 (GCI frame)
**             Vr       pointer to normalized reference vector 2 (GCI frame)
**             Ub       pointer to normalized measurement vector 2 (body frame)
**             Vb       pointer to normalized measurement vector 2 (body frame)
**
** \returns (int)
**          validity check,  1 if valid attitude estimate is produced, otherwise 0
**  
**  note: Use to create an initial attitude estimate based on data 
**        from two direction sensors (e.g., CSS and TAM). 
*/
int Matrix3x3f_Triad( Matrix3x3f      *AttDcm,
                      const Vector3f  *Ur,
                      const Vector3f  *Vr,
                      const Vector3f  *Ub,
                      const Vector3f  *Vb  );



/****************************   Include Files ****************************/

#include <math.h>
#include "vector3.h"

/**************************** Type Definintions  **************************/

typedef struct 
{
  double      Comp[3][3];
} Matrix3x3d;

/******************** Exported  Function Prototypes ***********************/

/**
** \brief Add two 3x3  matrices of type double.
**        Result[3][3] = Left[3][3] + Right[3][3]
**
** \param[out]  Result   Pointer to matrix containing the addition result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   Right    Pointer to right matrix operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_Add (Matrix3x3d       *Result,
                     const Matrix3x3d *Left,
                     const Matrix3x3d *Right);


/**
** \brief Create a 3x3 matrix out of three 3x1 column vectors.
**
** \par Description
** \verbatim
**          Result[3][3] = First[0]  Second[0]  Third[0]
**                         First[1]  Second[1]  Third[1]
**                         First[2]  Second[2]  Third[2]
** \endverbatim
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   First    Pointer to first column 3x1 vector
** \param[in]   Second   Pointer to second column 3x1 vector
** \param[in]   Third    Pointer to third column 3x1 vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_ColVecToMatrix (Matrix3x3d     *Result,
                                const Vector3d *First,
                                const Vector3d *Second,
                                const Vector3d *Third);


/**
** \brief Copy a 3x3 matrix of type double to another 3x3 matrix.
**        Result[3][3] = Operand[3][3]
**
** \param[out]  Result   Pointer to matrix containing a copy of \c Operand
** \param[in]   Operand  Pointer to source matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_Copy (Matrix3x3d       *Result,
                      const Matrix3x3d *Operand);


/**
** \brief Divide each element of a 3x3 Matrix of type double by a scalar.
**        Result[3][3] = Left[3][3] / Scalar
**
** \warning
**    -# Caller must ensure that Scalar is non-zero
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to the matrix whose elements will be divided by \c Scalar
** \param[in]   Scalar   Scalar value used for the division
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_DivScalar (Matrix3x3d       *Result,
                           const Matrix3x3d *Left,
                           double            Scalar);


/**
** \brief Set a 3x3 matrix of type double to identity matrix.
**        Result[3][3] = Identity
**
** \param[out]  Result   Pointer to matrix containing an identity matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_InitIdentity (Matrix3x3d *Result);


/**
** \brief Sets a 3x3 matrix of type double zeros.
**
** \param[out]  Result   Pointer to matrix containing a zero matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_InitZero (Matrix3x3d *Result);

/**
** \brief   Take inverse of 3x3 matrix
**
** \note
**  the input matrix is considered singular if (a) the size of the
**  determinant is less than 1.0e-15 of the largest product of 
**  matrix elements uesd to construct the determinant (i.e., there
**  was extreme cancellation in the determination of the determinant)
**  or (b) the determinant is actually zero
**
** \param[out]  MatrixInverse  pointer to inverse of input matrix m
** \param[in]   Matrix         pointer to 3x3 matrix to be inverted
**
** \returns (int)
**           inverseExists    1 if inverse exists, otherwise 0
*/
int Matrix3x3d_Invert( Matrix3x3d        *MatrixInverse,
                       const Matrix3x3d  *Matrix         );


/**
** \brief Multiply two 3x3 matrices of type double.
**        Result[3][3] = Left[3][3] * Right[3][3]
**
** \param[out]  Result  Pointer to matrix containing the multiplication result
** \param[in]   Left    Pointer to left matrix operand
** \param[in]   Right   Pointer to right matrix operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_Mult (Matrix3x3d       *Result,
                      const Matrix3x3d *Left,
                      const Matrix3x3d *Right);


/**
** \brief Multiply each element of a 3x3 matrix of type double by a scalar.
**        Result[3][3] = Left[3][3] * Scalar
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to matrix whose elements will be multiplied by \c Scalar
** \param[in]   Scalar   Scalar value used for the multiplication
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_MultScalar (Matrix3x3d       *Result,
                            const Matrix3x3d *Left,
                            double            Scalar);


/**
** \brief Multiply a 3x3 matrix of type double by a 3x1 vector of type double.
**        Result[3][1] = Left[3][3] * Right[3][1]
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to left operand matrix
** \param[in]   Right    Pointer to left operand vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_MultVec (Vector3d         *Result,
                         const Matrix3x3d *Left,
                         const Vector3d   *Right);


/**
** \brief Create a 3x3 matrix out of 3 1x3 row vectors.
** 
** \par Decsription
** \verbatim
**          Result[3][3] = First[0]   First[1]   First[2]
**                         Second[0]  Second[1]  Second[2]
**                         Third[0]   Third[1]   Third[2]
** \endverbatim
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   First    Pointer to first row 3x1 vector
** \param[in]   Second   Pointer to second row 3x1 vector
** \param[in]   Third    Pointer to third row 3x1 vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_RowVecToMatrix (Matrix3x3d     *Result,
                                const Vector3d *First,
                                const Vector3d *Second,
                                const Vector3d *Third);

/**
** \brief  Form skew matrix from vector
**
** \param[out] Skew    pointer to 3x3 skew matrix
** \param[in]  Vector  pointer to input 3-vector
**
** \note
**  given a V, the Skew matrix is defined as
**              0   V3 -V2
**             -V3  0   V1
**              V2 -V1  0
*/
void Matrix3x3d_Skew( Matrix3x3d     *Skew,
                      const Vector3d *Vector );


/**
** \brief Subtract two 3x3 matrices of type double.
**        Result[3][3] = Left[3][3] - Right[3][3]
**
** \param[out]  Result   Pointer to matrix containing the subtraction result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   Right    Pointer to right matrix operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_Sub (Matrix3x3d       *Result,
                     const Matrix3x3d *Left,
                     const Matrix3x3d *Right);


/**
** \brief Computes the trace of 3x3 matrix of type double.
**        Result[3][3] = Trace of Operand[3][3]
**
** \param[out]  Operand  Pointer to matrix whose trace is computed
**
** \returns
** \retcode double \retdesc Trace of 3x3 Matrix \endcode
** \endreturns
*/
double Matrix3x3d_Trace (const Matrix3x3d *Operand);


/**
** \brief Perform a matrix transposition on a 3x3 matrix of type double.
**       Result[3][3] = Transpose of Operand[3][3]
**
** \param[out]  Result   Pointer to matrix containing the transpose of \c Operand
** \param[in]   Operand  Pointer to matrix to be transposed
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix3x3d_Transpose (Matrix3x3d       *Result,
                           const Matrix3x3d *Operand);




/*
** \brief  apply TRIAD method to estimate attitude
**
** Note: Triad is a.k.a. the algebraic method for attitude estimation;
**       see Wertz section 12.2.2
**
** \param[out] attDcm   pointer to attitude direction cosine matrix (GCI-to-body)
**
** \param[in]  Ur       pointer to normalized reference vector 1 (GCI frame)
**             Vr       pointer to normalized reference vector 2 (GCI frame)
**             Ub       pointer to normalized measurement vector 2 (body frame)
**             Vb       pointer to normalized measurement vector 2 (body frame)
**
** \returns (int)
**          validity check,  1 if valid attitude estimate is produced, otherwise 0
**  
**  note: Use to create an initial attitude estimate based on data 
**        from two direction sensors (e.g., CSS and TAM). 
*/
int Matrix3x3d_Triad( Matrix3x3d      *AttDcm,
                      const Vector3d  *Ur,
                      const Vector3d  *Vr,
                      const Vector3d  *Ub,
                      const Vector3d  *Vb  );



#endif /* _matrix3x3_h */
/** @} */ 
