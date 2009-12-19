/*
  Teem: Gordon Kindlmann's research software
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "nrrd.h"

 /* ************************************************* */
 /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */
 /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */
 /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */
 /*                                                   */
 /*                                                   */
 /*       THIS FILE AUTOMATICALLY GENERATED FROM      */
 /*        PERL SCRIPTS IN THE tmf SUBDIRECTORY       */
 /*         EDIT THOSE SCRIPTS, NOT THIS FILE!        */
 /*                                                   */
 /*                                                   */
 /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */
 /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */
 /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */
 /* ************************************************* */

double
_nrrd_TMFBAD_Int(const double *parm) {
  AIR_UNUSED(parm);
  fprintf(stderr, "_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\n");
  return 0.0;
}

double
_nrrd_TMFBAD_Sup(const double *parm) {
  AIR_UNUSED(parm);
  fprintf(stderr, "_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\n");
  return 0.0;
}

double
_nrrd_TMFBAD_1_d(double x, const double *parm) {
  AIR_UNUSED(x);
  AIR_UNUSED(parm);
  fprintf(stderr, "_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\n");
  return 0.0;
}

float
_nrrd_TMFBAD_1_f(float x, const double *parm) {
  AIR_UNUSED(x);
  AIR_UNUSED(parm);
  fprintf(stderr, "_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\n");
  return 0.0;
}

void
_nrrd_TMFBAD_N_d(double *f, const double *x, size_t len, const double *parm) {
  AIR_UNUSED(f);
  AIR_UNUSED(x);
  AIR_UNUSED(len);
  AIR_UNUSED(parm);
  fprintf(stderr, "_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\n");
}

void
_nrrd_TMFBAD_N_f(float *f, const float *x, size_t len, const double *parm) {
  AIR_UNUSED(f);
  AIR_UNUSED(x);
  AIR_UNUSED(len);
  AIR_UNUSED(parm);
  fprintf(stderr, "_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\n");
}

NrrdKernel
_nrrdKernel_TMFBAD = {
  "TMFBAD",
  1, _nrrd_TMFBAD_Sup, _nrrd_TMFBAD_Int,
  _nrrd_TMFBAD_1_f, _nrrd_TMFBAD_N_f,
  _nrrd_TMFBAD_1_d, _nrrd_TMFBAD_N_d
};
#define OVER_3 0.33333333
#define OVER_6 0.16666666
#define OVER_12 0.0833333333
#define OVER_2_3 0.6666666666

/* ------------------------ TMF_dn_cn_1ef --------------------- */

#define TMF_dn_cn_1ef(a, i, t) ( \
  (i == 0 ? 0.5 : \
  (i == 1 ? 0.5 : \
  0)))


/* ------------------------ TMF_dn_cn_2ef --------------------- */

#define TMF_dn_cn_2ef(a, i, t) ( \
  TMF_d0_c0_2ef((double)(a), i, t))


/* ------------------------ TMF_dn_cn_3ef --------------------- */

#define TMF_dn_cn_3ef(a, i, t) ( \
  (i == 0 ? ( 0.25*t +(2*a-0.25))*t -   a   : \
  (i == 1 ? (-0.25*t -(6*a-1.25))*t + 3*a   : \
  (i == 2 ? (-0.25*t +(6*a-0.75))*t - 3*a+1 : \
  (i == 3 ? ( 0.25*t -(2*a+0.25))*t +   a   : \
  0)))))


/* ------------------------ TMF_dn_cn_4ef --------------------- */

#define TMF_dn_cn_4ef(a, i, t) ( \
  TMF_d0_c0_4ef((double)(a), i, t))


/* ------------------------ TMF_dn_c0_1ef --------------------- */

#define TMF_dn_c0_1ef(a, i, t) ( \
  TMF_d0_c0_2ef((double)(a), i, t))


/* ------------------------ TMF_dn_c0_2ef --------------------- */

#define TMF_dn_c0_2ef(a, i, t) ( \
  TMF_d0_c0_2ef((double)(a), i, t))


/* ------------------------ TMF_dn_c0_3ef --------------------- */

#define TMF_dn_c0_3ef(a, i, t) ( \
  TMF_d0_c0_3ef((double)(a), i, t))


/* ------------------------ TMF_dn_c0_4ef --------------------- */

#define TMF_dn_c0_4ef(a, i, t) ( \
  TMF_d0_c0_4ef((double)(a), i, t))


/* ------------------------ TMF_dn_c1_1ef --------------------- */

#define TMF_dn_c1_1ef(a, i, t) ( \
  TMF_d0_c1_1ef((double)(a), i, t))


/* ------------------------ TMF_dn_c1_2ef --------------------- */

#define TMF_dn_c1_2ef(a, i, t) ( \
  (i == 0 ? ( t   )*t/4 : \
  (i == 1 ? (-t +2)*t/4 +0.25 : \
  (i == 2 ? (-t   )*t/4 +0.5 : \
  (i == 3 ? ( t -2)*t/4 +0.25 : \
  0)))))


/* ------------------------ TMF_dn_c1_3ef --------------------- */

#define TMF_dn_c1_3ef(a, i, t) ( \
  TMF_d0_c1_3ef((double)(a), i, t))


/* ------------------------ TMF_dn_c1_4ef --------------------- */

#define TMF_dn_c1_4ef(a, i, t) ( \
  (i == 0 ? ((-(   a       )*t +(1.5*a- 1./24))*t +         0)*t +             0 : \
  (i == 1 ? (( ( 5*a+OVER_6)*t -(7.5*a- 1./8 ))*t - (OVER_12))*t +(0.5*a-1./24 ) : \
  (i == 2 ? ((-(10*a+   0.5)*t +( 15*a+ 5./12))*t +(OVER_2_3))*t -(  2*a-OVER_6) : \
  (i == 3 ? (( (10*a+   0.5)*t -( 15*a+13./12))*t +         0)*t +(  3*a+0.75  ) : \
  (i == 4 ? ((-( 5*a+OVER_6)*t +(7.5*a+ 5./8 ))*t -(OVER_2_3))*t -(  2*a-OVER_6) : \
  (i == 5 ? (( (   a       )*t -(1.5*a+ 1./24))*t + (OVER_12))*t +(0.5*a-1./24 ) : \
  0)))))))


/* ------------------------ TMF_dn_c2_1ef --------------------- */

#define TMF_dn_c2_1ef(a, i, t) ( \
  TMF_d0_c2_1ef((double)(a), i, t))


/* ------------------------ TMF_dn_c2_2ef --------------------- */

#define TMF_dn_c2_2ef(a, i, t) ( \
  (i == 0 ? (( OVER_6*t +  0)*t +  0)*t +       0 : \
  (i == 1 ? ((   -0.5*t +0.5)*t +0.5)*t +  OVER_6 : \
  (i == 2 ? ((    0.5*t -  1)*t +  0)*t +OVER_2_3 : \
  (i == 3 ? ((-OVER_6*t +0.5)*t -0.5)*t +  OVER_6 : \
  0)))))


/* ------------------------ TMF_dn_c2_3ef --------------------- */

#define TMF_dn_c2_3ef(a, i, t) ( \
  TMF_d0_c2_3ef((double)(a), i, t))


/* ------------------------ TMF_dn_c2_4ef --------------------- */

#define TMF_dn_c2_4ef(a, i, t) ( \
  TMF_dn_c1_4ef((double)(1./36), i, t))


/* ------------------------ TMF_dn_c3_1ef --------------------- */

#define TMF_dn_c3_1ef(a, i, t) ( \
  TMF_d0_c3_1ef((double)(a), i, t))


/* ------------------------ TMF_dn_c3_2ef --------------------- */

#define TMF_dn_c3_2ef(a, i, t) ( \
  (i == 0 ? (-0.10*t +0.25)*t*t*t*t : \
  (i == 1 ? ((( 0.30*t -0.75)*t*t +0.5)*t +0.5)*t +0.15 : \
  (i == 2 ? (((-0.30*t +0.75)*t*t -1  )*t +0  )*t +0.70 : \
  (i == 3 ? ((( 0.10*t -0.25)*t*t +0.5)*t -0.5)*t +0.15 : \
  0)))))


/* ------------------------ TMF_dn_c3_3ef --------------------- */

#define TMF_dn_c3_3ef(a, i, t) ( \
  TMF_d0_c3_3ef((double)(a), i, t))


/* ------------------------ TMF_dn_c3_4ef --------------------- */

#define TMF_dn_c3_4ef(a, i, t) ( \
  (i == 0 ? ((((  1./30*t - 1./16)*t +      0)*t +       0)*t +       0)*t + 0 : \
  (i == 1 ? ((((-OVER_6*t +17./48)*t +OVER_12)*t -   1./24)*t - OVER_12)*t - 7./240 : \
  (i == 2 ? (((( OVER_3*t -19./24)*t -OVER_6 )*t +OVER_2_3)*t +OVER_2_3)*t + 7./60 : \
  (i == 3 ? ((((-OVER_3*t + 7./8 )*t +      0)*t -    1.25)*t +       0)*t +33./40 : \
  (i == 4 ? (((( OVER_6*t -23./48)*t +OVER_6 )*t +OVER_2_3)*t -OVER_2_3)*t + 7./60 : \
  (i == 5 ? ((((- 1./30*t + 5./48)*t -OVER_12)*t -   1./24)*t + OVER_12)*t - 7./240 : \
  0)))))))


/* ------------------------ TMF_d0_cn_1ef --------------------- */

#define TMF_d0_cn_1ef(a, i, t) ( \
  TMF_d0_c0_2ef((double)(a), i, t))


/* ------------------------ TMF_d0_cn_2ef --------------------- */

#define TMF_d0_cn_2ef(a, i, t) ( \
  TMF_d0_c0_2ef((double)(a), i, t))


/* ------------------------ TMF_d0_cn_3ef --------------------- */

#define TMF_d0_cn_3ef(a, i, t) ( \
  TMF_d0_c0_3ef((double)(a), i, t))


/* ------------------------ TMF_d0_cn_4ef --------------------- */

#define TMF_d0_cn_4ef(a, i, t) ( \
  TMF_d0_c0_4ef((double)(a), i, t))


/* ------------------------ TMF_d0_c0_1ef --------------------- */

#define TMF_d0_c0_1ef(a, i, t) ( \
  TMF_d0_c0_2ef((double)(a), i, t))


/* ------------------------ TMF_d0_c0_2ef --------------------- */

#define TMF_d0_c0_2ef(a, i, t) ( \
  (i == 0 ? t : \
  (i == 1 ? 1-t : \
  0)))


/* ------------------------ TMF_d0_c0_3ef --------------------- */

#define TMF_d0_c0_3ef(a, i, t) ( \
  TMF_dn_cn_3ef((double)(0), i, t))


/* ------------------------ TMF_d0_c0_4ef --------------------- */

#define TMF_d0_c0_4ef(a, i, t) ( \
  (i == 0 ? (( OVER_6*t +0  )*t -OVER_6)*t   : \
  (i == 1 ? ((   -0.5*t +0.5)*t +1     )*t   : \
  (i == 2 ? ((    0.5*t -1  )*t -0.5   )*t+1 : \
  (i == 3 ? ((-OVER_6*t +0.5)*t -OVER_3)*t   : \
  0)))))


/* ------------------------ TMF_d0_c1_1ef --------------------- */

#define TMF_d0_c1_1ef(a, i, t) ( \
  (i == 0 ? (-2*t +3)*t*t : \
  (i == 1 ? ( 2*t -3)*t*t +1 : \
  0)))


/* ------------------------ TMF_d0_c1_2ef --------------------- */

#define TMF_d0_c1_2ef(a, i, t) ( \
  TMF_d0_c1_3ef((double)(a), i, t))


/* ------------------------ TMF_d0_c1_3ef --------------------- */

#define TMF_d0_c1_3ef(a, i, t) ( \
  (i == 0 ? (( 0.5*t -0.5)*t +0  )*t : \
  (i == 1 ? ((-1.5*t +2  )*t +0.5)*t : \
  (i == 2 ? (( 1.5*t -2.5)*t +0  )*t +1 : \
  (i == 3 ? ((-0.5*t +1  )*t -0.5)*t : \
  0)))))


/* ------------------------ TMF_d0_c1_4ef --------------------- */

#define TMF_d0_c1_4ef(a, i, t) ( \
  TMF_dn_c1_4ef((double)(1./12), i, t))


/* ------------------------ TMF_d0_c2_1ef --------------------- */

