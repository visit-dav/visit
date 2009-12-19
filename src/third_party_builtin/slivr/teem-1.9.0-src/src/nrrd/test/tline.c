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

/* learned: C++ name mangling means that you can't simply declare the
   function as extern, you need to do the same extern "C" wrapping as
   is done in the header file
*/
#ifdef __cplusplus
extern "C" {
#endif
extern int _nrrdOneLine(unsigned int *lenP, NrrdIoState *io, FILE *file);
#ifdef __cplusplus
}
#endif

FILE *
myopen(char *name) {

  if (!strcmp(name, "-")) {
    return stdin;
  } else {
    return fopen(name, "r");
  }
}

void
myclose(FILE *file) {

  if (file != stdin) {
    fclose(file);
  }
  return;
}


int
main(int argc, char *argv[]) {
  char *me, *fileS;
  FILE *file;
  unsigned int llen;
  NrrdIoState *io;

  me = argv[0];
  if (2 != argc) {
    /*                       0   1   (2) */
    fprintf(stderr, "usage: %s <file>\n", me);
    exit(1);
  }
  fileS = argv[1];
  if (!( file = myopen(fileS) )) {
    fprintf(stderr, "%s: couldn't open \"%s\" for reading\n", me, fileS);
    exit(1);
  }
  io = nrrdIoStateNew();
  do {
    if (_nrrdOneLine(&llen, io, file)) {
      fprintf(stderr, "%s: trouble:\n%s", me, biffGet(NRRD));
      exit(1);
    }
    if (llen) {
      printf("%2u   |%s|\n", llen, io->line);
    }
  } while(llen > 0);
  nrrdIoStateNix(io);
  myclose(file);

  exit(0);
}
