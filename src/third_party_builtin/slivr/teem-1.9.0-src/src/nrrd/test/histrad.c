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

#include "../nrrd.h"

char *histradInfo = ("like unu histax, but for circles");

int
main(int argc, char *argv[]) {
  char *me, *outS;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  char *err;
  Nrrd *nin, *nhist;
  double vmin, vmax, rmax, val, cent[2], rad;
  int bins[2], sx, sy, xi, yi, ridx, hidx, rbins, hbins;
  NrrdRange *range;
  double (*lup)(const void *v, size_t I), *hist;
  
  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input image", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "b", "rbins hbins", airTypeInt, 2, 2, bins, NULL,
             "# of histogram bins: radial and value");
  hestOptAdd(&hopt, "min", "value", airTypeDouble, 1, 1, &vmin, "nan",
             "Value at low end of histogram. Defaults to lowest value "
             "found in input nrrd.");
  hestOptAdd(&hopt, "max", "value", airTypeDouble, 1, 1, &vmax, "nan",
             "Value at high end of histogram. Defaults to highest value "
             "found in input nrrd.");
  hestOptAdd(&hopt, "rmax", "max radius", airTypeDouble, 1, 1, &rmax, "nan",
             "largest radius to include in histogram");
  hestOptAdd(&hopt, "c", "center x, y", airTypeDouble, 2, 2, cent, NULL,
             "The center point around which to build radial histogram");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
             "file to write histogram to");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, histradInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (2 != nin->dim) {
    fprintf(stderr, "%s: need 2-D input (not %d-D)\n", me, nin->dim);
    airMopError(mop); return 1;
  }

  rbins = bins[0];
  hbins = bins[1];
  nhist = nrrdNew();
  airMopAdd(mop, nhist, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdMaybeAlloc_va(nhist, nrrdTypeDouble, 2,
                        AIR_CAST(size_t, rbins),
                        AIR_CAST(size_t, hbins))) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate histogram:\n%s", me, err);
    airMopError(mop); return 1;
  }

  if (!( AIR_EXISTS(vmin) && AIR_EXISTS(vmax) )) {
    range = nrrdRangeNewSet(nin, nrrdStateBlind8BitRange);
    airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
    vmin = AIR_EXISTS(vmin) ? vmin : range->min;
    vmax = AIR_EXISTS(vmax) ? vmax : range->max;
  }

#define DIST(x0, y0, x1, y1) (sqrt((x0-x1)*(x0-x1) + (y0-y1)*(y0-y1)))

  sx = nin->axis[0].size;
  sy = nin->axis[1].size;
  if (!AIR_EXISTS(rmax)) {
    rmax = 0;
    rmax = AIR_MAX(rmax, DIST(cent[0], cent[1], 0, 0));
    rmax = AIR_MAX(rmax, DIST(cent[0], cent[1], sx-1, 0));
    rmax = AIR_MAX(rmax, DIST(cent[0], cent[1], 0, sy-1));
    rmax = AIR_MAX(rmax, DIST(cent[0], cent[1], sx-1, sy-1));
  }
  
  lup = nrrdDLookup[nin->type];
  hist = (double*)(nhist->data);
  for (xi=0; xi<sx; xi++) {
    for (yi=0; yi<sy; yi++) {
      rad = DIST(cent[0], cent[1], xi, yi);
      if (!AIR_IN_OP(0, rad, rmax)) {
        continue;
      }
      val = lup(nin->data, xi + sx*yi);
      if (!AIR_IN_OP(vmin, val, vmax)) {
        continue;
      }
      ridx = airIndex(0, rad, rmax, rbins);
      hidx = airIndex(vmin, val, vmax, hbins);
      hist[ridx + rbins*hidx] += 1;
    }
  }
  
  nhist->axis[0].min = 0;
  nhist->axis[0].max = rmax;
  nhist->axis[1].min = vmin;
  nhist->axis[1].max = vmax;
  if (nrrdSave(outS, nhist, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't save output:\n%s", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  exit(0);
}
