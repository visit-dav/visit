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
  /*                      0     1        2       (3) */
  fprintf(stderr, "usage: %s <colIn> <spaceOut> \n", me);
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *inS, *spcS, buff[512];
  dyeColor *col;
  int spc;
  
  me = argv[0];
  if (3 != argc)
    usage();
  inS = argv[1];
  spcS = argv[2];
  
  if (dyeColorParse(col = dyeColorNew(), inS)) {
    fprintf(stderr, "%s: trouble parsing \"%s\":\n%s", me, inS, biffGet(DYE));
    exit(1);
  }
  spc = dyeStrToSpace(spcS);
  if (dyeSpaceUnknown == spc) {
    fprintf(stderr, "%s: couldn't parse \"%s\" as colorspace\n", me, spcS);
    exit(1);
  }
  if (dyeConvert(col, spc)) {
    fprintf(stderr, "%s: trouble converting to %s:\n%s", 
            me, spcS, biffGet(DYE));
    exit(1);
  }
  printf("%s\n", dyeColorSprintf(buff, col));

  col = dyeColorNix(col);
  exit(0);
}
