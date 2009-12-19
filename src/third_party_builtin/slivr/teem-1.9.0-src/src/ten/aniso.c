/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ten.h"
#include "privateTen.h"

/*
** learned: don't take sqrt(FLT_EPSILON) and expect it to still be
** negligible
*/

float  _tenAnisoEval_Cl1_f(const float  eval[3]) {
  float sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return (eval[0] - eval[1])/(FLT_EPSILON + sum);
}
double _tenAnisoEval_Cl1_d(const double eval[3]) {
  double sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return (eval[0] - eval[1])/(DBL_EPSILON + sum);
}

float  _tenAnisoEval_Cp1_f(const float  eval[3]) {
  float sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return 2*(eval[1] - eval[2])/(FLT_EPSILON + sum);
}
double _tenAnisoEval_Cp1_d(const double eval[3]) {
  double sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return 2*(eval[1] - eval[2])/(DBL_EPSILON + sum);
}

float  _tenAnisoEval_Ca1_f(const float  eval[3]) {
  float sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return (eval[0] + eval[1] - 2*eval[2])/(FLT_EPSILON + sum);
}
double _tenAnisoEval_Ca1_d(const double eval[3]) {
  double sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return (eval[0] + eval[1] - 2*eval[2])/(DBL_EPSILON + sum);
}

float  _tenAnisoEval_Cs1_f(const float  eval[3]) {
  float sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return 3*eval[2]/(FLT_EPSILON + sum);
}
double _tenAnisoEval_Cs1_d(const double eval[3]) {
  double sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return 3*eval[2]/(DBL_EPSILON + sum);
}

float  _tenAnisoEval_Ct1_f(const float  eval[3]) {
  float dem;
  dem = eval[0] + eval[1] - 2*eval[2];
  return dem ? 2*(eval[1] - eval[2])/dem : 0.0f;
}
double _tenAnisoEval_Ct1_d(const double eval[3]) {
  double dem;
  dem = eval[0] + eval[1] - 2*eval[2];
  return dem ? 2*(eval[1] - eval[2])/dem : 0.0;
}

float  _tenAnisoEval_Cl2_f(const float  eval[3]) {
  float eval0 = AIR_MAX(0, eval[0]);
  return (eval[0] - eval[1])/(FLT_EPSILON + eval0);
}
double _tenAnisoEval_Cl2_d(const double eval[3]) {
  double eval0 = AIR_MAX(0, eval[0]);
  return (eval[0] - eval[1])/(DBL_EPSILON + eval0);
}

float  _tenAnisoEval_Cp2_f(const float  eval[3]) {
  float eval0 = AIR_MAX(0, eval[0]);
  return (eval[1] - eval[2])/(FLT_EPSILON + eval0);
}
double _tenAnisoEval_Cp2_d(const double eval[3]) {
  double eval0 = AIR_MAX(0, eval[0]);
  return (eval[1] - eval[2])/(DBL_EPSILON + eval0);
}

float  _tenAnisoEval_Ca2_f(const float  eval[3]) {
  float eval0 = AIR_MAX(0, eval[0]);
  return (eval[0] - eval[2])/(FLT_EPSILON + eval0);
}
double _tenAnisoEval_Ca2_d(const double eval[3]) {
  double eval0 = AIR_MAX(0, eval[0]);
  return (eval[0] - eval[2])/(DBL_EPSILON + eval0);
}

float  _tenAnisoEval_Cs2_f(const float  eval[3]) {
  float eval0 = AIR_MAX(0, eval[0]);
  return eval[2]/(FLT_EPSILON + eval0);
}
double _tenAnisoEval_Cs2_d(const double eval[3]) {
  double eval0 = AIR_MAX(0, eval[0]);
  return eval[2]/(DBL_EPSILON + eval0);
}

float  _tenAnisoEval_Ct2_f(const float  eval[3]) {
  float denom;
  denom = eval[0] - eval[2];
  return denom ? (eval[1] - eval[2])/denom : 0.0f;
}
double _tenAnisoEval_Ct2_d(const double eval[3]) {
  double denom;
  denom = eval[0] - eval[2];
  return denom ? (eval[1] - eval[2])/denom : 0.0;
}

