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


#include "../air.h"

void *
print(void *_iP) {
  int *iP;
  
  iP = (int *)_iP;
  printf("%d\n", *iP);
  return NULL;
}

int
main(int argc, char *argv[]) {
  void *ptr;
  int i = 111, j = 222, k = 333, l = 444;
  airArray *mop;
  char *str, *me;

  AIR_UNUSED(argc);
  me = argv[0];
  printf("%s: -------------------------------------\n", me);

  mop = airMopNew();
  str = airStrdup("this is just a test");
  printf("%s: str = \"%s\", str = 0x%p, &str = 0x%p\n", me,
         str, str, AIR_CAST(void*, &str));
  airMopMem(mop, &str, airMopAlways);
  airMopDebug(mop);
  airMopOkay(mop);
  

  printf("%s: -------------------------------------\n", me);

  mop = airMopNew();
  ptr = calloc(1024, sizeof(char));
  airMopMem(mop, &ptr, airMopAlways);
  airMopAdd(mop, &i, print, airMopNever);
  airMopAdd(mop, &j, print, airMopOnError);
  airMopAdd(mop, &k, print, airMopOnOkay);
  airMopAdd(mop, &l, print, airMopAlways);
  airMopPrint(mop, "this is a joke", airMopOnError);

  airMopDebug(mop);
  airMopError(mop);

  printf("%s: -------------------------------------\n", me);

  mop = airMopNew();
  ptr = calloc(1024, sizeof(char));
  airMopMem(mop, &ptr, airMopAlways);
  airMopAdd(mop, &i, print, airMopNever);
  airMopAdd(mop, &j, print, airMopOnError);
  airMopAdd(mop, &k, print, airMopOnOkay);
  airMopPrint(mop, "this is a joke", airMopOnError);
  airMopAdd(mop, &l, print, airMopAlways);

  airMopDebug(mop);
  airMopOkay(mop);

  printf("%s: -------------------------------------\n", me);

  mop = airMopNew();
  ptr = calloc(1024, sizeof(char));
  airMopMem(mop, &ptr, airMopAlways);
  free(ptr);
  airMopUnMem(mop, &ptr);
  

  airMopDebug(mop);
  airMopOkay(mop);

  printf("%s: -------------------------------------\n", me);

  exit(0);
}
