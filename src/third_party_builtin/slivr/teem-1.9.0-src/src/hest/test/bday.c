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

#include "../hest.h"

int
runexp(int *year, int N, int M) {
  int i, m;

  memset(year, 0, N*sizeof(int));
  for (m=0; m<M; m++) {
    year[airRandInt(N)]++;
  }
  for (i=0; i<N; i++) {
    if (year[i] > 1) {
      return 1;
    }
  }
  return 0;
}

char *info = ("simulates M people in a room finding out if two or more "
              "of them share a birthday.  For fun, can vary the number "
              "of days in the year.");

int
main(int argc, char *argv[]) {
  airArray *mop;
  hestOpt *hopt=NULL;
  int i, N, M, P, yes, *year;
  unsigned int E;
  char *me;
  double crct;
  
  me = argv[0];
  mop = airMopNew();
  hestOptAdd(&hopt, "N", "days", airTypeInt, 1, 1, &N, "365",
             "# of days in year");
  /* E != P */
  hestOptAdd(&hopt, "E", "exps", airTypeInt, 1, 1, &P, "100000",
             "number of experiments after which to print out newly "
             "computed probability");
  hestOptAdd(&hopt, NULL, "people", airTypeInt, 1, 1, &M, NULL,
             "# of people in room");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  if (!( N > 1 && M > 0 && P > 1)) {
    fprintf(stderr, "%s: need both M, P all > 1, M > 0\n", me);
    airMopError(mop); exit(1);
  }
  if (!(year = (int*)calloc(N, sizeof(int)))) {
    fprintf(stderr, "%s: couldn't calloc(%d,sizeof(int))\n", me, N); 
    airMopError(mop); exit(1);
  }
  airMopMem(mop, year, airMopAlways);
  
  crct = 1;
  for (i=0; i<M; i++) {
    crct *= (double)(N-i)/N;
  }
  crct = 1-crct;
  yes = 0;
  E = 1;
  airSrandMT((int)airTime());
  while (E) {
    yes += runexp(year, N, M);
    if (!(E % P)) {
      printf("P = %10d/%10d = %22.20f =?= %22.20f\n",
             yes, E, (double)yes/E, crct);
    }
    E++;
  }
  
  airMopOkay(mop);
  exit(0);
}
