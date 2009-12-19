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

#define INFO "Brighten or darken values with a gamma"
char *_unrrdu_gammaInfoL =
(INFO
 ". Just as in xv, the gamma value here is actually the "
 "reciprocal of the exponent actually used to transform "
 "the values.");

int
unrrdu_gammaMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  double min, max, gamma;
  airArray *mop;
  int pret, blind8BitRange;
  NrrdRange *range;

  hestOptAdd(&opt, "g,gamma", "gamma", airTypeDouble, 1, 1, &gamma, NULL,
             "gamma > 1.0 brightens; gamma < 1.0 darkens. "
             "Negative gammas invert values (like in xv). ");
  hestOptAdd(&opt, "min,minimum", "value", airTypeDouble, 1, 1, &min, "nan",
             "Value to implicitly map to 0.0 prior to calling pow(). "
             "Defaults to lowest value found in input nrrd.");
  hestOptAdd(&opt, "max,maximum", "value", airTypeDouble, 1, 1, &max, "nan",
             "Value to implicitly map to 1.0 prior to calling pow(). "
             "Defaults to highest value found in input nrrd.");
  hestOptAdd(&opt, "blind8", "bool", airTypeBool, 1, 1, &blind8BitRange,
             nrrdStateBlind8BitRange ? "true" : "false",
             "Whether to know the range of 8-bit data blindly "
             "(uchar is always [0,255], signed char is [-128,127]).");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_gammaInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  range = nrrdRangeNew(min, max);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  nrrdRangeSafeSet(range, nin, blind8BitRange);
  if (nrrdArithGamma(nout, nin, range, gamma)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error doing gamma:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(gamma, INFO);
