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

#include "hoover.h"

hooverContext *
hooverContextNew() {
  hooverContext *ctx;

  ctx = (hooverContext *)calloc(1, sizeof(hooverContext));
  if (ctx) {
    ctx->cam = limnCameraNew();
    ELL_3V_SET(ctx->volSize, 0, 0, 0);
    ELL_3V_SET(ctx->volSpacing, AIR_NAN, AIR_NAN, AIR_NAN);
    ctx->volCentering = hooverDefVolCentering;
    ctx->imgSize[0] = ctx->imgSize[1] = 0;
    ctx->imgCentering = hooverDefImgCentering;
    ctx->user = NULL;
    ctx->numThreads = 1;
    ctx->workIdx = 0;
    ctx->workMutex = NULL;
    ctx->renderBegin = hooverStubRenderBegin;
    ctx->threadBegin = hooverStubThreadBegin;
    ctx->rayBegin = hooverStubRayBegin;
    ctx->sample = hooverStubSample;
    ctx->rayEnd = hooverStubRayEnd;
    ctx->threadEnd = hooverStubThreadEnd;
    ctx->renderEnd = hooverStubRenderEnd;
  }
  return(ctx);
}

int
hooverContextCheck(hooverContext *ctx) {
  char me[]="hooverContextCheck", err[BIFF_STRLEN];
  int sxe, sye, sze, minSize;

  if (!ctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(HOOVER, err); return 1;
  }
  if (airEnumValCheck(nrrdCenter, ctx->imgCentering)) {
    sprintf(err, "%s: pixel centering (%d) invalid", me, ctx->imgCentering);
    biffAdd(HOOVER, err); return 1;
  }
  if (airEnumValCheck(nrrdCenter, ctx->volCentering)) {
    sprintf(err, "%s: voxel centering (%d) invalid", me, ctx->volCentering);
    biffAdd(HOOVER, err); return 1;
  }
  if (limnCameraAspectSet(ctx->cam,
                          ctx->imgSize[0], ctx->imgSize[1], ctx->imgCentering)
      || limnCameraUpdate(ctx->cam)) {
    sprintf(err, "%s: trouble setting up camera", me);
    biffMove(HOOVER, err, LIMN); return 1;
  }
  minSize = (nrrdCenterCell == ctx->volCentering ? 1 : 2);
  if (!(ctx->volSize[0] >= minSize
        && ctx->volSize[1] >= minSize 
        && ctx->volSize[2] >= minSize)) {
    sprintf(err, "%s: volume dimensions (%dx%dx%d) too small", me,
            ctx->volSize[0], ctx->volSize[1], ctx->volSize[2]);
    biffAdd(HOOVER, err); return 1;
  }
  sxe = AIR_EXISTS(ctx->volSpacing[0]);
  sye = AIR_EXISTS(ctx->volSpacing[1]);
  sze = AIR_EXISTS(ctx->volSpacing[2]);
  if (!sxe && !sye && !sze) {
    /* none of the incoming spacings existed, we'll go out on a limb
       and assume unit spacing */
    ctx->volSpacing[0] = nrrdDefaultSpacing;
    ctx->volSpacing[1] = ctx->volSpacing[2] = ctx->volSpacing[0];
    fprintf(stderr, "%s: WARNING: assuming spacing %g for all axes\n",
            me, ctx->volSpacing[0]);
    /* HEY : nrrdDefaultSpacing need not be the same as gageParm's 
       defaultSpacing, but we don't know anything about gage here,
       so what else can we do? */
  } else if (sxe && sye && sze) {
    /* all existed */
    if (!(ctx->volSpacing[0] > 0.0
          && ctx->volSpacing[1] > 0.0
          && ctx->volSpacing[2] > 0.0)) {
      sprintf(err, "%s: volume spacing (%gx%gx%g) invalid", me,
              ctx->volSpacing[0], ctx->volSpacing[1], ctx->volSpacing[2]);
      biffAdd(HOOVER, err); return 1;
    }
  } else {
    /* some existed, some didn't */
    sprintf(err, "%s: spacings %g, %g, %g don't all exist or not", me,
            ctx->volSpacing[0], ctx->volSpacing[1], ctx->volSpacing[2]);
    biffAdd(HOOVER, err); return 1;
  }
  if (!(ctx->imgSize[0] > 0 && ctx->imgSize[1] > 0)) {
    sprintf(err, "%s: image dimensions (%dx%d) invalid", me,
            ctx->imgSize[0], ctx->imgSize[1]);
    biffAdd(HOOVER, err); return 1;
  }
  if (!(ctx->numThreads >= 1)) {
    sprintf(err, "%s: number threads (%d) invalid", me, ctx->numThreads);
    biffAdd(HOOVER, err); return 1;
  }
  if (!(ctx->numThreads <= HOOVER_THREAD_MAX)) {
    sprintf(err, "%s: sorry, number threads (%d) > max (%d)", me, 
            ctx->numThreads, HOOVER_THREAD_MAX);
    biffAdd(HOOVER, err); return 1;
  }
  if (!ctx->renderBegin) {
    sprintf(err, "%s: need a non-NULL begin rendering callback", me);
    biffAdd(HOOVER, err); return 1;
  }
  if (!ctx->rayBegin) {
    sprintf(err, "%s: need a non-NULL begin ray callback", me);
    biffAdd(HOOVER, err); return 1;
  }
  if (!ctx->threadBegin) {
    sprintf(err, "%s: need a non-NULL begin thread callback", me);
    biffAdd(HOOVER, err); return 1;
  }
  if (!ctx->sample) {
    sprintf(err, "%s: need a non-NULL sampler callback function", me);
    biffAdd(HOOVER, err); return 1;
  }
  if (!ctx->rayEnd) {
    sprintf(err, "%s: need a non-NULL end ray callback", me);
    biffAdd(HOOVER, err); return 1;
  }
  if (!ctx->threadEnd) {
    sprintf(err, "%s: need a non-NULL end thread callback", me);
    biffAdd(HOOVER, err); return 1;
  }
  if (!ctx->renderEnd) {
    sprintf(err, "%s: need a non-NULL end render callback", me);
    biffAdd(HOOVER, err); return 1;
  }

  return 0;
}

void
hooverContextNix(hooverContext *ctx) {

  if (ctx) {
    limnCameraNix(ctx->cam);
    /* workMutex is cleaned up at end of render */
    free(ctx);
  }
}

