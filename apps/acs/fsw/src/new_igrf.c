/****************************************************************************/
/*                                                                          */
/*     NGDC's Geomagnetic Field Modeling software for the IGRF and WMM      */
/*                                                                          */
/****************************************************************************/
#include <math.h>
#include <float.h>
#include <stdio.h>
#include "vector3.h"

#define MAXDEG 13
#define MAXCOEFF (MAXDEG*(MAXDEG+2))
#define RAD2DEG (180.0/M_PI)


const int igrf_date=2015;
const int igrf_ord=13;
const int sv_ord=8;
const float igrf_coeffs[195]={-29442.0,-1501.0,4797.1,-2445.1,3012.9,-2845.6,1676.7,-641.9,1350.7,-2352.3,-115.3,1225.6,244.9,582.0,-538.4,907.6,813.7,283.3,120.4,-188.7,-334.9,180.9,70.4,-329.5,-232.6,360.1,47.3,192.4,197.0,-140.9,-119.3,-157.5,16.0,4.1,100.2,70.0,67.7,-20.8,72.7,33.2,-129.9,58.9,-28.9,-66.7,13.2,7.3,-70.9,62.6,81.6,-76.1,-54.1,-6.8,-19.5,51.8,5.7,15.0,24.4,9.4,3.4,-2.8,-27.4,6.8,-2.2,24.2,8.8,10.1,-16.9,-18.3,-3.2,13.3,-20.6,-14.6,13.4,16.2,11.7,5.7,-15.9,-9.1,-2.0,2.1,5.4,8.8,-21.6,3.1,10.8,-3.3,11.8,0.7,-6.8,-13.3,-6.9,-0.1,7.8,8.7,1.0,-9.1,-4.0,-10.5,8.4,-1.9,-6.3,3.2,0.1,-0.4,0.5,4.6,-0.5,4.4,1.8,-7.9,-0.7,-0.6,2.1,-4.2,2.4,-2.8,-1.8,-1.2,-3.6,-8.7,3.1,-1.5,-0.1,-2.3,2.0,2.0,-0.7,-0.8,-1.1,0.6,0.8,-0.7,-0.2,0.2,-2.2,1.7,-1.4,-0.2,-2.5,0.4,-2.0,3.5,-2.4,-1.9,-0.2,-1.1,0.4,0.4,1.2,1.9,-0.8,-2.2,0.9,0.3,0.1,0.7,0.5,-0.1,-0.3,0.3,-0.4,0.2,0.2,-0.9,-0.9,-0.1,0.0,0.7,0.0,-0.9,-0.9,0.4,0.4,0.5,1.6,-0.5,-0.5,1.0,-1.2,-0.2,-0.1,0.8,0.4,-0.1,-0.1,0.3,0.4,0.1,0.5,0.5,-0.3,-0.4,-0.4,-0.3,-0.8};
const float igrf_sv[80]={10.3,18.1,-26.6,-8.7,-3.3,-27.4,2.1,-14.1,3.4,-5.5,8.2,-0.7,-0.4,-10.1,1.8,-0.7,0.2,-1.3,-9.1,5.3,4.1,2.9,-4.3,-5.2,-0.2,0.5,0.6,-1.3,1.7,-0.1,-1.2,1.4,3.4,3.9,0.0,-0.3,-0.1,0.0,-0.7,-2.1,2.1,-0.7,-1.2,0.2,0.3,0.9,1.6,1.0,0.3,-0.2,0.8,-0.5,0.4,1.3,-0.2,0.1,-0.3,-0.6,-0.6,-0.8,0.1,0.2,-0.2,0.2,0.0,-0.3,-0.6,0.3,0.5,0.1,-0.2,0.5,0.4,-0.2,0.1,-0.3,-0.4,0.3,0.3};

float mag_coeff[MAXCOEFF];                   //Computed coefficients

/****************************************************************************/
/*                                                                          */
/*                           Subroutine extrapsh                            */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Extrapolates linearly a spherical harmonic model with a              */
/*     rate-of-change model.                                                */
/*                                                                          */
/*     Input:                                                               */
/*           date     - date of resulting model (in decimal year)           */
/*          igrf_date - date of base model                                  */
/*          igrf_ord  - maximum degree and order of base model              */
/*        igrf_coeffs - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of base model                 */
/*           sv_ord   - maximum degree and order of rate-of-change model    */
/*           igrf_sv  - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of rate-of-change model       */
/*                                                                          */
/*     Output:                                                              */
/*        mag_coeff - Schmidt quasi-normal internal spherical               */
/*                    harmonic coefficients                                 */
/*           nmax   - maximum degree and order of resulting model           */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 16, 1988                                                */
/*                                                                          */
/****************************************************************************/


int extrapsh(float date){
  int   nmax;
  int   k, l;
  int   i;
  int   igo=igrf_ord,svo=sv_ord;
  float factor;
  //# of years to extrapolate
  factor = date - igrf_date;
  //make shure that degree is smaller then MAXDEG
  if(igo>MAXDEG){
      igo=MAXDEG;
  }
  if(svo>MAXDEG){
      svo=MAXDEG;
  }
  //check for equal degree
  if (igo == svo){
      k =  igo * (igo + 2);
      nmax = igo;
  }else{
      //check if reference is bigger
      if (igo > svo){
          k = svo * (svo + 2);
          l = igo * (igo + 2);
          //copy extra elements unchanged
          for ( i = k ; i < l; ++i){
              mag_coeff[i] = igrf_coeffs[i];
				//printf("mag_coeff[%d] = %f, igrf_coeffs = %f", i, mag_coeff[i], igrf_coeffs[i]);
          }
          //maximum degree of model
          nmax = igo;
      }else{
          k = igo * (igo + 2);
          l = svo * (svo + 2);
          //put in change for extra elements?
          for(i=k;i<l;++i){
            mag_coeff[i] = factor * igrf_sv[i];
          }
          nmax = svo;
        }
    }
    //apply secular variations to model
    for ( i = 0; i < k; ++i){
        mag_coeff[i] = igrf_coeffs[i] + factor * igrf_sv[i];
    }
    //return maximum degree of model and secular variations
    return nmax;
}


