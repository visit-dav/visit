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

#include "mite.h"
#include "privateMite.h"

int
miteRayBegin(miteThread *mtt, miteRender *mrr, miteUser *muu,
             int uIndex, int vIndex, 
             double rayLen,
             double rayStartWorld[3], double rayStartIndex[3],
             double rayDirWorld[3], double rayDirIndex[3]) {

  AIR_UNUSED(mrr);
  AIR_UNUSED(rayStartWorld);
  AIR_UNUSED(rayStartIndex);
  AIR_UNUSED(rayDirIndex);

  mtt->ui = uIndex;
  mtt->vi = vIndex;
  mtt->rayStep = (muu->rayStep*rayLen /
                  (muu->hctx->cam->vspFaar - muu->hctx->cam->vspNeer));
  if (!uIndex) {
    fprintf(stderr, "%d/%d ", vIndex, muu->hctx->imgSize[1]);
    fflush(stderr);
  }
  mtt->verbose = (uIndex == muu->verbUi && vIndex == muu->verbVi);
  mtt->skip = (muu->verbUi >= 0 && muu->verbVi >= 0
               && !mtt->verbose);
  if (mtt->verbose) {
    /* create muu->ndebug */
    muu->ndebug = nrrdNew();
    /* we want to store the value and index for each txf domain variable,
       plus the RGBAZ computed for that sample */
    muu->ndebug->axis[0].size = 2*mtt->stageNum + 5;
    /* we really do want to associate ndebug with the miteUser's mop,
       because the information stored in it has to persist for as long as
       the user wants: mite itself doesn't call miteUserNix */
    airMopAdd(muu->umop, muu->ndebug, (airMopper)nrrdNuke, airMopAlways);
    /* but the scope of the debug array allocation is within this ray */
    muu->debugArr = airArrayNew((void**)(&(muu->debug)),
                                NULL, sizeof(double), 128);
  }
  mtt->raySample = 0;
  mtt->RR = mtt->GG = mtt->BB = 0.0;
  mtt->TT = 1.0;
  mtt->ZZ = AIR_NAN;
  ELL_3V_SCALE(mtt->V, -1, rayDirWorld);

  return 0;
}

void
_miteRGBACalc(mite_t *R, mite_t *G, mite_t *B, mite_t *A,
              miteThread *mtt, miteRender *mrr, miteUser *muu) {
  char me[]="_miteRGBACalc";
  mite_t tmp,
    ad[3],                          /* ambient+diffuse light contribution */
    s[3] = {0,0,0},                 /* specular light contribution */
    col[3], E, ka, kd, ks, sp,      /* txf-determined rendering variables */
    LdotN=0, HdotN, H[3], N[3];     /* for lighting calculation */

  col[0] = mtt->range[miteRangeRed];
  col[1] = mtt->range[miteRangeGreen];
  col[2] = mtt->range[miteRangeBlue];
  E = mtt->range[miteRangeEmissivity];
  ka = mtt->range[miteRangeKa];
  kd = mtt->range[miteRangeKd];
  ks = mtt->range[miteRangeKs];
  ELL_3V_SCALE(ad, ka, muu->lit->amb);
  switch (mrr->shadeSpec->method) {
  case miteShadeMethodNone:
    /* nothing to do */
    break;
  case miteShadeMethodPhong:
    if (kd || ks) {
      ELL_3V_NORM(N, mtt->shadeVec0, tmp);
      if (1 == muu->normalSide) {
        ELL_3V_SCALE(N, -1, N);
      }
      /* else -1==side --> N = -1*-1*N = N
         or 0==side --> N = N, so there's nothing to do */
      if (kd) {
        LdotN = ELL_3V_DOT(muu->lit->dir[0], N);
        if (!muu->normalSide) {
          LdotN = AIR_ABS(LdotN);
        }
        if (LdotN > 0) {
          ELL_3V_SCALE_INCR(ad, LdotN*kd, muu->lit->col[0]);
        }
      }
      if (ks) {
        sp = mtt->range[miteRangeSP];
        ELL_3V_ADD2(H, muu->lit->dir[0], mtt->V);
        ELL_3V_NORM(H, H, tmp);
        HdotN = ELL_3V_DOT(H, N);
        if (!muu->normalSide) {
          HdotN = AIR_ABS(HdotN);
        }
        if (HdotN > 0) {
          HdotN = pow(HdotN, sp);
          ELL_3V_SCALE(s, HdotN*ks, muu->lit->col[0]);
        }
      }
    }
    break;
  case miteShadeMethodLitTen:
    fprintf(stderr, "!%s: lit-tensor not yet implemented\n", me);
    break;
  default:
    fprintf(stderr, "!%s: PANIC, shadeMethod %d unimplemented\n", 
            me, mrr->shadeSpec->method);
    exit(1);
    break;
  }
  *R = (E - 1 + ad[0])*col[0] + s[0];
  *G = (E - 1 + ad[1])*col[1] + s[1];
  *B = (E - 1 + ad[2])*col[2] + s[2];
  *A = mtt->range[miteRangeAlpha];
  *A = AIR_CLAMP(0.0, *A, 1.0);
  /*
  if (mtt->verbose) {
    fprintf(stderr, "%s: col[] = %g,%g,%g; A,E = %g,%g; Kads = %g,%g,%g\n", me,
            col[0], col[1], col[2], mtt->range[miteRangeAlpha], E, ka, kd, ks);
    fprintf(stderr, "%s: N = (%g,%g,%g), L = (%g,%g,%g) ---> LdotN = %g\n",
            me, N[0], N[1], N[2], muu->lit->dir[0][0], muu->lit->dir[0][1],
            muu->lit->dir[0][2], LdotN);
    fprintf(stderr, "%s: ad[] = %g,%g,%g\n", me, ad[0], ad[1], ad[2]);
    fprintf(stderr, "%s:  --> R,G,B,A = %g,%g,%g,%g\n", me, *R, *G, *B, *A);
  }
  */
  return;
}

