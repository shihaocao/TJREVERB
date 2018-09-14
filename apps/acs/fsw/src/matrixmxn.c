/* 
** File:
**   $Id: matrixmxnd.c 1.1 2008/05/21 15:00:28EDT dcmccomas Exp  $
**
** Purpose: Provide the implementation of the MatrixMxNd package.  
**
** $Date: 2008/05/21 15:00:28EDT $
**
** $Revision: 1.1 $
**
** $Log: matrixmxnd.c  $
** Revision 1.1 2008/05/21 15:00:28EDT dcmccomas 
** Initial revision
** Revision 1.3 2007/06/11 11:27:47EDT dcmccomas 
** MatrixMxNd_Mult() – for loop changed from LeftColSize to RightRowSize. Use all input parameters.
** Revision 1.2 2005/11/29 07:49:15EST dcmccomas 
** New SDO delivery and added doxygen markup
** Revision 1.3  2004/03/23 15:20:13  ddawson
** removed DOS line endings
**
** Revision 1.2  2004/02/04 20:32:44  rbjork
** Updates resulting from vector code walkthrough
**
** Revision 1.1  2004/01/23 13:22:59  daviddawsonuser
** directory reorg
**
*/

/*
** Include Files
*/

#include "matrixmxn.h"


/*
** Exported Functions
*/

/******************************************************************************
**
** Function: MatrixMxNd_Add
**
** Notes:   None
*/
void MatrixMxNd_Add  (double            Result[], 
                      const double      Left[],
                      const double      Right[],
                      unsigned int      RowSize,
                      unsigned int      ColSize) 
{
   unsigned int     Element;
   unsigned int     NumElements;
	 
   NumElements = RowSize * ColSize;
 
   for(Element = 0; Element < NumElements; Element++) 
   {
      *(Result + Element) = *(Left + Element) + *(Right + Element);
   }

} /* End MatrixMxnd_Add */

/***********************************************************************************
** Function: MatrixMxNd_Copy
**
** Notes: 
**  1.    This function does not use the memset or memcopy functions, which would
**        be a more efficient way to perform the copy, because it would require
**        an additional package dependency on string.h
*/
void MatrixMxNd_Copy (double            Result[], 
                      const double      Input[], 
                      unsigned int      RowSize,
                      unsigned int      ColSize) 
{
   unsigned int     Element;
   unsigned int     NumElements;

   NumElements = RowSize * ColSize;
   for(Element = 0; Element < NumElements; Element++) 
   {
      *(Result + Element) = *(Input + Element); 
   }
} /* End MatrixMxNd_Copy */


/***********************************************************************************
** Function: MatrixMxNd_InitIndentiy
**
** Notes:None
*/
void MatrixMxNd_InitIdentity (double            Result[], 
                              unsigned int      RowSize,
                              unsigned int      ColSize) 
{
   unsigned int     Row;
   unsigned int     Col;
   unsigned int     Index;

   MatrixMxNd_InitZero(Result,RowSize,ColSize);
   Col=0;
   for(Row= 0; Row < RowSize; Row++) 
   {
     Index = Row*RowSize + Col; 
     *(Result + Index) = 1.0;
     Col++; 
   }
} /* End MatrixMxNd_InitIdentity */

/***********************************************************************************
** Function: MatrixMxNd_InitZero
**
** Notes: 
**  1.    This function does not use the memset or memcopy functions, which would
**        be a more efficient way to perform the copy, because it would require
**        an additional package dependency on string.h
*/
void MatrixMxNd_InitZero (double            Result[], 
                          unsigned int      RowSize,
                          unsigned int      ColSize) 
{
   unsigned int     Element;
   unsigned int     NumElements;

   NumElements = RowSize * ColSize;
   for(Element = 0; Element < NumElements; Element++) 
   {
      *(Result + Element) = 0.0; 
   }
} /* End MatrixMxNd_InitZero */

/***********************************************************************************
**
** Function: MatrixMxNd_Mult
**
** Notes:   None
*/
void MatrixMxNd_Mult (double            Result[], 
                      const double      Left[], 
                      unsigned int      LeftRowSize,
                      unsigned int      LeftColSize,
                      const double      Right[],
                      unsigned int      RightRowSize,
                      unsigned int      RightColSize) 
{
   double *ResultPtr;
   unsigned int Row, Col, Element;
   unsigned int RightOffset, LeftOffset;

   ResultPtr = Result;
   LeftOffset = 0;
   RightOffset = 0;
   for (Row = 0; Row < LeftRowSize; Row++) 
   {
      for(Col = 0; Col < RightColSize; Col ++) 
      {
         RightOffset = Col;
         *ResultPtr = 0.0;
         for(Element = 0; Element < RightRowSize; Element++) 
         {
            *ResultPtr += *(Left + LeftOffset) * *(Right + RightOffset);
            LeftOffset +=1;
            RightOffset +=RightColSize;
         }
         ResultPtr++;
         LeftOffset = 0;
      }
      Left += LeftColSize;
      RightOffset = 0;
   }

} /* End MatrixMxNd_Mult */

