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

#define INFO "Collapse scanlines to scalars along some axis"
char *_unrrdu_projectInfoL = 
(INFO
 ". The scanline is reduced to a single scalar by "
 "\"measuring\" all the values in the scanline "
 "with some measure.  The output nrrd has dimension "
 "one less than input; the output type depends on "
 "the measure in a non-trivial way, or it can be set explicitly "
 "with the \"-t\" option.");

int
unrrdu_projectMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  unsigned int axis;
  int measr, pret, type;
  airArray *mop;

  OPT_ADD_AXIS(axis, "axis to project along");
  hestOptAdd(&opt, "m,measure", "measr", airTypeEnum, 1, 1, &measr, NULL,
             "How to \"measure\" a scanline, by summarizing all its values "
             "with a single scalar. " NRRD_MEASURE_DESC,
             NULL, nrrdMeasure);
  hestOptAdd(&opt, "t,type", "type", airTypeOther, 1, 1, &type, "default",
             "type to use for output. By default (not using this option), "
             "the output type is determined auto-magically",
             NULL, NULL, &unrrduHestMaybeTypeCB);
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_projectInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdProject(nout, nin, axis, measr, type)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error projecting nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(project, INFO);
