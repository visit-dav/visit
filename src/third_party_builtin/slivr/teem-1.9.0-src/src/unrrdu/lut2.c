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

#define INFO "Map nrrd through a bivariate lookup table"
char *_unrrdu_lut2InfoL =
(INFO
 " (itself represented as a nrrd). The lookup table "
 "can be 2D, in which case the output "
 "has the same dimension as the input, or 3D, in which case "
 "the output has one more dimension than the input, and each "
 "pair of values is mapped to a scanline (along axis 0) from the "
 "lookup table.  In any case, axis 0 of the input must have length two.");

int
unrrdu_lut2Main(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nlut, *nout, *ntmp[2];
  airArray *mop;
  int typeOut, rescale[2], pret, blind8BitRange;
  double min[2], max[2];
  NrrdRange *range[2]={NULL,NULL};
  unsigned int mapAxis, rai;

  hestOptAdd(&opt, "m,map", "lut", airTypeOther, 1, 1, &nlut, NULL,
             "lookup table to map input nrrd through",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "r,rescale", "bool bool", airTypeBool, 2, 2, rescale,
             "false false",
             "rescale one or both of the input values from the "
             "input range to the lut domain.  The lut domain is either "
             "explicitly defined by the axis min,max along axis 0 or 1, "
             "or, it is implicitly defined as zero to the length of that axis "
             "minus one.");
  hestOptAdd(&opt, "min,minimum", "min0 min1", airTypeDouble, 2, 2, min,
             "nan nan",
             "Low ends of input range. Defaults to lowest values "
             "found in input nrrd.  Explicitly setting this is useful "
             "only with rescaling (\"-r\")");
  hestOptAdd(&opt, "max,maximum", "max0 max1", airTypeDouble, 2, 2, max,
             "nan nan",
             "High end of input range. Defaults to highest values "
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

  USAGE(_unrrdu_lut2InfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (!( nin->dim > 1 && 2 == nin->axis[0].size )) {
    fprintf(stderr, "%s: input nrrd dim must be > 1, and axis[0].size "
            "must be 2 (not " _AIR_SIZE_T_CNV ")", me, nin->axis[0].size);
    airMopError(mop);
    return 1;
  }
  mapAxis = nlut->dim - 2;
  if (!(0 == mapAxis || 1 == mapAxis)) {
    fprintf(stderr, "%s: dimension of lut should be 2 or 3, not %d", 
            me, nlut->dim);
    airMopError(mop);
    return 1;
  }

  /* see comment in rmap.c */
  for (rai=0; rai<=1; rai++) {
    if (!( AIR_EXISTS(nlut->axis[mapAxis + rai].min) && 
           AIR_EXISTS(nlut->axis[mapAxis + rai].max) )) {
      rescale[rai] = AIR_TRUE;
    }
    if (rescale[rai]) {
      ntmp[rai] = nrrdNew();
      airMopAdd(mop, ntmp[rai], AIR_CAST(airMopper, nrrdNuke), airMopAlways);
      if (nrrdSlice(ntmp[rai], nin, 0, rai)) {
        airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble slicing input value %u:\n%s",
                me, rai, err);
        airMopError(mop);
        return 1;
      }
      range[rai] = nrrdRangeNew(min[rai], max[rai]);
      airMopAdd(mop, range[rai], (airMopper)nrrdRangeNix, airMopAlways);
      nrrdRangeSafeSet(range[rai], ntmp[rai], blind8BitRange);
    }
  }
  if (nrrdTypeDefault == typeOut) {
    typeOut = nlut->type;
  }
  if (nrrdApply2DLut(nout, nin, 0, range[0], range[1], nlut, typeOut,
                     rescale[0], rescale[1])) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble applying 2-D LUT:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(lut2, INFO);