#define SQRT6 2.44948974278317809819
float  _tenAnisoEval_RA_f(const float  eval[3]) {
  float mean, stdv;
  mean = (eval[0] + eval[1] + eval[2])/3;
  stdv = AIR_CAST(float,
                  sqrt((mean-eval[0])*(mean-eval[0])   /* not exactly stdv */
                       + (mean-eval[1])*(mean-eval[1]) 
                       + (mean-eval[2])*(mean-eval[2])));
  return mean ? AIR_CAST(float, stdv/(mean*SQRT6)) : 0.0f;
}
double _tenAnisoEval_RA_d(const double eval[3]) {
  double mean, stdv;
  mean = (eval[0] + eval[1] + eval[2])/3;
  stdv = sqrt((mean-eval[0])*(mean-eval[0])   /* not exactly standard dev */
              + (mean-eval[1])*(mean-eval[1]) 
              + (mean-eval[2])*(mean-eval[2]));
  return mean ? stdv/(mean*SQRT6) : 0.0;
}

float  _tenAnisoEval_FA_f(const float  eval[3]) {
  float denom, mean, stdv;
  denom = 2.0f*(eval[0]*eval[0] + eval[1]*eval[1] + eval[2]*eval[2]);
  mean = (eval[0] + eval[1] + eval[2])/3;
  stdv = AIR_CAST(float,
                  sqrt((mean-eval[0])*(mean-eval[0]) /* not exactly stdv */
                       + (mean-eval[1])*(mean-eval[1]) 
                       + (mean-eval[2])*(mean-eval[2])));
  return denom ? AIR_CAST(float, stdv*sqrt(3.0/denom)) : 0.0f;
}
double _tenAnisoEval_FA_d(const double eval[3]) {
  double denom, mean, stdv;
  denom = 2.0*(eval[0]*eval[0] + eval[1]*eval[1] + eval[2]*eval[2]);
  mean = (eval[0] + eval[1] + eval[2])/3;
  stdv = sqrt((mean-eval[0])*(mean-eval[0])   /* not exactly standard dev */
              + (mean-eval[1])*(mean-eval[1]) 
              + (mean-eval[2])*(mean-eval[2]));
  return denom ? stdv*sqrt(3.0/denom) : 0.0;
}

float  _tenAnisoEval_VF_f(const float  eval[3]) {
  float mean;
  mean = (eval[0] + eval[1] + eval[2])/3.0f;
  return 1.0f - eval[0]*eval[1]*eval[2]/(mean*mean*mean);
}
double _tenAnisoEval_VF_d(const double eval[3]) {
  double mean;
  mean = (eval[0] + eval[1] + eval[2])/3;
  return 1.0 - eval[0]*eval[1]*eval[2]/(mean*mean*mean);
}

float  _tenAnisoEval_B_f(const float  eval[3]) {
  return eval[0]*eval[1] + eval[0]*eval[2] + eval[1]*eval[2];
}
double _tenAnisoEval_B_d(const double eval[3]) {
  return eval[0]*eval[1] + eval[0]*eval[2] + eval[1]*eval[2];
}

float  _tenAnisoEval_Q_f(const float  eval[3]) {
  float A, B;
  A = -(eval[0] + eval[1] + eval[2]);
  B = _tenAnisoEval_B_f(eval);
  return (A*A - 3.0f*B)/9.0f;
}
double _tenAnisoEval_Q_d(const double eval[3]) {
  double A, B;
  A = -(eval[0] + eval[1] + eval[2]);
  B = _tenAnisoEval_B_d(eval);
  return (A*A - 3.0*B)/9.0;
}

