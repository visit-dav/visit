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


#include "../bane.h"

char *me;

void
usage() {
  /*                      0     1       2     3    (4)  */
  fprintf(stderr, "usage: %s <hvolin> <dim> <nout>\n", me);
  exit(1);
}

int
main(int argc, char *argv[]) {
  Nrrd *hvol, *info;
  char *iStr, *dStr, *oStr;
  int dim;

  me = argv[0];
  if (argc != 4)
    usage();
  iStr = argv[1];
  dStr = argv[2];
  oStr = argv[3];

  if (nrrdLoad(hvol=nrrdNew(), iStr, NULL)) {
    fprintf(stderr, "%s: trouble reading hvol:\n%s\n", me, biffGet(NRRD));
    usage();
  }
  if (1 != sscanf(dStr, "%d", &dim)) {
    fprintf(stderr, "%s: trouble parsing %s as an int\n", me, dStr);
    usage();
  }
  if (baneOpacInfo(info = nrrdNew(), hvol, dim, nrrdMeasureHistoMean)) {
    fprintf(stderr, "%s: trouble calculting %d-D opacity info:\n%s\n",
            me, dim, biffGet(BANE));
    exit(1);
  }
  if (nrrdSave(oStr, info, NULL)) {
    fprintf(stderr, "%s: trouble saving nrrd to %s:\n%s\n", me, oStr,
            biffGet(NRRD));
    exit(1);
  }
  nrrdNuke(hvol);
  nrrdNuke(info);
  exit(0);
}
