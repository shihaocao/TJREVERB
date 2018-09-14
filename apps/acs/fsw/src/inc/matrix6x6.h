/** \file
**
** \brief Define 6x6 matrices of type float and functions to operate on them
** 
** $Id: matrix6x6f.h 1.1 2008/05/21 15:00:22EDT dcmccomas Exp  $
**
** \note
**  -# The matrix is represented as a 6x6 matrix of floats with
**     subcripts beginning at 0.
**  -# Row and Column Subscripts must range from 0 to 5.
**  -# Adequate stack space is a precondition.
**  -# The called functions do not return any error indicators
**  -# All the functions can be called to allow the result to be the
**     same variable as one of the inputs.
**
** References:
**  -# Math Library Specifications
**
**  $Date: 2008/05/21 15:00:22EDT $
**
**  $Revision: 1.1 $
**
**  $Log: matrix6x6f.h  $
**  Revision 1.1 2008/05/21 15:00:22EDT dcmccomas 
**  Initial revision
**  Revision 1.2 2005/11/29 07:49:15EST dcmccomas 
**  New SDO delivery and added doxygen markup
**  Revision 1.5  2004/03/23 15:20:13  ddawson
**  removed DOS line endings
**
**  Revision 1.4  2004/02/24 16:23:28  rbjork
**  Updated Prototypes to have one argument per line
**
**  Revision 1.3  2004/02/05 20:33:07  rbjork
**  Additional changes from code walkthrough
**
**  Revision 1.2  2004/02/04 20:32:44  rbjork
**  Updates resulting from vector code walkthrough
**
**  Revision 1.1  2004/01/23 13:22:59  daviddawsonuser
**  directory reorg
**
**
*/

/** 
** @addtogroup math_matrix
** @{
*/


#ifndef     _matrix6x6_h
#define     _matrix6x6_h

/******************************** Type Definitions **************************/

typedef struct 
{
  float      Comp[6][6];
} Matrix6x6f;

/**************************** Exported Function Prototypes *******************/

/**
** \brief Add two 6x6  matrices of type float.
**        Result[6][6] = Left[6][6] + Right[6][6]
**
** \param[out]  Result   Pointer to matrix containing the addition result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   Right    Pointer to right matrix operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6f_Add (Matrix6x6f       *Result,
                     const Matrix6x6f *Left,
                     const Matrix6x6f *Right);


/**
** \brief Copy a 6x6 matrix of type float.
**        Result[6][6] = Operand[6][6]
**
** \param[out]  Result   Pointer to matrix containing a copy of \c Operand
** \param[in]   Operand  Pointer to source matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6f_Copy (Matrix6x6f       *Result,
                      const Matrix6x6f *Operand);


/**
** \brief Divide each element of a 6x6 Matrix of type float by a scalar.
**        Result[6][6] = Left[6][6] / Scalar
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
void Matrix6x6f_DivScalar (Matrix6x6f       *Result,
                           const Matrix6x6f *Left,
                           float            Scalar);


/**
** \brief Set a 6x6 matrix of type float to identity matrix.
**        Result[6][6] = Identity
**
** \param[out]  Result   Pointer to matrix containing an identity matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6f_InitIdentity (Matrix6x6f *Result);


/**
** \brief Sets a 6x6 matrix of type float to all zeros.
**
** \param[out]  Result   Pointer to matrix containing a zero matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6f_InitZero (Matrix6x6f *Result);


/**
** \brief Multiply two 6x6 matrices of type float.
**        Result[6][6] = Left[6][6] * Right[6][6]
**
** \param[out]  Result  Pointer to matrix containing the multiplication result
** \param[in]   Left    Pointer to left matrix operand
** \param[in]   Right   Pointer to right matrix operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6f_Mult (Matrix6x6f       *Result,
                      const Matrix6x6f *Left,
                      const Matrix6x6f *Right);


/**
** \brief Multiply each element of a 6x6 matrix of type float  by a scalar.
**        Result[6][6] = Left[6][6] * Scalar
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to matrix whose elements will be multiplied by \c Scalar
** \param[in]   Scalar   Scalar value used for the multiplication
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6f_MultScalar (Matrix6x6f       *Result,
                            const Matrix6x6f *Left,
                            float            Scalar);


/**
** \brief Subtract two 6x6 matrices of type float.
**        Result[6][6] = Left[6][6] - Right[6][6]
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to left operand matrix
** \param[in]   Right    Pointer to left operand vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6f_Sub (Matrix6x6f       *Result,
                     const Matrix6x6f *Left,
                     const Matrix6x6f *Right);


/**
** \brief Computes the trance of a 6x6 matrix of type float.
**        Trace  = Trace of Operand[6][6]
**
** \param[out]  Operand  Pointer to matrix whose trace is computed
**
** \returns
** \retcode float \retdesc Trace of 6x6 Matrix \endcode
** \endreturns
*/
float Matrix6x6f_Trace (const Matrix6x6f *Operand);


