/* 
** File:
**   $Id: matrix3x3d.c 1.2 2008/09/10 10:01:44EDT gwelter Exp  $
**
**  Purpose: Provide the implementation of the Matrix3x3d package. 
**
**  $Date: 2008/09/10 10:01:44EDT $
**
**  $Revision: 1.2 $
**
**  $Log: matrix3x3d.c  $
**  Revision 1.2 2008/09/10 10:01:44EDT gwelter 
**  pre-gpm updates
**  Revision 1.1  2004/01/23 13:22:59  daviddawsonuser
**  directory reorg
**  Revision 1.1  2004/10/04 18:23:35  ddawson
**  imported mathlib to SDO repository
**  Revision 1.2  2004/02/04 20:32:44  rbjork
**  Updates resulting from vector code walkthrough
**  Revision 1.2 2005/03/01 12:31:04EST ddawson 
**  Temporarily added analyst DCM from vector function.  Function will be removed
**  when new math library functions are added
**  Revision 1.2 2005/11/29 07:49:13EST dcmccomas 
**  New SDO delivery and added doxygen markup
**  Revision 1.3  2004/03/23 15:20:13  ddawson
**  removed DOS line endings
**  Revision 1.3 2005/04/01 18:19:02EST ddawson 
**  Added Matrix3x3d_RowVecToMatrix and Matrix3x3d_ColVecToMatrix
**  Revision 1.3.8.1  2005/11/17 03:41:20  ddawson
**  Imported SDO changes into main trunk and then merged with doxygen branch
**  Revision 1.4  2005/11/16 20:41:59  ddawson
**  Update from SDO, added new RowVecToMatrix and ColVecToMatrix functions
**
**  Revision 2.0 2008/05/21 15:00:26EDT dcmccomas 
**  Revision for new MKS configuration
**  Revision 2.0 2008/06/23 07:00:00EDT gwelter
**  add Matrix3x3d_Skew, Matrix3x3d_Invert, Matrix3x3d_Triad
**
*/
/*
** Include Files 
*/

#include "matrix3x3.h"
#include "matrixmxn.h"

/*
** Exported Functions
*/

/***********************************************************************************
**
** Function: Matrix3x3d_Add
**
** Notes:   None
*/

void Matrix3x3d_Add (Matrix3x3d *Result, const Matrix3x3d *Left, const Matrix3x3d *Right) 
{

   MatrixMxNd_Add(&Result->Comp[0][0], &Left->Comp[0][0],&Right->Comp[0][0],3,3);

} /* End Matrix3x3d_Add() */



/***********************************************************************************
**
** Function: Matrix3x3_ColVecToMatrix
**
** Notes:   
**     
*/
void Matrix3x3d_ColVecToMatrix (Matrix3x3d     *Result,
                                const Vector3d *First,
                                const Vector3d *Second,
                                const Vector3d *Third)
{
   int i;
   
   for (i = 0; i < 3; i++)
   {
      Result->Comp[i][0] = First->Comp[i];
      Result->Comp[i][1] = Second->Comp[i];
      Result->Comp[i][2] = Third->Comp[i];
   }

} /* End Matrix3x3d_ColVecToMatrix */



/*********************************************************************************
**
** Function: Matrix3x3d_Copy
**
** Notes:   None
*/

void Matrix3x3d_Copy (Matrix3x3d *Result, const Matrix3x3d *Operand) 
{

   MatrixMxNd_Copy(&Result->Comp[0][0], &Operand->Comp[0][0],3,3);

} /* End Matrix3x3d_Copy() */

/***********************************************************************************
**
** Function: Matrix3x3d_DivScalar
**
** Notes: None
*/

void Matrix3x3d_DivScalar (Matrix3x3d *Result, const Matrix3x3d *Left, double Scalar) 
{
   double ScalarInv;

   ScalarInv = 1.0 / Scalar;
   MatrixMxNd_MultScalar(&Result->Comp[0][0], &Left->Comp[0][0],3,3,ScalarInv);

} /* End Matrix3x3d_DivScalar() */

/*********************************************************************************
**
** Function: Matrix3x3d_InitIndentity
**
** Notes:   None
*/

