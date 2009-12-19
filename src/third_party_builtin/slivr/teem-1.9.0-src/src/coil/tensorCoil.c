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

#include "coil.h"

void
_coilKind7TensorTangents(coil_t traceGrad[6],
                         coil_t varianceGrad[6],
                         coil_t skewGrad[6],
                         coil_t rot0Grad[6],
                         coil_t rot1Grad[6],
                         coil_t rot2Grad[6],
                         coil_t tensor[7]) {

  AIR_UNUSED(traceGrad);
  AIR_UNUSED(varianceGrad);
  AIR_UNUSED(skewGrad);
  AIR_UNUSED(rot0Grad);
  AIR_UNUSED(rot1Grad);
  AIR_UNUSED(rot2Grad);
  AIR_UNUSED(tensor);
  /*
  coil_t a, b, c, d, e, f;

  a = tensor[1];
  b = tensor[2];
  c = tensor[3];
  d = tensor[4];
  e = tensor[5];
  f = tensor[6];
  ELL_6V_SET(traceGrad, 1, 0, 0, 1, 0, 1);
  */
}

void
_coilKind7TensorFilterTesting(coil_t *delta, coil_t **iv3, 
                              double spacing[3],
                              double parm[COIL_PARMS_NUM]) {

  AIR_UNUSED(iv3);
  AIR_UNUSED(spacing);
  AIR_UNUSED(parm);
  delta[0] = 0;
  delta[1] = 0;
  delta[2] = 0;
  delta[3] = 0;
  delta[4] = 0;
  delta[5] = 0;
  delta[6] = 0;
}

/*
**  o ----> X
**   \   [0][ 0]-[06]  [1][ 0]  [2][ 0]
**  |  \   [0][ 7]-[13]  [1][ 7]  [2][ 7]
**  |    Y   [0][14]-[20]  [1][14]  [2][14]
**  |
**  |    [0][21]-[27]  [1][21]  [2][21]
**  Z      [0][28]-[34]  [1][28]  [2][28]
**           [0][35]-[41]  [1][35]  [2][35]
**  
**       [0][42]-[48]  [1][42]  [2][42]
**         [0][49]-[55]  [1][49]  [2][49]
**           [0][56]-[62]  [1][56]  [2][56]
*/

#define IND(iv3, vi, X, Y, Z) ((iv3)[(X)][(vi) + 7*((Y) + 3*(Z))])

#define TENS(ten, iv3) \
  TEN_T_SET(ten,                                                  \
            IND(iv3,0,1,1,1),                                     \
            IND(iv3,1,1,1,1), IND(iv3,2,1,1,1), IND(iv3,3,1,1,1), \
            IND(iv3,4,1,1,1), IND(iv3,5,1,1,1),                   \
            IND(iv3,6,1,1,1))

#define TENGRAD(tengrad, iv3, rspX, rspY, rspZ) (          \
  TEN_T_SET(tengrad + 0*7,                                 \
            1.0,                                           \
            rspX*(IND(iv3,1,2,1,1) - IND(iv3,1,0,1,1))/2,  \
            rspX*(IND(iv3,2,2,1,1) - IND(iv3,2,0,1,1))/2,  \
            rspX*(IND(iv3,3,2,1,1) - IND(iv3,3,0,1,1))/2,  \
            rspX*(IND(iv3,4,2,1,1) - IND(iv3,4,0,1,1))/2,  \
            rspX*(IND(iv3,5,2,1,1) - IND(iv3,5,0,1,1))/2,  \
            rspX*(IND(iv3,6,2,1,1) - IND(iv3,6,0,1,1))/2), \
  TEN_T_SET(tengrad + 1*7,                                 \
            1.0,                                           \
            rspY*(IND(iv3,1,1,2,1) - IND(iv3,1,1,0,1))/2,  \
            rspY*(IND(iv3,2,1,2,1) - IND(iv3,2,1,0,1))/2,  \
            rspY*(IND(iv3,3,1,2,1) - IND(iv3,3,1,0,1))/2,  \
            rspY*(IND(iv3,4,1,2,1) - IND(iv3,4,1,0,1))/2,  \
            rspY*(IND(iv3,5,1,2,1) - IND(iv3,5,1,0,1))/2,  \
            rspY*(IND(iv3,6,1,2,1) - IND(iv3,6,1,0,1))/2), \
  TEN_T_SET(tengrad + 2*7,                                 \
            1.0,                                           \
            rspZ*(IND(iv3,1,1,1,2) - IND(iv3,1,1,1,0))/2,  \
            rspZ*(IND(iv3,2,1,1,2) - IND(iv3,2,1,1,0))/2,  \
            rspZ*(IND(iv3,3,1,1,2) - IND(iv3,3,1,1,0))/2,  \
            rspZ*(IND(iv3,4,1,1,2) - IND(iv3,4,1,1,0))/2,  \
            rspZ*(IND(iv3,5,1,1,2) - IND(iv3,5,1,1,0))/2,  \
            rspZ*(IND(iv3,6,1,1,2) - IND(iv3,6,1,1,0))/2))