#define TMF_d0_c2_1ef(a, i, t) ( \
  (i == 0 ? (( 6*t -15)*t +10)*t*t*t : \
  (i == 1 ? ((-6*t +15)*t -10)*t*t*t +1 : \
  0)))


/* ------------------------ TMF_d0_c2_2ef --------------------- */

#define TMF_d0_c2_2ef(a, i, t) ( \
  (i == 0 ? ((( 0.5*t -0.5)*t +  0)*t +  0)*t : \
  (i == 1 ? (((-0.5*t -0.5)*t +1.5)*t +0.5)*t : \
  (i == 2 ? (((-0.5*t +2.5)*t -  3)*t +  0)*t +1 : \
  (i == 3 ? ((( 0.5*t -1.5)*t +1.5)*t -0.5)*t : \
  0)))))


/* ------------------------ TMF_d0_c2_3ef --------------------- */

#define TMF_d0_c2_3ef(a, i, t) ( \
  (i == 0 ? ((((-1*t +2.5)*t -1.5)*t +0  )*t +0  )*t    : \
  (i == 1 ? (((( 3*t -7.5)*t +4.5)*t +0.5)*t +0.5)*t    : \
  (i == 2 ? ((((-3*t +7.5)*t -4.5)*t -1  )*t +0  )*t +1 : \
  (i == 3 ? (((( 1*t -2.5)*t +1.5)*t +0.5)*t -0.5)*t    : \
  0)))))


/* ------------------------ TMF_d0_c2_4ef --------------------- */

#define TMF_d0_c2_4ef(a, i, t) ( \
  (i == 0 ? (((-1./12*t + 1./12)*t +   0)*t +       0)*t : \
  (i == 1 ? ((( 1./4 *t + 1./12)*t -1./4)*t - OVER_12)*t : \
  (i == 2 ? (((-1./6 *t - 1    )*t +3./2)*t +OVER_2_3)*t : \
  (i == 3 ? (((-1./6 *t + 5./3 )*t -5./2)*t +       0)*t +1 : \
  (i == 4 ? ((( 1./4 *t -13./12)*t +3./2)*t -OVER_2_3)*t : \
  (i == 5 ? (((-1./12*t + 1./4 )*t -1./4)*t + OVER_12)*t : \
  0)))))))


/* ------------------------ TMF_d0_c3_1ef --------------------- */

#define TMF_d0_c3_1ef(a, i, t) ( \
  (i == 0 ? (((-20*t +70)*t -84)*t +35)*t*t*t*t : \
  (i == 1 ? ((( 20*t -70)*t +84)*t -35)*t*t*t*t +1 : \
  0)))


/* ------------------------ TMF_d0_c3_2ef --------------------- */

#define TMF_d0_c3_2ef(a, i, t) ( \
  TMF_d0_c3_3ef((double)(a), i, t))


/* ------------------------ TMF_d0_c3_3ef --------------------- */

#define TMF_d0_c3_3ef(a, i, t) ( \
  (i == 0 ? ((((( 3*t -10.5)*t +12.5)*t - 5)*t*t +0  )*t +0  )*t    : \
  (i == 1 ? (((((-9*t +31.5)*t -37.5)*t +15)*t*t +0.5)*t +0.5)*t    : \
  (i == 2 ? ((((( 9*t -31.5)*t +37.5)*t -15)*t*t -1  )*t +0  )*t +1 : \
  (i == 3 ? (((((-3*t +10.5)*t -12.5)*t + 5)*t*t +0.5)*t -0.5)*t    : \
  0)))))


/* ------------------------ TMF_d0_c3_4ef --------------------- */

#define TMF_d0_c3_4ef(a, i, t) ( \
  (i == 0 ? ((((( 7./48*t - 3./8)*t +11./48)*t +0    )*t + 0    )*t +       0)*t : \
  (i == 1 ? (((((-7./16*t + 1   )*t - 3./8 )*t +1./12)*t - 3./16)*t - OVER_12)*t : \
  (i == 2 ? ((((( 7./24*t - 1./4)*t -19./24)*t -1./6 )*t + 5./4 )*t +OVER_2_3)*t : \
  (i == 3 ? ((((( 7./24*t - 3./2)*t + 7./3 )*t +0    )*t -17./8 )*t +       0)*t +1 : \
  (i == 4 ? (((((-7./16*t +13./8)*t -31./16)*t +1./6 )*t + 5./4 )*t -OVER_2_3)*t : \
  (i == 5 ? ((((( 7./48*t - 1./2)*t +13./24)*t -1./12)*t - 3./16)*t + OVER_12)*t : \
  0)))))))


/* ------------------------ TMF_d1_cn_1ef --------------------- */

#define TMF_d1_cn_1ef(a, i, t) ( \
  (i == 0 ? 1 : \
  (i == 1 ? -1 : \
  0)))


/* ------------------------ TMF_d1_cn_2ef --------------------- */

#define TMF_d1_cn_2ef(a, i, t) ( \
  (i == 0 ? 0.5*t +(  a    ) : \
  (i == 1 ? -0.5*t -(3*a-0.5) : \
  (i == 2 ? -0.5*t +(3*a    ) : \
  (i == 3 ? 0.5*t -(  a+0.5) : \
  0)))))


/* ------------------------ TMF_d1_cn_3ef --------------------- */

#define TMF_d1_cn_3ef(a, i, t) ( \
  (i == 0 ? ( 0.5*t +0)*t -OVER_6 : \
  (i == 1 ? (-1.5*t +1)*t +1 : \
  (i == 2 ? ( 1.5*t -2)*t -0.5 : \
  (i == 3 ? (-0.5*t +1)*t -OVER_3 : \
  0)))))


/* ------------------------ TMF_d1_cn_4ef --------------------- */

#define TMF_d1_cn_4ef(a, i, t) ( \
  TMF_d1_c0_4ef((double)(a), i, t))


/* ------------------------ TMF_d1_c0_1ef --------------------- */

#define TMF_d1_c0_1ef(a, i, t) ( \
  TMF_d1_c0_2ef((double)(a), i, t))


/* ------------------------ TMF_d1_c0_2ef --------------------- */

#define TMF_d1_c0_2ef(a, i, t) ( \
  TMF_d1_cn_2ef((double)(0), i, t))


/* ------------------------ TMF_d1_c0_3ef --------------------- */

#define TMF_d1_c0_3ef(a, i, t) ( \
  (i == 0 ? ( (   a    )*t -(   a+OVER_12))*t +0 : \
  (i == 1 ? (-( 5*a-0.5)*t +( 5*a+   0.25))*t -OVER_12 : \
  (i == 2 ? ( (10*a-1.5)*t -(10*a-   5./6))*t +OVER_2_3 : \
  (i == 3 ? (-(10*a-1.5)*t +(10*a-  13./6))*t +0 : \
  (i == 4 ? ( ( 5*a-0.5)*t -( 5*a-   1.25))*t -OVER_2_3 : \
  (i == 5 ? (-(   a    )*t +(   a-OVER_12))*t +OVER_12 : \
  0)))))))


/* ------------------------ TMF_d1_c0_4ef --------------------- */

#define TMF_d1_c0_4ef(a, i, t) ( \
  (i == 0 ? ((OVER_12*t +(   a     ))*t -(   a+ OVER_6))*t +0 : \
  (i == 1 ? ((  -0.25*t -( 5*a-0.25))*t +( 5*a+   0.75))*t -OVER_12 : \
  (i == 2 ? (( OVER_6*t +(10*a- 0.5))*t -(10*a+ OVER_3))*t +OVER_2_3 : \
  (i == 3 ? (( OVER_6*t -(10*a     ))*t +(10*a-   5./6))*t +0 : \
  (i == 4 ? ((  -0.25*t +( 5*a+ 0.5))*t -( 5*a-    0.5))*t -OVER_2_3 : \
  (i == 5 ? ((OVER_12*t -(   a+0.25))*t +(   a+OVER_12))*t +OVER_12 : \
  0)))))))


/* ------------------------ TMF_d1_c1_1ef --------------------- */

#define TMF_d1_c1_1ef(a, i, t) ( \
  TMF_d1_c1_2ef((double)(a), i, t))


/* ------------------------ TMF_d1_c1_2ef --------------------- */

#define TMF_d1_c1_2ef(a, i, t) ( \
  (i == 0 ? ( 0.5*t +0)*t +0 : \
  (i == 1 ? (-1.5*t +1)*t +0.5 : \
  (i == 2 ? ( 1.5*t -2)*t +0 : \
  (i == 3 ? (-0.5*t +1)*t -0.5 : \
  0)))))


/* ------------------------ TMF_d1_c1_3ef --------------------- */

#define TMF_d1_c1_3ef(a, i, t) ( \
  TMF_d1_c0_3ef((double)(-1./12), i, t))


/* ------------------------ TMF_d1_c1_4ef --------------------- */

#define TMF_d1_c1_4ef(a, i, t) ( \
  TMF_d1_c0_4ef((double)(-1./6), i, t))


/* ------------------------ TMF_d1_c2_1ef --------------------- */

#define TMF_d1_c2_1ef(a, i, t) ( \
  TMF_d1_c2_2ef((double)(a), i, t))


/* ------------------------ TMF_d1_c2_2ef --------------------- */

#define TMF_d1_c2_2ef(a, i, t) ( \
  (i == 0 ? ((-0.5*t +1)*t*t +0)*t +0 : \
  (i == 1 ? (( 1.5*t -3)*t*t +1)*t +0.5 : \
  (i == 2 ? ((-1.5*t +3)*t*t -2)*t +0 : \
  (i == 3 ? (( 0.5*t -1)*t*t +1)*t -0.5 : \
  0)))))


/* ------------------------ TMF_d1_c2_3ef --------------------- */

#define TMF_d1_c2_3ef(a, i, t) ( \
  TMF_d1_c2_4ef((double)(a), i, t))


/* ------------------------ TMF_d1_c2_4ef --------------------- */

#define TMF_d1_c2_4ef(a, i, t) ( \
  (i == 0 ? ((( OVER_6*t -  0.25)*t +   0)*t +      0)*t +0 : \
  (i == 1 ? (((-  5./6*t +17./12)*t +0.25)*t -OVER_12)*t -OVER_12 : \
  (i == 2 ? (((   5./3*t - 19./6)*t -0.5 )*t +   4./3)*t +OVER_2_3 : \
  (i == 3 ? (((-  5./3*t +   3.5)*t +0   )*t -    2.5)*t +0 : \
  (i == 4 ? (((   5./6*t -23./12)*t +0.5 )*t +   4./3)*t -OVER_2_3 : \
  (i == 5 ? (((-OVER_6*t + 5./12)*t -0.25)*t -OVER_12)*t +OVER_12 : \
  0)))))))


/* ------------------------ TMF_d1_c3_1ef --------------------- */

#define TMF_d1_c3_1ef(a, i, t) ( \
  (i == 0 ? (((-0.75*t +1.25)*t +  0)*t*t +   0)*t +0 : \
  (i == 1 ? ((( 0.75*t +   0)*t -2.5)*t*t +1.25)*t +0.5 : \
  (i == 2 ? ((( 0.75*t -3.75)*t +5  )*t*t -2.5 )*t +0 : \
  (i == 3 ? (((-0.75*t +2.5 )*t -2.5)*t*t +1.25)*t -0.5 : \
  0)))))


/* ------------------------ TMF_d1_c3_2ef --------------------- */

#define TMF_d1_c3_2ef(a, i, t) ( \
  (i == 0 ? ((( 1*t -3)*t +2.5)*t*t*t +0)*t +0 : \
  (i == 1 ? (((-3*t +9)*t -7.5)*t*t*t +1)*t +0.5 : \
  (i == 2 ? ((( 3*t -9)*t +7.5)*t*t*t -2)*t +0 : \
  (i == 3 ? (((-1*t +3)*t -2.5)*t*t*t +1)*t -0.5 : \
  0)))))


/* ------------------------ TMF_d1_c3_3ef --------------------- */

#define TMF_d1_c3_3ef(a, i, t) ( \
  (i == 0 ? (((( 3./16*t -  13./48)*t +     0)*t +   0)*t +     0)*t +0 : \
  (i == 1 ? ((((-9./16*t +   5./12)*t +19./24)*t +0.25)*t - 7./48)*t -OVER_12 : \
  (i == 2 ? (((( 3./8 *t +  25./24)*t -19./6 )*t - 0.5)*t +19./12)*t +OVER_2_3 : \
  (i == 3 ? (((( 3./8 *t -  35./12)*t +19./4 )*t +   0)*t -23./8 )*t +0 : \
  (i == 4 ? ((((-9./16*t + 115./48)*t -19./6 )*t + 0.5)*t +19./12)*t -OVER_2_3 : \
  (i == 5 ? (((( 3./16*t -OVER_2_3)*t +19./24)*t -0.25)*t - 7./48)*t +OVER_12 : \
  0)))))))


