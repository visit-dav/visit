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

#define INFO "Tile slices of one axis into two other axes"
char *_unrrdu_tileInfoL =
(INFO
 ". Tiling an array means splitting one axis into fast and slow parts, "
 "and then interleaving those parts into other (existing) axes by doing "
 "two axis merges, which combine an existing axis with part of the split "
 "axis.  This reduces the dimension by one.  The three axis arguments all "
 "identify axes in the input array as is.  This provides, for example, "
 "a simple way of viewing the 128 slices along the slow axis of a 3-D volume "
 "as a 16x8 tiled array of 2-D slices, as with \"-a 2 0 1 -s 16 8\".");

int
unrrdu_tileMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  int pret;
  size_t size[2];
  unsigned int axes[3];
  airArray *mop;

  hestOptAdd(&opt, "a,axis", "axSplit ax0 ax1", airTypeUInt, 3, 3, axes, NULL,
             "axSplit is divided and merged with ax0 and ax1");
  hestOptAdd(&opt, "s,size", "fast slow", airTypeSize_t, 2, 2, size, NULL,
             "fast and slow axis sizes to produce as result of splitting "
             "the axSplit axis.");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_tileInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  
  if (nrrdTile2D(nout, nin, axes[1], axes[2], axes[0], size[0], size[1])) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error tiling nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(tile, INFO);