void Matrix3x3d_InitIdentity (Matrix3x3d *Result) 
{

   MatrixMxNd_InitIdentity(&Result->Comp[0][0],3,3);

} /* End Matrix3x3d_InitIdentity() */

/*********************************************************************************
**
** Function: Matrix3x3d_InitZero
**
** Notes:   None
*/

void Matrix3x3d_InitZero (Matrix3x3d *Result) 
{

   MatrixMxNd_InitZero(&Result->Comp[0][0],3,3);

} /* End Matrix3x3d_InitZero() */
/****************************************************************************
** 
** Function: Matrix3x3d_Invert
**
*/
int Matrix3x3d_Invert( Matrix3x3d       *MatrixInverse,
                       const Matrix3x3d *Matrix         )
{

    int i, j;
    int InverseExists;

   double MatrixDeterminant;
	double MatrixDeterminantInverse;
	double Term[6];
	double MaxTerm, Ratio, Temp;

   Term[0] =   Matrix->Comp[0][0] * Matrix->Comp[1][1] * Matrix->Comp[2][2];
	Term[1] =   Matrix->Comp[0][1] * Matrix->Comp[1][2] * Matrix->Comp[2][0];
	Term[2] =   Matrix->Comp[0][2] * Matrix->Comp[1][0] * Matrix->Comp[2][1];
	Term[3] = - Matrix->Comp[0][2] * Matrix->Comp[1][1] * Matrix->Comp[2][0];
	Term[4] = - Matrix->Comp[0][1] * Matrix->Comp[1][0] * Matrix->Comp[2][2];
	Term[5] = - Matrix->Comp[0][0] * Matrix->Comp[1][2] * Matrix->Comp[2][1];

    MatrixDeterminant = 0.0;
	for ( i=0; i<6; i++ ) MatrixDeterminant += Term[i];

    MaxTerm = 0.0;
	for ( i=0; i<6; i++ ) { 
      Temp = fabs( Term[i] );
	  if ( MaxTerm < Temp ) MaxTerm = Temp;
	}

	if ( MaxTerm > 0.0 ) Ratio = fabs( MatrixDeterminant ) / MaxTerm;
	else                 Ratio = 0.0;

	/* This check is not just a protection against division by zero, 
	   but also protection against extreme loss of precision in the 
	   calculation of the determinant.  Given that IEEE double 
	   precision has ~ 18 decimal places of precision, then to have
	   at least 3 places of precision after large cancellation in 
	   the calculation of the determinant, the check is against 1.0e-15. */
    if ( Ratio < 1.0e-15 ) 
	{

      InverseExists = 0;  /* false, matrix is singular */

      for ( i=0; i<3; i++ ) {
	    for ( j=0; j<3; j++ ) MatrixInverse->Comp[i][j] = 0.0;
      }

    } 
	else 
	{

      InverseExists = 1;  /* true, matrix is invertable */

      MatrixDeterminantInverse = 1.0 / MatrixDeterminant; 

      MatrixInverse->Comp[0][0] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[1][1] * Matrix->Comp[2][2]
                                   - Matrix->Comp[1][2] * Matrix->Comp[2][1] );

      MatrixInverse->Comp[0][1] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[0][2] * Matrix->Comp[2][1]
                                   - Matrix->Comp[0][1] * Matrix->Comp[2][2] );

      MatrixInverse->Comp[0][2] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[0][1] * Matrix->Comp[1][2] 
                                   - Matrix->Comp[1][1] * Matrix->Comp[0][2] );


      MatrixInverse->Comp[1][0] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[2][0] * Matrix->Comp[1][2]
                                   - Matrix->Comp[1][0] * Matrix->Comp[2][2] );

      MatrixInverse->Comp[1][1] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[0][0] * Matrix->Comp[2][2]
                                   - Matrix->Comp[2][0] * Matrix->Comp[0][2] );

      MatrixInverse->Comp[1][2] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[1][0] * Matrix->Comp[0][2]
                                   - Matrix->Comp[0][0] * Matrix->Comp[1][2] );


      MatrixInverse->Comp[2][0] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[1][0] * Matrix->Comp[2][1]
                                   - Matrix->Comp[2][0] * Matrix->Comp[1][1] );

      MatrixInverse->Comp[2][1] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[2][0] * Matrix->Comp[0][1]
                                   - Matrix->Comp[0][0] * Matrix->Comp[2][1] );

      MatrixInverse->Comp[2][2] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[0][0] * Matrix->Comp[1][1]
                                   - Matrix->Comp[0][1] * Matrix->Comp[1][0] );

    }

    return InverseExists;

}

