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
rangeTest(char *me, double imin, double imax) {
  double omin, omax;
  baneRange *range;
  int i;

  printf("input range (%g,%g) ---------------------\n", imin, imax);
  for (i=1; i<baneRangeLast; i++) {
    range = baneRangeNew(i);
    range->answer(&omin, &omax, imin, imax);
    printf("%s: range %s --> (%g,%g)\n",
           me, range->name, omin, omax);
    range = baneRangeNix(range);
  }
}

void
incTest(char *me, int num, baneRange *range) {
  double *val, tmp, incParm[BANE_PARM_NUM], omin, omax, rmin, rmax;
  baneInc *inc;
  Nrrd *hist;
  int i, j;

  airSrand48();

  val = (double*)malloc(num*sizeof(double));
  /* from <http://www.itl.nist.gov/div898/handbook/index.htm>:
     the standard dev of a uniform distribution between A and B is
     sqrt((B-A)^2/12) */
  for (j=0; j<num; j++) {
    tmp = AIR_AFFINE(0.0, airDrand48(), 1.0, -1.0, 1.0);
    /* val[j] = tmp*tmp*tmp; */
    val[j] = tmp;
  }
  rmin = rmax = val[0];
  for (j=0; j<num; j++) {
    rmin = AIR_MIN(rmin, val[j]);
    rmax = AIR_MAX(rmax, val[j]);
  }
  fprintf(stderr, "incTest: real min,max = %g,%g\n", rmin, rmax);
  
  for (i=1; i<baneIncLast; i++) {
    /* NOTE: THIS IS BROKEN !!! */
    inc = baneIncNew(i, NULL, incParm);
    printf("%s: inclusion %s ------\n", me, inc->name);
    switch(i) {
    case baneIncAbsolute:
      ELL_3V_SET(incParm, -0.8, 1.5, AIR_NAN);
      break;
    case baneIncRangeRatio:
      ELL_3V_SET(incParm, 0.99, AIR_NAN, AIR_NAN);
      break;
    case baneIncPercentile:
      ELL_3V_SET(incParm, 1024, 10, AIR_NAN);
      break;
    case baneIncStdv:
      ELL_3V_SET(incParm, 1.0, AIR_NAN, AIR_NAN);
      break;
    }
    fprintf(stderr, "!%s: THIS IS BROKEN!!!\n", "incTest");
    /*
    if (inc->passA) {
      for (j=0; j<num; j++)
        inc->process[0](hist, val[j], incParm);
    }
    if (inc->passB) {
      for (j=0; j<num; j++)
        inc->process[1](hist, val[j], incParm);
    }
    inc->ans(&omin, &omax, hist, incParm, range);
    */
    printf(" --> (%g,%g)\n", omin, omax);
  }

  free(val);
}

int
main(int argc, char *argv[]) {
  me = argv[0];

  printf("%s ================================ range testing\n", me);
  rangeTest(me, -1, 2);
  rangeTest(me, -3, 2);

  printf("%s ================================ inclusion testing\n", me);
  incTest(me, 10000, baneRangeFloat);

  return 0;
}
