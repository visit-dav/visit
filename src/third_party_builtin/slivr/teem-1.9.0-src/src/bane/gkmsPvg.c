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

float _baneGkmsDonData[] = {0 /* AIR_NEG_INF */ , 0, 0, 0,
                            0 /* AIR_NAN */ , 0, 0, 0,
                            0 /* AIR_POS_INF */ , 0, 0, 0,
                            -9.5, 0, 107, 255,   /* (3) start: blue */
                            -8.5, 51, 104, 255,
                            -7.5, 103, 117, 255,
                            -6.5, 123, 124, 255,
                            -5.5, 141, 130, 255,
                            -4.5, 156, 132, 255,
                            -3.5, 166, 131, 245,
                            -2.5, 174, 131, 231,
                            -1.5, 181, 130, 216,
                            -0.5, 255, 255, 255, /* (12) middle */
                            0.5,  255, 255, 255, /* (13) middle */
                            1.5,  192, 129, 186,
                            2.5,  197, 128, 172,
                            3.5,  200, 128, 158,
                            4.5,  204, 127, 142,
                            5.5,  210, 126, 113,
                            6.5,  212, 126, 98,
                            7.5,  213, 126, 84,
                            8.5,  216, 126, 49,
                            9.5, 220, 133, 0};  /* (22) end: orange */
#define INVERT(d,i) \
    (d)[1 + (i)*4] = 255 - (d)[1 + (i)*4]; \
    (d)[2 + (i)*4] = 255 - (d)[2 + (i)*4]; \
    (d)[3 + (i)*4] = 255 - (d)[3 + (i)*4]
#define PVG_HISTEQ_BINS 2048

Nrrd *
_baneGkmsDonNew(int invert) {
  char me[]="_baneGkmsDonNew", err[BIFF_STRLEN];
  Nrrd *ret;
  float *data;

  if (nrrdMaybeAlloc_va(ret=nrrdNew(), nrrdTypeFloat, 2,
                        AIR_CAST(size_t, 4), AIR_CAST(size_t, 23))) {
    sprintf("%s: can't create output", me);
    biffAdd(BANE, err); return NULL;
  }
  data = (float *)ret->data;
  memcpy(data, _baneGkmsDonData, 4*23*sizeof(float));
  data[0 + 4*0] = AIR_NEG_INF;
  data[0 + 4*1] = AIR_NAN;
  data[0 + 4*2] = AIR_POS_INF;
  if (invert) {
    INVERT(data, 0);
    INVERT(data, 1);
    INVERT(data, 2);
    INVERT(data, 12);
    INVERT(data, 13);
  }
  return ret;
}

#define PVG_INFO "Create color-mapped pictures of p(v,g)"
char *_baneGkms_pvgInfoL =
  (PVG_INFO
   ".  This produces a qualitative visualization of the boundary information "
   "that was captured in the histogram volume.  The quantity shown is called "
   "the \"position function\" in GK's published work, but a better term "
   "would be \"distance map\", as a function of value (v) and gradient "
   "magnitude (g).");