#define LAPL(iv3, vi, rspsqX, rspsqY, rspsqZ) \
  (  rspsqX*(IND(iv3, vi, 0, 1, 1) - 2*IND(iv3, vi, 1, 1, 1) + IND(iv3, vi, 2, 1, 1)) \
   + rspsqY*(IND(iv3, vi, 1, 0, 1) - 2*IND(iv3, vi, 1, 1, 1) + IND(iv3, vi, 1, 2, 1)) \
   + rspsqZ*(IND(iv3, vi, 1, 1, 0) - 2*IND(iv3, vi, 1, 1, 1) + IND(iv3, vi, 1, 1, 2)))

void
_coilKind7TensorFilterHomogeneous(coil_t *delta, coil_t **iv3, 
                                  double spacing[3],
                                  double parm[COIL_PARMS_NUM]) {
  coil_t rspsqX, rspsqY, rspsqZ, parm0;
  
  rspsqX = AIR_CAST(coil_t, 1.0/(spacing[0]*spacing[0]));
  rspsqY = AIR_CAST(coil_t, 1.0/(spacing[1]*spacing[1]));
  rspsqZ = AIR_CAST(coil_t, 1.0/(spacing[2]*spacing[2]));
  parm0 = AIR_CAST(coil_t, parm[0]);
  delta[0] = 0;
  delta[1] = parm0*LAPL(iv3, 1, rspsqX, rspsqY, rspsqZ);
  delta[2] = parm0*LAPL(iv3, 2, rspsqX, rspsqY, rspsqZ);
  delta[3] = parm0*LAPL(iv3, 3, rspsqX, rspsqY, rspsqZ);
  delta[4] = parm0*LAPL(iv3, 4, rspsqX, rspsqY, rspsqZ);
  delta[5] = parm0*LAPL(iv3, 5, rspsqX, rspsqY, rspsqZ);
  delta[6] = parm0*LAPL(iv3, 6, rspsqX, rspsqY, rspsqZ);
}

#define HESS(hess, iv3, vi, rspX, rspY, rspZ) \
  (hess)[1] = rspX*rspX*(IND(iv3,vi,0,1,1) - 2*IND(iv3,vi,1,1,1) + IND(iv3,vi,2,1,1)); \
  (hess)[4] = rspY*rspY*(IND(iv3,vi,1,0,1) - 2*IND(iv3,vi,1,1,1) + IND(iv3,vi,1,2,1)); \
  (hess)[6] = rspZ*rspZ*(IND(iv3,vi,1,1,0) - 2*IND(iv3,vi,1,1,1) + IND(iv3,vi,1,1,2)); \
  (hess)[2] = rspX*rspY*(IND(iv3,vi,2,2,1) - IND(iv3,vi,0,2,1) - IND(iv3,vi,2,0,1) + IND(iv3,vi,0,0,1))/4.0f; \
  (hess)[3] = rspX*rspZ*(IND(iv3,vi,2,1,2) - IND(iv3,vi,0,1,2) - IND(iv3,vi,2,1,0) + IND(iv3,vi,0,1,0))/4.0f; \
  (hess)[5] = rspY*rspZ*(IND(iv3,vi,1,2,2) - IND(iv3,vi,1,0,2) - IND(iv3,vi,1,2,0) + IND(iv3,vi,1,0,0))/4.0f

#define _COIL_CONDUCT(LL, KK) \
  (exp(-0.5*(LL)/(KK)))

/*
#define _COIL_CONDUCT(vec, KK) \
  (1.0/(1.0 + (LL)/(KK)))
*/

/*
** watch out for false advertising!
*/
void
_coilKind7TensorFilterSelf(coil_t *delta, coil_t **iv3, 
                           double spacing[3],
                           double parm[COIL_PARMS_NUM]) {
  coil_t hess[7], rspX, rspY, rspZ, parm0;
  float eval[3], evec[9], tens[7], lin;

  rspX = AIR_CAST(coil_t, 1.0/spacing[0]);
  rspY = AIR_CAST(coil_t, 1.0/spacing[1]);
  rspZ = AIR_CAST(coil_t, 1.0/spacing[2]);
  TENS(tens, iv3);
  tenEigensolve_f(eval, evec, tens);
  lin = (eval[0] - eval[1])/(eval[0] - eval[2] + 0.000001f);
  TEN_T3V_OUTER(tens, evec + 3*0);
  delta[0] = 0;
  parm0 = AIR_CAST(coil_t, parm[0]);
  HESS(hess, iv3, 1, rspX, rspY, rspZ); delta[1] = lin*parm0*tens[0]*TEN_T_DOT(hess, tens);
  HESS(hess, iv3, 2, rspX, rspY, rspZ); delta[2] = lin*parm0*tens[0]*TEN_T_DOT(hess, tens);
  HESS(hess, iv3, 3, rspX, rspY, rspZ); delta[3] = lin*parm0*tens[0]*TEN_T_DOT(hess, tens);
  HESS(hess, iv3, 4, rspX, rspY, rspZ); delta[4] = lin*parm0*tens[0]*TEN_T_DOT(hess, tens);
  HESS(hess, iv3, 5, rspX, rspY, rspZ); delta[5] = lin*parm0*tens[0]*TEN_T_DOT(hess, tens);
  HESS(hess, iv3, 6, rspX, rspY, rspZ); delta[6] = lin*parm0*tens[0]*TEN_T_DOT(hess, tens);
}

