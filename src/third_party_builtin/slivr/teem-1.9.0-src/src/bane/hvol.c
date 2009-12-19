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

/* hz is defined in the aix system in /usr/include/sys/m_param.h .
   This causes compilation errors for later functions which want to use
   this as a variable.
*/
#ifdef hz
#  undef hz
#endif

/*
** learned: don't ever count on interleaved printfs to stdout
** and stderr to appear in the right order
*/

int
_baneAxisCheck (baneAxis *ax) {
  char me[]="_baneAxisCheck", err[BIFF_STRLEN];
  
  if (!(ax->res >= 2)) {
    sprintf(err, "%s: need resolution at least 2 (not %d)", me, ax->res);
    biffAdd(BANE, err); return 1;
  }
  if (!ax->measr) {
    sprintf(err, "%s: have NULL baneMeasr", me);
    biffAdd(BANE, err); return 1;
  }
  if (!ax->inc) {
    sprintf(err, "%s: have NULL baneInc", me);
    biffAdd(BANE, err); return 1;
  }

  /* all okay */
  return 0;
}

void
baneProbe(double val[3],
          Nrrd *nin, baneHVolParm *hvp, gageContext *ctx,
          unsigned int x, unsigned int y, unsigned int z) {
  float *data=NULL;

  if (hvp->makeMeasrVol) {
    data = ( (float*)(hvp->measrVol->data) 
             + 3*(x + nin->axis[0].size*(y + nin->axis[1].size*z)) );
  }
  if (!hvp->makeMeasrVol || !hvp->measrVolDone) {
    gageProbe(ctx,
              AIR_CAST(gage_t, x),
              AIR_CAST(gage_t, y),
              AIR_CAST(gage_t, z));
    val[0] = baneMeasrAnswer(hvp->axis[0].measr, ctx);
    val[1] = baneMeasrAnswer(hvp->axis[1].measr, ctx);
    val[2] = baneMeasrAnswer(hvp->axis[2].measr, ctx);
    if (hvp->makeMeasrVol) {
      data[0] = AIR_CAST(float, val[0]);
      data[1] = AIR_CAST(float, val[1]);
      data[2] = AIR_CAST(float, val[2]);
    }
  } else {
    val[0] = data[0];
    val[1] = data[1];
    val[2] = data[2];
  }
  return;
}

