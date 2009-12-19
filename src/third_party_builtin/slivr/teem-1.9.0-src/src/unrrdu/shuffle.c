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

#define INFO "Permute slices along one axis"
char *_unrrdu_shuffleInfoL = 
(INFO
 ". Slices along one axis are re-arranged as units "
 "according to the given permutation (or its inverse). "
 "The permutation tells which old slice to put at each "
 "new position.  For example, the shuffle "
 "0->1,\t1->2,\t2->0 would be \"2 0 1\".  Obviously, "
 "if you have to rearrange the many slices of a large "
 "dataset, you should probably store the permutation "
 "in a plain text file and use it as a "
 "\"response file\".");

int
unrrdu_shuffleMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  unsigned int di, axis, permLen, *perm, *iperm, *whichperm;
  size_t *realperm;
  int inverse, pret;
  airArray *mop;

  /* so that long permutations can be read from file */
  hparm->respFileEnable = AIR_TRUE;

  hestOptAdd(&opt, "p,permute", "slc0 slc1", airTypeUInt, 1, -1, &perm, NULL,
             "new slice ordering", &permLen);
  hestOptAdd(&opt, "inv,inverse", NULL, airTypeInt, 0, 0, &inverse, NULL,
             "use inverse of given permutation");
  OPT_ADD_AXIS(axis, "axis to shuffle along");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_shuffleInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  
  /* we have to do error checking on axis in order to do error
     checking on length of permutation */
  if (!( axis < nin->dim )) {
    fprintf(stderr, "%s: axis %d not in valid range [0,%d]\n", 
            me, axis, nin->dim-1);
    airMopError(mop);
    return 1;
  }
  if (!( permLen == nin->axis[axis].size )) {
    fprintf(stderr, "%s: permutation length (%u) != axis %d's size ("
            _AIR_SIZE_T_CNV ")\n",
            me, permLen, axis, nin->axis[axis].size);
    airMopError(mop);
    return 1;
  }
  if (inverse) {
    iperm = (unsigned int*)calloc(permLen, sizeof(unsigned int));
    airMopAdd(mop, iperm, airFree, airMopAlways);
    if (nrrdInvertPerm(iperm, perm, permLen)) {
      fprintf(stderr,
              "%s: couldn't compute inverse of given permutation\n", me);
      airMopError(mop);
      return 1;
    }
    whichperm = iperm;
  } else {
    whichperm = perm;
  }
  
  realperm = (size_t*)calloc(permLen, sizeof(size_t));
  airMopAdd(mop, realperm, airFree, airMopAlways);
  for (di=0; di<permLen; di++) {
    realperm[di] = whichperm[di];
  }
  if (nrrdShuffle(nout, nin, axis, realperm)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error shuffling nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(shuffle, INFO);