float  _tenAnisoEval_R_f(const float  eval[3]) {
  float A, B, C;
  A = -(eval[0] + eval[1] + eval[2]);
  B = _tenAnisoEval_B_f(eval);
  C = -eval[0]*eval[1]*eval[2];
  return (-2*A*A*A + 9*A*B - 27*C)/54;
}
double _tenAnisoEval_R_d(const double eval[3]) {
  double A, B, C;
  A = -(eval[0] + eval[1] + eval[2]);
  B = _tenAnisoEval_B_d(eval);
  C = -eval[0]*eval[1]*eval[2];
  return (-2*A*A*A + 9*A*B - 27*C)/54;
}

float  _tenAnisoEval_S_f(const float  eval[3]) {
  return eval[0]*eval[0] + eval[1]*eval[1] + eval[2]*eval[2];
}
double _tenAnisoEval_S_d(const double eval[3]) {
  return eval[0]*eval[0] + eval[1]*eval[1] + eval[2]*eval[2];
}

float  _tenAnisoEval_Skew_f(const float  eval[3]) {
  float Q;
  Q = _tenAnisoEval_Q_f(eval);
  return AIR_CAST(float, _tenAnisoEval_R_f(eval)/(FLT_EPSILON+sqrt(2*Q*Q*Q)));
}
double _tenAnisoEval_Skew_d(const double eval[3]) {
  double Q;
  Q = _tenAnisoEval_Q_d(eval);
  return _tenAnisoEval_R_d(eval)/(DBL_EPSILON + sqrt(2*Q*Q*Q));
}

float  _tenAnisoEval_Mode_f(const float  e[3]) {
  double n, d;
  n = (e[0] + e[1] - 2*e[2])*(2*e[0] - e[1] - e[2])*(e[0] - 2*e[1] + e[2]);
  d = sqrt(e[0]*e[0]+e[1]*e[1]+e[2]*e[2] - e[0]*e[1]-e[1]*e[2]-e[0]*e[2]);
  return AIR_CAST(float, n/(FLT_EPSILON + 2*d*d*d));
}
double _tenAnisoEval_Mode_d(const double e[3]) {
  double n, d;
  n = (e[0] + e[1] - 2*e[2])*(2*e[0] - e[1] - e[2])*(e[0] - 2*e[1] + e[2]);
  d = sqrt(e[0]*e[0]+e[1]*e[1]+e[2]*e[2] - e[0]*e[1]-e[1]*e[2]-e[0]*e[2]);
  return n/(DBL_EPSILON + 2*d*d*d);
}

float  _tenAnisoEval_Th_f(const float  eval[3]) {
  return AIR_CAST(float, acos(sqrt(2)*_tenAnisoEval_Skew_f(eval))/3);
}
double _tenAnisoEval_Th_d(const double eval[3]) {
  return acos(sqrt(2)*_tenAnisoEval_Skew_d(eval))/3;
}

float  _tenAnisoEval_Cz_f(const float  eval[3]) {
  return ((eval[0] + eval[1])/(FLT_EPSILON + eval[2]) 
          + (eval[1] + eval[2])/(FLT_EPSILON + eval[0]) 
          + (eval[0] + eval[2])/(FLT_EPSILON + eval[1]))/6;
}
double _tenAnisoEval_Cz_d(const double eval[3]) {
  return ((eval[0] + eval[1])/(DBL_EPSILON + eval[2]) 
          + (eval[1] + eval[2])/(DBL_EPSILON + eval[0]) 
          + (eval[0] + eval[2])/(DBL_EPSILON + eval[1]))/6;
}

float  _tenAnisoEval_Det_f(const float  eval[3]) {
  return eval[0]*eval[1]*eval[2];
}
double _tenAnisoEval_Det_d(const double eval[3]) {
  return eval[0]*eval[1]*eval[2];
}

float  _tenAnisoEval_Tr_f(const float  eval[3]) {
  return eval[0] + eval[1] + eval[2];
}
double _tenAnisoEval_Tr_d(const double eval[3]) {
  return eval[0] + eval[1] + eval[2];
}

float  _tenAnisoEval_eval0_f(const float  eval[3]) { return eval[0]; }
double _tenAnisoEval_eval0_d(const double eval[3]) { return eval[0]; }

float  _tenAnisoEval_eval1_f(const float  eval[3]) { return eval[1]; }
double _tenAnisoEval_eval1_d(const double eval[3]) { return eval[1]; }