/* ------------------------ TMF_d1_c3_4ef --------------------- */

#define TMF_d1_c3_4ef(a, i, t) ( \
  (i == 0 ? (((((-0.25*t +0.75)*t - 7./12)*t +       0)*t +   0)*t +      0)*t +0 : \
  (i == 1 ? ((((( 1.25*t -3.75)*t +35./12)*t +  OVER_6)*t +0.25)*t -OVER_12)*t -OVER_12 : \
  (i == 2 ? ((((( -2.5*t + 7.5)*t -35./6 )*t -OVER_2_3)*t - 0.5)*t +   4./3)*t +OVER_2_3 : \
  (i == 3 ? (((((  2.5*t - 7.5)*t +35./6 )*t +       1)*t +   0)*t -   5./2)*t +0 : \
  (i == 4 ? (((((-1.25*t +3.75)*t -35./12)*t -OVER_2_3)*t + 0.5)*t +   4./3)*t -OVER_2_3 : \
  (i == 5 ? ((((( 0.25*t -0.75)*t + 7./12)*t +  OVER_6)*t -0.25)*t -OVER_12)*t +OVER_12 : \
  0)))))))


/* ------------------------ TMF_d2_cn_1ef --------------------- */

#define TMF_d2_cn_1ef(a, i, t) ( \
  (i == 0 ? 0.5 : \
  (i == 1 ? -0.5 : \
  (i == 2 ? -0.5 : \
  (i == 3 ? 0.5 : \
  0)))))


/* ------------------------ TMF_d2_cn_2ef --------------------- */

#define TMF_d2_cn_2ef(a, i, t) ( \
  TMF_d2_c0_2ef((double)(a), i, t))


/* ------------------------ TMF_d2_cn_3ef --------------------- */

#define TMF_d2_cn_3ef(a, i, t) ( \
  (i == 0 ? ( 0.25*t +(a-30)/120)*t -(a+10)/240 : \
  (i == 1 ? (-0.75*t -(a-42)/24 )*t +(a+ 6)/48 : \
  (i == 2 ? ( 0.5 *t +(a-42)/12 )*t -(a-22)/24 : \
  (i == 3 ? ( 0.5 *t -(a-30)/12 )*t +(a-50)/24 : \
  (i == 4 ? (-0.75*t +(a- 6)/24 )*t -(a-54)/48 : \
  (i == 5 ? ( 0.25*t -(a+30)/120)*t +(a-10)/240 : \
  0)))))))


/* ------------------------ TMF_d2_cn_4ef --------------------- */

#define TMF_d2_cn_4ef(a, i, t) ( \
  TMF_d2_c0_4ef((double)(a), i, t))


/* ------------------------ TMF_d2_c0_1ef --------------------- */

#define TMF_d2_c0_1ef(a, i, t) ( \
  TMF_d2_c0_2ef((double)(a), i, t))


/* ------------------------ TMF_d2_c0_2ef --------------------- */

#define TMF_d2_c0_2ef(a, i, t) ( \
  (i == 0 ? t : \
  (i == 1 ? -3*t +1 : \
  (i == 2 ? 3*t -2 : \
  (i == 3 ? -  t +1 : \
  0)))))


/* ------------------------ TMF_d2_c0_3ef --------------------- */

#define TMF_d2_c0_3ef(a, i, t) ( \
  TMF_d2_cn_3ef((double)(-10), i, t))


/* ------------------------ TMF_d2_c0_4ef --------------------- */

#define TMF_d2_c0_4ef(a, i, t) ( \
  (i == 0 ? (( 1./6*t +0  )*t -0.25)*t +    0 : \
  (i == 1 ? ((-5./6*t +0.5)*t +1.75)*t -1./12 : \
  (i == 2 ? (( 5./3*t -2  )*t -3.5 )*t + 4./3 : \
  (i == 3 ? ((-5./3*t +3  )*t +2.5 )*t -  2.5 : \
  (i == 4 ? (( 5./6*t -2  )*t -0.25)*t + 4./3 : \
  (i == 5 ? ((-1./6*t +0.5)*t -0.25)*t -1./12 : \
  0)))))))


/* ------------------------ TMF_d2_c1_1ef --------------------- */

#define TMF_d2_c1_1ef(a, i, t) ( \
  (i == 0 ? (-2*t +3)*t*t +0 : \
  (i == 1 ? ( 6*t -9)*t*t +1 : \
  (i == 2 ? (-6*t +9)*t*t -2 : \
  (i == 3 ? ( 2*t -3)*t*t +1 : \
  0)))))


/* ------------------------ TMF_d2_c1_2ef --------------------- */

#define TMF_d2_c1_2ef(a, i, t) ( \
  (i == 0 ? ( 0.25*t +0  )*t : \
  (i == 1 ? (-0.75*t +0.5)*t +0.25 : \
  (i == 2 ? ( 0.5 *t -1  )*t : \
  (i == 3 ? ( 0.5 *t +0  )*t -0.5 : \
  (i == 4 ? (-0.75*t +1  )*t : \
  (i == 5 ? ( 0.25*t -0.5)*t +0.25 : \
  0)))))))


/* ------------------------ TMF_d2_c1_3ef --------------------- */

#define TMF_d2_c1_3ef(a, i, t) ( \
  (i == 0 ? ((  2./3*t - 0.75)*t +0  )*t : \
  (i == 1 ? ((-10./3*t + 4.25)*t +0.5)*t -1./12 : \
  (i == 2 ? (( 20./3*t - 9.5 )*t -1  )*t +4./3 : \
  (i == 3 ? ((-20./3*t +10.5 )*t +0  )*t -2.5 : \
  (i == 4 ? (( 10./3*t - 5.75)*t +1  )*t +4./3 : \
  (i == 5 ? ((- 2./3*t + 1.25)*t -0.5)*t -1./12 : \
  0)))))))


/* ------------------------ TMF_d2_c1_4ef --------------------- */

#define TMF_d2_c1_4ef(a, i, t) ( \
  (i == 0 ? ((-(  a+ 53)/360*t +(  a+ 38)/240)*t +0    )*t : \
  (i == 1 ? (( (7*a+431)/360*t -(7*a+296)/240)*t - 1./8)*t +(a+ 8)/720 : \
  (i == 2 ? ((-(7*a+471)/120*t +(7*a+366)/80 )*t +1    )*t -(a+18)/120 : \
  (i == 3 ? (( (7*a+491)/72 *t -(7*a+452)/48 )*t -13./8)*t +(a+72)/48  : \
  (i == 4 ? ((-(7*a+491)/72 *t +(7*a+530)/48 )*t +0    )*t -(a+98)/36  : \
  (i == 5 ? (( (7*a+471)/120*t -(7*a+576)/80 )*t +13./8)*t +(a+72)/48  : \
  (i == 6 ? ((-(7*a+431)/360*t +(7*a+566)/240)*t -1    )*t -(a+18)/120 : \
  (i == 7 ? (( (  a+ 53)/360*t -(  a+ 68)/240)*t + 1./8)*t +(a+ 8)/720 : \
  0)))))))))


/* ------------------------ TMF_d2_c2_1ef --------------------- */

#define TMF_d2_c2_1ef(a, i, t) ( \
  (i == 0 ? ((  6*t -15)*t +10)*t*t*t : \
  (i == 1 ? ((-18*t +45)*t -30)*t*t*t +1 : \
  (i == 2 ? (( 18*t -45)*t +30)*t*t*t -2 : \
  (i == 3 ? ((- 6*t +15)*t -10)*t*t*t +1 : \
  0)))))


/* ------------------------ TMF_d2_c2_2ef --------------------- */

#define TMF_d2_c2_2ef(a, i, t) ( \
  (i == 0 ? (( 1./6*t +0  )*t +0  )*t : \
  (i == 1 ? ((-5./6*t +0.5)*t +0.5)*t +1./6 : \
  (i == 2 ? (( 5./3*t -2  )*t -1  )*t +1./3 : \
  (i == 3 ? ((-5./3*t +3  )*t +0  )*t -1 : \
  (i == 4 ? (( 5./6*t -2  )*t +1  )*t +1./3 : \
  (i == 5 ? ((-1./6*t +0.5)*t -0.5)*t +1./6 : \
  0)))))))


/* ------------------------ TMF_d2_c2_3ef --------------------- */

#define TMF_d2_c2_3ef(a, i, t) ( \
  (i == 0 ? ((((-1.5*t + 3.75)*t - 7./3)*t +0  )*t +0  )*t : \
  (i == 1 ? (((( 7.5*t -18.75)*t +35./3)*t +0.5)*t +0.5)*t -1./12 : \
  (i == 2 ? ((((- 15*t +37.5 )*t -70./3)*t -2  )*t -1  )*t +4./3  : \
  (i == 3 ? ((((  15*t -37.5 )*t +70./3)*t +3  )*t +0  )*t -2.5   : \
  (i == 4 ? ((((-7.5*t +18.75)*t -35./3)*t -2  )*t +1  )*t +4./3  : \
  (i == 5 ? (((( 1.5*t - 3.75)*t + 7./3)*t +0.5)*t -0.5)*t -1./12 : \
  0)))))))


/* ------------------------ TMF_d2_c2_4ef --------------------- */

#define TMF_d2_c2_4ef(a, i, t) ( \
  TMF_d2_c1_4ef((double)(-38), i, t))


/* ------------------------ TMF_d2_c3_1ef --------------------- */

#define TMF_d2_c3_1ef(a, i, t) ( \
  (i == 0 ? (((-20*t + 70)*t - 84)*t + 35)*t*t*t*t : \
  (i == 1 ? ((( 60*t -210)*t +252)*t -105)*t*t*t*t +1 : \
  (i == 2 ? (((-60*t +210)*t -252)*t +105)*t*t*t*t -2 : \
  (i == 3 ? ((( 20*t - 70)*t + 84)*t - 35)*t*t*t*t +1 : \
  0)))))


/* ------------------------ TMF_d2_c3_2ef --------------------- */

#define TMF_d2_c3_2ef(a, i, t) ( \
  (i == 0 ? (((-0.1*t +0.25)*t*t +  0)*t +0  )*t : \
  (i == 1 ? ((( 0.5*t -1.25)*t*t +0.5)*t +0.5)*t +3./20 : \
  (i == 2 ? (((-1  *t +2.5 )*t*t -2  )*t -1  )*t +2./5 : \
  (i == 3 ? ((( 1  *t -2.5 )*t*t +3  )*t +0  )*t -11./10 : \
  (i == 4 ? (((-0.5*t +1.25)*t*t -2  )*t +1  )*t +2./5 : \
  (i == 5 ? ((( 0.1*t -0.25)*t*t +0.5)*t -0.5)*t +3./20 : \
  0)))))))


/* ------------------------ TMF_d2_c3_3ef --------------------- */

#define TMF_d2_c3_3ef(a, i, t) ( \
  (i == 0 ? (((((  14./3*t - 49./3)*t + 39./2)*t - 95./12)*t*t +0  )*t +0  )*t : \
  (i == 1 ? (((((- 70./3*t +245./3)*t -195./2)*t +475./12)*t*t +0.5)*t +0.5)*t -1./12 : \
  (i == 2 ? ((((( 140./3*t -490./3)*t +195   )*t -475./6 )*t*t -2  )*t -1  )*t +4./3 : \
  (i == 3 ? (((((-140./3*t +490./3)*t -195   )*t +475./6 )*t*t +3  )*t +0  )*t -5./2 : \
  (i == 4 ? (((((  70./3*t -245./3)*t +195./2)*t -475./12)*t*t -2  )*t +1  )*t +4./3 : \
  (i == 5 ? (((((- 14./3*t + 49./3)*t - 39./2)*t + 95./12)*t*t +0.5)*t -0.5)*t -1./12 : \
  0)))))))


/* ------------------------ TMF_d2_c3_4ef --------------------- */