void
_coilKind7TensorFilterFinish(coil_t *delta, coil_t **iv3, 
                             double spacing[3],
                             double parm[COIL_PARMS_NUM]) {
  coil_t rspX, rspY, rspZ,
    rspsqX, rspsqY, rspsqZ;
  double eval[3], evec[9], tens[7], tengrad[21], grad[3], LL, KK,
    cnd,
    dmu1[7], dmu2[7], mu2Norm, dskw[7], skwNorm,
    phi3[7];

  rspX = AIR_CAST(coil_t, 1.0/spacing[0]); rspsqX = rspX*rspX;
  rspY = AIR_CAST(coil_t, 1.0/spacing[1]); rspsqY = rspY*rspY;
  rspZ = AIR_CAST(coil_t, 1.0/spacing[2]); rspsqZ = rspZ*rspZ;
  TENS(tens, iv3);
  TENGRAD(tengrad, iv3, rspX, rspY, rspZ);
  tenEigensolve_d(eval, evec, tens);
  tenInvariantGradients_d(dmu1,
                          dmu2, &mu2Norm,
                          dskw, &skwNorm,
                          tens);
  tenRotationTangents_d(NULL, NULL, phi3, evec);
  /* \midhat{\nabla} \mu_1 ----------------- */
  ELL_3V_SET(grad,
             TEN_T_DOT(dmu1, tengrad + 0*7),
             TEN_T_DOT(dmu1, tengrad + 1*7),
             TEN_T_DOT(dmu1, tengrad + 2*7));
  LL = ELL_3V_DOT(grad,grad);
  KK = parm[1]*parm[1];
  cnd = _COIL_CONDUCT(LL, KK);
  /* \midhat{\nabla} \mu_2 ----------------- */
  ELL_3V_SET(grad,
             TEN_T_DOT(dmu2, tengrad + 0*7),
             TEN_T_DOT(dmu2, tengrad + 1*7),
             TEN_T_DOT(dmu2, tengrad + 2*7));
  LL = ELL_3V_DOT(grad,grad);
  KK = parm[2]*parm[2];
  cnd *= _COIL_CONDUCT(LL, KK);
  /* \midhat{\nabla} \skw and twist! ----------------- */
  ELL_3V_SET(grad,
             TEN_T_DOT(dskw, tengrad + 0*7),
             TEN_T_DOT(dskw, tengrad + 1*7),
             TEN_T_DOT(dskw, tengrad + 2*7));
  LL = ELL_3V_DOT(grad,grad);
  ELL_3V_SET(grad,
             TEN_T_DOT(phi3, tengrad + 0*7),
             TEN_T_DOT(phi3, tengrad + 1*7),
             TEN_T_DOT(phi3, tengrad + 2*7));
  LL += ELL_3V_DOT(grad,grad);
  KK = AIR_CAST(coil_t, parm[3]*parm[3]);
  cnd *= _COIL_CONDUCT(LL, KK); 
  delta[0]= 0.0f;
  delta[1]= AIR_CAST(coil_t, parm[0]*cnd*LAPL(iv3, 1, rspsqX, rspsqY, rspsqZ));
  delta[2]= AIR_CAST(coil_t, parm[0]*cnd*LAPL(iv3, 2, rspsqX, rspsqY, rspsqZ));
  delta[3]= AIR_CAST(coil_t, parm[0]*cnd*LAPL(iv3, 3, rspsqX, rspsqY, rspsqZ));
  delta[4]= AIR_CAST(coil_t, parm[0]*cnd*LAPL(iv3, 4, rspsqX, rspsqY, rspsqZ));
  delta[5]= AIR_CAST(coil_t, parm[0]*cnd*LAPL(iv3, 5, rspsqX, rspsqY, rspsqZ));
  delta[6]= AIR_CAST(coil_t, parm[0]*cnd*LAPL(iv3, 6, rspsqX, rspsqY, rspsqZ));
}

void
_coilKind7TensorUpdate(coil_t *val, coil_t *delta) {
  
  val[0] += delta[0]; /* WARNING: this could change confidence! */
  val[1] += delta[1];
  val[2] += delta[2];
  val[3] += delta[3];
  val[4] += delta[4];
  val[5] += delta[5];
  val[6] += delta[6];
}

const coilKind
_coilKind7Tensor = {
  "tensor",
  7,
  {NULL,
   _coilKind7TensorFilterTesting,
   _coilKind7TensorFilterHomogeneous,
   NULL,
   NULL,
   NULL,
   _coilKind7TensorFilterSelf,
   _coilKind7TensorFilterFinish},
  _coilKind7TensorUpdate
};

const coilKind *
coilKind7Tensor = &_coilKind7Tensor;
