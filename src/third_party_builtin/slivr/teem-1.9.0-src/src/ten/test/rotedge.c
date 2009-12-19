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


#include "../ten.h"

char *info = 
("Oh, just fricken super! "
 "Another stupid one-off program to make a thesis-related figure. "
 "Reproducibility in visualization, yea, yea fricken great. ");

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  hestOpt *hopt=NULL;
  airArray *mop;
  
  int xi, yi, samp[2];
  float *tdata, mrg, slp;
  double x, xx, y,
    mRot1[9], mRot2[9], mRot3[9],
    mT[9], mR[9], mD[9], mRT[9],
    rot1, rot2, rot3, theta, mean, var, varscl, radius;
  Nrrd *nten;
  mop = airMopNew();
  
  me = argv[0];
  hestOptAdd(&hopt, "nx ny", "# samples", airTypeInt, 2, 2, samp, "90 90",
             "number of samples along each edge of cube");
  hestOptAdd(&hopt, "mrg", "margin", airTypeFloat, 1, 1, &mrg, "0.11",
             "margin above and below anisotropic samples");
  hestOptAdd(&hopt, "slp", "slope", airTypeFloat, 1, 1, &slp, "35",
             "something about boundary between different shapes");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output file to save tensors into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  nten = nrrdNew();
  airMopAdd(mop, nten, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdMaybeAlloc_va(nten, nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 7),
                        AIR_CAST(size_t, samp[0]),
                        AIR_CAST(size_t, samp[1]),
                        AIR_CAST(size_t, 1))) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  
  mean = 0.333333333;
  varscl = 0.045;

  tdata = (float*)nten->data;
  for (yi=0; yi<samp[1]; yi++) {
    y = AIR_AFFINE(0, yi, samp[1]-1, -mrg, 1+mrg);
    theta = AIR_AFFINE(0, AIR_CLAMP(0.007, y, 0.993), 1, AIR_PI/3, 0);
    var = varscl*(airErf(y*slp)+airErf((1-y)*slp))/2;
    radius = sqrt(2*var);
    for (xi=0; xi<samp[0]; xi++) {
      x = AIR_AFFINE(0, xi, samp[0]-1, 0, 3);
      
      ELL_3M_IDENTITY_SET(mD);
      ELL_3M_DIAG_SET(mD,
                      mean + radius*cos(theta),
                      mean + radius*cos(theta - 2*AIR_PI/3),
                      mean + radius*cos(theta + 2*AIR_PI/3));
      rot1 = rot2 = rot3 = 0;
      if (x < 1) {
        xx = AIR_CLAMP(0, x, 1);
        rot1 = AIR_PI*(1-cos(AIR_PI*xx))/2;
      } else if (x < 2) {
        xx = AIR_CLAMP(0, x-1, 1);
        rot2 = AIR_PI*(1-cos(AIR_PI*xx))/2;
      } else {
        xx = AIR_CLAMP(0, x-2, 1);
        rot3 = AIR_PI*(1-cos(AIR_PI*xx))/2;
      }

      /* set mRT, mR */
      ELL_3M_ROTATE_X_SET(mRot1, rot1);
      ELL_3M_ROTATE_Y_SET(mRot2, rot2);
      ELL_3M_ROTATE_Z_SET(mRot3, rot3);
      
      ELL_3M_IDENTITY_SET(mR);
      ell_3m_post_mul_d(mR, mRot1);
      ell_3m_post_mul_d(mR, mRot2);
      ell_3m_post_mul_d(mR, mRot3);
      ELL_3M_TRANSPOSE(mRT, mR);

      ELL_3M_IDENTITY_SET(mT);
      ell_3m_post_mul_d(mT, mRT);
      ell_3m_post_mul_d(mT, mD);
      ell_3m_post_mul_d(mT, mR);
      
      tdata[0] = 1.0;
      TEN_M2T(tdata, mT);
      tdata += 7;
    }
  }
  
  if (nrrdSave(outS, nten, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't save output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  airMopOkay(mop);
  return 0;
}
