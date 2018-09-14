#include "matrix6x6.h"
#include "matrix3x3.h"
#include "vector3.h"
#include "vector4.h"
#include "vector6.h"
#include <stdio.h>

void print_Matrix3x3d ( const char *str, const Matrix3x3d *Mat)
{
 int i, j;
 Matrix3x3d blah = *Mat;

printf("%s\n", str);
 for (i=0; i<=2; i++) {
	 for (j=0; j<=2; j++) {
	 printf("%f\t", blah.Comp[i][j]);
	 }
	 printf("\n");
 }

}

void print_Matrix6x6d (const char *str,  const Matrix6x6d *Mat)
{
 int i, j;
 Matrix6x6d blah;

 blah = *Mat;

printf("%s\n", str);

 for (i=0; i<=5; i++) {
	 for (j=0; j<=5; j++) {
	 printf("%3.3e\t", blah.Comp[i][j]);
	 }
	 printf("\n");
 }

}

void print_Vector3d ( const char *str, const Vector3d *V3d)
{
 int i;
 Vector3d blah;

printf("%s: ", str);

 blah = *V3d;

 for (i=0; i<=2; i++)
	 printf("%f\t", blah.Comp[i]);
 printf("\n");

}

void print_Vector4d ( const char *str, const Vector4d *V4d)
{
 int i;
 Vector4d blah;

printf("%s: ", str);

 blah = *V4d;

 for (i=0; i<=3; i++)
	 printf("%f\t", blah.Comp[i]);
 printf("\n");

}

void print_Vector6d (const char *str,  const Vector6d *V6d)
{
 int i;
 Vector6d blah;

printf("%s: ", str);

 blah = *V6d;

 for (i=0; i<=5; i++)
	 printf("%f\t", blah.Comp[i]);
 printf("\n");

}

void print_Matrix3x3f ( const char *str, const Matrix3x3f *Mat)
{
 int i, j;
 Matrix3x3f blah = *Mat;

printf("%s\n", str);
 for (i=0; i<=2; i++) {
	 for (j=0; j<=2; j++) {
	 printf("%f\t", blah.Comp[i][j]);
	 }
	 printf("\n");
 }

}

void print_Matrix6x6f (const char *str,  const Matrix6x6f *Mat)
{
 int i, j;
 Matrix6x6f blah;

 blah = *Mat;

printf("%s\n", str);

 for (i=0; i<=5; i++) {
	 for (j=0; j<=5; j++) {
	 printf("%3.3e\t", blah.Comp[i][j]);
	 }
	 printf("\n");
 }

}

void print_Vector3f ( const char *str, const Vector3f *V3f)
{
 int i;
 Vector3f blah;

printf("%s: ", str);

 blah = *V3f;

 for (i=0; i<=2; i++)
	 printf("%f\t", blah.Comp[i]);
 printf("\n");
}

void print_Vector4f ( const char *str, const Vector4f *V4f)
{
 int i;
 Vector4f blah;

printf("%s: ", str);

 blah = *V4f;

 for (i=0; i<=3; i++)
	 printf("%f\t", blah.Comp[i]);
 printf("\n");

}

void print_Vector6f (const char *str,  const Vector6f *V6f)
{
 int i;
 Vector6f blah;

printf("%s: ", str);

 blah = *V6f;

 for (i=0; i<=5; i++)
	 printf("%f\t", blah.Comp[i]);
 printf("\n");

}
