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
  char **ptrP;

  ptrP = _ptr;
  *ptrP = airStrdup(str);
  if (!(*ptrP)) {
    sprintf(err, "couldn't strdup() str");
    return 1;
  }
  airToUpper(*ptrP);
  return 0;
}

hestCB cbinfo = {
  sizeof(char*),
  "token",
  parse,
  airFree
};

int
main(int argc, char **argv) {
  char *single, *triple[3], *maybe, **many;
  int howMany, i, N;
  hestOpt *opt = NULL;
  char *err = NULL;

  hestOptAdd(&opt, "A",      "token",           airTypeOther, 1,  1, &single, 
             "alpha",        "testing A",       NULL,  NULL,  &cbinfo);
  hestOptAdd(&opt, "B",      "tok1 tok2 tok3",  airTypeOther, 3,  3, triple,
             "beta psi rho", "testing B",       NULL,  NULL,  &cbinfo);
  hestOptAdd(&opt, "C",      "mtok",            airTypeOther, 0,  1, &maybe,
             "gamma",        "testing C",       NULL,  NULL,  &cbinfo);
  hestOptAdd(&opt, "D",      "tok",             airTypeOther, 1, -1, &many,
             "kappa omega",  "testing D",       &howMany, NULL, &cbinfo);
  hestOptAdd(&opt, "int",    "N",               airTypeInt,   1,  1, &N,
             NULL,           "an integer");
  
  if (hestParse(opt, argc-1, argv+1, &err, NULL)) {
    fprintf(stderr, "ERROR: %s\n", err); free(err);
    hestUsage(stderr, opt, argv[0], NULL);
    hestGlossary(stderr, opt, NULL);
    exit(1);
  }

  printf("single: %s\n", single);
  printf("triple: %s %s %s\n", triple[0], triple[1], triple[2]);
  printf("maybe:  %s\n", maybe);
  printf("many(%d):", howMany);
  for (i=0; i<=howMany-1; i++) {
    printf(" %s", many[i]);
  }
  printf("\n");

  hestParseFree(opt);
  exit(0);
}
