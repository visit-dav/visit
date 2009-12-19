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

tenGlyphParm *
tenGlyphParmNew() {
  tenGlyphParm *parm;

  parm = (tenGlyphParm *)calloc(1, sizeof(tenGlyphParm));
  if (parm) {
    parm->verbose = 0;
    parm->nmask = NULL;
    parm->anisoType = tenAnisoUnknown;
    parm->onlyPositive = AIR_TRUE;
    parm->confThresh = AIR_NAN;
    parm->anisoThresh = AIR_NAN;
    parm->maskThresh = AIR_NAN;

    parm->glyphType = tenGlyphTypeUnknown;
    parm->facetRes = 10;
    parm->glyphScale = 1.0;
    parm->sqdSharp = 3.0;
    ELL_5V_SET(parm->edgeWidth, 0.0f, 0.0f, 0.4f, 0.2f, 0.1f);

    parm->colEvec = 0;  /* first */
    parm->colMaxSat = 1; 
    parm->colGamma = 1;
    parm->colIsoGray = 1;
    parm->colAnisoType = tenAnisoUnknown;
    parm->colAnisoModulate = 0;
    ELL_4V_SET(parm->ADSP, 0, 1, 0, 30);

    parm->doSlice = AIR_FALSE;
    parm->sliceAxis = 0;
    parm->slicePos = 0;
    parm->sliceAnisoType = tenAnisoUnknown;
    parm->sliceOffset = 0.0;
    parm->sliceBias = 0.05f;
    parm->sliceGamma = 1.0;
  }
  return parm;
}

tenGlyphParm *
tenGlyphParmNix(tenGlyphParm *parm) {

  airFree(parm);
  return NULL;
}

int
tenGlyphParmCheck(tenGlyphParm *parm,
                  const Nrrd *nten, const Nrrd *npos, const Nrrd *nslc) {
  char me[]="tenGlyphParmCheck", err[BIFF_STRLEN];
  int duh;
  size_t tenSize[3];

  if (!(parm && nten)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenAniso, parm->anisoType)) {
    sprintf(err, "%s: unset (or invalid) anisoType (%d)", me, parm->anisoType);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenAniso, parm->colAnisoType)) {
    sprintf(err, "%s: unset (or invalid) colAnisoType (%d)",
            me, parm->colAnisoType);
    biffAdd(TEN, err); return 1;
  }
  if (!( parm->facetRes >= 3 )) {
    sprintf(err, "%s: facet resolution %d not >= 3", me, parm->facetRes);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_IN_OP(tenGlyphTypeUnknown, parm->glyphType,
                   tenGlyphTypeLast) )) {
    sprintf(err, "%s: unset (or invalid) glyphType (%d)", me, parm->glyphType);
    biffAdd(TEN, err); return 1;
  }
  if (!( parm->glyphScale > 0)) {
    sprintf(err, "%s: glyphScale must be > 0 (not %g)", me, parm->glyphScale);
    biffAdd(TEN, err); return 1;
  }
  if (parm->nmask) {
    if (npos) {
      sprintf(err, "%s: can't do masking with explicit coordinate list", me);
      biffAdd(TEN, err); return 1;
    }
    if (!( 3 == parm->nmask->dim
           && parm->nmask->axis[0].size == nten->axis[1].size
           && parm->nmask->axis[1].size == nten->axis[2].size
           && parm->nmask->axis[2].size == nten->axis[3].size )) {
      sprintf(err, "%s: mask isn't 3-D or doesn't have sizes ("
              _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV ")", me,
              nten->axis[1].size, nten->axis[2].size, nten->axis[3].size);
      biffAdd(TEN, err); return 1;
    }
    if (!(AIR_EXISTS(parm->maskThresh))) {
      sprintf(err, "%s: maskThresh hasn't been set", me);
      biffAdd(TEN, err); return 1;
    }
  }
  if (!( AIR_EXISTS(parm->anisoThresh)
         && AIR_EXISTS(parm->confThresh) )) {
    sprintf(err, "%s: anisoThresh and confThresh haven't both been set", me);
    biffAdd(TEN, err); return 1;
  }
  if (parm->doSlice) {
    if (npos) {
      sprintf(err, "%s: can't do slice with explicit coordinate list", me);
      biffAdd(TEN, err); return 1;
    }
    if (!( parm->sliceAxis <=2 )) {
      sprintf(err, "%s: slice axis %d invalid", me, parm->sliceAxis);
      biffAdd(TEN, err); return 1;
    }
    if (!( parm->slicePos < nten->axis[1+parm->sliceAxis].size )) {
      sprintf(err, "%s: slice pos " _AIR_SIZE_T_CNV 
              " not in valid range [0.." _AIR_SIZE_T_CNV "]", me,
              parm->slicePos, nten->axis[1+parm->sliceAxis].size-1);
      biffAdd(TEN, err); return 1;
    }
    if (nslc) {
      if (2 != nslc->dim) {
        sprintf(err, "%s: explicit slice must be 2-D (not %d)", me, nslc->dim);
        biffAdd(TEN, err); return 1;
      }
      tenSize[0] = nten->axis[1].size;
      tenSize[1] = nten->axis[2].size;
      tenSize[2] = nten->axis[3].size;
      for (duh=parm->sliceAxis; duh<2; duh++) {
        tenSize[duh] = tenSize[duh+1];
      }
      if (!( tenSize[0] == nslc->axis[0].size
             && tenSize[1] == nslc->axis[1].size )) {
        sprintf(err, "%s: axis %u slice of " 
                _AIR_SIZE_T_CNV "x" _AIR_SIZE_T_CNV "x" _AIR_SIZE_T_CNV 
                " volume is not " _AIR_SIZE_T_CNV "x" _AIR_SIZE_T_CNV , me,
                parm->sliceAxis, nten->axis[1].size, nten->axis[2].size,
                nten->axis[3].size, nslc->axis[0].size, nslc->axis[1].size);
        biffAdd(TEN, err); return 1;
      }
    } else {
      if (airEnumValCheck(tenAniso, parm->sliceAnisoType)) {
        sprintf(err, "%s: unset (or invalid) sliceAnisoType (%d)",
                me, parm->sliceAnisoType);
        biffAdd(TEN, err); return 1;
      }
    }
  }
  return 0;
}

