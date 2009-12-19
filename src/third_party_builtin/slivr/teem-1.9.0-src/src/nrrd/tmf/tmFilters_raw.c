/*
### Written by Torsten Moeller
### sometime in March 1998
### let me know, if you find any errors (moeller@cis.ohio-state.edu)
###
### March 2003 - modified to remove coeff part
*/

#pragma warning(disable:4244)
#pragma warning(disable:4305)

#define OVER_3 0.33333333
#define OVER_6 0.16666666
#define OVER_12 0.0833333333
#define OVER_2_3 0.6666666666

#include "tmFilters_raw.h"

/***************************************************************************/
/*                          Approximation Filters                          */
/***************************************************************************/

float dn_cn_1ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = 0.5; break;
  case 1: result = 0.5; break;
  default: result = 0;
  }
  return result;
}

float dn_cn_2ef(float a, float t) {

  return d0_c0_2ef(a, t);
}

float dn_cn_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ( 0.25*t +(2*a-0.25))*t -   a  ; break;
  case 1: result = (-0.25*t -(6*a-1.25))*t + 3*a  ; break;
  case 2: result = (-0.25*t +(6*a-0.75))*t - 3*a+1; break;
  case 3: result = ( 0.25*t -(2*a+0.25))*t +   a  ; break;
  default: result = 0;
  }
  return result;
}

float dn_cn_4ef(float a, float t) {

  return d0_c0_4ef(a, t);
}

float dn_c0_1ef(float a, float t) {

  return d0_c0_2ef(a, t);
}

float dn_c0_2ef(float a, float t) {

  return d0_c0_2ef(a, t);
}

float dn_c0_3ef(float a, float t) {

  return d0_c0_3ef(a, t);
}

float dn_c0_4ef(float a, float t) {

  return d0_c0_4ef(a,t);
}

float dn_c1_1ef(float a, float t) {

  return d0_c1_1ef(a, t);
}

float dn_c1_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ( t   )*t/4;       break;
  case 1: result = (-t +2)*t/4 +0.25; break;
  case 2: result = (-t   )*t/4 +0.5;  break;
  case 3: result = ( t -2)*t/4 +0.25; break;
  default: result = 0;
  }
  return result;
}

float dn_c1_3ef(float a, float t) {

  return d0_c1_3ef(a, t);
}

float dn_c1_4ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((-(   a       )*t +(1.5*a- 1./24))*t +         0)*t +             0; break;
  case 1: result = (( ( 5*a+OVER_6)*t -(7.5*a- 1./8 ))*t - (OVER_12))*t +(0.5*a-1./24 ); break;
  case 2: result = ((-(10*a+   0.5)*t +( 15*a+ 5./12))*t +(OVER_2_3))*t -(  2*a-OVER_6); break;
  case 3: result = (( (10*a+   0.5)*t -( 15*a+13./12))*t +         0)*t +(  3*a+0.75  ); break;
  case 4: result = ((-( 5*a+OVER_6)*t +(7.5*a+ 5./8 ))*t -(OVER_2_3))*t -(  2*a-OVER_6); break;
  case 5: result = (( (   a       )*t -(1.5*a+ 1./24))*t + (OVER_12))*t +(0.5*a-1./24 ); break;
  default: result = 0;
  }
  return result;
}

float dn_c2_1ef(float a, float t) {

  return d0_c2_1ef(a, t);
}

float dn_c2_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (( OVER_6*t +  0)*t +  0)*t +       0; break;
  case 1: result = ((   -0.5*t +0.5)*t +0.5)*t +  OVER_6; break;
  case 2: result = ((    0.5*t -  1)*t +  0)*t +OVER_2_3; break;
  case 3: result = ((-OVER_6*t +0.5)*t -0.5)*t +  OVER_6; break;
  default: result = 0;
  }
  return result;
}

float dn_c2_3ef(float a, float t) {

  return d0_c2_3ef(a, t);
}

float dn_c2_4ef(float a, float t) {

  return dn_c1_4ef(1./36, t);
}

float dn_c3_1ef(float a, float t) {

  return d0_c3_1ef(a, t);
}

