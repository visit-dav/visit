/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
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

#include "../gage.h"

char *ctfixInfo = ("removes circular streaks from CT datasets, "
                   "assuming you know where they are.  nrrd/test/histrad "
                   "is useful for finding the radius of a streak "
                   "in a given slice.");

int
main(int argc, char *argv[]) {
  char *me, *outS;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  char *err, done[13];
  Nrrd *nin, *_nsinfo, *nsinfo, *nout;
  NrrdKernelSpec *kern;
  int E;
  unsigned int sx, sy, sz, xi, yi, zi, si;
  gageContext *ctx;
  gagePerVolume *pvl;
  const gage_t *answer;
  double cent[2], r, srad, *sinfo, swidth, astart, angle, astop, corerad;
  double (*ins)(void *v, size_t I, double d);

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "si", "streak info", airTypeOther, 1, 1, &_nsinfo, NULL,
             "list of streaks as 2D nrrd; per line: \n "
             "<slice> <radius> <width> <start> <stop> \n "
             "where start and stop are angles in [0,4] "
             "(instead of [-pi,pi]) delimiting arc");
  hestOptAdd(&hopt, "k", "kernel", airTypeOther, 1, 1, &kern,
             "tent", "kernel to use for median weighting",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "c", "center x, y", airTypeDouble, 2, 2, cent, NULL,
             "The center point of tomography, around which streaks are "
             "circular arcs or fragments thereof");
  hestOptAdd(&hopt, "r", "core radius", airTypeDouble, 1, 1, &corerad, "0",
             "in addition to the per-streak filtering, giving a non-zero "
             "radius here turns on filtering of *every* slice within "
             "this radius, to handle glitches along central tomography "
             "radius");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
             "fixed volume output");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, ctfixInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!( 3 == nin->dim )) {
    fprintf(stderr, "%s: need 3-D input (not %d-D)\n", me, nin->dim);
    airMopError(mop); return 1;
  }
  if (!( 2 == _nsinfo->dim && 5 == _nsinfo->axis[0].size )) {
    fprintf(stderr, "%s: need a 2-D 5-by-N list of streak locations\n", me);
    airMopError(mop); return 1;
  }
  nsinfo = nrrdNew();
  airMopAdd(mop, nsinfo, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(nsinfo, _nsinfo, nrrdTypeDouble)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't convert streak info to doubles:\n%s",
            me, err);
    airMopError(mop); return 1;
  }
  sinfo = (double*)(nsinfo->data);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdCopy(nout, nin)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate output:\n%s", me, err);
    airMopError(mop); return 1;
  }

#define DIST(x0, y0, x1, y1) (sqrt((x0-x1)*(x0-x1) + (y0-y1)*(y0-y1)))

  sx = nin->axis[0].size;
  sy = nin->axis[1].size;
  sz = nin->axis[2].size;

  ctx = gageContextNew();
  airMopAdd(mop, ctx, (airMopper)gageContextNix, airMopAlways);
  gageParmSet(ctx, gageParmRenormalize, AIR_TRUE);
  gageParmSet(ctx, gageParmCheckIntegrals, AIR_TRUE);
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(ctx, nin, gageKindScl));
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  if (!E) E |= gageKernelSet(ctx, gageKernel00, kern->kernel, kern->parm);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclMedian);
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  answer = gageAnswerPointer(ctx, pvl, gageSclMedian);

  ins = nrrdDInsert[nout->type];
  fprintf(stderr, "%s: processing streaks ... \n", me);
  for (si=0; si<nsinfo->axis[1].size; si++) {
    fprintf(stderr, "%s: streak %d of %d       ",
            me, si+1, AIR_CAST(int, nsinfo->axis[1].size));
    fflush(stderr);
    zi = (int)(sinfo[0 + 5*si]);
    if (!( zi < sz )) {
      fprintf(stderr, "%s: streak[%d] zi=%d outside valid range [0,%d]\n",
              me, si, zi, sz-1);
      airMopError(mop); return 1;
    }
    srad = sinfo[1 + 5*si];
    swidth = sinfo[2 + 5*si];
    astart = sinfo[3 + 5*si];
    astop = sinfo[4 + 5*si];
    for (yi=0; yi<sy; yi++) {
      if (!(yi%3)) {
        fprintf(stderr, "%s", airDoneStr(0, yi, sy-1, done));
        fflush(stderr);
      }
      for (xi=0; xi<sx; xi++) {
        r = DIST(xi, yi, cent[0], cent[1]);
        if (!( AIR_ABS(r-srad) < swidth/2 )) {
          continue;
        }
        angle = atan2(yi-cent[1], xi-cent[0]);
        angle = AIR_AFFINE(-AIR_PI, angle, AIR_PI, 0, 4);
        if (!( (astart < astop && AIR_IN_CL(astart, angle, astop))
               || (astart > astop && (AIR_IN_CL(astart, angle, 4)
                                      || AIR_IN_CL(0, angle, astop))) )) {
          continue;
        }
        gageProbe(ctx, xi, yi, zi);
        ins(nout->data, xi + sx*(yi + sy*zi), *answer);
      }
    }
    fprintf(stderr, "\n");
  }

  if (corerad > 0) {
    fprintf(stderr, "%s: processing core ...       ", me);
    for (zi=0; zi<sz; zi++) {
      fprintf(stderr, "%s", airDoneStr(0, zi, sz-1, done));
      for (yi=0; yi<sy; yi++) {
        for (xi=0; xi<sx; xi++) {
          r = DIST(xi, yi, cent[0], cent[1]);
          if (!( r < corerad )) {
            continue;
          }
          gageProbe(ctx, xi, yi, zi);
          ins(nout->data, xi + sx*(yi + sy*zi), *answer);
        }
      }
    }
    fprintf(stderr, "\n");
  }

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't save output:\n%s", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  exit(0);
}