float  _tenAnisoEval_eval2_f(const float  eval[3]) { return eval[2]; }
double _tenAnisoEval_eval2_d(const double eval[3]) { return eval[2]; }

float  (*_tenAnisoEval_f[TEN_ANISO_MAX+1])(const float  eval[3]) = {
  NULL,
  _tenAnisoEval_Cl1_f,
  _tenAnisoEval_Cp1_f,
  _tenAnisoEval_Ca1_f,
  _tenAnisoEval_Cs1_f,
  _tenAnisoEval_Ct1_f,
  _tenAnisoEval_Cl2_f,
  _tenAnisoEval_Cp2_f,
  _tenAnisoEval_Ca2_f,
  _tenAnisoEval_Cs2_f,
  _tenAnisoEval_Ct2_f,
  _tenAnisoEval_RA_f,
  _tenAnisoEval_FA_f,
  _tenAnisoEval_VF_f,
  _tenAnisoEval_B_f,
  _tenAnisoEval_Q_f,
  _tenAnisoEval_R_f,
  _tenAnisoEval_S_f,
  _tenAnisoEval_Skew_f,
  _tenAnisoEval_Mode_f,
  _tenAnisoEval_Th_f,
  _tenAnisoEval_Cz_f,
  _tenAnisoEval_Det_f,
  _tenAnisoEval_Tr_f,
  _tenAnisoEval_eval0_f,
  _tenAnisoEval_eval1_f,
  _tenAnisoEval_eval2_f
};
   
double (*_tenAnisoEval_d[TEN_ANISO_MAX+1])(const double eval[3]) = {
  NULL,
  _tenAnisoEval_Cl1_d,
  _tenAnisoEval_Cp1_d,
  _tenAnisoEval_Ca1_d,
  _tenAnisoEval_Cs1_d,
  _tenAnisoEval_Ct1_d,
  _tenAnisoEval_Cl2_d,
  _tenAnisoEval_Cp2_d,
  _tenAnisoEval_Ca2_d,
  _tenAnisoEval_Cs2_d,
  _tenAnisoEval_Ct2_d,
  _tenAnisoEval_RA_d,
  _tenAnisoEval_FA_d,
  _tenAnisoEval_VF_d,
  _tenAnisoEval_B_d,
  _tenAnisoEval_Q_d,
  _tenAnisoEval_R_d,
  _tenAnisoEval_S_d,
  _tenAnisoEval_Skew_d,
  _tenAnisoEval_Mode_d,
  _tenAnisoEval_Th_d,
  _tenAnisoEval_Cz_d,
  _tenAnisoEval_Det_d,
  _tenAnisoEval_Tr_d,
  _tenAnisoEval_eval0_d,
  _tenAnisoEval_eval1_d,
  _tenAnisoEval_eval2_d
};
   
float
tenAnisoEval_f(const float  eval[3], int aniso) {

  return (AIR_IN_OP(tenAnisoUnknown, aniso, tenAnisoLast)
          ? _tenAnisoEval_f[aniso](eval)
          : 0);
}

double
tenAnisoEval_d(const double eval[3], int aniso) {

  return (AIR_IN_OP(tenAnisoUnknown, aniso, tenAnisoLast)
          ? _tenAnisoEval_d[aniso](eval)
          : 0);
}