float dn_c3_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result =    (-0.10*t +0.25)*t*t*t*t;                       break;
  case 1: result = ((( 0.30*t -0.75)*t*t +0.5)*t +0.5)*t +0.15; break;
  case 2: result = (((-0.30*t +0.75)*t*t -1  )*t +0  )*t +0.70; break;
  case 3: result = ((( 0.10*t -0.25)*t*t +0.5)*t -0.5)*t +0.15; break;
  default: result = 0;
  }
  return result;
}

float dn_c3_3ef(float a, float t) {

  return d0_c3_3ef(a, t);
}

float dn_c3_4ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((((  1./30*t - 1./16)*t +      0)*t +       0)*t +       0)*t + 0;      break;
  case 1: result = ((((-OVER_6*t +17./48)*t +OVER_12)*t -   1./24)*t - OVER_12)*t - 7./240; break;
  case 2: result = (((( OVER_3*t -19./24)*t -OVER_6 )*t +OVER_2_3)*t +OVER_2_3)*t + 7./60;  break;
  case 3: result = ((((-OVER_3*t + 7./8 )*t +      0)*t -    1.25)*t +       0)*t +33./40;  break;
  case 4: result = (((( OVER_6*t -23./48)*t +OVER_6 )*t +OVER_2_3)*t -OVER_2_3)*t + 7./60;  break;
  case 5: result = ((((- 1./30*t + 5./48)*t -OVER_12)*t -   1./24)*t + OVER_12)*t - 7./240; break;
  default: result = 0;
  }
  return result;
}

/***************************************************************************/
/*                          Interpolation Filters                          */
/***************************************************************************/

float d0_cn_1ef(float a, float t) {

  return d0_c0_2ef(a, t);
}

float d0_cn_2ef(float a, float t) {

  return d0_c0_2ef(a, t);
}

float d0_cn_3ef(float a, float t) {

  return d0_c0_3ef(a, t);
}

float d0_cn_4ef(float a, float t) {

  return d0_c0_4ef(a, t);
}

float d0_c0_1ef(float a, float t) {

  return d0_c0_2ef(a, t);
}

float d0_c0_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = t;   break;
  case 1: result = 1-t; break;
  default: result = 0;
  }
  return result;
}

float d0_c0_3ef(float a, float t) {

  return dn_cn_3ef(0, t);
}

float d0_c0_4ef(float a, float t) {


  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (( OVER_6*t +0  )*t -OVER_6)*t  ; break;
  case 1: result = ((   -0.5*t +0.5)*t +1     )*t  ; break;
  case 2: result = ((    0.5*t -1  )*t -0.5   )*t+1; break;
  case 3: result = ((-OVER_6*t +0.5)*t -OVER_3)*t  ; break;
  default: result = 0;
  }
  return result;
}

float d0_c1_1ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (-2*t +3)*t*t;    break;
  case 1: result = ( 2*t -3)*t*t +1; break;
  default: result = 0;
  }
  return result;
}

float d0_c1_2ef(float a, float t) {

  return d0_c1_3ef(a, t);
}

float d0_c1_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (( 0.5*t -0.5)*t +0  )*t;     break;
  case 1: result = ((-1.5*t +2  )*t +0.5)*t;    break;
  case 2: result = (( 1.5*t -2.5)*t +0  )*t +1; break;
  case 3: result = ((-0.5*t +1  )*t -0.5)*t;    break;
  default: result = 0;
  }
  return result;
}

float d0_c1_4ef(float a, float t) {

  return dn_c1_4ef(1./12, t);
}

float d0_c2_1ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (( 6*t -15)*t +10)*t*t*t;    break;
  case 1: result = ((-6*t +15)*t -10)*t*t*t +1; break;
  default: result = 0;
  }
  return result;
}

float d0_c2_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((( 0.5*t -0.5)*t +  0)*t +  0)*t;    break;
  case 1: result = (((-0.5*t -0.5)*t +1.5)*t +0.5)*t;    break;
  case 2: result = (((-0.5*t +2.5)*t -  3)*t +  0)*t +1; break;
  case 3: result = ((( 0.5*t -1.5)*t +1.5)*t -0.5)*t;    break;
  default: result = 0;
  }
  return result;
}