/**
** \brief Perform a matrix transposition on a 6x6 matrix of type float.
**        Result[6][6] = Transpose of Operand[6][6]
**
** \param[out]  Result   Pointer to matrix containing the transpose of \c Operand
** \param[in]   Operand  Pointer to matrix to be transposed
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6f_Transpose (Matrix6x6f       *Result,
                           const Matrix6x6f *Operand);


/******************************** Type Definitions **************************/

typedef struct 
{
  double      Comp[6][6];
} Matrix6x6d;

/**************************** Exported Function Prototypes *******************/

/**
** \brief Add two 6x6  matrices of type double.
**        Result[6][6] = Left[6][6] + Right[6][6]
**
** \param[out]  Result   Pointer to matrix containing the addition result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   Right    Pointer to right matrix operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6d_Add (Matrix6x6d       *Result,
                     const Matrix6x6d *Left,
                     const Matrix6x6d *Right);


/**
** \brief Copy a 6x6 matrix of type double.
**        Result[6][6] = Operand[6][6]
**
** \param[out]  Result   Pointer to matrix containing a copy of \c Operand
** \param[in]   Operand  Pointer to source matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6d_Copy (Matrix6x6d       *Result,
                      const Matrix6x6d *Operand);


/**
** \brief Divide each element of a 6x6 Matrix of type double by a scalar.
**        Result[6][6] = Left[6][6] / Scalar
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
void Matrix6x6d_DivScalar (Matrix6x6d       *Result,
                           const Matrix6x6d *Left,
                           double            Scalar);


/**
** \brief Set a 6x6 matrix of type double to identity matrix.
**        Result[6][6] = Identity
**
** \param[out]  Result   Pointer to matrix containing an identity matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6d_InitIdentity (Matrix6x6d *Result);


/**
** \brief Sets a 6x6 matrix of type double to all zeros.
**
** \param[out]  Result   Pointer to matrix containing a zero matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6d_InitZero (Matrix6x6d *Result);


/**
** \brief Multiply two 6x6 matrices of type double.
**        Result[6][6] = Left[6][6] * Right[6][6]
**
** \param[out]  Result  Pointer to matrix containing the multiplication result
** \param[in]   Left    Pointer to left matrix operand
** \param[in]   Right   Pointer to right matrix operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6d_Mult (Matrix6x6d       *Result,
                      const Matrix6x6d *Left,
                      const Matrix6x6d *Right);


/**
** \brief Multiply each element of a 6x6 matrix of type double  by a scalar.
**        Result[6][6] = Left[6][6] * Scalar
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to matrix whose elements will be multiplied by \c Scalar
** \param[in]   Scalar   Scalar value used for the multiplication
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6d_MultScalar (Matrix6x6d       *Result,
                            const Matrix6x6d *Left,
                            double            Scalar);


/**
** \brief Subtract two 6x6 matrices of type double.
**        Result[6][6] = Left[6][6] - Right[6][6]
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to left operand matrix
** \param[in]   Right    Pointer to left operand vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6d_Sub (Matrix6x6d       *Result,
                     const Matrix6x6d *Left,
                     const Matrix6x6d *Right);


/**
** \brief Computes the trance of a 6x6 matrix of type double.
**        Trace  = Trace of Operand[6][6]
**
** \param[out]  Operand  Pointer to matrix whose trace is computed
**
** \returns
** \retcode double \retdesc Trace of 6x6 Matrix \endcode
** \endreturns
*/
double Matrix6x6d_Trace (const Matrix6x6d *Operand);


/**
** \brief Perform a matrix transposition on a 6x6 matrix of type double.
**        Result[6][6] = Transpose of Operand[6][6]
**
** \param[out]  Result   Pointer to matrix containing the transpose of \c Operand
** \param[in]   Operand  Pointer to matrix to be transposed
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Matrix6x6d_Transpose (Matrix6x6d       *Result,
                           const Matrix6x6d *Operand);


#endif /* _matrix6x6_h */
/** @} */
