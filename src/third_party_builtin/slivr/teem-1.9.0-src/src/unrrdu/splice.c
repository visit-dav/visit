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

#define INFO "Replace a slice with a different nrrd"
char *_unrrdu_spliceInfoL = 
(INFO ". This is functionally the opposite of \"slice\".");

int
unrrdu_spliceMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout, *nslice;
  unsigned int axis;
  int _pos[2], pos, pret;
  airArray *mop;

  OPT_ADD_AXIS(axis, "axis to splice along");
  hestOptAdd(&opt, "p,position", "pos", airTypeOther, 1, 1, _pos, NULL,
             "position to splice at:\n "
             "\b\bo <int> gives 0-based index\n "
             "\b\bo M-<int> give index relative "
             "to the last sample on the axis (M == #samples-1).",
             NULL, NULL, &unrrduHestPosCB);
  hestOptAdd(&opt, "s,slice", "nslice", airTypeOther, 1, 1, &(nslice), NULL,
             "slice nrrd.  This the slice to be inserted in \"nin\"",
             NULL, NULL, nrrdHestNrrd);
  OPT_ADD_NIN(nin, "input nrrd.  This the nrrd into which the slice will "
              "be inserted");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_spliceInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);
  if (!( axis < nin->dim )) {
    fprintf(stderr, "%s: axis %u not in range [0,%u]\n", me, axis, nin->dim-1);
    return 1;
  }
  if (_pos[0] == -1) {
    fprintf(stderr, "%s: m+<int> specification format meaningless here\n", me);
    return 1;
  }
  pos = _pos[0]*(nin->axis[axis].size-1) + _pos[1];

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdSplice(nout, nin, nslice, axis, pos)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error splicing nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(splice, INFO);