float d0_c2_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((((-1*t +2.5)*t -1.5)*t +0  )*t +0  )*t   ; break;
  case 1: result = (((( 3*t -7.5)*t +4.5)*t +0.5)*t +0.5)*t   ; break;
  case 2: result = ((((-3*t +7.5)*t -4.5)*t -1  )*t +0  )*t +1; break;
  case 3: result = (((( 1*t -2.5)*t +1.5)*t +0.5)*t -0.5)*t   ; break;
  default: result = 0;
  }
  return result;
}

float d0_c2_4ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (((-1./12*t + 1./12)*t +   0)*t +       0)*t;    break;
  case 1: result = ((( 1./4 *t + 1./12)*t -1./4)*t - OVER_12)*t;    break;
  case 2: result = (((-1./6 *t - 1    )*t +3./2)*t +OVER_2_3)*t;    break;
  case 3: result = (((-1./6 *t + 5./3 )*t -5./2)*t +       0)*t +1; break;
  case 4: result = ((( 1./4 *t -13./12)*t +3./2)*t -OVER_2_3)*t;    break;
  case 5: result = (((-1./12*t + 1./4 )*t -1./4)*t + OVER_12)*t;    break;
  default: result = 0;
  }
  return result;
}

float d0_c3_1ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (((-20*t +70)*t -84)*t +35)*t*t*t*t;    break;
  case 1: result = ((( 20*t -70)*t +84)*t -35)*t*t*t*t +1; break;
  default: result = 0;
  }
  return result;
}

float d0_c3_2ef(float a, float t) {

  return d0_c3_3ef(a, t);
  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((((-0.75*t +2  )*t -1.25)*t*t +0   )*t +0  )*t   ; break;
  case 1: result = (((( 0.75*t -1.5)*t +0   )*t*t +1.25)*t +0.5)*t   ; break;
  case 2: result = (((( 0.75*t -3  )*t +3.75)*t*t -2.5 )*t +0  )*t +1; break;
  case 3: result = ((((-0.75*t +2.5)*t -2.5 )*t*t +1.25)*t -0.5)*t   ; break;
  default: result = 0;
  }
  return result;
}

float d0_c3_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((((( 3*t -10.5)*t +12.5)*t - 5)*t*t +0  )*t +0  )*t   ; break;
  case 1: result = (((((-9*t +31.5)*t -37.5)*t +15)*t*t +0.5)*t +0.5)*t   ; break;
  case 2: result = ((((( 9*t -31.5)*t +37.5)*t -15)*t*t -1  )*t +0  )*t +1; break;
  case 3: result = (((((-3*t +10.5)*t -12.5)*t + 5)*t*t +0.5)*t -0.5)*t   ; break;
  default: result = 0;
  }
  return result;
}

float d0_c3_4ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((((( 7./48*t - 3./8)*t +11./48)*t +0    )*t + 0    )*t +       0)*t;    break;
  case 1: result = (((((-7./16*t + 1   )*t - 3./8 )*t +1./12)*t - 3./16)*t - OVER_12)*t;    break;
  case 2: result = ((((( 7./24*t - 1./4)*t -19./24)*t -1./6 )*t + 5./4 )*t +OVER_2_3)*t;    break;
  case 3: result = ((((( 7./24*t - 3./2)*t + 7./3 )*t +0    )*t -17./8 )*t +       0)*t +1; break;
  case 4: result = (((((-7./16*t +13./8)*t -31./16)*t +1./6 )*t + 5./4 )*t -OVER_2_3)*t;    break;
  case 5: result = ((((( 7./48*t - 1./2)*t +13./24)*t -1./12)*t - 3./16)*t + OVER_12)*t;    break;
  default: result = 0;
  }
  return result;
}

/***************************************************************************/
/*                        First Derivative Filters                         */
/***************************************************************************/


float d1_cn_1ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result =  1; break;
  case 1: result = -1; break;
  default: result = 0;
  }
  return result;
}

float d1_cn_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result =  0.5*t +(  a    ); break;
  case 1: result = -0.5*t -(3*a-0.5); break;
  case 2: result = -0.5*t +(3*a    ); break;
  case 3: result =  0.5*t -(  a+0.5); break;
  default: result = 0;
  }
  return result;
}

float d1_cn_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ( 0.5*t +0)*t -OVER_6; break;
  case 1: result = (-1.5*t +1)*t +1; break;
  case 2: result = ( 1.5*t -2)*t -0.5; break;
  case 3: result = (-0.5*t +1)*t -OVER_3; break;
  default: result = 0;
  }
  return result;
}