int
baneFindInclusion(double min[3], double max[3], 
                  Nrrd *nin, baneHVolParm *hvp, gageContext *ctx) {
  char me[]="baneFindInclusion", err[BIFF_STRLEN], prog[13],
    aname[3][AIR_STRLEN_SMALL] = {"grad-mag", "2nd deriv", "data value"};
  int sx, sy, sz, x, y, z, E, ai;
  baneInc *inc[3];
  /* HEY HEY HEY:  The variable "hist" is used before its value is set.
  Nrrd *hist[3];
  */
  double val[3];
  
  /* conveniance copies */
  sx = nin->axis[0].size;
  sy = nin->axis[1].size;
  sz = nin->axis[2].size;
  inc[0] = hvp->axis[0].inc;
  inc[1] = hvp->axis[1].inc;
  inc[2] = hvp->axis[2].inc;
  if (hvp->verbose) {
    fprintf(stderr, "%s: inclusions: %s %s %s\n", me,
            inc[0]->name, inc[1]->name, inc[2]->name);
    fprintf(stderr, "%s: measures: %s %s %s\n", me,
            hvp->axis[0].measr->name, hvp->axis[1].measr->name,
            hvp->axis[2].measr->name);
    /*
    fprintf(stderr, "%s: gage query:\n", me);
    ctx->pvl[0]->kind->queryPrint(stderr, ctx->pvl[0]->query);
    */
  }

  /* Determining the inclusion ranges for the histogram volume takes
     some work- either finding the min and max values of some measure,
     and/or making a histogram of them.  The needed work for the three
     measures should done simultaneously during a given pass through
     the volume, so we break up the work into three stages- "passA",
     "passB", and then the final determination of ranges, "ans".  Here
     we start with passA.  If the chosen inclusion methods don't have
     anything to do at this stage (the callback is NULL), we don't do
     anything */
  if (hvp->verbose) {
    fprintf(stderr, "%s: pass A of inclusion initialization ...       ", me);
    fflush(stderr);
  }
  if (inc[0]->process[0] 
      || inc[1]->process[0] 
      || inc[2]->process[0]) {
    /*
    fprintf(stderr, "%s: inclusion pass CBs = %p %p %p \n", me, 
            incPass[0], incPass[1], incPass[2]);
    */
    if (hvp->makeMeasrVol && !hvp->measrVol) {
      if (nrrdMaybeAlloc_va(hvp->measrVol=nrrdNew(), nrrdTypeFloat, 4,
                            AIR_CAST(size_t, 3),
                            AIR_CAST(size_t, sx),
                            AIR_CAST(size_t, sy),
                            AIR_CAST(size_t, sz))) {
        sprintf(err, "%s: couldn't allocate 3x%dx%dx%d VGH volume",
                me, sx, sy, sz);
        biffMove(BANE, err, NRRD); return 1;
      }
    }
    for (z=0; z<sz; z++) {
      for (y=0; y<sy; y++) {
        if (hvp->verbose && !((y+sy*z)%200)) {
          fprintf(stderr, "%s", airDoneStr(0, y+sy*z, sy*sz, prog));
          fflush(stderr);
        }
        for (x=0; x<sx; x++) {
          baneProbe(val, nin, hvp, ctx, x, y, z);
          if (inc[0]->process[0]) inc[0]->process[0](inc[0], val[0]);
          if (inc[1]->process[0]) inc[1]->process[0](inc[1], val[1]);
          if (inc[2]->process[0]) inc[2]->process[0](inc[2], val[2]);
        }
      }
    }
    if (hvp->makeMeasrVol) {
      hvp->measrVolDone = AIR_TRUE;
      /* nrrdSave("VGH.nrrd", hvp->measrVol, NULL); */
    }
  }
  if (hvp->verbose)
    fprintf(stderr, "\b\b\b\b\b\b  done\n");
  /* HEY HEY HEY:  The variable "hist" is used before its value is set.
  if (hvp->verbose > 1) {
    fprintf(stderr, "%s: after pass A; ranges: [%g,%g] [%g,%g] [%g,%g]\n", me,
            hist[0]->axis[0].min, hist[0]->axis[0].max, 
            hist[1]->axis[0].min, hist[1]->axis[0].max, 
            hist[2]->axis[0].min, hist[2]->axis[0].max);
  }
  */

  /* second stage of initialization, includes creating histograms */
  if (hvp->verbose) {
    fprintf(stderr, "%s: pass B of inclusion initialization ...       ", me);
    fflush(stderr);
  }
  if (inc[0]->process[1] 
      || inc[1]->process[1]
      || inc[2]->process[1]) {
    if (hvp->makeMeasrVol && !hvp->measrVol) {
      if (nrrdMaybeAlloc_va(hvp->measrVol=nrrdNew(), nrrdTypeFloat, 4,
                            AIR_CAST(size_t, 3),
                            AIR_CAST(size_t, sx),
                            AIR_CAST(size_t, sy),
                            AIR_CAST(size_t, sz))) {
        sprintf(err, "%s: couldn't allocate 3x%dx%dx%d VGH volume",
                me, sx, sy, sz);
        biffMove(BANE, err, NRRD); return 1;
      }
    }
    for (z=0; z<sz; z++) {
      for (y=0; y<sy; y++) {
        if (hvp->verbose && !((y+sy*z)%200)) {
          fprintf(stderr, "%s", airDoneStr(0, y+sy*z, sy*sz, prog));
          fflush(stderr);
        }
        for (x=0; x<sx; x++) {
          baneProbe(val, nin, hvp, ctx, x, y, z);
          if (inc[0]->process[1]) inc[0]->process[1](inc[0], val[0]);
          if (inc[1]->process[1]) inc[1]->process[1](inc[1], val[1]);
          if (inc[2]->process[1]) inc[2]->process[1](inc[2], val[2]);
        }
      }
    }
    if (hvp->makeMeasrVol) {
      hvp->measrVolDone = AIR_TRUE;
    }
  }
  if (hvp->verbose)
    fprintf(stderr, "\b\b\b\b\b\b  done\n");
  /* HEY HEY HEY:  The variable "hist" is used before its value is set.
  if (hvp->verbose > 1) {
    fprintf(stderr, "%s: after pass B; ranges: [%g,%g] [%g,%g] [%g,%g]\n", me,
            hist[0]->axis[0].min, hist[0]->axis[0].max, 
            hist[1]->axis[0].min, hist[1]->axis[0].max, 
            hist[2]->axis[0].min, hist[2]->axis[0].max);
  }
  */

  /* now the real work of determining the inclusion */
  if (hvp->verbose) {
    fprintf(stderr, "%s: determining inclusion ... ", me);
    fflush(stderr);
  }
  E = 0;
  if (!E) {
    ai = 0;
    E |= baneIncAnswer(inc[0], 0 + min, 0 + max);
  }
  if (!E) {
    ai = 1;
    E |= baneIncAnswer(inc[1], 1 + min, 1 + max);
  }
  if (!E) {
    ai = 2;
    E |= baneIncAnswer(inc[2], 2 + min, 2 + max);
  }
  if (E) {
    sprintf(err, "%s: problem calculating inclusion for axis %d (%s)",
            me, ai, aname[ai]);
    biffAdd(BANE, err); return 1;
  }
  if (hvp->verbose)
    fprintf(stderr, "done\n");

  /* HEY HEY HEY:  The variable "hist" is used before its value is set.
  nrrdNuke(hist[0]);
  nrrdNuke(hist[1]);
  nrrdNuke(hist[2]);
  */
  return 0;
}