/********************************************************************************
**
** Function: Matrix3x3d_Mult
**
** Notes:  
**  1. Local variable Rslt allows the caller to set the result to
**     one of the input variables
*/

void Matrix3x3d_Mult (Matrix3x3d *Result, const Matrix3x3d *Left, const Matrix3x3d *Right) 
{
   Matrix3x3d Rslt;

   MatrixMxNd_Mult(&Rslt.Comp[0][0], &Left->Comp[0][0],3,3,&Right->Comp[0][0],3,3);
   MatrixMxNd_Copy(&Result->Comp[0][0],&Rslt.Comp[0][0], 3,3);

} /* End Matrix3x3d_Mult() */

/************************************************************************************
**
** Function: Matrix3x3d_MultScalar
**
** Notes: None
*/

void Matrix3x3d_MultScalar (Matrix3x3d *Result, const Matrix3x3d *Left, double Scalar) 
{

   MatrixMxNd_MultScalar(&Result->Comp[0][0], &Left->Comp[0][0],3,3,Scalar);

} /* End Matrix3x3d_MultScalar() */



/***********************************************************************************
**
** Function: Matrix3x3dMultVec
**
** Notes:   
**  1. Local variable Rslt allows the caller to set the result to be the
**     same variable as the input vector.
*/

void Matrix3x3d_MultVec (Vector3d *Result, const Matrix3x3d *Left, const Vector3d *Right) 
{
   Vector3d    Rslt;

   MatrixMxNd_Mult(&Rslt.Comp[0], &Left->Comp[0][0],3,3,&Right->Comp[0],3,1);
   MatrixMxNd_Copy(&Result->Comp[0],&Rslt.Comp[0], 3,1);
 

} /* End Matrix3x3d_MultVec() */



/***********************************************************************************
**
** Function: Matrix3x3_RowVecToMatrix
**
** Notes:   
**     
*/
void Matrix3x3d_RowVecToMatrix (Matrix3x3d     *Result,
                                const Vector3d *First,
                                const Vector3d *Second,
                                const Vector3d *Third)
{
   int i;
   
   for (i = 0; i < 3; i++)
   {
      Result->Comp[0][i] = First->Comp[i];
      Result->Comp[1][i] = Second->Comp[i];
      Result->Comp[2][i] = Third->Comp[i];
   }

} /* End Matrix3x3d_RowVecToMatrix */

/****************************************************************************
** 
** Function: Matrix3x3d_Skew
**
*/
void Matrix3x3d_Skew( Matrix3x3d     *Skew,
                      const Vector3d *Vector ) 
{

    Skew->Comp[0][0] =    0.0; 
    Skew->Comp[1][1] =    0.0;
    Skew->Comp[2][2] =    0.0;
	
    Skew->Comp[1][0] = - Vector->Comp[2];
    Skew->Comp[0][1] =   Vector->Comp[2];

    Skew->Comp[2][0] =   Vector->Comp[1];
    Skew->Comp[0][2] = - Vector->Comp[1];

    Skew->Comp[2][1] = - Vector->Comp[0];
    Skew->Comp[1][2] =   Vector->Comp[0];

}

/***********************************************************************************
**
** Function: Matrix3x3d_Sub
**
** Notes:   None
*/

void Matrix3x3d_Sub (Matrix3x3d *Result, const Matrix3x3d *Left, const Matrix3x3d *Right) 
{

   MatrixMxNd_Sub(&Result->Comp[0][0], &Left->Comp[0][0],&Right->Comp[0][0],3,3);

} /* End Matrix3x3d_Sub() */


/************************************************************************************
**
** Function: Matrix3x3d_Trace
**
** Notes: None  
*/

double Matrix3x3d_Trace (const Matrix3x3d *Operand) 
{
   double Trace;

   Trace=MatrixMxNd_Trace(&Operand->Comp[0][0],3,3);
   
   return (Trace);

} /* End Matrix3x3d_Trace() */