float d1_cn_4ef(float a, float t) {

  return d1_c0_4ef(a, t);
}

float d1_c0_1ef(float a, float t) {

  return d1_c0_2ef(a, t);
}

float d1_c0_2ef(float a, float t) {

  return d1_cn_2ef(0, t);
}

float d1_c0_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ( (   a    )*t -(   a+OVER_12))*t +0;        break;
  case 1: result = (-( 5*a-0.5)*t +( 5*a+   0.25))*t -OVER_12;  break;
  case 2: result = ( (10*a-1.5)*t -(10*a-   5./6))*t +OVER_2_3; break;
  case 3: result = (-(10*a-1.5)*t +(10*a-  13./6))*t +0;        break;
  case 4: result = ( ( 5*a-0.5)*t -( 5*a-   1.25))*t -OVER_2_3; break;
  case 5: result = (-(   a    )*t +(   a-OVER_12))*t +OVER_12;  break;
  default: result = 0;
  }
  return result;
}

float d1_c0_4ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((OVER_12*t +(   a     ))*t -(   a+ OVER_6))*t +0;        break;
  case 1: result = ((  -0.25*t -( 5*a-0.25))*t +( 5*a+   0.75))*t -OVER_12;  break;
  case 2: result = (( OVER_6*t +(10*a- 0.5))*t -(10*a+ OVER_3))*t +OVER_2_3; break;
  case 3: result = (( OVER_6*t -(10*a     ))*t +(10*a-   5./6))*t +0;        break;
  case 4: result = ((  -0.25*t +( 5*a+ 0.5))*t -( 5*a-    0.5))*t -OVER_2_3; break;
  case 5: result = ((OVER_12*t -(   a+0.25))*t +(   a+OVER_12))*t +OVER_12;  break;
  default: result = 0;
  }
  return result;
}

float d1_c1_1ef(float a, float t) {

  return d1_c1_2ef(a, t);
}

float d1_c1_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ( 0.5*t +0)*t +0; break;
  case 1: result = (-1.5*t +1)*t +0.5; break;
  case 2: result = ( 1.5*t -2)*t +0; break;
  case 3: result = (-0.5*t +1)*t -0.5; break;
  default: result = 0;
  }
  return result;
}

float d1_c1_3ef(float a, float t) {

  return d1_c0_3ef(-1./12, t);
}

float d1_c1_4ef(float a, float t) {

  return d1_c0_4ef(-1./6, t);
}

float d1_c2_1ef(float a, float t) {

  return d1_c2_2ef(a, t);
}

float d1_c2_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((-0.5*t +1)*t*t +0)*t +0; break;
  case 1: result = (( 1.5*t -3)*t*t +1)*t +0.5; break;
  case 2: result = ((-1.5*t +3)*t*t -2)*t +0; break;
  case 3: result = (( 0.5*t -1)*t*t +1)*t -0.5; break;
  default: result = 0;
  }
  return result;
}

float d1_c2_3ef(float a, float t) {

  return d1_c2_4ef(a, t);
}

float d1_c2_4ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((( OVER_6*t -  0.25)*t +   0)*t +      0)*t +0;        break;
  case 1: result = (((-  5./6*t +17./12)*t +0.25)*t -OVER_12)*t -OVER_12;  break;
  case 2: result = (((   5./3*t - 19./6)*t -0.5 )*t +   4./3)*t +OVER_2_3; break;
  case 3: result = (((-  5./3*t +   3.5)*t +0   )*t -    2.5)*t +0;        break;
  case 4: result = (((   5./6*t -23./12)*t +0.5 )*t +   4./3)*t -OVER_2_3; break;
  case 5: result = (((-OVER_6*t + 5./12)*t -0.25)*t -OVER_12)*t +OVER_12;  break;
  default: result = 0;
  }
  return result;
}

float d1_c3_1ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (((-0.75*t +1.25)*t +  0)*t*t +   0)*t +0;   break;
  case 1: result = ((( 0.75*t +   0)*t -2.5)*t*t +1.25)*t +0.5; break;
  case 2: result = ((( 0.75*t -3.75)*t +5  )*t*t -2.5 )*t +0;   break;
  case 3: result = (((-0.75*t +2.5 )*t -2.5)*t*t +1.25)*t -0.5; break;
  default: result = 0;
  }
  return result;
}

