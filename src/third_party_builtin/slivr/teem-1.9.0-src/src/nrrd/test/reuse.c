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

void
usage(char *me) { 
  /*                       0     1     2   (3) */
  fprintf(stderr, "usage: %s <nin1> <nin2>\n", me);
  exit(1);
}

int
main(int argc, char **argv) {
  char *me, *err;
  Nrrd *nrrd, *n2;
  size_t size[NRRD_DIM_MAX];

  me = argv[0];
  if (3 != argc) {
    usage(me);
  }

  nrrdStateVerboseIO = 10;
  
  if (nrrdLoad(nrrd=nrrdNew(), argv[1], NULL)) {
    fprintf(stderr, "%s: trouble loading \"%s\":\n%s", 
            me, argv[1], err = biffGet(NRRD));
    free(err);
    exit(1);
  }
  fprintf(stderr, "%s: data for \"%s\" at %p\n", me, argv[1], nrrd->data);

  if (nrrdLoad(nrrd, argv[2], NULL)) {
    fprintf(stderr, "%s: trouble loading \"%s\":\n%s", 
            me, argv[2], err = biffGet(NRRD));
    free(err);
    exit(1);
  }
  fprintf(stderr, "%s: data for \"%s\" at %p\n", me, argv[2], nrrd->data);

  n2 = nrrdNew();
  nrrdAxisInfoGet_nva(nrrd, nrrdAxisInfoSize, size);
  if (nrrdWrap_nva(n2, nrrd->data, nrrd->type, nrrd->dim, size)
      || nrrdAxesMerge(n2, nrrd, 0)) {
    fprintf(stderr, "%s: trouble wrapping or merging \"%s\":\n%s", 
            me, argv[2], err = biffGet(NRRD));
    free(err);
    exit(1);
  }
  fprintf(stderr, "%s: data for axmerge(\"%s\",0) at %p\n", 
          me, argv[2], n2->data);
  
  nrrdNuke(nrrd);
  nrrdNix(n2);

  exit(0);
}