int
baneGkms_pvgMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *outS, *perr, err[BIFF_STRLEN], *mapS;
  Nrrd *ninfo, *nposA, *nposB, *ndon, *npvg;
  NrrdIoState *nio;
  airArray *mop;
  int i, pret, invert, sv, sg, smlI;
  float *pos, p, min, max, sml, newsml, newmin, newmax;
  NrrdRange *range;

  hestOptAdd(&opt, "inv", NULL, airTypeInt, 0, 0, &invert, NULL,
             "Draw on white background, instead of black");
  hestOptAdd(&opt, "m", "mapOut", airTypeString, 1, 1, &mapS, "",
             "save out the colormap used here, so that it can be applied "
             "to other nrrds with \"unu imap -r\"");
  hestOptAdd(&opt, "i", "infoIn", airTypeOther, 1, 1, &ninfo, NULL,
             "input info file (from \"gkms info\")",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "o", "imageOut", airTypeString, 1, 1, &outS, NULL,
             "output image, in PPM format");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_baneGkms_pvgInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);
  airMopAdd(mop, ndon=_baneGkmsDonNew(invert),
            (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nposA=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nposB=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, npvg=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nio=nrrdIoStateNew(), (airMopper)nrrdIoStateNix,
            airMopAlways);

  if (airStrlen(mapS)) {
    if (nrrdSave(mapS, ndon, NULL)) {
      sprintf(err, "%s: trouble saving colormap", me);
      biffMove(BANE, err, NRRD); airMopError(mop); return 1;
    }
  }

  /* we use sigma = 1.0: different sigmas will scale the position,
     which will not change the coloring
     gthresh = 0.0: we want to see everything, and Simian has taught
     us that there can be boundaries at extremely low gradients */
  if (banePosCalc(nposA, 1.0, 0.0, ninfo)) {
    sprintf(err, "%s: trouble calculating position", me);
    biffAdd(BANE, err); airMopError(mop); return 1;
  }
  sv = nposA->axis[0].size;
  sg = nposA->axis[1].size;
  pos = (float *)nposA->data;

  /* find min, max, sml, smlI: histo-eq will warp values around such
     that min-->min and max-->max, but 0-->??.  So, find smallest
     magnitide position (sml) as a stand-in for 0.0 and its index
     (smlI) */
  sml = 0;
  smlI = 0;
  min = max = AIR_NAN;
  for (i=0; i<sv*sg; i++) {
    p = pos[i];
    if (!AIR_EXISTS(p))
      continue;
    if (!AIR_EXISTS(min)) {
      min = max = p;
      sml = AIR_ABS(p);
      smlI = i;
      continue;
    }
    min = AIR_MIN(p, min);
    max = AIR_MAX(p, max);
    if (AIR_ABS(p) < sml) {
      sml = AIR_ABS(p);
      smlI = i;
    }
  }
  if (!AIR_EXISTS(min)) {
    sprintf(err, "%s: didn't see any real data in position array", me);
    biffAdd(BANE, err); airMopError(mop); return 1;
  }
  if (nrrdHistoEq(nposB, nposA, NULL, PVG_HISTEQ_BINS, 3, 1.0)) {
    sprintf(err, "%s: trouble doing histo-eq on p(v,g)", me);
    biffMove(BANE, err, NRRD); airMopError(mop); return 1;
  }

  /* warp position values that pos[smlI] gets mapped back to zero,
     and so that [newmin,newmax] is centered on zero */
  pos = (float *)nposB->data;
  newsml = pos[smlI];
  if (min < -max) {
    newmin = min;
    newmax = -min;
  } else {
    newmin = -max;
    newmax = max;
  }
  for (i=0; i<sv*sg; i++) {
    if (!AIR_EXISTS(pos[i])) {
      continue;
    }
    if (pos[i] < newsml) {
      pos[i] = AIR_CAST(float, AIR_AFFINE(min, pos[i], newsml, newmin, 0.0));
    } else {
      pos[i] = AIR_CAST(float, AIR_AFFINE(newsml, pos[i], max, 0.0, newmax));
    }
  }
  range = nrrdRangeNew(newmin, newmax);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);

  if (nrrdFlip(nposA, nposB, 1) ||
      nrrdApply1DIrregMap(npvg, nposA, range, ndon,
                          NULL, nrrdTypeUChar, AIR_TRUE)) {
    sprintf(err, "%s: trouble applying colormap", me);
    biffMove(BANE, err, NRRD); airMopError(mop); return 1;
  }

  nio->format = nrrdFormatPNM;
  if (nrrdSave(outS, npvg, nio)) {
    sprintf(err, "%s: trouble saving pvg image", me);
    biffMove(BANE, err, NRRD); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}
BANE_GKMS_CMD(pvg, PVG_INFO);

