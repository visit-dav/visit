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


#include "bane.h"

/* learned:
** NEVER EVER EVER bypass your own damn pseudo-constructors!
** "npos" used to be a Nrrd (not a pointer), and Joe's 
** trex stuff was crashing because the if data free(data) in nrrd Alloc
** was freeing random stuff, but (and this is the weird part)
** only on some 1-D nrrds of 256 floats (pos1D info), and not others.
*/
Nrrd *baneNpos=NULL;

#define TREX_LUTLEN 256

float _baneTesting[256]={0};

float *
_baneTRexRead(char *fname) {
  char me[]="_baneTRexRead";

  if (nrrdLoad(baneNpos=nrrdNew(), fname, NULL)) {
    fprintf(stderr, "%s: !!! trouble reading \"%s\":\n%s\n", me, 
            fname, biffGet(NRRD));
    return NULL;
  }
  if (banePosCheck(baneNpos, 1)) {
    fprintf(stderr, "%s: !!! didn't get a valid p(x) file:\n%s\n", me, 
            biffGet(BANE));
    return NULL;
  }
  if (TREX_LUTLEN != baneNpos->axis[0].size) {
    fprintf(stderr, "%s: !!! need a length %d p(x) (not " _AIR_SIZE_T_CNV
            ")\n", me, TREX_LUTLEN, baneNpos->axis[0].size); 
    return NULL;
  }

  return (float *)baneNpos->data;
}

void
_baneTRexDone() {

  nrrdNuke(baneNpos); 
}
