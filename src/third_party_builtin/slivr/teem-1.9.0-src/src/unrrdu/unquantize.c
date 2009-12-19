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

#define INFO "Recover floating point values from quantized data"
char *_unrrdu_unquantizeInfoL = INFO;

int
unrrdu_unquantizeMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  int dbl, pret;
  double oldMin, oldMax;
  airArray *mop;

  /* mandatory arg so that "unu unquantize" produces usage info */
  hestOptAdd(&opt, "i,input", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input nrrd",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "min,minimum", "value", airTypeDouble, 1, 1, &oldMin, "nan",
             "Lowest value prior to quantization.  Defaults to "
             "nin->oldMin if it exists, otherwise 0.0");
  hestOptAdd(&opt, "max,maximum", "value", airTypeDouble, 1, 1, &oldMax, "nan",
             "Highest value prior to quantization.  Defaults to "
             "nin->oldMax if it exists, otherwise 1.0");
  hestOptAdd(&opt, "double", NULL, airTypeBool, 0, 0, &dbl, NULL,
             "Use double for output type, instead of float");
  OPT_ADD_NOUT(out, "output nrrd");
  
  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_unquantizeInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (AIR_EXISTS(oldMin))
    nin->oldMin = oldMin;
  if (AIR_EXISTS(oldMax))
    nin->oldMax = oldMax;
  if (nrrdUnquantize(nout, nin, dbl ? nrrdTypeDouble : nrrdTypeFloat)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error unquantizing nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(unquantize, INFO);
