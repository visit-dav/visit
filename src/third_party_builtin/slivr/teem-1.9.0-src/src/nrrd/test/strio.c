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

int
main(int argc, char **argv) {
  char *me, *err;
  Nrrd *nrrd;
  NrrdIoState *nio;
  char hstr[] = "NRRD0001\n"
    "# Complete NRRD file format specification at:\n"
    "# http://teem.sourceforge.net/nrrd/format.html\n"
    "# one comment\n"
    "# two comment\n"
    "# three comment\n"
    "type: float\n"
    "dimension: 2\n"
    "sizes: 91 114\n"
    "centerings: node node\n"
    "endian: big\n"
    "encoding: raw\n"
    "bingo:=bob\n"
    "foo:=super duper fancy bar with corona\n"
    /* "data file: tmp.raw\n" */;
  char *wstr;
  AIR_UNUSED(argc);
  me = argv[0];

  nrrdStateVerboseIO = 10;

  nio = nrrdIoStateNew();
  nrrd = nrrdNew();

  nio->path = airStrdup(".");
  if (nrrdStringRead(nrrd, hstr, nio)) {
    fprintf(stderr, "%s: trouble reading string:\n%s", 
            me, err = biffGet(NRRD));
    free(err);
    exit(1);
  }
  fprintf(stderr, "%s: nrrd->data = %p\n", me, nrrd->data);

  nrrdSave("out.nrrd", nrrd, NULL);
  if (nrrdStringWrite(&wstr, nrrd, NULL)) {
    fprintf(stderr, "%s: trouble writing string:\n%s", 
            me, err = biffGet(NRRD));
    free(err);
    exit(1);
  }
  fprintf(stderr, "%s: |%s|\n", me, wstr);

  free(wstr);
  nrrdIoStateNix(nio);
  nrrdNuke(nrrd);

  exit(0);
}
