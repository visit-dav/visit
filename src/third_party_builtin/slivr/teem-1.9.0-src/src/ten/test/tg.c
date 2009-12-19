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

char *info = ("Sample space of tensor shape.");

void
_clp2xyz(double xyz[3], double clp[2]) {
  double cl, cp, cs;
  
  cl = clp[0];
  cp = clp[1];
  cs = 1 - cl - cp;
  xyz[0] = cs*0.7 + cl*1.0 + cp*0.75;
  xyz[1] = cs*0.7 + cl*0.0 + cp*0.75;
  xyz[2] = cs*0.7 + cl*0.0 + cp*0.00;
}

void
washQtoM3(double m[9], double q[4]) {
  double p[4], w, x, y, z, len;

  ELL_4V_COPY(p, q);
  len = ELL_4V_LEN(p);
  ELL_4V_SCALE(p, 1.0/len, p);
  w = p[0];
  x = p[1];
  y = p[2];
  z = p[3];
  /* mathematica work implies that we should be 
     setting ROW vectors here */
  ELL_3V_SET(m+0, 
             1 - 2*(y*y + z*z),
             2*(x*y - w*z),
             2*(x*z + w*y));
  ELL_3V_SET(m+3,
             2*(x*y + w*z),
             1 - 2*(x*x + z*z),
             2*(y*z - w*x));
  ELL_3V_SET(m+6,
             2*(x*z - w*y),
             2*(y*z + w*x),
             1 - 2*(x*x + y*y));
}

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  hestOpt *hopt=NULL;
  airArray *mop;
  
  int xi, yi, zi, samp;
  float *tdata;
  double clp[2], xyz[3], q[4], len;
  double mD[9], mRF[9], mRI[9], mT[9];
  Nrrd *nten;
  mop = airMopNew();
  
  me = argv[0];
  hestOptAdd(&hopt, "n", "# samples", airTypeInt, 1, 1, &samp, "4",
             "number of samples along each edge of cube");
  hestOptAdd(&hopt, "c", "cl cp", airTypeDouble, 2, 2, clp, NULL,
             "shape of tensor to use; \"cl\" and \"cp\" are cl1 "
             "and cp1 values, both in [0.0,1.0]");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output file to save tensors into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  nten = nrrdNew();
  airMopAdd(mop, nten, (airMopper)nrrdNuke, airMopAlways);

  _clp2xyz(xyz, clp);
  fprintf(stderr, "%s: want evals = %g %g %g\n", me, xyz[0], xyz[1], xyz[2]);

  if (nrrdMaybeAlloc_va(nten, nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 7),
                        AIR_CAST(size_t, samp),
                        AIR_CAST(size_t, samp),
                        AIR_CAST(size_t, samp))) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  ELL_3M_IDENTITY_SET(mD);
  ELL_3M_DIAG_SET(mD, xyz[0], xyz[1], xyz[2]);
  tdata = (float*)nten->data;
  for (zi=0; zi<samp; zi++) {
    for (yi=0; yi<samp; yi++) {
      for (xi=0; xi<samp; xi++) {
        q[0] = 1.0;
        q[1] = AIR_AFFINE(-0.5, (float)xi, samp-0.5, -1, 1);
        q[2] = AIR_AFFINE(-0.5, (float)yi, samp-0.5, -1, 1);
        q[3] = AIR_AFFINE(-0.5, (float)zi, samp-0.5, -1, 1);
        len = ELL_4V_LEN(q);
        ELL_4V_SCALE(q, 1.0/len, q);
        washQtoM3(mRF, q);
        ELL_3M_TRANSPOSE(mRI, mRF);
        
        ELL_3M_IDENTITY_SET(mT);
        ell_3m_post_mul_d(mT, mRI);
        ell_3m_post_mul_d(mT, mD);
        ell_3m_post_mul_d(mT, mRF);

        tdata[0] = 1.0;
        TEN_M2T(tdata, mT);
        tdata += 7;
      }
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

