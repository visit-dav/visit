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

#define INFO "Remap CC values down to lowest contiguous values"
char *_unrrdu_ccsettleInfoL =
(INFO
 ".");

int
unrrdu_ccsettleMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err, *valS;
  Nrrd *nin, *nout, *nval=NULL;
  airArray *mop;
  int pret;

  mop = airMopNew();
  hestOptAdd(&opt, "i,input", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input nrrd",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "v,values", "filename", airTypeString, 1, 1, &valS, "",
             "Giving a filename here allows you to save out the mapping "
             "from new (settled) values to old values, in the form of a "
             "1-D lookup table");
  OPT_ADD_NOUT(out, "output nrrd");
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_ccsettleInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdCCSettle(nout, airStrlen(valS) ? &nval : NULL, nin)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error settling connected components:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  if (nval) {
    airMopAdd(mop, nval, (airMopper)nrrdNuke, airMopAlways);
  }

  if (airStrlen(valS)) {
    SAVE(valS, nval, NULL);
  }
  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(ccsettle, INFO);
