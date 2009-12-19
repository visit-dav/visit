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

#define INFO "Create 1-D histogram of values in a nrrd"
char *_unrrdu_histoInfoL = INFO;

int
unrrdu_histoMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout, *nwght;
  int type, pret, blind8BitRange;
  unsigned int bins;
  double min, max;
  NrrdRange *range;
  airArray *mop;

  hestOptAdd(&opt, "b,bins", "num", airTypeUInt, 1, 1, &bins, NULL,
             "# of bins in histogram");
  hestOptAdd(&opt, "w,weight", "nweight", airTypeOther, 1, 1, &nwght, "",
             "how to weigh contributions to histogram.  By default "
             "(not using this option), the increment is one bin count per "
             "sample, but by giving a nrrd, the value in the nrrd at the "
             "corresponding location will be the bin count increment ",
             NULL, NULL, nrrdHestNrrd);
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
  OPT_ADD_TYPE(type, "type to use for bins in output histogram", "uint");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_histoInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  
  /* If the input nrrd never specified min and max, then they'll be
     AIR_NAN, and nrrdRangeSafeSet will find them, and will do so
     according to blind8BitRange */
  range = nrrdRangeNew(min, max);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  nrrdRangeSafeSet(range, nin, blind8BitRange);
  if (nrrdHisto(nout, nin, range, nwght, bins, type)) {
    err = biffGet(NRRD);
    fprintf(stderr, "%s: error calculating histogram:\n%s", me, err);
    free(err);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(histo, INFO);
