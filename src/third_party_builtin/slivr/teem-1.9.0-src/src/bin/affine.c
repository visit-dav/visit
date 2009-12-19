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

#include <teem/air.h>

void
usage(char *me) {
  /*                       0   1        2       3        4        5  (6) */
  fprintf(stderr, "usage: %s <minIn> <value> <maxIn> <minOut> <maxOut>\n", me);
  fprintf(stderr, "\n");
  fprintf(stderr,
          "This uses air's AIR_AFFINE macro to print out the value which stands in\n");
  fprintf(stderr,
          "the same relationship with the interval [<minOut>, <maxOut>] as <value>\n");
  fprintf(stderr,
          "has with the interval [minIn, maxOut].\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *me;
  double minIn, valIn, maxIn, minOut, maxOut, valOut;

  me = argv[0];
  if (6 != argc) {
    usage(me);
  }

  if (5 != (sscanf(argv[1], "%lg", &minIn) +
            sscanf(argv[2], "%lg", &valIn) +
            sscanf(argv[3], "%lg", &maxIn) +
            sscanf(argv[4], "%lg", &minOut) +
            sscanf(argv[5], "%lg", &maxOut))) {
    fprintf(stderr, "%s: couldn't parse all args as doubles\n", me);
    usage(me);
  }

  valOut = AIR_AFFINE(minIn, valIn, maxIn, minOut, maxOut);
  printf("%g\n", valOut);

  exit(0);
}
