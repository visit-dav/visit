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

#define INFO "Replace each scanline along an axis with its histogram"
char *_unrrdu_histaxInfoL = (INFO
                         ". ");

int
unrrdu_histaxMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  int type, bins, pret, blind8BitRange;
  unsigned int axis;
  double min, max;
  airArray *mop;
  NrrdRange *range;

  OPT_ADD_AXIS(axis, "axis to histogram along");
  hestOptAdd(&opt, "b,bin", "bins", airTypeInt, 1, 1, &bins, NULL,
             "# of bins in histogram");
  OPT_ADD_TYPE(type, "output type", "uchar");
  hestOptAdd(&opt, "min,minimum", "value", airTypeDouble, 1, 1, &min, "nan",
             "Value at low end of histogram. Defaults to lowest value "
             "found in input nrrd.");
  hestOptAdd(&opt, "max,maximum", "value", airTypeDouble, 1, 1, &max, "nan",
             "Value at high end of histogram. Defaults to highest value "
             "found in input nrrd.");
  hestOptAdd(&opt, "blind8", "bool", airTypeBool, 1, 1, &blind8BitRange,
             nrrdStateBlind8BitRange ? "true" : "false",
             "Whether to know the range of 8-bit data blindly "
             "(uchar is always [0,255], signed char is [-128,127]).");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_histaxInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  range = nrrdRangeNew(min, max);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  nrrdRangeSafeSet(range, nin, blind8BitRange);
  if (nrrdHistoAxis(nout, nin, range, axis, bins, type)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error doing axis histogramming:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(histax, INFO);
