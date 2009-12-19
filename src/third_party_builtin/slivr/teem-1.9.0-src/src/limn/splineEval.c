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


#include "limn.h"

void
_limnSplineIntervalFind_Unknown(int *ii, double *ff,
                                limnSpline *spline, double tt) {
  char me[]="_limnSplineIntervalFind_Unknown";

  AIR_UNUSED(ii);
  AIR_UNUSED(ff);
  AIR_UNUSED(spline);
  AIR_UNUSED(tt);
  fprintf(stderr, "%s: WARNING: spline type unset somewhere\n", me);
  return;
}

void
_limnSplineIntervalFind_NonWarp(int *ii, double *ff,
                                limnSpline *spline, double tt) {
  int N;

  N = spline->ncpt->axis[2].size + (spline->loop ? 1 : 0);
  tt = AIR_CLAMP(0, tt, N-1);
  *ii = (int)tt;
  *ff = tt - *ii;
  return;
}

void
_limnSplineIntervalFind_Warp(int *ii, double *ff,
                             limnSpline *spline, double tt) {
  int N;

  N = spline->ncpt->axis[2].size;
  tt = AIR_CLAMP(spline->time[0], tt, spline->time[N-1]);
  *ii = AIR_CLAMP(0, *ii, N-2);
  /* the last value of ii may be the right one */
  if (!AIR_IN_CL(spline->time[*ii], tt, spline->time[*ii+1])) {
    /* HEY: make this a binary search */
    for (*ii=0; *ii<N-2; (*ii)++) {
      if (AIR_IN_CL(spline->time[*ii], tt, spline->time[*ii+1])) {
        break;
      }
    }
  }
  *ff = (tt - spline->time[*ii])/(spline->time[*ii+1] - spline->time[*ii]);
  return;
}

typedef void (*_limnSplineIntervalFind_t)(int *, double *,
                                          limnSpline *, double);
_limnSplineIntervalFind_t
_limnSplineIntervalFind[LIMN_SPLINE_TYPE_MAX+1] = {
  _limnSplineIntervalFind_Unknown,
  _limnSplineIntervalFind_NonWarp,
  _limnSplineIntervalFind_Warp,
  _limnSplineIntervalFind_NonWarp,
  _limnSplineIntervalFind_NonWarp,
  _limnSplineIntervalFind_NonWarp
};

void
_limnSplineWeightsFind_Unknown(double *wght, limnSpline *spline, double f) {
  char me[]="_limnSplineWeights_Unknown";

  AIR_UNUSED(wght);
  AIR_UNUSED(spline);
  AIR_UNUSED(f);
  fprintf(stderr, "%s: WARNING: spline type unset somewhere\n", me);
  return;
}

void
_limnSplineWeightsFind_Linear(double *wght, limnSpline *spline, double f) {

  AIR_UNUSED(spline);
  ELL_4V_SET(wght, 0, 1-f, f, 0);
  /*
  fprintf(stderr, "%g ----> %g %g %g %g\n", f,
          wght[0], wght[1], wght[2], wght[3]);
  */
  return;
}

void
_limnSplineWeightsFind_Hermite(double *wght, limnSpline *spline, double f) {
  double f3, f2;

  AIR_UNUSED(spline);
  f3 = f*(f2 = f*f);
  ELL_4V_SET(wght, 
             2*f3 - 3*f2 + 1,
             f3 - 2*f2 + f,
             f3 - f2,
             -2*f3 + 3*f2);
  return;
}

void
_limnSplineWeightsFind_CubicBezier(double *wght,
                                   limnSpline *spline, double f) {
  double g;
  
  AIR_UNUSED(spline);
  g = 1 - f;
  ELL_4V_SET(wght,
             g*g*g,
             3*g*g*f,
             3*g*f*f,
             f*f*f);
  return;
}

/* lifted from nrrd/kernel.c */
#define _BCCUBIC(x, B, C)                                           \
  ((x) >= 2.0 ? 0 :                                                 \
  ((x) >= 1.0                                                       \
   ? (((-B/6 - C)*(x) + B + 5*C)*(x) -2*B - 8*C)*(x) + 4*B/3 + 4*C  \
   : ((2 - 3*B/2 - C)*(x) - 3 + 2*B + C)*(x)*(x) + 1 - B/3))

void
_limnSplineWeightsFind_BC(double *wght, limnSpline *spline, double f) {
  double B, C, f0, f1, f2, f3;
  
  B = spline->B;
  C = spline->C;
  f0 = f+1;
  f1 = f;
  f2 = AIR_ABS(f-1);
  f3 = AIR_ABS(f-2);
  ELL_4V_SET(wght,
             _BCCUBIC(f0, B, C),
             _BCCUBIC(f1, B, C),
             _BCCUBIC(f2, B, C),
             _BCCUBIC(f3, B, C));
  return;
}

typedef void (*_limnSplineWeightsFind_t)(double *, limnSpline *, double);

