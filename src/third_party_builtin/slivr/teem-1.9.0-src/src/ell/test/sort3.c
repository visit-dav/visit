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


#include <stdio.h>
#include "../ell.h"

char *me;

void
usage() {
  /*                      0   1   2   3   (4) */
  fprintf(stderr, "usage: %s <a> <b> <c>\n", me);
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *aS, *bS, *cS;
  float a, b, c, t;

  me = argv[0];
  if (4 != argc) {
    usage();
  }

  aS = argv[1];
  bS = argv[2];
  cS = argv[3];
  if (3 != (sscanf(aS, "%g", &a) +
            sscanf(bS, "%g", &b) +
            sscanf(cS, "%g", &c))) {
    fprintf(stderr, "%s: couldn't parse \"%s\", \"%s\", \"%s\" as floats\n",
            me, aS, bS, cS);
    usage();
  }
  
  printf("%g %g %g (max idx %d; min idx %d) --> ", a, b, c,
         ELL_MAX3_IDX(a, b, c), ELL_MIN3_IDX(a, b, c));
  ELL_SORT3(a, b, c, t);
  printf("%g %g %g\n", a, b, c);

  exit(0);
}