float d1_c3_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((( 1*t -3)*t +2.5)*t*t*t +0)*t +0;   break;
  case 1: result = (((-3*t +9)*t -7.5)*t*t*t +1)*t +0.5; break;
  case 2: result = ((( 3*t -9)*t +7.5)*t*t*t -2)*t +0;   break;
  case 3: result = (((-1*t +3)*t -2.5)*t*t*t +1)*t -0.5; break;
  default: result = 0;
  }
  return result;
}

float d1_c3_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (((( 3./16*t -  13./48)*t +     0)*t +   0)*t +     0)*t +0;        break;
  case 1: result = ((((-9./16*t +   5./12)*t +19./24)*t +0.25)*t - 7./48)*t -OVER_12;  break;
  case 2: result = (((( 3./8 *t +  25./24)*t -19./6 )*t - 0.5)*t +19./12)*t +OVER_2_3; break;
  case 3: result = (((( 3./8 *t -  35./12)*t +19./4 )*t +   0)*t -23./8 )*t +0;        break;
  case 4: result = ((((-9./16*t + 115./48)*t -19./6 )*t + 0.5)*t +19./12)*t -OVER_2_3; break;
  case 5: result = (((( 3./16*t -OVER_2_3)*t +19./24)*t -0.25)*t - 7./48)*t +OVER_12;  break;
  default: result = 0;
  }
  return result;
}

float d1_c3_4ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (((((-0.25*t +0.75)*t - 7./12)*t +       0)*t +   0)*t +      0)*t +0;        break;
  case 1: result = ((((( 1.25*t -3.75)*t +35./12)*t +  OVER_6)*t +0.25)*t -OVER_12)*t -OVER_12;  break;
  case 2: result = ((((( -2.5*t + 7.5)*t -35./6 )*t -OVER_2_3)*t - 0.5)*t +   4./3)*t +OVER_2_3; break;
  case 3: result = (((((  2.5*t - 7.5)*t +35./6 )*t +       1)*t +   0)*t -   5./2)*t +0;        break;
  case 4: result = (((((-1.25*t +3.75)*t -35./12)*t -OVER_2_3)*t + 0.5)*t +   4./3)*t -OVER_2_3; break;
  case 5: result = ((((( 0.25*t -0.75)*t + 7./12)*t +  OVER_6)*t -0.25)*t -OVER_12)*t +OVER_12;  break;
  default: result = 0;
  }
  return result;
}


/***************************************************************************/
/*                       Second Derivative Filters                         */
/***************************************************************************/


float d2_cn_1ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result =  0.5; break;
  case 1: result = -0.5; break;
  case 2: result = -0.5; break;
  case 3: result =  0.5; break;
  default: result = 0;
  }
  return result;
}

float d2_cn_2ef(float a, float t) {

  return d2_c0_2ef(a, t);
}

float d2_cn_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ( 0.25*t +(a-30)/120)*t -(a+10)/240; break;
  case 1: result = (-0.75*t -(a-42)/24 )*t +(a+ 6)/48;  break;
  case 2: result = ( 0.5 *t +(a-42)/12 )*t -(a-22)/24;  break;
  case 3: result = ( 0.5 *t -(a-30)/12 )*t +(a-50)/24;  break;
  case 4: result = (-0.75*t +(a- 6)/24 )*t -(a-54)/48;  break;
  case 5: result = ( 0.25*t -(a+30)/120)*t +(a-10)/240; break;
  default: result = 0;
  }
  return result;
}

float d2_cn_4ef(float a, float t) {

  return d2_c0_4ef(a, t);
}

float d2_c0_1ef(float a, float t) {

  return d2_c0_2ef(a, t);
}

float d2_c0_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result =    t;    break;
  case 1: result = -3*t +1; break;
  case 2: result =  3*t -2; break;
  case 3: result = -  t +1; break;
  default: result = 0;
  }
  return result;
}

float d2_c0_3ef(float a, float t) {

  return d2_cn_3ef(-10, t);
}

