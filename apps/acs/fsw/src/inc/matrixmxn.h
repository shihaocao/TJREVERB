/** \file
** 
** \brief Define unbounded two dimensioned (MxN) matrices of type float and
**        functions to operate on them
**
** $Id: matrixmxnf.h 1.1 2008/05/21 15:00:22EDT dcmccomas Exp  $
**
** \note
**   -# The matrix is represented as a MxN matrix of floats with
**      subcripts beginning at 0.
**   -# Row Subscripts must range from 0 to (M-1).
**   -# Column Subscripts must range from 0 to (N-1).
**   -# Adequate stack space is a precondition.
**   -# The calling program is responsible for ensuring that the 
**      arguments are consistent with the requested feature.
**   -# The called functions do not return any error indicators
**   -# The math functions included in this package are defined as
**      using undimensioned arrays, the functions use pointers ot access
**      these undimensioned arrays.
**   -# In general the functions in this package allow for the result
**      to be one of the input variables.  However, the functions that
**      multiply two matrices or which computes the transpose of matrix  
**      do not allow the result to be one of the input variables.
**    
** References:
**   -# Math Library Specifications
**
**  $Date: 2008/05/21 15:00:22EDT $
**
**  $Revision: 1.1 $
**
**  $Log: matrixmxnf.h  $
**  Revision 1.1 2008/05/21 15:00:22EDT dcmccomas 
**  Initial revision
**  Revision 1.2 2005/11/29 07:49:15EST dcmccomas 
**  New SDO delivery and added doxygen markup
**  Revision 1.4  2004/03/23 15:20:13  ddawson
**  removed DOS line endings
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

#ifndef     _matrixMxN_h
#define     _matrixMxN_h



/************************ Exported Function prototypes ***************************/


/**
** \brief Add two MxN  matrices of type float.
**        Result[m][n] = Left[m][n] + Right[m][n]
**
** \param[out]  Result   Pointer to matrix containing the addition result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   Right    Pointer to right matrix operand
** \param[in]   RowSize  Number of rows in the matrices
** \param[in]   ColSize  Number of columns in the matrices
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNf_Add (float              Result[],
                     const float        Left[],
                     const float        Right[],
                     unsigned int        RowSize,
                     unsigned int        ColSize);


/**
** \brief Copy MxN matrix of type float to another MxN matrix.
**        Result[m][n] = Operand[m][n]
**
** \param[out]  Result   Pointer to matrix containing a copy of \c Operand
** \param[in]   Operand  Pointer to source matrix
** \param[in]   RowSize  Number of rows in the matrix
** \param[in]   ColSize  Number of columns in the matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNf_Copy (float              Result[],
                      const float        Operand[],
                      unsigned int        RowSize,
                      unsigned int        ColSize); 


/**
** \brief Initializes a MxM matrix of type float to the identity matrix.
**
** \warning
**    -# Calling function should ensure that RowSize equals ColSize.
**
** \param[out]  Result   Pointer to matrix containing an identity matrix
** \param[in]   RowSize  Number of rows in the matrix
** \param[in]   ColSize  Number of columns in the matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNf_InitIdentity (float              Result[],
                              unsigned int        RowSize,
                              unsigned int        ColSize);


/**
** \brief Initializes each element of a MxN matrix of type float to 0.0.
**        Result[m][n] = 0.0
**
** \param[out]  Result   Pointer to matrix containing a zero matrix
** \param[in]   RowSize  Number of rows in the matrix
** \param[in]   ColSize  Number of columns in the matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNf_InitZero (float              Result[],
                          unsigned int        RowSize,
                          unsigned int        ColSize);


/**
** \brief Multiply MxN matrix of type float by a NxM martrix of type float.
**        Result[m][n] = Left[m][n] * Right[n][m]
**
** \warning
**    -# Since the size of the matricies are unknown and since we do
**       not want to perform dynamic memory allocations, the Result 
**       can not be either the left or right.
**       (i.e., does not allow A = A*B or B = A*B)
**    -# LeftColSize must equal RightRowSize
**
** \param[out]  Result       Pointer to matrix containing the multiplication result
** \param[in]   Left         Pointer to left matrix operand
** \param[in]   LeftRowSize  Number of rows in the left matrix
** \param[in]   LeftColSize  Number of columns in the left matrix
** \param[in]   Right        Pointer to right matrix operand
** \param[in]   RightRowSize Number of rows in the right matrix
** \param[in]   RightColSize Number of columns in the right matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNf_Mult (float               Result[],
                      const float         Left[],
                      unsigned int         LeftRowSize,
                      unsigned int         LeftColSize,
                      const float         Right[],
                      unsigned int         RightRowSize,
                      unsigned int         RightColSize);



/**
** \brief Multiply a MxN matrix of type float by a scalar of type float.
**        Result[m][n] = Left[m][n] * Scalar
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   RowSize  Number of rows in the matrix
** \param[in]   ColSize  Number of columns in the matrix
** \param[in]   Scalar   Scalar value
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNf_MultScalar (float         Result[],
                            const float   Left[],
                            unsigned int   RowSize,
                            unsigned int   ColSize,
                            double         Scalar);


/**
** \brief Subtract two MxN matrices.
**        Result[m][n] = Left[m][n] - Right[m][n]
**
** \param[out]  Result   Pointer to matrix containing the subtraction result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   Right    Pointer to right matrix operand
** \param[in]   RowSize  Number of rows in the matrices
** \param[in]   ColSize  Number of columns in the matrices
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNf_Sub (float           Result[],
                     const float     Left[],
                     const float     Right[],
                     unsigned int     RowSize,
                     unsigned int     ColSize);


/**
** \brief Calculates the trace of a MxM  matrix  of type float.
**        Result = sum of the diagonal of Operand
**
** \warning
**    -# The trace can only be computed for a square matrix
**    -# The function requires both the row size and column size as
**       input argruments to be consistent with the other matrixMxN
**       functions.
**    -# The function does not check that rowsize = column size
**
** \param[out]  Operand  Pointer to matrix whose trace is computed
** \param[in]   RowSize  Number of rows in the matrices
** \param[in]   ColSize  Number of columns in the matrices
**
** \returns
** \retcode float \retdesc Trace of Matrix[m][m] \endcode
** \endreturns
*/
float  MatrixMxNf_Trace (const float      Operand[],
                          unsigned int      RowSize,
                          unsigned int      ColSize);


