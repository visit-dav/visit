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

miteUser *
miteUserNew() {
  miteUser *muu;
  int i;

  muu = (miteUser *)calloc(1, sizeof(miteUser));
  if (!muu)
    return NULL;

  muu->umop = airMopNew();
  muu->nsin = NULL;
  muu->nvin = NULL;
  muu->ntin = NULL;
  muu->ntxf = NULL;              /* managed by user (with miter: hest) */
  muu->nout = NULL;              /* managed by user (with miter: hest) */
  muu->debug = NULL;
  muu->debugArr = NULL;
  muu->ndebug = NULL;            /* not allocated until the debug pixel
                                    is rendered, see miteRayBegin */
  muu->ntxfNum = 0;
  muu->shadeStr[0] = 0;
  muu->normalStr[0] = 0;
  for (i=0; i<MITE_RANGE_NUM; i++) {
    muu->rangeInit[i] = 1.0;
  }
  muu->normalSide = miteDefNormalSide;
  muu->refStep = miteDefRefStep;
  muu->rayStep = AIR_NAN;
  muu->opacMatters = miteDefOpacMatters;
  muu->opacNear1 = miteDefOpacNear1;
  muu->hctx = hooverContextNew();
  ELL_3V_SET(muu->fakeFrom, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3V_SET(muu->vectorD, 0, 0, 0);
  airMopAdd(muu->umop, muu->hctx, (airMopper)hooverContextNix, airMopAlways);
  for (i=0; i<GAGE_KERNEL_NUM; i++) {
    muu->ksp[i] = NULL;
  }
  muu->gctx0 = gageContextNew();
  airMopAdd(muu->umop, muu->gctx0, (airMopper)gageContextNix, airMopAlways);
  gageParmSet(muu->gctx0, gageParmRequireAllSpacings, AIR_FALSE);
  muu->lit = limnLightNew();
  airMopAdd(muu->umop, muu->lit, (airMopper)limnLightNix, airMopAlways);
  muu->normalSide = miteDefNormalSide;
  muu->verbUi = muu->verbVi = -1;
  muu->rendTime = 0;
  muu->sampRate = 0;
  return muu;
}

miteUser *
miteUserNix(miteUser *muu) {

  if (muu) {
    airMopOkay(muu->umop);
    airFree(muu);
  }
  return NULL;
}

int
_miteUserCheck(miteUser *muu) {
  char me[]="miteUserCheck", err[BIFF_STRLEN];
  int T, gotOpac;
  gageItemSpec isp;
  gageQuery queryScl, queryVec, queryTen, queryMite;
  miteShadeSpec *shpec;
  airArray *mop;
  unsigned int axi;
  
  if (!muu) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(MITE, err); return 1;
  }
  mop = airMopNew();
  if (!( muu->ntxfNum >= 1 )) {
    sprintf(err, "%s: need at least one transfer function", me);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }
  gotOpac = AIR_FALSE;
  GAGE_QUERY_RESET(queryScl);
  GAGE_QUERY_RESET(queryVec);
  GAGE_QUERY_RESET(queryTen);
  GAGE_QUERY_RESET(queryMite);  /* not actually used here */

  /* add on all queries associated with transfer functions */
  for (T=0; T<muu->ntxfNum; T++) {
    if (miteNtxfCheck(muu->ntxf[T])) {
      sprintf(err, "%s: ntxf[%d] (%d of %d) can't be used as a txf",
              me, T, T+1, muu->ntxfNum);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    /* NOTE: no error checking because miteNtxfCheck succeeded */
    for (axi=1; axi<muu->ntxf[T]->dim; axi++) {
      miteVariableParse(&isp, muu->ntxf[T]->axis[axi].label);
      miteQueryAdd(queryScl, queryVec, queryTen, queryMite, &isp);
    }
    gotOpac |= !!strchr(muu->ntxf[T]->axis[0].label, 'A');
  }
  if (!gotOpac) {
    fprintf(stderr, "\n\n%s: ****************************************"
            "************************\n", me);
    fprintf(stderr, "%s: !!! WARNING !!! opacity (\"A\") not set "
            "by any transfer function\n", me);
    fprintf(stderr, "%s: ****************************************"
            "************************\n\n\n", me);
  }

  /* add on "normal"-based queries */
  if (airStrlen(muu->normalStr)) {
    miteVariableParse(&isp, muu->normalStr);
    if (miteValGageKind == isp.kind) {
      sprintf(err, "%s: normalStr \"%s\" refers to a miteVal "
              "(normal must be data-intrinsic)", me, muu->normalStr);
      biffAdd(MITE, err); airMopError(mop); return 1;
    }
    if (3 != isp.kind->table[isp.item].answerLength) {
      sprintf(err, "%s: %s not a vector: can't be used as normal",
              me, muu->normalStr);
      biffAdd(MITE, err); return 1;
    }
    miteQueryAdd(queryScl, queryVec, queryTen, queryMite, &isp);
  }

  /* add on shading-based queries */
  shpec = miteShadeSpecNew();
  airMopAdd(mop, shpec, (airMopper)miteShadeSpecNix, airMopAlways);
  if (miteShadeSpecParse(shpec, muu->shadeStr)) {
    sprintf(err, "%s: couldn't parse shading spec \"%s\"", 
            me, muu->shadeStr);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }
  miteShadeSpecQueryAdd(queryScl, queryVec, queryTen, queryMite, shpec);

  /* see if anyone asked for an unspecified normal */
  if ((GAGE_QUERY_ITEM_TEST(queryMite, miteValNdotV)
       || GAGE_QUERY_ITEM_TEST(queryMite, miteValNdotL)
       || GAGE_QUERY_ITEM_TEST(queryMite, miteValVrefN))
      && !airStrlen(muu->normalStr)) {
    sprintf(err, "%s: txf or shading requested a miteVal's use of the "
            "\"normal\", but one has not been specified in muu->normalStr",
            me);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }

  /* see if we have volumes for requested queries */
  if (GAGE_QUERY_NONZERO(queryScl) && !(muu->nsin)) {
    sprintf(err, "%s: txf or shading require %s volume, but don't have one",
            me, gageKindScl->name);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }
  if (GAGE_QUERY_NONZERO(queryVec) && !(muu->nvin)) {
    sprintf(err, "%s: txf or shading require %s volume, but don't have one",
            me, gageKindVec->name);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }
  if (GAGE_QUERY_NONZERO(queryTen) && !(muu->ntin)) {
    sprintf(err, "%s: txf or shading require %s volume, but don't have one",
            me, tenGageKind->name);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }

  /* check appropriateness of given volumes */
  if (muu->nsin) {
    if (gageVolumeCheck(muu->gctx0, muu->nsin, gageKindScl)) {
      sprintf(err, "%s: trouble with input %s volume",
              me, gageKindScl->name);
      biffMove(MITE, err, GAGE); airMopError(mop); return 1;
    }
  }
  if (muu->nvin) {
    if (gageVolumeCheck(muu->gctx0, muu->nvin, gageKindVec)) {
      sprintf(err, "%s: trouble with input %s volume", 
              me, gageKindVec->name);
      biffMove(MITE, err, GAGE); airMopError(mop); return 1;
    }
  }
  if (muu->ntin) {
    if (gageVolumeCheck(muu->gctx0, muu->ntin, tenGageKind)) {
      sprintf(err, "%s: trouble with input %s volume", 
              me, tenGageKind->name);
      biffMove(MITE, err, GAGE); airMopError(mop); return 1;
    }
  }

  if (!muu->nout) {
    sprintf(err, "%s: rendered image nrrd is NULL", me);
    biffAdd(MITE, err); airMopError(mop); return 1;
  }
  airMopOkay(mop); 
  return 0;
}
