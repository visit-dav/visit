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

int
tenSizeNormalize(Nrrd *nout, const Nrrd *nin, double _weight[3],
                 double amount, double target) {
  char me[]="tenSizeNormalize", err[BIFF_STRLEN];
  float *tin, *tout, eval[3], evec[9], size, weight[3];
  size_t N, I;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdCopy(nout, nin)) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(TEN, err, NRRD); return 1;
  }

  ELL_3V_COPY_TT(weight, float, _weight);
  size = weight[0] + weight[1] + weight[2];
  if (!size) {
    sprintf(err, "%s: some of eigenvalue weights is zero", me);
    biffAdd(TEN, err); return 1;
  }
  weight[0] /= size;
  weight[1] /= size;
  weight[2] /= size;
  /*
  fprintf(stderr, "!%s: real weight = %g %g %g; amount = %g\n",
          me, weight[0], weight[1], weight[2], amount);
  */
  tin = (float*)(nin->data);
  tout = (float*)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<=N-1; I++) {
    tenEigensolve_f(eval, evec, tin);
    size = (weight[0]*AIR_ABS(eval[0])
            + weight[1]*AIR_ABS(eval[1])
            + weight[2]*AIR_ABS(eval[2]));
    /*
    fprintf(stderr, "!%s: eval = %g %g %g --> size = %g --> ",
            me, eval[0], eval[1], eval[2], size);
    */
    ELL_3V_SET_TT(eval, float,
                  AIR_AFFINE(0, amount, 1, eval[0], target*eval[0]/size),
                  AIR_AFFINE(0, amount, 1, eval[1], target*eval[1]/size),
                  AIR_AFFINE(0, amount, 1, eval[2], target*eval[2]/size));
    /*
    fprintf(stderr, "%g %g %g\n", eval[0], eval[1], eval[2]);
    */
    tenMakeOne_f(tout, tin[0], eval, evec);
    tin += 7;
    tout += 7;
  }
  /* basic and per-axis info handled by nrrdCopy above */
  
  return 0;
}

int
tenSizeScale(Nrrd *nout, const Nrrd *nin, double amount) {
  char me[]="tenSizeScale", err[BIFF_STRLEN];
  size_t I, N;
  float *tin, *tout;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  tin = (float *)(nin->data);
  tout = (float *)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<N; I++) {
    TEN_T_SET_TT(tout, float,
                 tin[0],
                 amount*tin[1], amount*tin[2], amount*tin[3],
                 amount*tin[4], amount*tin[5],
                 amount*tin[6]);
    tin += 7;
    tout += 7;
  }
  /* basic and per-axis info handled by nrrdCopy above */

  return 0;
}


/*
******** tenAnisoScale
**
** scales the "deviatoric" part of a tensor up or down
*/
int
tenAnisoScale(Nrrd *nout, const Nrrd *nin, double scale,
              int fixDet, int makePositive) {
  char me[]="tenAnisoScale", err[BIFF_STRLEN];
  size_t I, N;
  float *tin, *tout, mean, eval[3], evec[9];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  tin = (float *)(nin->data);
  tout = (float *)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<N; I++) {
    tenEigensolve_f(eval, evec, tin);
    if (fixDet) {
      eval[0] = AIR_MAX(eval[0], 0.00001f);
      eval[1] = AIR_MAX(eval[1], 0.00001f);
      eval[2] = AIR_MAX(eval[2], 0.00001f);
      ELL_3V_SET_TT(eval, float, log(eval[0]), log(eval[1]), log(eval[2]));
    }
    mean = (eval[0] + eval[1] + eval[2])/3.0f;
    ELL_3V_SET_TT(eval, float,
                  AIR_LERP(scale, mean, eval[0]),
                  AIR_LERP(scale, mean, eval[1]),
                  AIR_LERP(scale, mean, eval[2]));
    if (fixDet) {
      ELL_3V_SET_TT(eval, float, exp(eval[0]), exp(eval[1]), exp(eval[2]));
    }
    if (eval[2] < 0 && makePositive) {
      eval[0] = AIR_MAX(eval[0], 0.0f);
      eval[1] = AIR_MAX(eval[1], 0.0f);
      eval[2] = AIR_MAX(eval[2], 0.0f);
    }
    tenMakeOne_f(tout, tin[0], eval, evec);
    tin += 7;
    tout += 7;
  }
  /* basic and per-axis info handled by nrrdCopy above */

  return 0;
}

/*
******** tenEigenvalueClamp
**
** enstates the given value as the lowest eigenvalue
*/
int
tenEigenvalueClamp(Nrrd *nout, const Nrrd *nin, double min, double max) {
  char me[]="tenEigenvalueClamp", err[BIFF_STRLEN];
  size_t I, N;
  float *tin, *tout, eval[3], evec[9];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  tin = (float *)(nin->data);
  tout = (float *)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<N; I++) {
    tenEigensolve_f(eval, evec, tin);
    if (AIR_EXISTS(min)) {
      ELL_3V_SET_TT(eval, float,
                    AIR_MAX(eval[0], min),
                    AIR_MAX(eval[1], min),
                    AIR_MAX(eval[2], min));
    }
    if (AIR_EXISTS(max)) {
      ELL_3V_SET_TT(eval, float,
                    AIR_MIN(eval[0], max),
                    AIR_MIN(eval[1], max),
                    AIR_MIN(eval[2], max));
    }
    tenMakeOne_f(tout, tin[0], eval, evec);
    tin += 7;
    tout += 7;
  }
  /* basic and per-axis info handled by nrrdCopy above */

  return 0;
}

/*
******** tenEigenvaluePower
**
** raises the eigenvalues to some power
*/
int
tenEigenvaluePower(Nrrd *nout, const Nrrd *nin, double expo) {
  char me[]="tenEigenvaluePower", err[BIFF_STRLEN];
  size_t I, N;
  float *tin, *tout, eval[3], evec[9];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  tin = (float *)(nin->data);
  tout = (float *)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<N; I++) {
    tenEigensolve_f(eval, evec, tin);
    ELL_3V_SET_TT(eval, float,
                  pow(eval[0], expo),
                  pow(eval[1], expo),
                  pow(eval[2], expo));
    tenMakeOne_f(tout, tin[0], eval, evec);
    tin += 7;
    tout += 7;
  }
  /* basic and per-axis info handled by nrrdCopy above */

  return 0;
}

/*
******** tenEigenvalueAdd
**
** adds something to all eigenvalues
*/
int
tenEigenvalueAdd(Nrrd *nout, const Nrrd *nin, double val) {
  char me[]="tenEigenvalueAdd", err[BIFF_STRLEN];
  size_t I, N;
  float *tin, *tout, eval[3], evec[9];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  tin = (float *)(nin->data);
  tout = (float *)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<N; I++) {
    tenEigensolve_f(eval, evec, tin);
    ELL_3V_SET_TT(eval, float,
                  eval[0] + val,
                  eval[1] + val,
                  eval[2] + val);
    tenMakeOne_f(tout, tin[0], eval, evec);
    tin += 7;
    tout += 7;
  }
  /* basic and per-axis info handled by nrrdCopy above */

  return 0;
}