int
baneMakeHVol(Nrrd *hvol, Nrrd *nin, baneHVolParm *hvp) {
  char me[]="baneMakeHVol", err[BIFF_STRLEN], prog[13];
  gageContext *ctx;
  gagePerVolume *pvl;
  int E, sx, sy, sz, shx, shy, shz, x, y, z, hx, hy, hz,
    *rhvdata, clipVal, hval, pad;
  /* these are doubles because ultimately the inclusion functions
     use doubles, because I wanted the most generality */
  double val[3], min[3], max[3];
  size_t hidx, included;
  float fracIncluded;
  unsigned char *nhvdata;
  Nrrd *rawhvol;
  airArray *mop;

  if (!(hvol && nin && hvp)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(BANE, err); return 1;
  }
  if (baneInputCheck(nin, hvp)) {
    sprintf(err, "%s: something wrong with input volume or parameters", me);
    biffAdd(BANE, err); return 1;
  }

  /* set up */
  sx = nin->axis[0].size;
  sy = nin->axis[1].size;
  sz = nin->axis[2].size;

  mop = airMopNew();
  ctx = gageContextNew();
  airMopAdd(mop, ctx, (airMopper)gageContextNix, airMopAlways);
  pvl = gagePerVolumeNew(ctx, nin, gageKindScl);
  gageParmSet(ctx, gageParmVerbose, 0);
  gageParmSet(ctx, gageParmRenormalize, AIR_CAST(gage_t, hvp->renormalize));
  gageParmSet(ctx, gageParmCheckIntegrals, AIR_TRUE);
  if (!hvp->k3pack) {
    sprintf(err, "%s: code currently assumes k3pack", me);
    biffAdd(BANE, err); airMopError(mop); return 1;
  }
  gageParmSet(ctx, gageParmK3Pack, AIR_CAST(gage_t, hvp->k3pack));
  E = 0;
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  if (!E) E |= gageKernelSet(ctx, gageKernel00, hvp->k[gageKernel00],
                             hvp->kparm[gageKernel00]);
  if (!E) E |= gageKernelSet(ctx, gageKernel11, hvp->k[gageKernel11],
                             hvp->kparm[gageKernel11]);
  if (!E) E |= gageKernelSet(ctx, gageKernel22, hvp->k[gageKernel22],
                             hvp->kparm[gageKernel22]);
  if (!E) E |= gageQueryReset(ctx, pvl);
  if (!E) E |= gageQueryAdd(ctx, pvl, hvp->axis[0].measr->query);
  if (!E) E |= gageQueryAdd(ctx, pvl, hvp->axis[1].measr->query);
  if (!E) E |= gageQueryAdd(ctx, pvl, hvp->axis[2].measr->query);
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    sprintf(err, "%s: trouble setting up gage", me);
    biffMove(BANE, err, GAGE); airMopError(mop); return 1;
  }
  pad = ctx->radius;
  
  if (baneFindInclusion(min, max, nin, hvp, ctx)) {
    sprintf(err, "%s: trouble finding inclusion ranges", me);
    biffAdd(BANE, err); airMopError(mop); return 1;
  }
  if (max[0] == min[0]) {
    max[0] += 1;
    if (hvp->verbose)
      sprintf(err, "%s: fixing range 0 [%g,%g] --> [%g,%g]\n",
              me, min[0], min[0], min[0], max[0]);
  }
  if (max[1] == min[1]) {
    max[1] += 1;
    if (hvp->verbose)
      sprintf(err, "%s: fixing range 1 [%g,%g] --> [%g,%g]\n",
              me, min[1], min[1], min[1], max[1]);
  }
  if (max[2] == min[2]) {
    max[2] += 1;
    if (hvp->verbose)
      sprintf(err, "%s: fixing range 2 [%g,%g] --> [%g,%g]\n",
              me, min[2], min[2], min[2], max[2]);
  }
  if (hvp->verbose)
    fprintf(stderr, "%s: inclusion: 0:[%g,%g], 1:[%g,%g], 2:[%g,%g]\n", me,
            min[0], max[0], min[1], max[1], min[2], max[2]);
  
  /* construct the "raw" (un-clipped) histogram volume */
  if (hvp->verbose) {
    fprintf(stderr, "%s: creating raw histogram volume ...       ", me);
    fflush(stderr);
  }
  shx = hvp->axis[0].res;
  shy = hvp->axis[1].res;
  shz = hvp->axis[2].res;
  if (nrrdMaybeAlloc_va(rawhvol=nrrdNew(), nrrdTypeInt, 3,
                        AIR_CAST(size_t, shx),
                        AIR_CAST(size_t, shy),
                        AIR_CAST(size_t, shz))) {
    sprintf(err, "%s: couldn't allocate raw histovol (%dx%dx%d)", me,
            shx, shy, shz);
    biffMove(BANE, err, NRRD); airMopError(mop); return 1;
  }
  airMopAdd(mop, rawhvol, (airMopper)nrrdNuke, airMopAlways);
  rhvdata = (int *)rawhvol->data;
  included = 0;
  
  for (z=pad; z<sz-pad; z++) {
    for (y=pad; y<sy-pad; y++) {
      if (hvp->verbose && !((y-pad+(sy-2*pad)*(z-pad))%200)) {
        fprintf(stderr, "%s", airDoneStr(0, y-pad+(sy-2*pad)*(z-pad),
                                         (sy-2*pad)*(sz-2*pad), prog));
        fflush(stderr);
      }
      for (x=pad; x<sx-pad; x++) {
        baneProbe(val, nin, hvp, ctx, x, y, z);
        if (!( AIR_IN_CL(min[0], val[0], max[0]) &&
               AIR_IN_CL(min[1], val[1], max[1]) &&
               AIR_IN_CL(min[2], val[2], max[2]) )) {
          continue;
        }
        /* else this voxel will contribute to the histovol */
        hx = airIndex(min[0], val[0], max[0], shx);
        hy = airIndex(min[1], val[1], max[1], shy);
        hz = airIndex(min[2], val[2], max[2], shz);
        hidx = hx + shx*(hy + shy*hz);
        if (rhvdata[hidx] < INT_MAX) {
          ++rhvdata[hidx];
        }
        ++included;
      }
    }
  }
  fracIncluded = (float)included/((sz-2*pad)*(sy-2*pad)*(sx-2*pad));
  if (fracIncluded < hvp->incLimit) {
    sprintf(err, "%s: included only %g%% of data, wanted at least %g%%",
            me, 100*fracIncluded, 100*hvp->incLimit);
    biffAdd(BANE, err); airMopError(mop); return 1;
  }
  if (hvp->verbose) {
    fprintf(stderr, "\b\b\b\b\b\b  done\n");
    fprintf(stderr, "%s: included %g%% of original voxels\n", me, 
            fracIncluded*100);
  }
  
  /* determine the clipping value and produce the final histogram volume */
  if (baneClipAnswer(&clipVal, hvp->clip, rawhvol)) {
    sprintf(err, "%s: trouble determining clip value", me);
    biffAdd(BANE, err); airMopError(mop); return 1;
  }
  if (hvp->verbose)
    fprintf(stderr, "%s: will clip at %d\n", me, clipVal);
  if (hvp->verbose) {
    fprintf(stderr, "%s: creating 8-bit histogram volume ...       ", me);
    fflush(stderr);
  }
  if (nrrdMaybeAlloc_va(hvol, nrrdTypeUChar, 3,
                        AIR_CAST(size_t, shx),
                        AIR_CAST(size_t, shy),
                        AIR_CAST(size_t, shz))) {
    sprintf(err, "%s: couldn't alloc finished histovol", me);
    biffMove(BANE, err, NRRD); airMopError(mop); return 1;
  }
  airMopAdd(mop, hvol, (airMopper)nrrdEmpty, airMopOnError);
  hvol->axis[0].min = min[0];
  hvol->axis[1].min = min[1];
  hvol->axis[2].min = min[2];
  hvol->axis[0].max = max[0];
  hvol->axis[1].max = max[1];
  hvol->axis[2].max = max[2];
  hvol->axis[0].label = airStrdup(hvp->axis[0].measr->name);
  hvol->axis[1].label = airStrdup(hvp->axis[1].measr->name);
  hvol->axis[2].label = airStrdup(hvp->axis[2].measr->name);
  hvol->axis[0].center = nrrdCenterCell;
  hvol->axis[1].center = nrrdCenterCell;
  hvol->axis[2].center = nrrdCenterCell;
  nhvdata = (unsigned char *)hvol->data;
  for (hz=0; hz<shz; hz++) {
    for (hy=0; hy<shy; hy++) {
      if (hvp->verbose && !((hy+shy*hz)%200)) {
        fprintf(stderr, "%s", airDoneStr(0, hy+shy*hz, shy*shz, prog));
        fflush(stderr);
      }
      for (hx=0; hx<shx; hx++) {
        hidx = hx + shx*(hy + shy*hz);
        hval = airIndexClamp(0, rhvdata[hidx], clipVal, 256);
        nhvdata[hidx] = hval;
      }
    }
  }
  if (hvp->verbose)
    fprintf(stderr, "\b\b\b\b\b\b  done\n");

  airMopOkay(mop); 
  return 0;
}