_limnSplineWeightsFind_t
_limnSplineWeightsFind[LIMN_SPLINE_TYPE_MAX+1] = {
  _limnSplineWeightsFind_Unknown,
  _limnSplineWeightsFind_Linear,
  _limnSplineWeightsFind_Hermite, /* TimeWarp */
  _limnSplineWeightsFind_Hermite,
  _limnSplineWeightsFind_CubicBezier,
  _limnSplineWeightsFind_BC
};

void
_limnSplineIndexFind(int *idx, limnSpline *spline, int ii) {
  int N, ti[4];

  N = spline->ncpt->axis[2].size;
  if (limnSplineTypeHasImplicitTangents[spline->type]) {
    if (spline->loop) {
      ELL_4V_SET(ti,
                 AIR_MOD(ii-1, N),
                 AIR_MOD(ii+0, N),
                 AIR_MOD(ii+1, N),
                 AIR_MOD(ii+2, N));
    } else {
      ELL_4V_SET(ti,
                 AIR_CLAMP(0, ii-1, N-1),
                 AIR_CLAMP(0, ii+0, N-1),
                 AIR_CLAMP(0, ii+1, N-1),
                 AIR_CLAMP(0, ii+2, N-1));
    }
    ELL_4V_SET(idx, 1 + 3*ti[0], 1 + 3*ti[1], 1 + 3*ti[2], 1 + 3*ti[3]);
  } else {
    if (spline->loop) {
      ELL_4V_SET(ti,
                 AIR_MOD(ii+0, N),
                 AIR_MOD(ii+0, N),
                 AIR_MOD(ii+1, N),
                 AIR_MOD(ii+1, N));
    } else {
      ELL_4V_SET(ti,
                 AIR_CLAMP(0, ii+0, N-1),
                 AIR_CLAMP(0, ii+0, N-1),
                 AIR_CLAMP(0, ii+1, N-1),
                 AIR_CLAMP(0, ii+1, N-1));
    }
    ELL_4V_SET(idx, 1 + 3*ti[0], 2 + 3*ti[1], 0 + 3*ti[2], 1 + 3*ti[3]);
  }
}

void
_limnSplineFinish_Unknown(double *out, limnSpline *spline,
                          int ii, double *wght) {
  char me[]="_limnSplineFinish_Unknown";
  
  AIR_UNUSED(out);
  AIR_UNUSED(spline);
  AIR_UNUSED(ii);
  AIR_UNUSED(wght);
  fprintf(stderr, "%s: WARNING: spline info unset somewhere\n", me);
  return;
}
  
void
_limnSplineFinish_Scalar(double *out, limnSpline *spline,
                         int ii, double *wght) {
  int idx[4];
  double *cpt;

  cpt = (double*)(spline->ncpt->data);
  _limnSplineIndexFind(idx, spline, ii);
  *out = (  wght[0]*cpt[idx[0]] + wght[1]*cpt[idx[1]] 
          + wght[2]*cpt[idx[2]] + wght[3]*cpt[idx[3]]);
  return;
}
  
void
_limnSplineFinish_2Vec(double *out, limnSpline *spline,
                       int ii, double *wght) {
  int idx[4];
  double *cpt;

  cpt = (double*)(spline->ncpt->data);
  _limnSplineIndexFind(idx, spline, ii);
  out[0] = (  wght[0]*cpt[0 + 2*idx[0]] + wght[1]*cpt[0 + 2*idx[1]] 
            + wght[2]*cpt[0 + 2*idx[2]] + wght[3]*cpt[0 + 2*idx[3]]);
  out[1] = (  wght[0]*cpt[1 + 2*idx[0]] + wght[1]*cpt[1 + 2*idx[1]] 
            + wght[2]*cpt[1 + 2*idx[2]] + wght[3]*cpt[1 + 2*idx[3]]);
  return;
}
  
void
_limnSplineFinish_3Vec(double *out, limnSpline *spline,
                       int ii, double *wght) {
  int idx[4];
  double *cpt;

  cpt = (double*)(spline->ncpt->data);
  _limnSplineIndexFind(idx, spline, ii);
  out[0] = (  wght[0]*cpt[0 + 3*idx[0]] + wght[1]*cpt[0 + 3*idx[1]] 
            + wght[2]*cpt[0 + 3*idx[2]] + wght[3]*cpt[0 + 3*idx[3]]);
  out[1] = (  wght[0]*cpt[1 + 3*idx[0]] + wght[1]*cpt[1 + 3*idx[1]] 
            + wght[2]*cpt[1 + 3*idx[2]] + wght[3]*cpt[1 + 3*idx[3]]);
  out[2] = (  wght[0]*cpt[2 + 3*idx[0]] + wght[1]*cpt[2 + 3*idx[1]] 
            + wght[2]*cpt[2 + 3*idx[2]] + wght[3]*cpt[2 + 3*idx[3]]);
  return;
}
  
void
_limnSplineFinish_Normal(double *out, limnSpline *spline,
                         int ii, double *wght) {

  AIR_UNUSED(out);
  AIR_UNUSED(spline);
  AIR_UNUSED(ii);
  AIR_UNUSED(wght);
  fprintf(stderr, "%s: NOT IMPLEMENTED\n", "_limnSplineFinish_Normal");
  return;
}
  