float d2_c0_4ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (( 1./6*t +0  )*t -0.25)*t +    0; break;
  case 1: result = ((-5./6*t +0.5)*t +1.75)*t -1./12; break;
  case 2: result = (( 5./3*t -2  )*t -3.5 )*t + 4./3; break;
  case 3: result = ((-5./3*t +3  )*t +2.5 )*t -  2.5; break;
  case 4: result = (( 5./6*t -2  )*t -0.25)*t + 4./3; break;
  case 5: result = ((-1./6*t +0.5)*t -0.25)*t -1./12; break;
  default: result = 0;
  }
  return result;
}

float d2_c1_1ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (-2*t +3)*t*t +0; break;
  case 1: result = ( 6*t -9)*t*t +1; break;
  case 2: result = (-6*t +9)*t*t -2; break;
  case 3: result = ( 2*t -3)*t*t +1; break;
  default: result = 0;
  }
  return result;
}

float d2_c1_2ef(float a, float t) {
  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ( 0.25*t +0  )*t;       break;
  case 1: result = (-0.75*t +0.5)*t +0.25; break;
  case 2: result = ( 0.5 *t -1  )*t;       break;
  case 3: result = ( 0.5 *t +0  )*t -0.5;  break;
  case 4: result = (-0.75*t +1  )*t;       break;
  case 5: result = ( 0.25*t -0.5)*t +0.25; break;
  default: result = 0;
  }
  return result;
}

float d2_c1_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((  2./3*t - 0.75)*t +0  )*t;        break;
  case 1: result = ((-10./3*t + 4.25)*t +0.5)*t -1./12; break;
  case 2: result = (( 20./3*t - 9.5 )*t -1  )*t +4./3;  break;
  case 3: result = ((-20./3*t +10.5 )*t +0  )*t -2.5;   break;
  case 4: result = (( 10./3*t - 5.75)*t +1  )*t +4./3;  break;
  case 5: result = ((- 2./3*t + 1.25)*t -0.5)*t -1./12; break;
  default: result = 0;
  }
  return result;
}

float d2_c1_4ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((-(  a+ 53)/360*t +(  a+ 38)/240)*t +0    )*t; break;
  case 1: result = (( (7*a+431)/360*t -(7*a+296)/240)*t - 1./8)*t +(a+ 8)/720; break;
  case 2: result = ((-(7*a+471)/120*t +(7*a+366)/80 )*t +1    )*t -(a+18)/120; break;
  case 3: result = (( (7*a+491)/72 *t -(7*a+452)/48 )*t -13./8)*t +(a+72)/48 ; break;
  case 4: result = ((-(7*a+491)/72 *t +(7*a+530)/48 )*t +0    )*t -(a+98)/36 ; break;
  case 5: result = (( (7*a+471)/120*t -(7*a+576)/80 )*t +13./8)*t +(a+72)/48 ; break;
  case 6: result = ((-(7*a+431)/360*t +(7*a+566)/240)*t -1    )*t -(a+18)/120; break;
  case 7: result = (( (  a+ 53)/360*t -(  a+ 68)/240)*t + 1./8)*t +(a+ 8)/720; break;
  default: result = 0;
  }
  return result;
}

float d2_c2_1ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((  6*t -15)*t +10)*t*t*t;    break;
  case 1: result = ((-18*t +45)*t -30)*t*t*t +1; break;
  case 2: result = (( 18*t -45)*t +30)*t*t*t -2; break;
  case 3: result = ((- 6*t +15)*t -10)*t*t*t +1; break;
  default: result = 0;
  }
  return result;
}

float d2_c2_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (( 1./6*t +0  )*t +0  )*t; break;
  case 1: result = ((-5./6*t +0.5)*t +0.5)*t +1./6; break;
  case 2: result = (( 5./3*t -2  )*t -1  )*t +1./3; break;
  case 3: result = ((-5./3*t +3  )*t +0  )*t -1;    break;
  case 4: result = (( 5./6*t -2  )*t +1  )*t +1./3; break;
  case 5: result = ((-1./6*t +0.5)*t -0.5)*t +1./6; break;
  default: result = 0;
  }
  return result;
}

