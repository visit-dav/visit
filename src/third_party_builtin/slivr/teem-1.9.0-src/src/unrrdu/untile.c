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

#define INFO "Undo \"unu tile\": merge slow parts of two axis splits"
char *_unrrdu_untileInfoL =
(INFO
 ". Untiling an array means spliting two axes, permuting the slow parts "
 "of those axes to be adjecent in the axis ordering, and then merging "
 "them.  This increases the dimension by one.  Undoing a \"unu tile\" "
 "uses the same \"-s\" argument, and sometimes a different \"-a\" argument, "
 "as demonstrated here for a 3-D array:\n "
 "\"unu untile -a 2 0 1\" undoes \"unu tile -a 2 0 1\"\n "
 "\"unu untile -a 1 0 1\" undoes \"unu tile -a 1 0 2\"\n "
 "\"unu untile -a 0 0 1\" undoes \"unu tile -a 0 1 2\". ");

int
unrrdu_untileMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  unsigned int axes[3];
  int pret;
  size_t size[2];
  airArray *mop;

  hestOptAdd(&opt, "a,axis", "axMerge ax0 ax1", airTypeUInt, 3, 3, axes, NULL,
             "the slow parts of axes ax0 and ax1 are merged into a (new) "
             "axis axMerge, with the axis ax0 part being faster than ax1.");
  hestOptAdd(&opt, "s,size", "size0 size1", airTypeSize_t, 2, 2, size, NULL,
             "the slow parts of axes ax0 and ax1 are taken to have size "
             "size0 and size1, respectively, and axis axMerge will have "
             "size size0*size1.");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_untileInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  
  if (nrrdUntile2D(nout, nin, axes[1], axes[2], axes[0], size[0], size[1])) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error tiling nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(untile, INFO);
