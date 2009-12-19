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

char *info = ("Save a single ellipsoid or superquadric into an OFF file.");

int
soidDoit(limnObject *obj, int look,
         int sphere, float gamma, int res,
         float AB[2], float ten[7]) {
  int partIdx, axis;
  float cl, cp, qA, qB, eval[3], evec[9], matA[16], matB[16];

  if (AB) {
    qA = AB[0];
    qB = AB[1];
    axis = 2;
  } else {
    tenEigensolve_f(eval, evec, ten);
    ELL_SORT3(eval[0], eval[1], eval[2], cl);
    cl = (eval[0] - eval[1])/(eval[0] + eval[1] + eval[2]);
    cp = 2*(eval[1] - eval[2])/(eval[0] + eval[1] + eval[2]);
    if (cl > cp) {
      axis = 0;
      qA = pow(1-cp, gamma);
      qB = pow(1-cl, gamma);
    } else {
      axis = 2;
      qA = pow(1-cl, gamma);
      qB = pow(1-cp, gamma);
    }

    fprintf(stderr, "eval = %g %g %g -> cl=%g %s cp=%g -> axis = %d\n",
            eval[0], eval[1], eval[2], cl, cl > cp ? ">" : "<", cp, axis);

  }

  if (sphere) {
    partIdx = limnObjectPolarSphereAdd(obj, look,
                                       0, 2*res, res);
  } else {
    partIdx = limnObjectPolarSuperquadAdd(obj, look,
                                          axis, qA, qB, 2*res, res);
  }
  ELL_4M_IDENTITY_SET(matA);
  ELL_4V_SET(matB + 0*4, eval[0],       0,       0, 0);
  ELL_4V_SET(matB + 1*4,       0, eval[1],       0, 0);
  ELL_4V_SET(matB + 2*4,       0,       0, eval[2], 0);
  ELL_4V_SET(matB + 3*4,       0,       0,       0, 1);
  ELL_4M_SCALE_SET(matB, eval[0], eval[1], eval[2]);
  ell_4m_post_mul_f(matA, matB);
  ELL_4V_SET(matB + 0*4, evec[0 + 0*3], evec[0 + 1*3], evec[0 + 2*3], 0);
  ELL_4V_SET(matB + 1*4, evec[1 + 0*3], evec[1 + 1*3], evec[1 + 2*3], 0);
  ELL_4V_SET(matB + 2*4, evec[2 + 0*3], evec[2 + 1*3], evec[2 + 2*3], 0);
  ELL_4V_SET(matB + 3*4,             0,             0,             0, 1);
  ell_4m_post_mul_f(matA, matB);
  limnObjectPartTransform(obj, partIdx, matA);

  return partIdx;
}

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  double scale[3], matA[9], matB[9], matC[9], sval[3], uu[9], vv[9];
  float matAf[9], matBf[9];
  float p[3], q[4], mR[9], len, gamma;
  float os, vs, rad, AB[2], ten[7], view[3];
  hestOpt *hopt=NULL;
  airArray *mop;
  limnObject *obj;
  limnLook *look; int lookRod, lookSoid;
  int partIdx=-1; /* sssh */
  int res, sphere;
  FILE *file;

  me = argv[0];
  hestOptAdd(&hopt, "sc", "scalings", airTypeDouble, 3, 3, scale, "1 1 1",
             "axis-aligned scaling to do on ellipsoid");
  hestOptAdd(&hopt, "AB", "A, B exponents", airTypeFloat, 2, 2, AB, "nan nan",
             "Directly set the A, B parameters to the superquadric surface, "
             "over-riding the default behavior of determining them from the "
             "scalings \"-sc\" as superquadric tensor glyphs");
  hestOptAdd(&hopt, "os", "over-all scaling", airTypeFloat, 1, 1, &os, "1",
             "over-all scaling (multiplied by scalings)");
  hestOptAdd(&hopt, "vs", "over-all scaling", airTypeFloat, 1, 1, &vs, "1",
             "scaling along view-direction (to show off bas-relief "
             "ambibuity of ellipsoids versus superquads)");
  hestOptAdd(&hopt, "fr", "from (eye) point", airTypeFloat, 3, 3, &view,
             "4 4 4", "eye point, needed for non-unity \"-vs\"");
  hestOptAdd(&hopt, "gamma", "superquad sharpness", airTypeFloat, 1, 1,
             &gamma, "0",
             "how much to sharpen edges as a "
             "function of differences between eigenvalues");
  hestOptAdd(&hopt, "sphere", NULL, airTypeInt, 0, 0, &sphere, NULL,
             "use a sphere instead of a superquadric");
  hestOptAdd(&hopt, "p", "x y z", airTypeFloat, 3, 3, p, "0 0 0",
             "location in quaternion quotient space");
  hestOptAdd(&hopt, "r", "radius", airTypeFloat, 1, 1, &rad, "0.015",
             "black axis cylinder radius (or 0.0 to not drawn these)");
  hestOptAdd(&hopt, "res", "resolution", airTypeInt, 1, 1, &res, "25",
             "tesselation resolution for both glyph and axis cylinders");
  hestOptAdd(&hopt, "o", "output OFF", airTypeString, 1, 1, &outS, "out.off",
             "output file to save OFF into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  obj = limnObjectNew(1000, AIR_TRUE);
  airMopAdd(mop, obj, (airMopper)limnObjectNix, airMopAlways);

  /* create limnLooks for ellipsoid and for rods */
  lookSoid = limnObjectLookAdd(obj);
  look = obj->look + lookSoid;
  ELL_4V_SET(look->rgba, 1, 1, 1, 1);
  ELL_3V_SET(look->kads, 0.2, 0.8, 0);
  look->spow = 0;
  lookRod = limnObjectLookAdd(obj);
  look = obj->look + lookRod;
  ELL_4V_SET(look->rgba, 0, 0, 0, 1);
  ELL_3V_SET(look->kads, 1, 0, 0);
  look->spow = 0;

  ELL_3M_IDENTITY_SET(matA);
  ELL_3V_SCALE(scale, os, scale);
  ELL_3M_SCALE_SET(matB, scale[0], scale[1], scale[2]);
  ell_3m_post_mul_d(matA, matB);
  if (1 != vs) {
    ELL_3V_NORM(view, view, len);
    if (!len) {
      /* HEY: perhaps do more diplomatic error message here */
      fprintf(stderr, "%s: stupido!\n", me);
      exit(1);
    }
    ELL_3MV_OUTER(matB, view, view);
    ELL_3M_SCALE(matB, vs-1, matB);
    ELL_3M_IDENTITY_SET(matC);
    ELL_3M_ADD2(matB, matC, matB);
    ell_3m_post_mul_d(matA, matB);
  }
  ell_3m_svd_d(uu, sval, vv, matA, AIR_TRUE);

  /*
  fprintf(stderr, "%s: ____________________________________\n", me);
  fprintf(stderr, "%s: mat = \n", me);
  ell_3m_print_d(stderr, matA);
  fprintf(stderr, "%s: uu = \n", me);
  ell_3m_print_d(stderr, uu);
  ELL_3M_TRANSPOSE(matC, uu);
  ELL_3M_MUL(matB, uu, matC);
  fprintf(stderr, "%s: uu * uu^T = \n", me);
  ell_3m_print_d(stderr, matB);
  fprintf(stderr, "%s: sval = %g %g %g\n", me, sval[0], sval[1], sval[2]);
  fprintf(stderr, "%s: vv = \n", me);
  ell_3m_print_d(stderr, vv);
  ELL_3M_MUL(matB, vv, vv);
  fprintf(stderr, "%s: vv * vv^T = \n", me);
  ELL_3M_TRANSPOSE(matC, vv);
  ELL_3M_MUL(matB, vv, matC);
  ell_3m_print_d(stderr, matB);
  ELL_3M_IDENTITY_SET(matA);
  ell_3m_pre_mul_d(matA, uu);
  ELL_3M_SCALE_SET(matB, sval[0], sval[1], sval[2]);
  ell_3m_pre_mul_d(matA, matB);
  ell_3m_pre_mul_d(matA, vv);
  fprintf(stderr, "%s: uu * diag(sval) * vv = \n", me);
  ell_3m_print_d(stderr, matA);
  fprintf(stderr, "%s: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n", me);
  */
  
  ELL_3M_IDENTITY_SET(matA);
  ell_3m_pre_mul_d(matA, uu);  
  ELL_3M_SCALE_SET(matB, sval[0], sval[1], sval[2]);
  ell_3m_pre_mul_d(matA, matB);  
  ELL_3M_TRANSPOSE(matB, uu);
  ell_3m_pre_mul_d(matA, matB);
  TEN_M2T(ten, matA);

  partIdx = soidDoit(obj, lookSoid,
                     sphere, gamma, res,
                     (AIR_EXISTS(AB[0]) && AIR_EXISTS(AB[1])) ? AB : NULL,
                     ten);

  ELL_4V_SET(q, 1, p[0], p[1], p[2]);
  ELL_4V_NORM(q, q, len);
  ell_q_to_3m_f(mR, q);
  ELL_43M_INSET(matBf, mR);
  limnObjectPartTransform(obj, partIdx, matBf);

  if (rad) {
    partIdx = limnObjectCylinderAdd(obj, lookRod, 0, res);
    ELL_4M_IDENTITY_SET(matAf);
    ELL_4M_SCALE_SET(matBf, (1-scale[0])/2, rad, rad);
    ell_4m_post_mul_f(matAf, matBf);
    ELL_4M_TRANSLATE_SET(matBf, (1+scale[0])/2, 0.0, 0.0); 
    ell_4m_post_mul_f(matAf, matBf);
    limnObjectPartTransform(obj, partIdx, matAf);
    
    partIdx = limnObjectCylinderAdd(obj, lookRod, 0, res);
    ELL_4M_IDENTITY_SET(matAf);
    ELL_4M_SCALE_SET(matBf, (1-scale[0])/2, rad, rad);
    ell_4m_post_mul_f(matAf, matBf);
    ELL_4M_TRANSLATE_SET(matBf, -(1+scale[0])/2, 0.0, 0.0); 
    ell_4m_post_mul_f(matAf, matBf);
    limnObjectPartTransform(obj, partIdx, matAf);
    
    partIdx = limnObjectCylinderAdd(obj, lookRod, 1, res);
    ELL_4M_IDENTITY_SET(matAf);
    ELL_4M_SCALE_SET(matBf, rad, (1-scale[1])/2, rad);
    ell_4m_post_mul_f(matAf, matBf);
    ELL_4M_TRANSLATE_SET(matBf, 0.0, (1+scale[1])/2, 0.0); 
    ell_4m_post_mul_f(matAf, matBf);
    limnObjectPartTransform(obj, partIdx, matAf);
    
    partIdx = limnObjectCylinderAdd(obj, lookRod, 1, res);
    ELL_4M_IDENTITY_SET(matAf);
    ELL_4M_SCALE_SET(matBf, rad, (1-scale[1])/2, rad);
    ell_4m_post_mul_f(matAf, matBf);
    ELL_4M_TRANSLATE_SET(matBf, 0.0, -(1+scale[1])/2, 0.0); 
    ell_4m_post_mul_f(matAf, matBf);
    limnObjectPartTransform(obj, partIdx, matAf);
    
    partIdx = limnObjectCylinderAdd(obj, lookRod, 2, res);
    ELL_4M_IDENTITY_SET(matAf);
    ELL_4M_SCALE_SET(matBf, rad, rad, (1-scale[2])/2);
    ell_4m_post_mul_f(matAf, matBf);
    ELL_4M_TRANSLATE_SET(matBf, 0.0, 0.0, (1+scale[2])/2); 
    ell_4m_post_mul_f(matAf, matBf);
    limnObjectPartTransform(obj, partIdx, matAf);
    
    partIdx = limnObjectCylinderAdd(obj, lookRod, 2, res);
    ELL_4M_IDENTITY_SET(matAf);
    ELL_4M_SCALE_SET(matBf, rad, rad, (1-scale[2])/2);
    ell_4m_post_mul_f(matAf, matBf);
    ELL_4M_TRANSLATE_SET(matBf, 0.0, 0.0, -(1+scale[2])/2); 
    ell_4m_post_mul_f(matAf, matBf);
    limnObjectPartTransform(obj, partIdx, matAf);
  }

  file = airFopen(outS, stdout, "w");
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);

  if (limnObjectOFFWrite(file, obj)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}

