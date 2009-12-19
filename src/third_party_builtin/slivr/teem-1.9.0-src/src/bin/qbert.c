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


#include <teem/air.h>
#include <teem/hest.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/bane.h>

#define QBERT "qbert"
#define QBERT_HIST_BINS 1024     /* histogram size for v, g, and h */

int qbertSaveAll = AIR_FALSE;  /* can be used to save output of every stage */

/*
** This padding/resampling is to get axis[i]'s size >= sz[i], which
** is only needed if the input volume is smaller along any of the axes
** than the desired output volume.
*/
int
qbertSizeUp(Nrrd *nout, Nrrd *nin, unsigned int *sz,
            NrrdKernelSpec *uk) {
  char me[]="qbertSizeUp", err[BIFF_STRLEN];
  int i, anyneed, need;
  ptrdiff_t padMin[3], padMax[3];
  NrrdResampleInfo *rsi;
  airArray *mop;

  mop = airMopNew();
  rsi=nrrdResampleInfoNew();
  airMopAdd(mop, rsi, (airMopper)nrrdResampleInfoNix, airMopAlways);
  anyneed = 0;
  if (uk) {
    for (i=0; i<=2; i++) {
      anyneed |= need = sz[i] - nin->axis[i].size;
      fprintf(stderr, "%s: sz[%d] = " _AIR_SIZE_T_CNV " -> need = %d --> ", 
              me, i, nin->axis[i].size, need);
      need = AIR_MAX(0, need);
      fprintf(stderr, "%d --> %s resample\n", need, need ? "WILL" : "won't");
      if (need) {
        rsi->kernel[i] = uk->kernel;
        memcpy(rsi->parm[i], uk->parm, uk->kernel->numParm*sizeof(double));
        if (!AIR_EXISTS(nin->axis[i].min)) {
          nin->axis[i].min = 0.0;
        }
        if (!AIR_EXISTS(nin->axis[i].max)) {
          nin->axis[i].max = nin->axis[i].size-1;
        }
        rsi->min[i] = nin->axis[i].min;
        rsi->max[i] = nin->axis[i].max;
        rsi->samples[i] = sz[i];
        nin->axis[i].center = nrrdCenterNode;
      } else {
        rsi->kernel[i] = NULL;
      }
    }
    if (anyneed) {
      rsi->boundary = nrrdBoundaryBleed;
      rsi->type = nrrdTypeFloat;
      rsi->renormalize = AIR_TRUE;
      rsi->clamp = AIR_TRUE;
      fprintf(stderr, "%s: resampling ... ", me); fflush(stderr);
      if (nrrdSpatialResample(nout, nin, rsi)) {
        sprintf(err, "%s: trouble upsampling", me);
        biffMove(QBERT, err, NRRD); airMopError(mop); return 1;
      }
      fprintf(stderr, "done\n");
    }
  } else {
    for (i=0; i<=2; i++) {
      anyneed |= need = sz[i] - nin->axis[i].size;
      fprintf(stderr, "%s: sz[%d] = " _AIR_SIZE_T_CNV " -> need = %d --> ", 
              me, i, nin->axis[i].size, need);
      need = AIR_MAX(0, need);
      fprintf(stderr, "%d --> ", need);
      padMin[i] = 0 - (int)floor(need/2.0);
      padMax[i] = nin->axis[i].size - 1 + (int)ceil(need/2.0);
      fprintf(stderr, "pad indices: [" _AIR_PTRDIFF_T_CNV ".." _AIR_PTRDIFF_T_CNV "]\n",
              padMin[i], padMax[i]);
    }
    if (anyneed) {
      fprintf(stderr, "%s: padding ... ", me); fflush(stderr);
      if (nrrdPad_va(nout, nin, padMin, padMax, nrrdBoundaryPad, 0.0)) {
        sprintf(err, "%s: trouble padding", me);
        biffMove(QBERT, err, NRRD); airMopError(mop); return 1;
      }
      fprintf(stderr, "done\n");
    }
  }
  if (!anyneed) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: trouble copying", me);
      biffMove(QBERT, err, NRRD); airMopError(mop); return 1;
    }
  }
  if (qbertSaveAll) {
    fprintf(stderr, "%s: saving up.nrrd\n", me);
    nrrdSave("up.nrrd", nout, NULL);
  }
  airMopOkay(mop);
  return 0;
}