Nrrd *
baneGKMSHVol(Nrrd *nin, float gradPerc, float hessPerc) {
  char me[]="baneGKMSHVol", err[BIFF_STRLEN];
  baneHVolParm *hvp;
  Nrrd *hvol;
  
  if (!(hvp = baneHVolParmNew())) {
    sprintf(err, "%s: couldn't get hvol parm struct", me);
    biffAdd(BANE, err); return NULL;
  }
  baneHVolParmGKMSInit(hvp);
  hvp->axis[0].inc->parm[1] = gradPerc;
  hvp->axis[1].inc->parm[1] = hessPerc;
  hvol = nrrdNew();
  if (baneMakeHVol(hvol, nin, hvp)) {
    sprintf(err, "%s: trouble making GKMS histogram volume", me);
    biffAdd(BANE, err); free(hvp); return NULL;
  }
  baneHVolParmNix(hvp);
  return hvol;
}

/*
int
baneApplyMeasr(Nrrd *nout, Nrrd *nin, int measr) {
  char me[]="baneApplyMeasr", err[BIFF_STRLEN];
  int sx, sy, sz, x, y, z, marg;
  baneMeasrType msr;
  nrrdBigInt idx;
  float (*insert)(void *, nrrdBigInt, float);
  
  if (3 != nin->dim) {
    sprintf(err, "%s: need a 3-dimensional nrrd (not %d)", me, nin->dim);
    biffAdd(BANE, err); return 1;
  }
  if (!( AIR_IN_OP(nrrdTypeUnknown, nin->type, nrrdTypeLast) &&
         nin->type != nrrdTypeBlock )) {
    sprintf(err, "%s: must have a scalar type nrrd", me);
    biffAdd(BANE, err); return 1;
  }
  if (!( AIR_EXISTS(nin->axis[0].spacing) && nin->axis[0].spacing > 0 &&
         AIR_EXISTS(nin->axis[1].spacing) && nin->axis[1].spacing > 0 &&
         AIR_EXISTS(nin->axis[2].spacing) && nin->axis[2].spacing > 0 )) {
    sprintf(err, "%s: must have positive spacing for all three axes", me);
    biffAdd(BANE, err); return 1;
  }

  sx = nin->axis[0].size;
  sy = nin->axis[1].size;
  sz = nin->axis[2].size;
  marg = baneMeasrMargin[measr];
  msr = baneMeasr[measr];

  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 3,
                        AIR_CAST(size_t, sx),
                        AIR_CAST(size_t, sy),
                        AIR_CAST(size_t, sz))) {
    sprintf(err, "%s: couldn't alloc output nrrd", me);
    biffMove(BANE, err, NRRD); return 1;
  }
  nout->axis[0].spacing = nin->axis[0].spacing;
  nout->axis[1].spacing = nin->axis[1].spacing;
  nout->axis[2].spacing = nin->axis[2].spacing;
  insert = nrrdFInsert[nrrdTypeFloat];
  for (z=marg; z<sz-marg; z++) {
    for (y=marg; y<sy-marg; y++) {
      for (x=marg; x<sx-marg; x++) {
        idx = x + sx*(y + sy*z);
        insert(nout->data, idx, msr(nin, idx));
      }
    }
  }
  return 0;
}
*/