/*
******** tenAnisoCalc_f
**
** given an array of three SORTED (descending) eigenvalues "e",
** calculates the anisotropy coefficients of Westin et al.,
** as well as various others.
**
** NOTE: with time, so many metrics have ended up here that under
** no cases should this be used in any kind of time-critical operations
**
** This does NOT use biff.  
*/
void
tenAnisoCalc_f(float c[TEN_ANISO_MAX+1], const float e[3]) {
  float e0, e1, e2, stdv, mean, sum, cl, cp, ca, ra, fa, vf, denom;

  float A, B, C, R, Q, N, D;

  if (!( e[0] >= e[1] && e[1] >= e[2] )) {
    fprintf(stderr, "tenAnisoCalc_f: eigen values not sorted: "
            "%g %g %g (%d %d)\n",
            e[0], e[1], e[2], e[0] >= e[1], e[1] >= e[2]);
  }
  if ((tenVerbose > 1) && !( e[0] >= 0 && e[1] >= 0 && e[2] >= 0 )) {
    fprintf(stderr, "tenAnisoCalc_f: eigen values not all >= 0: %g %g %g\n",
            e[0], e[1], e[2]);
  }
  e0 = AIR_MAX(e[0], 0);
  e1 = AIR_MAX(e[1], 0);
  e2 = AIR_MAX(e[2], 0);
  sum = e0 + e1 + e2;
  
  /* first version of cl, cp, cs */
  cl = (e0 - e1)/(FLT_EPSILON + sum);
  c[tenAniso_Cl1] = cl;
  cp = 2*(e1 - e2)/(FLT_EPSILON + sum);
  c[tenAniso_Cp1] = cp;
  ca = cl + cp;
  c[tenAniso_Ca1] = ca;
  c[tenAniso_Cs1] = 1 - ca;
  c[tenAniso_Ct1] = ca ? cp/ca : 0;
  /* second version of cl, cp, cs */
  cl = (e0 - e1)/(FLT_EPSILON + e0);
  c[tenAniso_Cl2] = cl;
  cp = (e1 - e2)/(FLT_EPSILON + e0);
  c[tenAniso_Cp2] = cp;
  ca = cl + cp;
  c[tenAniso_Ca2] = ca;
  c[tenAniso_Cs2] = 1 - ca;
  c[tenAniso_Ct2] = ca ? cp/ca : 0;
  /* non-westin anisos */
  mean = sum/3.0f;
  stdv = AIR_CAST(float,
                  sqrt((mean-e0)*(mean-e0) /* okay, not exactly standard dev */
                       + (mean-e1)*(mean-e1) 
                       + (mean-e2)*(mean-e2)));
  ra = AIR_CAST(float, stdv/(FLT_EPSILON + mean*sqrt(6.0)));
  ra = AIR_CLAMP(0.0f, ra, 1.0f);
  c[tenAniso_RA] = ra;
  denom = 2.0f*(e0*e0 + e1*e1 + e2*e2);
  if (denom) {
    fa = AIR_CAST(float, stdv*sqrt(3.0/denom));
    fa = AIR_CLAMP(0.0f, fa, 1.0f);
  } else {
    fa = 0.0;
  }
  c[tenAniso_FA] = fa;
  vf = 1 - e0*e1*e2/(mean*mean*mean);
  vf = AIR_CLAMP(0.0f, vf, 1.0f);
  c[tenAniso_VF] = vf;

  A = (-e0 - e1 - e2);
  B = c[tenAniso_B] = e0*e1 + e0*e2 + e1*e2;
  C = -e0*e1*e2;
  Q = c[tenAniso_Q] = (A*A - 3*B)/9;
  R = c[tenAniso_R] = (-2*A*A*A + 9*A*B - 27*C)/54;
  c[tenAniso_S] = e0*e0 + e1*e1 + e2*e2;
  c[tenAniso_Skew] = AIR_CAST(float, R/(FLT_EPSILON + sqrt(2*Q*Q*Q)));
  N = (e0 + e1 - 2*e2)*(2*e0 - e1 - e2)*(e0 - 2*e1 + e2);
  D = AIR_CAST(float, sqrt(e0*e0+e1*e1+e2*e2 - e0*e1-e1*e2-e0*e2));
  c[tenAniso_Mode] = N/(FLT_EPSILON + 2*D*D*D);
  c[tenAniso_Th] =
    AIR_CAST(float, acos(AIR_CLAMP(-1, sqrt(2)*c[tenAniso_Skew], 1))/3);
  c[tenAniso_Cz] = ((e0 + e1)/(FLT_EPSILON + e2) 
                    + (e1 + e2)/(FLT_EPSILON + e0) 
                    + (e0 + e2)/(FLT_EPSILON + e1))/6;
  c[tenAniso_Det] = e0*e1*e2;
  c[tenAniso_Tr] = e0 + e1 + e2;
  c[tenAniso_eval0] = e0;
  c[tenAniso_eval1] = e1;
  c[tenAniso_eval2] = e2;
  return;
}

