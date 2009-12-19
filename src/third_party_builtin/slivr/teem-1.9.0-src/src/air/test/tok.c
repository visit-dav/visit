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

int
main(int argc, char *argv[]) {
  char *s, *ct, *last, *ret;

  if (3 != argc) {
    /*             0   1    2    (3) */
    printf("usage: %s <s> <ct>\n", argv[0]);
    exit(1);
  }
  s = argv[1];
  ct = argv[2];
  printf("s = |%s|\n", s);
  printf("ct = |%s|\n", ct);
  ret = airStrtok(s, ct, &last);
  while (ret) {
    printf("|%s|\n", ret);
    ret = airStrtok(NULL, ct, &last);
  }
  printf("--------------\n");
  printf("hey, why doesn't this work?!?!?\n");
  printf("s = |%s|\n", s);
  ret = strtok(s, ct);
  while (ret) {
    printf("ret = |%s|\n", ret);
    ret = strtok(NULL, ct);
  }
  exit(0);
}
