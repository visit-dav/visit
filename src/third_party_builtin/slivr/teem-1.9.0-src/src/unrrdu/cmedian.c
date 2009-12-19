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

#define INFO "Cheap histogram-based median/mode filtering"
char *_unrrdu_cmedianInfoL =
(INFO
 ". Only works on 1, 2, or 3 dimensions.  The window "
 "over which filtering is done is always square, and "
 "only a simplistic weighting scheme is available. "
 "The method is cheap because it does the median "
 "or mode based on a histogram, which enforces a quantization to the number "
 "of bins in the histogram, which probably means a loss of precision for "
 "anything except 8-bit data.  Also, integral values can be recovered "
 "exactly only when the number of bins is exactly min-max+1 (as reported "
 "by \"unu minmax\"). ");

int
unrrdu_cmedianMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout, *ntmp, **mnout;
  int pad, pret, mode, chan, ni, nsize;
  unsigned int bins, radius;
  airArray *mop;
  float wght;

  hestOptAdd(&opt, "r,radius", "radius", airTypeUInt, 1, 1, &radius, NULL,
             "how big a window to filter over. \"-r 1\" leads to a "
             "3x3 window in an image, and a 3x3x3 window in a volume");
  hestOptAdd(&opt, "mode", NULL, airTypeInt, 0, 0, &mode, NULL,
             "By default, median filtering is done.  Using this option "
             "enables mode filtering, in which the most common value is "
             "used as output");
  hestOptAdd(&opt, "b,bins", "num", airTypeUInt, 1, 1, &bins, "256",
             "# of bins in histogram.  It is in your interest to minimize "
             "this number, since big histograms mean slower execution "
             "times.  8-bit data needs at most 256 bins.");
  hestOptAdd(&opt, "w,weight", "weight", airTypeFloat, 1, 1, &wght, "1.0",
             "How much higher to preferentially weight samples that are "
             "closer to the center of the window.  \"1.0\" weight means that "
             "all samples are uniformly weighted over the window, which "
             "facilitates a simple speed-up. ");
  hestOptAdd(&opt, "p,pad", NULL, airTypeInt, 0, 0, &pad, NULL,
             "Pad the input (with boundary method \"bleed\"), "
             "and crop the output, so as to "
             "overcome our cheapness and correctly "
             "handle the border.  Obviously, this takes more memory.");
  hestOptAdd(&opt, "c,channel", NULL, airTypeInt, 0, 0, &chan, NULL,
             "Slice the input along axis 0, run filtering on all slices, "
             "and join the results back together.  This is the way you'd "
             "want to process color (multi-channel) images or volumes.");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_cmedianInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);
  
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (chan) {
    nsize = nin->axis[0].size;
    mnout = (Nrrd **)calloc(nsize, sizeof(Nrrd));
    airMopAdd(mop, mnout, airFree, airMopAlways);
    ntmp = nrrdNew();
    airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
    for (ni=0; ni<nsize; ni++) {
      if (nrrdSlice(ntmp, nin, 0, ni)) {
        airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: error slicing input at pos = %d:\n%s",
                me, ni, err);
        airMopError(mop);
        return 1;
      }
      airMopAdd(mop, mnout[ni] = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
      if (nrrdCheapMedian(mnout[ni], ntmp, pad, mode, radius, wght, bins)) {
        airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: error doing cheap median:\n%s", me, err);
        airMopError(mop);
        return 1;
      }
    }
    if (nrrdJoin(nout, (const Nrrd**)mnout, nsize, 0, AIR_TRUE)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error doing final join:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
  } else {
    if (nrrdCheapMedian(nout, nin, pad, mode, radius, wght, bins)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error doing cheap median:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(cmedian, INFO);