/*
** resampling to get axis[i]'s size down to exactly sz[i]
*/
int
qbertSizeDown(Nrrd *nout, Nrrd *nin, unsigned int *sz,
              NrrdKernelSpec *dk) {
  char me[]="qbertSizeDown", err[BIFF_STRLEN];
  NrrdResampleInfo *rsi;
  int need;
  unsigned int i;
  airArray *mop;

  mop = airMopNew();
  rsi = nrrdResampleInfoNew();
  airMopAdd(mop, rsi, (airMopper)nrrdResampleInfoNix, airMopAlways);
  rsi->boundary = nrrdBoundaryBleed;
  rsi->type = nrrdTypeFloat;
  rsi->renormalize = AIR_TRUE;
  need = 0;
  for (i=0; i<=2; i++) {
    if (nin->axis[i].size > sz[i]) {
      need = 1;
      rsi->kernel[i] = dk->kernel;
      memcpy(rsi->parm[i], dk->parm, dk->kernel->numParm*sizeof(double));
      rsi->samples[i] = sz[i];
      if (!AIR_EXISTS(nin->axis[i].min)) {
        nin->axis[i].min = 0.0;
      }
      if (!AIR_EXISTS(nin->axis[i].max)) {
        nin->axis[i].max = nin->axis[i].size-1;
      }
      rsi->min[i] = nin->axis[i].min;
      rsi->max[i] = nin->axis[i].max;
      nin->axis[i].center = nrrdCenterNode;
      fprintf(stderr, "%s: downsampling axis %d from " _AIR_SIZE_T_CNV 
              " to " _AIR_SIZE_T_CNV " samples\n", 
              me, i, nin->axis[i].size, rsi->samples[i]);
    }
    else {
      rsi->kernel[i] = NULL;
    }
  }
  if (need) {
    fprintf(stderr, "%s: resampling ... ", me); fflush(stderr);
    if (nrrdSpatialResample(nout, nin, rsi)) {
      sprintf(err, "%s: trouble resampling", me);
      biffMove(QBERT, err, NRRD); airMopError(mop); return 1;
    }
    fprintf(stderr, "done\n");
  }
  else {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: trouble copying", me);
      biffMove(QBERT, err, NRRD); airMopError(mop); return 1;
    }
  }
  if (qbertSaveAll) {
    fprintf(stderr, "%s: saving down.nrrd\n", me);
    nrrdSave("down.nrrd", nout, NULL);
  }

  airMopOkay(mop); 
  return 0;
}