void
_limnSplineFinish_4Vec(double *out, limnSpline *spline,
                       int ii, double *wght) {
  int idx[4];
  double *cpt;

  cpt = (double*)(spline->ncpt->data);
  _limnSplineIndexFind(idx, spline, ii);
  out[0] = (  wght[0]*cpt[0 + 4*idx[0]] + wght[1]*cpt[0 + 4*idx[1]] 
            + wght[2]*cpt[0 + 4*idx[2]] + wght[3]*cpt[0 + 4*idx[3]]);
  out[1] = (  wght[0]*cpt[1 + 4*idx[0]] + wght[1]*cpt[1 + 4*idx[1]] 
            + wght[2]*cpt[1 + 4*idx[2]] + wght[3]*cpt[1 + 4*idx[3]]);
  out[2] = (  wght[0]*cpt[2 + 4*idx[0]] + wght[1]*cpt[2 + 4*idx[1]] 
            + wght[2]*cpt[2 + 4*idx[2]] + wght[3]*cpt[2 + 4*idx[3]]);
  out[3] = (  wght[0]*cpt[3 + 4*idx[0]] + wght[1]*cpt[3 + 4*idx[1]] 
            + wght[2]*cpt[3 + 4*idx[2]] + wght[3]*cpt[3 + 4*idx[3]]);
  return;
}

/*
** HEY: I have no whether Hermite splines work with this
*/
void
_limnSplineFinish_Quaternion(double *out, limnSpline *spline,
                             int ii, double *wght) {
  int idx[4];
  double *cpt;

  cpt = (double*)(spline->ncpt->data);
  _limnSplineIndexFind(idx, spline, ii);
  ell_q_avg4_d(out, LIMN_SPLINE_Q_AVG_EPS, wght, 
               cpt + 4*idx[0], cpt + 4*idx[1], 
               cpt + 4*idx[2], cpt + 4*idx[3]);
  return;
}

typedef void (*_limnSplineFinish_t)(double *, limnSpline *, int, double *);
_limnSplineFinish_t
_limnSplineFinish[LIMN_SPLINE_INFO_MAX+1] = {
  _limnSplineFinish_Unknown,
  _limnSplineFinish_Scalar,
  _limnSplineFinish_2Vec,
  _limnSplineFinish_3Vec,
  _limnSplineFinish_Normal,
  _limnSplineFinish_4Vec,
  _limnSplineFinish_Quaternion
};
  
void
limnSplineEvaluate(double *out, limnSpline *spline, double tt) {
  int ii=0;
  double ff, wght[4];
  
  if (out && spline) {
    _limnSplineIntervalFind[spline->type](&ii, &ff, spline, tt);
    _limnSplineWeightsFind[spline->type](wght, spline, ff);
    _limnSplineFinish[spline->info](out, spline, ii, wght);
  }
  return;
}

int
limnSplineNrrdEvaluate(Nrrd *nout, limnSpline *spline, Nrrd *nin) {
  char me[]="limnSplineNrrdEvaluate", err[BIFF_STRLEN];
  double tt, *out, (*lup)(const void *, size_t);
  int odim, infoSize;
  size_t I, M, size[NRRD_DIM_MAX+1];

  if (!(nout && spline && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  if (limnSplineInfoScalar == spline->info) {
    nrrdAxisInfoGet_va(nin, nrrdAxisInfoSize, size);
    infoSize = 1;
    odim = nin->dim;
  } else {
    nrrdAxisInfoGet_va(nin, nrrdAxisInfoSize, size+1);
    infoSize = size[0] = limnSplineInfoSize[spline->info];
    odim = 1 + nin->dim;
  }
  if (nrrdMaybeAlloc_nva(nout, nrrdTypeDouble, odim, size)) {
    sprintf(err, "%s: output allocation failed", me);
    biffMove(LIMN, err, NRRD); return 1;
  }
  lup = nrrdDLookup[nin->type];
  out = (double*)(nout->data);
  M = nrrdElementNumber(nin);
  for (I=0; I<M; I++) {
    tt = lup(nin->data, I);
    limnSplineEvaluate(out, spline, tt);
    out += infoSize;
  }

  /* HEY: peripheral info copying? */

  return 0;
}

int
limnSplineSample(Nrrd *nout, limnSpline *spline,
                 double minT, size_t M, double maxT) {
  char me[]="limnSplineSample", err[BIFF_STRLEN];
  airArray *mop;
  Nrrd *ntt;
  double *tt;
  size_t I;

  if (!(nout && spline)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, ntt=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdMaybeAlloc_va(ntt, nrrdTypeDouble, 1,
                        M)) {
    sprintf(err, "%s: trouble allocating tmp nrrd", me);
    biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
  }
  tt = (double*)(ntt->data);
  for (I=0; I<M; I++) {
    tt[I] = AIR_AFFINE(0, I, M-1, minT, maxT);
  }
  if (limnSplineNrrdEvaluate(nout, spline, ntt)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(LIMN, err); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