int
tenAnisoPlot(Nrrd *nout, int aniso, unsigned int res, int whole, int nanout) {
  char me[]="tenAnisoMap", err[BIFF_STRLEN];
  float *out, c[TEN_ANISO_MAX+1], tmp;
  unsigned int x, y;
  float m0[3], m1[3], m2[3], c0, c1, c2, e[3];
  float S = 1/3.0f, L = 1.0f, P = 1/2.0f;  /* these make Westin's original
                                              (cl,cp,cs) align with the 
                                              barycentric coordinates */

  if (airEnumValCheck(tenAniso, aniso)) {
    sprintf(err, "%s: invalid aniso (%d)", me, aniso);
    biffAdd(TEN, err); return 1;
  }
  if (!(res > 2)) {
    sprintf(err, "%s: resolution (%d) invalid", me, res);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 2,
                        AIR_CAST(size_t, res), AIR_CAST(size_t, res))) {
    sprintf(err, "%s: ", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  out = (float *)nout->data;
  if (whole) {
    ELL_3V_SET(m0, 1, 0, 0);
    ELL_3V_SET(m1, 0, 1, 0);
    ELL_3V_SET(m2, 0, 0, 1);
  } else {
    ELL_3V_SET(m0, S, S, S);
    ELL_3V_SET(m1, L, 0, 0);
    ELL_3V_SET(m2, P, P, 0);
  }
  for (y=0; y<res; y++) {
    for (x=0; x<=y; x++) {
      /* (c0,c1,c2) are the barycentric coordinates */
      c0 = AIR_CAST(float, 1.0 - AIR_AFFINE(-0.5, y, res-0.5, 0.0, 1.0));
      c2 = AIR_CAST(float, AIR_AFFINE(-0.5, x, res-0.5, 0.0, 1.0));
      c1 = 1 - c0 - c2;
      e[0] = c0*m0[0] + c1*m1[0] + c2*m2[0];
      e[1] = c0*m0[1] + c1*m1[1] + c2*m2[1];
      e[2] = c0*m0[2] + c1*m1[2] + c2*m2[2];
      ELL_SORT3(e[0], e[1], e[2], tmp); /* got some warnings w/out this */
      tenAnisoCalc_f(c, e);
      out[x + res*y] = c[aniso];
    }
    if (nanout) {
      for (x=y+1; x<res; x++) {
        out[x + res*y] = AIR_NAN;
      }
    }
  }

  return 0;
}

int
tenAnisoVolume(Nrrd *nout, const Nrrd *nin, int aniso, double confThresh) {
  char me[]="tenAnisoVolume", err[BIFF_STRLEN];
  size_t N, I, copyI;
  float *out, *in, *tensor, eval[3], evec[9], c[TEN_ANISO_MAX+1];
  int map[NRRD_DIM_MAX];
  size_t sx, sy, sz, size[3], coord[3];

  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenAniso, aniso)) {
    sprintf(err, "%s: invalid aniso (%d)", me, aniso);
    biffAdd(TEN, err); return 1;
  }
  confThresh = AIR_CLAMP(0.0, confThresh, 1.0);

  size[0] = sx = nin->axis[1].size;
  size[1] = sy = nin->axis[2].size;
  size[2] = sz = nin->axis[3].size;
  N = sx*sy*sz;
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 3, sx, sy, sz)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  out = (float *)nout->data;
  in = (float *)nin->data;
  for (I=0; I<=N-1; I++) {
    /* tenVerbose = (I == 911327); */
    tensor = in + I*7;
    if (tensor[0] < confThresh) {
      out[I] = 0.0;
      continue;
    }
    tenEigensolve_f(eval, evec, tensor);
    if (!(AIR_EXISTS(eval[0]) && AIR_EXISTS(eval[1]) && AIR_EXISTS(eval[2]))) {
      copyI = I;
      NRRD_COORD_GEN(coord, size, 3, copyI);
      sprintf(err, "%s: not all eigenvalues exist (%g,%g,%g) at sample "
              "%d = (%d,%d,%d)",
              me, eval[0], eval[1], eval[2], (int)I,
              (int)coord[0], (int)coord[1], (int)coord[2]);
      biffAdd(TEN, err); return 1;
    }
    tenAnisoCalc_f(c, eval);
    out[I] = c[aniso];
  }
  ELL_3V_SET(map, 1, 2, 3);
  if (nrrdAxisInfoCopy(nout, nin, map, NRRD_AXIS_INFO_SIZE_BIT)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }

  return 0;
}

