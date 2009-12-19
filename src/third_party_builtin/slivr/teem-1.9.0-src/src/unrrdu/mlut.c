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

#define INFO "Map nrrd through whole nrrd of univariate lookup tables"
char *_unrrdu_mlutInfoL =
(INFO
 ", with one lookup table per element of input nrrd.  The multiple "
 "tables are stored in a nrrd with a dimension which is either 1 or 2 "
 "more than the dimension of the input nrrd, resulting in an output "
 "which has either the same or one more dimension than the input, "
 "resptectively. ");

int
unrrdu_mlutMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, **_nmlut, *nmlut, *nout;
  airArray *mop;
  int typeOut, rescale, pret, blind8BitRange;
  unsigned int _nmlutLen, mapAxis;
  double min, max;
  NrrdRange *range=NULL;

  hestOptAdd(&opt, "m,map", "mlut", airTypeOther, 1, -1, &_nmlut, NULL,
             "one nrrd of lookup tables to map input nrrd through, or, "
             "list of nrrds which contain the individual entries of "
             "the lookup table at each voxel, which will be joined together.",
             &_nmlutLen, NULL, nrrdHestNrrd);
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

  USAGE(_unrrdu_mlutInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  /* by the end of this block we need to have nmlut and mapAxis */
  if (1 == _nmlutLen) {
    /* we got the mlut as a single nrrd */
    nmlut = _nmlut[0];
    mapAxis = nmlut->dim - nin->dim - 1;
    /* its not our job to do real error checking ... */
    mapAxis = AIR_MIN(mapAxis, nmlut->dim - 1);
  } else {
    /* we have to join together multiple nrrds to get the mlut */
    nmlut = nrrdNew();
    airMopAdd(mop, nmlut, (airMopper)nrrdNuke, airMopAlways);
    /* assume that mlut component nrrds are all compatible sizes,
       nrrdJoin will fail if they aren't */
    mapAxis = _nmlut[0]->dim - nin->dim;
    if (nrrdJoin(nmlut, (const Nrrd**)_nmlut, _nmlutLen, mapAxis, AIR_TRUE)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble joining mlut:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    /* set these if they were given, they'll be NaN otherwise */
    nmlut->axis[mapAxis].min = min;
    nmlut->axis[mapAxis].max = max;
  }

  if (!( AIR_EXISTS(nmlut->axis[mapAxis].min) && 
         AIR_EXISTS(nmlut->axis[mapAxis].max) )) {
    rescale = AIR_TRUE;
  }
  if (rescale) {
    range = nrrdRangeNew(min, max);
    airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
    nrrdRangeSafeSet(range, nin, blind8BitRange);
  }

  if (nrrdTypeDefault == typeOut) {
    typeOut = nmlut->type;
  }
  if (nrrdApplyMulti1DLut(nout, nin, range, nmlut, typeOut, rescale)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble applying multi-LUT:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(mlut, INFO);
