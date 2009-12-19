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
  /*                      0    1     2       3   (4) */
  fprintf(stderr, "usage: %s <bIn> <pIn> <opacOut>\n", me);
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *bStr, *pStr, *oStr;
  Nrrd *b, *p, *o;

  me = argv[0];
  if (argc != 4) 
    usage();
  bStr = argv[1];
  pStr = argv[2];
  oStr = argv[3];
  if (nrrdLoad(b=nrrdNew(), bStr, NULL)) {
    fprintf(stderr, "%s: trouble reading %s:\n%s\n", me, bStr, biffGet(NRRD));
    usage();
  }

  if (nrrdLoad(p=nrrdNew(), pStr, NULL)) {
    fprintf(stderr, "%s: trouble reading %s:\n%s\n", me, pStr, biffGet(NRRD));
    usage();
  }
  
  if (baneOpacCalc(o = nrrdNew(), b, p)) {
    fprintf(stderr, "%s: trouble calculating opac:\n%s", me, biffGet(BANE));
    exit(1);
  }

  if (nrrdSave(oStr, o, NULL)) {
    fprintf(stderr, "%s: trouble writing %s:\n%s\n", me, oStr, biffGet(NRRD));
    exit(1);
  }

  nrrdNuke(o);
  nrrdNuke(b);
  nrrdNuke(p);
  exit(0);
}
