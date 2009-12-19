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


#include "bane.h"
#include "privateBane.h"

#define BIFF_NULL "%s: got NULL pointer"
#define BIFF_NRRDALLOC "%s: couldn't allocate output nrrd"

int
baneOpacInfo(Nrrd *info, Nrrd *hvol, int dim, int measr) {
  char me[]="baneOpacInfo", err[BIFF_STRLEN];
  Nrrd *proj2, *proj1, *projT;
  float *data2D, *data1D;
  int i, len, sv, sg;

  if (!(info && hvol)) {
    sprintf(err, BIFF_NULL, me); biffAdd(BANE, err); return 1;
  }
  if (!(1 == dim || 2 == dim)) {
    sprintf(err, "%s: got dimension %d, not 1 or 2", me, dim);
    biffAdd(BANE, err); return 1;
  }
  if (!(nrrdMeasureHistoMin == measr ||
        nrrdMeasureHistoMax == measr ||
        nrrdMeasureHistoMean == measr ||
        nrrdMeasureHistoMedian == measr ||
        nrrdMeasureHistoMode == measr)) {
    sprintf(err, "%s: measure %d doesn't make sense for histovolume",
            me, dim);
    biffAdd(BANE, err); return 1;
  }
  if (baneHVolCheck(hvol)) {
    sprintf(err, "%s: given nrrd doesn't seem to be a histogram volume", me);
    biffAdd(BANE, err); return 1;
  }
  if (1 == dim) {
    len = hvol->axis[2].size;
    if (nrrdMaybeAlloc_va(info, nrrdTypeFloat, 2,
                          AIR_CAST(size_t, 2),
                          AIR_CAST(size_t, len))) {
      sprintf(err, BIFF_NRRDALLOC, me);
      biffMove(BANE, err, NRRD); return 1;
    }
    info->axis[1].min = hvol->axis[2].min;
    info->axis[1].max = hvol->axis[2].max;
    data1D = (float *)info->data;

    /* sum up along 2nd deriv for each data value, grad mag */
    if (nrrdProject(proj2=nrrdNew(), hvol, 1,
                    nrrdMeasureSum, nrrdTypeDefault)) {
      sprintf(err, "%s: trouble projecting out 2nd deriv. for g(v)", me);
      biffMove(BANE, err, NRRD); return 1;
    }
    /* now determine average gradient at each value (0: grad, 1: value) */
    if (nrrdProject(proj1=nrrdNew(), proj2, 0, measr, nrrdTypeDefault)) {
      sprintf(err, "%s: trouble projecting along gradient for g(v)", me);
      biffMove(BANE, err, NRRD); return 1;
    }
    for (i=0; i<len; i++) {
      data1D[0 + 2*i] = nrrdFLookup[proj1->type](proj1->data, i);
    }
    nrrdNuke(proj1);
    nrrdNuke(proj2);

    /* sum up along gradient for each data value, 2nd deriv */
    if (nrrdProject(proj2 = nrrdNew(), hvol, 0,
                    nrrdMeasureSum, nrrdTypeDefault)) {
      sprintf(err, "%s: trouble projecting out gradient for h(v)", me);
      biffMove(BANE, err, NRRD); return 1;
    }
    /* now determine average gradient at each value (0: 2nd deriv, 1: value) */
    if (nrrdProject(proj1 = nrrdNew(), proj2, 0, measr, nrrdTypeDefault)) {
      sprintf(err, "%s: trouble projecting along 2nd deriv. for h(v)", me);
      biffMove(BANE, err, NRRD); return 1;
    }
    for (i=0; i<len; i++) {
      data1D[1 + 2*i] = nrrdFLookup[proj1->type](proj1->data, i);
    }
    nrrdNuke(proj1);
    nrrdNuke(proj2);
  }
  else {
    /* 2 == dim */
    /* hvol axes: 0: grad, 1: 2nd deriv: 2: data value */
    sv = hvol->axis[2].size;
    sg = hvol->axis[0].size;
    if (nrrdMaybeAlloc_va(info, nrrdTypeFloat, 3,
                          AIR_CAST(size_t, 2),
                          AIR_CAST(size_t, sv),
                          AIR_CAST(size_t, sg))) {
      sprintf(err, BIFF_NRRDALLOC, me);
      biffMove(BANE, err, NRRD); return 1;
    }
    info->axis[1].min = hvol->axis[2].min;
    info->axis[1].max = hvol->axis[2].max;
    info->axis[2].min = hvol->axis[0].min;
    info->axis[2].max = hvol->axis[0].max;
    data2D = (float *)info->data;

    /* first create h(v,g) */
    if (nrrdProject(proj2=nrrdNew(), hvol, 1, measr, nrrdTypeDefault)) {
      sprintf(err, "%s: trouble projecting (step 1) to create h(v,g)", me);
      biffMove(BANE, err, NRRD); return 1;
    }
    if (nrrdAxesSwap(projT=nrrdNew(), proj2, 0, 1)) {
      sprintf(err, "%s: trouble projecting (step 2) to create h(v,g)", me);
      biffMove(BANE, err, NRRD); return 1;
    }
    for (i=0; i<sv*sg; i++) {
      data2D[0 + 2*i] = nrrdFLookup[projT->type](projT->data, i);
    }
    nrrdNuke(proj2);
    nrrdNuke(projT);

    /* then create #hits(v,g) */
    if (nrrdProject(proj2=nrrdNew(), hvol, 1,
                    nrrdMeasureSum, nrrdTypeDefault)) {
      sprintf(err, "%s: trouble projecting (step 1) to create #(v,g)", me);
      biffMove(BANE, err, NRRD); return 1;
    }
    if (nrrdAxesSwap(projT=nrrdNew(), proj2, 0, 1)) {
      sprintf(err, "%s: trouble projecting (step 2) to create #(v,g)", me);
      biffMove(BANE, err, NRRD); return 1;
    }
    for (i=0; i<sv*sg; i++) {
      data2D[1 + 2*i] = nrrdFLookup[projT->type](projT->data, i);
    }
    nrrdNuke(proj2);
    nrrdNuke(projT);
  }
  return 0;
}