double
miteSample(miteThread *mtt, miteRender *mrr, miteUser *muu,
           int num, double rayT, int inside,
           double samplePosWorld[3],
           double samplePosIndex[3]) {
  char me[]="miteSample", err[BIFF_STRLEN];
  mite_t R, G, B, A;
  gage_t *NN;
  double NdotV, kn[3], knd[3], ref[3], len, *dbg=NULL;

  if (!inside) {
    return mtt->rayStep;
  }

  if (mtt->skip) {
    /* we have one verbose pixel, but we're not on it */
    return 0.0;
  }

  /* early ray termination */
  if (1-mtt->TT >= muu->opacNear1) {
    mtt->TT = 0.0;
    return 0.0;
  }

  /* set (fake) view based on fake from */
  if (AIR_EXISTS(muu->fakeFrom[0])) {
    ELL_3V_SUB(mtt->V, samplePosWorld, muu->fakeFrom);
    ELL_3V_NORM(mtt->V, mtt->V, len);
  }

  /* do probing at this location to determine values of everything
     that might appear in the txf domain */
  if (gageProbe(mtt->gctx,
                AIR_CAST(gage_t, samplePosIndex[0]),
                AIR_CAST(gage_t, samplePosIndex[1]),
                AIR_CAST(gage_t, samplePosIndex[2]))) {
    sprintf(err, "%s: gage trouble: %s (%d)", me,
            mtt->gctx->errStr, mtt->gctx->errNum);
    biffAdd(MITE, err); return AIR_NAN;
  }
  
  if (mrr->queryMiteNonzero) {
    /* There is some optimal trade-off between slowing things down
       with too many branches on all possible checks of queryMite,
       and slowing things down with doing the work of setting them all.
       This code has not been profiled whatsoever */
    mtt->directAnsMiteVal[miteValXw][0] = AIR_CAST(gage_t, samplePosWorld[0]);
    mtt->directAnsMiteVal[miteValXi][0] = AIR_CAST(gage_t, samplePosIndex[0]);
    mtt->directAnsMiteVal[miteValYw][0] = AIR_CAST(gage_t, samplePosWorld[1]);
    mtt->directAnsMiteVal[miteValYi][0] = AIR_CAST(gage_t, samplePosIndex[1]);
    mtt->directAnsMiteVal[miteValZw][0] = AIR_CAST(gage_t, samplePosWorld[2]);
    mtt->directAnsMiteVal[miteValZi][0] = AIR_CAST(gage_t, samplePosIndex[2]);
    mtt->directAnsMiteVal[miteValRw][0] = AIR_CAST(gage_t,
                                                   ELL_3V_LEN(samplePosWorld));
    mtt->directAnsMiteVal[miteValRi][0] = AIR_CAST(gage_t,
                                                   ELL_3V_LEN(samplePosIndex));
    mtt->directAnsMiteVal[miteValTw][0] = AIR_CAST(gage_t, rayT);
    mtt->directAnsMiteVal[miteValTi][0] = AIR_CAST(gage_t, num);
    ELL_3V_COPY_TT(mtt->directAnsMiteVal[miteValView], gage_t, mtt->V);
    NN = mtt->directAnsMiteVal[miteValNormal];
    if (mtt->_normal) {
      if (1 == muu->normalSide) {
        ELL_3V_SCALE(NN, -1, mtt->_normal);
      } else {
        ELL_3V_COPY(NN, mtt->_normal);
      }
    }

    if ((GAGE_QUERY_ITEM_TEST(mrr->queryMite, miteValNdotV)
         || GAGE_QUERY_ITEM_TEST(mrr->queryMite, miteValNdotL)
         || GAGE_QUERY_ITEM_TEST(mrr->queryMite, miteValVrefN))) {
      mtt->directAnsMiteVal[miteValNdotV][0] =
        AIR_CAST(gage_t, ELL_3V_DOT(NN, mtt->V));
      mtt->directAnsMiteVal[miteValNdotL][0] =
        ELL_3V_DOT(NN, muu->lit->dir[0]);
      if (!muu->normalSide) {
        mtt->directAnsMiteVal[miteValNdotV][0] =
          AIR_ABS(mtt->directAnsMiteVal[miteValNdotV][0]);
        mtt->directAnsMiteVal[miteValNdotL][0] = 
          AIR_ABS(mtt->directAnsMiteVal[miteValNdotL][0]);
      }
      NdotV = mtt->directAnsMiteVal[miteValNdotV][0];
      ELL_3V_SCALE_ADD2(ref, 2*NdotV, NN, -1, mtt->V);
      ELL_3V_NORM_TT(mtt->directAnsMiteVal[miteValVrefN], gage_t, ref, len);
    }

    if (GAGE_QUERY_ITEM_TEST(mrr->queryMite, miteValGTdotV)) {
      ELL_3MV_MUL(kn, mtt->nPerp, mtt->V);
      ELL_3V_NORM(kn, kn, len);
      ELL_3MV_MUL(knd, mtt->geomTens, kn);
      mtt->directAnsMiteVal[miteValGTdotV][0] = 
        AIR_CAST(gage_t, ELL_3V_DOT(knd, kn));
    }
  }
  

  /* initialize txf range quantities, and apply all txfs */
  if (mtt->verbose) {
    muu->debugIdx = airArrayLenIncr(muu->debugArr, muu->ndebug->axis[0].size);
  }

  memcpy(mtt->range, muu->rangeInit, MITE_RANGE_NUM*sizeof(mite_t));
  _miteStageRun(mtt, muu);

  /* if there's opacity, do shading and compositing */
  if (mtt->range[miteRangeAlpha]) {
    /* fprintf(stderr, "%s: mtt->TT = %g\n", me, mtt->TT); */
    /*
    if (mtt->verbose) {
      fprintf(stderr, "%s: before compositing: RGBT = %g,%g,%g,%g\n",
              me, mtt->RR, mtt->GG, mtt->BB, mtt->TT);
    }
    */
    _miteRGBACalc(&R, &G, &B, &A, mtt, mrr, muu);
    mtt->RR += mtt->TT*A*R;
    mtt->GG += mtt->TT*A*G;
    mtt->BB += mtt->TT*A*B;
    mtt->TT *= 1-A;
    /*
    if (mtt->verbose) {
      fprintf(stderr, "%s: after compositing: RGBT = %g,%g,%g,%g\n",
              me, mtt->RR, mtt->GG, mtt->BB, mtt->TT);
    }
    */
    /* fprintf(stderr, "%s: mtt->TT = %g\n", me, mtt->TT); */
  } else {
    R = G = B = A = 0;
  }
  if (mtt->verbose) {
    dbg = muu->debug + muu->debugIdx;
    dbg[0 + 2*mtt->stageNum] = R;
    dbg[1 + 2*mtt->stageNum] = G;
    dbg[2 + 2*mtt->stageNum] = B;
    dbg[3 + 2*mtt->stageNum] = A;
    dbg[4 + 2*mtt->stageNum] = rayT;
  }

  /* set Z if it hasn't been set already */
  if (1-mtt->TT >= muu->opacMatters && !AIR_EXISTS(mtt->ZZ)) {
    mtt->ZZ = rayT;
  }

  /* this is used to index mtt->debug */
  mtt->raySample += 1;

  return mtt->rayStep;
}

