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


#include "../limn.h"

char *info = ("test stupid cubes isosurfaces.");

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  hestOpt *hopt=NULL;
  airArray *mop;
  limnObject *obj;
  Nrrd *nin;
  double isoval;
  limnContour3DContext *lctx;
  FILE *file;
  
  me = argv[0];
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume to surface",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "v", "iso", airTypeDouble, 1, 1, &isoval, NULL,
             "isovalue");
  hestOptAdd(&hopt, "o", "output OFF", airTypeString, 1, 1, &outS, "out.off",
             "output file to save OFF into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  obj = limnObjectNew(2000, AIR_FALSE);
  airMopAdd(mop, obj, (airMopper)limnObjectNix, airMopAlways);

  file = airFopen(outS, stdout, "w");
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);

  lctx = limnContour3DContextNew();
  airMopAdd(mop, lctx, (airMopper)limnContour3DContextNix, airMopAlways);

  if (limnContour3DVolumeSet(lctx, nin)
      || limnContour3DExtract(lctx, obj, isoval)
      || limnObjectOFFWrite(file, obj)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  fprintf(stderr, "%s: extraction time = %g\n", me, lctx->time);
  
  airMopOkay(mop);
  return 0;
}