/************************************************************************************
**
** Function: Matrix3x3d_Transpose
**
** Notes:   
**  1. Local variable Rslt allows the caller to set A = Transpose of A
*/

void Matrix3x3d_Transpose (Matrix3x3d *Result, const Matrix3x3d *Operand) 
{
   Matrix3x3d Rslt;

   MatrixMxNd_Transpose(&Rslt.Comp[0][0], &Operand->Comp[0][0],3,3);
   MatrixMxNd_Copy(&Result->Comp[0][0],&Rslt.Comp[0][0], 3,3);

} /* End Matrix3x3d_Transpose() */


/****************************************************************************
** 
** Function: Matrix3x3d_Triad
**
*/
int Matrix3x3d_Triad( Matrix3x3d      *AttDcm,
                      const Vector3d  *Ur,
                      const Vector3d  *Vr,
                      const Vector3d  *Ub,
                      const Vector3d  *Vb  )
{

    int    i;

    double RMag, RMagInv;

    Vector3d R, S;

    Matrix3x3d Mb, Mr_t;

    int Valid;

    Valid = 1;  /* true; we assume rotation matrix will be found */


    /*  GCI vectors  */
    for ( i=0; i<3; i++ ) Mr_t.Comp[0][i] = Ur->Comp[i];

    Vector3d_Cross( &R, Ur, Vr );

    RMag = 0.0;
    for ( i=0; i<3; i++ ) RMag += R.Comp[i] * R.Comp[i];
    if ( RMag > 0.0 ) {
      RMagInv = 1.0 / sqrt( RMag );
      for ( i=0; i<3; i++ ) R.Comp[i] *= RMagInv;
    } else {
      Valid = 0;  /* false, no rotation matrix */
    }

    for ( i=0; i<3; i++ ) Mr_t.Comp[1][i] = R.Comp[i];

    Vector3d_Cross( &S, Ur, &R );

    for ( i=0; i<3; i++ ) Mr_t.Comp[2][i] = S.Comp[i];


    /*  body frame vectors   */
    for ( i=0; i<3; i++ ) Mb.Comp[i][0] = Ub->Comp[i];

    Vector3d_Cross( &R, Ub, Vb );

    RMag = 0.0;
    for ( i=0; i<3; i++ ) RMag += R.Comp[i] * R.Comp[i];
    if ( RMag > 0.0 ) {
      RMagInv = 1.0 / sqrt( RMag );
      for ( i=0; i<3; i++ ) R.Comp[i] *= RMagInv;
    } else {
      Valid = 0;  /* false, no rotation matrix */
    }
    
    for ( i=0; i<3; i++ ) Mb.Comp[i][1] = R.Comp[i];

    Vector3d_Cross( &S, Ub, &R );

    for ( i=0; i<3; i++ ) Mb.Comp[i][2] = S.Comp[i];


    /*  Direction Cosine Matrix */
    Matrix3x3d_Mult( AttDcm, &Mb, &Mr_t );

    return Valid;

}


/*
** Exported Functions
*/

/***********************************************************************************
**
** Function: Matrix3x3f_Add
**
** Notes:   None
*/

void Matrix3x3f_Add (Matrix3x3f *Result, const Matrix3x3f *Left, const Matrix3x3f *Right) 
{

   MatrixMxNf_Add(&Result->Comp[0][0], &Left->Comp[0][0],&Right->Comp[0][0],3,3);

} /* End Matrix3x3f_Add() */



/***********************************************************************************
**
** Function: Matrix3x3_ColVecToMatrix
**
** Notes:   
**     
*/
void Matrix3x3f_ColVecToMatrix (Matrix3x3f     *Result,
                                const Vector3f *First,
                                const Vector3f *Second,
                                const Vector3f *Third)
{
   int i;
   
   for (i = 0; i < 3; i++)
   {
      Result->Comp[i][0] = First->Comp[i];
      Result->Comp[i][1] = Second->Comp[i];
      Result->Comp[i][2] = Third->Comp[i];
   }

} /* End Matrix3x3f_ColVecToMatrix */



