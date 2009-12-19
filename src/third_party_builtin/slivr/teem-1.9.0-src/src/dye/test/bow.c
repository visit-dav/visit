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


#include "../dye.h"

char *me;

void
usage() {
  /*                      0    1      2        3       (4) */
  fprintf(stderr, "usage: %s <res> <scale> <out.txt> \n", me);
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *resS, *scS, *outS;
  int i, res;
  FILE *out;
  float hue, R, G, B, sc;
  
  me = argv[0];
  if (4 != argc)
    usage();
  resS = argv[1];
  scS = argv[2];
  outS = argv[3];
  
  if (1 != sscanf(resS, "%d", &res)) {
    fprintf(stderr, "%s: couldn't parse \"%s\" as int\n", me, resS);
    exit(1);
  }
  if (1 != sscanf(scS, "%f", &sc)) {
    fprintf(stderr, "%s: couldn't parse \"%s\" as float\n", me, scS);
    exit(1);
  }
  if (!(out = fopen(outS, "wa"))) {
    fprintf(stderr, "%s: couldn't open \"%s\" for writing\n", me, outS);
    exit(1);
  }

  fprintf(out, "# space: RGB\n");
  for (i=0; i<=res-1; i++) {
    hue = AIR_AFFINE(0, i, res, 0.0, 1.0);
    dyeHSVtoRGB(&R, &G, &B, hue, 1, 1);
    fprintf(out, "%g %g %g %g\n", hue, sc*R, sc*G, sc*B);
  }

  fclose(out);
  exit(0);
}
