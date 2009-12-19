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


#include <stdlib.h>
#include "../nrrd.h"

float frand(float min, float max) {
  return (min + airDrandMT() * (max - min));
}

int
main(int argc, char *argv[]) {
  int i;
  Nrrd *nrrd;
  double diff, idx, idx2, idx3, idx4, lo, hi, pos, pos2, pos3, pos4;

  AIR_UNUSED(argc);
  AIR_UNUSED(argv);
  if (nrrdAlloc_va(nrrd=nrrdNew(), nrrdTypeFloat, 2,
                   AIR_CAST(size_t, 4),
                   AIR_CAST(size_t, 4))) {
    printf("trouble:\n%s\n", biffGet(NRRD));
    exit(1);
  }
  nrrdAxisInfoSet_va(nrrd, nrrdAxisInfoMin, 10.0, 10.0);
  nrrdAxisInfoSet_va(nrrd, nrrdAxisInfoMax, 12.0, 12.0);
  nrrdAxisInfoSet_va(nrrd, nrrdAxisInfoCenter, nrrdCenterNode, nrrdCenterCell);

  idx = 0;
  printf("\n");
  pos = nrrdAxisInfoPos(nrrd, 0, idx);
  printf("pos(0, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 0, pos));
  pos = nrrdAxisInfoPos(nrrd, 1, idx);
  printf("pos(1, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 1, pos));

  idx = 1;
  printf("\n");
  pos = nrrdAxisInfoPos(nrrd, 0, idx);
  printf("pos(0, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 0, pos));
  pos = nrrdAxisInfoPos(nrrd, 1, idx);
  printf("pos(1, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 1, pos));

  idx = 2;
  printf("\n");
  pos = nrrdAxisInfoPos(nrrd, 0, idx);
  printf("pos(0, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 0, pos));
  pos = nrrdAxisInfoPos(nrrd, 1, idx);
  printf("pos(1, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 1, pos));

  idx = 0; idx2 = 0;
  printf("\n");
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 0, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 0, lo, hi);
  printf("range(0, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 1, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 1, lo, hi);
  printf("range(1, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);

  idx = 0; idx2 = 1;
  printf("\n");
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 0, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 0, lo, hi);
  printf("range(0, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 1, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 1, lo, hi);
  printf("range(1, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);

  idx = 1; idx2 = 0;
  printf("\n");
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 0, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 0, lo, hi);
  printf("range(0, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 1, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 1, lo, hi);
  printf("range(1, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);

  nrrdAxisInfoSet_va(nrrd, nrrdAxisInfoMin, 12.0, 12.0);
  nrrdAxisInfoSet_va(nrrd, nrrdAxisInfoMax, 10.0, 10.0);
  printf("\n(axis min,max flipped)\n");

  idx = 0;
  printf("\n");
  pos = nrrdAxisInfoPos(nrrd, 0, idx);
  printf("pos(0, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 0, pos));
  pos = nrrdAxisInfoPos(nrrd, 1, idx);
  printf("pos(1, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 1, pos));

  idx = 1;
  printf("\n");
  pos = nrrdAxisInfoPos(nrrd, 0, idx);
  printf("pos(0, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 0, pos));
  pos = nrrdAxisInfoPos(nrrd, 1, idx);
  printf("pos(1, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 1, pos));

  idx = 2;
  printf("\n");
  pos = nrrdAxisInfoPos(nrrd, 0, idx);
  printf("pos(0, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 0, pos));
  pos = nrrdAxisInfoPos(nrrd, 1, idx);
  printf("pos(1, %g) == %g --> %g\n",
         idx, pos, nrrdAxisInfoIdx(nrrd, 1, pos));

  idx = 0; idx2 = 0;
  printf("\n");
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 0, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 0, lo, hi);
  printf("range(0, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 1, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 1, lo, hi);
  printf("range(1, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);

  idx = 0; idx2 = 2;
  printf("\n");
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 0, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 0, lo, hi);
  printf("range(0, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 1, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 1, lo, hi);
  printf("range(1, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);

  idx = 2; idx2 = 0;
  printf("\n");
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 0, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 0, lo, hi);
  printf("range(0, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);
  nrrdAxisInfoPosRange(&lo, &hi, nrrd, 1, idx, idx2);
  nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 1, lo, hi);
  printf("range(1, %g -- %g) == (%g -- %g) --> (%g -- %g)\n",
         idx, idx2, lo, hi, idx3, idx4);

  nrrd->axis[0].center = nrrdCenterCell;
  nrrd->axis[0].size = 4;
  nrrd->axis[0].min = -4;
  nrrd->axis[0].max = 4;
  pos = 0;
  pos2 = 1;
  nrrdAxisInfoIdxRange(&idx, &idx2, nrrd, 0, pos, pos2);
  nrrdAxisInfoPosRange(&pos3, &pos4, nrrd, 0, idx, idx2);
  printf("min, max = %g, %g\n", nrrd->axis[0].min, nrrd->axis[0].max);
  printf("pos, pos2 = %g, %g\n", pos, pos2);
  printf("idx, idx2 = %g, %g\n", idx, idx2);
  printf("pos3, pos4 = %g, %g\n", pos3, pos4);
  exit(1);

  
  /* and now for random-ness */
  airSrandMT((int)airTime());
  nrrd->axis[0].center = nrrdCenterNode;
  nrrd->axis[0].center = nrrdCenterCell;
  for (i=0; i<=1000000; i++) {
    nrrd->axis[0].min = frand(-3.0, 3.0);
    nrrd->axis[0].max = frand(-3.0, 3.0);
    idx = frand(-3.0, 3.0);
    pos = nrrdAxisInfoPos(nrrd, 0, idx);
    diff = idx - nrrdAxisInfoIdx(nrrd, 0, pos);
    if (AIR_ABS(diff) > 0.00000001) { printf("PANIC 0\n"); exit(2); }
    pos = frand(-3.0, 3.0);
    idx = nrrdAxisInfoIdx(nrrd, 0, pos);
    diff = pos - nrrdAxisInfoPos(nrrd, 0, idx);
    if (AIR_ABS(diff) > 0.00000001) { printf("PANIC 1\n"); exit(2); }
    
    nrrd->axis[0].min = (int)frand(-3.0, 3.0);
    nrrd->axis[0].max = (int)frand(-3.0, 3.0);
    idx = (int)frand(-10.0, 10.0);
    idx2 = (int)frand(-10.0, 10.0);
    nrrdAxisInfoPosRange(&pos, &pos2, nrrd, 0, idx, idx2);
    nrrdAxisInfoIdxRange(&idx3, &idx4, nrrd, 0, pos, pos2);
    diff = AIR_ABS(idx - idx3) + AIR_ABS(idx2 - idx4);
    if (AIR_ABS(diff) > 0.00000001) { printf("PANIC 2\n"); exit(2); }
    pos = (int)frand(-3.0, 3.0);
    pos2 = (int)frand(-3.0, 3.0);
    nrrdAxisInfoIdxRange(&idx, &idx2, nrrd, 0, pos, pos2);
    nrrdAxisInfoPosRange(&pos3, &pos4, nrrd, 0, idx, idx2);
    diff = AIR_ABS(pos - pos3) + AIR_ABS(pos2 - pos4);
    if (AIR_ABS(diff) > 0.00000001) { 
      printf("min, max = %g, %g\n", nrrd->axis[0].min, nrrd->axis[0].max);
      printf("pos, pos2 = %g, %g\n", pos, pos2);
      printf("idx, idx2 = %g, %g\n", idx, idx2);
      printf("pos3, pos4 = %g, %g\n", pos3, pos4);
      printf("PANIC (%d) 3 %g\n", (int)nrrd->axis[0].size, diff); exit(2); 
    }
  }

  exit(0);
}
