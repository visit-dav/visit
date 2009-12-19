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

#include "gage.h"
#include "privateGage.h"

void
gageShapeReset(gageShape *shape) {
  int i, ai;
  
  if (shape) {
    shape->defaultCenter = gageDefDefaultCenter;
    ELL_3V_SET(shape->size, 0, 0, 0);
    shape->center = nrrdCenterUnknown;
    shape->fromOrientation = AIR_FALSE;
    ELL_3V_SET(shape->spacing, AIR_NAN, AIR_NAN, AIR_NAN);
    for (i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      /* valgrind complained about AIR_NAN at -O2 */
      for (ai=0; ai<=2; ai++) {
        shape->fwScale[i][ai] = airNaN();
      }
    }
    ELL_3V_SET(shape->volHalfLen, AIR_NAN, AIR_NAN, AIR_NAN);
    ELL_3V_SET(shape->voxLen, AIR_NAN, AIR_NAN, AIR_NAN);
  }
  return;
}

gageShape *
gageShapeNew() {
  gageShape *shape;
  
  shape = (gageShape *)calloc(1, sizeof(gageShape));
  if (shape) {
    gageShapeReset(shape);
  }
  return shape;
}

gageShape *
gageShapeCopy(gageShape *shp) {
  gageShape *nhp;

  if ((nhp = gageShapeNew())) {
    /* no pointers, so this is easy */
    memcpy(nhp, shp, sizeof(gageShape));
  }
  return nhp;
}

gageShape *
gageShapeNix(gageShape *shape) {
  
  airFree(shape);
  return NULL;
}

