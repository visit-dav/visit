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

#include "moss.h"
#include "privateMoss.h"

/*

 0  1  2
 3  4  5
 6  7  8

 a  c  tx
 b  d  ty
 0  0  1

 0  1  2
 3  4  5

*/

void
mossMatPrint (FILE *f, double *mat) {

  fprintf(f, "% 15.7f % 15.7f % 15.7f\n", 
          (float)mat[0], (float)mat[1], (float)mat[2]);
  fprintf(f, "% 15.7f % 15.7f % 15.7f\n", 
          (float)mat[3], (float)mat[4], (float)mat[5]);
}

double *
mossMatRightMultiply (double *_mat, double *_x) {
  double mat[9], x[9];
  
  MOSS_MAT_6TO9(x, _x);
  MOSS_MAT_6TO9(mat, _mat);
  ell_3m_pre_mul_d(mat, x);
  MOSS_MAT_9TO6(_mat, mat);
  return _mat;
}

double *
mossMatLeftMultiply (double *_mat, double *_x) {
  double mat[9], x[9];
  
  MOSS_MAT_6TO9(x, _x);
  MOSS_MAT_6TO9(mat, _mat);
  ell_3m_post_mul_d(mat, x);
  MOSS_MAT_9TO6(_mat, mat);
  return _mat;
}

double *
mossMatInvert (double *inv, double *mat) {
  double inv9[9], mat9[9];

  MOSS_MAT_6TO9(mat9, mat);
  ell_3m_inv_d(inv9, mat9);
  MOSS_MAT_9TO6(inv, inv9);
  return inv;
}

double *
mossMatIdentitySet (double *mat) {

  MOSS_MAT_SET(mat, 1, 0, 0, 0, 1, 0);
  return mat;
}

double *
mossMatTranslateSet (double *mat, double tx, double ty) {

  MOSS_MAT_SET(mat, 1, 0, tx, 0, 1, ty);
  return mat;
}

double *
mossMatRotateSet (double *mat, double angle) {

  angle *= AIR_PI/180.0;
  MOSS_MAT_SET(mat, cos(angle), -sin(angle), 0, sin(angle), cos(angle), 0);
  return mat;
}

double *
mossMatFlipSet (double *mat, double angle) {
  double rot[6], flip[6];

  MOSS_MAT_SET(flip, -1, 0, 0, 0, 1, 0);
  mossMatIdentitySet(mat);
  mossMatLeftMultiply(mat, mossMatRotateSet(rot, -angle));
  mossMatLeftMultiply(mat, flip);
  mossMatLeftMultiply(mat, mossMatRotateSet(rot, angle));
  return mat;
}

double *
mossMatShearSet (double *mat, double angleFixed, double amount) {
  double rot[6], shear[6];

  MOSS_MAT_SET(shear, 1, amount, 0, 0, 1, 0);
  mossMatIdentitySet(mat);
  mossMatLeftMultiply(mat, mossMatRotateSet(rot, -angleFixed));
  mossMatLeftMultiply(mat, shear);
  mossMatLeftMultiply(mat, mossMatRotateSet(rot, angleFixed));
  return mat;
}

double *
mossMatScaleSet (double *mat, double sx, double sy) {

  MOSS_MAT_SET(mat, sx, 0, 0, 0, sy, 0);
  return mat;
}

void
mossMatApply (double *ox, double *oy, double *mat, double ix, double iy) {
  
  *ox = mat[0]*ix + mat[1]*iy + mat[2];
  *oy = mat[3]*ix + mat[4]*iy + mat[5];
}

int
mossLinearTransform (Nrrd *nout, Nrrd *nin, float *bg,
                     double *mat, mossSampler *msp,
                     double xMin, double xMax,
                     double yMin, double yMax,
                     int xSize, int ySize) {
  char me[]="mossLinearTransform", err[BIFF_STRLEN];
  int ncol, xi, yi, ci, ax0, xCent, yCent;
  float *val, (*ins)(void *v, size_t I, float f), (*clamp)(float val);
  double inv[6], xInPos, xOutPos, yInPos, yOutPos;

  if (!(nout && nin && mat && msp && !mossImageCheck(nin))) {
    sprintf(err, "%s: got NULL pointer or bad image", me);
    biffAdd(MOSS, err); return 1;
  }
  if (mossSamplerImageSet(msp, nin, bg) || mossSamplerUpdate(msp)) {
    sprintf(err, "%s: trouble with sampler", me);
    biffAdd(MOSS, err); return 1;
  }
  if (!( xMin != xMax && yMin != yMax && xSize > 1 && ySize > 1 )) {
    sprintf(err, "%s: bad args: {x,y}Min == {x,y}Max or {x,y}Size <= 1", me);
    biffAdd(MOSS, err); return 1;
  }
  ax0 = MOSS_AXIS0(nin);
  if (!( AIR_EXISTS(nin->axis[ax0+0].min)
         && AIR_EXISTS(nin->axis[ax0+0].max)
         && AIR_EXISTS(nin->axis[ax0+1].min)
         && AIR_EXISTS(nin->axis[ax0+1].max) )) {
    sprintf(err, "%s: input axis min,max not set on axes %d and %d", me,
            ax0+0, ax0+1); biffAdd(MOSS, err); return 1;
  }

  ncol = MOSS_NCOL(nin);
  if (mossImageAlloc(nout, nin->type, xSize, ySize, ncol)) {
    sprintf(err, "%s: ", me); biffAdd(MOSS, err); return 1;
  }
  val = (float*)calloc(ncol, sizeof(float));
  if (nrrdCenterUnknown == nout->axis[ax0+0].center)
    nout->axis[ax0+0].center = _mossCenter(nin->axis[ax0+0].center);
  xCent = nout->axis[ax0+0].center;
  if (nrrdCenterUnknown == nout->axis[ax0+1].center)
    nout->axis[ax0+1].center = _mossCenter(nin->axis[ax0+1].center);
  yCent = nout->axis[ax0+1].center;
  nout->axis[ax0+0].min = xMin;
  nout->axis[ax0+0].max = xMax;
  nout->axis[ax0+1].min = yMin;
  nout->axis[ax0+1].max = yMax;
  ins = nrrdFInsert[nin->type];
  clamp = nrrdFClamp[nin->type];
  
  if (mossSamplerSample(val, msp, 0, 0)) {
    sprintf(err, "%s: trouble in sampler", me);
    free(val); biffAdd(MOSS, err); return 1;
  }

  mossMatInvert(inv, mat);
  for (yi=0; yi<ySize; yi++) {
    yOutPos = NRRD_POS(yCent, yMin, yMax, ySize, yi);
    for (xi=0; xi<xSize; xi++) {
      /*
      mossVerbose = ( (36 == xi && 72 == yi) ||
                      (37 == xi && 73 == yi) ||
                      (105 == xi && 175 == yi) );
      */
      xOutPos = NRRD_POS(xCent, xMin, xMax, xSize, xi);
      mossMatApply(&xInPos, &yInPos, inv, xOutPos, yOutPos);
      xInPos = NRRD_IDX(xCent, nin->axis[ax0+0].min, nin->axis[ax0+0].max,
                        nin->axis[ax0+0].size, xInPos);
      yInPos = NRRD_IDX(yCent, nin->axis[ax0+1].min, nin->axis[ax0+1].max,
                        nin->axis[ax0+1].size, yInPos);
      mossSamplerSample(val, msp, xInPos, yInPos);
      for (ci=0; ci<ncol; ci++) {
        ins(nout->data, ci + ncol*(xi + xSize*yi), clamp(val[ci]));
      }
    }
  }

  free(val);
  return 0;
}