#define TMF_d2_c3_4ef(a, i, t) ( \
  (i == 0 ? ((((  1./24*t - 1./12)*t +0    )*t +0    )*t +0    )*t : \
  (i == 1 ? ((((- 7./24*t + 5./8 )*t +1./12)*t -1./12)*t - 1./8)*t - 1./24 : \
  (i == 2 ? ((((  7./8 *t - 2    )*t -1./3 )*t +1    )*t +1    )*t + 1./6 : \
  (i == 3 ? ((((-35./24*t +85./24)*t +5./12)*t -13./4)*t -13./8)*t +17./24 : \
  (i == 4 ? (((( 35./24*t -15./4 )*t +0    )*t +14./3)*t +0    )*t - 5./3 : \
  (i == 5 ? ((((- 7./8 *t +19./8 )*t -5./12)*t -13./4)*t +13./8)*t +17./24 : \
  (i == 6 ? ((((  7./24*t - 5./6 )*t +1./3 )*t +1    )*t -1    )*t + 1./6 : \
  (i == 7 ? ((((- 1./24*t + 1./8 )*t -1./12)*t -1./12)*t + 1./8)*t - 1./24 : \
  0)))))))))


/* ------------------------ TMF_dn_cn_1ef --------------------- */

double _nrrd_TMF_dn_cn_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_cn_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_dn_cn_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_cn_1ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_cn_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_cn_1ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_cn_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_cn_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_cn_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_cn_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_cn_1ef = {
  "TMF_dn_cn_1ef",
  1, _nrrd_TMF_dn_cn_1ef_Sup, _nrrd_TMF_dn_cn_1ef_Int,
  _nrrd_TMF_dn_cn_1ef_1_f,  _nrrd_TMF_dn_cn_1ef_N_f,
  _nrrd_TMF_dn_cn_1ef_1_d,  _nrrd_TMF_dn_cn_1ef_N_d
};


/* ------------------------ TMF_dn_cn_2ef --------------------- */

double _nrrd_TMF_dn_cn_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_cn_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_dn_cn_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_2ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_cn_2ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_cn_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_2ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_cn_2ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_cn_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_cn_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_cn_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_cn_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_cn_2ef = {
  "TMF_dn_cn_2ef",
  1, _nrrd_TMF_dn_cn_2ef_Sup, _nrrd_TMF_dn_cn_2ef_Int,
  _nrrd_TMF_dn_cn_2ef_1_f,  _nrrd_TMF_dn_cn_2ef_N_f,
  _nrrd_TMF_dn_cn_2ef_1_d,  _nrrd_TMF_dn_cn_2ef_N_d
};


/* ------------------------ TMF_dn_cn_3ef --------------------- */