int
bane1DOpacInfoFrom2D(Nrrd *info1D, Nrrd *info2D) {
  char me[]="bane1DOpacInfoFrom2D", err[BIFF_STRLEN];
  Nrrd *projH2=NULL, *projH1=NULL, *projN=NULL, *projG1=NULL;
  float *data1D;
  int i, len;
  
  if (!(info1D && info2D)) {
    sprintf(err, BIFF_NULL, me); biffAdd(BANE, err); return 1;
  }
  if (baneInfoCheck(info2D, 2)) {
    sprintf(err, "%s: didn't get valid 2D info", me);
    biffAdd(BANE, err); return 1;
  }
  
  len = info2D->axis[1].size;
  if (nrrdProject(projH2=nrrdNew(), info2D, 0,
                  nrrdMeasureProduct, nrrdTypeDefault)
      || nrrdProject(projH1=nrrdNew(), projH2, 1,
                     nrrdMeasureSum, nrrdTypeDefault)
      || nrrdProject(projN=nrrdNew(), info2D, 2,
                     nrrdMeasureSum, nrrdTypeDefault)
      || nrrdProject(projG1=nrrdNew(), info2D, 2,
                     nrrdMeasureHistoMean, nrrdTypeDefault)) {
    sprintf(err, "%s: trouble creating needed projections", me);
    biffAdd(BANE, err); return 1;
  }
  
  if (nrrdMaybeAlloc_va(info1D, nrrdTypeFloat, 2, 
                        AIR_CAST(size_t, 2),
                        AIR_CAST(size_t, len))) {
    sprintf(err, BIFF_NRRDALLOC, me);
    biffMove(BANE, err, NRRD); return 1;
  }
  info1D->axis[1].min = info2D->axis[1].min;
  info1D->axis[1].max = info2D->axis[1].max;
  data1D = (float *)info1D->data;

  for (i=0; i<len; i++) {
    data1D[0 + 2*i] = nrrdFLookup[projG1->type](projG1->data, 1 + 2*i);
    data1D[1 + 2*i] = (nrrdFLookup[projH1->type](projH1->data, i) / 
                       nrrdFLookup[projN->type](projN->data, 1 + 2*i));
  }
  nrrdNuke(projH2);
  nrrdNuke(projH1);
  nrrdNuke(projN);
  nrrdNuke(projG1);
  return 0;
}

int
_baneSigmaCalc1D(float *sP, Nrrd *info1D) {
  char me[]="_baneSigmaCalc1D", err[BIFF_STRLEN];
  int i, len;
  float maxg, maxh, minh, *data;
  
  len = info1D->axis[1].size;
  data = (float *)info1D->data;
  maxg = -1;
  maxh = -1;
  minh = 1;
  for (i=0; i<len; i++) {
    if (AIR_EXISTS(data[0 + 2*i]))
      maxg = AIR_MAX(maxg, data[0 + 2*i]);
    if (AIR_EXISTS(data[1 + 2*i])) {
      minh = AIR_MIN(minh, data[1 + 2*i]);
      maxh = AIR_MAX(maxh, data[1 + 2*i]);
    }
  }
  if (maxg == -1 || maxh == -1) {
    sprintf(err, "%s: saw only NaNs in 1D info!", me);
    biffAdd(BANE, err); return 1;
  }

  /* here's the actual calculation: from page 54 of GK's MS */
  /* This is after the typo report by Fernando Vega Higuera;
     the previous version of the code had a bug caused by
     mindless transcription of the erroneous equation 5.8 */
  *sP = AIR_CAST(float, 2*maxg/(sqrt(AIR_E)*(maxh - minh)));

  return 0;
}

