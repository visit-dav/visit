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


#include "../nrrd.h"

void
usage(char *me) { 
  /*                       0     1     2   (3) */
  fprintf(stderr, "usage: %s <nin> <nout>\n", me);
  exit(1);
}

int
main(int argc, char **argv) {
  char *me, *err;
  Nrrd *nrrd;
  NrrdIoState *io;
  unsigned int domNum, domAxi[NRRD_DIM_MAX], rngNum, rngAxi[NRRD_DIM_MAX], axi;
  double val[NRRD_DIM_MAX][NRRD_SPACE_DIM_MAX];

  me = argv[0];
  if (3 != argc) {
    usage(me);
  }

  io = nrrdIoStateNew();
  nrrdStateVerboseIO = 10;
  
  if (nrrdLoad(nrrd=nrrdNew(), argv[1], NULL)) {
    fprintf(stderr, "%s: trouble loading \"%s\":\n%s", 
            me, argv[1], err = biffGet(NRRD));
    free(err);
    exit(1);
  }

  domNum = nrrdDomainAxesGet(nrrd, domAxi);
  rngNum = nrrdRangeAxesGet(nrrd, rngAxi);
  fprintf(stderr, "%s domain axes (%u):", me, domNum);
  for (axi=0; axi<domNum; axi++) {
    fprintf(stderr, " %u(%s)", domAxi[axi],
            airEnumStr(nrrdKind, nrrd->axis[domAxi[axi]].kind));
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "%s range naxes (%u):", me, rngNum);
  for (axi=0; axi<rngNum; axi++) {
    fprintf(stderr, " %u(%s)", rngAxi[axi],
            airEnumStr(nrrdKind, nrrd->axis[rngAxi[axi]].kind));
  }
  fprintf(stderr, "\n");

  fprintf(stderr, "0 --------------------------------------\n");
  nrrdDescribe(stderr, nrrd);
  fprintf(stderr, "1 --------------------------------------\n");

  nrrd->spaceDim = 3;
  val[0][0] = 1.11;
  val[0][1] = 2.22;
  val[0][2] = 3.33;
  val[1][0] = 4.11;
  val[1][1] = 4.22;
  val[1][2] = 4.33;
  val[2][0] = 5.11;
  val[2][1] = 6.11;
  val[2][2] = 7.11;
  fprintf(stderr, "%s: val[0,1,2] = %lu %lu %lu\n", me,
          (unsigned long)(val[0]),
          (unsigned long)(val[1]),
          (unsigned long)(val[2]));
  nrrdAxisInfoSet_va(nrrd, nrrdAxisInfoSpaceDirection, val[0], val[1], val[2]);
  fprintf(stderr, "2 --------------------------------------\n");
  nrrdAxisInfoGet_nva(nrrd, nrrdAxisInfoSpaceDirection, val);
  fprintf(stderr, "%s: val[0] = %g %g %g\n", me,
          val[0][0], val[0][1], val[0][2]);
  fprintf(stderr, "%s: val[1] = %g %g %g\n", me,
          val[1][0], val[1][1], val[1][2]);
  fprintf(stderr, "%s: val[2] = %g %g %g\n", me,
          val[2][0], val[2][1], val[2][2]);
  fprintf(stderr, "3 --------------------------------------\n");
  nrrdAxisInfoGet_va(nrrd, nrrdAxisInfoSpaceDirection, val[0], val[1], val[2]);
  fprintf(stderr, "4 --------------------------------------\n");
  fprintf(stderr, "%s: val[0] = %g %g %g\n", me,
          val[0][0], val[0][1], val[0][2]);
  fprintf(stderr, "%s: val[1] = %g %g %g\n", me,
          val[1][0], val[1][1], val[1][2]);
  fprintf(stderr, "%s: val[2] = %g %g %g\n", me,
          val[2][0], val[2][1], val[2][2]);
  fprintf(stderr, "5 --------------------------------------\n");

  if (nrrdSave("out.nrrd", nrrd, io)) {
    fprintf(stderr, "%s: trouble saving \"%s\":\n%s", 
            me, argv[1], err = biffGet(NRRD));
    free(err);
    exit(1);
  }
  nrrdIoStateInit(io);
  if (nrrdSave(argv[2], nrrd, io)) {
    fprintf(stderr, "%s: trouble saving \"%s\":\n%s", 
            me, argv[1], err = biffGet(NRRD));
    free(err);
    exit(1);
  }
  nrrdIoStateInit(io);
  if (nrrdSave(argv[2], nrrd, io)) {
    fprintf(stderr, "%s: trouble saving \"%s\":\n%s", 
            me, argv[1], err = biffGet(NRRD));
    free(err);
    exit(1);
  }
  nrrdIoStateInit(io);
  if (nrrdSave(argv[2], nrrd, io)) {
    fprintf(stderr, "%s: trouble saving \"%s\":\n%s", 
            me, argv[1], err = biffGet(NRRD));
    free(err);
    exit(1);
  }

  
  nrrdIoStateNix(io);
  nrrdNuke(nrrd);

  exit(0);
}
