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

#define INFO "Map nrrd through a whole nrrd of regular univariate maps"
char *_unrrdu_mrmapInfoL =
(INFO
 ", one map per sample in input. The \"mmap\" nrrd has the same dimensional "
 "constraints as the \"mlut\" nrrd for \"unu mlut\".  This functionality "
 "is a generalization of \"unu 3op lerp\": it allows you to lerp through "
 "multiple nrrds, instead of just two.");

int
unrrdu_mrmapMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, **_nmmap, *nmmap, *nout;
  airArray *mop;
  NrrdRange *range=NULL;
  unsigned int mapAxis;
  int typeOut, rescale, pret, blind8BitRange;
  unsigned int _nmmapLen;
  double min, max;

  hestOptAdd(&opt, "m,map", "mmap", airTypeOther, 1, -1, &_nmmap, NULL,
             "one nrrd of regular maps to map input nrrd through, or, "
             "list of nrrds which contain the individual entries of the map "
             "at each voxel, which will be joined together.",
             &_nmmapLen, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "r,rescale", NULL, airTypeInt, 0, 0, &rescale, NULL,
             "rescale the input values from the input range to the "
             "map domain.  The map domain is either explicitly "
             "defined by the axis min,max along axis 0 or 1, or, it "
             "is implicitly defined as zero to one minus the length of "
             "that axis.");
  hestOptAdd(&opt, "min,minimum", "value", airTypeDouble, 1, 1, &min, "nan",
             "Low end of input range. Defaults to lowest value "
             "found in input nrrd.  Explicitly setting this is useful "
             "only with rescaling (\"-r\") or if the map domain is only "
             "implicitly defined");
  hestOptAdd(&opt, "max,maximum", "value", airTypeDouble, 1, 1, &max, "nan",
             "High end of input range. Defaults to highest value "
             "found in input nrrd.  Explicitly setting this is useful "
             "only with rescaling (\"-r\") or if the map domain is only "
             "implicitly defined");
  hestOptAdd(&opt, "blind8", "bool", airTypeBool, 1, 1, &blind8BitRange,
             nrrdStateBlind8BitRange ? "true" : "false",
             "Whether to know the range of 8-bit data blindly "
             "(uchar is always [0,255], signed char is [-128,127]). "
             "Explicitly setting this is useful "
             "only with rescaling (\"-r\") or if the map domain is only "
             "implicitly defined");
  hestOptAdd(&opt, "t,type", "type", airTypeOther, 1, 1, &typeOut, "default",
             "specify the type (\"int\", \"float\", etc.) of the "
             "output nrrd. "
             "By default (not using this option), the output type "
             "is the map's type.",
             NULL, NULL, &unrrduHestMaybeTypeCB);
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_mrmapInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  /* see comment rmap.c */

  /* by the end of this block we need to have nmmap and mapAxis */
  if (1 == _nmmapLen) {
    /* we got the mmap as a single nrrd */
    nmmap = _nmmap[0];
    mapAxis = nmmap->dim - nin->dim - 1;
    /* its not our job to do real error checking ... */
    mapAxis = AIR_MIN(mapAxis, nmmap->dim - 1);
  } else {
    /* we have to join together multiple nrrds to get the mmap */
    nmmap = nrrdNew();
    airMopAdd(mop, nmmap, (airMopper)nrrdNuke, airMopAlways);
    /* assume that mmap component nrrds are all compatible sizes,
       nrrdJoin will fail if they aren't */
    mapAxis = _nmmap[0]->dim - nin->dim;
    if (nrrdJoin(nmmap, (const Nrrd**)_nmmap, _nmmapLen, mapAxis, AIR_TRUE)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble joining mmap:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    /* set these if they were given, they'll be NaN otherwise */
    nmmap->axis[mapAxis].min = min;
    nmmap->axis[mapAxis].max = max;
  }

  if (!( AIR_EXISTS(nmmap->axis[mapAxis].min) && 
         AIR_EXISTS(nmmap->axis[mapAxis].max) )) {
    rescale = AIR_TRUE;
  }
  if (rescale) {
    range = nrrdRangeNew(min, max);
    airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
    nrrdRangeSafeSet(range, nin, blind8BitRange);
  }

  if (nrrdTypeDefault == typeOut) {
    typeOut = nmmap->type;
  }
  if (nrrdApplyMulti1DRegMap(nout, nin, range, nmmap, typeOut, rescale)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble applying map:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(mrmap, INFO);