double _nrrd_TMF_dn_cn_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_cn_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_dn_cn_3ef_1_d(double x, const double *parm) {
  int i;

  
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_cn_3ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_cn_3ef_1_f(float x, const double *parm) {
  int i;

  
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_cn_3ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_cn_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_cn_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_cn_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_cn_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_cn_3ef = {
  "TMF_dn_cn_3ef",
  1, _nrrd_TMF_dn_cn_3ef_Sup, _nrrd_TMF_dn_cn_3ef_Int,
  _nrrd_TMF_dn_cn_3ef_1_f,  _nrrd_TMF_dn_cn_3ef_N_f,
  _nrrd_TMF_dn_cn_3ef_1_d,  _nrrd_TMF_dn_cn_3ef_N_d
};


/* ------------------------ TMF_dn_cn_4ef --------------------- */

double _nrrd_TMF_dn_cn_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_cn_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_dn_cn_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_4ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_cn_4ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_cn_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_4ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_cn_4ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_cn_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_cn_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_cn_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_cn_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_cn_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_cn_4ef = {
  "TMF_dn_cn_4ef",
  1, _nrrd_TMF_dn_cn_4ef_Sup, _nrrd_TMF_dn_cn_4ef_Int,
  _nrrd_TMF_dn_cn_4ef_1_f,  _nrrd_TMF_dn_cn_4ef_N_f,
  _nrrd_TMF_dn_cn_4ef_1_d,  _nrrd_TMF_dn_cn_4ef_N_d
};


/* ------------------------ TMF_dn_c0_1ef --------------------- */

double _nrrd_TMF_dn_c0_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c0_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_dn_c0_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c0_1ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c0_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c0_1ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c0_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c0_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c0_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c0_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c0_1ef = {
  "TMF_dn_c0_1ef",
  1, _nrrd_TMF_dn_c0_1ef_Sup, _nrrd_TMF_dn_c0_1ef_Int,
  _nrrd_TMF_dn_c0_1ef_1_f,  _nrrd_TMF_dn_c0_1ef_N_f,
  _nrrd_TMF_dn_c0_1ef_1_d,  _nrrd_TMF_dn_c0_1ef_N_d
};


/* ------------------------ TMF_dn_c0_2ef --------------------- */

double _nrrd_TMF_dn_c0_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c0_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_dn_c0_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_2ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c0_2ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c0_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_2ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c0_2ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c0_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c0_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c0_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c0_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c0_2ef = {
  "TMF_dn_c0_2ef",
  1, _nrrd_TMF_dn_c0_2ef_Sup, _nrrd_TMF_dn_c0_2ef_Int,
  _nrrd_TMF_dn_c0_2ef_1_f,  _nrrd_TMF_dn_c0_2ef_N_f,
  _nrrd_TMF_dn_c0_2ef_1_d,  _nrrd_TMF_dn_c0_2ef_N_d
};


/* ------------------------ TMF_dn_c0_3ef --------------------- */

double _nrrd_TMF_dn_c0_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c0_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_dn_c0_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c0_3ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c0_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c0_3ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c0_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c0_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c0_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c0_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c0_3ef = {
  "TMF_dn_c0_3ef",
  1, _nrrd_TMF_dn_c0_3ef_Sup, _nrrd_TMF_dn_c0_3ef_Int,
  _nrrd_TMF_dn_c0_3ef_1_f,  _nrrd_TMF_dn_c0_3ef_N_f,
  _nrrd_TMF_dn_c0_3ef_1_d,  _nrrd_TMF_dn_c0_3ef_N_d
};


/* ------------------------ TMF_dn_c0_4ef --------------------- */

double _nrrd_TMF_dn_c0_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c0_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_dn_c0_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_4ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c0_4ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c0_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_4ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c0_4ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c0_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c0_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c0_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c0_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c0_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c0_4ef = {
  "TMF_dn_c0_4ef",
  1, _nrrd_TMF_dn_c0_4ef_Sup, _nrrd_TMF_dn_c0_4ef_Int,
  _nrrd_TMF_dn_c0_4ef_1_f,  _nrrd_TMF_dn_c0_4ef_N_f,
  _nrrd_TMF_dn_c0_4ef_1_d,  _nrrd_TMF_dn_c0_4ef_N_d
};


/* ------------------------ TMF_dn_c1_1ef --------------------- */

double _nrrd_TMF_dn_c1_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c1_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_dn_c1_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c1_1ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c1_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c1_1ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c1_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c1_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c1_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c1_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c1_1ef = {
  "TMF_dn_c1_1ef",
  1, _nrrd_TMF_dn_c1_1ef_Sup, _nrrd_TMF_dn_c1_1ef_Int,
  _nrrd_TMF_dn_c1_1ef_1_f,  _nrrd_TMF_dn_c1_1ef_N_f,
  _nrrd_TMF_dn_c1_1ef_1_d,  _nrrd_TMF_dn_c1_1ef_N_d
};


/* ------------------------ TMF_dn_c1_2ef --------------------- */

double _nrrd_TMF_dn_c1_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c1_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_dn_c1_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c1_2ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c1_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c1_2ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c1_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c1_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c1_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c1_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c1_2ef = {
  "TMF_dn_c1_2ef",
  1, _nrrd_TMF_dn_c1_2ef_Sup, _nrrd_TMF_dn_c1_2ef_Int,
  _nrrd_TMF_dn_c1_2ef_1_f,  _nrrd_TMF_dn_c1_2ef_N_f,
  _nrrd_TMF_dn_c1_2ef_1_d,  _nrrd_TMF_dn_c1_2ef_N_d
};


/* ------------------------ TMF_dn_c1_3ef --------------------- */

double _nrrd_TMF_dn_c1_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c1_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_dn_c1_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c1_3ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c1_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c1_3ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c1_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c1_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c1_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c1_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c1_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c1_3ef = {
  "TMF_dn_c1_3ef",
  1, _nrrd_TMF_dn_c1_3ef_Sup, _nrrd_TMF_dn_c1_3ef_Int,
  _nrrd_TMF_dn_c1_3ef_1_f,  _nrrd_TMF_dn_c1_3ef_N_f,
  _nrrd_TMF_dn_c1_3ef_1_d,  _nrrd_TMF_dn_c1_3ef_N_d
};


/* ------------------------ TMF_dn_c1_4ef --------------------- */

double _nrrd_TMF_dn_c1_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c1_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_dn_c1_4ef_1_d(double x, const double *parm) {
  int i;

  
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c1_4ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c1_4ef_1_f(float x, const double *parm) {
  int i;

  
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c1_4ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c1_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c1_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c1_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c1_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c1_4ef = {
  "TMF_dn_c1_4ef",
  1, _nrrd_TMF_dn_c1_4ef_Sup, _nrrd_TMF_dn_c1_4ef_Int,
  _nrrd_TMF_dn_c1_4ef_1_f,  _nrrd_TMF_dn_c1_4ef_N_f,
  _nrrd_TMF_dn_c1_4ef_1_d,  _nrrd_TMF_dn_c1_4ef_N_d
};


/* ------------------------ TMF_dn_c2_1ef --------------------- */

double _nrrd_TMF_dn_c2_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c2_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_dn_c2_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c2_1ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c2_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c2_1ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c2_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c2_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c2_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c2_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c2_1ef = {
  "TMF_dn_c2_1ef",
  1, _nrrd_TMF_dn_c2_1ef_Sup, _nrrd_TMF_dn_c2_1ef_Int,
  _nrrd_TMF_dn_c2_1ef_1_f,  _nrrd_TMF_dn_c2_1ef_N_f,
  _nrrd_TMF_dn_c2_1ef_1_d,  _nrrd_TMF_dn_c2_1ef_N_d
};


/* ------------------------ TMF_dn_c2_2ef --------------------- */

double _nrrd_TMF_dn_c2_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c2_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_dn_c2_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c2_2ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c2_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c2_2ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c2_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c2_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c2_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c2_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c2_2ef = {
  "TMF_dn_c2_2ef",
  1, _nrrd_TMF_dn_c2_2ef_Sup, _nrrd_TMF_dn_c2_2ef_Int,
  _nrrd_TMF_dn_c2_2ef_1_f,  _nrrd_TMF_dn_c2_2ef_N_f,
  _nrrd_TMF_dn_c2_2ef_1_d,  _nrrd_TMF_dn_c2_2ef_N_d
};


/* ------------------------ TMF_dn_c2_3ef --------------------- */

double _nrrd_TMF_dn_c2_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c2_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_dn_c2_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c2_3ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c2_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c2_3ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c2_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c2_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c2_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c2_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c2_3ef = {
  "TMF_dn_c2_3ef",
  1, _nrrd_TMF_dn_c2_3ef_Sup, _nrrd_TMF_dn_c2_3ef_Int,
  _nrrd_TMF_dn_c2_3ef_1_f,  _nrrd_TMF_dn_c2_3ef_N_f,
  _nrrd_TMF_dn_c2_3ef_1_d,  _nrrd_TMF_dn_c2_3ef_N_d
};


/* ------------------------ TMF_dn_c2_4ef --------------------- */

double _nrrd_TMF_dn_c2_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c2_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_dn_c2_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c2_4ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c2_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c2_4ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c2_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c2_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c2_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c2_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c2_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c2_4ef = {
  "TMF_dn_c2_4ef",
  1, _nrrd_TMF_dn_c2_4ef_Sup, _nrrd_TMF_dn_c2_4ef_Int,
  _nrrd_TMF_dn_c2_4ef_1_f,  _nrrd_TMF_dn_c2_4ef_N_f,
  _nrrd_TMF_dn_c2_4ef_1_d,  _nrrd_TMF_dn_c2_4ef_N_d
};


/* ------------------------ TMF_dn_c3_1ef --------------------- */

double _nrrd_TMF_dn_c3_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c3_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_dn_c3_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c3_1ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c3_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c3_1ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c3_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c3_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c3_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c3_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c3_1ef = {
  "TMF_dn_c3_1ef",
  1, _nrrd_TMF_dn_c3_1ef_Sup, _nrrd_TMF_dn_c3_1ef_Int,
  _nrrd_TMF_dn_c3_1ef_1_f,  _nrrd_TMF_dn_c3_1ef_N_f,
  _nrrd_TMF_dn_c3_1ef_1_d,  _nrrd_TMF_dn_c3_1ef_N_d
};


/* ------------------------ TMF_dn_c3_2ef --------------------- */

double _nrrd_TMF_dn_c3_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c3_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_dn_c3_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c3_2ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c3_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c3_2ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c3_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c3_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c3_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c3_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c3_2ef = {
  "TMF_dn_c3_2ef",
  1, _nrrd_TMF_dn_c3_2ef_Sup, _nrrd_TMF_dn_c3_2ef_Int,
  _nrrd_TMF_dn_c3_2ef_1_f,  _nrrd_TMF_dn_c3_2ef_N_f,
  _nrrd_TMF_dn_c3_2ef_1_d,  _nrrd_TMF_dn_c3_2ef_N_d
};


/* ------------------------ TMF_dn_c3_3ef --------------------- */

double _nrrd_TMF_dn_c3_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c3_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_dn_c3_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c3_3ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c3_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c3_3ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c3_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c3_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c3_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c3_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c3_3ef = {
  "TMF_dn_c3_3ef",
  1, _nrrd_TMF_dn_c3_3ef_Sup, _nrrd_TMF_dn_c3_3ef_Int,
  _nrrd_TMF_dn_c3_3ef_1_f,  _nrrd_TMF_dn_c3_3ef_N_f,
  _nrrd_TMF_dn_c3_3ef_1_d,  _nrrd_TMF_dn_c3_3ef_N_d
};


/* ------------------------ TMF_dn_c3_4ef --------------------- */

double _nrrd_TMF_dn_c3_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_dn_c3_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_dn_c3_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_dn_c3_4ef(parm[0], i, x);
}

float
_nrrd_TMF_dn_c3_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_dn_c3_4ef(parm[0], i, x));
}

void
_nrrd_TMF_dn_c3_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_dn_c3_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_dn_c3_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_dn_c3_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_dn_c3_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_dn_c3_4ef = {
  "TMF_dn_c3_4ef",
  1, _nrrd_TMF_dn_c3_4ef_Sup, _nrrd_TMF_dn_c3_4ef_Int,
  _nrrd_TMF_dn_c3_4ef_1_f,  _nrrd_TMF_dn_c3_4ef_N_f,
  _nrrd_TMF_dn_c3_4ef_1_d,  _nrrd_TMF_dn_c3_4ef_N_d
};


/* ------------------------ TMF_d0_cn_1ef --------------------- */

double _nrrd_TMF_d0_cn_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_cn_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_d0_cn_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_cn_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_cn_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_cn_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_cn_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_cn_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_cn_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_cn_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_cn_1ef = {
  "TMF_d0_cn_1ef",
  1, _nrrd_TMF_d0_cn_1ef_Sup, _nrrd_TMF_d0_cn_1ef_Int,
  _nrrd_TMF_d0_cn_1ef_1_f,  _nrrd_TMF_d0_cn_1ef_N_f,
  _nrrd_TMF_d0_cn_1ef_1_d,  _nrrd_TMF_d0_cn_1ef_N_d
};


/* ------------------------ TMF_d0_cn_2ef --------------------- */

double _nrrd_TMF_d0_cn_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_cn_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_d0_cn_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_2ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_cn_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_cn_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_2ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_cn_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_cn_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_cn_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_cn_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_cn_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_cn_2ef = {
  "TMF_d0_cn_2ef",
  1, _nrrd_TMF_d0_cn_2ef_Sup, _nrrd_TMF_d0_cn_2ef_Int,
  _nrrd_TMF_d0_cn_2ef_1_f,  _nrrd_TMF_d0_cn_2ef_N_f,
  _nrrd_TMF_d0_cn_2ef_1_d,  _nrrd_TMF_d0_cn_2ef_N_d
};


/* ------------------------ TMF_d0_cn_3ef --------------------- */

double _nrrd_TMF_d0_cn_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_cn_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d0_cn_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_cn_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_cn_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_cn_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_cn_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_cn_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_cn_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_cn_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_cn_3ef = {
  "TMF_d0_cn_3ef",
  1, _nrrd_TMF_d0_cn_3ef_Sup, _nrrd_TMF_d0_cn_3ef_Int,
  _nrrd_TMF_d0_cn_3ef_1_f,  _nrrd_TMF_d0_cn_3ef_N_f,
  _nrrd_TMF_d0_cn_3ef_1_d,  _nrrd_TMF_d0_cn_3ef_N_d
};


/* ------------------------ TMF_d0_cn_4ef --------------------- */

double _nrrd_TMF_d0_cn_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_cn_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d0_cn_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_4ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_cn_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_cn_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_4ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_cn_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_cn_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_cn_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_cn_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_cn_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_cn_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_cn_4ef = {
  "TMF_d0_cn_4ef",
  1, _nrrd_TMF_d0_cn_4ef_Sup, _nrrd_TMF_d0_cn_4ef_Int,
  _nrrd_TMF_d0_cn_4ef_1_f,  _nrrd_TMF_d0_cn_4ef_N_f,
  _nrrd_TMF_d0_cn_4ef_1_d,  _nrrd_TMF_d0_cn_4ef_N_d
};


/* ------------------------ TMF_d0_c0_1ef --------------------- */

double _nrrd_TMF_d0_c0_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c0_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_d0_c0_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c0_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c0_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c0_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c0_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c0_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c0_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c0_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c0_1ef = {
  "TMF_d0_c0_1ef",
  1, _nrrd_TMF_d0_c0_1ef_Sup, _nrrd_TMF_d0_c0_1ef_Int,
  _nrrd_TMF_d0_c0_1ef_1_f,  _nrrd_TMF_d0_c0_1ef_N_f,
  _nrrd_TMF_d0_c0_1ef_1_d,  _nrrd_TMF_d0_c0_1ef_N_d
};


/* ------------------------ TMF_d0_c0_2ef --------------------- */

double _nrrd_TMF_d0_c0_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c0_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_d0_c0_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_2ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c0_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c0_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_2ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c0_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c0_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c0_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c0_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c0_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c0_2ef = {
  "TMF_d0_c0_2ef",
  1, _nrrd_TMF_d0_c0_2ef_Sup, _nrrd_TMF_d0_c0_2ef_Int,
  _nrrd_TMF_d0_c0_2ef_1_f,  _nrrd_TMF_d0_c0_2ef_N_f,
  _nrrd_TMF_d0_c0_2ef_1_d,  _nrrd_TMF_d0_c0_2ef_N_d
};


/* ------------------------ TMF_d0_c0_3ef --------------------- */

double _nrrd_TMF_d0_c0_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c0_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d0_c0_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c0_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c0_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c0_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c0_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c0_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c0_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c0_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c0_3ef = {
  "TMF_d0_c0_3ef",
  1, _nrrd_TMF_d0_c0_3ef_Sup, _nrrd_TMF_d0_c0_3ef_Int,
  _nrrd_TMF_d0_c0_3ef_1_f,  _nrrd_TMF_d0_c0_3ef_N_f,
  _nrrd_TMF_d0_c0_3ef_1_d,  _nrrd_TMF_d0_c0_3ef_N_d
};


/* ------------------------ TMF_d0_c0_4ef --------------------- */

double _nrrd_TMF_d0_c0_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c0_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d0_c0_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_4ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c0_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c0_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_4ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c0_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c0_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c0_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c0_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c0_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c0_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c0_4ef = {
  "TMF_d0_c0_4ef",
  1, _nrrd_TMF_d0_c0_4ef_Sup, _nrrd_TMF_d0_c0_4ef_Int,
  _nrrd_TMF_d0_c0_4ef_1_f,  _nrrd_TMF_d0_c0_4ef_N_f,
  _nrrd_TMF_d0_c0_4ef_1_d,  _nrrd_TMF_d0_c0_4ef_N_d
};


/* ------------------------ TMF_d0_c1_1ef --------------------- */

double _nrrd_TMF_d0_c1_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c1_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_d0_c1_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c1_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c1_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c1_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c1_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c1_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c1_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c1_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c1_1ef = {
  "TMF_d0_c1_1ef",
  1, _nrrd_TMF_d0_c1_1ef_Sup, _nrrd_TMF_d0_c1_1ef_Int,
  _nrrd_TMF_d0_c1_1ef_1_f,  _nrrd_TMF_d0_c1_1ef_N_f,
  _nrrd_TMF_d0_c1_1ef_1_d,  _nrrd_TMF_d0_c1_1ef_N_d
};


/* ------------------------ TMF_d0_c1_2ef --------------------- */

double _nrrd_TMF_d0_c1_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c1_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d0_c1_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c1_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c1_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c1_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c1_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c1_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c1_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c1_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c1_2ef = {
  "TMF_d0_c1_2ef",
  1, _nrrd_TMF_d0_c1_2ef_Sup, _nrrd_TMF_d0_c1_2ef_Int,
  _nrrd_TMF_d0_c1_2ef_1_f,  _nrrd_TMF_d0_c1_2ef_N_f,
  _nrrd_TMF_d0_c1_2ef_1_d,  _nrrd_TMF_d0_c1_2ef_N_d
};


/* ------------------------ TMF_d0_c1_3ef --------------------- */

double _nrrd_TMF_d0_c1_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c1_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d0_c1_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c1_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c1_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c1_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c1_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c1_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c1_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c1_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c1_3ef = {
  "TMF_d0_c1_3ef",
  1, _nrrd_TMF_d0_c1_3ef_Sup, _nrrd_TMF_d0_c1_3ef_Int,
  _nrrd_TMF_d0_c1_3ef_1_f,  _nrrd_TMF_d0_c1_3ef_N_f,
  _nrrd_TMF_d0_c1_3ef_1_d,  _nrrd_TMF_d0_c1_3ef_N_d
};


/* ------------------------ TMF_d0_c1_4ef --------------------- */

double _nrrd_TMF_d0_c1_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c1_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d0_c1_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c1_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c1_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c1_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c1_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c1_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c1_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c1_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c1_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c1_4ef = {
  "TMF_d0_c1_4ef",
  1, _nrrd_TMF_d0_c1_4ef_Sup, _nrrd_TMF_d0_c1_4ef_Int,
  _nrrd_TMF_d0_c1_4ef_1_f,  _nrrd_TMF_d0_c1_4ef_N_f,
  _nrrd_TMF_d0_c1_4ef_1_d,  _nrrd_TMF_d0_c1_4ef_N_d
};


/* ------------------------ TMF_d0_c2_1ef --------------------- */

double _nrrd_TMF_d0_c2_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c2_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_d0_c2_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c2_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c2_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c2_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c2_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c2_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c2_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c2_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c2_1ef = {
  "TMF_d0_c2_1ef",
  1, _nrrd_TMF_d0_c2_1ef_Sup, _nrrd_TMF_d0_c2_1ef_Int,
  _nrrd_TMF_d0_c2_1ef_1_f,  _nrrd_TMF_d0_c2_1ef_N_f,
  _nrrd_TMF_d0_c2_1ef_1_d,  _nrrd_TMF_d0_c2_1ef_N_d
};


/* ------------------------ TMF_d0_c2_2ef --------------------- */

double _nrrd_TMF_d0_c2_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c2_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d0_c2_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c2_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c2_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c2_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c2_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c2_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c2_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c2_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c2_2ef = {
  "TMF_d0_c2_2ef",
  1, _nrrd_TMF_d0_c2_2ef_Sup, _nrrd_TMF_d0_c2_2ef_Int,
  _nrrd_TMF_d0_c2_2ef_1_f,  _nrrd_TMF_d0_c2_2ef_N_f,
  _nrrd_TMF_d0_c2_2ef_1_d,  _nrrd_TMF_d0_c2_2ef_N_d
};


/* ------------------------ TMF_d0_c2_3ef --------------------- */

double _nrrd_TMF_d0_c2_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c2_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d0_c2_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c2_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c2_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c2_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c2_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c2_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c2_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c2_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c2_3ef = {
  "TMF_d0_c2_3ef",
  1, _nrrd_TMF_d0_c2_3ef_Sup, _nrrd_TMF_d0_c2_3ef_Int,
  _nrrd_TMF_d0_c2_3ef_1_f,  _nrrd_TMF_d0_c2_3ef_N_f,
  _nrrd_TMF_d0_c2_3ef_1_d,  _nrrd_TMF_d0_c2_3ef_N_d
};


/* ------------------------ TMF_d0_c2_4ef --------------------- */

double _nrrd_TMF_d0_c2_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c2_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d0_c2_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c2_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c2_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c2_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c2_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c2_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c2_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c2_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c2_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c2_4ef = {
  "TMF_d0_c2_4ef",
  1, _nrrd_TMF_d0_c2_4ef_Sup, _nrrd_TMF_d0_c2_4ef_Int,
  _nrrd_TMF_d0_c2_4ef_1_f,  _nrrd_TMF_d0_c2_4ef_N_f,
  _nrrd_TMF_d0_c2_4ef_1_d,  _nrrd_TMF_d0_c2_4ef_N_d
};


/* ------------------------ TMF_d0_c3_1ef --------------------- */

double _nrrd_TMF_d0_c3_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c3_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_d0_c3_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c3_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c3_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c3_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c3_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c3_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c3_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c3_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c3_1ef = {
  "TMF_d0_c3_1ef",
  1, _nrrd_TMF_d0_c3_1ef_Sup, _nrrd_TMF_d0_c3_1ef_Int,
  _nrrd_TMF_d0_c3_1ef_1_f,  _nrrd_TMF_d0_c3_1ef_N_f,
  _nrrd_TMF_d0_c3_1ef_1_d,  _nrrd_TMF_d0_c3_1ef_N_d
};


/* ------------------------ TMF_d0_c3_2ef --------------------- */

double _nrrd_TMF_d0_c3_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c3_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d0_c3_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c3_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c3_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c3_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c3_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c3_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c3_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c3_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c3_2ef = {
  "TMF_d0_c3_2ef",
  1, _nrrd_TMF_d0_c3_2ef_Sup, _nrrd_TMF_d0_c3_2ef_Int,
  _nrrd_TMF_d0_c3_2ef_1_f,  _nrrd_TMF_d0_c3_2ef_N_f,
  _nrrd_TMF_d0_c3_2ef_1_d,  _nrrd_TMF_d0_c3_2ef_N_d
};


/* ------------------------ TMF_d0_c3_3ef --------------------- */

double _nrrd_TMF_d0_c3_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c3_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d0_c3_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c3_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c3_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c3_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c3_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c3_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c3_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c3_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c3_3ef = {
  "TMF_d0_c3_3ef",
  1, _nrrd_TMF_d0_c3_3ef_Sup, _nrrd_TMF_d0_c3_3ef_Int,
  _nrrd_TMF_d0_c3_3ef_1_f,  _nrrd_TMF_d0_c3_3ef_N_f,
  _nrrd_TMF_d0_c3_3ef_1_d,  _nrrd_TMF_d0_c3_3ef_N_d
};


/* ------------------------ TMF_d0_c3_4ef --------------------- */

double _nrrd_TMF_d0_c3_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double _nrrd_TMF_d0_c3_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d0_c3_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d0_c3_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d0_c3_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d0_c3_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d0_c3_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d0_c3_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d0_c3_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d0_c3_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d0_c3_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d0_c3_4ef = {
  "TMF_d0_c3_4ef",
  1, _nrrd_TMF_d0_c3_4ef_Sup, _nrrd_TMF_d0_c3_4ef_Int,
  _nrrd_TMF_d0_c3_4ef_1_f,  _nrrd_TMF_d0_c3_4ef_N_f,
  _nrrd_TMF_d0_c3_4ef_1_d,  _nrrd_TMF_d0_c3_4ef_N_d
};


/* ------------------------ TMF_d1_cn_1ef --------------------- */

double _nrrd_TMF_d1_cn_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_cn_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1;
}