/*
** probing to getting floating point V, G, and maybe H values
*/
int
qbertProbe(Nrrd *nout, Nrrd *nin,
           NrrdKernelSpec *k00, NrrdKernelSpec *k11, NrrdKernelSpec *k22,
           int doH, unsigned int *sz) {
  char me[]="qbertProbe", err[BIFF_STRLEN], prog[AIR_STRLEN_SMALL];
  gageContext *ctx;
  gagePerVolume *pvl;
  const gage_t *val, *gmag, *scnd;
  float *vghF;
  int E;
  unsigned int i, j, k;
  airArray *mop;
  
  doH = !!doH;
  mop = airMopNew();
  ctx = gageContextNew();
  airMopAdd(mop, ctx, (airMopper)gageContextNix, airMopAlways);
  
  nin->axis[0].center = nrrdCenterNode;
  nin->axis[1].center = nrrdCenterNode;
  nin->axis[2].center = nrrdCenterNode;
  if (!(pvl = gagePerVolumeNew(ctx, nin, gageKindScl))) {
    sprintf(err, "%s: gage trouble", me);
    biffMove(QBERT, err, GAGE); airMopError(mop); return 1;
  }
  gageParmSet(ctx, gageParmVerbose, 0);
  gageParmSet(ctx, gageParmRenormalize, AIR_TRUE);
  gageParmSet(ctx, gageParmCheckIntegrals, AIR_TRUE);
  E = 0;
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  /* about kernel setting for probing: currently, the way that probing is
     done is ONLY on grid locations, and never in between voxels.  That 
     means that the kernels set below are really only used for creating
     discrete convolution masks at unit locations */
  if (!E) E |= gageKernelSet(ctx, gageKernel00, k00->kernel, k00->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel11, k11->kernel, k11->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel22, k22->kernel, k22->parm);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclValue);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclGradMag);
  if (doH) {
    if (!E) E |= gageQueryItemOn(ctx, pvl, gageScl2ndDD);
  }
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    sprintf(err, "%s: gage trouble", me);
    biffMove(QBERT, err, GAGE); airMopError(mop); return 1;
  }
  gageParmSet(ctx, gageParmVerbose, 0);
  val = gageAnswerPointer(ctx, pvl, gageSclValue);
  gmag = gageAnswerPointer(ctx, pvl, gageSclGradMag);
  scnd = gageAnswerPointer(ctx, pvl, gageScl2ndDD);
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 2+doH),
                        AIR_CAST(size_t, sz[0]),
                        AIR_CAST(size_t, sz[1]),
                        AIR_CAST(size_t, sz[2]))) {
    sprintf(err, "%s: couldn't allocate floating point VG%s volume",
            me, doH ? "H" : "");
    biffMove(QBERT, err, NRRD); airMopError(mop); return 1;
  }
  vghF = (float *)nout->data;
  fprintf(stderr, "%s: probing ...       ", me); fflush(stderr);
  for (k=0; k<sz[2]; k++) {
    for (j=0; j<sz[1]; j++) {
      if (!((j + sz[1]*k)%100)) {
        fprintf(stderr, "%s", airDoneStr(0, j + sz[1]*k, sz[1]*sz[2], prog));
        fflush(stderr);
      }
      for (i=0; i<sz[0]; i++) {
        gageProbe(ctx,
                  AIR_CAST(gage_t, i),
                  AIR_CAST(gage_t, j),
                  AIR_CAST(gage_t, k));
        vghF[0] = *val;
        vghF[1] = *gmag;
        if (doH) {
          vghF[2] = *scnd;
        }
        vghF += 2+doH;
      }
    }
  }
  fprintf(stderr, "%s\n", airDoneStr(0, 2, 1, prog));
  if (qbertSaveAll) {
    fprintf(stderr, "%s: saving vghF.nrrd\n", me);
    nrrdSave("vghF.nrrd", nout, NULL);
  }

  airMopOkay(mop); 
  return 0;
}