int
tenGlyphGen(limnObject *glyphsLimn, echoScene *glyphsEcho,
            tenGlyphParm *parm,
            const Nrrd *nten, const Nrrd *npos, const Nrrd *nslc) {
  char me[]="tenGlyphGen", err[BIFF_STRLEN];
  gageShape *shape;
  airArray *mop;
  float *tdata, eval[3], evec[9], *cvec, rotEvec[9], mA_f[16],
    aniso[TEN_ANISO_MAX+1];
  double pI[3], pW[3], cl, cp, sRot[16], mA[16], mB[16], msFr[9], tmpvec[3],
    R, G, B, qA, qB, glyphAniso, sliceGray;
  unsigned int duh;
  int slcCoord[3], idx, _idx=0, glyphIdx, axis, numGlyphs,
    svRGBAfl=AIR_FALSE;
  limnLook *look; int lookIdx;
  echoObject *eglyph, *inst, *list=NULL, *split, *esquare;
  echoPos_t eM[16], originOffset[3], edge0[3], edge1[3];
  /*
  int eret;
  double tmp1[3], tmp2[3];  
  */

  if (!( (glyphsLimn || glyphsEcho) && nten && parm)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  mop = airMopNew();
  shape = gageShapeNew();
  shape->defaultCenter = nrrdCenterCell;
  airMopAdd(mop, shape, (airMopper)gageShapeNix, airMopAlways);
  if (npos) {
    if (!( 2 == nten->dim && 7 == nten->axis[0].size )) {
      sprintf(err, "%s: nten isn't 2-D 7-by-N array", me);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
    if (!( 2 == npos->dim && 3 == npos->axis[0].size
           && nten->axis[1].size == npos->axis[1].size )) {
      sprintf(err, "%s: npos isn't 2-D 3-by-" _AIR_SIZE_T_CNV " array", 
              me, nten->axis[1].size);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
    if (!( nrrdTypeFloat == nten->type && nrrdTypeFloat == npos->type )) {
      sprintf(err, "%s: nten and npos must be %s, not %s and %s", me,
              airEnumStr(nrrdType, nrrdTypeFloat),
              airEnumStr(nrrdType, nten->type),
              airEnumStr(nrrdType, npos->type));
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
  } else {
    if (tenTensorCheck(nten, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
      sprintf(err, "%s: didn't get a valid DT volume", me);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
  }
  if (tenGlyphParmCheck(parm, nten, npos, nslc)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  if (!npos) {
    if (gageShapeSet(shape, nten, tenGageKind->baseDim)) {
      sprintf(err, "%s: trouble", me);
      biffMove(TEN, err, GAGE); airMopError(mop); return 1;
    }
  }
  if (parm->doSlice) {
    ELL_3V_COPY(edge0, shape->voxLen);
    ELL_3V_COPY(edge1, shape->voxLen);
    edge0[parm->sliceAxis] = edge1[parm->sliceAxis] = 0.0;
    switch(parm->sliceAxis) {
    case 0:
      edge0[1] = edge1[2] = 0;
      ELL_4M_ROTATE_Y_SET(sRot, AIR_PI/2);
      break;
    case 1:
      edge0[0] = edge1[2] = 0;
      ELL_4M_ROTATE_X_SET(sRot, AIR_PI/2);
      break;
    case 2: default:
      edge0[0] = edge1[1] = 0;
      ELL_4M_IDENTITY_SET(sRot);
      break;
    }
    ELL_3V_COPY(originOffset, shape->voxLen);
    ELL_3V_SCALE(originOffset, -0.5, originOffset);
    originOffset[parm->sliceAxis] *= -2*parm->sliceOffset;
  }
  if (glyphsLimn) {
    /* create limnLooks for diffuse and ambient-only shading */
    /* ??? */
    /* hack: save old value of setVertexRGBAFromLook, and set to true */
    svRGBAfl = glyphsLimn->setVertexRGBAFromLook;
    glyphsLimn->setVertexRGBAFromLook = AIR_TRUE;
  }
  if (glyphsEcho) {
    list = echoObjectNew(glyphsEcho, echoTypeList);
  }
  if (npos) {
    numGlyphs = nten->axis[1].size;
  } else {
    numGlyphs = shape->size[0] * shape->size[1] * shape->size[2];
  }
  /* find measurement frame transform */
  if (3 == nten->spaceDim 
      && AIR_EXISTS(nten->measurementFrame[0][0])) {
    /*     msFr        nten->measurementFrame
    **   0  1  2      [0][0]   [1][0]   [2][0]
    **   3  4  5      [0][1]   [1][1]   [2][1]
    **   6  7  8      [0][2]   [1][2]   [2][2]
    */
    msFr[0] = nten->measurementFrame[0][0];
    msFr[3] = nten->measurementFrame[0][1];
    msFr[6] = nten->measurementFrame[0][2];
    msFr[1] = nten->measurementFrame[1][0];
    msFr[4] = nten->measurementFrame[1][1];
    msFr[7] = nten->measurementFrame[1][2];
    msFr[2] = nten->measurementFrame[2][0];
    msFr[5] = nten->measurementFrame[2][1];
    msFr[8] = nten->measurementFrame[2][2];
  } else {
    ELL_3M_IDENTITY_SET(msFr);
  }
  for (idx=0; idx<numGlyphs; idx++, _idx = idx) {
    tdata = (float*)(nten->data) + 7*idx;
    if (parm->verbose >= 2) {
      fprintf(stderr, "%s: glyph %d/%d: hello %g    %g %g %g %g %g %g\n",
              me, idx, numGlyphs, tdata[0], 
              tdata[1], tdata[2], tdata[3], 
              tdata[4], tdata[5], tdata[6]);
    }
    if (!( TEN_T_EXISTS(tdata) )) {
      /* there's nothing we can do here */
      if (parm->verbose >= 2) {
        fprintf(stderr, "%s: glyph %d/%d: non-existant data\n",
                me, idx, numGlyphs);
      }
      continue;
    }
    if (npos) {
      ELL_3V_COPY(pW, (float*)(npos->data) + 3*idx);
      if (!( AIR_EXISTS(pW[0]) && AIR_EXISTS(pW[1]) && AIR_EXISTS(pW[2]) )) {
        /* position doesn't exist- perhaps because its from the push
           library, which might kill points by setting coords to nan */
        continue;
      }
    } else {
      NRRD_COORD_GEN(pI, shape->size, 3, _idx);
      /* this does take into account full orientation */
      gageShapeItoW(shape, pW, pI);
      if (parm->nmask) {
        if (!( nrrdFLookup[parm->nmask->type](parm->nmask->data, idx)
               >= parm->maskThresh )) {
          if (parm->verbose >= 2) {
            fprintf(stderr, "%s: glyph %d/%d: doesn't meet mask thresh\n",
                    me, idx, numGlyphs);
          }
          continue;
        }
      }
    }
    tenEigensolve_f(eval, evec, tdata);
    /* transform eigenvectors by measurement frame */
    ELL_3MV_MUL(tmpvec, msFr, evec + 0);
    ELL_3V_COPY_TT(evec + 0, float, tmpvec);
    ELL_3MV_MUL(tmpvec, msFr, evec + 3);
    ELL_3V_COPY_TT(evec + 3, float, tmpvec);
    ELL_3MV_MUL(tmpvec, msFr, evec + 6);
    ELL_3V_COPY_TT(evec + 6, float, tmpvec);
    ELL_3V_CROSS(tmpvec, evec + 0, evec + 3);
    if (0 > ELL_3V_DOT(tmpvec, evec + 6)) {
      ELL_3V_SCALE(evec + 6, -1, evec + 6);
    }
    ELL_3M_TRANSPOSE(rotEvec, evec);
    tenAnisoCalc_f(aniso, eval);
    if (parm->doSlice
        && pI[parm->sliceAxis] == parm->slicePos) {
      /* set sliceGray */
      if (nslc) {
        /* we aren't masked by confidence, as anisotropy slice is */
        for (duh=0; duh<parm->sliceAxis; duh++) {
          slcCoord[duh] = (int)(pI[duh]);
        }
        for (duh=duh<parm->sliceAxis; duh<2; duh++) {
          slcCoord[duh] = (int)(pI[duh+1]);
        }
        /* HEY: GLK has no idea what's going here */
        slcCoord[0] = (int)(pI[0]);
        slcCoord[1] = (int)(pI[1]);
        slcCoord[2] = (int)(pI[2]);
        sliceGray = 
          nrrdFLookup[nslc->type](nslc->data, slcCoord[0] 
                                  + nslc->axis[0].size*slcCoord[1]);
      } else {
        if (!( tdata[0] >= parm->confThresh )) {
          if (parm->verbose >= 2) {
            fprintf(stderr, "%s: glyph %d/%d (slice): conf %g < thresh %g\n",
                    me, idx, numGlyphs, tdata[0], parm->confThresh);
          }
          continue;
        }
        sliceGray = aniso[parm->sliceAnisoType];
      }
      if (parm->sliceGamma > 0) {
        sliceGray = AIR_AFFINE(0, sliceGray, 1, parm->sliceBias, 1);
        sliceGray = pow(sliceGray, 1.0/parm->sliceGamma);
      } else {
        sliceGray = AIR_AFFINE(0, sliceGray, 1, 0, 1-parm->sliceBias);
        sliceGray = 1.0 - pow(sliceGray, -1.0/parm->sliceGamma);
      }
      /* make slice contribution */
      /* HEY: this is *NOT* aware of shape->fromOrientation */
      if (glyphsLimn) {
        lookIdx = limnObjectLookAdd(glyphsLimn);
        look = glyphsLimn->look + lookIdx;
        ELL_4V_SET_TT(look->rgba, float, sliceGray, sliceGray, sliceGray, 1);
        ELL_3V_SET(look->kads, 1, 0, 0);
        look->spow = 0;
        glyphIdx = limnObjectSquareAdd(glyphsLimn, lookIdx);
        ELL_4M_IDENTITY_SET(mA);
        ell_4m_post_mul_d(mA, sRot);
        if (!npos) {
          ELL_4M_SCALE_SET(mB,
                           shape->voxLen[0],
                           shape->voxLen[1],
                           shape->voxLen[2]);
        }
        ell_4m_post_mul_d(mA, mB);
        ELL_4M_TRANSLATE_SET(mB, pW[0], pW[1], pW[2]);
        ell_4m_post_mul_d(mA, mB);
        ELL_4M_TRANSLATE_SET(mB,
                             originOffset[0],
                             originOffset[1],
                             originOffset[2]);
        ell_4m_post_mul_d(mA, mB);
        ELL_4M_COPY_TT(mA_f, float, mA);
        limnObjectPartTransform(glyphsLimn, glyphIdx, mA_f);
      }
      if (glyphsEcho) {
        esquare = echoObjectNew(glyphsEcho,echoTypeRectangle);
        ELL_3V_ADD2(((echoRectangle*)esquare)->origin, pW, originOffset);
        ELL_3V_COPY(((echoRectangle*)esquare)->edge0, edge0);
        ELL_3V_COPY(((echoRectangle*)esquare)->edge1, edge1);
        echoColorSet(esquare,
                     AIR_CAST(echoCol_t, sliceGray),
                     AIR_CAST(echoCol_t, sliceGray),
                     AIR_CAST(echoCol_t, sliceGray), 1);
        /* this is pretty arbitrary- but I want shadows to have some effect.
           Previously, the material was all ambient: (A,D,S) = (1,0,0),
           which avoided all shadow effects. */
        echoMatterPhongSet(glyphsEcho, esquare, 0.4f, 0.6f, 0, 40);
        echoListAdd(list, esquare);
      }
    }
    if (parm->onlyPositive) {
      if (eval[2] < 0) {
        /* didn't have all positive eigenvalues, its outta here */
        if (parm->verbose >= 2) {
          fprintf(stderr, "%s: glyph %d/%d: not all evals %g %g %g > 0\n",
                  me, idx, numGlyphs, eval[0], eval[1], eval[2]);
        }
        continue;
      }
    }
    if (!( tdata[0] >= parm->confThresh )) {
      if (parm->verbose >= 2) {
        fprintf(stderr, "%s: glyph %d/%d: conf %g < thresh %g\n",
                me, idx, numGlyphs, tdata[0], parm->confThresh);
      }
      continue;
    }
    if (!( aniso[parm->anisoType] >= parm->anisoThresh )) {
      if (parm->verbose >= 2) {
        fprintf(stderr, "%s: glyph %d/%d: aniso[%d] %g < thresh %g\n",
                me, idx, numGlyphs, parm->anisoType,
                aniso[parm->anisoType], parm->anisoThresh);
      }
      continue;
    }
    glyphAniso = aniso[parm->colAnisoType];
    /*
      fprintf(stderr, "%s: eret = %d; evals = %g %g %g\n", me,
      eret, eval[0], eval[1], eval[2]);
      ELL_3V_CROSS(tmp1, evec+0, evec+3); tmp2[0] = ELL_3V_LEN(tmp1);
      ELL_3V_CROSS(tmp1, evec+0, evec+6); tmp2[1] = ELL_3V_LEN(tmp1);
      ELL_3V_CROSS(tmp1, evec+3, evec+6); tmp2[2] = ELL_3V_LEN(tmp1);
      fprintf(stderr, "%s: crosses = %g %g %g\n", me,
      tmp2[0], tmp2[1], tmp2[2]);
    */
    
    /* set transform (in mA) */
    ELL_4M_IDENTITY_SET(mA);                        /* reset */
    ELL_3V_SCALE(eval, parm->glyphScale, eval);     /* scale by evals */
    ELL_4M_SCALE_SET(mB, eval[0], eval[1], eval[2]);

    ell_4m_post_mul_d(mA, mB);
    ELL_43M_INSET(mB, rotEvec);                     /* rotate by evecs */
    ell_4m_post_mul_d(mA, mB);
    ELL_4M_TRANSLATE_SET(mB, pW[0], pW[1], pW[2]);  /* translate */
    ell_4m_post_mul_d(mA, mB);

    /* set color (in R,G,B) */
    cvec = evec + 3*(AIR_CLAMP(0, parm->colEvec, 2));
    R = AIR_ABS(cvec[0]);                           /* standard mapping */
    G = AIR_ABS(cvec[1]);
    B = AIR_ABS(cvec[2]);
    /* desaturate by colMaxSat */
    R = AIR_AFFINE(0.0, parm->colMaxSat, 1.0, parm->colIsoGray, R);
    G = AIR_AFFINE(0.0, parm->colMaxSat, 1.0, parm->colIsoGray, G);
    B = AIR_AFFINE(0.0, parm->colMaxSat, 1.0, parm->colIsoGray, B);
    /* desaturate some by anisotropy */
    R = AIR_AFFINE(0.0, parm->colAnisoModulate, 1.0,
                   R, AIR_AFFINE(0.0, glyphAniso, 1.0, parm->colIsoGray, R));
    G = AIR_AFFINE(0.0, parm->colAnisoModulate, 1.0,
                   G, AIR_AFFINE(0.0, glyphAniso, 1.0, parm->colIsoGray, G));
    B = AIR_AFFINE(0.0, parm->colAnisoModulate, 1.0,
                   B, AIR_AFFINE(0.0, glyphAniso, 1.0, parm->colIsoGray, B));
    /* clamp and do gamma */
    R = AIR_CLAMP(0.0, R, 1.0);
    G = AIR_CLAMP(0.0, G, 1.0);
    B = AIR_CLAMP(0.0, B, 1.0);
    R = pow(R, parm->colGamma);
    G = pow(G, parm->colGamma);
    B = pow(B, parm->colGamma);
    
    /* which is the axis of revolution */
    cl = AIR_MIN(0.99, aniso[tenAniso_Cl1]);
    cp = AIR_MIN(0.99, aniso[tenAniso_Cp1]);
    if (cl > cp) {
      axis = 0;
      qA = pow(1-cp, parm->sqdSharp);
      qB = pow(1-cl, parm->sqdSharp);
    } else {
      axis = 2;
      qA = pow(1-cl, parm->sqdSharp);
      qB = pow(1-cp, parm->sqdSharp);
    }
    
    /* add the glyph */
    if (parm->verbose >= 2) {
      fprintf(stderr, "%s: glyph %d/%d: the glyph stays!\n",
              me, idx, numGlyphs);
    }
    if (glyphsLimn) {
      lookIdx = limnObjectLookAdd(glyphsLimn);
      look = glyphsLimn->look + lookIdx;
      ELL_4V_SET_TT(look->rgba, float, R, G, B, 1);
      ELL_3V_SET(look->kads, parm->ADSP[0], parm->ADSP[1], parm->ADSP[2]);
      look->spow = 0;
      switch(parm->glyphType) {
      case tenGlyphTypeBox:
        glyphIdx = limnObjectCubeAdd(glyphsLimn, lookIdx);
        break;
      case tenGlyphTypeSphere:
        glyphIdx = limnObjectPolarSphereAdd(glyphsLimn, lookIdx, axis,
                                            2*parm->facetRes, parm->facetRes);
        break;
      case tenGlyphTypeCylinder:
        glyphIdx = limnObjectCylinderAdd(glyphsLimn, lookIdx, axis,
                                         parm->facetRes);
        break;
      case tenGlyphTypeSuperquad:
      default:
        glyphIdx = limnObjectPolarSuperquadAdd(glyphsLimn, lookIdx, axis,
                                               AIR_CAST(float, qA),
                                               AIR_CAST(float, qB),
                                               2*parm->facetRes,
                                               parm->facetRes);
        break;
      }
      ELL_4M_COPY_TT(mA_f, float, mA);
      limnObjectPartTransform(glyphsLimn, glyphIdx, mA_f);
    }
    if (glyphsEcho) {
      switch(parm->glyphType) {
      case tenGlyphTypeBox:
        eglyph = echoObjectNew(glyphsEcho, echoTypeCube);
        /* nothing else to set */
        break;
      case tenGlyphTypeSphere:
        eglyph = echoObjectNew(glyphsEcho, echoTypeSphere);
        echoSphereSet(eglyph, 0, 0, 0, 1);
        break;
      case tenGlyphTypeCylinder:
        eglyph = echoObjectNew(glyphsEcho, echoTypeCylinder);
        echoCylinderSet(eglyph, axis);
        break;
      case tenGlyphTypeSuperquad:
      default:
        eglyph = echoObjectNew(glyphsEcho, echoTypeSuperquad);
        echoSuperquadSet(eglyph, axis, qA, qB);
        break;
      }
      echoColorSet(eglyph,
                   AIR_CAST(echoCol_t, R),
                   AIR_CAST(echoCol_t, G),
                   AIR_CAST(echoCol_t, B), 1);
      echoMatterPhongSet(glyphsEcho, eglyph, 
                         parm->ADSP[0], parm->ADSP[1],
                         parm->ADSP[2], parm->ADSP[3]);
      inst = echoObjectNew(glyphsEcho, echoTypeInstance);
      ELL_4M_COPY(eM, mA);
      echoInstanceSet(inst, eM, eglyph);
      echoListAdd(list, inst);
    }
  }
  if (glyphsLimn) {
    glyphsLimn->setVertexRGBAFromLook = svRGBAfl;
  }
  if (glyphsEcho) {
    split = echoListSplit3(glyphsEcho, list, 10);
    echoObjectAdd(glyphsEcho, split);
  }
  
  airMopOkay(mop);
  return 0;
}