/*********************************************************************************
**
** Function: Matrix3x3f_Copy
**
** Notes:   None
*/

void Matrix3x3f_Copy (Matrix3x3f *Result, const Matrix3x3f *Operand) 
{

   MatrixMxNf_Copy(&Result->Comp[0][0], &Operand->Comp[0][0],3,3);

} /* End Matrix3x3f_Copy() */

/***********************************************************************************
**
** Function: Matrix3x3f_DivScalar
**
** Notes: None
*/

void Matrix3x3f_DivScalar (Matrix3x3f *Result, const Matrix3x3f *Left, double Scalar) 
{
   double ScalarInv;

   ScalarInv = 1.0 / Scalar;
   MatrixMxNf_MultScalar(&Result->Comp[0][0], &Left->Comp[0][0],3,3,ScalarInv);

} /* End Matrix3x3f_DivScalar() */

/*********************************************************************************
**
** Function: Matrix3x3f_InitIndentity
**
** Notes:   None
*/

void Matrix3x3f_InitIdentity (Matrix3x3f *Result) 
{

   MatrixMxNf_InitIdentity(&Result->Comp[0][0],3,3);

} /* End Matrix3x3f_InitIdentity() */

/*********************************************************************************
**
** Function: Matrix3x3f_InitZero
**
** Notes:   None
*/

void Matrix3x3f_InitZero (Matrix3x3f *Result) 
{

   MatrixMxNf_InitZero(&Result->Comp[0][0],3,3);

} /* End Matrix3x3f_InitZero() */
/****************************************************************************
** 
** Function: Matrix3x3f_Invert
**
*/
int Matrix3x3f_Invert( Matrix3x3f       *MatrixInverse,
                       const Matrix3x3f *Matrix         )
{

    int i, j;
    int InverseExists;

    float MatrixDeterminant;
	float MatrixDeterminantInverse;
	float Term[6];
	float MaxTerm, Ratio, Temp;

    Term[0] =   Matrix->Comp[0][0] * Matrix->Comp[1][1] * Matrix->Comp[2][2];
	Term[1] =   Matrix->Comp[0][1] * Matrix->Comp[1][2] * Matrix->Comp[2][0];
	Term[2] =   Matrix->Comp[0][2] * Matrix->Comp[1][0] * Matrix->Comp[2][1];
	Term[3] = - Matrix->Comp[0][2] * Matrix->Comp[1][1] * Matrix->Comp[2][0];
	Term[4] = - Matrix->Comp[0][1] * Matrix->Comp[1][0] * Matrix->Comp[2][2];
	Term[5] = - Matrix->Comp[0][0] * Matrix->Comp[1][2] * Matrix->Comp[2][1];

    MatrixDeterminant = 0.0;
	for ( i=0; i<6; i++ ) MatrixDeterminant += Term[i];

    MaxTerm = 0.0;
	for ( i=0; i<6; i++ ) { 
      Temp = fabs( Term[i] );
	  if ( MaxTerm < Temp ) MaxTerm = Temp;
	}

	if ( MaxTerm > 0.0 ) Ratio = fabs( MatrixDeterminant ) / MaxTerm;
	else                 Ratio = 0.0;

	/* This check is not just a protection against division by zero, 
	   but also protection against extreme loss of precision in the 
	   calculation of the determinant.  Given that IEEE float 
	   precision has ~ 18 decimal places of precision, then to have
	   at least 3 places of precision after large cancellation in 
	   the calculation of the determinant, the check is against 1.0e-15. */
    if ( Ratio < 1.0e-15 ) 
	{

      InverseExists = 0;  /* false, matrix is singular */

      for ( i=0; i<3; i++ ) {
	    for ( j=0; j<3; j++ ) MatrixInverse->Comp[i][j] = 0.0;
      }

    } 
	else 
	{

      InverseExists = 1;  /* true, matrix is invertable */

      MatrixDeterminantInverse = 1.0 / MatrixDeterminant; 

      MatrixInverse->Comp[0][0] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[1][1] * Matrix->Comp[2][2]
                                   - Matrix->Comp[1][2] * Matrix->Comp[2][1] );

      MatrixInverse->Comp[0][1] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[0][2] * Matrix->Comp[2][1]
                                   - Matrix->Comp[0][1] * Matrix->Comp[2][2] );

      MatrixInverse->Comp[0][2] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[0][1] * Matrix->Comp[1][2] 
                                   - Matrix->Comp[1][1] * Matrix->Comp[0][2] );


      MatrixInverse->Comp[1][0] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[2][0] * Matrix->Comp[1][2]
                                   - Matrix->Comp[1][0] * Matrix->Comp[2][2] );

      MatrixInverse->Comp[1][1] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[0][0] * Matrix->Comp[2][2]
                                   - Matrix->Comp[2][0] * Matrix->Comp[0][2] );

      MatrixInverse->Comp[1][2] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[1][0] * Matrix->Comp[0][2]
                                   - Matrix->Comp[0][0] * Matrix->Comp[1][2] );


      MatrixInverse->Comp[2][0] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[1][0] * Matrix->Comp[2][1]
                                   - Matrix->Comp[2][0] * Matrix->Comp[1][1] );

      MatrixInverse->Comp[2][1] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[2][0] * Matrix->Comp[0][1]
                                   - Matrix->Comp[0][0] * Matrix->Comp[2][1] );

      MatrixInverse->Comp[2][2] = MatrixDeterminantInverse *
		                          (  Matrix->Comp[0][0] * Matrix->Comp[1][1]
                                   - Matrix->Comp[0][1] * Matrix->Comp[1][0] );

    }

    return InverseExists;

}