/************************************************************************************
**
** Function: MatrixMxNd_MultScalar
**
** Notes:   None
*/
void MatrixMxNd_MultScalar (double           Result[], 
                            const double     Left[], 
                            unsigned int     RowSize,
                            unsigned int     ColSize,
                            double           Scalar) 
{
   unsigned int     Element;
   unsigned int     NumElements;
	 
   NumElements = RowSize * ColSize;
 
   for(Element = 0; Element < NumElements; Element++) 
   {
      *(Result + Element) = *(Left + Element) * Scalar;
   }

} /* End MatrixMxNd_MultScalar */


/*************************************************************************************
**
** Function: MatrixMxNd_Sub
**
** Notes:   None
*/
void MatrixMxNd_Sub  (double            Result[], 
                      const double      Left[],
                      const double      Right[],
                      unsigned int      RowSize,
                      unsigned int      ColSize) 
{
   unsigned int     Element;
   unsigned int     NumElements;
	 
   NumElements = RowSize * ColSize;
   for(Element = 0; Element < NumElements; Element++) 
   {
      *(Result + Element) = *(Left + Element) - *(Right + Element);
   }

} /* End MatrixMxNd_Sub */

/******************************************************************************
**
** Function  MatrixMxNd_Trace
**
** Notes: None
*/
double  MatrixMxNd_Trace (const double      Operand[],
                          unsigned int      RowSize,
                          unsigned int      ColSize)
{
   unsigned int Row;
   unsigned int Offset;
   double Trace;

   Trace = 0.0;
   Offset = 0;
   for(Row = 0; Row < RowSize; Row++)
   {
     Offset = Row*ColSize + Row;
     Trace += *(Operand + Offset);
   }
   return(Trace);

}  /* End of MatrixMxNd_Trace */

/************************************************************************************
**
** Function: MatrixMxNd_Transpose
**
** Notes:   None
*/
void MatrixMxNd_Transpose  (double            Result[], 
                            const double      Input[],
                            unsigned int      RowSize,
                            unsigned int      ColSize) 
{
   unsigned int     i,j;
   unsigned int     Offset;
 
   Offset = 0; 
   for(i = 0; i < RowSize; i++) 
   {
      for(j=0; j < ColSize; j++) 
      {
         *(Result + j*RowSize + i) = *(Input + Offset++);
      }
   }

} /* End MatrixMxNd_Transpose */


/*
** Exported Functions
*/

/******************************************************************************
**
** Function: MatrixMxNf_Add
**
** Notes:   None
*/
void MatrixMxNf_Add  (float            Result[], 
                      const float      Left[],
                      const float      Right[],
                      unsigned int      RowSize,
                      unsigned int      ColSize) 
{
   unsigned int     Element;
   unsigned int     NumElements;
	 
   NumElements = RowSize * ColSize;
 
   for(Element = 0; Element < NumElements; Element++) 
   {
      *(Result + Element) = *(Left + Element) + *(Right + Element);
   }

} /* End MatrixMxnf_Add */

/***********************************************************************************
** Function: MatrixMxNf_Copy
**
** Notes: 
**  1.    This function does not use the memset or memcopy functions, which would
**        be a more efficient way to perform the copy, because it would require
**        an additional package dependency on string.h
*/
void MatrixMxNf_Copy (float            Result[], 
                      const float      Input[], 
                      unsigned int      RowSize,
                      unsigned int      ColSize) 
{
   unsigned int     Element;
   unsigned int     NumElements;

   NumElements = RowSize * ColSize;
   for(Element = 0; Element < NumElements; Element++) 
   {
      *(Result + Element) = *(Input + Element); 
   }
} /* End MatrixMxNf_Copy */


/***********************************************************************************
** Function: MatrixMxNf_InitIndentiy
**
** Notes:None
*/
void MatrixMxNf_InitIdentity (float            Result[], 
                              unsigned int      RowSize,
                              unsigned int      ColSize) 
{
   unsigned int     Row;
   unsigned int     Col;
   unsigned int     Index;

   MatrixMxNf_InitZero(Result,RowSize,ColSize);
   Col=0;
   for(Row= 0; Row < RowSize; Row++) 
   {
     Index = Row*RowSize + Col; 
     *(Result + Index) = 1.0;
     Col++; 
   }
} /* End MatrixMxNf_InitIdentity */