/*
** make histograms of v,g,h values as first step in determining the
** inclusions for the later quantization
*/
int
qbertMakeVghHists(Nrrd *nvhist, Nrrd *nghist, Nrrd *nhhist,
                  unsigned int *sz, int bins,
                  Nrrd *nvghF, Nrrd *nin) {
  char me[]="qbertMakeVghHists", err[BIFF_STRLEN];
  double minv, maxv, ming, maxg, minh=0, maxh=0;
  float *vghF;
  unsigned int i;
  int nval, doH, E, *vhist, *ghist, *hhist=NULL, vi, gi, hi;

  nval = nvghF->axis[0].size;
  doH = !!(nval == 3);
  vghF = (float *)nvghF->data;
  minv = maxv = vghF[0 + nval*0];
  ming = maxg = vghF[1 + nval*0];
  if (doH) {
    minh = maxh = vghF[2 + nval*0];
  }
  for (i=0; i<sz[0]*sz[1]*sz[2]; i++) {
    minv = AIR_MIN(minv, vghF[0 + nval*i]);
    maxv = AIR_MAX(maxv, vghF[0 + nval*i]);
    ming = AIR_MIN(ming, vghF[1 + nval*i]);
    maxg = AIR_MAX(maxg, vghF[1 + nval*i]);
    if (doH) {
      minh = AIR_MIN(minh, vghF[2 + nval*i]);
      maxh = AIR_MAX(maxh, vghF[2 + nval*i]);
    }
  }
  fprintf(stderr, "%s: values: [%g .. %g] -> ", me, minv, maxv);
  /* just because we're bastards, we're going to enforce minv >= 0 for
     types that started as unsigned integral types.  Downsampling with a
     ringing kernel can have produced negative values, so this change to
     minv can actually restrict the range, in contrast to to the changes
     to ming, minh, and maxh below */
  if (nrrdTypeIsUnsigned[nin->type]) {
    minv = AIR_MAX(minv, 0.0);
  }
  fprintf(stderr, "[%g .. %g]\n", minv, maxv);
  fprintf(stderr, "%s:  grads: [%g .. %g] -> ", me, ming, maxg);
  ming = 0;
  fprintf(stderr, "[%g .. %g]\n", ming, maxg);
  if (doH) {
    fprintf(stderr, "%s: 2ndDDs: [%g .. %g] -> ", me, minh, maxh);
    if (maxh > -minh) 
      minh = -maxh;
    else
      maxh = -minh;
    fprintf(stderr, "[%g .. %g]\n", minh, maxh);
  }
  fprintf(stderr, "%s: using %d-bin histograms\n", me, bins);
  E = 0;
  if (!E) E |= nrrdMaybeAlloc_va(nvhist, nrrdTypeInt, 1,
                                 AIR_CAST(size_t, bins));
  if (!E) E |= nrrdMaybeAlloc_va(nghist, nrrdTypeInt, 1,
                                 AIR_CAST(size_t, bins));
  if (doH) {
    if (!E) E |= nrrdMaybeAlloc_va(nhhist, nrrdTypeInt, 1,
                                   AIR_CAST(size_t, bins));
  }
  if (E) {
    sprintf(err, "%s: couldn't allocate %d %d-bin histograms",
            me, nval, bins);
    biffMove(QBERT, err, NRRD); return 1;
  }
  nvhist->axis[0].min = minv;   nvhist->axis[0].max = maxv;
  nghist->axis[0].min = ming;   nghist->axis[0].max = maxg;
  vhist = (int *)nvhist->data;
  ghist = (int *)nghist->data;
  memset(vhist, 0, bins*sizeof(int));
  memset(ghist, 0, bins*sizeof(int));
  if (doH) {
    nhhist->axis[0].min = minh;   nhhist->axis[0].max = maxh; 
    hhist = (int *)nhhist->data;
    memset(hhist, 0, bins*sizeof(int));
  }
  vghF = (float *)nvghF->data;
  for (i=0; i<sz[0]*sz[1]*sz[2]; i++) {
    vi = airIndexClamp(minv, vghF[0], maxv, bins);
    gi = airIndex(ming, vghF[1], maxg, bins);
    vhist[vi]++;
    ghist[gi]++;
    if (doH) {
      hi = airIndex(minh, vghF[2], maxh, bins);
      hhist[hi]++;
    }
    vghF += nval;
  }
  if (qbertSaveAll) {
    fprintf(stderr, "%s: saving {v,g,h}hist.nrrd\n", me);
    nrrdSave("vhist.nrrd", nvhist, NULL);
    nrrdSave("ghist.nrrd", nghist, NULL);
    if (doH) {
      nrrdSave("hhist.nrrd", nhhist, NULL);
    }
  }

  return 0;
}

