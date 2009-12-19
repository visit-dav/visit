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

#include <math.h>
#include "../nrrd.h"

char *quadInfo = ("generates quadratic test volumes, with isosurfaces "
                    "which should resemble z = A*x^2 + B*y^2");

float
quadFunc(float x, float y, float z, float A, float B, float off) {

  return A*x*x + B*y*y - z + off;
}

int
main(int argc, char *argv[]) {
  char *me, *err, *out;
  int size[3], xi, yi, zi;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;
  float min[3], max[3], AB[2], x, y, z, *data, off;
  Nrrd *nout;
  
  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "s", "sx sy sz", airTypeInt, 3, 3, size, "128 128 128",
             "dimensions of output volume");
  hestOptAdd(&hopt, "min", "x y z", airTypeFloat, 3, 3, min, "-1 -1 -1",
             "lower bounding corner of volume");
  hestOptAdd(&hopt, "max", "x y z", airTypeFloat, 3, 3, max, "1 1 1",
             "upper bounding corner of volume");
  hestOptAdd(&hopt, "c", "A B", airTypeFloat, 2, 2, AB, NULL,
             "A and B quadratic coefficients");
  hestOptAdd(&hopt, "off", "z offset", airTypeFloat, 1, 1, &off, "0.0",
             "vertical offset");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &out, "-",
             "file to write output nrrd to");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, quadInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdAlloc_va(nout, nrrdTypeFloat, 3,
                   AIR_CAST(size_t, size[0]),
                   AIR_CAST(size_t, size[1]),
                   AIR_CAST(size_t, size[2]))) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem allocating volume:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  data = (float *)nout->data;
  for (zi=0; zi<size[2]; zi++) {
    z = AIR_AFFINE(0, zi, size[2]-1, min[2], max[2]);
    for (yi=0; yi<size[1]; yi++) {
      y = AIR_AFFINE(0, yi, size[1]-1, min[1], max[1]);
      for (xi=0; xi<size[0]; xi++) {
        x = AIR_AFFINE(0, xi, size[0]-1, min[0], max[0]);
        *data = quadFunc(x,y,z, AB[0], AB[1], off);
        data += 1;
      }
    }
  }

  nrrdAxisInfoSet_va(nout, nrrdAxisInfoMin, min[0], min[1], min[2]);
  nrrdAxisInfoSet_va(nout, nrrdAxisInfoMax, max[0], max[1], max[2]);
  nrrdAxisInfoSpacingSet(nout, 0);
  nrrdAxisInfoSpacingSet(nout, 1);
  nrrdAxisInfoSpacingSet(nout, 2);
  if (nrrdSave(out, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem saving output:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  exit(0);
}