int
baneSigmaCalc(float *sP, Nrrd *_info) {
  char me[]="baneSigmaCalc", err[BIFF_STRLEN];
  Nrrd *info;

  if (!(sP && _info)) { 
    sprintf(err, BIFF_NULL, me); biffAdd(BANE, err); return 1;
  }
  if (baneInfoCheck(_info, 0)) {
    sprintf(err, "%s: didn't get a valid info", me);
    biffAdd(BANE, err); return 1;
  }
  if (3 == _info->dim) {
    if (bane1DOpacInfoFrom2D(info = nrrdNew(), _info)) {
      sprintf(err, "%s: couldn't create 1D opac info from 2D", me);
      biffAdd(BANE, err); return 1;
    }
  }
  else {
    info = _info;
  }
  if (_baneSigmaCalc1D(sP, info)) {
    sprintf(err, "%s: trouble calculating sigma", me);
    biffAdd(BANE, err); return 1;
  }
  if (_info != info) {
    nrrdNuke(info);
  }
  return 0;
}

int
banePosCalc(Nrrd *pos, float sigma, float gthresh, Nrrd *info) {
  char me[]="banePosCalc", err[BIFF_STRLEN];
  int d, i, len, vi, gi, sv, sg;
  float *posData, *infoData, h, g, p;

  if (!(pos && info)) {
    sprintf(err, BIFF_NULL, me); biffAdd(BANE, err); return 1;
  }
  if (baneInfoCheck(info, 0)) {
    sprintf(err, "%s: didn't get a valid info", me);
    biffAdd(BANE, err); return 1;
  }
  d = info->dim-1;
  if (1 == d) {
    len = info->axis[1].size;
    if (nrrdMaybeAlloc_va(pos,  nrrdTypeFloat, 1,
                          AIR_CAST(size_t, len))) {
      sprintf(err, BIFF_NRRDALLOC, me); 
      biffMove(BANE, err, NRRD); return 1;
    }
    pos->axis[0].min = info->axis[1].min;
    pos->axis[0].max = info->axis[1].max;
    posData = (float *)pos->data;
    infoData = (float *)info->data;
    for (i=0; i<len; i++) {
      /* from pg. 55 of GK's MS */
      g = infoData[0+2*i];
      h = infoData[1+2*i];
      if (AIR_EXISTS(g) && AIR_EXISTS(h))
        p = -sigma*sigma*h/AIR_MAX(0, g-gthresh);
      else
        p = AIR_NAN;
      p = airIsInf_f(p) ? 10000 : p;
      posData[i] = p;
    }
  }
  else {
    /* 2 == d */
    sv = info->axis[1].size;
    sg = info->axis[2].size;
    if (nrrdMaybeAlloc_va(pos, nrrdTypeFloat, 2,
                          AIR_CAST(size_t, sv),
                          AIR_CAST(size_t, sg))) {
      sprintf(err, BIFF_NRRDALLOC, me); biffMove(BANE, err, NRRD); return 1;
    }
    pos->axis[0].min = info->axis[1].min;
    pos->axis[0].max = info->axis[1].max;
    pos->axis[1].min = info->axis[2].min;
    pos->axis[1].max = info->axis[2].max;
    posData = (float *)pos->data;
    for (gi=0; gi<sg; gi++) {
      g = AIR_CAST(float, AIR_AFFINE(0, gi, sg-1, 
                                     info->axis[2].min, info->axis[2].max));
      for (vi=0; vi<sv; vi++) {
        h = nrrdFLookup[info->type](info->data, 0 + 2*(vi + sv*gi));
        /* from pg. 61 of GK's MS */
        if (AIR_EXISTS(h)) {
          p = -sigma*sigma*h/AIR_MAX(0, g-gthresh);
        }
        else {
          p = AIR_NAN;
        }
        p = airIsInf_f(p) ? AIR_NAN : p;
        posData[vi + sv*gi] = p;
      }
    }
  }
  return 0;
}

