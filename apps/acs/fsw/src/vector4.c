/* 
** File:
**   $Id: vector4d.c 1.1 2008/05/21 15:00:30EDT dcmccomas Exp  $
**
**  Purpose: Provide the implementation of the Vector4d package. 
**
**  $Date: 2008/05/21 15:00:30EDT $
**
**  $Revision: 1.1 $
**
**  $Log: vector4d.c  $
**  Revision 1.1 2008/05/21 15:00:30EDT dcmccomas 
**  Initial revision
**  Revision 1.2 2005/11/29 07:49:53EST dcmccomas 
**  New SDO delivery and added doxygen markup
**  Revision 1.3  2004/02/04 20:32:44  rbjork
**  Updates resulting from vector code walkthrough
**
**  Revision 1.2  2004/02/04 16:57:56  daviddawsonuser
**  replaced word files with actual files
**
*/

/*
** Include Files
*/

#include "vector4.h"
#include "matrixmxn.h"
#include <math.h>

/*
** Exported Functions
*/

/***********************************************************************************
**
** Function: Vector4d_Add
**
** Notes:   None
*/

void Vector4d_Add (Vector4d *Result, const Vector4d *Left, const Vector4d *Right) 
{

   MatrixMxNd_Add(&Result->Comp[0], &Left->Comp[0],&Right->Comp[0],4,1);

} /* End Vector4d_Add() */


/*********************************************************************************
**
** Function: Vector4d_Copy
**
** Notes:   None
*/

void Vector4d_Copy (Vector4d *Result, const Vector4d *Operand) 
{

   MatrixMxNd_Copy(&Result->Comp[0], &Operand->Comp[0],4,1);

} /* End Vector4d_Copy() */


/***********************************************************************************
**
** Function: Vector4d_DivScalar
**
** Notes: None
*/

void Vector4d_DivScalar (Vector4d *Result, const Vector4d *Left, double Scalar) 
{
   double ScalarInv;

   ScalarInv = 1.0 / Scalar;
   MatrixMxNd_MultScalar(&Result->Comp[0], &Left->Comp[0],4,1,ScalarInv);

} /* End Vector4d_DivScalar() */

/***********************************************************************************
**
** Function: Vector4d_Dot
**
** Notes:   None
*/
double Vector4d_Dot ( const Vector4d *Left, const Vector4d *Right)
{
   double Dot;

   MatrixMxNd_Mult( &Dot, &Left->Comp[0],1,4, &Right->Comp[0],4,1);
   return (Dot);
   
} /* End of Vector4d_Dot */


/*********************************************************************************
**
** Function: Vector4d_InitZero
**
** Notes:   None
*/

void Vector4d_InitZero (Vector4d *Result) 
{

   MatrixMxNd_InitZero(&Result->Comp[0],4,1);

} /* End Vector4d_InitZero() */

/********************************************************************************
**
** Function: Vector4d_Magnitude
**
** Notes:  None  
**  
*/

double Vector4d_Magnitude (const Vector4d *Operand) 
{
   double Mag;
   
   MatrixMxNd_Mult( &Mag, &Operand->Comp[0],1,4, &Operand->Comp[0],4,1);
   Mag = sqrt(Mag);
   return(Mag);

} /* End Vector4d_Magnitude() */

/************************************************************************************
**
** Function: Vector4d_MultScalar
**
** Notes: None
*/

void Vector4d_MultScalar (Vector4d *Result, const Vector4d *Left, double Scalar) 
{

   MatrixMxNd_MultScalar(&Result->Comp[0], &Left->Comp[0],4,1,Scalar);

} /* End Vector4d_MultScalar() */


/***********************************************************************************
**
** Function: Vector4d_Normalize
**
** Notes:   None
*/

void Vector4d_Normalize (Vector4d *Result, const Vector4d *Operand) 
{
   double Magnitude;

   Magnitude = Vector4d_Magnitude(Operand);
   Vector4d_DivScalar(Result,Operand,Magnitude);

} /* End Vector4d_Normalize() */