/*
** determine inclusion from histograms and create 8-bit VGH volume
*/
int
qbertMakeVgh(Nrrd *nvgh, Nrrd *nvhist, Nrrd *nghist, Nrrd *nhhist,
             unsigned int *sz, float *perc,
             Nrrd *nvghF) {
  char me[]="qbertMakeVgh", err[BIFF_STRLEN], cmt[AIR_STRLEN_SMALL];
  double minv, maxv, ming, maxg, minh=0, maxh=0;
  int lose, *vhist, *ghist, *hhist=NULL, bins, vi, gi, hi, nval, doH;
  unsigned int i;
  unsigned char *vgh;
  float *vghF;

  nval = nvghF->axis[0].size;
  doH = !!(nval == 3);
  minv = nvhist->axis[0].min;   maxv = nvhist->axis[0].max; 
  ming = nghist->axis[0].min;   maxg = nghist->axis[0].max; 
  vhist = (int *)nvhist->data;
  ghist = (int *)nghist->data;
  if (doH) {
    minh = nhhist->axis[0].min;   maxh = nhhist->axis[0].max; 
    hhist = (int *)nhhist->data;
  }

  lose = (int)(perc[0]*sz[0]*sz[1]*sz[2]/100);
  bins = nvhist->axis[0].size;
  i = bins-1;
  while (lose > 0) {
    /* HEY: we're nibbling only from top, even though for signed
       value types, there could be a tail at low negative values (had
       this problem with some CT data) */
    lose -= vhist[i--];
  }
  maxv = AIR_AFFINE(0, i, bins-1, minv, maxv);

  lose = (int)(perc[1]*sz[0]*sz[1]*sz[2]/100);
  bins = nghist->axis[0].size;
  i = bins-1;
  while (lose > 0) {
    /* nibble from top */
    lose -= ghist[i--];
  }
  maxg = AIR_AFFINE(0, i, bins-1, ming, maxg);

  if (doH) {
    lose = (int)(perc[2]*sz[0]*sz[1]*sz[2]/100);
    bins = nhhist->axis[0].size;
    i = 0;
    while (lose > 0) {
      /* nibble from top and bottom at equal rates */
      lose -= hhist[i] + hhist[bins-1-i];
      i++;
    }
    minh = AIR_AFFINE(0, i, bins-1, minh, maxh);
    maxh = -minh;
  }

  fprintf(stderr, "%s: new values (ignored %5d): [%g .. %g]\n",
          me, (int)(perc[0]*sz[0]*sz[1]*sz[2]/100), minv, maxv);
  fprintf(stderr, "%s: new  grads (ignored %5d): [%g .. %g]\n",
          me, (int)(perc[1]*sz[0]*sz[1]*sz[2]/100), ming, maxg);
  if (doH) {
    fprintf(stderr, "%s: new 2ndDDs (ignored %5d): [%g .. %g]\n",
            me, (int)(perc[2]*sz[0]*sz[1]*sz[2]/100), minh, maxh);
    fprintf(stderr, "%s: putting 2ndDD in range 1 to 169 (0.0 -> 85)\n", me);
  }
  
  if (nrrdMaybeAlloc_va(nvgh, nrrdTypeUChar, 4,
                        AIR_CAST(size_t, nval),
                        AIR_CAST(size_t, sz[0]),
                        AIR_CAST(size_t, sz[1]),
                        AIR_CAST(size_t, sz[2]))) {
    sprintf(err, "%s: couldn't allocate 8-bit VG%s volume",
            me, doH ? "H" : "");
    biffMove(QBERT, err, NRRD); return 1;
  }
  vgh = (unsigned char*)nvgh->data;
  vghF = (float*)nvghF->data;
  for (i=0; i<sz[0]*sz[1]*sz[2]; i++) {
    vi = airIndex(minv, vghF[0], maxv, 254);
    vgh[0] = AIR_CLAMP(1, vi+1, 254);
    gi = airIndex(ming, vghF[1], maxg, 254);
    vgh[1] = AIR_CLAMP(1, gi+1, 254);
    if (doH) {
      hi = airIndex(minh, vghF[2], maxh, 168);
      vgh[2] = AIR_CLAMP(1, hi+1, 169);
    }
    vgh += nval;
    vghF += nval;
  }
  if (doH) {
    sprintf(cmt, "exclusions (v g h): %g %g %g", perc[0], perc[1], perc[2]);
  } else {
    sprintf(cmt, "exclusions (v g): %g %g", perc[0], perc[1]);
  }
  nrrdCommentAdd(nvgh, cmt);
  sprintf(cmt, "minv: %g", minv);  nrrdCommentAdd(nvgh, cmt);
  sprintf(cmt, "maxv: %g", maxv);  nrrdCommentAdd(nvgh, cmt);
  sprintf(cmt, "ming: %g", ming);  nrrdCommentAdd(nvgh, cmt);
  sprintf(cmt, "maxg: %g", maxg);  nrrdCommentAdd(nvgh, cmt);
  if (doH) {
    sprintf(cmt, "minh: %g", minh);  nrrdCommentAdd(nvgh, cmt);
    sprintf(cmt, "maxh: %g", maxh);  nrrdCommentAdd(nvgh, cmt);
  }
  nrrdAxisInfoSet_va(nvgh, nrrdAxisInfoCenter, nrrdCenterUnknown,
                     nrrdCenterNode, nrrdCenterNode, nrrdCenterNode);

  return 0;
}
  