void
_baneOpacCalcA(unsigned int lutLen, float *opacLut, 
               unsigned int numCpts, float *xo,
               float *pos) {
  unsigned int i, j;
  float p;

  for (i=0; i<lutLen; i++) {
    p = pos[i];
    if (!AIR_EXISTS(p)) {
      opacLut[i] = 0;
      continue;
    }
    if (p <= xo[0 + 2*0]) {
      opacLut[i] = xo[1 + 2*0];
      continue;
    }
    if (p >= xo[0 + 2*(numCpts-1)]) {
      opacLut[i] = xo[1 + 2*(numCpts-1)];
      continue;
    }
    for (j=1; j<numCpts; j++)
      if (p < xo[0 + 2*j])
        break;
    opacLut[i] = AIR_CAST(float, AIR_AFFINE(xo[0 + 2*(j-1)], p, xo[0 + 2*j], 
                                            xo[1 + 2*(j-1)], xo[1 + 2*j]));
  }
  /*
  for (i=0; i<numCpts; i++)
    printf("b(%g) = %g\n", xo[0+2*i], xo[1+2*i]);
  for (i=0; i<lutLen; i++)
    printf("p[%d] = %g -> o = %g\n", i, pos[i], opacLut[i]);
  */
}

void
_baneOpacCalcB(unsigned int lutLen, float *opacLut, 
               unsigned int numCpts, float *x, float *o,
               float *pos) {
  /* char me[]="_baneOpacCalcB"; */
  unsigned int i, j;
  double p, op;

  /*
  printf("%s(%d, %lu, %d, %lu, %lu, %lu): hello\n", me, lutLen,
         (unsigned long)opacLut, numCpts, 
         (unsigned long)x, (unsigned long)o, 
         (unsigned long)pos);
  */
  /*
  for (i=0; i<numCpts; i++) {
    printf("%s: opac(%g) = %g\n", me, x[i], o[i]);
  }
  printf("----------\n");
  */
  for (i=0; i<lutLen; i++) {
    p = pos[i];
    /*
    printf("%s: pos[%d] = %g -->", me, i, p); fflush(stdout);
    */
    if (!AIR_EXISTS(p)) {
      op = 0;
      goto endloop;
    }
    if (p <= x[0]) {
      op = o[0];
      goto endloop;
    }
    if (p >= x[numCpts-1]) {
      op = o[numCpts-1];
      goto endloop;
    }
    for (j=1; j<numCpts; j++)
      if (p < x[j])
        break;
    op = AIR_AFFINE(x[j-1], p, x[j], o[j-1], o[j]);
  endloop:
    opacLut[i] = AIR_CAST(float, op);
    /* 
    printf("opac[%d] = %g\n", i, op);
    */
  }
  /*
  printf("^^^^^^^^^\n");
  */
}

int
baneOpacCalc(Nrrd *opac, Nrrd *Bcpts, Nrrd *pos) {
  char me[]="baneOpacCalc", err[BIFF_STRLEN];
  int dim, sv, sg, len, npts;
  float *bdata, *odata, *pdata;

  if (!(opac && Bcpts && pos)) {
    sprintf(err, BIFF_NULL, me); biffAdd(BANE, err); return 1;
  }
  if (baneBcptsCheck(Bcpts)) {
    sprintf(err, "%s: didn't get valid control points for b(x)", me);
    biffAdd(BANE, err); return 1;
  }
  if (banePosCheck(pos, 0)) {
    sprintf(err, "%s: didn't get valid position data", me);
    biffAdd(BANE, err); return 1;
  }
  dim = pos->dim;
  if (1 == dim) {
    len = pos->axis[0].size;
    if (nrrdMaybeAlloc_va(opac, nrrdTypeFloat, 1,
                          AIR_CAST(size_t, len))) {
      sprintf(err, BIFF_NRRDALLOC, me); biffMove(BANE, err, NRRD); return 1;
    }
    opac->axis[0].min = pos->axis[0].min;
    opac->axis[0].max = pos->axis[0].max;
    odata = (float *)opac->data;
    bdata = (float *)Bcpts->data;
    pdata = (float *)pos->data;
    npts = Bcpts->axis[1].size;
    _baneOpacCalcA(len, odata, npts, bdata, pdata);
  }
  else {
    sv = pos->axis[0].size;
    sg = pos->axis[1].size;
    if (nrrdMaybeAlloc_va(opac, nrrdTypeFloat, 2,
                          AIR_CAST(size_t, sv),
                          AIR_CAST(size_t, sg))) {
      sprintf(err, BIFF_NRRDALLOC, me); biffMove(BANE, err, NRRD); return 1;
    }
    opac->axis[0].min = pos->axis[0].min;
    opac->axis[0].max = pos->axis[0].max;
    opac->axis[1].min = pos->axis[1].min;
    opac->axis[1].max = pos->axis[1].max;
    odata = (float *)opac->data;
    bdata = (float *)Bcpts->data;
    pdata = (float *)pos->data;
    npts = Bcpts->axis[1].size;
    _baneOpacCalcA(sv*sg, odata, npts, bdata, pdata);
  }
  return 0;
}