/***********************************************************************************
**
** Function: Vector4d_Sub
**
** Notes:   None
*/

void Vector4d_Sub (Vector4d *Result, const Vector4d *Left, const Vector4d *Right) 
{

   MatrixMxNd_Sub(&Result->Comp[0], &Left->Comp[0],&Right->Comp[0],4,1);

} /* End Vector4d_Sub() */

/*
** Include Files
*/


/*
** Exported Functions
*/

/***********************************************************************************
**
** Function: Vector4f_Add
**
** Notes:   None
*/

void Vector4f_Add (Vector4f *Result, const Vector4f *Left, const Vector4f *Right) 
{

   MatrixMxNf_Add(&Result->Comp[0], &Left->Comp[0],&Right->Comp[0],4,1);

} /* End Vector4f_Add() */


/*********************************************************************************
**
** Function: Vector4f_Copy
**
** Notes:   None
*/

void Vector4f_Copy (Vector4f *Result, const Vector4f *Operand) 
{

   MatrixMxNf_Copy(&Result->Comp[0], &Operand->Comp[0],4,1);

} /* End Vector4f_Copy() */


/***********************************************************************************
**
** Function: Vector4f_DivScalar
**
** Notes: None
*/

void Vector4f_DivScalar (Vector4f *Result, const Vector4f *Left, double Scalar) 
{
   double ScalarInv;

   ScalarInv = 1.0 / Scalar;
   MatrixMxNf_MultScalar(&Result->Comp[0], &Left->Comp[0],4,1,ScalarInv);

} /* End Vector4f_DivScalar() */

/***********************************************************************************
**
** Function: Vector4f_Dot
**
** Notes:   None
*/
float Vector4f_Dot ( const Vector4f *Left, const Vector4f *Right)
{
   float Dot;

   MatrixMxNf_Mult( &Dot, &Left->Comp[0],1,4, &Right->Comp[0],4,1);
   return (Dot);
   
} /* End of Vector4f_Dot */


/*********************************************************************************
**
** Function: Vector4f_InitZero
**
** Notes:   None
*/

void Vector4f_InitZero (Vector4f *Result) 
{

   MatrixMxNf_InitZero(&Result->Comp[0],4,1);

} /* End Vector4f_InitZero() */

/********************************************************************************
**
** Function: Vector4f_Magnitude
**
** Notes:  None  
**  
*/

float Vector4f_Magnitude (const Vector4f *Operand) 
{
   float Mag;
   
   MatrixMxNf_Mult( &Mag, &Operand->Comp[0],1,4, &Operand->Comp[0],4,1);
   Mag = sqrt(Mag);
   return(Mag);

} /* End Vector4f_Magnitude() */

/************************************************************************************
**
** Function: Vector4f_MultScalar
**
** Notes: None
*/

void Vector4f_MultScalar (Vector4f *Result, const Vector4f *Left, double Scalar) 
{

   MatrixMxNf_MultScalar(&Result->Comp[0], &Left->Comp[0],4,1,Scalar);

} /* End Vector4f_MultScalar() */


/***********************************************************************************
**
** Function: Vector4f_Normalize
**
** Notes:   None
*/

void Vector4f_Normalize (Vector4f *Result, const Vector4f *Operand) 
{
   float Magnitude;

   Magnitude = Vector4f_Magnitude(Operand);
   Vector4f_DivScalar(Result,Operand,Magnitude);

} /* End Vector4f_Normalize() */


/***********************************************************************************
**
** Function: Vector4f_Sub
**
** Notes:   None
*/

void Vector4f_Sub (Vector4f *Result, const Vector4f *Left, const Vector4f *Right) 
{

   MatrixMxNf_Sub(&Result->Comp[0], &Left->Comp[0],&Right->Comp[0],4,1);

} /* End Vector4f_Sub() */

/* end of file */
