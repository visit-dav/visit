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

#include "ten.h"
#include "privateTen.h"

gageItemEntry
_tenGageTable[TEN_GAGE_ITEM_MAX+1] = {
  /* enum value              len,deriv,  prereqs,                                                                    parent item, parent index, needData*/
  {tenGageTensor,              7,  0,  {-1, -1, -1, -1, -1},                                                                  -1,        -1,    0},
  {tenGageConfidence,          1,  0,  {tenGageTensor, -1, -1, -1, -1},                                            tenGageTensor,         0,    0},

  {tenGageTrace,               1,  0,  {tenGageTensor, -1, -1, -1, -1},                                                       -1,        -1,    0},
  {tenGageB,                   1,  0,  {tenGageTensor, -1, -1, -1, -1},                                                       -1,        -1,    0},
  {tenGageDet,                 1,  0,  {tenGageTensor, -1, -1, -1, -1},                                                       -1,        -1,    0},
  {tenGageS,                   1,  0,  {tenGageTensor, -1, -1, -1, -1},                                                       -1,        -1,    0},
  {tenGageQ,                   1,  0,  {tenGageS, tenGageB, -1, -1, -1},                                                      -1,        -1,    0},
  {tenGageFA,                  1,  0,  {tenGageQ, tenGageS, -1, -1, -1},                                                      -1,        -1,    0},
  {tenGageR,                   1,  0,  {tenGageTrace, tenGageB, tenGageDet, tenGageS, -1},                                    -1,        -1,    0},
  {tenGageTheta,               1,  0,  {tenGageR, tenGageQ, -1, -1, -1},                                                      -1,        -1,    0},

  {tenGageEval,                3,  0,  {tenGageTensor, -1, -1, -1, -1},                                                       -1,        -1,    0},
  {tenGageEval0,               1,  0,  {tenGageEval, -1, -1, -1, -1},                                                tenGageEval,         0,    0},
  {tenGageEval1,               1,  0,  {tenGageEval, -1, -1, -1, -1},                                                tenGageEval,         1,    0},
  {tenGageEval2,               1,  0,  {tenGageEval, -1, -1, -1, -1},                                                tenGageEval,         2,    0},
  {tenGageEvec,                9,  0,  {tenGageTensor, -1, -1, -1, -1},                                                       -1,        -1,    0},
  {tenGageEvec0,               3,  0,  {tenGageEvec, -1, -1, -1, -1},                                                tenGageEvec,         0,    0},
  {tenGageEvec1,               3,  0,  {tenGageEvec, -1, -1, -1, -1},                                                tenGageEvec,         3,    0},
  {tenGageEvec2,               3,  0,  {tenGageEvec, -1, -1, -1, -1},                                                tenGageEvec,         6,    0},

  {tenGageTensorGrad,         21,  1,  {-1, -1, -1, -1, -1},                                                                  -1,        -1,    0},
  {tenGageTensorGradMag,       3,  1,  {tenGageTensorGrad, -1, -1, -1, -1},                                                   -1,        -1,    0},
  {tenGageTensorGradMagMag,    1,  1,  {tenGageTensorGradMag, -1, -1, -1, -1},                                                -1,        -1,    0},

  {tenGageTraceGradVec,        3,  1,  {tenGageTensor, tenGageTensorGrad, -1, -1, -1},                                        -1,        -1,    0},
  {tenGageTraceGradMag,        1,  1,  {tenGageTraceGradVec, -1, -1, -1, -1},                                                 -1,        -1,    0},
  {tenGageTraceNormal,         3,  1,  {tenGageTraceGradVec, tenGageTraceGradMag, -1, -1, -1},                                -1,        -1,    0},

  {tenGageBGradVec,            3,  1,  {tenGageTensor, tenGageTensorGrad, -1, -1, -1},                                        -1,        -1,    0},
  {tenGageBGradMag,            1,  1,  {tenGageBGradVec, -1, -1, -1, -1},                                                     -1,        -1,    0},
  {tenGageBNormal,             3,  1,  {tenGageBGradVec, tenGageBGradMag, -1, -1, -1},                                        -1,        -1,    0},

  {tenGageDetGradVec,          3,  1,  {tenGageTensor, tenGageTensorGrad, -1, -1, -1},                                        -1,        -1,    0},
  {tenGageDetGradMag,          1,  1,  {tenGageDetGradVec, -1, -1, -1, -1},                                                   -1,        -1,    0},
  {tenGageDetNormal,           3,  1,  {tenGageDetGradVec, tenGageDetGradMag, -1, -1, -1},                                    -1,        -1,    0},

  {tenGageSGradVec,            3,  1,  {tenGageTensor, tenGageTensorGrad, -1, -1, -1},                                        -1,        -1,    0},
  {tenGageSGradMag,            1,  1,  {tenGageSGradVec, -1, -1, -1, -1},                                                     -1,        -1,    0},
  {tenGageSNormal,             3,  1,  {tenGageSGradVec, tenGageSGradMag, -1, -1, -1},                                        -1,        -1,    0},

  {tenGageQGradVec,            3,  1,  {tenGageSGradVec, tenGageBGradVec, -1, -1, -1},                                        -1,        -1,    0},
  {tenGageQGradMag,            1,  1,  {tenGageQGradVec, -1, -1, -1, -1},                                                     -1,        -1,    0},
  {tenGageQNormal,             3,  1,  {tenGageQGradVec, tenGageQGradMag, -1, -1, -1},                                        -1,        -1,    0},

  {tenGageFAGradVec,           3,  1,  {tenGageQGradVec, tenGageSGradVec, tenGageFA, -1, -1},                                 -1,        -1,    0},
  {tenGageFAGradMag,           1,  1,  {tenGageFAGradVec, -1, -1, -1, -1},                                                    -1,        -1,    0},
  {tenGageFANormal,            3,  1,  {tenGageFAGradVec, tenGageFAGradMag, -1, -1, -1},                                      -1,        -1,    0},

  {tenGageRGradVec,            3,  1,  {tenGageTraceGradVec, tenGageBGradVec, tenGageDetGradVec, tenGageSGradVec, -1},        -1,        -1,    0},
  {tenGageRGradMag,            1,  1,  {tenGageRGradVec, -1, -1, -1, -1},                                                     -1,        -1,    0},
  {tenGageRNormal,             3,  1,  {tenGageRGradVec, tenGageRGradMag, -1, -1, -1},                                        -1,        -1,    0},

  {tenGageThetaGradVec,        3,  1,  {tenGageRGradVec, tenGageQGradVec, tenGageTheta, -1, -1},                              -1,        -1,    0},
  {tenGageThetaGradMag,        1,  1,  {tenGageThetaGradVec, -1, -1, -1, -1},                                                 -1,        -1,    0},
  {tenGageThetaNormal,         3,  1,  {tenGageThetaGradVec, tenGageThetaGradMag, -1, -1, -1},                                -1,        -1,    0},
  
  {tenGageInvarGrads,          9,  1,  {tenGageTensor, tenGageTensorGrad, -1, -1, -1},                                        -1,        -1,    0},
  {tenGageInvarGradMags,       3,  1,  {tenGageInvarGrads, -1, -1, -1, -1},                                                   -1,        -1,    0},
  {tenGageRotTans,             9,  1,  {tenGageTensor, tenGageTensorGrad, tenGageEval, tenGageEvec, -1},                      -1,        -1,    0},
  {tenGageRotTanMags,          3,  1,  {tenGageRotTans, -1, -1, -1, -1},                                                      -1,        -1,    0},

  {tenGageEvalGrads,           9,  1,  {tenGageTensorGrad, tenGageEval, tenGageEvec, -1},                                     -1,        -1,    0},

  {tenGageCl1,                 1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2, -1},                         -1,        -1,    0},
  {tenGageCp1,                 1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2, -1},                         -1,        -1,    0},
  {tenGageCa1,                 1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2, -1},                         -1,        -1,    0},
  {tenGageCl2,                 1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2, -1},                         -1,        -1,    0},
  {tenGageCp2,                 1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2, -1},                         -1,        -1,    0},
  {tenGageCa2,                 1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2, -1},                         -1,        -1,    0},
  {tenGageAniso, TEN_ANISO_MAX+1,  0,  {tenGageEval0, tenGageEval1, tenGageEval2, -1, -1},                                    -1,        -1,    0}
};

