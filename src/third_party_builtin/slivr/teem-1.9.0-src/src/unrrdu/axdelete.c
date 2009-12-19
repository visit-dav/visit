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

#define INFO "Remove one or more singleton axes from a nrrd"
char *_unrrdu_axdeleteInfoL =
(INFO
 ". Singleton axes have only a single sample along them. "
 "The underlying linear ordering of the samples is "
 "unchanged, and the information about the other axes is "
 "shifted downwards as needed.  As a total hack, if you give "
 "-1 as the axis, this will do a matlab-style \"squeeze\", in which "
 "any and all singleton axes are removed.");

int
unrrdu_axdeleteMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout, *ntmp;
  int pret, _axis;
  unsigned axis;
  airArray *mop;

  hestOptAdd(&opt, "a,axis", "axis", airTypeInt, 1, 1, &_axis, NULL, 
             "dimension (axis index) of the axis to remove");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_axdeleteInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (-1 == _axis) {
    ntmp = nrrdNew();
    airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdCopy(nout, nin)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error copying axis:\n%s", me, err);
      airMopError(mop); return 1;
    }
    for (axis=0;
         axis<nout->dim && nout->axis[axis].size > 1;
         axis++);
    while (axis<nout->dim) {
      if (nrrdAxesDelete(ntmp, nout, axis)
          || nrrdCopy(nout, ntmp)) {
        airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: error deleting axis:\n%s", me, err);
        airMopError(mop); return 1;
      }
      for (axis=0;
           axis<nout->dim && nout->axis[axis].size > 1;
           axis++);
    }
  } else {
    if (nrrdAxesDelete(nout, nin, _axis)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error deleting axis:\n%s", me, err);
      airMopError(mop); return 1;
    }
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(axdelete, INFO);
