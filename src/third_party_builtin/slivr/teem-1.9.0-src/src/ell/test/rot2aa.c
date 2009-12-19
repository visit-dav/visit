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

char *rot2aaInfo = ("converts a list of rotation matrices to a list of "
                    "angle axis specifications");

int
main(int argc, char *argv[]) {
  char *me, *outS, *err;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  Nrrd *_nmat, *nmat;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hparm->elideMultipleNonExistFloatDefault = AIR_TRUE;
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, NULL, "input matrices", airTypeOther, 1, 1, &_nmat, NULL,
             "list of rotation matrices, 2-D array with one matrix "
             "(in ROW-major order) per scanline", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, NULL, "output filename", airTypeString, 1, 1, &outS, "-",
             "file to write EPS output to");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, rot2aaInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!( 2 == _nmat->dim && 9 == _nmat->axis[0].size )) {
    fprintf(stderr, "%s: need a 2-D 9-by-N nrrd\n", me);
    airMopError(mop); return 1;
  }
  nmat = nrrdNew();
  airMopAdd(mop, nmat, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(nmat, _nmat, nrrdTypeDouble)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't convert to double:\n%s", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  exit(0);
}
