/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
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

#include <teem/air.h>
#include <teem32bit.h>

int
main(int argc, char *argv[]) {
  int aret;
  char *me;

  AIR_UNUSED(argc);
  me = argv[0];
  aret = airSanity();
  if (airInsane_not == aret) {
    fprintf(stderr, "%s: air sanity check passed.\n", me);
    fprintf(stderr, "\n");
    fprintf(stderr, "AIR_ENDIAN == %d\n", AIR_ENDIAN);
    fprintf(stderr, "AIR_QNANHIBIT == %d\n", AIR_QNANHIBIT);
    fprintf(stderr, "AIR_DIO == %d\n", AIR_DIO);
    fprintf(stderr, "AIR_32BIT == %d\n", AIR_32BIT);
    fprintf(stderr, "sizeof(size_t) = " _AIR_SIZE_T_CNV 
            "; sizeof(void*) = " _AIR_SIZE_T_CNV "\n",
            sizeof(size_t), sizeof(void*));
    fprintf(stderr, "_AIR_SIZE_T_CNV = \"%s\"\n", _AIR_SIZE_T_CNV);
    return 0;
  }
  /* else */
  fprintf(stderr, "%s: air sanity check FAILED:\n%s\n",
          me, airInsaneErr(aret));
  return 1;
}
