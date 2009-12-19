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
_ra2t(Nrrd *nten, double rad, double angle, 
      double mRI[9], double mRF[9], double hack) {
  double x, y, xyz[3], XX[3], YY[3], CC[3], EE[3], VV[3], tmp, mD[9], mT[9];
  float *tdata;
  int xi, yi, sx, sy;

  sx = nten->axis[1].size;
  sy = nten->axis[2].size;
  x = rad*sin(AIR_PI*angle/180);
  y = rad*cos(AIR_PI*angle/180);
  xi = airIndexClamp(0.0, x, sqrt(3.0)/2.0, sx);
  yi = airIndexClamp(0.0, y, 0.5, sy);
  ELL_3V_SET(VV, 0, 3, 0);
  ELL_3V_SET(EE, 1.5, 1.5, 0);
  ELL_3V_SET(CC, 1, 1, 1);
  ELL_3V_SUB(YY, EE, CC);
  ELL_3V_SUB(XX, VV, EE);
  ELL_3V_NORM(XX, XX, tmp);
  ELL_3V_NORM(YY, YY, tmp);
  ELL_3V_SCALE_ADD3(xyz, 1.0, CC, hack*x, XX, hack*y, YY);
  
  ELL_3M_IDENTITY_SET(mD);
  ELL_3M_DIAG_SET(mD, xyz[0], xyz[1], xyz[2]);
  ELL_3M_IDENTITY_SET(mT);
  ell_3m_post_mul_d(mT, mRI);
  ell_3m_post_mul_d(mT, mD);
  ell_3m_post_mul_d(mT, mRF);
  tdata = (float*)(nten->data) + 7*(xi + sx*(yi + 1*sy));
  tdata[0] = 1.0;
  TEN_M2T(tdata, mT);
}