float d2_c2_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((((-1.5*t + 3.75)*t - 7./3)*t +0  )*t +0  )*t; break;
  case 1: result = (((( 7.5*t -18.75)*t +35./3)*t +0.5)*t +0.5)*t -1./12; break;
  case 2: result = ((((- 15*t +37.5 )*t -70./3)*t -2  )*t -1  )*t +4./3 ; break;
  case 3: result = ((((  15*t -37.5 )*t +70./3)*t +3  )*t +0  )*t -2.5  ; break;
  case 4: result = ((((-7.5*t +18.75)*t -35./3)*t -2  )*t +1  )*t +4./3 ; break;
  case 5: result = (((( 1.5*t - 3.75)*t + 7./3)*t +0.5)*t -0.5)*t -1./12; break;
  default: result = 0;
  }
  return result;
}

float d2_c2_4ef(float a, float t) {

  return d2_c1_4ef(-38, t);
}

float d2_c3_1ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (((-20*t + 70)*t - 84)*t + 35)*t*t*t*t; break;
  case 1: result = ((( 60*t -210)*t +252)*t -105)*t*t*t*t +1; break;
  case 2: result = (((-60*t +210)*t -252)*t +105)*t*t*t*t -2; break;
  case 3: result = ((( 20*t - 70)*t + 84)*t - 35)*t*t*t*t +1; break;
  default: result = 0;
  }
  return result;
}

float d2_c3_2ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (((-0.1*t +0.25)*t*t +  0)*t +0  )*t; break;
  case 1: result = ((( 0.5*t -1.25)*t*t +0.5)*t +0.5)*t +3./20; break;
  case 2: result = (((-1  *t +2.5 )*t*t -2  )*t -1  )*t +2./5; break;
  case 3: result = ((( 1  *t -2.5 )*t*t +3  )*t +0  )*t -11./10; break;
  case 4: result = (((-0.5*t +1.25)*t*t -2  )*t +1  )*t +2./5; break;
  case 5: result = ((( 0.1*t -0.25)*t*t +0.5)*t -0.5)*t +3./20; break;
  default: result = 0;
  }
  return result;
}

float d2_c3_3ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = (((((  14./3*t - 49./3)*t + 39./2)*t - 95./12)*t*t +0  )*t +0  )*t; break;
  case 1: result = (((((- 70./3*t +245./3)*t -195./2)*t +475./12)*t*t +0.5)*t +0.5)*t -1./12; break;
  case 2: result = ((((( 140./3*t -490./3)*t +195   )*t -475./6 )*t*t -2  )*t -1  )*t +4./3; break;
  case 3: result = (((((-140./3*t +490./3)*t -195   )*t +475./6 )*t*t +3  )*t +0  )*t -5./2; break;
  case 4: result = (((((  70./3*t -245./3)*t +195./2)*t -475./12)*t*t -2  )*t +1  )*t +4./3; break;
  case 5: result = (((((- 14./3*t + 49./3)*t - 39./2)*t + 95./12)*t*t +0.5)*t -0.5)*t -1./12; break;
  default: result = 0;
  }
  return result;
}

float d2_c3_4ef(float a, float t) {

  float result;
  int i;

  i = (t<0) ? (int)t-1:(int)t;
  t = t - i;

  switch (i) {
  case 0: result = ((((  1./24*t - 1./12)*t +0    )*t +0    )*t +0    )*t; break;
  case 1: result = ((((- 7./24*t + 5./8 )*t +1./12)*t -1./12)*t - 1./8)*t - 1./24; break;
  case 2: result = ((((  7./8 *t - 2    )*t -1./3 )*t +1    )*t +1    )*t + 1./6;  break;
  case 3: result = ((((-35./24*t +85./24)*t +5./12)*t -13./4)*t -13./8)*t +17./24; break;
  case 4: result = (((( 35./24*t -15./4 )*t +0    )*t +14./3)*t +0    )*t - 5./3;  break;
  case 5: result = ((((- 7./8 *t +19./8 )*t -5./12)*t -13./4)*t +13./8)*t +17./24; break;
  case 6: result = ((((  7./24*t - 5./6 )*t +1./3 )*t +1    )*t -1    )*t + 1./6;  break;
  case 7: result = ((((- 1./24*t + 1./8 )*t -1./12)*t -1./12)*t + 1./8)*t - 1./24; break;
  default: result = 0;
  }
  return result;
}
