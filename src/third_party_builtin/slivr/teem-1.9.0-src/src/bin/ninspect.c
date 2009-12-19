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
#include <teem/biff.h>
#include <teem/nrrd.h>
#include <teem/ell.h>  /* for macros only */

#define NINSPECT "ninspect"

int
fixproj(Nrrd *nproj[3], Nrrd *nvol) {
  char me[]="fixproj", err[BIFF_STRLEN];
  airArray *mop;
  Nrrd *ntmp[3], *nt;
  int sz[3], ii, map[3], h[3], E, mi;
  size_t rsz[3];
  double vec[3][3], dot[3], sp[3], parm[NRRD_KERNEL_PARMS_NUM];

  mop = airMopNew();

  if (!( ELL_3V_EXISTS(nvol->axis[0].spaceDirection)
         && ELL_3V_EXISTS(nvol->axis[1].spaceDirection)
         && ELL_3V_EXISTS(nvol->axis[2].spaceDirection) )) {
    sprintf(err, "%s: space directions don't exist for all 3 axes", me);
    biffAdd(NINSPECT, err);
    airMopError(mop); return 1;
  }

  airMopAdd(mop, nt = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntmp[0] = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntmp[1] = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntmp[2] = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);

  /*                 RL  AP  SI */
  ELL_3V_SET(vec[0], 1,  0,  0);
  ELL_3V_SET(vec[1], 0,  1,  0);
  ELL_3V_SET(vec[2], 0,  0,  1);
  for (ii=0; ii<3; ii++) {
    dot[0] = ELL_3V_DOT(vec[ii], nvol->axis[0].spaceDirection);
    dot[1] = ELL_3V_DOT(vec[ii], nvol->axis[1].spaceDirection);
    dot[2] = ELL_3V_DOT(vec[ii], nvol->axis[2].spaceDirection);
    dot[0] = AIR_ABS(dot[0]);
    dot[1] = AIR_ABS(dot[1]);
    dot[2] = AIR_ABS(dot[2]);
    map[ii] = ELL_MAX3_IDX(dot[0], dot[1], dot[2]);
  }
  ELL_3V_SET(h, 1, 0, 0);
  E = 0;
  for (ii=0; ii<3; ii++) {
    if (h[map[ii]] != map[h[ii]]) {
      if (!E) E |= nrrdAxesSwap(ntmp[ii], nproj[map[ii]], 1, 2);
    } else {
      if (!E) E |= nrrdCopy(ntmp[ii], nproj[map[ii]]);
    }
  }
  if (E) {
    sprintf(err, "%s: trouble with nrrd operations", me);
    biffMove(NINSPECT, err, NRRD);
    airMopError(mop); return 1;
  }
  E = 0;
  if (nvol->axis[map[0]].spaceDirection[0] > 0) {
    if (!E) E |= nrrdFlip(nt, ntmp[1], 0+1);
    if (!E) E |= nrrdCopy(ntmp[1], nt);
    if (!E) E |= nrrdFlip(nt, ntmp[2], 0+1);
    if (!E) E |= nrrdCopy(ntmp[2], nt);
  }
  if (nvol->axis[map[1]].spaceDirection[1] > 0) {
    if (!E) E |= nrrdFlip(nt, ntmp[0], 0+1);
    if (!E) E |= nrrdCopy(ntmp[0], nt);
    if (!E) E |= nrrdFlip(nt, ntmp[2], 1+1);
    if (!E) E |= nrrdCopy(ntmp[2], nt);
  }
  if (nvol->axis[map[2]].spaceDirection[2] > 0) {
    if (!E) E |= nrrdFlip(nt, ntmp[0], 1+1);
    if (!E) E |= nrrdCopy(ntmp[0], nt);
    if (!E) E |= nrrdFlip(nt, ntmp[1], 1+1);
    if (!E) E |= nrrdCopy(ntmp[1], nt);
  }
  if (E) {
    sprintf(err, "%s: trouble with nrrd operations", me);
    biffMove(NINSPECT, err, NRRD);
    airMopError(mop); return 1;
  }

  for (ii=0; ii<3; ii++) {
    sz[ii] = nvol->axis[map[ii]].size;
    sp[ii] = ELL_3V_LEN(nvol->axis[map[ii]].spaceDirection);
  }
  mi = ELL_MIN3_IDX(sp[0], sp[1], sp[2]);
  sz[0] = (int)(sz[0]*sp[0]/sp[mi]);
  sz[1] = (int)(sz[1]*sp[1]/sp[mi]);
  sz[2] = (int)(sz[2]*sp[2]/sp[mi]);

  parm[0] = 1;
  ELL_3V_SET(rsz, 3, sz[1], sz[2]);
  nrrdSimpleResample(nproj[0], ntmp[0], nrrdKernelBox, parm, rsz, NULL);
  ELL_3V_SET(rsz, 3, sz[0], sz[2]);
  nrrdSimpleResample(nproj[1], ntmp[1], nrrdKernelBox, parm, rsz, NULL);
  ELL_3V_SET(rsz, 3, sz[0], sz[1]);
  nrrdSimpleResample(nproj[2], ntmp[2], nrrdKernelBox, parm, rsz, NULL);

  airMopOkay(mop);
  return 0;
}

