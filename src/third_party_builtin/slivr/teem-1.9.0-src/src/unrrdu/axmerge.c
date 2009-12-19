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

#define INFO "Merge two adjacent axes into one"
char *_unrrdu_axmergeInfoL =
(INFO
 ". A more general version of \"unu axdelete\". "
 "The underlying linear ordering of the samples is "
 "unchanged, and the information about the other axes is "
 "shifted downwards as needed.");

int
unrrdu_axmergeMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout[2];
  int *axes, pret, ni;
  unsigned int ii, jj, axesLen;
  airArray *mop;

  hestOptAdd(&opt, "a,axis", "ax0", airTypeInt, 1, -1, &axes, NULL,
             "axis (or axes) to merge.  Each axis index identified is the "
             "lower of the pair of axes that will be merged.  Saying \"-a 2\" "
             "means to merge axis 2 and axis 3 into axis 2.  If multiple "
             "merges are to be done, the indices listed here are for "
             "the axes prior to any merging.", &axesLen);
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_axmergeInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  airMopAdd(mop, nout[0]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nout[1]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);

  if (axesLen > 1) {
    /* sort merge axes into ascending order */
    qsort(axes, axesLen, sizeof(int), nrrdValCompare[nrrdTypeInt]);
  }

  ni = 0;
  for (ii=0; ii<axesLen; ii++) {
    if (nrrdAxesMerge(nout[ni], !ii ? nin : nout[1-ni], axes[ii])) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error merging axes:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    for (jj=ii+1; jj<axesLen; jj++) {
      axes[jj] -= 1;
    }
    ni = 1-ni;
  }
  
  SAVE(out, nout[1-ni], NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(axmerge, INFO);
