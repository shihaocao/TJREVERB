/** \file
** 
** \brief Define a 4x1 vector of float snd functions to operate on 4x1 vectors.
** 
** $Id: vector4f.h 1.1 2008/05/21 15:00:24EDT dcmccomas Exp  $
**
** \note
**  -# Vectors are implemented using a structure containing an array to
**     allow type checking.
**  -# Adequate stack space is a precondition.
**  -# The calling function is responsible for ensuring that the calling
**     arguments are consistent with the requested feature.
**  -# The called functions do not return any error indicators
**  -# All of the functions allow the result to be one of the input
**     variables
**
** References:
**  -# Math Library Specifications
**
**  $Date: 2008/05/21 15:00:24EDT $
**
**  $Revision: 1.1 $
**
**  $Log: vector4f.h  $
**  Revision 1.1 2008/05/21 15:00:24EDT dcmccomas 
**  Initial revision
**  Revision 1.2 2005/11/29 07:49:54EST dcmccomas 
**  New SDO delivery and added doxygen markup
**  Revision 1.4  2004/02/24 16:26:45  rbjork
**  Updated prototypes to have one argument per line
**
**  Revision 1.3  2004/02/04 20:32:44  rbjork
**  Updates resulting from vector code walkthrough
**
**  Revision 1.2  2004/02/04 16:57:56  daviddawsonuser
**  replaced word files with actual files
**
**
*/

/** 
** @addtogroup math_vector
** @{
*/

#ifndef _vector4_h_
#define _vector4_h_

/*************************** Type definitions     **************************/

typedef struct 
{
  float      Comp[4];
} Vector4f;


/********************** Exported Function prototypes *************************/

/**
** \brief Add two 4x1 Vectors of type float.
**        Result[4] = Left[4] + Right[4]
**
** \param[out]  Result  Pointer to vector containing the addition result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4f_Add (Vector4f       *Result,
                   const Vector4f *Left,
                   const Vector4f *Right);

/**
** \brief Copy 4x1 vector of type float into another 4x1 vector.
**        Result[4] = Operand[4]
**
** \param[out]  Result   Pointer to vector containing a copy of the \c Operand
** \param[in]   Operand  Pointer to source vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4f_Copy (Vector4f       *Result,
                    const Vector4f *Operand);


/**
** \brief Divide each element of a 4x1 vector of type float by a scalar.
**        Result[4] = Left[4] / Scalar
**
** \warning
**    Calling program must ensure that scalar is non-zero.
**
** \param[out]  Result  Pointer to vector containing the result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Scalar  Value of the scalar operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4f_DivScalar (Vector4f       *Result,
                         const Vector4f *Left,
                         double Scalar);


/**
** \brief Compute inner product (dot product) of two 4x1 vectors of type float.
**        Result = Left[4] .dot. Right[4]
**
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode float \retdesc Scalar dot product \endcode
** \endreturns
*/
float Vector4f_Dot(const Vector4f *Left,
                    const Vector4f *Right);


/**
** \brief Sets a 4x1 Vector of type float to zero
**
** \param[out]  Result  Pointer to vector initialized to (0,0,0,0)
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4f_InitZero(Vector4f *Result);



/**
** \brief Compute the magnitude of a 4x1 vector of type float.
**   
** \param[in]   Operand  Pointer to vector whose magnitude is computed
**
** \returns
** \retcode float \retdesc Vector magnitude \endcode
** \endreturns
*/
float Vector4f_Magnitude (const Vector4f *Operand);



/**
** \brief Multiply each element of a 4x1 vector of type float by a scalar.
**        Result[4] = Left[4] * Scalar
**
** \param[out]  Result  Pointer to vector containing the result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Scalar  Value of scalar operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4f_MultScalar (Vector4f       *Result,
                          const Vector4f *Left,
                          double Scalar);


/**
** \brief Normalize a 4x1 vector of type float.
**   
** \warning
**    -  The calling module must ensure that the magnitude of the vector is 
**       non-zero
**
** \param[out]  Result  Pointer to vector containing the result
** \param[in]   Operand Pointer to vector to be normalized
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4f_Normalize (Vector4f       *Result, /**< Pointer to storage for the result  */
                         const Vector4f *Operand /**< Pointer to storage for the operand */
                         );