int
ninspect_proj(Nrrd *nout, Nrrd *nin, int axis, int smart, float amount) {
  char me[]="ninspect_proj", err[BIFF_STRLEN];
  airArray *mop;
  Nrrd *ntmpA, *ntmpB, *nrgb[3];
  int bins;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NINSPECT, err);
    return 1;
  }
  if (!( AIR_IN_CL(0, axis, 2) )) {
    sprintf(err, "%s: given axis %d outside valid range [0,1,2]", me, axis);
    biffAdd(NINSPECT, err);
    return 1;
  }

  /* allocate a bunch of nrrds to use as basically temp variables */
  mop = airMopNew();
  airMopAdd(mop, ntmpA = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntmpB = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nrgb[0] = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nrgb[1] = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nrgb[2] = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);

  /* these arguments to nrrdHistoEq will control its behavior */
  bins = 3000;  /* equalization will use a histogram with this many bins */

  /* the following idiom is one way of handling the fact that any 
     non-trivial nrrd call can fail, and if it does, then any subsequent
     nrrd calls should be avoided (to be perfectly safe), so that you can
     get the error message from biff.  Because of the left-to-right ordering
     ensured for logical expressions, this will all be called in sequence
     until one of them has a non-zero return.  If he had exception handling,
     we'd put all the nrrd calls in one "try" block.  */
  if (nrrdProject(ntmpA, nin, axis, nrrdMeasureSum, nrrdTypeDefault)
      || nrrdHistoEq(ntmpB, ntmpA, NULL, bins, smart, amount)
      || nrrdQuantize(nrgb[0], ntmpB, NULL, 8)
      || nrrdProject(ntmpA, nin, axis, nrrdMeasureVariance, nrrdTypeDefault)
      || nrrdHistoEq(ntmpB, ntmpA, NULL, bins, smart, amount)
      || nrrdQuantize(nrgb[1], ntmpB, NULL, 8)
      || nrrdProject(ntmpA, nin, axis, nrrdMeasureMax, nrrdTypeDefault)
      || nrrdQuantize(nrgb[2], ntmpA, NULL, 8)
      || nrrdJoin(nout, (const Nrrd**)nrgb, 3, 0, AIR_TRUE)) {
    sprintf(err, "%s: trouble with nrrd operations", me);
    biffMove(NINSPECT, err, NRRD);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

int
doit(Nrrd *nout, Nrrd *nin, int smart, float amount) {
  char me[]="doit", err[BIFF_STRLEN];
  Nrrd *nproj[3];
  airArray *mop;
  int axis, srl, sap, ssi, E, margin, which;
  size_t min[3];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NINSPECT, err);
    return 1;
  }
  if (!(3 == nin->dim)) {
    sprintf(err, "%s: given nrrd has dimension %d, not 3\n", me, nin->dim);
    biffAdd(NINSPECT, err);
    return 1;
  }
  
  mop = airMopNew();
  airMopAdd(mop, nproj[0]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nproj[1]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nproj[2]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);

  /* how much space to put between and around the projections */
  margin = 6;

  /* do projections for each axis, with some progress indication to sterr */
  for (axis=0; axis<=2; axis++) {
    fprintf(stderr, "%s: doing axis %d projections ... ", me, axis);
    fflush(stderr);
    if (ninspect_proj(nproj[axis], nin, axis, smart, amount)) {
      fprintf(stderr, "ERROR\n");
      sprintf(err, "%s: trouble doing projections for axis %d", me, axis);
      biffAdd(NINSPECT, err);
      airMopError(mop); return 1;
    }
    fprintf(stderr, "done\n");
  }

  if (nrrdSpaceRightAnteriorSuperior == nin->space) {
    if (fixproj(nproj, nin)) {
      fprintf(stderr, "ERROR\n");
      sprintf(err, "%s: trouble orienting projections", me);
      biffAdd(NINSPECT, err);
      airMopError(mop); return 1;
    }
  }
  srl = nproj[1]->axis[0+1].size;
  sap = nproj[0]->axis[0+1].size;
  ssi = nproj[1]->axis[1+1].size;

  /* allocate output as 8-bit color image.  We know output type is
     nrrdTypeUChar because ninspect_proj finishes each projection
     with nrrdQuantize to 8-bits */
  if (nrrdAlloc_va(nout, nrrdTypeUChar, 3,
                   AIR_CAST(size_t, 3),
                   AIR_CAST(size_t, srl + 3*margin + sap),
                   AIR_CAST(size_t, ssi + 3*margin + sap))) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(NINSPECT, err, NRRD);
    airMopError(mop); return 1;
  }

  min[0] = 0; 
  E = 0;
  which = 0;
  if (!E) { min[1] = margin; min[2] = margin; which = 1; }
  if (!E) E |= nrrdInset(nout, nout, nproj[1], min);
  if (!E) { min[1] = margin; min[2] = 2*margin + ssi; which = 2; }
  if (!E) E |= nrrdInset(nout, nout, nproj[2], min);
  if (!E) { min[1] = 2*margin + srl; min[2] = margin; which = 3; }
  if (!E) E |= nrrdInset(nout, nout, nproj[0], min);
  if (E) {
    sprintf(err, "%s: couldn't composite output (which = %d)", 
            me, which);
    biffMove(NINSPECT, err, NRRD);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

void
ninspect_usage() {

  fprintf(stderr, "\nusage: %s <input volume> <output image>\n\n",
          NINSPECT);
  fprintf(stderr, "<input volume>: must be a 3-D array in NRRD or "
          "NRRD-compatible format.\n");
  fprintf(stderr, "<output image>: will be saved out in whatever format " 
          "is implied by the\n");
  fprintf(stderr, "   extension (if recognized), or else in NRRD format\n");
}

char *info = ("Quick way of seeing what's inside a 3D volume.  A color image "
              "of three axis-aligned projections is composed of histogram-"
              "equalized and quantized images of the summation (red), "
              "variance (green), and maximum (blue) intensity projections. "
              "If volume is orientation in RAS space, then a standard "
              "orientation is used for projections and projections are "
              "upsampled (with box kernel) to have isotropic pixels.");

int
main(int argc, char *argv[]) {
  hestOpt *hopt=NULL;
  airArray *mop;
  char *me, *outS, *err;
  Nrrd *nin, *nout;
  NrrdIoState *nio;
  float heqamount;

  me = argv[0];
  mop = airMopNew();
  
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input nrrd to project.  Must be three dimensional.",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "amt", "heq", airTypeFloat, 1, 1, &heqamount, "0.5",
             "how much to apply histogram equalization to projection images");
  hestOptAdd(&hopt, "o", "img out", airTypeString, 1, 1, &outS,
             NULL, "output image to save to.  Will try to use whatever "
             "format is implied by extention, but will fall back to PPM.");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  nio = nrrdIoStateNew();
  airMopAdd(mop, nio, (airMopper)nrrdIoStateNix, airMopAlways);

  nrrdStateDisableContent = AIR_TRUE;

  if (doit(nout, nin, 1, heqamount)) {
    err=biffGetDone(NINSPECT);
    airMopAdd(mop, err, airFree, airMopAlways);
    fprintf(stderr, "%s: trouble creating output:\n%s", me, err);
    airMopError(mop); return 1;
  }

  if (nrrdFormatPNG->nameLooksLike(outS)
      && !nrrdFormatPNG->available()) {
    fprintf(stderr, "(%s: using PPM format for output)\n", me);
    nio->format = nrrdFormatPNM;
  }
  if (nrrdSave(outS, nout, nio)) {
    err=biffGetDone(NRRD);
    airMopAdd(mop, err, airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output image \"%s\":\n%s",
            me, outS, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
