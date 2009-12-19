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

#include "unrrdu.h"
#include "privateUnrrdu.h"

#define INFO "Crop along each axis to make a smaller nrrd"
char *_unrrdu_cropInfoL = INFO;

int
unrrdu_cropMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  unsigned int ai;
  int *minOff, minLen, *maxOff, maxLen, pret;
  size_t min[NRRD_DIM_MAX], max[NRRD_DIM_MAX];
  airArray *mop;

  OPT_ADD_BOUND("min,minimum", minOff,
                "low corner of bounding box.\n "
                "\b\bo <int> gives 0-based index\n "
                "\b\bo M, M+<int>, M-<int> give index relative "
                "to the last sample on the axis (M == #samples-1).",
                minLen);
  OPT_ADD_BOUND("max,maximum", maxOff, "high corner of bounding box.  Besides "
                "the specification styles described above, there's also:\n "
                "\b\bo m+<int> give index relative to minimum.",
                maxLen);
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_cropInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  if (!( minLen == (int)nin->dim && maxLen == (int)nin->dim )) {
    fprintf(stderr,
            "%s: # min coords (%d) or max coords (%d) != nrrd dim (%d)\n",
            me, minLen, maxLen, nin->dim);
    airMopError(mop);
    return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    if (-1 == minOff[0 + 2*ai]) {
      fprintf(stderr, "%s: can't use m+<int> specification for axis %d min\n",
              me, ai);
      airMopError(mop);
      return 1;
    }
  }
  for (ai=0; ai<nin->dim; ai++) {
    min[ai] = minOff[0 + 2*ai]*(nin->axis[ai].size-1) + minOff[1 + 2*ai];
    if (-1 == maxOff[0 + 2*ai]) {
      max[ai] = min[ai] + maxOff[1 + 2*ai];
    } else {
      max[ai] = maxOff[0 + 2*ai]*(nin->axis[ai].size-1) + maxOff[1 + 2*ai];
    }
    /*
    fprintf(stderr, "%s: ai %2d: min = %4d, max = %4d\n",
            me, ai, min[ai], max[ai]);
    */
  }

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdCrop(nout, nin, min, max)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error cropping nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(crop, INFO);
