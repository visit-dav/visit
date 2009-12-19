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


int
_baneClipAnswer_Absolute(int *countP, Nrrd *hvol, double *clipParm) {

  AIR_UNUSED(hvol);
  *countP = (int)(clipParm[0]);
  return 0;
}

int
_baneClipAnswer_PeakRatio(int *countP, Nrrd *hvol, double *clipParm) {
  int *hits, maxhits;
  size_t idx, num;
  
  hits = (int *)hvol->data;
  maxhits = 0;
  num = nrrdElementNumber(hvol);
  for (idx=0; idx<num; idx++) {
    maxhits = AIR_MAX(maxhits, hits[idx]);
  }

  *countP = (int)(maxhits*clipParm[0]);
  return 0;
}

int
_baneClipAnswer_Percentile(int *countP, Nrrd *hvol, double *clipParm) {
  char me[]="_baneClipAnswer_Percentile", err[BIFF_STRLEN];
  Nrrd *ncopy;
  int *hits, clip;
  size_t num, sum, out, outsofar, hi;

  if (nrrdCopy(ncopy=nrrdNew(), hvol)) {
    sprintf(err, "%s: couldn't create copy of histovol", me);
    biffMove(BANE, err, NRRD); return 1;
  }
  hits = (int *)ncopy->data;
  num = nrrdElementNumber(ncopy);
  qsort(hits, num, sizeof(int), nrrdValCompare[nrrdTypeInt]);
  sum = 0;
  for (hi=0; hi<num; hi++) {
    sum += hits[hi];
  }
  out = (size_t)(sum*clipParm[0]/100);
  outsofar = 0;
  hi = num-1;
  do {
    outsofar += hits[hi--];
  } while (outsofar < out);
  clip = hits[hi];
  nrrdNuke(ncopy);

  *countP = clip;
  return 0;
}

int
_baneClipAnswer_TopN(int *countP, Nrrd *hvol, double *clipParm) {
  char me[]="_baneClipAnwer_TopN", err[BIFF_STRLEN];
  Nrrd *copy;
  int *hits, tmp;
  size_t num;

  if (nrrdCopy(copy=nrrdNew(), hvol)) {
    sprintf(err, "%s: couldn't create copy of histovol", me);
    biffMove(BANE, err, NRRD); return 1;
  }
  hits = (int *)copy->data;
  num = nrrdElementNumber(copy);
  qsort(hits, num, sizeof(int), nrrdValCompare[nrrdTypeInt]);
  tmp = AIR_CLAMP(0, (int)clipParm[0], (int)num-1);
  *countP = hits[num-tmp-1];
  nrrdNuke(copy);

  return 0;
}

baneClip *
baneClipNew(int type, double *parm) {
  char me[]="baneClipNew", err[BIFF_STRLEN];
  baneClip *clip;

  if (!( AIR_IN_OP(baneClipUnknown, type, baneClipLast) )) {
    sprintf(err, "%s: baneClip %d invalid", me, type);
    biffAdd(BANE, err); return NULL;
  }
  if (!parm) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(BANE, err); return NULL;
  }
  if (!(AIR_EXISTS(parm[0]))) {
    sprintf(err, "%s: parm[0] doesn't exist", me);
    biffAdd(BANE, err); return NULL;
  }
  clip = (baneClip*)calloc(1, sizeof(baneClip));
  if (!clip) {
    sprintf(err, "%s: couldn't allocate baneClip!", me);
    biffAdd(BANE, err); return NULL;
  }
  clip->parm[0] = parm[0];
  clip->type = type;
  switch(type) {
  case baneClipAbsolute:
    sprintf(clip->name, "absolute");
    clip->answer = _baneClipAnswer_Absolute;
    break;
  case baneClipPeakRatio:
    sprintf(clip->name, "peak ratio");
    clip->answer = _baneClipAnswer_PeakRatio;
    break;
  case baneClipPercentile:
    sprintf(clip->name, "percentile");
    clip->answer = _baneClipAnswer_Percentile;
    break;
  case baneClipTopN:
    sprintf(clip->name, "top N");
    clip->answer = _baneClipAnswer_TopN;
    break;
  default:
    sprintf(err, "%s: sorry, baneClip %d not implemented", me, type);
    biffAdd(BANE, err); baneClipNix(clip); return NULL;
    break;
  }
  return clip;
}

int
baneClipAnswer(int *countP, baneClip *clip, Nrrd *hvol) {
  char me[]="baneClipAnswer", err[BIFF_STRLEN];

  if (!( countP && clip && hvol )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(BANE, err); return 0;
  }
  if (clip->answer(countP, hvol, clip->parm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(BANE, err); return 0;
  }
  return 0;
}

baneClip *
baneClipCopy(baneClip *clip) {
  char me[]="baneClipCopy", err[BIFF_STRLEN];
  baneClip *ret = NULL;
  
  ret = baneClipNew(clip->type, clip->parm);
  if (!ret) {
    sprintf(err, "%s: couldn't make new clip", me);
    biffAdd(BANE, err); return NULL;
  }
  return ret;
}

baneClip *
baneClipNix(baneClip *clip) {

  if (clip) {
    airFree(clip->name);
    airFree(clip);
  }
  return NULL;
}

