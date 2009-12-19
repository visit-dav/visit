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
  /*                      0     1        2       3          4   (5) */
  fprintf(stderr, "usage: %s <infoIn> <sigma> <gthresh> <posOut>\n", me);
  exit(1);
}

int
main(int argc, char *argv[]) {
  Nrrd *info, *pos;
  float sigma, gthresh;
  char *iStr, *oStr, *sigStr, *gthStr;

  me = argv[0];
  if (argc != 5) {
    usage();
  }
  iStr = argv[1];
  sigStr = argv[2];
  gthStr = argv[3];
  oStr = argv[4];

  if (1 != sscanf(sigStr, "%g", &sigma) ||
      1 != sscanf(gthStr, "%g", &gthresh)) {
    fprintf(stderr, "%s: couldn't parse %s and %s as floats\n", me, 
            sigStr, gthStr);
    usage();
  }

  if (nrrdLoad(info=nrrdNew(), iStr, NULL)) {
    fprintf(stderr, "%s: trouble reading \"%s\" :\n%s\n", me, 
            iStr, biffGet(NRRD));
    exit(1);
  }
  if (banePosCalc(pos = nrrdNew(), sigma, gthresh, info)) {
    fprintf(stderr, "%s: trouble calculating %s:\n%s\n", me,
            2 == info->dim ? "p(v,g)" : "p(v)", biffGet(BANE));
    exit(1);
  }
  if (nrrdSave(oStr, pos, NULL)) {
    fprintf(stderr, "%s: trouble writing output to \"%s\"\n", me, oStr);
    exit(1);
  }
  nrrdNuke(info);
  nrrdNuke(pos);
  exit(0);
}

