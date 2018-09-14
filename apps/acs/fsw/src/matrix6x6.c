/* 
** File:
**   $Id: matrix6x6d.c 1.1 2008/05/21 15:00:27EDT dcmccomas Exp  $
**
**  Purpose: Provide the implementation of the Matrix6x6d package. 
**
**  $Date: 2008/05/21 15:00:27EDT $
**
**  $Revision: 1.1 $
**
**  $Log: matrix6x6d.c  $
**  Revision 1.1 2008/05/21 15:00:27EDT dcmccomas 
**  Initial revision
**  Revision 1.2 2005/11/29 07:49:14EST dcmccomas 
**  New SDO delivery and added doxygen markup
**  Revision 1.3  2004/03/23 15:20:13  ddawson
**  removed DOS line endings
**
**  Revision 1.2  2004/02/04 20:32:44  rbjork
**  Updates resulting from vector code walkthrough
**
**  Revision 1.1  2004/01/23 13:22:59  daviddawsonuser
**  directory reorg
**
*/

/*
** Include Files 
*/

#include "matrix6x6.h"
#include "matrixmxn.h"

/*
** Exported Functions 
*/


/*********************************************************************************
**
** Function: Matrix6x6d_Add
**
** Notes:   None
*/

void Matrix6x6d_Add (Matrix6x6d *Result, const Matrix6x6d *Left, const Matrix6x6d *Right) 
{

   MatrixMxNd_Add(&Result->Comp[0][0], &Left->Comp[0][0],&Right->Comp[0][0],6,6);

} /* End Matrix6x6d_Add() */


/********************************************************************************************
**
** Function: Matrix6x6d_Copy
**
** Notes:
**   None
*/

void Matrix6x6d_Copy (Matrix6x6d *Result, const Matrix6x6d *Operand) 
{

   MatrixMxNd_Copy(&Result->Comp[0][0], &Operand->Comp[0][0],6,6);

} /* End Matrix6x6d_Copy() */

/*****************************************************************************************
**
** Function: Matrix6x6d_DivScalar
**
** Notes: None
*/

void Matrix6x6d_DivScalar (Matrix6x6d *Result, const Matrix6x6d *Left, double Scalar) 
{
   double ScalarInv;

   ScalarInv = 1.0 / Scalar;
   MatrixMxNd_MultScalar(&Result->Comp[0][0], &Left->Comp[0][0],6,6,ScalarInv);

} /* End Matrix6x6d_DivScalar() */

/*********************************************************************************
**
** Function: Matrix6x6d_InitIndentity
**
** Notes:   None
*/

void Matrix6x6d_InitIdentity (Matrix6x6d *Result) 
{

   MatrixMxNd_InitIdentity(&Result->Comp[0][0],6, 6);

} /* End Matrix6x6d_InitIdentity() */

/*********************************************************************************
**
** Function: Matrix6x6d_InitZero
**
** Notes:   None
*/

void Matrix6x6d_InitZero (Matrix6x6d *Result) 
{

   MatrixMxNd_InitZero(&Result->Comp[0][0],6,6);

} /* End Matrix6x6d_InitZero() */

/*******************************************************************************************
**
** Function: Matrix6x6d_Mult
**
** Notes:   
**  1.  Local variable Rslt allows the caller to set the result to
**     one of the input variables.
*/

void Matrix6x6d_Mult (Matrix6x6d *Result, const Matrix6x6d *Left,const Matrix6x6d *Right) 
{
   Matrix6x6d Rslt;

   MatrixMxNd_Mult(&Rslt.Comp[0][0], &Left->Comp[0][0],6,6,&Right->Comp[0][0],6,6);
   MatrixMxNd_Copy(&Result->Comp[0][0],&Rslt.Comp[0][0], 6,6);

} /* End Matrix6x6d_Mult() */

/******************************************************************************************
**
** Function: Matrix6x6d_MultScalar
**
** Notes: None
*/

void Matrix6x6d_MultScalar (Matrix6x6d *Result, const Matrix6x6d *Left, double Scalar) 
{

   MatrixMxNd_MultScalar(&Result->Comp[0][0], &Left->Comp[0][0],6,6,Scalar);

} /* End Matrix6x6d_MultScalar() */


/******************************************************************************************
**
** Function: Matrix6x6d_Sub
**
** Notes:   None
*/

void Matrix6x6d_Sub (Matrix6x6d *Result,const Matrix6x6d *Left, const Matrix6x6d *Right) 
{

   MatrixMxNd_Sub(&Result->Comp[0][0], &Left->Comp[0][0],&Right->Comp[0][0],6,6);

} /* End Matrix6x6d_Sub() */


/*****************************************************************************************
**
** Function: Matrix6x6d_Trace
**
** Notes: None
*/

double Matrix6x6d_Trace (const Matrix6x6d *Operand) 
{
   double Trace;

   Trace = MatrixMxNd_Trace(&Operand->Comp[0][0],6,6); 
   return(Trace);

} /* End Matrix6x6d_Trace() */