void
_cap2xyz(double xyz[3], double ca, double cp, int version, int whole) {
  double cl, cs, mean;

  cs = 1 - ca;
  cl = 1 - cs - cp;
  mean = (cs + cp + cl)/3;
  /*
    xyz[0] = cs*0.333 + cl*1.0 + cp*0.5;
    xyz[1] = cs*0.333 + cl*0.0 + cp*0.5;
    xyz[2] = cs*0.333 + cl*0.0 + cp*0.0;
    xyz[0] = AIR_AFFINE(0, ca, 1, 1.1*xyz[0], 0.86*xyz[0]);
    xyz[1] = AIR_AFFINE(0, ca, 1, 1.1*xyz[1], 0.86*xyz[1]);
    xyz[2] = AIR_AFFINE(0, ca, 1, 1.1*xyz[2], 0.86*xyz[2]);
  */
  if (whole) {
    ELL_3V_SET(xyz,
               AIR_AFFINE(0.0, 0.9, 1.0, mean, cl),
               AIR_AFFINE(0.0, 0.9, 1.0, mean, cp),
               AIR_AFFINE(0.0, 0.9, 1.0, mean, cs));
    ELL_3V_SET(xyz, cl, cp, cs);
  } else {
    if (1 == version) {
      ELL_3V_SET(xyz,
                 (3 + 3*cl - cs)/6,
                 (2 - 2*cl + cp)/6,
                 2*cs/6);
    } else {
      ELL_3V_SET(xyz, 1, 1 - cl, cs);
    }
  }
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
  
  int sx, sy, xi, yi, samp, version, whole, right;
  float *tdata;
  double p[3], xyz[3], q[4], len, hackcp=0, maxca;
  double ca, cp, mD[9], mRF[9], mRI[9], mT[9], hack;
  Nrrd *nten;
  mop = airMopNew();
  
  me = argv[0];
  hestOptAdd(&hopt, "n", "# samples", airTypeInt, 1, 1, &samp, "4",
             "number of glyphs along each edge of triangle");
  hestOptAdd(&hopt, "p", "x y z", airTypeDouble, 3, 3, p, NULL,
             "location in quaternion quotient space");
  hestOptAdd(&hopt, "ca", "max ca", airTypeDouble, 1, 1, &maxca, "0.8",
             "maximum ca to use at bottom edge of triangle");
  hestOptAdd(&hopt, "r", NULL, airTypeInt, 0, 0, &right, NULL,
             "sample a right-triangle-shaped region, instead of "
             "a roughly equilateral triangle. ");
  hestOptAdd(&hopt, "w", NULL, airTypeInt, 0, 0, &whole, NULL,
             "sample the whole triangle of constant trace, "
             "instead of just the "
             "sixth of it in which the eigenvalues have the "
             "traditional sorted order. ");
  hestOptAdd(&hopt, "hack", "hack", airTypeDouble, 1, 1, &hack, "0.04",
             "this is a hack");
  hestOptAdd(&hopt, "v", "version", airTypeInt, 1, 1, &version, "1",
             "which version of the Westin metrics to use to parameterize "
             "triangle; \"1\" for ISMRM 97, \"2\" for MICCAI 99");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output file to save tensors into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  nten = nrrdNew();
  airMopAdd(mop, nten, (airMopper)nrrdNuke, airMopAlways);

  if (!( 1 == version || 2 == version )) {
    fprintf(stderr, "%s: version must be 1 or 2 (not %d)\n", me, version);
    airMopError(mop); 
    return 1;
  }
  if (right) {
    sx = samp;
    sy = (int)(1.0*samp/sqrt(3.0));
  } else {
    sx = 2*samp-1;
    sy = samp;
  }
  if (nrrdMaybeAlloc_va(nten, nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 7),
                        AIR_CAST(size_t, sx),
                        AIR_CAST(size_t, sy),
                        AIR_CAST(size_t, 3))) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  q[0] = 1.0;
  q[1] = p[0];
  q[2] = p[1];
  q[3] = p[2];
  len = ELL_4V_LEN(q);
  ELL_4V_SCALE(q, 1.0/len, q);
  washQtoM3(mRF, q);
  ELL_3M_TRANSPOSE(mRI, mRF);
  if (right) {
    _ra2t(nten, 0.00, 0.0, mRI, mRF, hack);

    _ra2t(nten, 0.10, 0.0, mRI, mRF, hack);
    _ra2t(nten, 0.10, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.20, 0.0, mRI, mRF, hack);
    _ra2t(nten, 0.20, 30.0, mRI, mRF, hack);
    _ra2t(nten, 0.20, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.30, 0.0, mRI, mRF, hack);
    _ra2t(nten, 0.30, 20.0, mRI, mRF, hack);
    _ra2t(nten, 0.30, 40.0, mRI, mRF, hack);
    _ra2t(nten, 0.30, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.40, 0.0, mRI, mRF, hack);
    _ra2t(nten, 0.40, 15.0, mRI, mRF, hack);
    _ra2t(nten, 0.40, 30.0, mRI, mRF, hack);
    _ra2t(nten, 0.40, 45.0, mRI, mRF, hack);
    _ra2t(nten, 0.40, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.50, 0.0, mRI, mRF, hack);
    _ra2t(nten, 0.50, 12.0, mRI, mRF, hack);
    _ra2t(nten, 0.50, 24.0, mRI, mRF, hack);
    _ra2t(nten, 0.50, 36.0, mRI, mRF, hack);
    _ra2t(nten, 0.50, 48.0, mRI, mRF, hack);
    _ra2t(nten, 0.50, 60.0, mRI, mRF, hack);

    /* _ra2t(nten, 0.60, 30.0, mRI, mRF, hack); */
    _ra2t(nten, 0.60, 40.0, mRI, mRF, hack);
    _ra2t(nten, 0.60, 50.0, mRI, mRF, hack);
    _ra2t(nten, 0.60, 60.0, mRI, mRF, hack);

    /* _ra2t(nten, 0.70, 34.3, mRI, mRF, hack); */
    /* _ra2t(nten, 0.70, 42.8, mRI, mRF, hack); */
    _ra2t(nten, 0.70, 51.4, mRI, mRF, hack);
    _ra2t(nten, 0.70, 60.0, mRI, mRF, hack);

    /* _ra2t(nten, 0.80, 45.0, mRI, mRF, hack); */
    _ra2t(nten, 0.80, 52.5, mRI, mRF, hack);
    _ra2t(nten, 0.80, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.90, 60.0, mRI, mRF, hack);

    _ra2t(nten, 1.00, 60.0, mRI, mRF, hack);
    /*
    _ra2t(nten, 0.000, 0.0, mRI, mRF, hack);

    _ra2t(nten, 0.125, 0.0, mRI, mRF, hack);
    _ra2t(nten, 0.125, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.250, 0.0, mRI, mRF, hack);
    _ra2t(nten, 0.250, 30.0, mRI, mRF, hack);
    _ra2t(nten, 0.250, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.375, 0.0, mRI, mRF, hack);
    _ra2t(nten, 0.375, 20.0, mRI, mRF, hack);
    _ra2t(nten, 0.375, 40.0, mRI, mRF, hack);
    _ra2t(nten, 0.375, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.500, 0.0, mRI, mRF, hack);
    _ra2t(nten, 0.500, 15.0, mRI, mRF, hack);
    _ra2t(nten, 0.500, 30.0, mRI, mRF, hack);
    _ra2t(nten, 0.500, 45.0, mRI, mRF, hack);
    _ra2t(nten, 0.500, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.625, 37.0, mRI, mRF, hack);
    _ra2t(nten, 0.625, 47.5, mRI, mRF, hack);
    _ra2t(nten, 0.625, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.750, 49.2, mRI, mRF, hack);
    _ra2t(nten, 0.750, 60.0, mRI, mRF, hack);

    _ra2t(nten, 0.875, 60.0, mRI, mRF, hack);

    _ra2t(nten, 1.000, 60.0, mRI, mRF, hack);
    */
    nten->axis[1].spacing = 1;
    nten->axis[2].spacing = (sx-1)/(sqrt(3.0)*(sy-1));
    nten->axis[3].spacing = 1;
  } else {
    for (yi=0; yi<samp; yi++) {
      if (whole) {
        ca = AIR_AFFINE(0, yi, samp-1, 0.0, 1.0);
      } else {
        ca = AIR_AFFINE(0, yi, samp-1, hack, maxca);
        hackcp = AIR_AFFINE(0, yi, samp-1, hack, 0);
      }
      for (xi=0; xi<=yi; xi++) {
        if (whole) {
          cp = AIR_AFFINE(0, xi, samp-1, 0.0, 1.0);
        } else {
          cp = AIR_AFFINE(0, xi, samp-1, hackcp, maxca-hack/2.0);
        }
        _cap2xyz(xyz, ca, cp, version, whole);
        /*
          fprintf(stderr, "%s: (%d,%d) -> (%g,%g) -> %g %g %g\n", me,
          yi, xi, ca, cp, xyz[0], xyz[1], xyz[2]);
        */
        ELL_3M_IDENTITY_SET(mD);
        ELL_3M_DIAG_SET(mD, xyz[0], xyz[1], xyz[2]);
        ELL_3M_IDENTITY_SET(mT);
        ell_3m_post_mul_d(mT, mRI);
        ell_3m_post_mul_d(mT, mD);
        ell_3m_post_mul_d(mT, mRF);
        
        tdata = (float*)nten->data + 
          7*(2*(samp-1-xi) - (samp-1-yi) + (2*samp-1)*((samp-1-yi) + samp));
        tdata[0] = 1.0;
        TEN_M2T(tdata, mT);
      }
    }
    nten->axis[1].spacing = 1;
    nten->axis[2].spacing = 1.5;
    nten->axis[3].spacing = 1;
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

