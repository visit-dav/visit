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


#include "../limn.h"

void
cb(float rgb[3], float vec[3], void *blah) {
  float r, g, b;

  AIR_UNUSED(blah);
  ELL_3V_GET(r, g, b, vec);
  r = AIR_MAX(0, r);
  g = AIR_MAX(0, g);
  b = AIR_MAX(0, b);
  ELL_3V_SET(rgb, r, g, b);
  return;
}

char *me;

int
main(int argc, char *argv[]) {
  Nrrd *map, *ppm;
  NrrdRange *range;

  AIR_UNUSED(argc);
  me = argv[0];

  if (limnEnvMapFill(map=nrrdNew(), cb, limnQN16checker, NULL)) {
    fprintf(stderr, "%s: trouble:\n%s", me, biffGet(LIMN));
    exit(1);
  }
  range = nrrdRangeNew(0, 1);
  if (nrrdQuantize(ppm=nrrdNew(), map, range, 8)) {
    fprintf(stderr, "%s: trouble:\n%s", me, biffGet(NRRD));
    exit(1);
  }
  if (nrrdSave("map.ppm", ppm, NULL)) {
    fprintf(stderr, "%s: trouble:\n%s", me, biffGet(NRRD));
    exit(1);
  }

  nrrdNuke(map);
  nrrdNuke(ppm);
  nrrdRangeNix(range);
  exit(0);
}