/*
** _gageShapeSet
**
** we are serving two masters here.  If ctx is non-NULL, we are being called
** from within gage, and we are to be lax or strict according to the settings
** of ctx->parm.requireAllSpacings and ctx->parm.requireEqualCenters.  If
** ctx is NULL, gageShapeSet was called, in which case we go with lax
** behavior (nothing "required")
**
** This function has subsumed the old gageVolumeCheck, and hence has 
** become this weird beast- part error checker and part (gageShape)
** initializer.  Oh well...
*/
int
_gageShapeSet(const gageContext *ctx, gageShape *shape,
              const Nrrd *nin, unsigned int baseDim) {
  char me[]="_gageShapeSet", err[BIFF_STRLEN];
  int i, ai, cx, cy, cz, defCenter, statCalc[3];
  unsigned int minsize, sx, sy, sz, num[3];
  const NrrdAxisInfo *ax[3];
  double maxLen, xs, ys, zs, defSpacing,
    vecA[4], vecB[3], vecC[3], vecD[4],
    spcCalc[3], vecCalc[3][NRRD_SPACE_DIM_MAX], orig[NRRD_SPACE_DIM_MAX];

  /* ------ basic error checking */
  if (!( shape && nin )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err);  if (shape) { gageShapeReset(shape); }
    return 1;
  }
  if (nrrdCheck(nin)) {
    sprintf(err, "%s: basic nrrd validity check failed", me);
    biffMove(GAGE, err, NRRD); gageShapeReset(shape);
    return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    sprintf(err, "%s: need a non-block type nrrd", me);
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }
  if (!(nin->dim == 3 + baseDim)) {
    sprintf(err, "%s: nrrd should be %u-D, not %u-D",
            me, 3 + baseDim, nin->dim);
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }
  ax[0] = &(nin->axis[baseDim+0]);
  ax[1] = &(nin->axis[baseDim+1]);
  ax[2] = &(nin->axis[baseDim+2]);
  statCalc[0] = nrrdSpacingCalculate(nin, baseDim + 0,
                                     spcCalc + 0, vecCalc[0]);
  statCalc[1] = nrrdSpacingCalculate(nin, baseDim + 1,
                                     spcCalc + 1, vecCalc[1]);
  statCalc[2] = nrrdSpacingCalculate(nin, baseDim + 2,
                                     spcCalc + 2, vecCalc[2]);
  /* see if nrrdSpacingCalculate ever *failed* */
  if (nrrdSpacingStatusUnknown == statCalc[0] ||
      nrrdSpacingStatusUnknown == statCalc[1] ||
      nrrdSpacingStatusUnknown == statCalc[2]) {
    sprintf(err, "%s: nrrdSpacingCalculate trouble on axis %d, %d, or %d",
            me, baseDim + 0, baseDim + 1, baseDim + 2);
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }
  /* see if nrrdSpacingCalculate encountered an axis with no space
     direction in a nrrd that nominally has a surrounding space */
  if (nrrdSpacingStatusScalarWithSpace == statCalc[0] ||
      nrrdSpacingStatusScalarWithSpace == statCalc[1] ||
      nrrdSpacingStatusScalarWithSpace == statCalc[2]) {
    sprintf(err, "%s: nrrdSpacingCalculate weirdness on axis %d, %d, or %d",
            me, baseDim + 0, baseDim + 1, baseDim + 2);
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }
  if (!( (nrrdSpacingStatusDirection == statCalc[0] &&
          nrrdSpacingStatusDirection == statCalc[1] &&
          nrrdSpacingStatusDirection == statCalc[2])
         ||
         (nrrdSpacingStatusDirection != statCalc[0] &&
          nrrdSpacingStatusDirection != statCalc[1] &&
          nrrdSpacingStatusDirection != statCalc[2])
         )) {
    sprintf(err, "%s: inconsistent space directions use "
            "in axis %d, %d, and %d",
            me, baseDim + 0, baseDim + 1, baseDim + 2);
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }
  if (nrrdSpacingStatusDirection == statCalc[0] &&
      nrrdSpacingStatusDirection == statCalc[1] &&
      nrrdSpacingStatusDirection == statCalc[2]) {
    /* this will get reset to false in case of error */
    shape->fromOrientation = AIR_TRUE;
  } else {
    shape->fromOrientation = AIR_FALSE;
  }
  /* oh yea, we should make sure the space dimension is right! */
  if (shape->fromOrientation && 3 != nin->spaceDim) {
    sprintf(err, "%s: orientation space dimension %d != 3",
            me, nin->spaceDim);
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }

  /* ------ find centering (set shape->center) */
  /* HEY: when we have full orientation information (via spaceDirections
     and spaceOrigin) the centering information is moot for determining
     shape, but until all usage of gageShape stuff is properly overhauled
     to take orientation into account, we'll still set shape->center */
  cx = ax[0]->center;
  cy = ax[1]->center;
  cz = ax[2]->center;
  if (ctx && ctx->parm.requireEqualCenters) {
    if (!( cx == cy && cx == cz )) {
      sprintf(err, "%s: axes %d,%d,%d centerings (%s,%s,%s) not equal", me,
              baseDim+0, baseDim+1, baseDim+2,
              airEnumStr(nrrdCenter, cx),
              airEnumStr(nrrdCenter, cy),
              airEnumStr(nrrdCenter, cz));
      biffAdd(GAGE, err); gageShapeReset(shape);
      return 1;
    }
  } else {
    if ( (nrrdCenterUnknown != cx && nrrdCenterUnknown != cy && cx != cy) ||
         (nrrdCenterUnknown != cy && nrrdCenterUnknown != cz && cy != cz) ||
         (nrrdCenterUnknown != cx && nrrdCenterUnknown != cz && cx != cz) ) {
      sprintf(err, "%s: two known centerings (of %s,%s,%s) are unequal", me,
              airEnumStr(nrrdCenter, cx),
              airEnumStr(nrrdCenter, cy),
              airEnumStr(nrrdCenter, cz));
      biffAdd(GAGE, err); gageShapeReset(shape);
      return 1;
    }
  }
  defCenter = ctx ? ctx->parm.defaultCenter : shape->defaultCenter;
  shape->center = (nrrdCenterUnknown != cx ? cx
                   : (nrrdCenterUnknown != cy ? cy
                      : (nrrdCenterUnknown != cz ? cz
                         : defCenter)));

  /* ------ find sizes (set shape->size[0,1,2]) */
  sx = ax[0]->size;
  sy = ax[1]->size;
  sz = ax[2]->size;
  minsize = (nrrdCenterCell == shape->center ? 1 : 2);
  /* HEY: perhaps this should be relaxed if we have full orientation info */
  if (!(sx >= minsize && sy >= minsize && sz >= minsize )) {
    sprintf(err, "%s: sizes (%u,%u,%u) must all be >= %u "
            "(min number of %s-centered samples)", me, 
            sx, sy, sz, minsize, airEnumStr(nrrdCenter, shape->center));
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }
  shape->size[0] = sx;
  shape->size[1] = sy;
  shape->size[2] = sz;

  /* ------ find spacings (set shape->spacing[0,1,2]) */
  if (shape->fromOrientation) {
    xs = spcCalc[0];
    ys = spcCalc[1];
    zs = spcCalc[2];
  } else {
    xs = ax[0]->spacing;
    ys = ax[1]->spacing;
    zs = ax[2]->spacing;
    if (ctx && ctx->parm.requireAllSpacings) {
      if (!( AIR_EXISTS(xs) && AIR_EXISTS(ys) && AIR_EXISTS(zs) )) {
        sprintf(err, "%s: spacings for axes %d,%d,%d don't all exist",
                me, baseDim+0, baseDim+1, baseDim+2);
        biffAdd(GAGE, err); gageShapeReset(shape);
        return 1;
      }
    }
    /* there is no shape->defaultSpacing, we'll go out on a limb ... */
    defSpacing = ctx ? ctx->parm.defaultSpacing : nrrdDefaultSpacing;
    xs = AIR_EXISTS(xs) ? xs : defSpacing;
    ys = AIR_EXISTS(ys) ? ys : defSpacing;
    zs = AIR_EXISTS(zs) ? zs : defSpacing;
    if (!( xs != 0 && ys != 0 && zs != 0 )) {
      sprintf(err, "%s: spacings (%g,%g,%g) for axes %d,%d,%d not all "
              "non-zero", me, xs, ys, zs, baseDim+0, baseDim+1, baseDim+2);
      biffAdd(GAGE, err); gageShapeReset(shape);
      return 1;
    }
  }
  shape->spacing[0] = AIR_CAST(gage_t, AIR_ABS(xs));
  shape->spacing[1] = AIR_CAST(gage_t, AIR_ABS(ys));
  shape->spacing[2] = AIR_CAST(gage_t, AIR_ABS(zs));
  
  /* ------ set spacing-dependent filter weight scalings */
  for (i=0; i<GAGE_KERNEL_NUM; i++) {
    switch (i) {
    case gageKernel00:
    case gageKernel10:
    case gageKernel20:
      /* interpolation requires no re-weighting for non-unit spacing */
      for (ai=0; ai<=2; ai++) {
        shape->fwScale[i][ai] = 1.0;
      }
      break;
    case gageKernel11:
    case gageKernel21:
      for (ai=0; ai<=2; ai++) {
        shape->fwScale[i][ai] = 1.0f/(shape->spacing[ai]);
      }
      break;
    case gageKernel22:
      for (ai=0; ai<=2; ai++) {
        shape->fwScale[i][ai] = 
          1.0f/((shape->spacing[ai])*(shape->spacing[ai]));
      }
      break;
    }
  }

  /* ------ learn lengths for bounding nrrd in bi-unit cube
     (set shape->volHalfLen[0,1,2] and shape->voxLen[0,1,2]) */
  /* HEY: does it make any sense to be setting this if fromOrientation? */
  maxLen = 0.0;
  for (ai=0; ai<=2; ai++) {
    num[ai] = (nrrdCenterNode == shape->center
               ? shape->size[ai]-1
               : shape->size[ai]);
    shape->volHalfLen[ai] = num[ai]*shape->spacing[ai];
    maxLen = AIR_MAX(maxLen, shape->volHalfLen[ai]);
  }
  for (ai=0; ai<=2; ai++) {
    shape->volHalfLen[ai] /= maxLen;
    shape->voxLen[ai] = 2*shape->volHalfLen[ai]/num[ai];
  }

  /* ------ set transform matrices */
  if (shape->fromOrientation) {
    /* find translation vector (we check above that spaceDim == 3) */
    nrrdSpaceOriginGet(nin, orig);
    if (!( AIR_EXISTS(orig[0]) &&
           AIR_EXISTS(orig[1]) && 
           AIR_EXISTS(orig[2]) )) {
      /* don't have origin, so set it to come from the middle of volume */
      ELL_3V_SET(orig, 0.0f, 0.0f, 0.0f);
      ELL_3V_SCALE_INCR(orig, -(shape->size[0] - 1.0f)*spcCalc[0]/2.0f,
                        vecCalc[0]);
      ELL_3V_SCALE_INCR(orig, -(shape->size[1] - 1.0f)*spcCalc[1]/2.0f,
                        vecCalc[1]);
      ELL_3V_SCALE_INCR(orig, -(shape->size[2] - 1.0f)*spcCalc[2]/2.0f,
                        vecCalc[2]);
    }
    vecD[3] = 0;
    ELL_3V_SCALE(vecD, spcCalc[0], vecCalc[0]);
    ELL_4MV_COL0_SET(shape->ItoW, vecD);
    ELL_3V_SCALE(vecD, spcCalc[1], vecCalc[1]);
    ELL_4MV_COL1_SET(shape->ItoW, vecD);
    ELL_3V_SCALE(vecD, spcCalc[2], vecCalc[2]);
    ELL_4MV_COL2_SET(shape->ItoW, vecD);
    vecD[3] = 1;
    ELL_3V_COPY(vecD, orig);
    ELL_4MV_COL3_SET(shape->ItoW, vecD);
    /*
    fprintf(stderr, "%s: %g (%g,%g,%g)\n", me,
            spcCalc[0], vecCalc[0][0], vecCalc[0][1], vecCalc[0][2]);
    fprintf(stderr, "%s: %g (%g,%g,%g)\n", me,
            spcCalc[1], vecCalc[1][0], vecCalc[1][1], vecCalc[1][2]);
    fprintf(stderr, "%s: %g (%g,%g,%g)\n", me,
            spcCalc[2], vecCalc[2][0], vecCalc[2][1], vecCalc[2][2]);
    */
  } else {
    ELL_3V_SET(vecC, 0, 0, 0);
    gageShapeUnitItoW(shape, vecA, vecC);
    ELL_3V_SET(vecC, 1, 0, 0);
    gageShapeUnitItoW(shape, vecB, vecC);
    ELL_3V_SUB(vecD, vecB, vecA);
    vecD[3] = 0;
    ELL_4MV_COL0_SET(shape->ItoW, vecD);

    ELL_3V_SET(vecC, 0, 1, 0);
    gageShapeUnitItoW(shape, vecB, vecC);
    ELL_3V_SUB(vecD, vecB, vecA);
    vecD[3] = 0;
    ELL_4MV_COL1_SET(shape->ItoW, vecD);

    ELL_3V_SET(vecC, 0, 0, 1);
    gageShapeUnitItoW(shape, vecB, vecC);
    ELL_3V_SUB(vecD, vecB, vecA);
    vecD[3] = 0;
    ELL_4MV_COL2_SET(shape->ItoW, vecD);

    vecA[3] = 1;
    ELL_4MV_COL3_SET(shape->ItoW, vecA);
  }
  ell_4m_inv_d(shape->WtoI, shape->ItoW);

  return 0;
}

