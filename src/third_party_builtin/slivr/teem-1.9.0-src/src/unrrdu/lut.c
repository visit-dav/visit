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

#define INFO "Map nrrd through one univariate lookup table"
char *_unrrdu_lutInfoL =
(INFO
 " (itself represented as a nrrd). The lookup table "
 "can be 1D, in which case the output "
 "has the same dimension as the input, or 2D, in which case "
 "the output has one more dimension than the input, and each "
 "value is mapped to a scanline (along axis 0) from the "
 "lookup table.");

int
unrrdu_lutMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nlut, *nout;
  airArray *mop;
  int typeOut, rescale, pret, blind8BitRange;
  double min, max;
  NrrdRange *range=NULL;

  hestOptAdd(&opt, "m,map", "lut", airTypeOther, 1, 1, &nlut, NULL,
             "lookup table to map input nrrd through",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "r,rescale", NULL, airTypeInt, 0, 0, &rescale, NULL,
             "rescale the input values from the input range to the "
             "lut domain.  The lut domain is either explicitly "
             "defined by the axis min,max along axis 0 or 1, or, it "
             "is implicitly defined as zero to the length of that axis "
             "minus one.");
  hestOptAdd(&opt, "min,minimum", "value", airTypeDouble, 1, 1, &min, "nan",
             "Low end of input range. Defaults to lowest value "
             "found in input nrrd.  Explicitly setting this is useful "
             "only with rescaling (\"-r\")");
  hestOptAdd(&opt, "max,maximum", "value", airTypeDouble, 1, 1, &max, "nan",
             "High end of input range. Defaults to highest value "
             "found in input nrrd.  Explicitly setting this is useful "
             "only with rescaling (\"-r\")");
  hestOptAdd(&opt, "blind8", "bool", airTypeBool, 1, 1, &blind8BitRange,
             nrrdStateBlind8BitRange ? "true" : "false",
             "Whether to know the range of 8-bit data blindly "
             "(uchar is always [0,255], signed char is [-128,127]). "
             "Explicitly setting this is useful only with rescaling (\"-r\")");
  hestOptAdd(&opt, "t,type", "type", airTypeOther, 1, 1, &typeOut, "default",
             "specify the type (\"int\", \"float\", etc.) of the "
             "output nrrd. "
             "By default (not using this option), the output type "
             "is the lut's type.",
             NULL, NULL, &unrrduHestMaybeTypeCB);
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_lutInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  /* see comment rmap.c */
  if (!( AIR_EXISTS(nlut->axis[nlut->dim - 1].min) && 
         AIR_EXISTS(nlut->axis[nlut->dim - 1].max) )) {
    rescale = AIR_TRUE;
  }
  if (rescale) {
    range = nrrdRangeNew(min, max);
    airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
    nrrdRangeSafeSet(range, nin, blind8BitRange);
  }

  if (nrrdTypeDefault == typeOut) {
    typeOut = nlut->type;
  }
  if (nrrdApply1DLut(nout, nin, range, nlut, typeOut, rescale)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble applying LUT:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(lut, INFO);
