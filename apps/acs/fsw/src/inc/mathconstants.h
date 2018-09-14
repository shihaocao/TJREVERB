/** \file
** 
** \brief Define macros for basic physical constants and unit conversion ratios.
**
** $Id: mathconstants.h 1.1 2008/05/21 15:00:20EDT dcmccomas Exp  $
** $Date: 2008/05/21 15:00:20EDT $
** $Revision: 1.1 $
** $Log: mathconstants.h  $
** Revision 1.1 2008/05/21 15:00:20EDT dcmccomas 
** Initial revision
** Revision 1.2 2005/11/29 07:48:56EST dcmccomas 
** New SDO delivery and added doxygen markup
** Revision 1.1.8.1  2005/11/17 04:08:32  ddawson
** Modified comments for doxygen
**
** Revision 1.1  2004/01/28 20:24:58  daviddawsonuser
** added mathconstants test
**
** Revision 1.1  2004/01/23 13:22:59  daviddawsonuser
** directory reorg
**
** Revision 1.3  2004/01/20 19:13:29  daviddawsonuser
** Made changes based on code review
**
** Revision 1.2  2003/12/12 21:06:46  daviddawsonuser
** *** empty log message ***
**
** Revision 1.1.1.1  2003/12/08 20:00:06  daviddawsonuser
** Imported Sources
**
**
*/

/** 
** @addtogroup math_const
** @{
*/

#ifndef _math_constants_h_
#define _math_constants_h_


#define    MTH_PI                  (3.141592653589793238460)

#define    MTH_TWO_PI              (6.283185307179586476920)

#define    MTH_HALF_PI             (1.570796326794896619230)

#define    MTH_RADS_PER_DEG        (0.0174532925199432957692)

#define    MTH_DEGS_PER_RAD        (57.29577951308232087685)

#define    MTH_RADS_PER_ARCSEC     (0.0000048481368110953599) 

#define    MTH_ARCSECS_PER_RAD     (206264.8062470963551566469116)

#define    MTH_SECS_PER_HOUR       (3600.00)

#define    MTH_SECS_PER_DAY        (86400.00)

#define    MTH_DAYS_PER_CENTURY    (36525.0)

#define    MTH_SECS_PER_CENTURY    (3155760000.0)                     /**< MTH_SECS_PER_DAY * MTH_DAYS_PER_CENTURY */


/* 
** The following tolerances and comments were taken from the MAP math library.
**
** These tolerances are generally used to protect against a divide-by-zero.
** However, the inverse must also be considered; overflow due to
** division by a small number. ACS FSW usually sticks with units that keep the
** exponents within +/- 20.  Therefore these tolerances have been defined to
** protect a divide by zero, and depending on how the application code is
** written, will "protect" against overflows in divisions using reasonable
** numerators.
**
** The tolerances are based on the following ANSI C (IEEE) definitions
** found in float.h:
**
**       #define FLT_MIN   1.17549435E-38F
**       #define FLT_MAX   3.40282347E+38F
**       #define DBL_MIN   2.2250738585072014E-308
**       #define DBL_MAX   1.7976931348623157E+308
**
*/
#define    MTH_FLOAT_ZERO_TOLERANCE  (1.0e-25)       /**< About 3/4'th of the float range */
#define    MTH_DOUBLE_ZERO_TOLERANCE (1.0e-30)       /**< Just a bit smaller than float tolerance. GN&C FSW typically does not use smaller values */

#define    MTH_SPEED_OF_LIGHT      (2.99792458e8)    /**< Meters per second */




#endif   /* #ifndef _math_constants_h_ */
/** @} */
