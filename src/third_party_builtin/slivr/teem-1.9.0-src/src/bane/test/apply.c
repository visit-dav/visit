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
  /*                      0     1     2       3   (4) */
  fprintf(stderr, "usage: %s <nin> <measr> <nout>\n", me);
  exit(1);
}

int
main(int argc, char *argv[]) {
  int measr;
  char *iStr, *mStr, *oStr;
  Nrrd *nin, *nout;
  
  me = argv[0];
  if (4 != argc)
    usage();

  iStr = argv[1];
  mStr = argv[2];
  oStr = argv[3];
  
  if (nrrdLoad(nin=nrrdNew(), iStr)) {
    fprintf(stderr, "%s: trouble reading input nrrd:\n%s\n", me, 
            biffGet(NRRD));
    usage();
  }

  if (1 != sscanf(mStr, "%d", &measr)) {
    fprintf(stderr, "%s: couldn't parse %s as int\n", me, mStr);
    usage();
  }
  
  if (baneApplyMeasr(nout = nrrdNew(), nin, measr)) {
    fprintf(stderr, "%s: trouble:\n%s\n", me, biffGet(BANE));
    exit(1);
  }

  if (nrrdSave(oStr, nout, NULL)) {
    fprintf(stderr, "%s: trouble writing output nrrd:\n%s\n", me,
            biffGet(NRRD));
    usage();
  }
  exit(0);
}
