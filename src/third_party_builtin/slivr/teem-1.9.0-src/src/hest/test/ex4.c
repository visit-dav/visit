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
parse(void *_ptr, char *str, char *err) {
  double *ptr;
  int ret;

  ptr = _ptr;
  ret = sscanf(str, "%lf,%lf", ptr + 0, ptr + 1);
  if (2 != ret) {
    sprintf(err, "parsed %d (not 2) doubles", ret);
    return 1;
  }
  return 0;
}

hestCB cbinfo = {
  2*sizeof(double),
  "location",
  parse,
  NULL
};

int
main(int argc, char **argv) {
  double single[2], triple[6], maybe[2], *many;
  int howMany, i, N;
  hestOpt *opt = NULL;
  char *err = NULL;

  hestOptAdd(&opt, "A",     "x,y",               airTypeOther, 1,  1, single, 
             "30,50",       "testing A",         NULL,  NULL,  &cbinfo);
  hestOptAdd(&opt, "B",     "x1,y1 x2,y2 x3,y3", airTypeOther, 3,  3, triple,
             "1,2 3,4 5,6", "testing B",         NULL,  NULL,  &cbinfo);
  hestOptAdd(&opt, "C",     "mx,my",             airTypeOther, 0,  1, maybe,
             "-0.1,-0.2",   "testing C. The utility of this can be better "
             "demonstrated in the following manner:\n "
             "- wash the dishes\n "
             "- put the dishes in the cupboard\n "
             "- watch football on TV\n "
             "- remember to walk the dog",
             NULL,  NULL,   &cbinfo);
  hestOptAdd(&opt, "D",     "nx,ny",             airTypeOther, 1, -1, &many,
             "8,8 7,7",     "testing D",         &howMany,     NULL, &cbinfo);
  hestOptAdd(&opt, "int",    "N",                airTypeInt,   1,  1, &N,
             NULL,           "an integer");
  
  if (hestParse(opt, argc-1, argv+1, &err, NULL)) {
    fprintf(stderr, "ERROR: %s\n", err); free(err);
    hestUsage(stderr, opt, argv[0], NULL);
    hestGlossary(stderr, opt, NULL);
    exit(1);
  }

  printf("single: (%g,%g)\n", single[0], single[1]);
  printf("triple: (%g,%g) (%g,%g) (%g,%g)\n", triple[0], triple[1],
         triple[2], triple[3], triple[4], triple[5]);
  printf("maybe: (%g,%g)\n", maybe[0], maybe[1]);
  printf("many(%d):", howMany);
  for (i=0; i<=howMany-1; i++) {
    printf(" (%g,%g)", many[0 + 2*i], many[1 + 2*i]);
  }
  printf("\n");

  hestParseFree(opt);
  exit(0);
}