/*****************************************************************************************
**
** Function: Matrix6x6d_Transpose
**
** Notes: 
**  1. Local variable Rslt allows the caller to set A = Transpose of A
*/

void Matrix6x6d_Transpose (Matrix6x6d *Result, const Matrix6x6d *Operand) 
{

   Matrix6x6d Rslt;

   MatrixMxNd_Transpose(&Rslt.Comp[0][0], &Operand->Comp[0][0],6,6);
   MatrixMxNd_Copy(&Result->Comp[0][0],&Rslt.Comp[0][0], 6,6);

} /* End Matrix6x6d_Transpose() */


/*
** Exported Functions 
*/


/*********************************************************************************
**
** Function: Matrix6x6f_Add
**
** Notes:   None
*/

void Matrix6x6f_Add (Matrix6x6f *Result, const Matrix6x6f *Left, const Matrix6x6f *Right) 
{

   MatrixMxNf_Add(&Result->Comp[0][0], &Left->Comp[0][0],&Right->Comp[0][0],6,6);

} /* End Matrix6x6f_Add() */


/********************************************************************************************
**
** Function: Matrix6x6f_Copy
**
** Notes:
**   None
*/

void Matrix6x6f_Copy (Matrix6x6f *Result, const Matrix6x6f *Operand) 
{

   MatrixMxNf_Copy(&Result->Comp[0][0], &Operand->Comp[0][0],6,6);

} /* End Matrix6x6f_Copy() */

/*****************************************************************************************
**
** Function: Matrix6x6f_DivScalar
**
** Notes: None
*/

void Matrix6x6f_DivScalar (Matrix6x6f *Result, const Matrix6x6f *Left, float Scalar) 
{
   float ScalarInv;

   ScalarInv = 1.0 / Scalar;
   MatrixMxNf_MultScalar(&Result->Comp[0][0], &Left->Comp[0][0],6,6,ScalarInv);

} /* End Matrix6x6f_DivScalar() */

/*********************************************************************************
**
** Function: Matrix6x6f_InitIndentity
**
** Notes:   None
*/

void Matrix6x6f_InitIdentity (Matrix6x6f *Result) 
{

   MatrixMxNf_InitIdentity(&Result->Comp[0][0],6, 6);

} /* End Matrix6x6f_InitIdentity() */

/*********************************************************************************
**
** Function: Matrix6x6f_InitZero
**
** Notes:   None
*/

void Matrix6x6f_InitZero (Matrix6x6f *Result) 
{

   MatrixMxNf_InitZero(&Result->Comp[0][0],6,6);

} /* End Matrix6x6f_InitZero() */

/*******************************************************************************************
**
** Function: Matrix6x6f_Mult
**
** Notes:   
**  1.  Local variable Rslt allows the caller to set the result to
**     one of the input variables.
*/

void Matrix6x6f_Mult (Matrix6x6f *Result, const Matrix6x6f *Left,const Matrix6x6f *Right) 
{
   Matrix6x6f Rslt;

   MatrixMxNf_Mult(&Rslt.Comp[0][0], &Left->Comp[0][0],6,6,&Right->Comp[0][0],6,6);
   MatrixMxNf_Copy(&Result->Comp[0][0],&Rslt.Comp[0][0], 6,6);

} /* End Matrix6x6f_Mult() */

/******************************************************************************************
**
** Function: Matrix6x6f_MultScalar
**
** Notes: None
*/

void Matrix6x6f_MultScalar (Matrix6x6f *Result, const Matrix6x6f *Left, float Scalar) 
{

   MatrixMxNf_MultScalar(&Result->Comp[0][0], &Left->Comp[0][0],6,6,Scalar);

} /* End Matrix6x6f_MultScalar() */


/******************************************************************************************
**
** Function: Matrix6x6f_Sub
**
** Notes:   None
*/

void Matrix6x6f_Sub (Matrix6x6f *Result,const Matrix6x6f *Left, const Matrix6x6f *Right) 
{

   MatrixMxNf_Sub(&Result->Comp[0][0], &Left->Comp[0][0],&Right->Comp[0][0],6,6);

} /* End Matrix6x6f_Sub() */


/*****************************************************************************************
**
** Function: Matrix6x6f_Trace
**
** Notes: None
*/

float Matrix6x6f_Trace (const Matrix6x6f *Operand) 
{
   float Trace;

   Trace = MatrixMxNf_Trace(&Operand->Comp[0][0],6,6); 
   return(Trace);

} /* End Matrix6x6f_Trace() */

/*****************************************************************************************
**
** Function: Matrix6x6f_Transpose
**
** Notes: 
**  1. Local variable Rslt allows the caller to set A = Transpose of A
*/

void Matrix6x6f_Transpose (Matrix6x6f *Result, const Matrix6x6f *Operand) 
{

   Matrix6x6f Rslt;

   MatrixMxNf_Transpose(&Rslt.Comp[0][0], &Operand->Comp[0][0],6,6);
   MatrixMxNf_Copy(&Result->Comp[0][0],&Rslt.Comp[0][0], 6,6);

} /* End Matrix6x6f_Transpose() */


/* end of file */