/**
** \brief Perform a matrix transposition on a MxN matrix of type float.
**        Result = Transpose of Operand
**
** \warning
**    -# Since the size of the matricies are unknown and since we do
**       not want to perform dynamic memory allocations, the Result 
**       can not be the same as the input matrix
**       (i.e., does not allow A = A**T)
**
** \param[out]  Result   Pointer to matrix containing the transpose of \c Operand
** \param[in]   Operand  Pointer to matrix to be transposed
** \param[in]   RowSize  Number of rows in the matrices
** \param[in]   ColSize  Number of columns in the matrices
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNf_Transpose (float            Result[],
                           const float      Operand[],
                           unsigned int      RowSize,
                           unsigned int      ColSize);



/************************ Exported Function prototypes ***************************/


/**
** \brief Add two MxN  matrices of type double.
**        Result[m][n] = Left[m][n] + Right[m][n]
**
** \param[out]  Result   Pointer to matrix containing the addition result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   Right    Pointer to right matrix operand
** \param[in]   RowSize  Number of rows in the matrices
** \param[in]   ColSize  Number of columns in the matrices
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNd_Add (double              Result[],
                     const double        Left[],
                     const double        Right[],
                     unsigned int        RowSize,
                     unsigned int        ColSize);


/**
** \brief Copy MxN matrix of type double to another MxN matrix.
**        Result[m][n] = Operand[m][n]
**
** \param[out]  Result   Pointer to matrix containing a copy of \c Operand
** \param[in]   Operand  Pointer to source matrix
** \param[in]   RowSize  Number of rows in the matrix
** \param[in]   ColSize  Number of columns in the matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNd_Copy (double              Result[],
                      const double        Operand[],
                      unsigned int        RowSize,
                      unsigned int        ColSize); 


/**
** \brief Initializes a MxM matrix of type double to the identity matrix.
**
** \warning
**    -# Calling function should ensure that RowSize equals ColSize.
**
** \param[out]  Result   Pointer to matrix containing an identity matrix
** \param[in]   RowSize  Number of rows in the matrix
** \param[in]   ColSize  Number of columns in the matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNd_InitIdentity (double              Result[],
                              unsigned int        RowSize,
                              unsigned int        ColSize);


/**
** \brief Initializes each element of a MxN matrix of type double to 0.0.
**        Result[m][n] = 0.0
**
** \param[out]  Result   Pointer to matrix containing a zero matrix
** \param[in]   RowSize  Number of rows in the matrix
** \param[in]   ColSize  Number of columns in the matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNd_InitZero (double              Result[],
                          unsigned int        RowSize,
                          unsigned int        ColSize);


/**
** \brief Multiply MxN matrix of type double by a NxM martrix of type double.
**        Result[m][n] = Left[m][n] * Right[n][m]
**
** \warning
**    -# Since the size of the matricies are unknown and since we do
**       not want to perform dynamic memory allocations, the Result 
**       can not be either the left or right.
**       (i.e., does not allow A = A*B or B = A*B)
**    -# LeftColSize must equal RightRowSize
**
** \param[out]  Result       Pointer to matrix containing the multiplication result
** \param[in]   Left         Pointer to left matrix operand
** \param[in]   LeftRowSize  Number of rows in the left matrix
** \param[in]   LeftColSize  Number of columns in the left matrix
** \param[in]   Right        Pointer to right matrix operand
** \param[in]   RightRowSize Number of rows in the right matrix
** \param[in]   RightColSize Number of columns in the right matrix
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNd_Mult (double               Result[],
                      const double         Left[],
                      unsigned int         LeftRowSize,
                      unsigned int         LeftColSize,
                      const double         Right[],
                      unsigned int         RightRowSize,
                      unsigned int         RightColSize);



/**
** \brief Multiply a MxN matrix of type double by a scalar of type double.
**        Result[m][n] = Left[m][n] * Scalar
**
** \param[out]  Result   Pointer to matrix containing the result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   RowSize  Number of rows in the matrix
** \param[in]   ColSize  Number of columns in the matrix
** \param[in]   Scalar   Scalar value
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNd_MultScalar (double         Result[],
                            const double   Left[],
                            unsigned int   RowSize,
                            unsigned int   ColSize,
                            double         Scalar);


/**
** \brief Subtract two MxN matrices.
**        Result[m][n] = Left[m][n] - Right[m][n]
**
** \param[out]  Result   Pointer to matrix containing the subtraction result
** \param[in]   Left     Pointer to left matrix operand
** \param[in]   Right    Pointer to right matrix operand
** \param[in]   RowSize  Number of rows in the matrices
** \param[in]   ColSize  Number of columns in the matrices
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNd_Sub (double           Result[],
                     const double     Left[],
                     const double     Right[],
                     unsigned int     RowSize,
                     unsigned int     ColSize);


/**
** \brief Calculates the trace of a MxM  matrix  of type double.
**        Result = sum of the diagonal of Operand
**
** \warning
**    -# The trace can only be computed for a square matrix
**    -# The function requires both the row size and column size as
**       input argruments to be consistent with the other matrixMxN
**       functions.
**    -# The function does not check that rowsize = column size
**
** \param[out]  Operand  Pointer to matrix whose trace is computed
** \param[in]   RowSize  Number of rows in the matrices
** \param[in]   ColSize  Number of columns in the matrices
**
** \returns
** \retcode double \retdesc Trace of Matrix[m][m] \endcode
** \endreturns
*/
double  MatrixMxNd_Trace (const double      Operand[],
                          unsigned int      RowSize,
                          unsigned int      ColSize);


/**
** \brief Perform a matrix transposition on a MxN matrix of type double.
**        Result = Transpose of Operand
**
** \warning
**    -# Since the size of the matricies are unknown and since we do
**       not want to perform dynamic memory allocations, the Result 
**       can not be the same as the input matrix
**       (i.e., does not allow A = A**T)
**
** \param[out]  Result   Pointer to matrix containing the transpose of \c Operand
** \param[in]   Operand  Pointer to matrix to be transposed
** \param[in]   RowSize  Number of rows in the matrices
** \param[in]   ColSize  Number of columns in the matrices
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void MatrixMxNd_Transpose (double            Result[],
                           const double      Operand[],
                           unsigned int      RowSize,
                           unsigned int      ColSize);


#endif /* _matrixMxN_h */
/** @} */