/********************************************************************************
**
** Function: Matrix3x3f_Mult
**
** Notes:  
**  1. Local variable Rslt allows the caller to set the result to
**     one of the input variables
*/

void Matrix3x3f_Mult (Matrix3x3f *Result, const Matrix3x3f *Left, const Matrix3x3f *Right) 
{
   Matrix3x3f Rslt;

   MatrixMxNf_Mult(&Rslt.Comp[0][0], &Left->Comp[0][0],3,3,&Right->Comp[0][0],3,3);
   MatrixMxNf_Copy(&Result->Comp[0][0],&Rslt.Comp[0][0], 3,3);

} /* End Matrix3x3f_Mult() */

/************************************************************************************
**
** Function: Matrix3x3f_MultScalar
**
** Notes: None
*/

void Matrix3x3f_MultScalar (Matrix3x3f *Result, const Matrix3x3f *Left, double Scalar) 
{

   MatrixMxNf_MultScalar(&Result->Comp[0][0], &Left->Comp[0][0],3,3,Scalar);

} /* End Matrix3x3f_MultScalar() */



/***********************************************************************************
**
** Function: Matrix3x3fMultVec
**
** Notes:   
**  1. Local variable Rslt allows the caller to set the result to be the
**     same variable as the input vector.
*/

void Matrix3x3f_MultVec (Vector3f *Result, const Matrix3x3f *Left, const Vector3f *Right) 
{
   Vector3f    Rslt;

   MatrixMxNf_Mult(&Rslt.Comp[0], &Left->Comp[0][0],3,3,&Right->Comp[0],3,1);
   MatrixMxNf_Copy(&Result->Comp[0],&Rslt.Comp[0], 3,1);
 

} /* End Matrix3x3f_MultVec() */



/***********************************************************************************
**
** Function: Matrix3x3_RowVecToMatrix
**
** Notes:   
**     
*/
void Matrix3x3f_RowVecToMatrix (Matrix3x3f     *Result,
                                const Vector3f *First,
                                const Vector3f *Second,
                                const Vector3f *Third)
{
   int i;
   
   for (i = 0; i < 3; i++)
   {
      Result->Comp[0][i] = First->Comp[i];
      Result->Comp[1][i] = Second->Comp[i];
      Result->Comp[2][i] = Third->Comp[i];
   }

} /* End Matrix3x3f_RowVecToMatrix */

