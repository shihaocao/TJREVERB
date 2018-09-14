/** \file
** 
** \brief Define a 3x1 vector of double snd functions to operate on 3x1 vectors.
** 
** $Id: vector3.h 1.1 2008/05/21 15:00:24EDT dcmccomas Exp  $
**
** \note
**   -# Vectors are implemented using a structure containing an array to
**      allow type checking.
**   -# Adequate stack space is a precondition.
**   -# The calling function is responsible for ensuring that the calling
**      arguments are consistent with the requested feature.
**   -# The called functions do not return any error indicators
**   -# All of the functions allow the result to be one of the input
**      variables
**
** References:
**   -# Math Library Specifications
**
**  $Date: 2008/05/21 15:00:24EDT $
**  $Revision: 1.1 $
**  $Log: vector3.h  $
**  Revision 1.1 2008/05/21 15:00:24EDT dcmccomas 
**  Initial revision
**  Revision 1.2 2005/11/29 07:49:53EST dcmccomas 
**  New SDO delivery and added doxygen markup
**  Revision 1.4.8.1  2005/11/17 04:03:56  ddawson
**  Modified comments for doxygen
**
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


#ifndef _vector3_h_
#define _vector3_h_

/*************************** Type definitions     **************************/

typedef struct 
{
  float      Comp[3];
} Vector3f;



/********************** Exported Function prototypes *************************/

/**
** \brief Add two 3x1 Vectors of type double
**        Result[3] = Left[3] + Right[3].
**
** \param[out]  Result  Pointer to vector containing the addition result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector3f_Add (Vector3f       *Result,
                   const Vector3f *Left,
                   const Vector3f *Right);



/**
** \brief Copy 3x1 vector of type double into another 3x1 vector.
**        Result[3] = Operand[3]
**
** \param[out]  Result   Pointer to vector containing a copy of the \c Operand
** \param[in]   Operand  Pointer to source vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector3f_Copy (Vector3f       *Result,
                    const Vector3f *Operand);



/**
** \brief Compute the cross product of two 3x1 vectors of type double.
**        Result[3] = Left[3] cross Right[3]
**
** \note
**   -#  The  operation is not commutative so the order of
**       the Left and Right operands must correspond to the caller's
**       intended order.
**
** \param[out]  Result  Pointer to vector containing the cross product result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector3f_Cross (Vector3f       *Result,
                     const Vector3f *Left,
                     const Vector3f *Right);



/**
** \brief Divide each element of a 3x1 vector of type double by a scalar.
**        Result[3] = Left[3] / Scalar
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
void Vector3f_DivScalar (Vector3f       *Result,
                         const Vector3f *Left,
                         float          Scalar);


/**
** \brief Compute inner product (dot product) of two 3x1 vectors of type double.
**        Result = Left[3] .dot. Right[3]
**
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode double \retdesc Scalar dot product \endcode
** \endreturns
*/
float Vector3f_Dot(const Vector3f *Left,
                    const Vector3f *Right);



/**
** \brief Constructs the angular 3x1 vector of type double from a given right ascension
**        and declination.
**   
** \note
**    -# The Right Acsension and Declination are in radians
**
** \param[out]  Result  Pointer to vector containing the result
** \param[in]   Ra      Right Ascension in radians
** \param[in]   Dec     Declination in radians
**
** \returns
** \retcode void \endcode
** \endreturns
*/

void Vector3f_FromRaDec (Vector3f *Result,
                         double    Ra,
                         double    Dec);



/**
** \brief Sets a 3x1 Vector of type double to zero
**
** \param[out]  Result  Pointer to vector initialized to (0,0,0)
**
** \returns
** \retcode void \endcode
** \endreturns
*/

void Vector3f_InitZero(Vector3f *Result);


/**
** \brief Compute the magnitude of a 3x1 vector of type double.
**   
** \param[in]   Operand  Pointer to vector whose magnitude is computed
**
** \returns
** \retcode double \retdesc Vector magnitude \endcode
** \endreturns
*/

float Vector3f_Magnitude (const Vector3f *Operand);

