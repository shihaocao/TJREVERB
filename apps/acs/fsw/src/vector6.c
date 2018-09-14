/* 
** File:
**   $Id: vector6d.c 1.1 2008/05/21 15:00:30EDT dcmccomas Exp  $
**
** Purpose: Provide the implementation of the Vector6d package. 
**
** $Date: 2008/05/21 15:00:30EDT $
** $Revision: 1.1 $
** $Log: vector6d.c  $
** Revision 1.1 2008/05/21 15:00:30EDT dcmccomas 
** Initial revision
** Revision 1.1 2006/12/14 13:41:44EST myyang 
** Initial revision
**
*/

/*
** Include Files
*/

#include "vector6.h"
#include "matrixmxn.h"
#include <math.h>

/*
** Exported Functions
*/

/***********************************************************************************
**
** Function: Vector6d_Add
**
** Notes:   None
*/
void Vector6d_Add (Vector6d *Result, const Vector6d *Left, const Vector6d *Right) 
{

   MatrixMxNd_Add(&Result->Comp[0], &Left->Comp[0],&Right->Comp[0],6,1);

} /* End Vector6d_Add() */


/*********************************************************************************
**
** Function: Vector6d_Copy
**
** Notes:   None
*/
void Vector6d_Copy (Vector6d *Result, const Vector6d *Operand) 
{

   MatrixMxNd_Copy(&Result->Comp[0], &Operand->Comp[0],6,1);

} /* End Vector6d_Copy() */


/***********************************************************************************
**
** Function: Vector6d_DivScalar
**
** Notes: None
*/
void Vector6d_DivScalar (Vector6d *Result, const Vector6d *Left, double Scalar) 
{
   double ScalarInv;

   ScalarInv = 1.0 / Scalar;
   MatrixMxNd_MultScalar(&Result->Comp[0], &Left->Comp[0],6,1,ScalarInv);

} /* End Vector6d_DivScalar() */


/***********************************************************************************
**
** Function: Vector6d_Dot
**
** Notes:   None
*/
double Vector6d_Dot ( const Vector6d *Left, const Vector6d *Right)
{
   double Dot;

   MatrixMxNd_Mult( &Dot, &Left->Comp[0],1,6, &Right->Comp[0],6,1);
   return (Dot);
   
} /* End of Vector6d_Dot */


/*********************************************************************************
**
** Function: Vector6d_InitZero
**
** Notes:   None
*/
void Vector6d_InitZero (Vector6d *Result) 
{

   MatrixMxNd_InitZero(&Result->Comp[0],6,1);

} /* End Vector6d_InitZero() */


/********************************************************************************
**
** Function: Vector6d_Magnitude
**
** Notes:  None  
**  
*/
double Vector6d_Magnitude (const Vector6d *Operand) 
{
   double Mag;
   
   MatrixMxNd_Mult( &Mag, &Operand->Comp[0],1,6, &Operand->Comp[0],6,1);
   Mag = sqrt(Mag);
   return(Mag);

} /* End Vector6d_Magnitude() */


/************************************************************************************
**
** Function: Vector6d_MultScalar
**
** Notes: None
*/
void Vector6d_MultScalar (Vector6d *Result, const Vector6d *Left, double Scalar) 
{

   MatrixMxNd_MultScalar(&Result->Comp[0], &Left->Comp[0],6,1,Scalar);

} /* End Vector6d_MultScalar() */


/***********************************************************************************
**
** Function: Vector6d_Normalize
**
** Notes:   None
*/
void Vector6d_Normalize (Vector6d *Result, const Vector6d *Operand) 
{
   double Magnitude;

   Magnitude = Vector6d_Magnitude(Operand);
   Vector6d_DivScalar(Result,Operand,Magnitude);

} /* End Vector6d_Normalize() */


/***********************************************************************************
**
** Function: Vector6d_Sub
**
** Notes:   None
*/
void Vector6d_Sub (Vector6d *Result, const Vector6d *Left, const Vector6d *Right) 
{

   MatrixMxNd_Sub(&Result->Comp[0], &Left->Comp[0],&Right->Comp[0],6,1);

} /* End Vector6d_Sub() */

/*
** Exported Functions
*/

/***********************************************************************************
**
** Function: Vector6f_Add
**
** Notes:   None
*/
void Vector6f_Add (Vector6f *Result, const Vector6f *Left, const Vector6f *Right) 
{

   MatrixMxNf_Add(&Result->Comp[0], &Left->Comp[0],&Right->Comp[0],6,1);

} /* End Vector6f_Add() */


/*********************************************************************************
**
** Function: Vector6f_Copy
**
** Notes:   None
*/
void Vector6f_Copy (Vector6f *Result, const Vector6f *Operand) 
{

   MatrixMxNf_Copy(&Result->Comp[0], &Operand->Comp[0],6,1);

} /* End Vector6f_Copy() */


/***********************************************************************************
**
** Function: Vector6f_DivScalar
**
** Notes: None
*/
void Vector6f_DivScalar (Vector6f *Result, const Vector6f *Left, double Scalar) 
{
   double ScalarInv;

   ScalarInv = 1.0 / Scalar;
   MatrixMxNf_MultScalar(&Result->Comp[0], &Left->Comp[0],6,1,ScalarInv);

} /* End Vector6f_DivScalar() */


/***********************************************************************************
**
** Function: Vector6f_Dot
**
** Notes:   None
*/
float Vector6f_Dot ( const Vector6f *Left, const Vector6f *Right)
{
   float Dot;

   MatrixMxNf_Mult( &Dot, &Left->Comp[0],1,6, &Right->Comp[0],6,1);
   return (Dot);
   
} /* End of Vector6f_Dot */


/*********************************************************************************
**
** Function: Vector6f_InitZero
**
** Notes:   None
*/
void Vector6f_InitZero (Vector6f *Result) 
{

   MatrixMxNf_InitZero(&Result->Comp[0],6,1);

} /* End Vector6f_InitZero() */


/********************************************************************************
**
** Function: Vector6f_Magnitude
**
** Notes:  None  
**  
*/
float Vector6f_Magnitude (const Vector6f *Operand) 
{
   float Mag;
   
   MatrixMxNf_Mult( &Mag, &Operand->Comp[0],1,6, &Operand->Comp[0],6,1);
   Mag = sqrt(Mag);
   return(Mag);

} /* End Vector6f_Magnitude() */


/************************************************************************************
**
** Function: Vector6f_MultScalar
**
** Notes: None
*/
void Vector6f_MultScalar (Vector6f *Result, const Vector6f *Left, double Scalar) 
{

   MatrixMxNf_MultScalar(&Result->Comp[0], &Left->Comp[0],6,1,Scalar);

} /* End Vector6f_MultScalar() */


/***********************************************************************************
**
** Function: Vector6f_Normalize
**
** Notes:   None
*/
void Vector6f_Normalize (Vector6f *Result, const Vector6f *Operand) 
{
   float Magnitude;

   Magnitude = Vector6f_Magnitude(Operand);
   Vector6f_DivScalar(Result,Operand,Magnitude);

} /* End Vector6f_Normalize() */


/***********************************************************************************
**
** Function: Vector6f_Sub
**
** Notes:   None
*/
void Vector6f_Sub (Vector6f *Result, const Vector6f *Left, const Vector6f *Right) 
{

   MatrixMxNf_Sub(&Result->Comp[0], &Left->Comp[0],&Right->Comp[0],6,1);

} /* End Vector6f_Sub() */

/* end of file */