int
gageShapeSet(gageShape *shape, const Nrrd *nin, int baseDim) {
  char me[]="gageShapeSet", err[BIFF_STRLEN];

  if (_gageShapeSet(NULL, shape, nin, baseDim)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(GAGE, err); return 1;
  }
  return 0;
}

void
gageShapeUnitWtoI(gageShape *shape, double index[3], double world[3]) {
  int i;
  
  if (nrrdCenterNode == shape->center) {
    for (i=0; i<=2; i++) {
      index[i] = NRRD_NODE_IDX(-shape->volHalfLen[i], shape->volHalfLen[i],
                               shape->size[i], world[i]);
    }
  } else {
    for (i=0; i<=2; i++) {
      index[i] = NRRD_CELL_IDX(-shape->volHalfLen[i], shape->volHalfLen[i],
                               shape->size[i], world[i]);
    }
  }
}

void
gageShapeWtoI(gageShape *shape, double _index[3], double _world[3]) {
  double index[4], world[4];

  ELL_3V_COPY(world, _world);
  world[3] = 1.0;
  ELL_4MV_MUL(index, shape->WtoI, world);
  ELL_3V_SCALE(_index, 1.0/index[3], index);
}

void
gageShapeUnitItoW(gageShape *shape, double world[3], double index[3]) {
  int i;
  
  if (nrrdCenterNode == shape->center) {
    for (i=0; i<=2; i++) {
      world[i] = NRRD_NODE_POS(-shape->volHalfLen[i], shape->volHalfLen[i],
                               shape->size[i], index[i]);
    }
  } else {
    for (i=0; i<=2; i++) {
      world[i] = NRRD_CELL_POS(-shape->volHalfLen[i], shape->volHalfLen[i],
                               shape->size[i], index[i]);
    }
  }
}

