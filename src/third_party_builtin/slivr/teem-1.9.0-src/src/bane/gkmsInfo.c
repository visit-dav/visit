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

#include "bane.h"
#include "privateBane.h"

#define INFO_INFO "Project histogram volume for opacity function generation"
char *_baneGkms_infoInfoL =
  (INFO_INFO
   ". This distills the histogram volume down to the information required "
   "to create either 1-D or 2-D opacity functions.");

int
baneGkms_infoMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *outS, *perr, err[BIFF_STRLEN];
  Nrrd *hvol, *nout;
  airArray *mop;
  int pret, one, measr;

  hestOptAdd(&opt, "m", "measr", airTypeEnum, 1, 1, &measr, "mean",
             "How to project along the 2nd derivative axis.  Possibilities "
             "include:\n "
             "\b\bo \"mean\": average value\n "
             "\b\bo \"median\": value at 50th percentile\n "
             "\b\bo \"mode\": most common value\n "
             "\b\bo \"min\", \"max\": probably not useful",
             NULL, baneGkmsMeasr);
  hestOptAdd(&opt, "one", NULL, airTypeInt, 0, 0, &one, NULL,
             "Create 1-dimensional info file; default is 2-dimensional");
  hestOptAdd(&opt, "i", "hvolIn", airTypeOther, 1, 1, &hvol, NULL,
             "input histogram volume (from \"gkms hvol\")",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "o", "infoOut", airTypeString, 1, 1, &outS, NULL,
             "output info file, used by \"gkms pvg\" and \"gkms opac\"");
  
  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_baneGkms_infoInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (baneOpacInfo(nout, hvol, one ? 1 : 2, measr)) {
    sprintf(err, "%s: trouble distilling histogram info", me);
    biffAdd(BANE, err); airMopError(mop); return 1;
  }

  if (nrrdSave(outS, nout, NULL)) {
    sprintf(err, "%s: trouble saving info file", me);
    biffMove(BANE, err, NRRD); airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
BANE_GKMS_CMD(info, INFO_INFO);