double
_nrrd_TMF_d1_cn_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_cn_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_cn_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_cn_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_cn_1ef */
  x += 1;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_cn_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_cn_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_cn_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_cn_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_cn_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_cn_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 1;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_cn_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_cn_1ef = {
  "TMF_d1_cn_1ef",
  1, _nrrd_TMF_d1_cn_1ef_Sup, _nrrd_TMF_d1_cn_1ef_Int,
  _nrrd_TMF_d1_cn_1ef_1_f,  _nrrd_TMF_d1_cn_1ef_N_f,
  _nrrd_TMF_d1_cn_1ef_1_d,  _nrrd_TMF_d1_cn_1ef_N_d
};


/* ------------------------ TMF_d1_cn_2ef --------------------- */

double _nrrd_TMF_d1_cn_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_cn_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d1_cn_2ef_1_d(double x, const double *parm) {
  int i;

  
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_cn_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_cn_2ef_1_f(float x, const double *parm) {
  int i;

  
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_cn_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_cn_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_cn_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_cn_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_cn_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_cn_2ef = {
  "TMF_d1_cn_2ef",
  1, _nrrd_TMF_d1_cn_2ef_Sup, _nrrd_TMF_d1_cn_2ef_Int,
  _nrrd_TMF_d1_cn_2ef_1_f,  _nrrd_TMF_d1_cn_2ef_N_f,
  _nrrd_TMF_d1_cn_2ef_1_d,  _nrrd_TMF_d1_cn_2ef_N_d
};


/* ------------------------ TMF_d1_cn_3ef --------------------- */

double _nrrd_TMF_d1_cn_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_cn_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d1_cn_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_cn_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_cn_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_cn_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_cn_3ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_cn_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_cn_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_cn_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_cn_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_cn_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_cn_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_cn_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_cn_3ef = {
  "TMF_d1_cn_3ef",
  1, _nrrd_TMF_d1_cn_3ef_Sup, _nrrd_TMF_d1_cn_3ef_Int,
  _nrrd_TMF_d1_cn_3ef_1_f,  _nrrd_TMF_d1_cn_3ef_N_f,
  _nrrd_TMF_d1_cn_3ef_1_d,  _nrrd_TMF_d1_cn_3ef_N_d
};


/* ------------------------ TMF_d1_cn_4ef --------------------- */

double _nrrd_TMF_d1_cn_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_cn_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d1_cn_4ef_1_d(double x, const double *parm) {
  int i;

  
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_cn_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_cn_4ef_1_f(float x, const double *parm) {
  int i;

  
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_cn_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_cn_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_cn_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_cn_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_cn_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_cn_4ef = {
  "TMF_d1_cn_4ef",
  1, _nrrd_TMF_d1_cn_4ef_Sup, _nrrd_TMF_d1_cn_4ef_Int,
  _nrrd_TMF_d1_cn_4ef_1_f,  _nrrd_TMF_d1_cn_4ef_N_f,
  _nrrd_TMF_d1_cn_4ef_1_d,  _nrrd_TMF_d1_cn_4ef_N_d
};


/* ------------------------ TMF_d1_c0_1ef --------------------- */

double _nrrd_TMF_d1_c0_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c0_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d1_c0_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c0_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c0_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c0_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c0_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c0_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c0_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c0_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c0_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c0_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c0_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c0_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c0_1ef = {
  "TMF_d1_c0_1ef",
  1, _nrrd_TMF_d1_c0_1ef_Sup, _nrrd_TMF_d1_c0_1ef_Int,
  _nrrd_TMF_d1_c0_1ef_1_f,  _nrrd_TMF_d1_c0_1ef_N_f,
  _nrrd_TMF_d1_c0_1ef_1_d,  _nrrd_TMF_d1_c0_1ef_N_d
};


/* ------------------------ TMF_d1_c0_2ef --------------------- */

double _nrrd_TMF_d1_c0_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c0_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d1_c0_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c0_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c0_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c0_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c0_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c0_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c0_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c0_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c0_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c0_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c0_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c0_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c0_2ef = {
  "TMF_d1_c0_2ef",
  1, _nrrd_TMF_d1_c0_2ef_Sup, _nrrd_TMF_d1_c0_2ef_Int,
  _nrrd_TMF_d1_c0_2ef_1_f,  _nrrd_TMF_d1_c0_2ef_N_f,
  _nrrd_TMF_d1_c0_2ef_1_d,  _nrrd_TMF_d1_c0_2ef_N_d
};


/* ------------------------ TMF_d1_c0_3ef --------------------- */

double _nrrd_TMF_d1_c0_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c0_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d1_c0_3ef_1_d(double x, const double *parm) {
  int i;

  
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c0_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c0_3ef_1_f(float x, const double *parm) {
  int i;

  
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c0_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c0_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c0_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c0_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c0_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c0_3ef = {
  "TMF_d1_c0_3ef",
  1, _nrrd_TMF_d1_c0_3ef_Sup, _nrrd_TMF_d1_c0_3ef_Int,
  _nrrd_TMF_d1_c0_3ef_1_f,  _nrrd_TMF_d1_c0_3ef_N_f,
  _nrrd_TMF_d1_c0_3ef_1_d,  _nrrd_TMF_d1_c0_3ef_N_d
};


/* ------------------------ TMF_d1_c0_4ef --------------------- */

double _nrrd_TMF_d1_c0_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c0_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d1_c0_4ef_1_d(double x, const double *parm) {
  int i;

  
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c0_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c0_4ef_1_f(float x, const double *parm) {
  int i;

  
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c0_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c0_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c0_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c0_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c0_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c0_4ef = {
  "TMF_d1_c0_4ef",
  1, _nrrd_TMF_d1_c0_4ef_Sup, _nrrd_TMF_d1_c0_4ef_Int,
  _nrrd_TMF_d1_c0_4ef_1_f,  _nrrd_TMF_d1_c0_4ef_N_f,
  _nrrd_TMF_d1_c0_4ef_1_d,  _nrrd_TMF_d1_c0_4ef_N_d
};


/* ------------------------ TMF_d1_c1_1ef --------------------- */

double _nrrd_TMF_d1_c1_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c1_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d1_c1_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c1_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c1_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c1_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c1_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c1_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c1_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c1_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c1_1ef = {
  "TMF_d1_c1_1ef",
  1, _nrrd_TMF_d1_c1_1ef_Sup, _nrrd_TMF_d1_c1_1ef_Int,
  _nrrd_TMF_d1_c1_1ef_1_f,  _nrrd_TMF_d1_c1_1ef_N_f,
  _nrrd_TMF_d1_c1_1ef_1_d,  _nrrd_TMF_d1_c1_1ef_N_d
};


/* ------------------------ TMF_d1_c1_2ef --------------------- */

double _nrrd_TMF_d1_c1_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c1_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d1_c1_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c1_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c1_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c1_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c1_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c1_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c1_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c1_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c1_2ef = {
  "TMF_d1_c1_2ef",
  1, _nrrd_TMF_d1_c1_2ef_Sup, _nrrd_TMF_d1_c1_2ef_Int,
  _nrrd_TMF_d1_c1_2ef_1_f,  _nrrd_TMF_d1_c1_2ef_N_f,
  _nrrd_TMF_d1_c1_2ef_1_d,  _nrrd_TMF_d1_c1_2ef_N_d
};


/* ------------------------ TMF_d1_c1_3ef --------------------- */

double _nrrd_TMF_d1_c1_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c1_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d1_c1_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c1_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c1_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c1_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c1_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c1_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c1_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c1_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c1_3ef = {
  "TMF_d1_c1_3ef",
  1, _nrrd_TMF_d1_c1_3ef_Sup, _nrrd_TMF_d1_c1_3ef_Int,
  _nrrd_TMF_d1_c1_3ef_1_f,  _nrrd_TMF_d1_c1_3ef_N_f,
  _nrrd_TMF_d1_c1_3ef_1_d,  _nrrd_TMF_d1_c1_3ef_N_d
};


/* ------------------------ TMF_d1_c1_4ef --------------------- */

double _nrrd_TMF_d1_c1_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c1_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d1_c1_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c1_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c1_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c1_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c1_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c1_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c1_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c1_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c1_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c1_4ef = {
  "TMF_d1_c1_4ef",
  1, _nrrd_TMF_d1_c1_4ef_Sup, _nrrd_TMF_d1_c1_4ef_Int,
  _nrrd_TMF_d1_c1_4ef_1_f,  _nrrd_TMF_d1_c1_4ef_N_f,
  _nrrd_TMF_d1_c1_4ef_1_d,  _nrrd_TMF_d1_c1_4ef_N_d
};


/* ------------------------ TMF_d1_c2_1ef --------------------- */

double _nrrd_TMF_d1_c2_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c2_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d1_c2_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c2_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c2_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c2_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c2_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c2_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c2_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c2_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c2_1ef = {
  "TMF_d1_c2_1ef",
  1, _nrrd_TMF_d1_c2_1ef_Sup, _nrrd_TMF_d1_c2_1ef_Int,
  _nrrd_TMF_d1_c2_1ef_1_f,  _nrrd_TMF_d1_c2_1ef_N_f,
  _nrrd_TMF_d1_c2_1ef_1_d,  _nrrd_TMF_d1_c2_1ef_N_d
};


/* ------------------------ TMF_d1_c2_2ef --------------------- */

double _nrrd_TMF_d1_c2_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c2_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d1_c2_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c2_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c2_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c2_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c2_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c2_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c2_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c2_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c2_2ef = {
  "TMF_d1_c2_2ef",
  1, _nrrd_TMF_d1_c2_2ef_Sup, _nrrd_TMF_d1_c2_2ef_Int,
  _nrrd_TMF_d1_c2_2ef_1_f,  _nrrd_TMF_d1_c2_2ef_N_f,
  _nrrd_TMF_d1_c2_2ef_1_d,  _nrrd_TMF_d1_c2_2ef_N_d
};


/* ------------------------ TMF_d1_c2_3ef --------------------- */

double _nrrd_TMF_d1_c2_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c2_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d1_c2_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c2_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c2_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c2_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c2_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c2_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c2_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c2_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c2_3ef = {
  "TMF_d1_c2_3ef",
  1, _nrrd_TMF_d1_c2_3ef_Sup, _nrrd_TMF_d1_c2_3ef_Int,
  _nrrd_TMF_d1_c2_3ef_1_f,  _nrrd_TMF_d1_c2_3ef_N_f,
  _nrrd_TMF_d1_c2_3ef_1_d,  _nrrd_TMF_d1_c2_3ef_N_d
};


/* ------------------------ TMF_d1_c2_4ef --------------------- */

double _nrrd_TMF_d1_c2_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c2_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d1_c2_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c2_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c2_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c2_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c2_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c2_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c2_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c2_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c2_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c2_4ef = {
  "TMF_d1_c2_4ef",
  1, _nrrd_TMF_d1_c2_4ef_Sup, _nrrd_TMF_d1_c2_4ef_Int,
  _nrrd_TMF_d1_c2_4ef_1_f,  _nrrd_TMF_d1_c2_4ef_N_f,
  _nrrd_TMF_d1_c2_4ef_1_d,  _nrrd_TMF_d1_c2_4ef_N_d
};


/* ------------------------ TMF_d1_c3_1ef --------------------- */

double _nrrd_TMF_d1_c3_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c3_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d1_c3_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c3_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c3_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c3_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c3_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c3_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c3_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c3_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c3_1ef = {
  "TMF_d1_c3_1ef",
  1, _nrrd_TMF_d1_c3_1ef_Sup, _nrrd_TMF_d1_c3_1ef_Int,
  _nrrd_TMF_d1_c3_1ef_1_f,  _nrrd_TMF_d1_c3_1ef_N_f,
  _nrrd_TMF_d1_c3_1ef_1_d,  _nrrd_TMF_d1_c3_1ef_N_d
};


/* ------------------------ TMF_d1_c3_2ef --------------------- */

double _nrrd_TMF_d1_c3_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c3_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d1_c3_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c3_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c3_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c3_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c3_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c3_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c3_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c3_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c3_2ef = {
  "TMF_d1_c3_2ef",
  1, _nrrd_TMF_d1_c3_2ef_Sup, _nrrd_TMF_d1_c3_2ef_Int,
  _nrrd_TMF_d1_c3_2ef_1_f,  _nrrd_TMF_d1_c3_2ef_N_f,
  _nrrd_TMF_d1_c3_2ef_1_d,  _nrrd_TMF_d1_c3_2ef_N_d
};


/* ------------------------ TMF_d1_c3_3ef --------------------- */

double _nrrd_TMF_d1_c3_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c3_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d1_c3_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c3_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c3_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c3_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c3_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c3_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c3_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c3_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c3_3ef = {
  "TMF_d1_c3_3ef",
  1, _nrrd_TMF_d1_c3_3ef_Sup, _nrrd_TMF_d1_c3_3ef_Int,
  _nrrd_TMF_d1_c3_3ef_1_f,  _nrrd_TMF_d1_c3_3ef_N_f,
  _nrrd_TMF_d1_c3_3ef_1_d,  _nrrd_TMF_d1_c3_3ef_N_d
};


/* ------------------------ TMF_d1_c3_4ef --------------------- */

double _nrrd_TMF_d1_c3_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d1_c3_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d1_c3_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d1_c3_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d1_c3_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d1_c3_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d1_c3_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d1_c3_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d1_c3_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d1_c3_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d1_c3_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d1_c3_4ef = {
  "TMF_d1_c3_4ef",
  1, _nrrd_TMF_d1_c3_4ef_Sup, _nrrd_TMF_d1_c3_4ef_Int,
  _nrrd_TMF_d1_c3_4ef_1_f,  _nrrd_TMF_d1_c3_4ef_N_f,
  _nrrd_TMF_d1_c3_4ef_1_d,  _nrrd_TMF_d1_c3_4ef_N_d
};


/* ------------------------ TMF_d2_cn_1ef --------------------- */

double _nrrd_TMF_d2_cn_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_cn_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d2_cn_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_cn_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_cn_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_cn_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_cn_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_cn_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_cn_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_cn_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_cn_1ef = {
  "TMF_d2_cn_1ef",
  1, _nrrd_TMF_d2_cn_1ef_Sup, _nrrd_TMF_d2_cn_1ef_Int,
  _nrrd_TMF_d2_cn_1ef_1_f,  _nrrd_TMF_d2_cn_1ef_N_f,
  _nrrd_TMF_d2_cn_1ef_1_d,  _nrrd_TMF_d2_cn_1ef_N_d
};


/* ------------------------ TMF_d2_cn_2ef --------------------- */

double _nrrd_TMF_d2_cn_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_cn_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d2_cn_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_cn_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_cn_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_cn_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_cn_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_cn_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_cn_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_cn_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_cn_2ef = {
  "TMF_d2_cn_2ef",
  1, _nrrd_TMF_d2_cn_2ef_Sup, _nrrd_TMF_d2_cn_2ef_Int,
  _nrrd_TMF_d2_cn_2ef_1_f,  _nrrd_TMF_d2_cn_2ef_N_f,
  _nrrd_TMF_d2_cn_2ef_1_d,  _nrrd_TMF_d2_cn_2ef_N_d
};


/* ------------------------ TMF_d2_cn_3ef --------------------- */

double _nrrd_TMF_d2_cn_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_cn_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d2_cn_3ef_1_d(double x, const double *parm) {
  int i;

  
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_cn_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_cn_3ef_1_f(float x, const double *parm) {
  int i;

  
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_cn_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_cn_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_cn_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_cn_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_cn_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_cn_3ef = {
  "TMF_d2_cn_3ef",
  1, _nrrd_TMF_d2_cn_3ef_Sup, _nrrd_TMF_d2_cn_3ef_Int,
  _nrrd_TMF_d2_cn_3ef_1_f,  _nrrd_TMF_d2_cn_3ef_N_f,
  _nrrd_TMF_d2_cn_3ef_1_d,  _nrrd_TMF_d2_cn_3ef_N_d
};


/* ------------------------ TMF_d2_cn_4ef --------------------- */

double _nrrd_TMF_d2_cn_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_cn_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d2_cn_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_cn_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_cn_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_cn_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_cn_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_cn_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_cn_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_cn_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_cn_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_cn_4ef = {
  "TMF_d2_cn_4ef",
  1, _nrrd_TMF_d2_cn_4ef_Sup, _nrrd_TMF_d2_cn_4ef_Int,
  _nrrd_TMF_d2_cn_4ef_1_f,  _nrrd_TMF_d2_cn_4ef_N_f,
  _nrrd_TMF_d2_cn_4ef_1_d,  _nrrd_TMF_d2_cn_4ef_N_d
};


/* ------------------------ TMF_d2_c0_1ef --------------------- */

double _nrrd_TMF_d2_c0_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c0_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d2_c0_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c0_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c0_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c0_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c0_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c0_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c0_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c0_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c0_1ef = {
  "TMF_d2_c0_1ef",
  1, _nrrd_TMF_d2_c0_1ef_Sup, _nrrd_TMF_d2_c0_1ef_Int,
  _nrrd_TMF_d2_c0_1ef_1_f,  _nrrd_TMF_d2_c0_1ef_N_f,
  _nrrd_TMF_d2_c0_1ef_1_d,  _nrrd_TMF_d2_c0_1ef_N_d
};


/* ------------------------ TMF_d2_c0_2ef --------------------- */

double _nrrd_TMF_d2_c0_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c0_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d2_c0_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c0_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c0_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_2ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c0_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c0_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c0_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c0_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c0_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c0_2ef = {
  "TMF_d2_c0_2ef",
  1, _nrrd_TMF_d2_c0_2ef_Sup, _nrrd_TMF_d2_c0_2ef_Int,
  _nrrd_TMF_d2_c0_2ef_1_f,  _nrrd_TMF_d2_c0_2ef_N_f,
  _nrrd_TMF_d2_c0_2ef_1_d,  _nrrd_TMF_d2_c0_2ef_N_d
};


/* ------------------------ TMF_d2_c0_3ef --------------------- */

double _nrrd_TMF_d2_c0_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c0_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d2_c0_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c0_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c0_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c0_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c0_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c0_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c0_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c0_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c0_3ef = {
  "TMF_d2_c0_3ef",
  1, _nrrd_TMF_d2_c0_3ef_Sup, _nrrd_TMF_d2_c0_3ef_Int,
  _nrrd_TMF_d2_c0_3ef_1_f,  _nrrd_TMF_d2_c0_3ef_N_f,
  _nrrd_TMF_d2_c0_3ef_1_d,  _nrrd_TMF_d2_c0_3ef_N_d
};


/* ------------------------ TMF_d2_c0_4ef --------------------- */

double _nrrd_TMF_d2_c0_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c0_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d2_c0_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c0_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c0_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_4ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c0_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c0_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c0_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c0_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c0_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c0_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c0_4ef = {
  "TMF_d2_c0_4ef",
  1, _nrrd_TMF_d2_c0_4ef_Sup, _nrrd_TMF_d2_c0_4ef_Int,
  _nrrd_TMF_d2_c0_4ef_1_f,  _nrrd_TMF_d2_c0_4ef_N_f,
  _nrrd_TMF_d2_c0_4ef_1_d,  _nrrd_TMF_d2_c0_4ef_N_d
};


/* ------------------------ TMF_d2_c1_1ef --------------------- */

double _nrrd_TMF_d2_c1_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c1_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d2_c1_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c1_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c1_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c1_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c1_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c1_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c1_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c1_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c1_1ef = {
  "TMF_d2_c1_1ef",
  1, _nrrd_TMF_d2_c1_1ef_Sup, _nrrd_TMF_d2_c1_1ef_Int,
  _nrrd_TMF_d2_c1_1ef_1_f,  _nrrd_TMF_d2_c1_1ef_N_f,
  _nrrd_TMF_d2_c1_1ef_1_d,  _nrrd_TMF_d2_c1_1ef_N_d
};


/* ------------------------ TMF_d2_c1_2ef --------------------- */

double _nrrd_TMF_d2_c1_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c1_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d2_c1_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_2ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c1_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c1_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_2ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c1_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c1_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c1_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c1_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c1_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c1_2ef = {
  "TMF_d2_c1_2ef",
  1, _nrrd_TMF_d2_c1_2ef_Sup, _nrrd_TMF_d2_c1_2ef_Int,
  _nrrd_TMF_d2_c1_2ef_1_f,  _nrrd_TMF_d2_c1_2ef_N_f,
  _nrrd_TMF_d2_c1_2ef_1_d,  _nrrd_TMF_d2_c1_2ef_N_d
};


/* ------------------------ TMF_d2_c1_3ef --------------------- */

double _nrrd_TMF_d2_c1_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c1_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d2_c1_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c1_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c1_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c1_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c1_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c1_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c1_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c1_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c1_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c1_3ef = {
  "TMF_d2_c1_3ef",
  1, _nrrd_TMF_d2_c1_3ef_Sup, _nrrd_TMF_d2_c1_3ef_Int,
  _nrrd_TMF_d2_c1_3ef_1_f,  _nrrd_TMF_d2_c1_3ef_N_f,
  _nrrd_TMF_d2_c1_3ef_1_d,  _nrrd_TMF_d2_c1_3ef_N_d
};


/* ------------------------ TMF_d2_c1_4ef --------------------- */

double _nrrd_TMF_d2_c1_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c1_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 4;
}

double
_nrrd_TMF_d2_c1_4ef_1_d(double x, const double *parm) {
  int i;

  
  x += 4;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c1_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c1_4ef_1_f(float x, const double *parm) {
  int i;

  
  x += 4;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c1_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c1_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 4;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c1_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c1_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  
  for (I=0; I<len; I++) {
    t = x[I] + 4;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c1_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c1_4ef = {
  "TMF_d2_c1_4ef",
  1, _nrrd_TMF_d2_c1_4ef_Sup, _nrrd_TMF_d2_c1_4ef_Int,
  _nrrd_TMF_d2_c1_4ef_1_f,  _nrrd_TMF_d2_c1_4ef_N_f,
  _nrrd_TMF_d2_c1_4ef_1_d,  _nrrd_TMF_d2_c1_4ef_N_d
};


/* ------------------------ TMF_d2_c2_1ef --------------------- */

double _nrrd_TMF_d2_c2_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c2_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d2_c2_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c2_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c2_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c2_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c2_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c2_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c2_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c2_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c2_1ef = {
  "TMF_d2_c2_1ef",
  1, _nrrd_TMF_d2_c2_1ef_Sup, _nrrd_TMF_d2_c2_1ef_Int,
  _nrrd_TMF_d2_c2_1ef_1_f,  _nrrd_TMF_d2_c2_1ef_N_f,
  _nrrd_TMF_d2_c2_1ef_1_d,  _nrrd_TMF_d2_c2_1ef_N_d
};


/* ------------------------ TMF_d2_c2_2ef --------------------- */

double _nrrd_TMF_d2_c2_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c2_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d2_c2_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_2ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c2_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c2_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_2ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c2_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c2_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c2_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c2_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c2_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c2_2ef = {
  "TMF_d2_c2_2ef",
  1, _nrrd_TMF_d2_c2_2ef_Sup, _nrrd_TMF_d2_c2_2ef_Int,
  _nrrd_TMF_d2_c2_2ef_1_f,  _nrrd_TMF_d2_c2_2ef_N_f,
  _nrrd_TMF_d2_c2_2ef_1_d,  _nrrd_TMF_d2_c2_2ef_N_d
};


/* ------------------------ TMF_d2_c2_3ef --------------------- */

double _nrrd_TMF_d2_c2_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c2_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d2_c2_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c2_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c2_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c2_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c2_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c2_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c2_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c2_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c2_3ef = {
  "TMF_d2_c2_3ef",
  1, _nrrd_TMF_d2_c2_3ef_Sup, _nrrd_TMF_d2_c2_3ef_Int,
  _nrrd_TMF_d2_c2_3ef_1_f,  _nrrd_TMF_d2_c2_3ef_N_f,
  _nrrd_TMF_d2_c2_3ef_1_d,  _nrrd_TMF_d2_c2_3ef_N_d
};


/* ------------------------ TMF_d2_c2_4ef --------------------- */

double _nrrd_TMF_d2_c2_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c2_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 4;
}

double
_nrrd_TMF_d2_c2_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_4ef */
  x += 4;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c2_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c2_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_4ef */
  x += 4;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c2_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c2_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 4;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c2_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c2_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c2_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 4;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c2_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c2_4ef = {
  "TMF_d2_c2_4ef",
  1, _nrrd_TMF_d2_c2_4ef_Sup, _nrrd_TMF_d2_c2_4ef_Int,
  _nrrd_TMF_d2_c2_4ef_1_f,  _nrrd_TMF_d2_c2_4ef_N_f,
  _nrrd_TMF_d2_c2_4ef_1_d,  _nrrd_TMF_d2_c2_4ef_N_d
};


/* ------------------------ TMF_d2_c3_1ef --------------------- */

double _nrrd_TMF_d2_c3_1ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c3_1ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2;
}

double
_nrrd_TMF_d2_c3_1ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c3_1ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c3_1ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_1ef */
  x += 2;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c3_1ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c3_1ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c3_1ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c3_1ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_1ef */
  for (I=0; I<len; I++) {
    t = x[I] + 2;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c3_1ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c3_1ef = {
  "TMF_d2_c3_1ef",
  1, _nrrd_TMF_d2_c3_1ef_Sup, _nrrd_TMF_d2_c3_1ef_Int,
  _nrrd_TMF_d2_c3_1ef_1_f,  _nrrd_TMF_d2_c3_1ef_N_f,
  _nrrd_TMF_d2_c3_1ef_1_d,  _nrrd_TMF_d2_c3_1ef_N_d
};


/* ------------------------ TMF_d2_c3_2ef --------------------- */

double _nrrd_TMF_d2_c3_2ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c3_2ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d2_c3_2ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_2ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c3_2ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c3_2ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_2ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c3_2ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c3_2ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c3_2ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c3_2ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_2ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c3_2ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c3_2ef = {
  "TMF_d2_c3_2ef",
  1, _nrrd_TMF_d2_c3_2ef_Sup, _nrrd_TMF_d2_c3_2ef_Int,
  _nrrd_TMF_d2_c3_2ef_1_f,  _nrrd_TMF_d2_c3_2ef_N_f,
  _nrrd_TMF_d2_c3_2ef_1_d,  _nrrd_TMF_d2_c3_2ef_N_d
};


/* ------------------------ TMF_d2_c3_3ef --------------------- */

double _nrrd_TMF_d2_c3_3ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c3_3ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3;
}

double
_nrrd_TMF_d2_c3_3ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c3_3ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c3_3ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_3ef */
  x += 3;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c3_3ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c3_3ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c3_3ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c3_3ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_3ef */
  for (I=0; I<len; I++) {
    t = x[I] + 3;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c3_3ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c3_3ef = {
  "TMF_d2_c3_3ef",
  1, _nrrd_TMF_d2_c3_3ef_Sup, _nrrd_TMF_d2_c3_3ef_Int,
  _nrrd_TMF_d2_c3_3ef_1_f,  _nrrd_TMF_d2_c3_3ef_N_f,
  _nrrd_TMF_d2_c3_3ef_1_d,  _nrrd_TMF_d2_c3_3ef_N_d
};


/* ------------------------ TMF_d2_c3_4ef --------------------- */

double _nrrd_TMF_d2_c3_4ef_Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double _nrrd_TMF_d2_c3_4ef_Sup(const double *parm) {
  AIR_UNUSED(parm);
  return 4;
}

double
_nrrd_TMF_d2_c3_4ef_1_d(double x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_4ef */
  x += 4;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return TMF_d2_c3_4ef(parm[0], i, x);
}

float
_nrrd_TMF_d2_c3_4ef_1_f(float x, const double *parm) {
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_4ef */
  x += 4;
  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */
  x -= i;
  return AIR_CAST(float, TMF_d2_c3_4ef(parm[0], i, x));
}

void
_nrrd_TMF_d2_c3_4ef_N_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 4;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = TMF_d2_c3_4ef(parm[0], i, t);
  }
}

void
_nrrd_TMF_d2_c3_4ef_N_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t I;
  int i;

  AIR_UNUSED(parm); /* TMF_d2_c3_4ef */
  for (I=0; I<len; I++) {
    t = x[I] + 4;
    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */
    t -= i;
    f[I] = AIR_CAST(float, TMF_d2_c3_4ef(parm[0], i, t));
  }
}

NrrdKernel
_nrrdKernel_TMF_d2_c3_4ef = {
  "TMF_d2_c3_4ef",
  1, _nrrd_TMF_d2_c3_4ef_Sup, _nrrd_TMF_d2_c3_4ef_Int,
  _nrrd_TMF_d2_c3_4ef_1_f,  _nrrd_TMF_d2_c3_4ef_N_f,
  _nrrd_TMF_d2_c3_4ef_1_d,  _nrrd_TMF_d2_c3_4ef_N_d
};


NrrdKernel *const
nrrdKernelTMF[4][5][5] = {
  {            /* d = n */ 
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_dn_cn_1ef,
       &_nrrdKernel_TMF_dn_cn_2ef,
       &_nrrdKernel_TMF_dn_cn_3ef,
       &_nrrdKernel_TMF_dn_cn_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_dn_c0_1ef,
       &_nrrdKernel_TMF_dn_c0_2ef,
       &_nrrdKernel_TMF_dn_c0_3ef,
       &_nrrdKernel_TMF_dn_c0_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_dn_c1_1ef,
       &_nrrdKernel_TMF_dn_c1_2ef,
       &_nrrdKernel_TMF_dn_c1_3ef,
       &_nrrdKernel_TMF_dn_c1_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_dn_c2_1ef,
       &_nrrdKernel_TMF_dn_c2_2ef,
       &_nrrdKernel_TMF_dn_c2_3ef,
       &_nrrdKernel_TMF_dn_c2_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_dn_c3_1ef,
       &_nrrdKernel_TMF_dn_c3_2ef,
       &_nrrdKernel_TMF_dn_c3_3ef,
       &_nrrdKernel_TMF_dn_c3_4ef,
    },
  },
  {            /* d = 0 */ 
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d0_cn_1ef,
       &_nrrdKernel_TMF_d0_cn_2ef,
       &_nrrdKernel_TMF_d0_cn_3ef,
       &_nrrdKernel_TMF_d0_cn_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d0_c0_1ef,
       &_nrrdKernel_TMF_d0_c0_2ef,
       &_nrrdKernel_TMF_d0_c0_3ef,
       &_nrrdKernel_TMF_d0_c0_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d0_c1_1ef,
       &_nrrdKernel_TMF_d0_c1_2ef,
       &_nrrdKernel_TMF_d0_c1_3ef,
       &_nrrdKernel_TMF_d0_c1_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d0_c2_1ef,
       &_nrrdKernel_TMF_d0_c2_2ef,
       &_nrrdKernel_TMF_d0_c2_3ef,
       &_nrrdKernel_TMF_d0_c2_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d0_c3_1ef,
       &_nrrdKernel_TMF_d0_c3_2ef,
       &_nrrdKernel_TMF_d0_c3_3ef,
       &_nrrdKernel_TMF_d0_c3_4ef,
    },
  },
  {            /* d = 1 */ 
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d1_cn_1ef,
       &_nrrdKernel_TMF_d1_cn_2ef,
       &_nrrdKernel_TMF_d1_cn_3ef,
       &_nrrdKernel_TMF_d1_cn_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d1_c0_1ef,
       &_nrrdKernel_TMF_d1_c0_2ef,
       &_nrrdKernel_TMF_d1_c0_3ef,
       &_nrrdKernel_TMF_d1_c0_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d1_c1_1ef,
       &_nrrdKernel_TMF_d1_c1_2ef,
       &_nrrdKernel_TMF_d1_c1_3ef,
       &_nrrdKernel_TMF_d1_c1_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d1_c2_1ef,
       &_nrrdKernel_TMF_d1_c2_2ef,
       &_nrrdKernel_TMF_d1_c2_3ef,
       &_nrrdKernel_TMF_d1_c2_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d1_c3_1ef,
       &_nrrdKernel_TMF_d1_c3_2ef,
       &_nrrdKernel_TMF_d1_c3_3ef,
       &_nrrdKernel_TMF_d1_c3_4ef,
    },
  },
  {            /* d = 2 */ 
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d2_cn_1ef,
       &_nrrdKernel_TMF_d2_cn_2ef,
       &_nrrdKernel_TMF_d2_cn_3ef,
       &_nrrdKernel_TMF_d2_cn_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d2_c0_1ef,
       &_nrrdKernel_TMF_d2_c0_2ef,
       &_nrrdKernel_TMF_d2_c0_3ef,
       &_nrrdKernel_TMF_d2_c0_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d2_c1_1ef,
       &_nrrdKernel_TMF_d2_c1_2ef,
       &_nrrdKernel_TMF_d2_c1_3ef,
       &_nrrdKernel_TMF_d2_c1_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d2_c2_1ef,
       &_nrrdKernel_TMF_d2_c2_2ef,
       &_nrrdKernel_TMF_d2_c2_3ef,
       &_nrrdKernel_TMF_d2_c2_4ef,
    },
    {
       &_nrrdKernel_TMFBAD,
       &_nrrdKernel_TMF_d2_c3_1ef,
       &_nrrdKernel_TMF_d2_c3_2ef,
       &_nrrdKernel_TMF_d2_c3_3ef,
       &_nrrdKernel_TMF_d2_c3_4ef,
    },
  },
};

const unsigned int nrrdKernelTMF_maxD = 2;
const unsigned int nrrdKernelTMF_maxC = 3;
const unsigned int nrrdKernelTMF_maxA = 4;