void
gageShapeItoW(gageShape *shape, double _world[3], double _index[3]) {
  double world[4], index[4];

  ELL_3V_COPY(index, _index);
  index[3] = 1.0;
  ELL_4MV_MUL(world, shape->ItoW, index);
  ELL_3V_SCALE(_world, 1.0/world[3], world);
}

int
gageShapeEqual(gageShape *shape1, char *_name1,
               gageShape *shape2, char *_name2) {
  char me[]="_gageShapeEqual", err[BIFF_STRLEN],
    *name1, *name2, what[] = "???";

  name1 = _name1 ? _name1 : what;
  name2 = _name2 ? _name2 : what;
  if (!( shape1->fromOrientation == shape2->fromOrientation )) {
    sprintf(err, "%s: fromOrientation of %s (%s) != %s's (%s)", me,
            name1, shape1->fromOrientation ? "true" : "false",
            name2, shape2->fromOrientation ? "true" : "false");
    biffAdd(GAGE, err); return 0;
  }
  if (shape1->fromOrientation) {
    if (!( ELL_4M_EQUAL(shape1->ItoW,shape2->ItoW) )) {
      sprintf(err, "%s: ItoW matrices of %s and %s not the same", me,
              name1, name2);
      biffAdd(GAGE, err); return 0;
    }
  } else {
    if (!( shape1->size[0] == shape2->size[0] &&
           shape1->size[1] == shape2->size[1] &&
           shape1->size[2] == shape2->size[2] )) {
      sprintf(err, "%s: dimensions of %s (%u,%u,%u) != %s's (%u,%u,%u)", me,
              name1, 
              shape1->size[0], shape1->size[1], shape1->size[2],
              name2,
              shape2->size[0], shape2->size[1], shape2->size[2]);
      biffAdd(GAGE, err); return 0;
    }
    if (!( shape1->spacing[0] == shape2->spacing[0] &&
           shape1->spacing[1] == shape2->spacing[1] &&
           shape1->spacing[2] == shape2->spacing[2] )) {
      sprintf(err, "%s: spacings of %s (%g,%g,%g) != %s's (%g,%g,%g)", me,
              name1,
              shape1->spacing[0], shape1->spacing[1], shape1->spacing[2],
              name2,
              shape2->spacing[0], shape2->spacing[1], shape2->spacing[2]);
      biffAdd(GAGE, err); return 0;
    }
    if (!( shape1->center == shape2->center )) {
      sprintf(err, "%s: centering of %s (%s) != %s's (%s)", me,
              name1, airEnumStr(nrrdCenter, shape1->center),
              name2, airEnumStr(nrrdCenter, shape2->center));
      biffAdd(GAGE, err); return 0;
    }
  }

  return 1;
}