int 
miteRayEnd(miteThread *mtt, miteRender *mrr, miteUser *muu) {
  int idx, slen, stageIdx;
  mite_t *imgData;
  double A;
  
  AIR_UNUSED(mrr);
  mtt->samples += mtt->raySample;
  idx = mtt->ui + (muu->nout->axis[1].size)*mtt->vi;
  imgData = (mite_t*)muu->nout->data;
  A = 1 - mtt->TT;
  if (A) {
    ELL_5V_SET(imgData + 5*idx, mtt->RR/A, mtt->GG/A, mtt->BB/A,
               A, mtt->ZZ);
  } else {
    ELL_5V_SET(imgData + 5*idx, 0, 0, 0, 0, AIR_NAN);
  }
  if (mtt->verbose) {
    /* muu->debug may be over-allocated, but that's harmless */
    muu->ndebug->axis[1].size = mtt->raySample;
    nrrdWrap_va(muu->ndebug, muu->debug, nrrdTypeDouble, 2,
                AIR_CAST(size_t, muu->ndebug->axis[0].size),
                AIR_CAST(size_t, mtt->raySample));
    airArrayNix(muu->debugArr);
    slen = 0;
    for (stageIdx=0; stageIdx<mtt->stageNum; stageIdx++) {
      slen += strlen(mtt->stage[stageIdx].label) + 2;
    }
    slen += strlen("R,G,B,A,Z") + 1;
    muu->ndebug->axis[0].label = (char *)calloc(slen, sizeof(char));
    for (stageIdx=0; stageIdx<mtt->stageNum; stageIdx++) {
      strcat(muu->ndebug->axis[0].label, mtt->stage[stageIdx].label);
      strcat(muu->ndebug->axis[0].label, ",,");
    }      
    strcat(muu->ndebug->axis[0].label, "R,G,B,A,Z");
  }
  return 0;
}