int
tenAnisoHistogram(Nrrd *nout, const Nrrd *nin, const Nrrd *nwght,
                  int right, int version, unsigned int res) {
  char me[]="tenAnisoHistogram", err[BIFF_STRLEN];
  size_t N, I;
  int csIdx, clIdx, cpIdx, xi, yi;
  float *tdata, *out, eval[3], evec[9], c[TEN_ANISO_MAX+1],
    cs, cl, cp, (*wlup)(const void *data, size_t idx), weight;
  unsigned int yres;

  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nwght) {
    if (nrrdCheck(nwght)) {
      sprintf(err, "%s: trouble with weighting nrrd", me);
      biffMove(TEN, err, NRRD); return 1;
    }
    if (nrrdElementNumber(nwght)
        != nrrdElementNumber(nin)/nrrdKindSize(nrrdKind3DMaskedSymMatrix) ) {
      sprintf(err, "%s: # elements in weight nrrd (" _AIR_SIZE_T_CNV 
              ") != # tensors (" _AIR_SIZE_T_CNV ")", me,
              nrrdElementNumber(nwght),
              nrrdElementNumber(nin)/nrrdKindSize(nrrdKind3DMaskedSymMatrix));
      biffAdd(TEN, err); return 1;
    }
  }
  if (!( 1 == version || 2 == version )) {
    sprintf(err, "%s: version (%d) wasn't 1 or 2", me, version);
    biffAdd(TEN, err); return 1;
  }
  if (!(res > 10)) {
    sprintf(err, "%s: resolution (%d) invalid", me, res);
    biffAdd(TEN, err); return 1;
  }
  if (right) {
    yres = AIR_CAST(unsigned int, AIR_CAST(double, res)/sqrt(3));
  } else {
    yres = res;
  }
  if (nwght) {
    wlup = nrrdFLookup[nwght->type];
  } else {
    wlup = NULL;
  }
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 2,
                        AIR_CAST(size_t, res), AIR_CAST(size_t, yres))) {
    sprintf(err, "%s: ", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  out = (float *)nout->data;
  tdata = (float *)nin->data;
  if (right || 1 == version) {
    clIdx = tenAniso_Cl1;
    cpIdx = tenAniso_Cp1;
    csIdx = tenAniso_Cs1;
  } else {
    clIdx = tenAniso_Cl2;
    cpIdx = tenAniso_Cp2;
    csIdx = tenAniso_Cs2;
  }
  N = nrrdElementNumber(nin)/nrrdKindSize(nrrdKind3DMaskedSymMatrix);
  for (I=0; I<N; I++) {
    tenEigensolve_f(eval, evec, tdata);
    tenAnisoCalc_f(c, eval);
    cl = c[clIdx];
    cp = c[cpIdx];
    cs = c[csIdx];
    if (right) {
      xi = AIR_CAST(unsigned int, cs*0 + cl*(res-1) + cp*0);
      yi = AIR_CAST(unsigned int, cs*0 + cl*(yres-1) + cp*(yres-1));
    } else {
      xi = AIR_CAST(unsigned int, cs*0 + cl*0 + cp*(res-1));
      yi = AIR_CAST(unsigned int, cs*0 + cl*(res-1) + cp*(res-1));
    }
    weight = wlup ? wlup(nwght->data, I) : 1.0f;
    out[xi + res*yi] += tdata[0]*weight;
    tdata += nrrdKindSize(nrrdKind3DMaskedSymMatrix);
  }
  
  return 0;
}

tenEvecRGBParm *
tenEvecRGBParmNew() {
  tenEvecRGBParm *rgbp;
  
  rgbp = AIR_CAST(tenEvecRGBParm *, calloc(1, sizeof(tenEvecRGBParm)));
  if (rgbp) {
    rgbp->which = 0;
    rgbp->aniso = tenAniso_Cl2;
    rgbp->confThresh = 0.5;
    rgbp->anisoGamma = 1.0;
    rgbp->gamma = 1.0;
    rgbp->bgGray = 0.0;
    rgbp->isoGray = 0.0;
    rgbp->maxSat = 1.0;
    rgbp->typeOut = nrrdTypeFloat;
    rgbp->genAlpha = AIR_FALSE;
  }
  return rgbp;
}

tenEvecRGBParm *
tenEvecRGBParmNix(tenEvecRGBParm *rgbp) {
  
  if (rgbp) {
    airFree(rgbp);
  }
  return NULL;
}

int
tenEvecRGBParmCheck(const tenEvecRGBParm *rgbp) {
  char me[]="tenEvecRGBParmCheck", err[BIFF_STRLEN];

  if (!rgbp) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( rgbp->which <= 2 )) {
    sprintf(err, "%s: which must be 0, 1, or 2 (not %u)", me, rgbp->which);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenAniso, rgbp->aniso)) {
    sprintf(err, "%s: anisotropy metric %d not valid", me, rgbp->aniso);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdTypeDefault != rgbp->typeOut
      && airEnumValCheck(nrrdType, rgbp->typeOut)) {
    sprintf(err, "%s: output type (%d) not valid", me, rgbp->typeOut);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

float
_tenEvecRGBComp_f(float conf, float aniso, float comp,
                  const tenEvecRGBParm *rgbp) {
  double X;

  X = AIR_ABS(comp);
  X = pow(X, 1.0/rgbp->gamma);
  X = AIR_LERP(rgbp->maxSat*aniso, rgbp->isoGray, X);
  return AIR_CAST(float, conf > rgbp->confThresh ? X : rgbp->bgGray);
}

double
_tenEvecRGBComp_d(double conf, double aniso, double comp,
                  const tenEvecRGBParm *rgbp) {
  double X;

  X = AIR_ABS(comp);
  X = pow(X, 1.0/rgbp->gamma);
  X = AIR_LERP(rgbp->maxSat*aniso, rgbp->isoGray, X);
  return conf > rgbp->confThresh ? X : rgbp->bgGray;
}

void
tenEvecRGBSingle_f(float RGB[3], float conf, const float eval[3], 
                   const float evec[3], const tenEvecRGBParm *rgbp) {
  float aniso;

  if (RGB && eval && rgbp) {
    aniso = tenAnisoEval_f(eval, rgbp->aniso);
    aniso = AIR_CAST(float, pow(aniso, 1.0/rgbp->anisoGamma));
    ELL_3V_SET(RGB,
               _tenEvecRGBComp_f(conf, aniso, evec[0], rgbp),
               _tenEvecRGBComp_f(conf, aniso, evec[1], rgbp),
               _tenEvecRGBComp_f(conf, aniso, evec[2], rgbp));
  }
  return;
}

void
tenEvecRGBSingle_d(double RGB[3], double conf, const double eval[3], 
                   const double evec[3], const tenEvecRGBParm *rgbp) {
  double aniso;

  if (RGB && eval && rgbp) {
    aniso = tenAnisoEval_d(eval, rgbp->aniso);
    aniso = pow(aniso, 1.0/rgbp->anisoGamma);
    ELL_3V_SET(RGB,
               _tenEvecRGBComp_d(conf, aniso, evec[0], rgbp),
               _tenEvecRGBComp_d(conf, aniso, evec[1], rgbp),
               _tenEvecRGBComp_d(conf, aniso, evec[2], rgbp));
  }
  return;
}