/**
** \brief Subtract two 4x1 Vectors of type float.
**        Result[4] = Left[4] - Right[4]
**
** \param[out]  Result  Pointer to vector containing the subtraction result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4f_Sub (Vector4f       *Result, /**< Pointer to storage for the result of the operation */
                   const Vector4f *Left,   /**< Pointer to storage for the left operand            */
                   const Vector4f *Right   /**< Pointer to storage for the right operand           */
                   );


/*************************** Type definitions     **************************/

typedef struct 
{
  double      Comp[4];
} Vector4d;


/********************** Exported Function prototypes *************************/

/**
** \brief Add two 4x1 Vectors of type double.
**        Result[4] = Left[4] + Right[4]
**
** \param[out]  Result  Pointer to vector containing the addition result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4d_Add (Vector4d       *Result,
                   const Vector4d *Left,
                   const Vector4d *Right);

/**
** \brief Copy 4x1 vector of type double into another 4x1 vector.
**        Result[4] = Operand[4]
**
** \param[out]  Result   Pointer to vector containing a copy of the \c Operand
** \param[in]   Operand  Pointer to source vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4d_Copy (Vector4d       *Result,
                    const Vector4d *Operand);


/**
** \brief Divide each element of a 4x1 vector of type double by a scalar.
**        Result[4] = Left[4] / Scalar
**
** \warning
**    Calling program must ensure that scalar is non-zero.
**
** \param[out]  Result  Pointer to vector containing the result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Scalar  Value of the scalar operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4d_DivScalar (Vector4d       *Result,
                         const Vector4d *Left,
                         double Scalar);


/**
** \brief Compute inner product (dot product) of two 4x1 vectors of type double.
**        Result = Left[4] .dot. Right[4]
**
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode double \retdesc Scalar dot product \endcode
** \endreturns
*/
double Vector4d_Dot(const Vector4d *Left,
                    const Vector4d *Right);


/**
** \brief Sets a 4x1 Vector of type double to zero
**
** \param[out]  Result  Pointer to vector initialized to (0,0,0,0)
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4d_InitZero(Vector4d *Result);



/**
** \brief Compute the magnitude of a 4x1 vector of type double.
**   
** \param[in]   Operand  Pointer to vector whose magnitude is computed
**
** \returns
** \retcode double \retdesc Vector magnitude \endcode
** \endreturns
*/
double Vector4d_Magnitude (const Vector4d *Operand);



/**
** \brief Multiply each element of a 4x1 vector of type double by a scalar.
**        Result[4] = Left[4] * Scalar
**
** \param[out]  Result  Pointer to vector containing the result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Scalar  Value of scalar operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4d_MultScalar (Vector4d       *Result,
                          const Vector4d *Left,
                          double Scalar);


/**
** \brief Normalize a 4x1 vector of type double.
**   
** \warning
**    -  The calling module must ensure that the magnitude of the vector is 
**       non-zero
**
** \param[out]  Result  Pointer to vector containing the result
** \param[in]   Operand Pointer to vector to be normalized
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4d_Normalize (Vector4d       *Result, /**< Pointer to storage for the result  */
                         const Vector4d *Operand /**< Pointer to storage for the operand */
                         );



/**
** \brief Subtract two 4x1 Vectors of type double.
**        Result[4] = Left[4] - Right[4]
**
** \param[out]  Result  Pointer to vector containing the subtraction result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector4d_Sub (Vector4d       *Result, /**< Pointer to storage for the result of the operation */
                   const Vector4d *Left,   /**< Pointer to storage for the left operand            */
                   const Vector4d *Right   /**< Pointer to storage for the right operand           */
                   );


#endif /* _vector4_h_ */
/** @} */
/****************** end of file vec4d.h ********************************/