/****************************************************************************/
/*                                                                          */
/*                           Subroutine new_igrf                            */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Calculates field components from spherical harmonic (sh)             */
/*     models.                                                              */
/*                                                                          */
/*     Input:                                                               */
/*           latitude  - north latitude, in radians                         */
/*           longitude - east longitude, in radians                         */
/*           elev      - radial distance from earth's center                */
/*           nmax      - maximum degree and order of coefficients           */
/*                                                                          */
/*     Output:                                                              */
/*           x         - northward component                                */
/*           y         - eastward component                                 */
/*           z         - vertically-downward component                      */
/*                                                                          */
/*     based on subroutine 'igrf' by D. R. Barraclough and S. R. C. Malin,  */
/*     report no. 71/1, institute of geological sciences, U.K.              */
/*                                                                          */
/*     FORTRAN                                                              */
/*           Norman W. Peddie                                               */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 17, 1988                                                */
/*                                                                          */
/****************************************************************************/

//define buffer size for q and q
#define PQ_BUFFSIZE         32

int new_igrf(float flat,float flon,float elev,int nmax,Vector3f *dest){
  const float earths_radius = 6371.2;
  float slat;
  float clat;
  float ratio;
  float aa, bb, cc;
  float rr;
  float fm,fn;
  float sl[MAXDEG];
  float cl[MAXDEG];
  float p[PQ_BUFFSIZE];
  float q[PQ_BUFFSIZE];
  int i,j,k,l,m,n;
  int kw;
  int npq;
  float x,y,z;

  flat = flat/RAD2DEG;
  flon = flon/RAD2DEG;
  //printf("lat, long, elev, nmax: %f %f, %f, %d\n", flat, flon, elev, nmax);
  //calculate sin and cos of latitude
  slat = sin(flat);
  clat = cos(flat);
  //prevent divide by zero
  if(clat==0){
    clat=DBL_EPSILON;
  }

  //calculate sin and cos of longitude
  sl[0] = sin(flon);
  cl[0] = cos(flon);
  //initialize coordinates
  x = 0;
  y = 0;
  z = 0;

  //calculate loop iterations
  npq = (nmax * (nmax + 3)) / 2;

  //calculate ratio of earths radius to elevation
  ratio = earths_radius / elev;

  aa = sqrt(3.0);

  //set initial values of p
  p[0] = 2.0 * slat;
  p[1] = 2.0 * clat;
  p[2] = 4.5 * slat * slat - 1.5;
  p[3] = 3.0 * aa * clat * slat;

  //Set initial values of q
  q[0] = -clat;
  q[1] = slat;
  q[2] = -3.0 * clat * slat;
  q[3] = aa * (slat * slat - clat * clat);

  for(k=0,l=1,n=0,m=0,rr=ratio*ratio; k < npq;k++,m++){
      //testing get wrapped idx
      kw=k%PQ_BUFFSIZE;
      if (n <= m){
          m = -1;
          n+= 1;
          //rr = pow(ratio,n+2);
          rr*=ratio;
          fn = n;
      }
      fm = m+1;
      if (k >= 4){
          j = k - n ;
          //wrap j for smaller array
          j=j%PQ_BUFFSIZE;
          if (m+1 == n){
              aa = sqrt(1.0 - 0.5/fm);
              p[kw] = (1.0 + 1.0/fm) * aa * clat * p[j-1];
              q[kw] = aa * (clat * q[j-1] + slat/fm * p[j-1]);
              sl[m] = sl[m-1] * cl[0] + cl[m-1] * sl[0];
              cl[m] = cl[m-1] * cl[0] - sl[m-1] * sl[0];
          }else{
              aa = sqrt(fn*fn - fm*fm);
              bb = sqrt(((fn - 1.0)*(fn-1.0)) - (fm * fm))/aa;
              cc = (2.0 * fn - 1.0)/aa;
              i = k - 2 * n + 1;
              //wrap i for smaller array
              i=i%PQ_BUFFSIZE;
              p[kw] = (fn + 1.0) * (cc * slat/fn * p[j] - bb/(fn - 1.0) * p[i]);
              q[kw] = cc * (slat * q[j] - clat/fn * p[j]) - bb * q[i];
            }
        }
        aa = rr * mag_coeff[l-1];

      if (m == -1){
          x = x + aa * q[kw];
          z = z - aa * p[kw];
          l+= 1;
      }else{
              bb = rr * mag_coeff[l];
              cc = aa * cl[m] + bb * sl[m];
              x = x + cc * q[kw];
              z = z - cc * p[kw];
              if (clat > 0){
                  y = y + (aa * sl[m] - bb * cl[m]) *fm * p[kw]/((fn + 1.0) * clat);
              }else{
                  y = y + (aa * sl[m] - bb * cl[m]) * q[kw] * slat;
              }
              l+= 2;
      }
    }
    //printf("x, y, z: %f %f %f\n", x,y,z);
    //set destination values
    dest->Comp[0]=x;
    dest->Comp[1]=y;
    dest->Comp[2]=z;
    //always returns zero
    return 0;
}