/****************************************************************************
** 
** Function: Matrix3x3f_Skew
**
*/
void Matrix3x3f_Skew( Matrix3x3f     *Skew,
                      const Vector3f *Vector ) 
{

    Skew->Comp[0][0] =    0.0; 
    Skew->Comp[1][1] =    0.0;
    Skew->Comp[2][2] =    0.0;
	
    Skew->Comp[1][0] = - Vector->Comp[2];
    Skew->Comp[0][1] =   Vector->Comp[2];

    Skew->Comp[2][0] =   Vector->Comp[1];
    Skew->Comp[0][2] = - Vector->Comp[1];

    Skew->Comp[2][1] = - Vector->Comp[0];
    Skew->Comp[1][2] =   Vector->Comp[0];

}

/***********************************************************************************
**
** Function: Matrix3x3f_Sub
**
** Notes:   None
*/

void Matrix3x3f_Sub (Matrix3x3f *Result, const Matrix3x3f *Left, const Matrix3x3f *Right) 
{

   MatrixMxNf_Sub(&Result->Comp[0][0], &Left->Comp[0][0],&Right->Comp[0][0],3,3);

} /* End Matrix3x3f_Sub() */


/************************************************************************************
**
** Function: Matrix3x3f_Trace
**
** Notes: None  
*/

float Matrix3x3f_Trace (const Matrix3x3f *Operand) 
{
   float Trace;

   Trace=MatrixMxNf_Trace(&Operand->Comp[0][0],3,3);
   
   return (Trace);

} /* End Matrix3x3f_Trace() */

/************************************************************************************
**
** Function: Matrix3x3f_Transpose
**
** Notes:   
**  1. Local variable Rslt allows the caller to set A = Transpose of A
*/

void Matrix3x3f_Transpose (Matrix3x3f *Result, const Matrix3x3f *Operand) 
{
   Matrix3x3f Rslt;

   MatrixMxNf_Transpose(&Rslt.Comp[0][0], &Operand->Comp[0][0],3,3);
   MatrixMxNf_Copy(&Result->Comp[0][0],&Rslt.Comp[0][0], 3,3);

} /* End Matrix3x3f_Transpose() */


/****************************************************************************
** 
** Function: Matrix3x3f_Triad
**
*/
int Matrix3x3f_Triad( Matrix3x3f      *AttDcm,
                      const Vector3f  *Ur,
                      const Vector3f  *Vr,
                      const Vector3f  *Ub,
                      const Vector3f  *Vb  )
{

    int    i;

    float RMag, RMagInv;

    Vector3f R, S;

    Matrix3x3f Mb, Mr_t;

    int Valid;

    Valid = 1;  /* true; we assume rotation matrix will be found */


    /*  GCI vectors  */
    for ( i=0; i<3; i++ ) Mr_t.Comp[0][i] = Ur->Comp[i];

    Vector3f_Cross( &R, Ur, Vr );

    RMag = 0.0;
    for ( i=0; i<3; i++ ) RMag += R.Comp[i] * R.Comp[i];
    if ( RMag > 0.0 ) {
      RMagInv = 1.0 / sqrt( RMag );
      for ( i=0; i<3; i++ ) R.Comp[i] *= RMagInv;
    } else {
      Valid = 0;  /* false, no rotation matrix */
    }

    for ( i=0; i<3; i++ ) Mr_t.Comp[1][i] = R.Comp[i];

    Vector3f_Cross( &S, Ur, &R );

    for ( i=0; i<3; i++ ) Mr_t.Comp[2][i] = S.Comp[i];


    /*  body frame vectors   */
    for ( i=0; i<3; i++ ) Mb.Comp[i][0] = Ub->Comp[i];

    Vector3f_Cross( &R, Ub, Vb );

    RMag = 0.0;
    for ( i=0; i<3; i++ ) RMag += R.Comp[i] * R.Comp[i];
    if ( RMag > 0.0 ) {
      RMagInv = 1.0 / sqrt( RMag );
      for ( i=0; i<3; i++ ) R.Comp[i] *= RMagInv;
    } else {
      Valid = 0;  /* false, no rotation matrix */
    }
    
    for ( i=0; i<3; i++ ) Mb.Comp[i][1] = R.Comp[i];

    Vector3f_Cross( &S, Ub, &R );

    for ( i=0; i<3; i++ ) Mb.Comp[i][2] = S.Comp[i];


    /*  Direction Cosine Matrix */
    Matrix3x3f_Mult( AttDcm, &Mb, &Mr_t );

    return Valid;

}
/* end of file */
