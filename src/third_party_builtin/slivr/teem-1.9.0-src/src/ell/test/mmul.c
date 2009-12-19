/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
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

#include "../ell.h"

char *mulInfo = ("Tests ell_Nm_mul");

int
main(int argc, char *argv[]) {
  char *me, *outS, *err;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;
  Nrrd *_ninA, *_ninB, *ninA, *ninB, *nmul;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, NULL, "matrix", airTypeOther, 1, 1, &_ninA, NULL,
             "first matrix",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, NULL, "matrix", airTypeOther, 1, 1, &_ninB, NULL,
             "first matrix",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
             "file to write output nrrd to");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, mulInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  ninA = nrrdNew();
  airMopAdd(mop, ninA, (airMopper)nrrdNuke, airMopAlways);
  ninB = nrrdNew();
  airMopAdd(mop, ninB, (airMopper)nrrdNuke, airMopAlways);
  nmul = nrrdNew();
  airMopAdd(mop, nmul, (airMopper)nrrdNuke, airMopAlways);
  
  nrrdConvert(ninA, _ninA, nrrdTypeDouble);
  nrrdConvert(ninB, _ninB, nrrdTypeDouble);
  if (ell_Nm_mul(nmul, ninA, ninB)) {
    airMopAdd(mop, err = biffGetDone(ELL), airFree, airMopAlways);
    fprintf(stderr, "%s: problem inverting:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  if (nrrdSave(outS, nmul, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem saving output:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  exit(0);
}

