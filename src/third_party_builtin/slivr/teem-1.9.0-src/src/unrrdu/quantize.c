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

#define INFO "Quantize values to 8, 16, or 32 bits"
char *_unrrdu_quantizeInfoL = 
(INFO ". Input values can be fixed point (e.g. quantizing ushorts down to "
 "uchars) or floating point.  Values are clamped to the min and max before "
 "they are quantized, so there is no risk of getting 255 where you expect 0 "
 "(with unsigned char output, for example).  This does not do any non-linear "
 "quantization.  See also \"unu convert\", \"unu 2op x\", and \"unu 3op "
 "clamp\".");

int
unrrdu_quantizeMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  int pret, blind8BitRange;
  unsigned int bits;
  double min, max;
  NrrdRange *range;
  airArray *mop;

  hestOptAdd(&opt, "b,bits", "bits", airTypeOther, 1, 1, &bits, NULL,
             "Number of bits to quantize down to; determines the type "
             "of the output nrrd:\n "
             "\b\bo \"8\": unsigned char\n "
             "\b\bo \"16\": unsigned short\n "
             "\b\bo \"32\": unsigned int",
             NULL, NULL, &unrrduHestBitsCB);
  hestOptAdd(&opt, "min,minimum", "value", airTypeDouble, 1, 1, &min, "nan",
             "Value to map to zero. Defaults to lowest value found in "
             "input nrrd.");
  hestOptAdd(&opt, "max,maximum", "value", airTypeDouble, 1, 1, &max, "nan",
             "Value to map to highest unsigned integral value. "
             "Defaults to highest value found in input nrrd.");
  hestOptAdd(&opt, "blind8", "bool", airTypeBool, 1, 1, &blind8BitRange,
             nrrdStateBlind8BitRange ? "true" : "false",
             "if not using \"-min\" or \"-max\", whether to know "
             "the range of 8-bit data blindly (uchar is always [0,255], "
             "signed char is [-128,127])");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_quantizeInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  /* If the input nrrd never specified min and max, then they'll be
     AIR_NAN, and nrrdRangeSafeSet will find them, and will do so
     according to blind8BitRange */
  range = nrrdRangeNew(min, max);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  nrrdRangeSafeSet(range, nin, blind8BitRange);
  if (nrrdQuantize(nout, nin, range, bits)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error quantizing nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(quantize, INFO);
