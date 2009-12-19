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
  /*                      0    1      2        3        4         5   (6) */
  fprintf(stderr, "usage: %s <bIn> <sigma> <gthresh> <info2In> <trnsfOut>\n",
          me);
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *iStr, *bStr, *pStr, *sStr, *gStr, *tStr;
  FILE *file;
  Nrrd *info2, *Bcpts;
  dirtTrnsf *trnsf;
  float sigma, gthresh;

  me = argv[0];
  if (argc != 6) 
    usage();
  bStr = argv[1];
  sStr = argv[2];
  gStr = argv[3];
  iStr = argv[4];
  tStr = argv[5];

  if (!(file = fopen(iStr, "r"))) {
    fprintf(stderr, "%s: couldn't open info2 %s for reading\n", me, iStr);
    usage();
  }
  if (!(info2 = nrrdNewRead(file))) {
    fprintf(stderr, "%s: trouble reading info 2%s:\n%s\n", me, iStr, 
            biffGet(NRRD));
    usage();
  }
  fclose(file);

  if (!(file = fopen(bStr, "r"))) {
    fprintf(stderr, "%s: couldn't open b(x) %s for reading\n", me, bStr);
    usage();
  }
  if (!(Bcpts = nrrdNewRead(file))) {
    fprintf(stderr, "%s: trouble reading b(x) %s:\n%s\n", me, 
            bStr, biffGet(NRRD));
    usage();
  }
  fclose(file);

  if (!strcmp(sStr, "nan")) {
    sigma = airNanf();
  }
  else {
    if (1 != sscanf(sStr, "%g", &sigma)) {
      fprintf(stderr, "%s: couldn't parse sigma %s\n", me, sStr);
      usage();
    }
  }
  if (1 != sscanf(gStr, "%g", &gthresh)) {
    fprintf(stderr, "%s: couldn't parse gthresh %s\n", me, gStr);
    usage();
  }

  trnsf = baneNewDirtTrnsf(Bcpts, sigma, gthresh, info2);
  if (!trnsf) {
    fprintf(stderr, "%s: trouble computing opacity functions:\n%s\n", me,
            biffGet(BANE));
    exit(1);
  }
  if (!(file = fopen(tStr, "w"))) {
    fprintf(stderr, "%s: couldn't open %s for writing\n", me, tStr);
    usage();
  }
  if (dirtTrnsfWrite(file, trnsf)) {
    fprintf(stderr, "%s: trouble writing trnsf to %s\n:%s\n", me, 
            tStr, biffGet(DIRT));
    usage();
  }
  fclose(file);

  nrrdNuke(info2);
  dirtNixTrnsf(trnsf);
  
  exit(0);
}