void
_tenGageIv3Print (FILE *file, gageContext *ctx, gagePerVolume *pvl) {
  gage_t *iv3;
  int i, fd;

  fd = 2*ctx->radius;
  iv3 = pvl->iv3 + fd*fd*fd;
  fprintf(file, "iv3[]'s *Dxx* component:\n");
  switch(fd) {
  case 2:
    fprintf(file, "% 10.4f   % 10.4f\n", (float)iv3[6], (float)iv3[7]);
    fprintf(file, "   % 10.4f   % 10.4f\n\n", (float)iv3[4], (float)iv3[5]);
    fprintf(file, "% 10.4f   % 10.4f\n", (float)iv3[2], (float)iv3[3]);
    fprintf(file, "   % 10.4f   % 10.4f\n", (float)iv3[0], (float)iv3[1]);
    break;
  case 4:
    for (i=3; i>=0; i--) {
      fprintf(file, "% 10.4f   % 10.4f   % 10.4f   % 10.4f\n", 
              (float)iv3[12+16*i], (float)iv3[13+16*i], 
              (float)iv3[14+16*i], (float)iv3[15+16*i]);
      fprintf(file, "   % 10.4f  %c% 10.4f   % 10.4f%c   % 10.4f\n", 
              (float)iv3[ 8+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[ 9+16*i], (float)iv3[10+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[11+16*i]);
      fprintf(file, "      % 10.4f  %c% 10.4f   % 10.4f%c   % 10.4f\n", 
              (float)iv3[ 4+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[ 5+16*i], (float)iv3[ 6+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[ 7+16*i]);
      fprintf(file, "         % 10.4f   % 10.4f   % 10.4f   % 10.4f\n", 
              (float)iv3[ 0+16*i], (float)iv3[ 1+16*i],
              (float)iv3[ 2+16*i], (float)iv3[ 3+16*i]);
      if (i) fprintf(file, "\n");
    }
    break;
  default:
    for (i=0; i<fd*fd*fd; i++) {
      fprintf(file, "  iv3[% 3d,% 3d,% 3d] = % 10.4f\n",
              i%fd, (i/fd)%fd, i/(fd*fd), (float)iv3[i]);
    }
    break;
  }
  return;
}

void
_tenGageFilter (gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_tenGageFilter";
  gage_t *fw00, *fw11, *fw22, *tensor, *tgrad;
  int fd;

  fd = 2*ctx->radius;
  tensor = pvl->directAnswer[tenGageTensor];
  tgrad = pvl->directAnswer[tenGageTensorGrad];
  if (!ctx->parm.k3pack) {
    fprintf(stderr, "!%s: sorry, 6pack filtering not implemented\n", me);
    return;
  }
  fw00 = ctx->fw + fd*3*gageKernel00;
  fw11 = ctx->fw + fd*3*gageKernel11;
  fw22 = ctx->fw + fd*3*gageKernel22;
  /* perform the filtering */
  switch (fd) {
  case 2:
#define DOIT_2(J) \
      gageScl3PFilter2(pvl->iv3 + J*8, pvl->iv2 + J*4, pvl->iv1 + J*2, \
                       fw00, fw11, fw22, \
                       tensor + J, tgrad + J*3, NULL, \
                       pvl->needD[0], pvl->needD[1], AIR_FALSE)
    /* HEY: want trilinear interpolation of confidence */
    /* old idea: do average of confidence at 8 corners of containing voxel
    tensor[0] = (pvl->iv3[0] + pvl->iv3[1] + pvl->iv3[2] + pvl->iv3[3]
                 + pvl->iv3[4] + pvl->iv3[5] + pvl->iv3[6] + pvl->iv3[7])/8;
    */
    /* new idea (circa Sat Apr  2 06:59:02 EST 2005):
       do the same filtering- its just too weird for confidence to not 
       be C0 when the filtering result is */
    DOIT_2(0); 
    DOIT_2(1); DOIT_2(2); DOIT_2(3);
    DOIT_2(4); DOIT_2(5); DOIT_2(6); 
    break;
  case 4:
#define DOIT_4(J) \
      gageScl3PFilter4(pvl->iv3 + J*64, pvl->iv2 + J*16, pvl->iv1 + J*4, \
                       fw00, fw11, fw22, \
                       tensor + J, tgrad + J*3, NULL, \
                       pvl->needD[0], pvl->needD[1], AIR_FALSE)
    /* HEY: want trilinear interpolation of confidence */
    /* old: SEE NOTE ABOVE
    tensor[0] = (pvl->iv3[21] + pvl->iv3[22]
                 + pvl->iv3[25] + pvl->iv3[26]
                 + pvl->iv3[37] + pvl->iv3[38] 
                 + pvl->iv3[41] + pvl->iv3[42])/8;
    */
    DOIT_4(0); 
    DOIT_4(1); DOIT_4(2); DOIT_4(3);
    DOIT_4(4); DOIT_4(5); DOIT_4(6); 
    break;
  default:
#define DOIT_N(J)\
      gageScl3PFilterN(fd, \
                       pvl->iv3 + J*fd*fd*fd, \
                       pvl->iv2 + J*fd*fd, pvl->iv1 + J*fd, \
                       fw00, fw11, fw22, \
                       tensor + J, tgrad + J*3, NULL, \
                       pvl->needD[0], pvl->needD[1], AIR_FALSE)
    /* HEY: this sucks: want trilinear interpolation of confidence */
    DOIT_N(0); DOIT_N(1); DOIT_N(2); DOIT_N(3);
    DOIT_N(4); DOIT_N(5); DOIT_N(6); 
    break;
  }

  return;
}

void
_tenGageAnswer (gageContext *ctx, gagePerVolume *pvl) {
  /* char me[]="_tenGageAnswer"; */
  gage_t epsilon=1.0E-10f;
  gage_t *tenAns, *evalAns, *evecAns, *vecTmp=NULL,
    *gradDtA=NULL, *gradDtB=NULL, *gradDtC=NULL,
    *gradDtD=NULL, *gradDtE=NULL, *gradDtF=NULL,
    gradDdXYZ[21]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    *gradCbS=NULL, *gradCbB=NULL, *gradCbQ=NULL, *gradCbR=NULL;
  gage_t tmp0, tmp1, magTmp=0,
    gradCbA[3]={0,0,0}, gradCbC[3]={0,0,0},
    dtA=0, dtB=0, dtC=0, dtD=0, dtE=0, dtF=0,
    cbQQQ=0, cbQ=0, cbR=0, cbA=0, cbB=0, cbC=0, cbS=0;

#if !GAGE_TYPE_FLOAT
  int ci;
  float evalAnsF[3], aniso[TEN_ANISO_MAX+1];
#endif

  tenAns = pvl->directAnswer[tenGageTensor];
  evalAns = pvl->directAnswer[tenGageEval];
  evecAns = pvl->directAnswer[tenGageEvec];
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensor)) {
    /* done if doV */
    tenAns[0] = AIR_CLAMP(0.0f, tenAns[0], 1.0f);
    dtA = tenAns[1];
    dtB = tenAns[2];
    dtC = tenAns[3];
    dtD = tenAns[4];
    dtE = tenAns[5];
    dtF = tenAns[6];
    if (ctx->verbose) {
      fprintf(stderr, "tensor = (%g) %g %g %g   %g %g   %g\n", tenAns[0],
              dtA, dtB, dtC, dtD, dtE, dtF);
    }
  }
  /* done if doV 
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageConfidence)) {
  }
  */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTrace)) {
    cbA = -(pvl->directAnswer[tenGageTrace][0] = dtA + dtD + dtF);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageB)) {
    cbB = pvl->directAnswer[tenGageB][0] = 
      dtA*dtD + dtA*dtF + dtD*dtF - dtB*dtB - dtC*dtC - dtE*dtE;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDet)) {
    cbC = -(pvl->directAnswer[tenGageDet][0] = 
            2.0f*dtB*dtC*dtE + dtA*dtD*dtF 
            - dtC*dtC*dtD - dtA*dtE*dtE - dtB*dtB*dtF);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageS)) {
    cbS = (pvl->directAnswer[tenGageS][0] = 
           dtA*dtA + dtD*dtD + dtF*dtF
           + 2.0f*dtB*dtB + 2.0f*dtC*dtC + 2.0f*dtE*dtE);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageQ)) {
    cbQ = pvl->directAnswer[tenGageQ][0] = (cbS - cbB)/9.0f;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFA)) {
    pvl->directAnswer[tenGageFA][0] =
      AIR_CAST(gage_t, 3*sqrt(cbQ/(epsilon + cbS)));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageR)) {
    cbR = pvl->directAnswer[tenGageR][0] =
      (5.0f*cbA*cbB - 27.0f*cbC - 2.0f*cbA*cbS)/54.0f;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTheta)) {
    cbQQQ = cbQ*cbQ*cbQ;
    tmp0 = AIR_CAST(gage_t, cbR/(epsilon + sqrt(cbQQQ)));
    tmp0 = AIR_CLAMP(-1.0f, tmp0, 1.0f);
    pvl->directAnswer[tenGageTheta][0] = AIR_CAST(gage_t, acos(tmp0)/AIR_PI);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageEvec)) {
    /* we do the longer process to get eigenvectors, and in the process
       we always find the eigenvalues, whether or not they were asked for */
#if GAGE_TYPE_FLOAT
    tenEigensolve_f(evalAns, evecAns, tenAns);
#else
    tenEigensolve_d(evalAns, evecAns, tenAns);
#endif
  } else if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageEval)) {
    /* else eigenvectors are NOT needed, but eigenvalues ARE needed */
#if GAGE_TYPE_FLOAT
    tenEigensolve_f(evalAns, NULL, tenAns);
#else
    tenEigensolve_d(evalAns, NULL, tenAns);
#endif
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorGrad)) {
    /* done if doD1 */
    /* still have to set up pointer variables that item answers
       below will rely on as short-cuts */
    vecTmp = pvl->directAnswer[tenGageTensorGrad];
    gradDtA = vecTmp + 1*3;
    gradDtB = vecTmp + 2*3;
    gradDtC = vecTmp + 3*3;
    gradDtD = vecTmp + 4*3;
    gradDtE = vecTmp + 5*3;
    gradDtF = vecTmp + 6*3;
    TEN_T_SET(gradDdXYZ + 0*7, tenAns[0],
              gradDtA[0], gradDtB[0], gradDtC[0],
              gradDtD[0], gradDtE[0],
              gradDtF[0]);
    TEN_T_SET(gradDdXYZ + 1*7, tenAns[0],
              gradDtA[1], gradDtB[1], gradDtC[1],
              gradDtD[1], gradDtE[1],
              gradDtF[1]);
    TEN_T_SET(gradDdXYZ + 2*7, tenAns[0],
              gradDtA[2], gradDtB[2], gradDtC[2],
              gradDtD[2], gradDtE[2],
              gradDtF[2]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorGradMag)) {
    vecTmp = pvl->directAnswer[tenGageTensorGradMag];
    vecTmp[0] = AIR_CAST(gage_t, sqrt(TEN_T_DOT(gradDdXYZ + 0*7,
                                                gradDdXYZ + 0*7)));
    vecTmp[1] = AIR_CAST(gage_t, sqrt(TEN_T_DOT(gradDdXYZ + 1*7,
                                                gradDdXYZ + 1*7)));
    vecTmp[2] = AIR_CAST(gage_t, sqrt(TEN_T_DOT(gradDdXYZ + 2*7,
                                                gradDdXYZ + 2*7)));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorGradMag)) {
    pvl->directAnswer[tenGageTensorGradMagMag][0] =
      AIR_CAST(gage_t, ELL_3V_LEN(vecTmp));
  }

  /* --- Trace --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTraceGradVec)) {
    vecTmp = pvl->directAnswer[tenGageTraceGradVec];
    ELL_3V_ADD3(vecTmp, gradDtA, gradDtD, gradDtF);
    ELL_3V_SCALE(gradCbA, -1, vecTmp);

  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTraceGradMag)) {
    magTmp = pvl->directAnswer[tenGageTraceGradMag][0] =
      AIR_CAST(gage_t, ELL_3V_LEN(vecTmp));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTraceNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageTraceNormal],
                 1.0f/(epsilon + magTmp), vecTmp);
  }
  /* --- B --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageBGradVec)) {
    gradCbB = vecTmp = pvl->directAnswer[tenGageBGradVec];
    ELL_3V_SCALE_ADD6(vecTmp, 
                      dtD + dtF, gradDtA,
                      -2.0f*dtB, gradDtB,
                      -2.0f*dtC, gradDtC,
                      dtA + dtF, gradDtD,
                      -2.0f*dtE, gradDtE,
                      dtA + dtD, gradDtF);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageBGradMag)) {
    magTmp = pvl->directAnswer[tenGageBGradMag][0] =
      AIR_CAST(gage_t, ELL_3V_LEN(vecTmp));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageBNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageBNormal],
                 1.0f/(epsilon + magTmp), vecTmp);
  }
  /* --- Det --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDetGradVec)) {
    vecTmp = pvl->directAnswer[tenGageDetGradVec];
    ELL_3V_SCALE_ADD6(vecTmp,
                      dtD*dtF - dtE*dtE, gradDtA,
                      2.0f*(dtC*dtE - dtB*dtF), gradDtB,
                      2.0f*(dtB*dtE - dtC*dtD), gradDtC,
                      dtA*dtF - dtC*dtC, gradDtD,
                      2.0f*(dtB*dtC - dtA*dtE), gradDtE,
                      dtA*dtD - dtB*dtB, gradDtF);
    ELL_3V_SCALE(gradCbC, -1, vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDetGradMag)) {
    magTmp = pvl->directAnswer[tenGageDetGradMag][0] =
      AIR_CAST(float, ELL_3V_LEN(vecTmp));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDetNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageDetNormal],
                 1.0f/(epsilon + magTmp), vecTmp);
  }
  /* --- S --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageSGradVec)) {
    gradCbS = vecTmp = pvl->directAnswer[tenGageSGradVec];
    ELL_3V_SCALE_ADD6(vecTmp,
                      2.0f*dtA, gradDtA,
                      4.0f*dtB, gradDtB,
                      4.0f*dtC, gradDtC,
                      2.0f*dtD, gradDtD,
                      4.0f*dtE, gradDtE,
                      2.0f*dtF, gradDtF);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageSGradMag)) {
    magTmp = pvl->directAnswer[tenGageSGradMag][0] =
      AIR_CAST(gage_t, ELL_3V_LEN(vecTmp));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageSNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageSNormal],
                 1.0f/(epsilon + magTmp), vecTmp);
  }
  /* --- Q --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageQGradVec)) {
    gradCbQ = vecTmp = pvl->directAnswer[tenGageQGradVec];
    ELL_3V_SCALE_ADD2(vecTmp,
                      1.0f/9.0f, gradCbS, 
                      -1.0f/9.0f, gradCbB);

  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageQGradMag)) {
    magTmp = pvl->directAnswer[tenGageQGradMag][0] =
      AIR_CAST(gage_t, ELL_3V_LEN(vecTmp));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageQNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageQNormal],
                 1.0f/(epsilon + magTmp), vecTmp);
  }
  /* --- FA --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAGradVec)) {
    vecTmp = pvl->directAnswer[tenGageFAGradVec];
    tmp0 = 9.0f/(epsilon + 2*pvl->directAnswer[tenGageFA][0]*cbS);
    tmp1 = -tmp0*cbQ/(epsilon + cbS);
    ELL_3V_SCALE_ADD2(vecTmp,
                      AIR_CAST(gage_t, tmp0), gradCbQ, 
                      AIR_CAST(gage_t, tmp1), gradCbS);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAGradMag)) {
    magTmp = pvl->directAnswer[tenGageFAGradMag][0] =
      AIR_CAST(gage_t, ELL_3V_LEN(vecTmp));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFANormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageFANormal],
                 1.0f/(epsilon + magTmp), vecTmp);
  }
  /* --- R --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRGradVec)) {
    gradCbR = vecTmp = pvl->directAnswer[tenGageRGradVec];
    tmp0 = AIR_CAST(gage_t, 1.0/(epsilon + 2*sqrt(cbQ*cbQ*cbQ)));
    ELL_3V_SCALE_ADD4(vecTmp,
                      (5.0f*cbB - 2.0f*cbS)/54.0f, gradCbA,
                      -1.0f/2.0f, gradCbC,
                      5.0f*cbA/54.0f, gradCbB,
                      -cbA/27.0f, gradCbS);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRGradMag)) {
    magTmp = pvl->directAnswer[tenGageRGradMag][0] =
      AIR_CAST(gage_t, ELL_3V_LEN(vecTmp));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageRNormal],
                 1.0f/(epsilon + magTmp), vecTmp);
  }
  /* --- Theta --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageThetaGradVec)) {
    vecTmp = pvl->directAnswer[tenGageThetaGradVec];
    tmp1 = cbQ*cbQ*cbQ;
    tmp1 = AIR_CAST(gage_t, sqrt(tmp1)*sqrt(1.0 - cbR*cbR/(epsilon + tmp1)));
    tmp1 = 1.0f/(epsilon + tmp1);
    tmp0 = tmp1*3*cbR/(epsilon + 2*cbQ);
    ELL_3V_SCALE_ADD2(vecTmp,
                      tmp0, gradCbQ,
                      -tmp1, gradCbR);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageThetaGradMag)) {
    magTmp = pvl->directAnswer[tenGageThetaGradMag][0] = 
      AIR_CAST(gage_t, ELL_3V_LEN(vecTmp));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageThetaNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageThetaNormal],
                 1.0f/(epsilon + magTmp), vecTmp);
  }
  /* --- Invariant gradients + rotation tangents --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageInvarGrads)) {
    double mu1Grad[7], mu2Grad[7], mu2Norm,
      skwGrad[7], skwNorm, copyT[7];
    
    TEN_T_COPY(copyT, tenAns);
    tenInvariantGradients_d(mu1Grad, 
                            mu2Grad, &mu2Norm,
                            skwGrad, &skwNorm,
                            copyT);
    ELL_3V_SET_TT(pvl->directAnswer[tenGageInvarGrads] + 0*3, gage_t,
                  TEN_T_DOT(mu1Grad, gradDdXYZ + 0*7),
                  TEN_T_DOT(mu1Grad, gradDdXYZ + 1*7),
                  TEN_T_DOT(mu1Grad, gradDdXYZ + 2*7));
    ELL_3V_SET_TT(pvl->directAnswer[tenGageInvarGrads] + 1*3, gage_t,
                  TEN_T_DOT(mu2Grad, gradDdXYZ + 0*7),
                  TEN_T_DOT(mu2Grad, gradDdXYZ + 1*7),
                  TEN_T_DOT(mu2Grad, gradDdXYZ + 2*7));
    ELL_3V_SET_TT(pvl->directAnswer[tenGageInvarGrads] + 2*3, gage_t,
                  TEN_T_DOT(skwGrad, gradDdXYZ + 0*7),
                  TEN_T_DOT(skwGrad, gradDdXYZ + 1*7),
                  TEN_T_DOT(skwGrad, gradDdXYZ + 2*7));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageInvarGradMags)) {
    ELL_3V_SET_TT(pvl->directAnswer[tenGageInvarGradMags], gage_t,
                  ELL_3V_LEN(pvl->directAnswer[tenGageInvarGrads] + 0*3),
                  ELL_3V_LEN(pvl->directAnswer[tenGageInvarGrads] + 1*3),
                  ELL_3V_LEN(pvl->directAnswer[tenGageInvarGrads] + 2*3));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageEvalGrads)) {
    double matOut[9], tenOut[9];
    int evi;

    for (evi=0; evi<=2; evi++) {
      ELL_3MV_OUTER(matOut, evecAns + evi*3, evecAns + evi*3);
      TEN_M2T(tenOut, matOut);
      ELL_3V_SET_TT(pvl->directAnswer[tenGageEvalGrads] + evi*3, gage_t, 
                    TEN_T_DOT(tenOut, gradDdXYZ + 0*7),
                    TEN_T_DOT(tenOut, gradDdXYZ + 1*7),
                    TEN_T_DOT(tenOut, gradDdXYZ + 2*7));
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRotTans)) {
    double phi1[7], phi2[7], phi3[7], evec[9];

    ELL_9V_COPY(evec, evecAns);
    tenRotationTangents_d(phi1, phi2, phi3, evec);
    ELL_3V_SET_TT(pvl->directAnswer[tenGageRotTans] + 0*3, gage_t, 
                  TEN_T_DOT(phi1, gradDdXYZ + 0*7),
                  TEN_T_DOT(phi1, gradDdXYZ + 1*7),
                  TEN_T_DOT(phi1, gradDdXYZ + 2*7));
    ELL_3V_SET_TT(pvl->directAnswer[tenGageRotTans] + 1*3, gage_t,
                  TEN_T_DOT(phi2, gradDdXYZ + 0*7),
                  TEN_T_DOT(phi2, gradDdXYZ + 1*7),
                  TEN_T_DOT(phi2, gradDdXYZ + 2*7));
    ELL_3V_SET_TT(pvl->directAnswer[tenGageRotTans] + 2*3, gage_t,
                  TEN_T_DOT(phi3, gradDdXYZ + 0*7),
                  TEN_T_DOT(phi3, gradDdXYZ + 1*7),
                  TEN_T_DOT(phi3, gradDdXYZ + 2*7));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRotTanMags)) {
    ELL_3V_SET_TT(pvl->directAnswer[tenGageRotTanMags], gage_t,
                  ELL_3V_LEN(pvl->directAnswer[tenGageRotTans] + 0*3),
                  ELL_3V_LEN(pvl->directAnswer[tenGageRotTans] + 1*3),
                  ELL_3V_LEN(pvl->directAnswer[tenGageRotTans] + 2*3));
  }
  /* --- C{l,p,a}1 --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCl1)) {
#if GAGE_TYPE_FLOAT
    tmp0 = tenAnisoEval_f(evalAns, tenAniso_Cl1);
#else
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Cl1);
#endif
    pvl->directAnswer[tenGageCl1][0] = AIR_CLAMP(0.0f, tmp0, 1.0f);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCp1)) {
#if GAGE_TYPE_FLOAT
    tmp0 = tenAnisoEval_f(evalAns, tenAniso_Cp1);
#else
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Cp1);
#endif
    pvl->directAnswer[tenGageCp1][0] = AIR_CLAMP(0.0f, tmp0, 1.0f);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCa1)) {
#if GAGE_TYPE_FLOAT
    tmp0 = tenAnisoEval_f(evalAns, tenAniso_Ca1);
#else
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Ca1);
#endif
    pvl->directAnswer[tenGageCa1][0] = AIR_CLAMP(0.0f, tmp0, 1.0f);
  }
  /* --- C{l,p,a}2 --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCl2)) {
#if GAGE_TYPE_FLOAT
    tmp0 = tenAnisoEval_f(evalAns, tenAniso_Cl2);
#else
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Cl2);
#endif
    pvl->directAnswer[tenGageCl2][0] = AIR_CLAMP(0.0f, tmp0, 1.0f);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCp2)) {
#if GAGE_TYPE_FLOAT
    tmp0 = tenAnisoEval_f(evalAns, tenAniso_Cp2);
#else
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Cp2);
#endif
    pvl->directAnswer[tenGageCp2][0] = AIR_CLAMP(0.0f, tmp0, 1.0f);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCa2)) {
#if GAGE_TYPE_FLOAT
    tmp0 = tenAnisoEval_f(evalAns, tenAniso_Ca2);
#else
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Ca2);
#endif
    pvl->directAnswer[tenGageCa2][0] = AIR_CLAMP(0.0f, tmp0, 1.0f);
  }
  /* --- Aniso --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageAniso)) {
#if GAGE_TYPE_FLOAT
    tenAnisoCalc_f(pvl->directAnswer[tenGageAniso], evalAns);
#else
    ELL_3V_COPY(evalAnsF, evalAns);
    tenAnisoCalc_f(aniso, evalAnsF);
    for (ci=0; ci<=TEN_ANISO_MAX; ci++) {
      pvl->directAnswer[tenGageAniso][ci] = aniso[ci];
    }
#endif
  }
  return;
}

gageKind
_tenGageKind = {
  "tensor",
  &_tenGage,
  1,
  7,
  TEN_GAGE_ITEM_MAX,
  _tenGageTable,
  _tenGageIv3Print,
  _tenGageFilter,
  _tenGageAnswer,
  NULL, NULL, NULL,
  NULL
};
gageKind *
tenGageKind = &_tenGageKind;