/**
** \brief Multiply each element of a 3x1 vector of type double by a scalar.
**        Result[3] = Left[3] * Scalar
**
** \param[out]  Result  Pointer to vector containing the result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Scalar  Value of scalar operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector3f_MultScalar (Vector3f       *Result,
                          const Vector3f *Left,
                          double          Scalar);



/**
** \brief Normalize a 3x1 vector of type double.
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
void Vector3f_Normalize (Vector3f       *Result,
                         const Vector3f *Operand);



/**
** \brief Subtract two 3x1 Vectors of type double.
**        Result[3] = Left[3] - Right[3]
**
** \param[out]  Result  Pointer to vector containing the subtraction result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector3f_Sub (Vector3f       *Result,
                   const Vector3f *Left,
                   const Vector3f *Right
                   );


void Vector3f_Abs(Vector3f *Result, Vector3f *Operand);
float Vector3f_Max(Vector3f *array);


/*************************** Type definitions     **************************/

typedef struct 
{
  double      Comp[3];
} Vector3d;



/********************** Exported Function prototypes *************************/

/**
** \brief Add two 3x1 Vectors of type double
**        Result[3] = Left[3] + Right[3].
**
** \param[out]  Result  Pointer to vector containing the addition result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector3d_Add (Vector3d       *Result,
                   const Vector3d *Left,
                   const Vector3d *Right);



/**
** \brief Copy 3x1 vector of type double into another 3x1 vector.
**        Result[3] = Operand[3]
**
** \param[out]  Result   Pointer to vector containing a copy of the \c Operand
** \param[in]   Operand  Pointer to source vector
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector3d_Copy (Vector3d       *Result,
                    const Vector3d *Operand);



/**
** \brief Compute the cross product of two 3x1 vectors of type double.
**        Result[3] = Left[3] cross Right[3]
**
** \note
**   -#  The  operation is not commutative so the order of
**       the Left and Right operands must correspond to the caller's
**       intended order.
**
** \param[out]  Result  Pointer to vector containing the cross product result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector3d_Cross (Vector3d       *Result,
                     const Vector3d *Left,
                     const Vector3d *Right);



/**
** \brief Divide each element of a 3x1 vector of type double by a scalar.
**        Result[3] = Left[3] / Scalar
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
void Vector3d_DivScalar (Vector3d       *Result,
                         const Vector3d *Left,
                         double          Scalar);


/**
** \brief Compute inner product (dot product) of two 3x1 vectors of type double.
**        Result = Left[3] .dot. Right[3]
**
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode double \retdesc Scalar dot product \endcode
** \endreturns
*/
double Vector3d_Dot(const Vector3d *Left,
                    const Vector3d *Right);



/**
** \brief Constructs the angular 3x1 vector of type double from a given right ascension
**        and declination.
**   
** \note
**    -# The Right Acsension and Declination are in radians
**
** \param[out]  Result  Pointer to vector containing the result
** \param[in]   Ra      Right Ascension in radians
** \param[in]   Dec     Declination in radians
**
** \returns
** \retcode void \endcode
** \endreturns
*/

void Vector3d_FromRaDec (Vector3d *Result,
                         double    Ra,
                         double    Dec);



/**
** \brief Sets a 3x1 Vector of type double to zero
**
** \param[out]  Result  Pointer to vector initialized to (0,0,0)
**
** \returns
** \retcode void \endcode
** \endreturns
*/

void Vector3d_InitZero(Vector3d *Result);


/**
** \brief Compute the magnitude of a 3x1 vector of type double.
**   
** \param[in]   Operand  Pointer to vector whose magnitude is computed
**
** \returns
** \retcode double \retdesc Vector magnitude \endcode
** \endreturns
*/

double Vector3d_Magnitude (const Vector3d *Operand);



/**
** \brief Multiply each element of a 3x1 vector of type double by a scalar.
**        Result[3] = Left[3] * Scalar
**
** \param[out]  Result  Pointer to vector containing the result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Scalar  Value of scalar operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector3d_MultScalar (Vector3d       *Result,
                          const Vector3d *Left,
                          double          Scalar);



/**
** \brief Normalize a 3x1 vector of type double.
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
void Vector3d_Normalize (Vector3d       *Result,
                         const Vector3d *Operand);



/**
** \brief Subtract two 3x1 Vectors of type double.
**        Result[3] = Left[3] - Right[3]
**
** \param[out]  Result  Pointer to vector containing the subtraction result
** \param[in]   Left    Pointer to left vector operand
** \param[in]   Right   Pointer to right vector operand
**
** \returns
** \retcode void \endcode
** \endreturns
*/
void Vector3d_Sub (Vector3d       *Result,
                   const Vector3d *Left,
                   const Vector3d *Right
                   );


void Vector3d_Abs(Vector3d *Result, Vector3d *Operand);
double Vector3d_Max(Vector3d *array);


#endif /* _vector3_h_ */
/** @} */
/****************** end of file vec3.h ********************************/
