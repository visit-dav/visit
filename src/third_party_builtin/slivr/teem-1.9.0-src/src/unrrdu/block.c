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

#define INFO "Condense axis-0 scanlines into \"blocks\""
char *_unrrdu_blockInfoL = 
(INFO
 ". Output nrrd will be of type \"block\": the type "
 "for an opaque chunk of "
 "memory.  Block samples can be sliced, cropped, shuffled, "
 "permuted, etc., but there is no scalar value associated "
 "with them, so they can not be histogrammed, quantized, "
 "resampled, converted, etc.  The output nrrd will have "
 "one less dimension than input; axis N information will "
 "be shifted down to axis N-1.  Underlying data "
 "is unchanged.");

int
unrrdu_blockMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  airArray *mop;
  int pret;

  /* if we gave a default for this, then there it would fine to have 
     no command-line arguments whatsoever, and then the user would not
     know how to get the basic usage information */
  hestOptAdd(&opt, "i", "nin", airTypeOther, 1, 1, &nin, NULL, "input nrrd",
             NULL, NULL, nrrdHestNrrd);
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_blockInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdBlock(nout, nin)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error blocking nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(block, INFO);