/***********************************************************************************
** Function: MatrixMxNf_InitZero
**
** Notes: 
**  1.    This function does not use the memset or memcopy functions, which would
**        be a more efficient way to perform the copy, because it would require
**        an additional package dependency on string.h
*/
void MatrixMxNf_InitZero (float            Result[], 
                          unsigned int      RowSize,
                          unsigned int      ColSize) 
{
   unsigned int     Element;
   unsigned int     NumElements;

   NumElements = RowSize * ColSize;
   for(Element = 0; Element < NumElements; Element++) 
   {
      *(Result + Element) = 0.0; 
   }
} /* End MatrixMxNf_InitZero */

/***********************************************************************************
**
** Function: MatrixMxNf_Mult
**
** Notes:   None
*/
void MatrixMxNf_Mult (float            Result[], 
                      const float      Left[], 
                      unsigned int      LeftRowSize,
                      unsigned int      LeftColSize,
                      const float      Right[],
                      unsigned int      RightRowSize,
                      unsigned int      RightColSize) 
{
   float *ResultPtr;
   unsigned int Row, Col, Element;
   unsigned int RightOffset, LeftOffset;

   ResultPtr = Result;
   LeftOffset = 0;
   RightOffset = 0;
   for (Row = 0; Row < LeftRowSize; Row++) 
   {
      for(Col = 0; Col < RightColSize; Col ++) 
      {
         RightOffset = Col;
         *ResultPtr = 0.0;
         for(Element = 0; Element < RightRowSize; Element++) 
         {
            *ResultPtr += *(Left + LeftOffset) * *(Right + RightOffset);
            LeftOffset +=1;
            RightOffset +=RightColSize;
         }
         ResultPtr++;
         LeftOffset = 0;
      }
      Left += LeftColSize;
      RightOffset = 0;
   }

} /* End MatrixMxNf_Mult */

/************************************************************************************
**
** Function: MatrixMxNf_MultScalar
**
** Notes:   None
*/
void MatrixMxNf_MultScalar (float           Result[], 
                            const float     Left[], 
                            unsigned int     RowSize,
                            unsigned int     ColSize,
                            double           Scalar) 
{
   unsigned int     Element;
   unsigned int     NumElements;
	 
   NumElements = RowSize * ColSize;
 
   for(Element = 0; Element < NumElements; Element++) 
   {
      *(Result + Element) = *(Left + Element) * Scalar;
   }

} /* End MatrixMxNf_MultScalar */


/*************************************************************************************
**
** Function: MatrixMxNf_Sub
**
** Notes:   None
*/
void MatrixMxNf_Sub  (float            Result[], 
                      const float      Left[],
                      const float      Right[],
                      unsigned int      RowSize,
                      unsigned int      ColSize) 
{
   unsigned int     Element;
   unsigned int     NumElements;
	 
   NumElements = RowSize * ColSize;
   for(Element = 0; Element < NumElements; Element++) 
   {
      *(Result + Element) = *(Left + Element) - *(Right + Element);
   }

} /* End MatrixMxNf_Sub */

/******************************************************************************
**
** Function  MatrixMxNf_Trace
**
** Notes: None
*/
float  MatrixMxNf_Trace (const float      Operand[],
                          unsigned int      RowSize,
                          unsigned int      ColSize)
{
   unsigned int Row;
   unsigned int Offset;
   float Trace;

   Trace = 0.0;
   Offset = 0;
   for(Row = 0; Row < RowSize; Row++)
   {
     Offset = Row*ColSize + Row;
     Trace += *(Operand + Offset);
   }
   return(Trace);

}  /* End of MatrixMxNf_Trace */

/************************************************************************************
**
** Function: MatrixMxNf_Transpose
**
** Notes:   None
*/
void MatrixMxNf_Transpose  (float            Result[], 
                            const float      Input[],
                            unsigned int      RowSize,
                            unsigned int      ColSize) 
{
   unsigned int     i,j;
   unsigned int     Offset;
 
   Offset = 0; 
   for(i = 0; i < RowSize; i++) 
   {
      for(j=0; j < ColSize; j++) 
      {
         *(Result + j*RowSize + i) = *(Input + Offset++);
      }
   }

} /* End MatrixMxNf_Transpose */

/* end of file */