int
qbertScat(Nrrd *nvgh, int pos, int size, char *name) {
  char me[]="qbertScat", err[BIFF_STRLEN];
  Nrrd *nin[2], *nv, *nx, *nscA, *nscB;
  airArray *mop;
  size_t bins[2];
  int E, clamp[2];
  NrrdRange *range;

  bins[0] = bins[1] = size;
  clamp[0] = clamp[1] = AIR_FALSE;
  mop = airMopNew();
  airMopAdd(mop, nv=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nx=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nscA=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nscB=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  nin[0] = nv;
  nin[1] = nx;
  E = 0;
  if (!E) E |= nrrdSlice(nv, nvgh, 0, 0);
  if (!E) E |= nrrdSlice(nx, nvgh, 0, pos);
  if (!E) E |= nrrdHistoJoint(nscA, (const Nrrd**)nin, NULL, 2,
                              NULL, bins, nrrdTypeFloat, clamp);
  if (!E) E |= nrrdArithUnaryOp(nscB, nrrdUnaryOpLog1p, nscA);
  if (!E) E |= nrrdHistoEq(nscA, nscB, NULL, 2048, 2, 0.45f);
  if (!E) { 
    range = nrrdRangeNewSet(nscA, nrrdBlind8BitRangeTrue);
    airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
    range->max *= 0.8; 
  }
  if (!E) E |= nrrdQuantize(nscB, nscA, range, 8);
  if (!E) E |= nrrdFlip(nscA, nscB, 1);
  if (!E) E |= nrrdSave(name, nscA, NULL);
  if (E) {
    sprintf(err, "%s: trouble generating/saving scatterplot", me);
    biffMove(QBERT, err, NRRD); airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

char qbertInfo[]="Generates volume datasets friendly to hardware-based "
"volume renderers. "
"The main value of this is a means of combining the functions of "
"resampling a dataset to a particular size, measuring first (and "
"optionally second) derivatives, and doing some semi-intelligent "
"quantization of the derivative values down to 8-bits (if quantization "
"is desired).  The various up and down sampling, as well as the "
"the VGH measurements, can be done with various nrrd kernels.  Also, "
"histogram-equalized "
"VG and VH scatterplots can be generated at a specified resolution.";

int
main(int argc, char *argv[]) {
  char *me, *outS, *errS;
  Nrrd *nin, *npad, *nrsmp, *nvghF, *nvhist, *nghist, *nhhist, *nvgh;
  int E, i, ups, notdoH, useFloat, scat;
  unsigned int sz[3];
  NrrdKernelSpec *k00, *k11, *k22;
  double amin[4], amax[4], spacing[4];
  float vperc, gperc, hperc, perc[3];
  NrrdKernelSpec *dk, *uk;
  hestParm *hparm;
  hestOpt *hopt = NULL;
  airArray *mop;

  mop = airMopNew();
  me = argv[0];
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  
  hparm->elideSingleOtherType = AIR_TRUE;
  hparm->elideSingleNonExistFloatDefault = AIR_TRUE;
  hparm->elideMultipleNonExistFloatDefault = AIR_TRUE;

  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume, in nrrd format",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "vg", NULL, airTypeInt, 0, 0, &notdoH, NULL,
             "Make a 2-channel VG volume, instead of the usual (default) "
             "3-channel VGH volume.");
  hestOptAdd(&hopt, "f", NULL, airTypeInt, 0, 0, &useFloat, NULL,
             "Keep the output volume in floating point, instead of "
             "(by default) quantizing down to 8-bits.  The "
             "\"-vp\", \"-gp\", and \"-hp\" options become moot.");
  hestOptAdd(&hopt, "d", "dimX dimY dimZ", airTypeUInt, 3, 3, sz, NULL,
             "dimensions of output volume");
  hestOptAdd(&hopt, "up", NULL, airTypeInt, 0, 0, &ups, NULL,
             "Instead of just padding axes up to dimensions given "
             "with \"-d\" when original dimensions are smaller, do filtered "
             "upsampling.");
  hestOptAdd(&hopt, "uk", "upsample k", airTypeOther, 1, 1, &uk,
             "cubic:0,0.5",
             "kernel to use when doing the upsampling enabled by \"-up\"",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "dk", "downsample k", airTypeOther, 1, 1, &dk, "tent",
             "kernel to use when DOWNsampling volume to fit with specified "
             "dimensions. NOTE: ringing can be problematic here.",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k00", "kern00", airTypeOther, 1, 1, &k00,
             "tent", "kernel for gageKernel00, used to probe values (\"V\") "
             "in the volume that has been padded/resampled to fit in the "
             "dimensions given by \"-d\"",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k11", "kern11", airTypeOther, 1, 1, &k11,
             "cubicd:1,0", "kernel for gageKernel11, used with k00 to probe "
             "gradient magnitudes (\"G\")",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k22", "kern22", airTypeOther, 1, 1, &k22,
             "cubicdd:1,0", "kernel for gageKernel22, used with k00,k11 to "
             "probe Hessian-based 2nd derivatives (\"H\")",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "vp", "V excl perc", airTypeFloat, 1, 1, &vperc,
             "0.000",
             "Percent of voxels to through away in quantization (if doing "
             "quantization) based their data value being too high or "
             "too low. ");
  hestOptAdd(&hopt, "gp", "G perc", airTypeFloat, 1, 1, &gperc, "0.002",
             "Like \"-vp\", but for gradient magnitudes. ");
  hestOptAdd(&hopt, "hp", "H perc", airTypeFloat, 1, 1, &hperc, "0.004",
             "Like \"-vp\", but for Hessian-based 2nd derivatives. ");
  hestOptAdd(&hopt, "scat", "scat size", airTypeInt, 1, 1, &scat, "0",
             "generate VG (and VH) scatterplots with this resolution. "
             "Size 0 means \"no scatterplots\".  The scatterplots are "
             "histogram equalized, quantized, and saved out as PGM images "
             "named \"vg.pgm\" (and \"vh.pgm\").");
  hestOptAdd(&hopt, "o", "output", airTypeString, 1, 1, &outS, NULL,
             "output volume in nrrd format");
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, qbertInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (3 != nin->dim) {
    fprintf(stderr, "%s: input nrrd is %-dimensional, not 3\n", me, nin->dim);
    airMopError(mop); exit(1);
  }
  if (!AIR_EXISTS(nin->axis[0].spacing)) {
    nrrdAxisInfoSpacingSet(nin, 0);
  }
  if (!AIR_EXISTS(nin->axis[1].spacing)) {
    nrrdAxisInfoSpacingSet(nin, 1);
  }
  if (!AIR_EXISTS(nin->axis[2].spacing)) {
    nrrdAxisInfoSpacingSet(nin, 2);
  }

  npad = nrrdNew();
  airMopAdd(mop, npad, (airMopper)nrrdNuke, airMopAlways);
  if (qbertSizeUp(npad, nin, sz, ups ? uk : NULL)) {
    airMopAdd(mop, errS=biffGetDone(QBERT), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, errS);
    airMopError(mop); exit(1);
  }  

  nrsmp = nrrdNew();
  airMopAdd(mop, nrsmp, (airMopper)nrrdNuke, airMopAlways);
  if (qbertSizeDown(nrsmp, npad, sz, dk)) {
    airMopAdd(mop, errS=biffGetDone(QBERT), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, errS);
    airMopError(mop); exit(1);
  }
  airMopSub(mop, npad, (airMopper)nrrdNuke);
  npad = nrrdNuke(npad);
  
  /* this axis info is being saved so that it can be re-enstated at the end */
  spacing[0] = amin[0] = amax[0] = AIR_NAN;
  nrrdAxisInfoGet_nva(nrsmp, nrrdAxisInfoSpacing, spacing+1);
  nrrdAxisInfoGet_nva(nrsmp, nrrdAxisInfoMin, amin+1);
  nrrdAxisInfoGet_nva(nrsmp, nrrdAxisInfoMax, amax+1);
  /* if we had to downsample, we may have enstated axis mins and maxs where
     they didn't exist before, and those shouldn't be saved in output.  But
     we can't just copy axis mins and maxs from the original input because
     padding could have changed them.  If no axis mins and maxs existed on
     the input nrrd, these will all be nan, so they won't be saved out. 
     NOTE: we're only nixing axis min/max information, not spacing. */
  for (i=0; i<=2; i++) {
    if (!AIR_EXISTS(nin->axis[i].min))
      amin[1+i] = AIR_NAN;
    if (!AIR_EXISTS(nin->axis[i].max))
      amax[1+i] = AIR_NAN;
  }
  
  nvghF = nrrdNew();
  airMopAdd(mop, nvghF, (airMopper)nrrdNuke, airMopAlways);
  if (qbertProbe(nvghF, nrsmp, k00, k11, k22, !notdoH, sz)) {
    airMopAdd(mop, errS=biffGetDone(QBERT), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, errS);
    airMopError(mop); exit(1);
  }
  airMopSub(mop, nrsmp, (airMopper)nrrdNuke);
  nrsmp = nrrdNuke(nrsmp);

  if (useFloat) {
    /* we're done! */
    if (scat && (qbertScat(nvghF, 1, scat, "vg.pgm")
                 || (!notdoH && qbertScat(nvghF, 2, scat, "vh.pgm")))) {
      airMopAdd(mop, errS=biffGetDone(QBERT), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble:\n%s\n", me, errS);
      airMopError(mop); exit(1);
    }
    E = nrrdSave(outS, nvghF, NULL);
  } else {
    nvhist = nrrdNew();
    nghist = nrrdNew();
    nhhist = nrrdNew();
    airMopAdd(mop, nvhist, (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, nghist, (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, nhhist, (airMopper)nrrdNuke, airMopAlways);
    if (qbertMakeVghHists(nvhist, nghist, nhhist,
                          sz, QBERT_HIST_BINS,
                          nvghF, nin)) {
      fprintf(stderr, "%s: trouble:\n%s\n", me, errS = biffGetDone(QBERT));
      free(errS); exit(1);
    }
    
    nvgh = nrrdNew();
    airMopAdd(mop, nvgh, (airMopper)nrrdNuke, airMopAlways);
    ELL_3V_SET(perc, vperc, gperc, hperc);
    if (qbertMakeVgh(nvgh, nvhist, nghist, nhhist, sz, perc, nvghF)) {
      airMopAdd(mop, errS=biffGetDone(QBERT), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble:\n%s\n", me, errS);
      airMopError(mop); exit(1);
    }
    airMopSub(mop, nvghF, (airMopper)nrrdNuke);
    nvghF = nrrdNuke(nvghF);
    
    if (scat && (qbertScat(nvgh, 1, scat, "vg.pgm")
                 || (!notdoH && qbertScat(nvgh, 2, scat, "vh.pgm")))) {
      airMopAdd(mop, errS=biffGetDone(QBERT), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble:\n%s\n", me, errS);
      airMopError(mop); exit(1);
    }

    /* do final decoration of axes */
    nrrdAxisInfoSet_va(nvgh, nrrdAxisInfoLabel,
                       !notdoH ? "vgh" : "vg",
                       "x", "y", "z");
    nrrdAxisInfoSet_nva(nvgh, nrrdAxisInfoMin, amin);
    nrrdAxisInfoSet_nva(nvgh, nrrdAxisInfoMax, amax);
    nrrdAxisInfoSet_nva(nvgh, nrrdAxisInfoSpacing, spacing);
    nrrdContentSet_va(nvgh, "qbert", nin, "");
    
    E = nrrdSave(outS, nvgh, NULL);
  }
  if (E) {
    airMopAdd(mop, errS=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, errS);
    airMopError(mop); exit(1);
  }

  /* HEY: why am I getting memory-in-use with purify? */
  airMopOkay(mop);
  exit(0);
}
