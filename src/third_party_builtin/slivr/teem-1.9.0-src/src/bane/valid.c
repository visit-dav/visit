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
baneInputCheck (Nrrd *nin, baneHVolParm *hvp) {
  char me[]="baneInputCheck", err[BIFF_STRLEN];
  int i;

  if (nrrdCheck(nin)) {
    sprintf(err, "%s: basic nrrd validity check failed", me);
    biffMove(BANE, err, NRRD); return 1;
  }
  if (3 != nin->dim) {
    sprintf(err, "%s: need a 3-dimensional nrrd (not %d)", me, nin->dim);
    biffAdd(BANE, err); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    sprintf(err, "%s: can't operate on block type", me);
    biffAdd(BANE, err); return 1;
  }
  if (!( AIR_EXISTS(nin->axis[0].spacing) && nin->axis[0].spacing != 0 &&
         AIR_EXISTS(nin->axis[1].spacing) && nin->axis[1].spacing != 0 &&
         AIR_EXISTS(nin->axis[2].spacing) && nin->axis[2].spacing != 0 )) {
    sprintf(err, "%s: must have non-zero existant spacing for all 3 axes", me);
    biffAdd(BANE, err); return 1;
  }
  for (i=0; i<=2; i++) {
    if (_baneAxisCheck(hvp->axis + i)) {
      sprintf(err, "%s: trouble with axis %d", me, i);
      biffAdd(BANE, err); return 1;
    }
  }
  if (!hvp->clip) {
    sprintf(err, "%s: got NULL baneClip", me);
    biffAdd(BANE, err); return 1;
  }

  /* all okay */
  return 0;
}

int
baneHVolCheck (Nrrd *hvol) {
  char me[]="baneHVolCheck", err[BIFF_STRLEN];

  if (3 != hvol->dim) {
    sprintf(err, "%s: need dimension to be 3 (not %d)", me, hvol->dim);
    biffAdd(BANE, err); return 1;
  }
  if (nrrdTypeUChar != hvol->type) {
    sprintf(err, "%s: need type to be %s (not %s)", 
            me, airEnumStr(nrrdType, nrrdTypeUChar),
            airEnumStr(nrrdType, hvol->type));
    biffAdd(BANE, err); return 1;
  }
  if (!( AIR_EXISTS(hvol->axis[0].min) && AIR_EXISTS(hvol->axis[0].max) && 
         AIR_EXISTS(hvol->axis[1].min) && AIR_EXISTS(hvol->axis[1].max) && 
         AIR_EXISTS(hvol->axis[2].min) && AIR_EXISTS(hvol->axis[2].max) )) {
    sprintf(err, "%s: axisMin and axisMax must be set for all axes", me);
    biffAdd(BANE, err); return 1;
  }
  /* 
  ** NOTE: For the time being, I'm giving up on enforcing a 
  ** particular kind of histogram volume ...
  if (strcmp(hvol->axis[0].label, baneMeasrGradMag->name)) {
    sprintf(err, "%s: expected \"%s\" on axis 0 label",
            me, baneMeasrGradMag->name);
    biffAdd(BANE, err); return 1;
  }
  if (strcmp(hvol->axis[1].label, baneMeasrLapl->name) &&
      strcmp(hvol->axis[1].label, baneMeasrHess->name)) {
    sprintf(err, "%s: expected a 2nd deriv. measr on axis 1 (%s or %s)",
            me, baneMeasrHess->name, baneMeasrLapl->name);
    biffAdd(BANE, err); return 1;    
  }
  if (strcmp(hvol->axis[2].label, baneMeasrVal->name)) {
    sprintf(err, "%s: expected \"%s\" on axis 2",
            me, baneMeasrVal->name);
    biffAdd(BANE, err); return 1;
  }
  */
  return 0;
}

int
baneInfoCheck (Nrrd *info, int wantDim) {
  char me[]="baneInfoCheck", err[BIFF_STRLEN];
  int gotDim;

  if (!info) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(BANE, err); return 1;
  }
  gotDim = info->dim;
  if (wantDim) {
    if (!(1 == wantDim || 2 == wantDim)) {
      sprintf(err, "%s: wantDim should be 1 or 2, not %d", me, wantDim);
      biffAdd(BANE, err); return 1;
    }
    if (wantDim+1 != gotDim) {
      sprintf(err, "%s: dim is %d, not %d", me, gotDim, wantDim+1);
      biffAdd(BANE, err); return 1;
    }
  }
  else {
    if (!(2 == gotDim || 3 == gotDim)) {
      sprintf(err, "%s: dim is %d, not 2 or 3", me, gotDim);
      biffAdd(BANE, err); return 1;
    }
  }
  if (nrrdTypeFloat != info->type) {
    sprintf(err, "%s: need data of type float", me);
    biffAdd(BANE, err); return 1;
  }
  if (2 != info->axis[0].size) {
    sprintf(err, "%s: 1st axis needs size 2 (not " _AIR_SIZE_T_CNV ")", 
            me, info->axis[0].size);
    biffAdd(BANE, err); return 1;
  }
  return 0;
}

int
banePosCheck (Nrrd *pos, int wantDim) {
  char me[]="banePosCheck", err[BIFF_STRLEN];
  int gotDim;

  if (!pos) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(BANE, err); return 1;
  }
  gotDim = pos->dim;
  if (wantDim) {
    if (!(1 == wantDim || 2 == wantDim)) {
      sprintf(err, "%s: wantDim should be 1 or 2, not %d", me, wantDim);
      biffAdd(BANE, err); return 1;
    }
    if (wantDim != gotDim) {
      sprintf(err, "%s: dim is %d, not %d", me, gotDim, wantDim);
      biffAdd(BANE, err); return 1;
    }
  }
  else {
    if (!(1 == gotDim || 2 == gotDim)) {
      sprintf(err, "%s: dim is %d, not 1 or 2", me, gotDim);
      biffAdd(BANE, err); return 1;
    }
  }
  if (nrrdTypeFloat != pos->type) {
    sprintf(err, "%s: need data of type float", me);
    biffAdd(BANE, err); return 1;
  }
  /* HEY? check for values in axisMin[0] and axisMax[0] ? */
  /* HEY? check for values in axisMin[0] and axisMax[0] ? */
  /* HEY? check for values in axisMin[1] and axisMax[1] ? */
  return 0;
}

int
baneBcptsCheck (Nrrd *Bcpts) {
  char me[]="baneBcptsCheck", err[BIFF_STRLEN];
  int i, len;
  float *data;

  if (2 != Bcpts->dim) {
    sprintf(err, "%s: need 2-dimensional (not %d)", me, Bcpts->dim);
    biffAdd(BANE, err); return 1;
  }
  if (2 != Bcpts->axis[0].size) {
    sprintf(err, "%s: axis#0 needs size 2 (not " _AIR_SIZE_T_CNV ")",
            me, Bcpts->axis[0].size);
    biffAdd(BANE, err); return 1;
  }
  if (nrrdTypeFloat != Bcpts->type) {
    sprintf(err, "%s: need data of type float", me);
    biffAdd(BANE, err); return 1;
  }
  len = Bcpts->axis[1].size;
  data = (float *)Bcpts->data;
  for (i=0; i<=len-2; i++) {
    if (!(data[0 + 2*i] <= data[0 + 2*(i+1)])) {
      sprintf(err, "%s: value coord %d (%g) not <= coord %d (%g)", me,
              i, data[0 + 2*i], i+1, data[0 + 2*(i+1)]);
      biffAdd(BANE, err); return 1;
    }
  }
  return 0;
}

