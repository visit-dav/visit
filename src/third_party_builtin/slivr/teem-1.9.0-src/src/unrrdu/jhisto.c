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

#define INFO "Create joint histogram of two or more nrrds"
char *_unrrdu_jhistoInfoL =
(INFO
 ". Each axis of the output corresponds to one of the "
 "input nrrds, and each bin in the output records the "
 "number of corresponding positions in the inputs with "
 "a combination of values represented by the coordinates "
 "of the bin.");

int
unrrdu_jhistoMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd **nin;
  Nrrd *nout, *nwght;
  size_t *bin;
  int type, clamp[NRRD_DIM_MAX], pret;
  unsigned int binLen, minLen, maxLen, ninLen, ai;
  airArray *mop;
  double *min, *max;
  NrrdRange **range;

  hestOptAdd(&opt, "b,bin", "bins0 bins1", airTypeSize_t, 2, -1, &bin, NULL,
             "bins<i> is the number of bins to use along axis i (of joint "
             "histogram), which represents the values of nin<i> ",
             &binLen);
  hestOptAdd(&opt, "w,weight", "nweight", airTypeOther, 1, 1, &nwght, "",
             "how to weigh contributions to joint histogram.  By default "
             "(not using this option), the increment is one bin count per "
             "sample, but by giving a nrrd, the value in the nrrd at the "
             "corresponding location will be the bin count increment ",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "min,minimum", "min0 min1", airTypeDouble, 2, -1,
             &min, "nan nan",
             "min<i> is the low range of values to be quantized along "
             "axis i; use \"nan\" to represent lowest value present ",
             &minLen);
  hestOptAdd(&opt, "max,maximum", "max0 max1", airTypeDouble, 2, -1,
             &max, "nan nan",
             "max<i> is the high range of values to be quantized along "
             "axis i; use \"nan\" to represent highest value present ",
             &maxLen);
  OPT_ADD_TYPE(type, "type to use for output (the type used to store hit "
               "counts in the joint histogram).  Clamping is done on hit "
               "counts so that they never overflow a fixed-point type",
               "uint");
  hestOptAdd(&opt, "i,input", "nin0 nin1", airTypeOther, 2, -1, &nin, NULL,
             "All input nrrds",
             &ninLen, NULL, nrrdHestNrrd);
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_jhistoInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  if (ninLen != binLen) {
    fprintf(stderr, "%s: # input nrrds (%d) != # bin specifications (%d)\n",
            me, ninLen, binLen);
    airMopError(mop);
    return 1;
  }
  range = (NrrdRange **)calloc(ninLen, sizeof(NrrdRange*));
  airMopAdd(mop, range, airFree, airMopAlways);
  for (ai=0; ai<ninLen; ai++) {
    range[ai] = nrrdRangeNew(AIR_NAN, AIR_NAN);
    airMopAdd(mop, range[ai], (airMopper)nrrdRangeNix, airMopAlways);
  }
  if (2 != minLen || (AIR_EXISTS(min[0]) || AIR_EXISTS(min[1]))) {
    if (minLen != ninLen) {
      fprintf(stderr, "%s: # mins (%d) != # input nrrds (%d)\n", me,
              minLen, ninLen);
      airMopError(mop); return 1;
    }
    for (ai=0; ai<ninLen; ai++) {
      range[ai]->min = min[ai];
    }
  }
  if (2 != maxLen || (AIR_EXISTS(max[0]) || AIR_EXISTS(max[1]))) {
    if (maxLen != ninLen) {
      fprintf(stderr, "%s: # maxs (%d) != # input nrrds (%d)\n", me,
              maxLen, ninLen);
      airMopError(mop); return 1;
    }
    for (ai=0; ai<ninLen; ai++) {
      range[ai]->max = max[ai];
    }
  }
  for (ai=0; ai<ninLen; ai++) {
    clamp[ai] = 0;
  }

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdHistoJoint(nout, (const Nrrd**)nin, (const NrrdRange**)range,
                     ninLen, nwght, bin, type, clamp)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error doing joint histogram:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(jhisto, INFO);
